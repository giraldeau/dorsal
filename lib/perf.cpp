#include <cstring>
#include <QtGlobal>
#include <QDebug>
#include <QSet>

#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>

#include "perf.h"

// Source: qbenchmarkperfevents.cpp
//
// for PERF_TYPE_HW_CACHE, the config is a bitmask
// lowest 8 bits: cache type
// bits 8 to 15: cache operation
// bits 16 to 23: cache result
#define CACHE_L1D_READ              (PERF_COUNT_HW_CACHE_L1D | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
#define CACHE_L1D_WRITE             (PERF_COUNT_HW_CACHE_L1D | PERF_COUNT_HW_CACHE_OP_WRITE << 8 | PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
#define CACHE_L1D_PREFETCH          (PERF_COUNT_HW_CACHE_L1D | PERF_COUNT_HW_CACHE_OP_PREFETCH << 8 | PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
#define CACHE_L1I_READ              (PERF_COUNT_HW_CACHE_L1I | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
#define CACHE_L1I_PREFETCH          (PERF_COUNT_HW_CACHE_L1I | PERF_COUNT_HW_CACHE_OP_PREFETCH << 8 | PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
#define CACHE_LLC_READ              (PERF_COUNT_HW_CACHE_LL  | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
#define CACHE_LLC_WRITE             (PERF_COUNT_HW_CACHE_LL  | PERF_COUNT_HW_CACHE_OP_WRITE << 8| PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
#define CACHE_LLC_PREFETCH          (PERF_COUNT_HW_CACHE_LL  | PERF_COUNT_HW_CACHE_OP_PREFETCH << 8 | PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
#define CACHE_L1D_READ_MISS         (PERF_COUNT_HW_CACHE_L1D | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16)
#define CACHE_L1D_WRITE_MISS        (PERF_COUNT_HW_CACHE_L1D | PERF_COUNT_HW_CACHE_OP_WRITE << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16)
#define CACHE_L1D_PREFETCH_MISS     (PERF_COUNT_HW_CACHE_L1D | PERF_COUNT_HW_CACHE_OP_PREFETCH << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16)
#define CACHE_L1I_READ_MISS         (PERF_COUNT_HW_CACHE_L1I | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16)
#define CACHE_L1I_PREFETCH_MISS     (PERF_COUNT_HW_CACHE_L1I | PERF_COUNT_HW_CACHE_OP_PREFETCH << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16)
#define CACHE_LLC_READ_MISS         (PERF_COUNT_HW_CACHE_LL  | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16)
#define CACHE_LLC_WRITE_MISS        (PERF_COUNT_HW_CACHE_LL  | PERF_COUNT_HW_CACHE_OP_WRITE << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16)
#define CACHE_LLC_PREFETCH_MISS     (PERF_COUNT_HW_CACHE_LL  | PERF_COUNT_HW_CACHE_OP_PREFETCH << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16)
#define CACHE_BRANCH_READ           (PERF_COUNT_HW_CACHE_BPU | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
#define CACHE_BRANCH_READ_MISS      (PERF_COUNT_HW_CACHE_BPU | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16)

struct EventDef {
    quint32 type;
    quint64 event_id;
    QString name;

};

