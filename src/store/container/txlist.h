#ifndef TRANSLIST_H
#define TRANSLIST_H

#include <cstdint>
#include "store/transaction/set.h"
#include "common/memory/epochproxy.h"
#include "common/utils/common.h"

template<typename KeyType>
class TxList : public Set<KeyType>
{
public:
    struct Node
    {
        Node() : key(0), next(nullptr), info(nullptr) {}
        
        Node(KeyType _key, Node* _next, TxInfo* _info) 
            : key(_key), next(_next), info(_info) {}

        KeyType key;
        Node* next;
        TxInfo* info;
    };

    typedef Set<KeyType> Base;
    typedef EpochGC<Node> GC;
    typedef EpochProxy<Node> GCProxy;
    
    TxList();
    ~TxList();

private:
    virtual bool Insert(const KeyType& key, TxInfo* info, TxCall& onAbort);
    virtual bool Delete(const KeyType& key, TxInfo* info, TxCall& onCommit);
    virtual bool Find(const KeyType& key, TxInfo* info);

    bool IsNodeExist(Node* node, const KeyType& key);

    TriState UpdateNodeInfo(Node*& curr, TxInfo* info, bool wantKey);
    TriState DoInsert(Node*& curr, Node* pred, const KeyType& key, TxInfo* info, GCProxy& gcProxy);

    void LocatePred(Node*& pred, Node*& curr, const KeyType& key);

    static void MarkNodeInfo(Node* node, Node* pred, TxDesc* desc);

    void Print();

private:
    Node* tail;
    Node* head;
    GC gc;
};

#include "store/container/txlist.hh"

#endif /* end of include guard: TRANSLIST_H */    
