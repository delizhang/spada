//------------------------------------------------------------------------------
// 
//     
//
//------------------------------------------------------------------------------

#include <cstring>
#include <stack>
#include <iostream>

template<typename T, uintptr_t F = 0x1>
inline T* SetFlag(T* ptr)
{
    return ((T*)(((uintptr_t)(ptr)) | F));
}

template<typename T, uintptr_t F = 0x1>
inline T* ClearFlag(T* ptr)
{
    return ((T*)(((uintptr_t)(ptr)) & ~F));
}

template<typename T, uintptr_t F = 0x1>
inline bool HasFlag(T* ptr)
{
    return ((uintptr_t)(ptr)) & F;
}

//std doesn't have fetch_or for pointer atomic
template<typename T, uintptr_t F = 0x1>
inline T* FetchOrFlag(std::atomic<T*>& atomic_ptr)
{
    T* ptr;
    do
    {
        ptr = atomic_ptr.load(std::memory_order_acquire);
    }
    while(!HasFlag<T, F>(ptr) && atomic_ptr.compare_exchange_weak(ptr, SetFlag<T, F>(ptr), std::memory_order_release, std::memory_order_relaxed));

    return ClearFlag<T, F>(ptr);
}

//------------------------------------------------------------------------------
template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline MDList<KeyType, ValueType, D, Hash>::PredQuery::PredQuery(Node* _curr)
: pred(NULL)
, curr(_curr)
, dp(0)
, dc(0)
{}

template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline MDList<KeyType, ValueType, D, Hash>::Node::Node()
: coord()
, key()
, val()
, child()
, adopt(0)
{}


template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline MDList<KeyType, ValueType, D, Hash>::Node::Node(const KeyType& _key, const ValueType& _val)
: coord(KeyToCoord(Hash()(_key)))
, key(_key)
, val(_val)
, child()
, adopt(0)
{}

template<typename KeyType, typename ValueType, uint8_t D, class Hash>
void MDList<KeyType, ValueType, D, Hash>::Node::Fill(const PredQuery& loc)
{
    //shift dp to higher 8 bits of a 16 bits integer, and mask with dc
    adopt = loc.dp != loc.dc ? ((uint16_t)loc.dp) << 8 | loc.dc : 0;

    //Fill values for newNode, child is set to 1 for all children before dp
    //dp is the dimension where newNode is inserted, all dimension before that are invalid for newNode
    for(uint32_t i = 0; i < loc.dp; ++i)
    {
        child[i] = (Node*)0x1;
    }

    //be careful with the length of memset, should be D - dp NOT (D - 1 - dp)
    memset(child + loc.dp, 0, sizeof(Node*) * (D - loc.dp));

    if(loc.dc < D)
    {
        child[loc.dc] = loc.curr;
    }
}


//------------------------------------------------------------------------------
template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline MDList<KeyType, ValueType, D, Hash>::MDList()
: head(new Node)
{
    static_assert(D <= 32, "Dimension must not be greater than 32");
}

template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline MDList<KeyType, ValueType, D, Hash>::~MDList()
{
    ForEach([](Node& n){delete &n;});
}


//------------------------------------------------------------------------------
template<typename KeyType, typename ValueType, uint8_t D, class Hash>
ValueType* MDList<KeyType, ValueType, D, Hash>::Insert(const KeyType& key, const ValueType& val)
{
    Node* newNode = new Node(key, val);
    PredQuery loc(head);

    while(true)
    {
        LocatePred(newNode->coord, loc);

        if(loc.dc == D) //node exists
        {
            delete newNode;
            return &loc.curr->val; 
        }

        if(loc.curr) 
        {
            AdoptChildren(loc.curr, loc.dp, loc.dc);
        }

        newNode->Fill(loc);
        if(loc.pred->child[loc.dp].compare_exchange_weak(loc.curr, newNode, std::memory_order_release, std::memory_order_relaxed))
        {
            AdoptChildren(newNode, 0, D);
            return &newNode->val;
        }

        //If the code reaches here it means the CAS failed for two reasons:
        //1. the child slot has been marked as invalid by parents
        //2. another thread inserted a child into the slot
        Node* predChild = loc.pred->child[loc.dp].load(std::memory_order_acquire);
        if(HasFlag(predChild))
        {
            loc.pred = NULL;
            loc.curr = head;
            loc.dp = 0;
            loc.dc = 0;
        }
        else
        {
            loc.curr = loc.pred;
            loc.dc = loc.dp;
        }
    }
}

template<typename KeyType, typename ValueType, uint8_t D, class Hash>
ValueType* MDList<KeyType, ValueType, D, Hash>::Find(const KeyType& key)
{
    PredQuery loc(head);

    LocatePred(Node::KeyToCoord(Hash()(key)), loc);

    return loc.dc == D ? &loc.curr->val : NULL;
}


