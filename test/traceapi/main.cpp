#include <QDebug>
#include <QThread>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <lttng/lttng.h>

void dump(char *data, int size) {
    for (int i = 0; i < size; i++) {
        std::printf("%2x ", data[i] & 0xFF);
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;
    int ret;

    lttng_domain domain = { };
    domain.type = LTTNG_DOMAIN_KERNEL;

    ret = lttng_create_session("foo", "file:///tmp/foo");
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

    lttng_event ev = { };
    strcpy(ev.name, "sched_switch");
    ev.type = LTTNG_EVENT_TRACEPOINT;
    ev.loglevel_type = LTTNG_EVENT_LOGLEVEL_ALL;
    ret = lttng_enable_event(handle, &ev, "k");
    qDebug() << ret;

    ret = lttng_start_tracing("foo");
    qDebug() << ret;

    QThread::msleep(1);
    ret = lttng_stop_tracing("foo");
    qDebug() << ret;

    ret = lttng_destroy_session("foo");
    qDebug() << ret;

    return 0;
}

