#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <atomic>
#include <string>
#include "CurrentThread.h"
#include "Thread.hpp"
#include "Exception.h"
#include "Timestamp.h"
namespace raine {
    namespace detail {
        pid_t gettid() {
            uint64_t tmp;
            pthread_threadid_np(nullptr, &tmp);
            return tmp;
        }

        void afterFork() {
            raine::CurrentThread::t_cachedTid = 0;
            raine::CurrentThread::t_threadName = "main";
            raine::CurrentThread::tid();
        }

        class ThreadNameInitializer {
        public:
            ThreadNameInitializer() 
            {
                CurrentThread::t_threadName = "main";
                CurrentThread::tid();
                pthread_atfork(nullptr,nullptr,&afterFork);
            }
        };

        ThreadNameInitializer init;

        struct ThreadData {
            typedef Thread::ThreadFunc ThreadFunc;
            ThreadFunc func_;
            std::string name_;
            pid_t* tid_;
            CountDownLatch* latch_;

            ThreadData(ThreadFunc func,
                       const std::string& name,
                       pid_t* tid,
                       CountDownLatch* latch): func_(std::move(func)),
                                               name_(name),
                                               tid_(tid),
                                               latch_(latch) {}

            void runInThread() {
                *tid_ = CurrentThread::tid();
                tid_ = nullptr;
                latch_->countDown();
                latch_ = nullptr;
                CurrentThread::t_threadName = name_.empty() ? "raineThread" : name_.c_str();
                pthread_setname_np(CurrentThread::t_threadName);
                try {
                    func_();
                    CurrentThread::t_threadName = "finished";
                } catch(const Exception& ex) {
                    CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                    fprintf(stderr, "reason: %s\n", ex.what());
                    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
                    abort();
                }
                catch(const std::exception& ex) {
                    CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                    fprintf(stderr, "reason: %s\n", ex.what());
                    abort();
                }
                catch(...) {
                    CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
                    throw;
                }
            }
        };

        void* startThread(void* obj) {
            ThreadData* data = static_cast<ThreadData*>(obj);
            data->runInThread();
            delete data;
            return nullptr;
        }
    } // namespace detail 

    void CurrentThread::cacheTid() {
        if (t_cachedTid == 0) {
            std::cout << "t_cachedTid fetch begin" << std::endl;
            t_cachedTid = detail::gettid();
            std::cout << "t_cachedTid fetch end " << t_cachedTid << std::endl;
            t_tidStringLength = snprintf(t_tidString,sizeof t_tidString, "%5d", t_cachedTid);
        }
    }

    bool CurrentThread::isMainThread() {
        return tid() == ::getpid();
    }

    void CurrentThread::sleepUsec(int64_t usec) {
        struct timespec ts = {0, 0};
        ts.tv_sec = static_cast<time_t>(usec/Timestamp::kMicroSecondsPerSeconds);
        ts.tv_nsec = static_cast<long>(usec%Timestamp::kMicroSecondsPerSeconds*1000);
        ::nanosleep(&ts,nullptr);
    }

    std::atomic_int32_t Thread::numCreated_;
    
    Thread::Thread(ThreadFunc func, const std::string& name)
        :   started_(false),
            joined_(false),
            pthreadId_(0),
            tid_(0),
            func_(std::move(func)),
            name_(name),
            latch_(1)
         {
             setDefaultName();
         }

    Thread::~Thread() {
        if (started_ && !joined_) {
            pthread_detach(pthreadId_);
        }
    }
    
    void Thread::setDefaultName() {
        auto num = ++numCreated_;
        if (name_.empty()) {
            char buf[32];
            snprintf(buf,sizeof(buf),"Thread%d", num);
            name_ = buf;
        }
    }

    void Thread::start() {
        assert(!started_);
        started_ = true;
        detail::ThreadData* data = new detail::ThreadData(func_,name_,&tid_,&latch_);
        if (pthread_create(&pthreadId_, nullptr, &detail::startThread, data))
        {
            started_ = false;
            delete data;
            // pthread创建失败
        }
        else {
            latch_.await();
            assert(tid_>0);
        }
    }

    int Thread::join() {
        assert(started_);
        assert(!joined_);
        joined_ = true;
        return pthread_join(pthreadId_, nullptr);
    }
}