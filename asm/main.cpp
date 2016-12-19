#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "ops.h"

#define DEPTH_MAX 15
void *buf[DEPTH_MAX];
char page[4096];

void do_unwind()
{
    unw_backtrace((void **)&buf, DEPTH_MAX);
}

void foo1()
{
    asm volatile(
        "push %%rbp\n"
        "mov %%rsp, %%rbp\n"
        "mov $0x1234, %%r10\n"
        "sub $0x1000, %%rbp\n"
        "pop %%rbp"
        : );
    do_unwind();
}

void foo2()
{
    asm volatile(
        "push %%rax\n"
        "push %%rax\n"
        "push %%rax\n" :);
    do_unwind();
    asm volatile(
        "pop %%rax\n"
        "pop %%rax\n"
        "pop %%rax\n"
        : );
    //do_unwind();
}

void save_page(const char *fname, char *ptr)
{
    int dest_fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    int ret = write(dest_fd, ptr, 4096);
    (void) ret;
    close(dest_fd);
}

__attribute__((noinline))
void hello()
{
    volatile int cafe = 0xCAFEBABE;
    (void) cafe;
    register unsigned long rsp asm("rsp");
    unsigned long val = rsp;
    memcpy(page, (char *)val, 4096);
    printf("hello %lx\n", rsp);
    do_unwind();
}

int main()
{
    register unsigned long rsp asm("rsp");
    printf("main %lx\n", rsp);
    hello();
    save_page("stack.page", page);

    fn();

    return 0;
}

