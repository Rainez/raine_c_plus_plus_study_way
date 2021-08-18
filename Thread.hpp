#ifndef __RAINE_THREAD__
#define __RAINE_THREAD__
#include "noncopyable.hpp"
#include <functional>
#include <sys/types.h>
#include <pthread.h>
#include <iostream>
#include <memory>
#include <functional>
#include "CountDownLatch.hpp"
#include <atomic>
namespace raine {
    class Thread : noncopyable {
    public:
        typedef std::function<void()> ThreadFunc;
        explicit Thread(ThreadFunc, const std::string& name = std::string());
        ~Thread();

        void start();
        int join();
        bool started() const { return started_; }
        pid_t tid() const { return tid_; }
        const std::string& name() const { return name_; }
        static int numCreated() { return numCreated_; }
    private:
        void setDefaultName();
        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        pid_t tid_;
        ThreadFunc func_;
        std::string name_;
        raine::CountDownLatch latch_;

        static std::atomic_int32_t numCreated_;
    };
};
#endif