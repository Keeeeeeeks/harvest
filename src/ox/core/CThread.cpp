// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CThread.h"
#include <unistd.h>
// The object has an iostream static initializer; the header that pulled it in is not identified yet.
#include <iostream>

namespace ox {
namespace core {

CThread::CThread(void (*function)(void*), void* parameter)
    : Function(function)
{
    pthread_create(&Thread, 0, (void* (*)(void*))function, parameter);
}

void CThread::sleep(unsigned int milliseconds)
{
    usleep(milliseconds * 1000);
}

CThread::~CThread()
{
    pthread_join(Thread, 0);
}

} // end namespace core
} // end namespace ox
