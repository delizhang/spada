template<typename ValueType>
inline List<ValueType>::Node::Node(ValueType&& _val)
: next(NULL)
, val(std::forward<ValueType>(_val))
{
}

template<typename ValueType>
inline List<ValueType>::List()
: head(NULL)
{
}

template<typename ValueType>
inline List<ValueType>::~List()
{
    Node* curr = head;

    while(curr != NULL)
    {
        Node* currNext = curr->next;
        delete curr;
        curr = currNext;
    }
}

template<typename ValueType>
inline ValueType& List<ValueType>::Insert(InsertType val)
{
    Node* newNode = new Node(std::forward<ValueType>(val));
    Node* oldHead = NULL;

    do{
        oldHead = head.load(std::memory_order_acquire);
        newNode->next = oldHead;
    }
    while(!head.compare_exchange_weak(oldHead, newNode, std::memory_order_release, std::memory_order_relaxed));

    return newNode->val;
}

template<typename ValueType>
inline void List<ValueType>::ForEach(std::function<bool (ValueType&)> func)
{
    Node* curr = head;

    while(curr != NULL)
    {
        if(!func(curr->val))
            break;
        curr = curr->next;
    }
}
