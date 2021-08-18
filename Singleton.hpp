#ifndef __RAINE__SINGLETON__
#define __RAINE__SINGLETON__
#include <pthread.h>
namespace raine {
template<typename T>
class Singleton {
public:
    static T& instance() {
        pthread_once(&ponce_,&Singleton::init);
    }    
private:
    Singleton();
    ~Singleton();
    static void init() {
        value_ = new T();
    }
private:
    static pthread_once_t ponce_;
    static T* value_;
};
template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::value_ = nullptr;
}
#endif