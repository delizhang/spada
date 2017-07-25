#ifndef SET_H
#define SET_H

#include "store/transaction/descriptor.h"

template<typename KeyType>
class Set
{
public:

    struct KeyPredicate
    {
        virtual bool IsKeyExist(const TxDesc* desc, const TxDesc* observer) = 0;
    };

    struct InsertOp : public TxOp, public KeyPredicate
    {
        InsertOp(const KeyType& _key, Set& _s)
            : s(_s)
              , key(_key)
        {}

        virtual bool Execute(TxInfo* info)
        {
            return s.Insert(key, info, onAbort);
        }

        virtual bool IsKeyExist(const TxDesc* desc, const TxDesc* observer)
        {
        	// PL: If last operation performed was insert, and it committed, then the key exists in the abstract state (and concrete)
        	// If the insert operation is still pending, and the observer is in the same transaction, then the key exists in the
        	// abstract state (not concrete?)
            return (desc->status == COMMITTED) || (desc->status == ACTIVE && desc == observer);
        }

        Set& s;
        KeyType key;
    };

    struct DeleteOp : public TxOp, public KeyPredicate
    {
        DeleteOp(const KeyType& _key, Set& _s)
            : s(_s)
            , key(_key)
        {}

        virtual bool Execute(TxInfo* info)
        {
            return s.Delete(key, info, onCommit);
        }

        virtual bool IsKeyExist(const TxDesc* desc, const TxDesc* observer)
        {
            return desc->status == ABORTED;
            // || (desc->status == ACTIVE && desc != observer)
            // Key being actively deleted is still visible to observers other than this desc
            // However, due to helping other observers will NEVER see an active transaction
            // So we can skip this condition check
        }

        Set& s;
        KeyType key;
    };

    struct FindOp : public TxOp, public KeyPredicate
    {
        FindOp(const KeyType& _key, Set& _s)
            : s(_s)
              , key(_key)
        {}

        virtual bool Execute(TxInfo* info)
        {
            return s.Find(key, info);
        }

        virtual bool IsKeyExist(const TxDesc* desc, const TxDesc* observer)
        {
            return true;
        }

        Set& s;
        KeyType key;
    };

protected:
    virtual bool Insert(const KeyType& key, TxInfo* info, TxCall& onAbort) = 0;

    virtual bool Delete(const KeyType& key, TxInfo* info, TxCall& onCommit) = 0;

    virtual bool Find(const KeyType& key, TxInfo* info) = 0;

    bool IsKeyExist(TxInfo* info, TxDesc* observer)
    {
        KeyPredicate* keyPred = dynamic_cast<KeyPredicate*>(info->desc->ops[info->opid]);
        return keyPred->IsKeyExist(info->desc, observer);
    }
};

#include "store/transaction/set.hh"

#endif /* end of include guard: SET_H */
