#ifndef SHAREDPTR_H
#define SHAREDPTR_H

#include "libtxd/memory/refcount.h"

template<typename T>
class WeakPtr;

//Thread safe reference counting for holding resources
template<typename T>
class SharedPtr
{
public:
    SharedPtr(const SharedPtr& lval);
    SharedPtr(SharedPtr&& rval);
    ~SharedPtr();

    SharedPtr& operator=(const SharedPtr& lval);
    SharedPtr& operator=(SharedPtr&& rval);

    T& operator*() const;
    T* operator->() const;

    template<typename... Args>
    static SharedPtr Make(Args&&... args);

private:
    //SharedPtr();
    SharedPtr(RefCount<T>* _ptr);

    void Reset();

private:
    RefCount<T>* ptr;

    friend class WeakPtr<T>;
};

#include <libtxd/memory/sharedptr.hh>

#endif /* end of include guard: SHAREDPTR_H */
