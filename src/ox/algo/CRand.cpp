// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CRand.h"
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace algo {

int CRand::ms_seed = 0x0f0f0f0f;

int CRand::performRand(int seed)
{
    seed = (seed % 52774) * 40692 - (seed / 52774) * 3791;
    if (seed <= 0)
        seed += 2147483399;
    return seed;
}

int CRand::rand()
{
    ms_seed = performRand(ms_seed);
    return ms_seed;
}

void CRand::reset()
{
    ms_seed = 0x0f0f0f0f;
}

void CRand::srand(int seed)
{
    ms_seed = seed;
    if (ms_seed == 0)
        ms_seed = 1;
}

int CRand::nextInt()
{
    Current = performRand(Current);
    return Current;
}

int CRand::nextInt(int max)
{
    Current = performRand(Current);
    return Current % max;
}

void CRand::setCurrent(int current)
{
    Current = current;
}

} // end namespace algo
} // end namespace ox
