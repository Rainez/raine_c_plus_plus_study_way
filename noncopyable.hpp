#ifndef __NONCOPYABLE__
#define __NONCOPYABLE__
namespace raine {
    class noncopyable {
    public:
        noncopyable(const noncopyable&) = delete;
        noncopyable() = default;
        noncopyable& operator=(const noncopyable&) = delete;
    };
};
#endif