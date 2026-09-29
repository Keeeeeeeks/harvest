// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CTimeCounter.h"
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace algo {

CTimeCounter::CTimeCounter()
    : Remaining(0.0f), Delay(1.0f)
{
}

CTimeCounter::~CTimeCounter()
{
}

void CTimeCounter::putDelay(float time, float delay)
{
    Remaining += time;
    Delay = delay;
}

void CTimeCounter::setDelay(float delay)
{
    Remaining = delay;
    Delay = delay;
}

void CTimeCounter::overrideStartingValue(float delay)
{
    Delay = delay;
}

bool CTimeCounter::updateCounter(float frameDelta)
{
    if (Remaining < 0.0f)
    {
        Remaining = 0.0f;
        return false;
    }

    if (Remaining >= 0.0f)
    {
        Remaining -= frameDelta;
        if (Remaining <= 0.0f)
            return true;
    }

    return false;
}

bool CTimeCounter::isStarted()
{
    return Remaining > 0.0f;
}

bool CTimeCounter::isDelayed()
{
    return Remaining > 0.0f;
}

float CTimeCounter::getRemainingTime()
{
    return Remaining;
}

float CTimeCounter::getProgress()
{
    if (Remaining < 0.0f)
        return 1.0f;

    return 1.0f - Remaining / Delay;
}

float CTimeCounter::getInvertedProgress()
{
    if (Remaining < 0.0f)
        return 0.0f;

    return Remaining / Delay;
}

} // end namespace algo
} // end namespace ox
