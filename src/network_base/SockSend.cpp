#include "SockSend.h"
#include <unistd.h> // close()
#include <sys/socket.h> // socket()
#include <netinet/ether.h>
#include <arpa/inet.h>

using namespace std;

CSockSend::CSockSend()
{
    m_sockfd = -1;
    pthread_mutex_init(&mutex,NULL);
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
    pthread_mutex_lock(&mutex); 
    if(m_sockfd < 0)
    {
        m_sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    }
    pthread_mutex_unlock(&mutex); 
    return m_sockfd;
}
