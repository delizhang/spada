#ifndef MDLIST_H
#define MDLIST_H

#include <cinttypes>
#include <functional>

template<typename KeyType, typename ValueType, uint8_t D = 16, class Hash = std::hash<KeyType>>
class MDList 
{
private:
    struct PredQuery;
    
    struct Node
    {
        Node();
        Node(const KeyType& key, const ValueType& val);
        void Fill(const PredQuery& loc);

        typedef std::array<uint8_t, D> Coord;
        static Coord KeyToCoord(uint32_t key);

        Coord coord;
        KeyType key;
        ValueType val;

        std::atomic<Node*> child[D];
        std::atomic<uint16_t> adopt;        //higher 8 bits contain dp; lower 8 bits for dc
    };

    struct PredQuery
    {
        PredQuery(Node* _curr);

        Node* pred;
        Node* curr;
        uint8_t dp;
        uint8_t dc;
    };

public:
    MDList();
    ~MDList();
    
    ValueType* Insert(const KeyType& key, const ValueType& val);
    ValueType* Find(const KeyType& key);

    void ForEach(std::function<void (const KeyType&, ValueType&)> func);
    void Print() const;

private:
    void LocatePred(const typename Node::Coord& coord, PredQuery& loc);
    void AdoptChildren(Node* n, uint8_t _dp, uint8_t _dc);

    void ForEach(std::function<void (Node&)> func);
    void Traverse(Node* n, Node* parent, int d, std::string& prefix) const;

private:
    Node* head;
};

#include "common/container/mdlist.hh"

#endif /* end of include guard: MDLIST_H */
