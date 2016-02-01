#ifndef PERF_H
#define PERF_H

#include "perf_global.h"

// Copied from qtest (definitions not exported)


class PERFSHARED_EXPORT Perf
{

public:
    Perf();

    // Pointer To Implementation
    class PerfImpl;
    PerfImpl* m_impl;
};

#endif // PERF_H
