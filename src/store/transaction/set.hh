
//template<typename KeyType>
//Set<KeyType>::Insert::Insert(const KeyType& _key, Set& _s)
    //: s(_s)
    //, key(_key)
//{}

//template<typename KeyType>
//inline virtual bool Set<KeyType>::Insert::Execute(TxInfo* info)
//{
    //return s.Insert(key, info);
//}

//template<typename KeyType>
//inline virtual bool Set<KeyType>::Insert::IsKeyExist(const TxDesc* desc, const TxDesc* observer)
//{
    //return desc->status == COMMITTED || (desc->status == ACTIVE && desc == observer);
//}

//template<typename KeyType>
//Set<KeyType>::Delete::Delete(const KeyType& _key, Set& _s)
    //: s(_s)
    //, key(_key)
//{}

//template<typename KeyType>
//inline virtual bool Set<KeyType>::Delete::Execute(TxInfo* info, TxPostOp*& succOp, TxPostOp*& failOp)
//{
    //return s.Delete(key, info, succOp, failOp);
//}

//template<typename KeyType>
//inline virtual bool Set<KeyType>::Delete::IsKeyExist(const TxDesc* desc, const TxDesc* observer)
//{
    //return desc->status == ABORTED;
    //// || (desc->status == ACTIVE && desc != observer)
    //// Key being actively deleted is still visible to observers other than this desc
    //// However, due to helping other observers will NEVER see an active transaction
    //// So we can skip this condition check
//}


//template<typename KeyType>
//Set<KeyType>::Find::Find(const KeyType& _key, Set& _s)
    //: s(_s)
    //, key(_key)
//{}

//template<typename KeyType>
//inline virtual bool Set<KeyType>::Find::Execute(TxInfo* info, TxPostOp*& succOp, TxPostOp*& failOp)
//{
    //return s.Find(key, info, succOp, failOp);
//}

//template<typename KeyType>
//inline virtual bool Set<KeyType>::Find::IsKeyExist(const TxDesc* desc, const TxDesc* observer)
//{
    //return true;
//}
