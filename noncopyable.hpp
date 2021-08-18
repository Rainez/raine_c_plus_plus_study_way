#ifndef RAINE__NONCOPYABLE__H
#define RAINE__NONCOPYABLE__H
namespace raine {
    class noncopyable {
    public:
        noncopyable(const noncopyable&) = delete;
        noncopyable() = default;
        noncopyable& operator=(const noncopyable&) = delete;
    };
};
#endif