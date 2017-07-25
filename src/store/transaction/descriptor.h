#ifndef DESCRIPTOR_H
#define DESCRIPTOR_H

#include <cstdint>
#include <atomic>
#include <vector>
#include <functional>

enum TxStatus
{
    ACTIVE,
    COMMITTED,
    ABORTED
};

struct TxDesc;

struct TxInfo
{
    TxInfo(TxDesc* _desc, uint8_t _opid)
        : desc(_desc)
        , opid(_opid)
    {}

    TxDesc* desc;
    uint8_t opid;
};

typedef std::function<void()> TxCall;

struct TxOp
{    
    virtual bool Execute(TxInfo* info) = 0;

    TxCall onCommit;
    TxCall onAbort;
};

struct TxDesc
{
    TxDesc(std::initializer_list<TxOp*> _ops)
        : status(ACTIVE)
        , ops(_ops)
    {}

    bool Execute();
    bool Execute(uint8_t opid);

    std::atomic<TxStatus> status;
    std::vector<TxOp*> ops;
};

#endif /* end of include guard: DESCRIPTOR_H */
