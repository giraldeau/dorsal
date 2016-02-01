#include <QString>
#include <QtTest>

#include "perf.h"

class PerfTest : public QObject
{
    Q_OBJECT

public:
    PerfTest();

private Q_SLOTS:
    void testCreateCounter();
};

PerfTest::PerfTest()
{
}

void PerfTest::testCreateCounter()
{
    Perf p;

    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(PerfTest)

#include "tst_perftest.moc"
