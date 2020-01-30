#include "SockSend.h"
#include <unistd.h> // close()
#include <sys/socket.h> // socket()
#include <netinet/ether.h>
#include <arpa/inet.h>

using namespace std;

CSockSend::CSockSend()
{
    m_sockfd = -1;
}

CSockSend::~CSockSend()
{
    if(m_sockfd >= 0)
    {
        close(m_sockfd);
    }
}

CSockSend* CSockSend::instance()
{
    static CSockSend* ptr = new CSockSend();
    return ptr;
}

int16_t CSockSend::createSocket()
{
    CGuard guard(mutex_createSocket);
    if(m_sockfd < 0)
    {
        m_sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    }
    return m_sockfd;
}
