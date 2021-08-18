#include "noncopyable.hpp"
#include "MutexGuard.hpp"
#include "Condition.hpp"
#ifndef __RAINE_COUNTDOWNLATCH__
#define __RAINE_COUNTDOWNLATCH__
namespace raine {
    class CountDownLatch: noncopyable {
    public:
        CountDownLatch(int count):
            mutex_(),
            cond_(mutex_),
            count_(count) {
                assert(count_ >= 0);
            }

        void await() {
            MutexLockGuard guard(mutex_);
            while (count_ > 0) {
                cond_.wait();
            }
        }

        void countDown() {
            MutexLockGuard guard(mutex_);
            if (count_ > 0) {
                --count_;
                cond_.notifyAll();
            }
        }
    private:
        mutable MutexLock mutex_;
        Condition cond_;
        int count_;
    };
}
#endif