template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline void MDList<KeyType, ValueType, D, Hash>::ForEach(std::function<void (const KeyType&, ValueType&)> func)
{
    ForEach([func, this](Node& n)
    {
        if(&n != head)
            func(n.key, n.val);
    });
}

template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline void MDList<KeyType, ValueType, D, Hash>::Print() const
{
    std::string prefix;
    Traverse(head, NULL, 0, prefix);
}


//------------------------------------------------------------------------------
template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline void MDList<KeyType, ValueType, D, Hash>::LocatePred(const typename Node::Coord& coord, PredQuery& loc)
{
    while(loc.dc < D)
    {
        while(loc.curr && coord[loc.dc] > loc.curr->coord[loc.dc])
        {
            loc.dp = loc.dc;
            loc.pred = loc.curr;
            AdoptChildren(loc.curr, loc.dc, loc.dc + 1);
            loc.curr = ClearFlag(loc.curr->child[loc.dc].load(std::memory_order_relaxed));
        }

        if(loc.curr == NULL || coord[loc.dc] < loc.curr->coord[loc.dc]) 
        {
            break;
        }
        else
        {
            ++loc.dc;
        }
    }
}

template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline void MDList<KeyType, ValueType, D, Hash>::AdoptChildren(Node* n, uint8_t _dp, uint8_t _dc)
{
    uint16_t adopt = n->adopt.load(std::memory_order_relaxed);
    if(adopt == 0) return;

    uint8_t dp = adopt >> 8;
    uint8_t dc = adopt & 0x00ff;

    //Read curr node, then read adopt again
    //if adoption is still needed when adopt is read, 
    //then curr must be the correct node to adopt from
    Node* curr = n->child[dc].load(std::memory_order_relaxed);
    adopt = n->adopt.load(std::memory_order_acquire);

    //No need to adopt if [dp, dc) and [_dp, _dc) do not overlap
    if(adopt == 0 || _dc <= dp || _dp >= dc) return;

    for (uint8_t i = dp; i < dc; ++i) 
    {
        //TODO: make sure curr does not need to adopt children
        Node* child = curr->child[i];

        //Children slot of curr_node need to be marked as invalid before we copy them to newNode
        child = FetchOrFlag(curr->child[i]);
        if(child)
        {
            static Node* nil = NULL; //std::cas does not like immediate value as expected
            n->child[i].compare_exchange_weak(nil, child, std::memory_order_relaxed, std::memory_order_relaxed);
        }
    }

    //Clear the adopt task
    n->adopt.compare_exchange_weak(adopt, 0, std::memory_order_release, std::memory_order_relaxed);
}


//------------------------------------------------------------------------------
template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline typename MDList<KeyType, ValueType, D, Hash>::Node::Coord 
MDList<KeyType, ValueType, D, Hash>::Node::KeyToCoord(uint32_t key)
{
    const static uint32_t basis[32] = {0xffffffff, 0x10000, 0x800, 0x100, 0x80, 0x40, 0x20, 0x10, 
                                       0xC, 0xA, 0x8, 0x7, 0x6, 0x5, 0x5, 0x4,
                                       0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                                       0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x2};

    Coord coord;
    uint32_t quotient = key;

    for (int i = D - 1; i >= 0 ; --i)
    {
        coord[i] = quotient % basis[D - 1];
        quotient /= basis[D - 1];
    }

    return coord;
}


//------------------------------------------------------------------------------
template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline void MDList<KeyType, ValueType, D, Hash>::ForEach(std::function<void (Node&)> func)
{
    struct Frame
    {
        Node* node;
        uint8_t dc;
    };

    std::stack<Frame> nodeStack;
    nodeStack.push({head, 0});

    while(!nodeStack.empty())
    {
        Frame f = nodeStack.top();
        nodeStack.pop();

        for (uint8_t i = f.dc; i < D; i++) 
        {
            Node* child = f.node->child[i].load(std::memory_order_relaxed);

            if(child != NULL)
            {
                nodeStack.push({child, i});
            }
        }

        func(*f.node);
    }
}

template<typename KeyType, typename ValueType, uint8_t D, class Hash>
inline void MDList<KeyType, ValueType, D, Hash>::Traverse(Node* n, Node* parent, int d, std::string& prefix) const
{
    std::cout << prefix << "Node " << n << " [" << n->key << " : " << n->val << "] DIM " << d << " of Parent " << parent << std::endl;

    n = ClearFlag(n);    

    //traverse from last dimension up to current dim
    //The valid children include child nodes up to dim
    //e.g. a node on dimension 3 has only valid children on dimensions 3~8
    for (int i = D - 1; i >= d; --i) 
    {
        Node* child = n->child[i];

        if(child != NULL)
        {
            prefix.push_back('|');
            prefix.insert(prefix.size(), i, ' ');

            Traverse(child, n, i, prefix);

            prefix.erase(prefix.size() - i - 1, i + 1);
        }
    }
}
