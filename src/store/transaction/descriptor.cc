#include <algorithm>
#include "store/transaction/descriptor.h"
#include "common/utils/assert.h"

bool TxDesc::Execute()
{
    //start from the first operation
    return Execute(0);
}

bool TxDesc::Execute(uint8_t opid)
{
    bool ret = true;

    while(status == ACTIVE && ret == true && opid < ops.size())
    {
        TxInfo* info = new TxInfo(this, opid);

        ret = ops[opid++]->Execute(info);
    }

    //compare_exchange requires a memery reference to the expected value
    TxStatus expected = status;

    if(ret == true)
    {
        if(status.compare_exchange_strong(expected, COMMITTED, std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            //PL: Check for callable target stored in op->onCommit, then call
            std::for_each(ops.begin(), ops.end(), [](TxOp* op){if(op->onCommit) op->onCommit();});
        }
    }
    else
    {
        if(status.compare_exchange_strong(expected, ABORTED, std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            std::for_each(ops.begin(), ops.end(), [](TxOp* op){if(op->onAbort) op->onAbort();});
        }     
    }
   
    return status == COMMITTED;
}
