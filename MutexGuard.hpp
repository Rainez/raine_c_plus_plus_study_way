#include <pthread.h>
#include <sys/types.h>
#include <cassert>
#include "noncopyable.hpp"
#include "predefined_symbol.hpp"
#ifndef __MutexGuard__
#define __MutexGuard__
namespace raine {
    class MutexLockGuard;
    final class MutexLock : noncopyable {
    friend class MutexLockGuard;
    friend class Condition;
    public:
        MutexLock(): holder_(0) {
            assert(pthread_mutex_init(&mutex_, PTHREAD_MUTEX_NORMAL) == 0);
        }

        ~MutexLock() {
            assert(holder_ == 0);
            pthread_mutex_destroy(&mutex_);
        }
    private:
        void lock() {
            pthread_mutex_lock(&mutex_);
        }

        void unlock() {
            holder_ = 0;
            pthread_mutex_unlock(&mutex_);
        }

        pthread_mutex_t* getPthreadMutex() {
            return &mutex_;
        }
    /** even friend class please don't touch data zoo. **/
    private:
        pthread_mutex_t mutex_;
        pid_t holder_;
    };

    class MutexLockGuard : noncopyable {
    public:
        explicit MutexLockGuard(MutexLock& mutex): mutex_(mutex) {
            mutex_.lock();
        }

        ~MutexLockGuard() {
            mutex_.unlock();
        }
    private:
        MutexLock& mutex_; 
    };
}
#define MutexLockGuard(x) static_assert(false, "missing mutex guard var name")
#endif