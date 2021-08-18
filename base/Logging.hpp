#ifndef RAINE_BASE_LOGGING_H
#define RAINE_BASE_LOGGING_H
#include <cstring>
#include "LogStream.h"
#include "../Timestamp.h"
namespace raine 
{
class TimeZone;
class Logger
{
public:
    enum LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS
    };

    class SourceFile
    {
    public:
        template<int N>
        SourceFile(const char(&arr)[N]):data_(arr),size_(N-1)
        {
            const char* slash = strrchr(data_, '/');
            if (slash)
            {
                data_ = slash+1;
                size_ -= static_cast(data_-arr);
            }
        }

        explicit SourceFile(const char* filename): data_(filename)
        {
            const char* slash = strrchr(filename,'/');
            if (slash) {
                data_ = slash+1;
            }
            size_ = static_cast<int>(strlen(data_));
        }
        const char* data_;
        int size_;
    };

    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    Logger(SourceFile file, int line, bool toAbort);
    ~Logger();

    LogStream& stream() { return impl_.stream_; }

    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    typedef void (*OutputFunc)(const char* msg, int len);
    typedef void (*FlushFunc)();
    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);
    static void setTimeZone(const TimeZone& tz);

private:
    class Impl 
    {
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
        void formatTime();
        void finish();

        Timestamp time_;
        LogStream stream_;
        LogLevel level_;
        int line_;
        SourceFile basename_;
    };
    Impl impl_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel() 
{
    return g_logLevel;
}


#define LOG_TRACE if (raine::Logger::logLevel() <= Logger::TRACE) \
    Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (raine::Logger::logLevel() <= Logger::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::LogLevel::DEBUG, __func__).stream()
#define LOG_INFO if (raine::Logger::logLevel() <= Logger::INFO) \
    Logger(__FILE__, __LINE__, Logger::LogLevel::INFO, __func__).stream()
#define LOG_WARN if (raine::Logger::logLevel() <= Logger::LogLevel::WARN) \
    Logger(__FILE__, __LINE__, Logger::LogLevel::WARN, __func__).stream()
#define LOG_ERROR if (raine::Logger::logLevel() <= Logger::LogLevel::ERROR) \
    Logger(__FILE__, __LINE__, Logger::LogLevel::ERROR, __func__).stream()
#define LOG_FATAL if (raine::Logger::logLevel() <= Logger::LogLevel::FATAL) \
    Logger(__FILE__, __LINE__, Logger::LogLevel::FATAL, __func__).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, Logger::LogLevel::FATAL, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, Logger::LogLevel::FATAL, true).stream()

const char* strerror_tl(int savedError);
#define CHECK_NOTNULL(val) \ 
    ::raine::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))
    

template<typename T>
T* CheckNotNull(Logger::SourceFile file, int line, const char* names, T* ptr)
{
    if (ptr == nullptr)
    {
        Logger(file, line,Logger::FATAL).stream() << names;
    }
    return ptr;
}
} // namespace raine
#endif