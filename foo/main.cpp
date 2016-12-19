#include <iostream>

using namespace std;

class Foo {
public:
    __attribute__((noinline)) Foo(int niters) : m_niters(niters) {}
    __attribute__((noinline)) void foo() {
        bar();
        hog(m_niters);
    }
    __attribute__((noinline)) void bar() {
        baz();
        hog(m_niters);
    }
    __attribute__((noinline)) void baz() {
        hog(m_niters);
    }
    __attribute__((noinline)) void hog(long niters) {
        volatile int dummy = 1;
        while (dummy && --niters);
    }

private:
    int m_niters;
};

int main()
{
    Foo *foo = new Foo(1E7);

    for (int i = 0; i < 10; i++) {
        foo->foo();
        foo->bar();
        foo->baz();
    }

    return 0;
}

/*
Sans instrumentation
00000000004007d0 <_ZN3Foo3fooEv>:
  4007d0:       53                      push   %rbx
  4007d1:       48 89 fb                mov    %rdi,%rbx
  4007d4:       e8 d7 ff ff ff          callq  4007b0 <_ZN3Foo3barEv>
  4007d9:       48 63 3b                movslq (%rbx),%rdi
  4007dc:       5b                      pop    %rbx
  4007dd:       e9 8e ff ff ff          jmpq   400770 <_ZN3Foo3hogEl.isra.0>
  4007e2:       66 2e 0f 1f 84 00 00    nopw   %cs:0x0(%rax,%rax,1)
  4007e9:       00 00 00
  4007ec:       0f 1f 40 00             nopl   0x0(%rax)


0000000000400860 <_ZN3Foo3fooEv>:
  400860:       55                      push   %rbp
  400861:       48 89 e5                mov    %rsp,%rbp
  400864:       53                      push   %rbx
  400865:       48 83 ec 08             sub    $0x8,%rsp
  400869:       ff 15 89 07 20 00       callq  *0x200789(%rip)        # 600ff8 <_DYNAMIC+0x1f0>
  40086f:       48 89 fb                mov    %rdi,%rbx
  400872:       e8 b9 ff ff ff          callq  400830 <_ZN3Foo3barEv>
  400877:       48 63 3b                movslq (%rbx),%rdi
  40087a:       48 83 c4 08             add    $0x8,%rsp
  40087e:       5b                      pop    %rbx
  40087f:       5d                      pop    %rbp
  400880:       e9 4b ff ff ff          jmpq   4007d0 <_ZN3Foo3hogEl.isra.0>
  400885:       66 2e 0f 1f 84 00 00    nopw   %cs:0x0(%rax,%rax,1)
  40088c:       00 00 00
  40088f:       90                      nop


  */
