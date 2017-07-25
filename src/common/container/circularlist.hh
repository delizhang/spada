template<typename T>
inline CircularList<T>::CircularList()
: head(NULL)
{}

template<typename T>
inline CircularList<T>::CircularList(Node* _head)
: head(_head)
{}

template<typename T>
inline CircularList<T>::CircularList(uint32_t sz)
: head(new Node)
{
    ProfilerInc("GC::ListHeapMemory", sizeof(Node) * sz, "MB", 1 << 20);

    Node* curr = head;

    // Set next pointer for sz - 1 nodes
    // head already counted as the first node
    for(uint32_t i = 1; i < sz; i++)
    {
        curr->next = new Node;
        curr = curr->next;
    }

    //close the loop
    curr->next = head;
}

template<typename T>
inline CircularList<T>::~CircularList()
{
    Destory();
}

template<typename T>
inline CircularList<T>::CircularList(CircularList<T>&& moveFrom)
: head(moveFrom.head)
{
    moveFrom.head = NULL;
}

template<typename T>
inline CircularList<T>& CircularList<T>::operator=(CircularList<T>&& moveFrom)
{
    Destory();
    head = moveFrom.head;
    moveFrom.head = NULL;

    return *this;
}

template<typename T>
inline void CircularList<T>::Destory()
{
    if(head == NULL)
        return;

    Node* curr = head;
    do
    {
        Node* currNext = curr->next;
        delete curr;
        curr = currNext;
    }
    while(curr != head);
}

template<typename T>
inline CircularList<T> CircularList<T>::Create(Node* _head)
{
    return CircularList<T>(_head);
}


//-----------------------------------------------------------------------------
template<typename T>
inline T& CircularList<T>::Head() const
{
    return head->val;
}

template<typename T>
inline bool CircularList<T>::Empty() const
{
    return head == NULL;
}

template<typename T>
inline void CircularList<T>::ForEach(std::function<void (T&)> func)
{
    Node* curr = head;
    do
    {
        func(curr->val);
        curr = curr->next;
    }
    while(curr != head);
}

template<typename T>
template<Safety S>
inline typename std::enable_if<S == MT, CircularList<T>>::type 
CircularList<T>::Pop(uint32_t num)
{
    Node* headNext = NULL;
    Node* curr = NULL;

    do
    {
        headNext = head->next.load(std::memory_order_acquire);
        curr = head;

        // Move the curr pointer num nodes away from heaed
        for(uint32_t i = 0; i < num; i++)
        {
            curr = curr->next.load(std::memory_order_acquire);

            // Not enough nodes in the list, return NULL
            if(curr == head)
            {
                return CircularList();
            }
        }
    }
    while(!head->next.compare_exchange_weak(headNext, curr->next, std::memory_order_release, std::memory_order_relaxed));

    curr->next = headNext;

    return CircularList(headNext);
}

template<typename T>
template<Safety S>
inline typename std::enable_if<S == ST, void>::type 
CircularList<T>::Merge(CircularList<T>&& target)
{
    if(head == NULL)
    {
        head = target.head;
    }
    else
    {
        Node* headNext = head->next;
        head->next = target.head->next.load(std::memory_order_relaxed);
        target.head->next = headNext; 
        head = head->next; //head now points to the node from the newly merged list
    }
    target.head = NULL; //enforce move semantic
}

template<typename T>
template<Safety S>
inline typename std::enable_if<S == MT, void>::type 
CircularList<T>::Merge(CircularList<T>&& target)
{
    Node* headNext = NULL;
    Node* targetHeadNext = target.head->next.load(std::memory_order_relaxed);

    do
    {
        headNext = head->next.load(std::memory_order_acquire);
        target.head->next = headNext;
    }
    while(!head->next.compare_exchange_weak(headNext, targetHeadNext, std::memory_order_release, std::memory_order_relaxed));

    target.head = NULL;
}


//-----------------------------------------------------------------------------
template<typename T>
inline CircularListWithTail<T>::CircularListWithTail()
: Base()
, tail(NULL)
{}

template<typename T>
inline CircularListWithTail<T>::CircularListWithTail(Base&& moveFrom)
: Base(std::forward<Base>(moveFrom))
{
    Node* curr = head;
    while(curr != NULL && curr->next != head)
    {
        curr = curr->next;
    }
    tail = curr;
}

template<typename T>
inline CircularList<T> CircularListWithTail<T>::PopAll()
{
    Node* headNext = NULL; 
    if(head->next != head)
    {
        headNext = head->next;
        tail->next = headNext;
        head->next = head;
    }

    return Base::Create(headNext);
}

template<typename T>
template<Safety S>
inline typename std::enable_if<S == ST, void>::type 
CircularListWithTail<T>::Merge(Base&& target)
{
    Node* oldHead = head;
    Base::template Merge<S>(std::forward<Base>(target));
    tail = oldHead;
}
