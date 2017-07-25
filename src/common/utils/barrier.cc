#include "common/utils/barrier.h"

Barrier::Barrier(unsigned numThread)
: m_numThread(numThread)
, m_arrived(0)
{
}

void Barrier::Wait()
{
    m_arrived++;

    while(m_arrived < m_numThread);
}
