#ifndef LIST_H
#define LIST_H

template<typename ValueType>
class List
{
private:
    typedef typename std::conditional<std::is_pointer<ValueType>::value, ValueType, ValueType&&>::type InsertType;
    struct Node
    {
        Node(ValueType&& _val);

        Node* next;
        ValueType val;
    };

public:
    List();
    ~List();

    //do not allow copy
    List(List const&) = delete;
    List& operator=(List const&) = delete;

    ValueType& Insert(InsertType val);
    void ForEach(std::function<bool (ValueType&)> func);

private:
    std::atomic<Node*> head;
};

#include <libtxd/container/list.hh>

#endif /* end of include guard: LIST_H */
