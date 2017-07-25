#ifndef WEAKPTR_H
#define WEAKPTR_H

#include "libtxd/memory/sharedptr.h"

//Thread safe reference counting for holding resources
template<typename T>
class WeakPtr
{
public:
    WeakPtr(const SharedPtr<T>& lval);
    WeakPtr(const WeakPtr& lval);
    ~WeakPtr();

    WeakPtr& operator=(const SharedPtr<T>& lval);
    WeakPtr& operator=(const WeakPtr& lval);

    T& operator*() const;
    T* operator->() const;

private:
    RefCount<T>* ptr;
};

#include <libtxd/memory/weakptr.hh>

#endif /* end of include guard: WEAKPTR_H */
