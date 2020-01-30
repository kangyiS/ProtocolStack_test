#include "Guard.h"

CGuard::CGuard(pthread_mutex_t mutexTemp)
{
    mutex = mutexTemp;
    pthread_mutex_init(&mutex,NULL);
    pthread_mutex_lock(&mutex);
}

CGuard::~CGuard()
{
    pthread_mutex_unlock(&mutex);
}

void CGuard::lock()
{
    pthread_mutex_lock(&mutex);
}

void CGuard::unlock()
{
    pthread_mutex_unlock(&mutex);
} 
