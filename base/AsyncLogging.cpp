#include "AsyncLogging.hpp"
#include "Logging.hpp"
namespace raine {
void AsyncLogging::append(const char* logline, int len)
{
    raine::MutexLockGuard guard(mutex_);
    if (currentBuffer_->avail() > len) 
    {
        currentBuffer_->append(logline,len);
    }    
    else 
    {
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_)
        {
            // 获得所有权
            currentBuffer_ = std::move(nextBuffer_);
        }
        else 
        {
            currentBuffer_.reset(new Buffer);
        }
        currentBuffer_->append(logline,len);
        cond_.notify();
    }
}

void AsyncLogging::threadFunc()
{
    assert(running_ == true);
    latch_.countDown();
    

}

}
