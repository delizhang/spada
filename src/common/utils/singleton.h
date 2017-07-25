#ifndef SINGLETON_H
#define SINGLETON_H

//Base class for singletons
template<typename T>
class Singleton
{
public:
    static T& GetInstance();

    Singleton(Singleton const&) = delete; // delete copy construct
    Singleton(Singleton &&) = delete; // delete move construct
    Singleton& operator=(Singleton const&) = delete; // delete copy assign
    Singleton& operator=(Singleton &&) = delete; // delete move assign

protected:
    Singleton(){};
    virtual ~Singleton(){};
};

#include <common/utils/singleton.hh>

#endif /* end of include guard: SINGLETON_H */
