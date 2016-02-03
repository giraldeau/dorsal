#ifndef TASKRT_H
#define TASKRT_H

#include <QObject>
#include <QRunnable>
#include <QAtomicInt>

class TaskRT : public QObject, QRunnable
{
    Q_OBJECT
    Q_PROPERTY(uint period READ period WRITE setPeriod)
    Q_PROPERTY(uint deadline READ deadline WRITE setDeadline)
    Q_PROPERTY(uint work READ work WRITE setWork)

public:
    explicit TaskRT(uint p = 0, uint d = 0, uint w = 0, QObject *parent = 0);

    uint period() const { return m_period; }
    void setPeriod(uint period) { m_period = period; }

    uint deadline() const { return m_deadline; }
    void setDeadline(uint deadline) { m_deadline = deadline; }

    uint work() const { return m_work; }
    void setWork(uint work) { m_work = work; }

    void run();

signals:
public slots:
private:
    uint m_period;
    uint m_deadline;
    uint m_work;
};

#endif // TASKRT_H
