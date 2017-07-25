#ifndef EPOCHGC_H
#define EPOCHGC_H

#include <pthread.h>
#include <cinttypes>
#include <atomic>
#include <iostream>
#include "config.h"
#include "libtxd/container/circularlist.h"
#include "libtxd/container/list.h"

template<typename T>
class EpochGC
{
private:
    static const uint32_t NUM_EPOCH = 3;
    static const uint32_t BLOCK_CAPACITY = 100;     //Number of entries in one block
    static const uint32_t BLOCK_RECYCLE = 100;      //Number of blocks to deplete before sending them to global reserve
    static const uint32_t BLOCK_RECLAIM = 100;      //Number of blocks to fill before invoking relaim
    static const uint32_t EMPTY_BLOCK_ALLOC = 1000; //Number of emoty blocks to allocate from heap when we run out
    static const uint32_t FILLED_BLOCK_ALLOC = 10;  //Number of emoty blocks to allocate from heap when we run out

    // A block of object pointers
    struct Block
    {
        T* entries[BLOCK_CAPACITY];
    };

    typedef CircularList<Block> BlockList;
    typedef CircularListWithTail<Block> GarbageList;
    typedef std::array<uint32_t, NUM_EPOCH> IntArray;
    typedef std::array<CircularListWithTail<Block>, NUM_EPOCH> BlockListArray;
    typedef std::array<GarbageList, NUM_EPOCH> GarbageListArray;

    // Per-thread status 
    struct alignas(CACHE_LINE_SIZE) State
    {
        State(EpochGC<T>& globalState);
        static void Release(void* st);

        State(State &&);
        State& operator=(State &&);

        State(State const&) = delete;
        State& operator=(State const&) = delete;

        uint32_t epoch;                     //Epoch observed by thread
        uint32_t entryAfterReclaim;         //Number of entries after last reclaim attempt
        uint32_t allocCount;                //Number of allocated objects from current block
        uint32_t blockCount;                //Number of used blocks
        IntArray freeCount;                 //Number of free objects filled in current garbage block
        IntArray reclaimCount;              //Number of filled garbage blocks
        std::atomic<uint32_t> reentry;      //Number of times a thread is entering critical regions

        BlockList allocation;               //List of blocks with availabe objects
        GarbageListArray garbage;           //List of garbage blocks
    };

    typedef List<State> StateList;
    typedef List<T*> MemList;

public:
    typedef State& StateRef;

public:
    EpochGC();
    ~EpochGC();

    State& EnterEpoch();
    void ExitEpoch(State& st);

    T* Alloc(State& st);
    void Free(State& st, T* obj);

private:
    void Reclaim();
    State* InitState();                                 //Initialize per-thread state
    
    BlockList TakeFilledBlock();                        //Take one alloc block from global allocReserve 
    BlockList AllocateFilledBlock(uint32_t num);        //Allocate a numBlock of filled blocks 

    BlockList TakeEmptyBlock(uint32_t num);
    BlockList AllocateEmptyBlock();

private:
    pthread_key_t stateKey;             //Pthread key for dynamic thread_local state 
    StateList stateList;                //per-thread state list

    std::atomic<uint32_t> epoch;        //Current global epoch
    std::atomic<bool> reclaimFlag;      //Exlusive access to recliam procedure

    MemList heapRecord;                 //Allocated chunks from heap; must be initialized before blockList; allocation need to use it
    BlockList blockReserve;             //List of delepted or empty blocks
    BlockList allocReserve;             //List of blocks with available objects
};

#include "libtxd/memory/epochgc.hh"

#endif /* end of include guard: EPOCHGC_H */
