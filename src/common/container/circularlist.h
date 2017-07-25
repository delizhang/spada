#ifndef CIRCULARLIST_H
#define CIRCULARLIST_H

#include <type_traits>

enum Safety
{
    MT, // Mutli thread safe
    ST  // Single thread safe
};

template<typename T>
class CircularList
{
protected:
    struct Node
    {
        std::atomic<Node*> next;
        T val;
    };

public:
    CircularList();
    CircularList(uint32_t sz);
    ~CircularList();

    //allow move
    CircularList(CircularList &&); 
    CircularList& operator=(CircularList &&);

    //do not allow copy
    CircularList(CircularList const&) = delete;
    CircularList& operator=(CircularList const&) = delete;


    T& Head() const;
    bool Empty() const;
    void ForEach(std::function<void (T&)> func);

    template<Safety S>
    typename std::enable_if<S == MT, CircularList<T>>::type 
    Pop(uint32_t num);

    //Merge accept rvalue reference, so that the merge target will be disopoed afterwards 
    template<Safety S>
    typename std::enable_if<S == MT, void>::type 
    Merge(CircularList<T>&& target);

    template<Safety S>
    typename std::enable_if<S == ST, void>::type 
    Merge(CircularList<T>&& target);

protected:
    // Internally used
    CircularList(Node* _head);
    CircularList<T> Create(Node* _head);

    void Destory();

protected:
    Node* head;
};


template<typename T>
class CircularListWithTail : public CircularList<T>
{
private:
    typedef CircularList<T> Base;
    typedef typename Base::Node Node;
    using Base::head;
    
public: 
    CircularListWithTail();
    CircularListWithTail(Base &&); 
    
    Base PopAll();

    template<Safety S>
    typename std::enable_if<S == ST, void>::type 
    Merge(Base&& target);

private:
    Node* tail;
};

#include "libtxd/container/circularlist.hh"

#endif /* end of include guard: CIRCULARLIST_H */
