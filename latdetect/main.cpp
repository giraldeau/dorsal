#include <QDebug>
#include <QElapsedTimer>

int main(int argc, char *argv[])
{
    Q_UNUSED(argc); Q_UNUSED(argv);

    int repeat = 1E6;
    QElapsedTimer time;
    QVector<qint64> samples(repeat);
    qFill(samples, 0);

    // generate samples
    for (int i = 0; i < repeat; i++) {
        time.restart();
        samples[i] = time.nsecsElapsed();
    }

    // report
    qSort(samples.begin(), samples.end(), qGreater<qint64>());
    int top = 5;
    for (int i = 0; i < top; i++) {
        qDebug() << samples[i];
    }

    for (int i = repeat - top; i < repeat; i++) {
        qDebug() << samples[i];
    }

    return 0;
}

