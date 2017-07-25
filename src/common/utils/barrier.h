//------------------------------------------------------------------------------
// 
//     
//
//------------------------------------------------------------------------------

#ifndef _BARRIER_INCLUDED_
#define _BARRIER_INCLUDED_

#include <atomic>

class Barrier 
{
public:
    Barrier(unsigned numThread);

    void Wait();
    
private:
    unsigned m_numThread;
    std::atomic<unsigned> m_arrived;
};

#endif //_BARRIER_INCLUDED_
