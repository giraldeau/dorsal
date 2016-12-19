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

0000000000400b20 <_ZN3Foo3fooEv>:
  400b20:       55                      push   %rbp
  400b21:       53                      push   %rbx
  400b22:       48 89 fb                mov    %rdi,%rbx
  400b25:       48 83 ec 08             sub    $0x8,%rsp
  400b29:       48 8d 2d f0 ff ff ff    lea    -0x10(%rip),%rbp        # 400b20 <_ZN3Foo3fooEv>
  400b30:       48 8b 74 24 18          mov    0x18(%rsp),%rsi
  400b35:       48 89 ef                mov    %rbp,%rdi
  400b38:       e8 f3 fb ff ff          callq  400730 <__cyg_profile_func_enter@plt>
  400b3d:       48 89 df                mov    %rbx,%rdi
  400b40:       e8 7b ff ff ff          callq  400ac0 <_ZN3Foo3barEv>
  400b45:       48 63 33                movslq (%rbx),%rsi
  400b48:       48 89 df                mov    %rbx,%rdi
  400b4b:       e8 e0 fe ff ff          callq  400a30 <_ZN3Foo3hogEl>
  400b50:       48 8b 74 24 18          mov    0x18(%rsp),%rsi
  400b55:       48 83 c4 08             add    $0x8,%rsp
  400b59:       48 89 ef                mov    %rbp,%rdi
  400b5c:       5b                      pop    %rbx
  400b5d:       5d                      pop    %rbp
  400b5e:       e9 ed fb ff ff          jmpq   400750 <__cyg_profile_func_exit@plt>
  400b63:       48 89 c3                mov    %rax,%rbx
  400b66:       48 8b 74 24 18          mov    0x18(%rsp),%rsi
  400b6b:       48 89 ef                mov    %rbp,%rdi
  400b6e:       e8 dd fb ff ff          callq  400750 <__cyg_profile_func_exit@plt>
  400b73:       48 89 df                mov    %rbx,%rdi
  400b76:       e8 f5 fb ff ff          callq  400770 <_Unwind_Resume@plt>
  400b7b:       0f 1f 44 00 00          nopl   0x0(%rax,%rax,1)


  */
