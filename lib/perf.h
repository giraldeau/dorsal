#ifndef PERF_H
#define PERF_H

#include <memory>

#include "perf_global.h"

class PERFSHARED_EXPORT PerfEvent
{
public:
    PerfEvent();
    ~PerfEvent();

    bool isAvailable();
    QSet<QString> availableEvents();
    bool setEvent(const QString &name);
    QString& event();
    bool open();
    void close();
    void enable();
    void disable();
    quint64 read();

private:
    // Pointer To Implementation
    class impl;
    std::unique_ptr<impl> m_impl;
};

#endif // PERF_H
