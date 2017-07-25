template<typename T>
inline EpochProxy<T>::EpochProxy(GCType& _gc)
: gc(_gc)
, st(gc.EnterEpoch())
{}

template<typename T>
inline EpochProxy<T>::~EpochProxy()
{
    gc.ExitEpoch(st);
}

template<typename T>
template<typename... Args>
inline T* EpochProxy<T>::Alloc(Args&&... args)
{
    T* obj = gc.Alloc(st);
    new(obj) T(std::forward<Args>(args)...);
    return obj;
}

template<typename T>
inline void EpochProxy<T>::Free(T* obj)
{
    return gc.Free(st, obj);
}
