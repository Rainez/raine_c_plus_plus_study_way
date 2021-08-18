#include "noncopyable.hpp"
#include "MutexGuard.hpp"
#include <pthread.h>
#ifndef __RAINE__CONDITION__
#define __RAINE__CONDITION__
namespace raine {
    class Condition: noncopyable {
    public:
        explicit Condition(MutexLock& mutex): mutex_(mutex) {
            pthread_cond_init(&pcond_, nullptr);
        }
        
        ~Condition() {
            pthread_cond_destroy(&pcond_);
        }

        void wait() noexcept {
            // 可以理解为当这个函数返回的时候；对应的锁又重新获取了
            pthread_cond_wait(&pcond_, mutex_.getPthreadMutex());
        }
        
        // 如果超时导致的返回；那么就返回为true
        bool waitForSeconds(double seconds) {
            struct timespec abstime;
            clock_gettime(CLOCK_REALTIME,&abstime);
            const int64_t kNanoSecondsPerSecond = 1000000000;
            int64_t nanoseconds = static_cast<int64_t>(seconds*kNanoSecondsPerSecond);
            abstime.tv_sec += static_cast<time_t>((abstime.tv_nsec+nanoseconds)/kNanoSecondsPerSecond);
            abstime.tv_nsec = static_cast<long>((abstime.tv_nsec+nanoseconds)%kNanoSecondsPerSecond);
            MutexLockGuard guard(mutex_);
            return ETIMEDOUT == pthread_cond_timedwait(&pcond_, mutex_.getPthreadMutex(), &abstime);
        }

        void notify() noexcept {
            pthread_cond_signal(&pcond_);
        }

        void notifyAll() {
            pthread_cond_broadcast(&pcond_);
        }

    private:
        MutexLock& mutex_;
        pthread_cond_t pcond_;
    };
}
#endif