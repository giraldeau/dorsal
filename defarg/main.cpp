#include <QDebug>

#include <limits>

#include "test.h"

int main(int argc, char *argv[])
{

    Test t1;
    Test t2(42);

    qDebug() << t1.m_x << t2.m_x;

    qDebug() << std::numeric_limits<float>::min();

    return 0;
}

