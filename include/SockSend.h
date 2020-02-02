#ifndef __SOCK_SEND_H__
#define __SOCK_SEND_H__

#include <iostream> // std
#include "Guard.h"

class CSockSend
{
public:
    static CSockSend* instance();
    int16_t createSocket();
    void setNIC(std::string nic);
    std::string getNIC();
    int16_t sendData(int16_t sock, uint8_t* frame, uint16_t frameLen, int16_t flags);
private:
    CSockSend();
    ~CSockSend();
    int16_t m_sockfd;
    std::string m_nic;
    pthread_mutex_t mutex_createSocket;
    pthread_mutex_t mutex_nic;
};
#endif
