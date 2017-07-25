//template<typename T>
//inline SharedPtr<T>::SharedPtr()
//: ptr(NULL)
//{}

template<typename T>
inline SharedPtr<T>::SharedPtr(RefCount<T>* _ptr)
: ptr(_ptr)
{}

template<typename T>
inline SharedPtr<T>::SharedPtr(const SharedPtr& lval)
: ptr(lval.ptr)
{
    ptr->ref.fetch_add(1);
}

template<typename T>
inline SharedPtr<T>::SharedPtr(SharedPtr&& rval)
: ptr(rval.ptr)
{
    rval.ptr = NULL;
}

template<typename T>
inline SharedPtr<T>::~SharedPtr()
{
    Reset(); 
}

template<typename T>
inline SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& lval)
{
    if(ptr != lval.ptr)
    {
        Reset();
        ptr = lval.ptr;
        ptr->ref.fetch_add(1);
    }

    return *this;
}

template<typename T>
inline SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& rval)
{
    if(ptr != rval.ptr)
    {
        Reset();
        ptr = rval.ptr;
    }
    return *this;
}

template<typename T>
inline T& SharedPtr<T>::operator*() const
{
    return ptr->obj;
}

template<typename T>
inline T* SharedPtr<T>::operator->() const
{
    return &ptr->obj;
}

template<typename T>
inline void SharedPtr<T>::Reset()
{
    if(ptr != NULL)
    {
        uint32_t ref = ptr->ref.fetch_sub(1);

        if(ref == 1)
        {
            delete ptr;
        }
        ptr = NULL;
    }
}

template<typename T>
template<typename... Args>
inline SharedPtr<T> SharedPtr<T>::Make(Args&&... args)
{
    return SharedPtr(new RefCount<T>(std::forward<Args>(args)...));
}
