#include "sleepernano.h"

SleeperNano::SleeperNano(uint period) : Sleeper(period)
{

}

void SleeperNano::start()
{

}

void SleeperNano::waitNextPeriod()
{
    struct timespec ts = { 0, m_period };
    nanosleep(&ts, nullptr);
}

void SleeperNano::stop()
{

}
