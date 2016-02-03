#include <QDebug>

#include "taskrt.h"

TaskRT::TaskRT(uint p, uint d, uint w, QObject *parent) :
    QObject(parent), m_period(p), m_deadline(d), m_work(w)
{
}

void TaskRT::run()
{
    qDebug() << "work";
}
