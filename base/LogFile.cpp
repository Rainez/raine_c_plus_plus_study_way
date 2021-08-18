#include <cassert>
#include <cstdio>
#include <ctime>
#include <string>
#include "LogFile.hpp"
using namespace raine;
using string = std::string;

LogFile::LogFile(const std::string& basename,
            off_t rollSize,
            bool threadSafe,
            int flushInterval,
            int checkEveryN)
            : basename_(basename),
              rollSize_(rollSize),
              flushInterval_(flushInterval),
              checkEveryN_(checkEveryN),
              count_(0),
              mutex_(threadSafe ? new MutexLock : nullptr),
              startOfPeriod_(0),
              lastRoll_(0),
              lastFlush_(0)
{
    assert(basename.find('/') == string::npos);
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* logline, int len)
{
    if (mutex_)
    {
        MutexLockGuard lock(*mutex_);
        append_unlocked(logline, len);
    }
    else 
    {
        append_unlocked(logline, len);
    }
}

void LogFile::flush()
{
    if (mutex_)
    {
        MutexLockGuard lock(*mutex_);
        file_->flush();
    }
    else 
    {
        file_->flush();
    }
}

void LogFile::append_unlocked(const char* logline, int len)
{
    file_->append(logline,len);
    if (file_->writtenBytes() > rollSize_)
    {
        rollFile();
    }
    else 
    {
        ++count_;
        if (count_ >)
    }
}