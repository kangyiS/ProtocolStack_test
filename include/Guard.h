#ifndef __GUARD_H__
#define __GUARD_H__

#include <pthread.h>

class CGuard
{
public:
    CGuard(pthread_mutex_t mutexTemp);
    ~CGuard();
    void lock();
    void unlock();
private:
    pthread_mutex_t mutex;
};

#endif
