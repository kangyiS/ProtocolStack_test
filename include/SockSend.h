#ifndef __SOCK_SEND_H__
#define __SOCK_SEND_H__

#include <iostream> // std
#include "Guard.h"

class CSockSend
{
public:
    static CSockSend* instance();
    int16_t createSocket();
private:
    CSockSend();
    ~CSockSend();
    int16_t m_sockfd;
    pthread_mutex_t mutex_createSocket;
};
#endif
