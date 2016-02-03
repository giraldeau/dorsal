#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>

#include "taskrt.h"

class TaskManager : public QObject
{
    Q_OBJECT    

public:
    explicit TaskManager(QObject *parent = 0);
    bool start(QVector<TaskRT> &tasks);
    bool stop();
    bool join();
signals:

public slots:
};

#endif // TASKMANAGER_H
