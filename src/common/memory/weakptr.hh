template<typename T>
inline WeakPtr<T>::WeakPtr(const SharedPtr<T>& lval)
: ptr(lval.ptr)
{ }

template<typename T>
inline WeakPtr<T>::WeakPtr(const WeakPtr& lval)
: ptr(lval.ptr)
{ }

template<typename T>
inline WeakPtr<T>::~WeakPtr()
{ }

template<typename T>
inline WeakPtr<T>& WeakPtr<T>::operator=(const SharedPtr<T>& lval)
{
    ptr = lval.ptr;
    return *this;
}

template<typename T>
inline WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& lval)
{
    ptr = lval.ptr;
    return *this;
}

template<typename T>
inline T& WeakPtr<T>::operator*() const
{
    return ptr->obj;
}

template<typename T>
inline T* WeakPtr<T>::operator->() const
{
    return &ptr->obj;
}