static const EventDef eventlist[] = {
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ALIGNMENT_FAULTS,       "alignment-faults" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS,    "branch-instructions" },
    { PERF_TYPE_HW_CACHE, CACHE_BRANCH_READ_MISS,               "branch-load-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_BRANCH_READ,                    "branch-loads" },
    { PERF_TYPE_HW_CACHE, CACHE_BRANCH_READ_MISS,               "branch-mispredicts" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES,          "branch-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_BRANCH_READ,                    "branch-predicts" },
    { PERF_TYPE_HW_CACHE, CACHE_BRANCH_READ_MISS,               "branch-read-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_BRANCH_READ,                    "branch-reads" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS,    "branches" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES,             "bus-cycles" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES,           "cache-misses" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES,       "cache-references" },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES,       "context-switches" },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_CLOCK,              "cpu-clock" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES,             "cpu-cycles" },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS,         "cpu-migrations" },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES,       "cs" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES,             "cycles" },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_EMULATION_FAULTS,       "emulation-faults" },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS,            "faults" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND, "idle-cycles-backend" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND, "idle-cycles-frontend" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS,           "instructions" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_READ_MISS,                  "l1d-cache-load-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_READ,                       "l1d-cache-loads" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_PREFETCH_MISS,              "l1d-cache-prefetch-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_PREFETCH,                   "l1d-cache-prefetches" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_READ_MISS,                  "l1d-cache-read-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_READ,                       "l1d-cache-reads" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_WRITE_MISS,                 "l1d-cache-store-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_WRITE,                      "l1d-cache-stores" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_WRITE_MISS,                 "l1d-cache-write-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_WRITE,                      "l1d-cache-writes" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_READ_MISS,                  "l1d-load-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_READ,                       "l1d-loads" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_PREFETCH_MISS,              "l1d-prefetch-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_PREFETCH,                   "l1d-prefetches" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_READ_MISS,                  "l1d-read-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_READ,                       "l1d-reads" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_WRITE_MISS,                 "l1d-store-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_WRITE,                      "l1d-stores" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_WRITE_MISS,                 "l1d-write-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1D_WRITE,                      "l1d-writes" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_READ_MISS,                  "l1i-cache-load-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_READ,                       "l1i-cache-loads" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_PREFETCH_MISS,              "l1i-cache-prefetch-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_PREFETCH,                   "l1i-cache-prefetches" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_READ_MISS,                  "l1i-cache-read-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_READ,                       "l1i-cache-reads" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_READ_MISS,                  "l1i-load-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_READ,                       "l1i-loads" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_PREFETCH_MISS,              "l1i-prefetch-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_PREFETCH,                   "l1i-prefetches" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_READ_MISS,                  "l1i-read-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_L1I_READ,                       "l1i-reads" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_READ_MISS,                  "llc-cache-load-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_READ,                       "llc-cache-loads" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_PREFETCH_MISS,              "llc-cache-prefetch-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_PREFETCH,                   "llc-cache-prefetches" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_READ_MISS,                  "llc-cache-read-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_READ,                       "llc-cache-reads" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_WRITE_MISS,                 "llc-cache-store-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_WRITE,                      "llc-cache-stores" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_WRITE_MISS,                 "llc-cache-write-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_WRITE,                      "llc-cache-writes" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_READ_MISS,                  "llc-load-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_READ,                       "llc-loads" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_PREFETCH_MISS,              "llc-prefetch-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_PREFETCH,                   "llc-prefetches" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_READ_MISS,                  "llc-read-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_READ,                       "llc-reads" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_WRITE_MISS,                 "llc-store-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_WRITE,                      "llc-stores" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_WRITE_MISS,                 "llc-write-misses" },
    { PERF_TYPE_HW_CACHE, CACHE_LLC_WRITE,                      "llc-writes" },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MAJ,        "major-faults" },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS,         "migrations" },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MIN,        "minor-faults" },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS,            "page-faults" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND, "stalled-cycles-backend" },
    { PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND, "stalled-cycles-frontend" },
    { PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK,             "task-clock" },
    { PERF_TYPE_MAX, 0, "none" }
};

#define EINTR_LOOP(var, cmd)                    \
    do {                                        \
        var = cmd;                              \
    } while (var == -1 && errno == EINTR)

static inline qint64 qt_safe_read(int fd, void *data, qint64 maxlen)
{
    qint64 ret = 0;
    EINTR_LOOP(ret, ::read(fd, data, maxlen));
    return ret;
}

static int perf_event_open(perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags)
{
#ifdef SYS_perf_event_open
    return syscall(SYS_perf_event_open, attr, pid, cpu, group_fd, flags);
#else
    Q_UNUSED(attr);
    Q_UNUSED(pid);
    Q_UNUSED(cpu);
    Q_UNUSED(group_fd);
    Q_UNUSED(flags);
    errno = ENOSYS;
    return -1;
#endif
}

class PerfEvent::impl {
public:
    impl(const QString& name);
    ~impl() { close(); }
    bool isAvailable();
    bool setEvent(const QString &name);
    QString& event();
    bool open();
    void close();
    void enable();
    void disable();
    quint64 read();
private:

    struct read_format {
        quint64 value;
        quint64 time_enabled;
        quint64 time_running;
    };

