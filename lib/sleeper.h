#ifndef SLEEPER_H
#define SLEEPER_H

#include <QObject>

class Sleeper
{
public:
    Sleeper(uint period = 0, uint sync = 0);
    virtual void start() = 0;
    virtual void waitNextPeriod() = 0;
    virtual void stop() = 0;

protected:
    uint m_period;
    uint m_sync;
};

#endif // SLEEPER_H
