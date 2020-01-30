#include "SockTemp.h"
#include <unistd.h> // close()
#include <sys/socket.h> // socket()

CSockTemp::CSockTemp()
{
    m_sockfd = -1;
}

CSockTemp::~CSockTemp()
{
    if(m_sockfd >= 0)
    {
        close(m_sockfd);
    }
}

int16_t CSockTemp::createSocket(int16_t af, int16_t type, int16_t proto)
{
    if(m_sockfd < 0)
    {
        m_sockfd = socket(af, type, proto);
    }
    return m_sockfd;
}
