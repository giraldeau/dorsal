#include <QDebug>
#include <iostream>

#include "perf.h"

void display_events()
{
    PerfEvent perf;

    // hum... this is quite convoluted just to display a sorted list

    QSet<QString> set = perf.availableEvents();
    QVector<QString> events;
    for (const QString name: set) {
        events.append(name);
    }
    qSort(events);

    std::cout << events.size() << " events available(s):" << std::endl;
    for (const QString name: events) {
        std::cout << "    " << name.toStdString() << std::endl;
    }
}

int main(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    display_events();

    return 0;
}

