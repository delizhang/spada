template<typename T>
inline T& Singleton<T>::GetInstance()
{
    //C++ 11 6.7 [stmt.dcl] p4 ensures that concurrent initialization of static variable will be thread-safe
    //The advatange of this approach is that client doesn't need to explicitly create/destroy singltons
    //The downside is the order of singleton deletion is not defined
    static T instance;
    return instance;
}
