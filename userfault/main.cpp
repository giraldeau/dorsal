#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <linux/userfaultfd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <assert.h>

pthread_barrier_t barrier;

struct buf {
    void *data;
    int size;
    int nr_pages;
    int page_size;
};

struct ctx {
    int uffd;
    int uffd_flags;
    struct buf buf;
    int nr_events;
};

void *monitor_loop(void *arg)
{
    struct ctx *c = (struct ctx *) arg;
    unsigned long page_mask = ~(c->buf.page_size - 1);
    int ret;

    pthread_barrier_wait(&barrier);

    void *fancy_page = mmap(NULL, c->buf.page_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memset(fancy_page, 42, c->buf.page_size);

    while(true) {
        /*
         * read page fault events
         */
        struct uffd_msg msg;
        ret = read(c->uffd, &msg, sizeof(msg));
        if (errno == EAGAIN)
            continue;
        if (ret < 0)
            break;
        if (ret != sizeof(msg))
            break;
        if (msg.event != UFFD_EVENT_PAGEFAULT)
            continue;
        printf("page fault at address 0x%llx\n", msg.arg.pagefault.address);

        /*
         * fixup this page fault
         */
        struct uffdio_copy cpy;

        cpy.dst = msg.arg.pagefault.address & page_mask;
        cpy.src = (unsigned long) fancy_page;
        cpy.len = c->buf.page_size;
        cpy.mode = 0;
        cpy.copy = 0;
        ret = ioctl(c->uffd, UFFDIO_COPY, &cpy);
        if (ret) {
            fprintf(stderr, "ioctl returned %d\n", ret);
            exit(1);
        }
        c->nr_events++;
    }

    munmap(fancy_page, c->buf.page_size);
    return NULL;
}

int uffd_init(struct ctx *ctx)
{
    struct uffdio_api uffdio_api;

    ctx->uffd = syscall(__NR_userfaultfd, O_CLOEXEC | O_NONBLOCK);

    if (ctx->uffd < 0) {
        printf("userfault not supported\n");
        return -1;
    }

    ctx->uffd_flags = fcntl(ctx->uffd, F_GETFD, NULL);

    uffdio_api.api = UFFD_API;
    uffdio_api.features = 0;
    if (ioctl(ctx->uffd, UFFDIO_API, &uffdio_api)) {
        fprintf(stderr, "UFFDIO_API\n");
        return -1;
    }
    if (uffdio_api.api != UFFD_API) {
        fprintf(stderr, "UFFDIO_API error %Lu\n", uffdio_api.api);
        return -1;
    }
    return 0;
}

int alloc_buf(struct buf *buf, int nr_pages)
{
    buf->nr_pages = nr_pages;
    buf->page_size = getpagesize();
    buf->size = nr_pages * buf->page_size;

    /*
     * Allocate memory, initialize it and mark it as MADV_DONTNEED
     */
    buf->data = mmap(NULL, buf->size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memset(buf->data, 0, buf->size);
    madvise(buf->data, buf->size, MADV_DONTNEED);
    return buf->data == NULL;
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    struct ctx ctx;
    const int nr_pages = 10;

    if (uffd_init(&ctx) < 0) {
        fprintf(stderr, "uffd_init failed\n");
        return 1;
    }

    if (alloc_buf(&ctx.buf, nr_pages)) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    struct uffdio_register uffdio_register;
    fcntl(ctx.uffd, F_SETFL, ctx.uffd_flags & ~O_NONBLOCK);

    uffdio_register.range.start = (unsigned long) ctx.buf.data;
    uffdio_register.range.len = ctx.buf.size;
    uffdio_register.mode = UFFDIO_REGISTER_MODE_MISSING;
    if (ioctl(ctx.uffd, UFFDIO_REGISTER, &uffdio_register)) {
        fprintf(stderr, "register failure\n");
        return 1;
    }

    ctx.nr_events = 0;
    pthread_t monitor;
    pthread_barrier_init(&barrier, NULL, 2);
    pthread_create(&monitor, NULL, monitor_loop, &ctx);
    pthread_barrier_wait(&barrier);

    for (int i = 0; i < ctx.buf.size; i++) {
        char *b = (char *) ctx.buf.data;
        assert(b[i] == 42);
    }

    /*
     * Cancel is required, otherwise the thread get stuck blocking on read()
     */
    pthread_cancel(monitor);
    pthread_join(monitor, NULL);

    printf("nr_events: %d\n", ctx.nr_events);
    assert(ctx.nr_events == nr_pages);

    munmap(ctx.buf.data, ctx.buf.size);
    return 0;
}

