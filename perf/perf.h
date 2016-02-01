#ifndef PERF_H
#define PERF_H

#include "perf_global.h"

namespace PerfNS {

enum PerfMetric {
    WalltimeMilliseconds,
    CPUMigrations,
    CPUCycles,
    BusCycles,
    StalledCycles,
    Instructions,
    BranchInstructions,
    BranchMisses,
    CacheReferences,
    CacheReads,
    CacheWrites,
    CachePrefetches,
    CacheMisses,
    CacheReadMisses,
    CacheWriteMisses,
    CachePrefetchMisses,
    ContextSwitches,
    PageFaults,
    MinorPageFaults,
    MajorPageFaults,
    AlignmentFaults,
    EmulationFaults,
    Events
};

class PERFSHARED_EXPORT Perf
{

public:
    Perf();
private:
    // Pointer To Implementation
    class PerfImpl;
    PerfImpl* m_impl;
};

}

#endif // PERF_H
