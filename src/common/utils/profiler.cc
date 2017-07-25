#include <iomanip>
#include "common/utils/profiler.h"
#include "common/utils/assert.h"

Profiler::Metric::Metric()
: unit()
, basis()
, val()
{}

Profiler::Metric::Metric(const std::string& _unit, int64_t _basis)
: unit(_unit)
, basis(_basis)
, val()
{}

Profiler::Metric::Metric(const Metric& rhs)
: unit(rhs.unit)
, basis(rhs.basis)
, val(rhs.val.load(std::memory_order_relaxed)) // atomic does not have copy construtor
{}

void Profiler::Metric::Inc(int64_t num)
{
    val.fetch_add(num);
}

void Profiler::Metric::Print(const std::string& metricName, const Metric& m)
{
    std::cout << metricName << "\t\t: ";

    if(!m.unit.empty())
    {
        float f = m.val;
        f /= m.basis;

        std::cout << f << " " << m.unit;
    }
    else
    {
        std::cout << m.val;
    }
        
    std::cout << std::endl;
}

//------------------------------------------------------------------------------
Profiler::Profiler()
{
}

Profiler::~Profiler()
{
    Print();
}

//------------------------------------------------------------------------------
Profiler::Metric& Profiler::GetMetric(const std::string& name, const std::string& unit, int64_t basis)
{
    ASSERT(!name.empty(), "Metric name cannot be empty.");
    
    Metric* m = storage.Find(name);

    if(m == NULL)
    {
        m = storage.Insert(name, Metric(unit, basis));
    }

    return *m;
}

void Profiler::Print()
{
    std::cout << "**********PROFILER BEGIN**********" << std::endl;

    storage.ForEach(Metric::Print);

    std::cout << "**********PROFILER END************" << std::endl;
}
