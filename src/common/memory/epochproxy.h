#ifndef EPOCHPROXY_H
#define EPOCHPROXY_H

#include "epochgc.h"

template<typename T>
class EpochProxy
{
private:
    typedef EpochGC<T> GCType;
    typedef typename GCType::StateRef GCStateRef;

public:
    EpochProxy(GCType& _gc);
    ~EpochProxy();

    template<typename... Args>
    T* Alloc(Args&&... args);

    void Free(T* obj);

private:
   GCType& gc; 
   GCStateRef st;
};

#include "epochproxy.hh"

#endif /* end of include guard: EPOCHPROXY_H */
