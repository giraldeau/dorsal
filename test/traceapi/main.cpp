#include <QDebug>
#include <QThread>
#include <QRunnable>
#include <QThreadPool>
#include <QMutex>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <functional>
#include <lttng/lttng.h>

void dump(char *data, int size) {
    for (int i = 0; i < size; i++) {
        std::printf("%2x ", data[i] & 0xFF);
    }
    std::cout << std::endl;
}

QMutex mutex;
bool active;

class LambdaRunnable : public QRunnable
{
public:
    LambdaRunnable(std::function<void ()> fn) : m_fn(fn) { }
    void run() {
        m_fn();
    }
private:
    std::function<void ()> m_fn;
};

void trigger_snapshot()
{
    mutex.lock();
    if (active) {
        mutex.unlock();
        return;
    }
    active = true;
    QRunnable *snap = new LambdaRunnable([](){
        lttng_snapshot_output *snap = lttng_snapshot_output_create();
        int ret = lttng_snapshot_record("foo", snap, 0);
        qDebug() << ret;
        mutex.lock();
        active = false;
        mutex.unlock();
    });
    QThreadPool::globalInstance()->start(snap);
    mutex.unlock();
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;
    int ret;

    QThreadPool::globalInstance()->start(new LambdaRunnable([](){
        return;
    }));

    lttng_domain domain = { };
    domain.type = LTTNG_DOMAIN_KERNEL;

    ret = lttng_create_session_snapshot("foo", "file:///tmp/foo");
    qDebug() << ret;

    lttng_handle *handle = lttng_create_handle("foo", &domain);
    qDebug() << handle;

    lttng_channel chan = { };
    strcpy(chan.name, "k");
    chan.attr.overwrite = 0;
    chan.attr.subbuf_size = 4096;
    chan.attr.num_subbuf = 8;
    chan.attr.switch_timer_interval = 0;
    chan.attr.read_timer_interval = 200;
    chan.attr.output = LTTNG_EVENT_SPLICE;

    ret = lttng_enable_channel(handle, &chan);
    qDebug() << ret;

    lttng_event *events;
    ret = lttng_list_tracepoints(handle, &events);
    qDebug() << ret;

    for (int i = 0; i < ret; i++) {
        qDebug() << i << events[i].name;
    }
    free(events); events = nullptr;

    ret = lttng_list_syscalls(&events);
    qDebug() << ret;

    for (int i = 0; i < ret; i++) {
        qDebug() << i << events[i].name;
    }
    free(events); events = nullptr;

    // TODO: how to enable all tracepoints/syscalls/ust?
    lttng_event ev = { };
    strncpy(ev.name, "sched_switch", LTTNG_SYMBOL_NAME_LEN);
    ev.type = LTTNG_EVENT_TRACEPOINT;
    ev.loglevel_type = LTTNG_EVENT_LOGLEVEL_ALL;
    ret = lttng_enable_event(handle, &ev, "k");
    qDebug() << ret;

    strcpy(ev.name, "open");
    ev.type = LTTNG_EVENT_SYSCALL;
    ev.loglevel_type = LTTNG_EVENT_LOGLEVEL_ALL;
    ret = lttng_enable_event(handle, &ev, "k");
    qDebug() << ret;

    ret = lttng_start_tracing("foo");
    qDebug() << ret;

    for (int i = 0; i < 3; i++) {
        trigger_snapshot();
        QThread::msleep(1);
    }

    QThreadPool::globalInstance()->waitForDone();

    ret = lttng_stop_tracing("foo");
    qDebug() << ret;

    ret = lttng_destroy_session("foo");
    qDebug() << ret;

    return 0;
}

