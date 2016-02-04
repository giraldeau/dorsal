#ifndef SLEEPERNANO_H
#define SLEEPERNANO_H

#include "sleeper.h"

class SleeperNano : public Sleeper
{
public:
    SleeperNano(uint period);
    void start();
    void waitNextPeriod();
    void stop();
};

#endif // SLEEPERNANO_H
