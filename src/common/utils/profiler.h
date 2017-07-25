#ifndef PROFILER_H
#define PROFILER_H

#include <cinttypes>
#include <string>
#include <atomic>
#include "config.h"
#include "common/utils/singleton.h"
#include "common/container/mdlist.h"

//A light weight multi-thread safe profiler
class Profiler : public Singleton<Profiler>
{
public:
    struct Metric
    {
        Metric();
        Metric(const std::string& unit, int64_t basis);
        Metric(const Metric& rhs);

        static void Print(const std::string& metricName, const Metric& m);

        void Inc(int64_t num);              //increase metric val by num

        std::string unit;                   //unit, e.g., MB, GB, Second, etc.
        int64_t basis;                      //how to convert raw value to human-readable format
        std::atomic<int64_t> val;           //atomic integer as raw value
    };

public:
    void Print();

    Metric& GetMetric(const std::string& name, const std::string& unit, int64_t basis);

private:
    Profiler();
    ~Profiler();

    friend class Singleton<Profiler>;

private:
    MDList<std::string, Metric> storage;
};

//Enabled profiler
template<int E>
typename std::enable_if<E == 1, void>::type ProfilerIncImpl(const std::string& name, int64_t inc, const std::string& unit, int64_t basis)
{
    Profiler::GetInstance().GetMetric(name, unit, basis).Inc(inc);
}

//Function stub for disabled profiler
template<int E>
typename std::enable_if<E == 0, void>::type ProfilerIncImpl(const std::string& name, int64_t inc, const std::string& unit, int64_t basis)
{}

//Convinice wrapper for profiler metric increment
void ProfilerInc(const std::string& name, int64_t inc, const std::string& unit = "", int64_t basis = 0)
{
    ProfilerIncImpl<ENABLE_PROFILER>(name, inc, unit, basis);
}

#endif /* end of include guard: PROFILER_H */
