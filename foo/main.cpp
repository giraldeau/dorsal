#include <iostream>

using namespace std;



class Foo {
public:
    Foo(int niters) : m_niters(niters) {}
    void foo() {
        bar();
        hog(m_niters);
    }
    void bar() {
        baz();
        hog(m_niters);
    }
    void baz() {
        hog(m_niters);
    }
    void hog(long niters) {
        volatile int dummy = 1;
        while (dummy && --niters);
    }

private:
    int m_niters;
};

int main()
{
    Foo foo(1E7);

    for (int i = 0; i < 10; i++) {
        foo.foo();
        foo.bar();
        foo.baz();
    }

    return 0;
}

