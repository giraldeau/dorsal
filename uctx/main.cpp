#include <iostream>
#include <ucontext.h>

ucontext_t ctx;
int done;

void foo() {
    if (done == 2)
        return;
    setcontext(&ctx);
}

void bar() {
    getcontext(&ctx);
    std::cout << "done " << done++ << std::endl;
    foo();
}


int main(int argc, char *argv[])
{
    (void) argc; (void) argv;
    bar();
}

