#include "common/utils/profiler.h"

template<typename T>
inline EpochGC<T>::State::State(EpochGC<T>& globalState)
: epoch(0)
, entryAfterReclaim(0)
, allocCount(0)
, blockCount(0)
, freeCount{0}
, reclaimCount{0}
, reentry(1)
, allocation(globalState.TakeFilledBlock())
{ 
    for(uint32_t i = 0; i < NUM_EPOCH; i++)
    {
        garbage[i] = globalState.TakeEmptyBlock(1);
    }
}

template<typename T>
inline EpochGC<T>::State::State(State&& _state)
: epoch(_state.epoch)
, entryAfterReclaim(_state.entryAfterReclaim)
, allocCount(_state.allocCount)
, blockCount(_state.blockCount)
, freeCount(_state.freeCount)
, reclaimCount(_state.freeCount)
, reentry(_state.reentry.load(std::memory_order_relaxed))
, allocation(std::move(_state.allocation))
, garbage(std::move(_state.garbage))
{ }

template<typename T>
inline void EpochGC<T>::State::Release(void* _st)
{
    // Clear reentry flag so that this state can be reused by incoming threads
    ((State*)_st)->reentry = 0;
}

template<typename T>
inline EpochGC<T>::EpochGC()
: stateKey(0) 
, stateList()
, epoch(0)
, reclaimFlag(false)
, heapRecord()
, blockReserve(AllocateEmptyBlock())
, allocReserve(AllocateFilledBlock(FILLED_BLOCK_ALLOC))
{
    if(pthread_key_create(&stateKey, State::Release))
    {
        exit(1);
    }
}

template<typename T>
inline EpochGC<T>::~EpochGC()
{
    //Release object memory back to heap
    heapRecord.ForEach([](T*& mem){delete [] mem; return true;});
}

template<typename T>
inline typename EpochGC<T>::State& EpochGC<T>::EnterEpoch()
{
    // Get state associated with this thread
    State* st = (State*)pthread_getspecific(stateKey);

    // Lazy state initialization
    if(st == NULL)
    {
        st = InitState();
    }

    do
    {
        uint32_t reentry = st->reentry++;
        if(reentry == 1)
        {
            uint32_t newEpoch = epoch.load(std::memory_order_acquire);

            if(st->epoch != newEpoch)
            {
                st->epoch = newEpoch;
                st->entryAfterReclaim = 0;
            }
            else if(st->entryAfterReclaim++ == BLOCK_RECLAIM)
            {
                st->reentry--;
                st->entryAfterReclaim = 0;
                Reclaim();
            }
        }
    }
    while(st->reentry == 1);

    return *st;
}

template<typename T>
inline void EpochGC<T>::ExitEpoch(State& st)
{
    st.reentry--;
}

template<typename T>
inline T* EpochGC<T>::Alloc(State& st)
{
    // Current block has been depelted
    if(st.allocCount == BLOCK_CAPACITY)
    {
        // Recycle the batch of used blocks to global list
        if(st.blockCount++ == BLOCK_RECYCLE)
        {
            ProfilerInc("GC::RecycleEmptyBlock", BLOCK_RECYCLE);

            blockReserve.template Merge<MT>(std::move(st.allocation));
            st.blockCount = 0;
        }
        
        // Take one alloc block from global list
        st.allocation.template Merge<ST>(TakeFilledBlock());
        st.allocCount = 0;
    }

    const Block& blk = st.allocation.Head();
    T* obj = blk.entries[st.allocCount++]; 

    return obj;
}

template<typename T>
inline void EpochGC<T>::Free(State& st, T* obj)
{
    uint32_t& freeCount = st.freeCount[st.epoch];
    uint32_t& reclaimCount = st.reclaimCount[st.epoch];
    GarbageList& garbage = st.garbage[st.epoch];

    if(freeCount == BLOCK_CAPACITY)
    {
        garbage.template Merge<ST>(TakeEmptyBlock(1));
        reclaimCount++;
        freeCount = 0;
    }

    Block& blk = garbage.Head();
    blk.entries[freeCount++] = obj;
}


