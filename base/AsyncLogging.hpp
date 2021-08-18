#ifndef RAINE_ASYNCLOGGING_H
#define RAINE_ASYNCLOGGING_H
#include "../noncopyable.hpp"
#include "LogStream.h"
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include "../MutexGuard.hpp"
#include "../Condition.hpp"
#include "../CountDownLatch.hpp"
namespace raine 
{
class AsyncLogging : noncopyable
{
public:
    AsyncLogging(const std::string& basename,
                 off_t rollSize,
                 int flushInterval = 3);
    ~AsyncLogging();
    // 日志前端；使用此方式添加日志
    void append(const char* logLine, int len);
private:
    // 日志后端；
    void threadFunc();
private:
    typedef raine::detail::FixedBuffer<raine::detail::kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;
private:
    const std::string basename_;
    const off_t rollSize_;
    const int flushInterval_;
    std::atomic<bool> running_;
    raine::MutexLock mutex_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
    raine::CountDownLatch latch_;
    raine::Condition cond_;
};
}
#endif