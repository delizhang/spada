//------------------------------------------------------------------------------
// 
//     
//
//------------------------------------------------------------------------------

#define SET_MARK(_p)    ((Node *)(((uintptr_t)(_p)) | 1))
#define CLR_MARK(_p)    ((Node *)(((uintptr_t)(_p)) & ~1))
#define CLR_MARKD(_p)    ((TxInfo *)(((uintptr_t)(_p)) & ~1))
#define IS_MARKED(_p)     (((uintptr_t)(_p)) & 1)

template<typename KeyType>
TxList<KeyType>::TxList()
    : tail(new Node(0xffffffff, nullptr, nullptr))
    , head(new Node(0, tail, nullptr))
{}

template<typename KeyType>
TxList<KeyType>::~TxList()
{
    Print();
}

template<typename KeyType>
inline bool TxList<KeyType>::Insert(const KeyType& key, TxInfo* info, TxCall& onAbort)
{
    Node* curr = head;
    Node* pred = nullptr;

    TriState ret = RETRY;

    GCProxy gcProxy(gc);

    do
    {
        LocatePred(pred, curr, key);

        if(IsNodeExist(curr, key))
        {
            ret = UpdateNodeInfo(curr, info, false);
        }
        else 
        {
            ret = DoInsert(curr, pred, key, info, gcProxy);
        }
    }
    while(ret == RETRY);

    if(ret == SUCCESS)
    {
        onAbort = std::bind(&MarkNodeInfo, curr, pred, info->desc);
        return true;
    }
    else
    {
        return false;
    }
}

template<typename KeyType>
inline bool TxList<KeyType>::Delete(const KeyType& key, TxInfo* info, TxCall& onCommit)
{
    Node* curr = head;
    Node* pred = nullptr;

    TriState ret = RETRY;

    GCProxy gcProxy(gc);

    do
    {
        LocatePred(pred, curr, key);

        if(IsNodeExist(curr, key))
        {
            ret = UpdateNodeInfo(curr, info, true);
        }
        else 
        {
            ret = FAIL;
        }
    }
    while(ret == RETRY);

    if(ret == SUCCESS)
    {
        onCommit = std::bind(&MarkNodeInfo, curr, pred, info->desc);
        return true;
    }
    else
    {
        return false;
    }
}

template<typename KeyType>
inline bool TxList<KeyType>::Find(const KeyType& key, TxInfo* info)
{
    Node* curr = head;
    Node* pred = nullptr;

    TriState ret = RETRY;

    GCProxy gcProxy(gc);

    do
    {
        LocatePred(pred, curr, key);

        if(IsNodeExist(curr, key))
        {
            ret = UpdateNodeInfo(curr, info, true);
        }
        else 
        {
            ret = FAIL;
        }
    }
    while(ret == RETRY);

    return ret == SUCCESS;
}

template<typename KeyType>
inline TriState TxList<KeyType>::UpdateNodeInfo(Node*& curr, TxInfo* info, bool wantKey)
{
    TxInfo* oldInfo = curr->info;

    if(IS_MARKED(oldInfo))
    {
        // Do_Delete from base lock-free linked list
        if(!IS_MARKED(curr->next))
        {
            std::ignore = __sync_fetch_and_or(&curr->next, 0x1);
        }
        curr = head;
        return RETRY;
    }

    if(oldInfo->desc != info->desc)
    {
        oldInfo->desc->Execute(oldInfo->opid + 1);
    }
    else if(oldInfo->opid >= info->opid)
    {
        return SUCCESS;
    }

    bool hasKey = Base::IsKeyExist(oldInfo, info->desc);

    if((!hasKey && wantKey) || (hasKey && !wantKey))
    {
        return FAIL;
    }
   
    if(info->desc->status != ACTIVE)
    {
        return FAIL;
    }

    if(__sync_bool_compare_and_swap(&curr->info, oldInfo, info))
    {
        return SUCCESS;
    }
    else
    {
        return RETRY;
    }
}

template<typename KeyType>
inline TriState TxList<KeyType>::DoInsert(Node*& curr, Node* pred, const KeyType& key, TxInfo* info, GCProxy& gcProxy)
{
    //we must verify txn status immediately before the CAS update
    if(info->desc->status != ACTIVE)
    {
        return FAIL;
    }

    Node* newNode = gcProxy.Alloc(key, curr, info);

    Node* predNext = __sync_val_compare_and_swap(&pred->next, curr, newNode);

    if(predNext == curr)
    {
        curr = newNode;
        return SUCCESS;
    }
    else
    {
        curr = IS_MARKED(predNext) ? head : pred;
        return RETRY;
    }
}

template<typename KeyType>
inline bool TxList<KeyType>::IsNodeExist(Node* node, const KeyType& key)
{
    return node != nullptr && node->key == key;
}

template<typename KeyType>
inline void TxList<KeyType>::MarkNodeInfo(Node* node, Node* pred, TxDesc* desc)
{
    TxInfo* info = node->info;

    if(info->desc == desc)
    {
        if(__sync_bool_compare_and_swap(&node->info, info, SET_MARK(info)))
        {
            Node* succ = CLR_MARK(__sync_fetch_and_or(&node->next, 0x1));
            __sync_bool_compare_and_swap(&pred->next, node, succ);
            //TODO: free nodes
        }
    }
}

template<typename KeyType>
inline void TxList<KeyType>::LocatePred(Node*& pred, Node*& curr, const KeyType& key)
{
    Node* pred_next;

    while(curr->key < key)
    {
        pred = curr;
        pred_next = CLR_MARK(pred->next);
        curr = pred_next;

        while(IS_MARKED(curr->next))
        {
            curr = CLR_MARK(curr->next);
        }

        if(curr != pred_next)
        {
            //Failed to remove deleted nodes, start over from pred
            if(!__sync_bool_compare_and_swap(&pred->next, pred_next, curr))
            {
                curr = head;
            }
            else
            {
                //TODO:Free nodes
            }
        }
    }
}

template<typename KeyType>
inline void TxList<KeyType>::Print()
{
    Node* curr = head->next;

    while(curr != tail)
    {
        bool hasKey = Base::IsKeyExist(CLR_MARKD(curr->info), nullptr);

        printf("Node [%p] Key [%u] Status [%s]\n", curr, curr->key, hasKey? "Exist":"Inexist");
        curr = CLR_MARK(curr->next);
    }
}
