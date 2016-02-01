#ifndef PERFEVENT_H
#define PERFEVENT_H

#include <linux/perf_event.h>

class PerfEvent
{
public:
    PerfEvent();

private:
    int m_fd;
    struct perf_event_attr m_attr;
};

#endif // PERFEVENT_H
