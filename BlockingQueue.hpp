#ifndef RAINE_BLOCKINGQUEUE_H
#define RAINE_BLOCKINGQUEUE_H
#include "noncopyable.hpp"
#include "Condition.hpp"
#include "MutexGuard.hpp"
#include <deque>
#include <utility>
namespace raine {
template<typename T>
class BlockingQueue: noncopyable {
public:
    using queue_type = std::deque<T>
    
    BlockingQueue(int capacity = 16): lock(),
                     nonEmptyCond(lock),
                     fullCond(lock),
                     capacity_(capacity),
                     queue_() {}

    void put(T&& obj) {
        MutexLockGuard guard(lock);
        while (queue_.size() >= capacity_) {
            // 当前队列到达了最大的限制；需要在满的条件上等待
            fullCond.wait();
        }
        queue_.push(std::forward(obj));
        nonEmptyCond.notify();
    }


    T poll() {
        /**
         * 需要注意的是poll拿出的对象是新的
         */
        MutexLockGuard guard(lock);
        while (queue_.empty()) {
            // 需要等待非空的条件满足
            nonEmptyCond.wait();
        }
        // 尝试调用移动构造函数;构造obj;最大限度的降低空间的消耗
        // 对于POD类型来说，原始对象和拷贝对象可以当成同一个对象，因为它只是所谓的值对象，拷贝值当然不会有任何问题
        // 对于非POD对象，如这个对象有指针，这个时候就设计到指针所有权的转移，其实这个时候新构造的对象理论上来说还是可以当成原来的那个对象
        // 结论，移动构造晚岁,
        T obj(std::move(queue_.front()));
        queue_.pop_front();
        return obj;
    }

private:
    MutexLock lock;
    Condition nonEmptyCond;
    Condition fullCond;
    size_t capacity_;
    queue_type queue_;

};
}
#endif