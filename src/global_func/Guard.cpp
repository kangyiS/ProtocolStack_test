#include "Guard.h"

CGuard::CGuard(pthread_mutex_t mutexTemp, uint8_t need_init)
{
    mutex = mutexTemp;
    if(need_init)
    {
        pthread_mutex_init(&mutex,NULL);
    }
    pthread_mutex_lock(&mutex);
    locked = 1;
}

CGuard::~CGuard()
{
    if(locked == 1)
    {
        pthread_mutex_unlock(&mutex);
    }
}

void CGuard::lock()
{
    pthread_mutex_lock(&mutex);
    locked = 1;
}

void CGuard::unlock()
{
    pthread_mutex_unlock(&mutex);
    locked = 0;
} 
