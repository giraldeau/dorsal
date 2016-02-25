//#define _GNU_SOURCE // already defined
#include <QDebug>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>
#include <pthread.h>

#define gettid() syscall(__NR_gettid)

#define SCHED_DEADLINE 6

/* XXX use the proper syscall numbers */
#ifdef __x86_64__
#define __NR_sched_setattr             314
#define __NR_sched_getattr             315
#endif

#ifdef __i386__
#define __NR_sched_setattr             351
#define __NR_sched_getattr             352
#endif

#ifdef __arm__
#define __NR_sched_setattr             380
#define __NR_sched_getattr             381
#endif


static volatile int done;

struct sched_attr {
    __u32 size;

    __u32 sched_policy;
    __u64 sched_flags;

    /* SCHED_NORMAL, SCHED_BATCH */
    __s32 sched_nice;

    /* SCHED_FIFO, SCHED_RR */
    __u32 sched_priority;

    /* SCHED_DEADLINE (nsec) */
    __u64 sched_runtime;
    __u64 sched_deadline;
    __u64 sched_period;
};

int sched_setattr(pid_t pid,
                  const struct sched_attr *attr,
                  unsigned int flags)
{
    printf("syscall __NR_sched_setattr=%d\n", __NR_sched_setattr);
    return syscall(__NR_sched_setattr, pid, attr, flags);
}

int sched_getattr(pid_t pid,
                  struct sched_attr *attr,
                  unsigned int size,
                  unsigned int flags)
{
    return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

void *run_deadline(void *data)
{
    Q_UNUSED(data);
    struct sched_attr attr;
    int x = 0;
    int ret;
    unsigned int flags = 0;
    cpu_set_t cpuset;

    CPU_SET(0, &cpuset);
    CPU_SET(1, &cpuset);

    printf("deadline thread started [%ld]\n", gettid());

    /* */
    ret = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    if (ret < 0) {
        done = 0;
        perror("pthread_setaffinity_np");
        exit(-1);
    }

    attr.size = sizeof(attr);
    attr.sched_flags = 0;
    attr.sched_nice = 0;
    attr.sched_priority = 0;

    /* This creates a 10ms/30ms reservation */
    attr.sched_policy = SCHED_DEADLINE;
    //attr.sched_runtime = 10 * 1000 * 1000;
    //attr.sched_period = attr.sched_deadline = 30 * 1000 * 1000;
    attr.sched_runtime = 1050 * 1000;
    attr.sched_period = 10000 * 1000;
    attr.sched_deadline = 8000 * 1000;

    ret = sched_setattr(gettid(), &attr, flags);
    if (ret < 0) {
        done = 0;
        perror("sched_setattr");
        exit(-1);
    }

    FILE *berk = fopen("berk.data", "w");
    fwrite(&attr, sizeof(attr), 1, berk);
    fclose(berk);

    while (!done) {
        x++;
    }

    printf("deadline thread dies [%ld]\n", gettid());
    return NULL;
}

int main (int argc, char **argv)
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    pthread_t thread;

    printf("main thread [%ld]\n", gettid());

    pthread_create(&thread, NULL, run_deadline, NULL);

    sleep(10);

    done = 1;
    pthread_join(thread, NULL);

    printf("main dies [%ld]\n", gettid());
    return 0;
}
