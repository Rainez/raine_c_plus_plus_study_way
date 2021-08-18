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