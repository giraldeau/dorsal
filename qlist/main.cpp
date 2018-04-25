#include <QDebug>
#include <QList>

class Foo {
public:
    Foo() { qDebug() << "Foo::Foo" << this; }
    Foo(const Foo& other) { val=other.val; qDebug() << "Foo::Foo (copy)" << this; }
    ~Foo() { qDebug() << "Foo::~Foo()" << this; }
    int val;
};

QDebug operator<<(QDebug dbg, const Foo &foo)
{
    dbg.nospace() << "Foo " << foo.val;
    return dbg.maybeSpace();
}

void add(QList<Foo>& lst)
{
    static int x = 42;
    Foo foo;
    foo.val = x++;
    lst.insert(0, foo);
}

void run()
{
    QList<Foo> lst;
    add(lst);
    add(lst);
    add(lst);
    qDebug() << lst;
}

int main()
{

    qDebug() << "before";
    run();
    qDebug() << "after";

    return 0;
}