    int m_fd;
    QString m_name;
    struct perf_event_attr m_attr;
    struct read_format m_prev;
    struct read_format m_curr;
};

PerfEvent::impl::impl(const QString& name = "task-clock") :
    m_fd(-1), m_name(name), m_attr(), m_prev(), m_curr()
{
    memset(&m_attr, 0, sizeof(m_attr));
    m_attr.size = sizeof(m_attr); // required for the perf ABI
    m_attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
    m_attr.disabled = true; // we'll enable later
    m_attr.inherit = true; // let children processes inherit the monitoring
    m_attr.pinned = true; // keep it running in the hardware
    m_attr.inherit_stat = true; // aggregate all the info from child processes
    m_attr.task = true; // trace fork/exits
    setEvent(name);
}

bool PerfEvent::impl::isAvailable()
{
    // this generates an EFAULT because attr == NULL if perf_event_open is available
    // if the kernel is too old, it generates ENOSYS
    return perf_event_open(0, 0, 0, 0, 0) == -1 && errno != ENOSYS;
}


const EventDef* lookupEventDef(const QString &name)
{
    for (int i = 0; eventlist[i].type != PERF_TYPE_MAX; i++) {
        if (name == eventlist[i].name) {
            return &eventlist[i];
        }
    }
    return nullptr;
}

bool PerfEvent::impl::setEvent(const QString &name)
{
    if (const EventDef *def = lookupEventDef(name)) {
        m_attr.type = def->type;
        m_attr.config = def->event_id;
        m_name = name;
        return true;
    }
    return false;
}

QString& PerfEvent::impl::event()
{
    return m_name;
}

bool PerfEvent::impl::open()
{
    if (!isAvailable()) {
        return false;
    }
    close();
    m_fd = perf_event_open(&m_attr, 0, -1, -1, 0);
    if (m_fd > 0) {
        ::fcntl(m_fd, F_SETFD, FD_CLOEXEC);
        ::ioctl(m_fd, PERF_EVENT_IOC_RESET);
        return true;
    }
    return false;
}

void PerfEvent::impl::enable()
{
    ioctl(m_fd, PERF_EVENT_IOC_ENABLE);
}

void PerfEvent::impl::disable()
{
    ioctl(m_fd, PERF_EVENT_IOC_DISABLE);
}

void PerfEvent::impl::close()
{
    int ret;
    EINTR_LOOP(ret, ::close(m_fd));
    m_fd = -1;
}

quint64 PerfEvent::impl::read()
{
    struct read_format results = { 0, 0, 0 };
    size_t nread = 0;
    while (nread < sizeof(results)) {
        char *ptr = reinterpret_cast<char *>(&results);
        qint64 r = qt_safe_read(m_fd, ptr + nread, sizeof results - nread);
        if (r == -1) {
            results = { 0, 0, 0 };
            break;
        }
        nread += quint64(r);
    }

    if (results.time_running == results.time_enabled)
        return results.value;

    // scale the results in case multiplexing was necessary
    // time_enabled should not be zero
    return results.value * (double(results.time_running) / double(results.time_enabled));
}

// The outer class seems just boilerplate and is ackward...
// Is there a better way to implement such facade?
PerfEvent::PerfEvent() : m_impl(new impl())
{
}

// The default destructor is inline, and the client needs to know
// the size of the internal implementation. Explicit destructor
// avoids the default inline destructor
PerfEvent::~PerfEvent()
{
}

bool PerfEvent::setEvent(const QString &name)
{
    return m_impl->setEvent(name);
}

QString& PerfEvent::event()
{
    return m_impl->event();
}

bool PerfEvent::isAvailable()
{
    return m_impl->isAvailable();
}

bool PerfEvent::open()
{
    return m_impl->open();
}

void PerfEvent::close()
{
    m_impl->close();
}

void PerfEvent::enable()
{
    m_impl->enable();
}

void PerfEvent::disable()
{
    m_impl->disable();
}

quint64 PerfEvent::read()
{
    return m_impl->read();
}

QSet<QString> PerfEvent::availableEvents()
{
    QSet<QString> lst;
    for (int i = 0; eventlist[i].type != PERF_TYPE_MAX; i++) {
        const QString name = eventlist[i].name;
        PerfEvent::impl p(name);
        if (p.open()) {
            lst.insert(name);
        }
    }
    return lst;
}