template<typename T>
inline void EpochGC<T>::Reclaim()
{
    ProfilerInc("GC::ReclaimAttemp", 1);

    //only one thread is needed to perform reclaim
    bool flag = reclaimFlag.load(std::memory_order_acquire);
    if(flag || !reclaimFlag.compare_exchange_weak(flag, true, std::memory_order_release, std::memory_order_relaxed))
    {
        return;
    }
    
    uint32_t currentEpoch = epoch.load(std::memory_order_acquire);
    bool isEpochObserved = true;

    stateList.ForEach([&isEpochObserved, currentEpoch](State& st)
    {
        if(st.reentry > 1 && st.epoch != currentEpoch)  
        {
            isEpochObserved = false;
        }
        return isEpochObserved;
    });

    if(!isEpochObserved)
    {
        reclaimFlag.store(false, std::memory_order_release);
        return;
    }

    ProfilerInc("GC::ReclaimSuccess", 1);

    //we can reclaim garbage from two epoch ago
    uint32_t reclaimEpoch = (currentEpoch + NUM_EPOCH - 2) % NUM_EPOCH;

    stateList.ForEach([reclaimEpoch, this](State& st)
    {
        if(st.reclaimCount[reclaimEpoch] > 1)
        {
            ProfilerInc("GC::ReclaimBlock", st.reclaimCount[reclaimEpoch]);

            allocReserve.template Merge<MT>(st.garbage[reclaimEpoch].PopAll());
            st.reclaimCount[reclaimEpoch] = 0;
        }
        return true;
    });
    
    epoch.store((currentEpoch + 1) % NUM_EPOCH, std::memory_order_release);

    reclaimFlag.store(false, std::memory_order_release);
}

template<typename T>
inline typename EpochGC<T>::State* EpochGC<T>::InitState()
{
    State* st = NULL;

    // Find an existing ThreadState from the list that is not being used
    // i.e., the thread creating it has terminated
    stateList.ForEach([&st](State& state)
    {
        //// Set entry count to 1 to signify that the ThreadState is taken
        uint32_t reentry = state.reentry.load(std::memory_order_acquire);
        if( (reentry == 0) && state.reentry.compare_exchange_weak(reentry, 1, std::memory_order_release, std::memory_order_relaxed))
        {
            st = &state;
            return false;
        }
        else
        {
            return true;
        }
    });

    // If no available State is found, create one
    if(st == NULL)
    {
        st = &stateList.Insert(State(*this));
    }

    pthread_setspecific(stateKey, st);
    return st;
}

template<typename T>
inline typename EpochGC<T>::BlockList EpochGC<T>::TakeFilledBlock()
{
    BlockList filledBlock;
     
    do
    {
        filledBlock = allocReserve.template Pop<MT>(1);

        if(filledBlock.Empty())
        {
            allocReserve.template Merge<MT>(AllocateFilledBlock(FILLED_BLOCK_ALLOC));
        }
    }
    while(filledBlock.Empty());

    ProfilerInc("GC::TakeFilledBlock", 1);

    return filledBlock;
}

template<typename T>
inline typename EpochGC<T>::BlockList EpochGC<T>::AllocateFilledBlock(uint32_t numBlock)
{
    T* mem = new T[numBlock * BLOCK_CAPACITY];
    uint32_t filled = 0;

    heapRecord.Insert(mem);

    BlockList blk = TakeEmptyBlock(numBlock);
    blk.ForEach([mem, &filled](Block& blk)
    {
        for (uint32_t i = 0; i < BLOCK_CAPACITY; i++) 
        {
            blk.entries[i] = mem + filled;
            filled++;
        }
    });

    ProfilerInc("GC::HeapAllocCount", 1);
    ProfilerInc("GC::AllocFilledBlock", numBlock);
    ProfilerInc("GC::ObjectHeapMemory", numBlock * BLOCK_CAPACITY * sizeof(T), "MB", 1 << 20);

    return blk;
}

template<typename T>
inline typename EpochGC<T>::BlockList EpochGC<T>::TakeEmptyBlock(uint32_t numBlock)
{
    BlockList emptyBlock;

    do
    {
        emptyBlock = blockReserve.template Pop<MT>(numBlock);
        if(emptyBlock.Empty())
        {
            blockReserve.template Merge<MT>(AllocateEmptyBlock());
        }
    }
    while(emptyBlock.Empty());

    ProfilerInc("GC::TakeEmptyBlock", numBlock);

    return emptyBlock;
}

template<typename T>
inline typename EpochGC<T>::BlockList EpochGC<T>::AllocateEmptyBlock()
{
    BlockList newEmptyBlocks(EMPTY_BLOCK_ALLOC);

    ProfilerInc("GC::AllocEmptyBlock", EMPTY_BLOCK_ALLOC);

    return newEmptyBlocks;
}
