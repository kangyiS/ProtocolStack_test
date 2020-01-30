#ifndef __SOCK_TEMP_H__
#define __SOCK_TEMP_H__

#include <iostream>

class CSockTemp
{
public:
    CSockTemp();
    ~CSockTemp();
    int16_t createSocket(int16_t af, int16_t type, int16_t proto);
private:
    int16_t m_sockfd;
};
#endif
