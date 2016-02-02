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
    PerfEvent p;
    QString event = "cycles";

    p.setEvent(event);
    QVERIFY2(event == p.event(), "mismatch");

    bool ret = p.open();
    QVERIFY2(ret, "open failed");

    p.enable();
    p.disable();
    quint64 val1 = p.read();
    quint64 val2 = p.read();
    p.enable();
    quint64 val3 = p.read();
    p.disable();

    QVERIFY2(val1 > 0, "read failed");
    QVERIFY2(val1 == val2, "counter was enabled");
    QVERIFY2(val3 > val2, "counter was disabled");

    QSet<QString> events = p.availableEvents();
    QVERIFY2(event.size() > 0, "no available events");
    for (const QString ev: events) {
        p.setEvent(ev);
        QVERIFY2(p.open(), ev.toStdString().c_str());
    }

    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(PerfTest)

#include "tst_perftest.moc"
