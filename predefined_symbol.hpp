// used on a class declaration. don't inherient this class
#ifndef RAINE_PREDEFINED_SYMBOL_H
#define RAINE_PREDEFINED_SYMBOL_H
#define final
#define NotThreadSafe
#include <cstring>
namespace raine {
inline void memZero(void* p, size_t n) {
    memset(p,0,n);
}
}
#endif