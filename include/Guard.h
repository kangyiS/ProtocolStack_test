#ifndef __GUARD_H__
#define __GUARD_H__

#include <iostream> // std
#include <pthread.h>

class CGuard
{
public:
    CGuard(pthread_mutex_t mutexTemp, uint8_t need_init = 1);
    ~CGuard();
    void lock();
    void unlock();
private:
    pthread_mutex_t mutex;
    uint8_t locked;
};

#endif
