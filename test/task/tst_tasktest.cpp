#include <QString>
#include <QtTest>
#include <QThread>

#include "taskmanager.h"
#include "taskrt.h"

class TaskTest : public QObject
{
    Q_OBJECT

public:
    TaskTest();

private Q_SLOTS:
    void testTaskManager();
};

TaskTest::TaskTest()
{
}

// Prototype of monitor
class TaskMonitor {
public:
    void begin() {
        qDebug() << "start";
    }
    void waiting() {
        qDebug() << "waiting";
    }
    void running() {
        qDebug() << "running";
    }
    void done() {
        qDebug() << "done";
    }
};

// Prototype of thread control
class ThreadRT {
public:
    ThreadRT() :m_shutdown(0), m_thd(0) { }
    void start() {
        pthread_create(&m_thd, nullptr, fn, this);
    }

    void join() {
        pthread_join(m_thd, nullptr);
    }

    void stop() {
        m_shutdown++;
    }

    bool isShutdown() {
        return !!m_shutdown;
    }

private:
    static void *fn(void *args) {
        ThreadRT *th = (ThreadRT*) args;
        // TODO: set thread priority and scheduler

        // TODO: factorize parameters and monitor
        TaskRT t1(10, 20, 30);
        TaskMonitor monitor;

        /* TaskMonitorGroup:
        QVector<QSharedPointer<TaskMonitor>> monitors;
        std::for_each(monitors.begin(), monitors.end(), [](){
            mon.waiting();
        });
        */

        monitor.begin();
        while (!th->isShutdown()) {
            monitor.waiting();
            QThread::msleep(1); // TODO: factorize timer
            monitor.running();
            t1.run(); // TODO: can it be a lambda?
        }
        monitor.done();
        return nullptr;
    }
    int m_shutdown;
    int m_prio;
    pthread_t m_thd;
};

void TaskTest::testTaskManager()
{

    // TODO: put the logic into the TaskManager
    ThreadRT t;
    t.start();
    QThread::msleep(10);
    t.stop();
    t.join();

    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(TaskTest)

#include "tst_tasktest.moc"
