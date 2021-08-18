#include "Exception.h"
#include "CurrentThread.h"
namespace raine {
    Exception::Exception(std::string msg): message_(std::move(msg)), stack_(CurrentThread::stackTrace(false)) {}
}