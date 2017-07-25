#ifndef REFCOUNTOBJECT_H
#define REFCOUNTOBJECT_H

template<typename T>
struct RefCount
{
    template<typename... Args>
    RefCount(Args&&... args)
    : ref(1)
    , obj(std::forward<Args>(args)...)
    { }

    std::atomic<uint32_t> ref;
    T obj;
};


#endif /* end of include guard: REFCOUNTOBJECT_H */
