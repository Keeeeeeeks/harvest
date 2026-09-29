// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CCriticalSection.h"
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace core {

CCriticalSection::CCriticalSection()
{
    pthread_mutex_init(&Mutex, 0);
}

CCriticalSection::~CCriticalSection()
{
    pthread_mutex_destroy(&Mutex);
}

void CCriticalSection::enter()
{
    pthread_mutex_lock(&Mutex);
}

void CCriticalSection::leave()
{
    pthread_mutex_unlock(&Mutex);
}

} // end namespace core
} // end namespace ox
