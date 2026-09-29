// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef OX_CORE_CTHREAD_H
#define OX_CORE_CTHREAD_H

#include <pthread.h>

namespace ox {
namespace core {

//! A thread running function(parameter) from construction.
class CThread
{
public:
    CThread(void (*function)(void*), void* parameter);
    virtual ~CThread();

    static void sleep(unsigned int milliseconds);

private:
    void (*Function)(void*);
    pthread_t Thread;
};

} // end namespace core
} // end namespace ox

#endif
