// Recovered for Harvest from the Linux 1.18 build; not the original source.

#ifndef OX_CORE_CCRITICALSECTION_H
#define OX_CORE_CCRITICALSECTION_H

#include <pthread.h>

namespace ox {
namespace core {

//! A mutex.
class CCriticalSection
{
public:
    CCriticalSection();
    virtual ~CCriticalSection();

    void enter();
    void leave();

private:
    pthread_mutex_t Mutex;
};

} // end namespace core
} // end namespace ox

#endif
