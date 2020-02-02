#include "SockSend.h"
#include <unistd.h> // close()
#include <sys/socket.h> // socket()
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <net/if.h> // if_nametoindex()
#include <linux/if_packet.h> // struct sockaddr_ll
#include <string.h> // memset()

using namespace std;

CSockSend::CSockSend()
{
    m_sockfd = -1;
    m_nic = "";
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

void CSockSend::setNIC(string nic)
{
    CGuard guard(mutex_nic);
    m_nic = nic;
}

string CSockSend::getNIC()
{
    CGuard guard(mutex_nic);
    return m_nic;
}

int16_t CSockSend::sendData(int16_t sock, uint8_t* frame, uint16_t frameLen, int16_t flags)
{
    int16_t res = -1;
    struct sockaddr_ll dst_info;
    memset(&dst_info, 0, sizeof(dst_info));
    dst_info.sll_family = PF_PACKET;
    dst_info.sll_ifindex = if_nametoindex(m_nic.c_str());//返回对应接口名的编号

    if(sock == -1)
    {
        sock = m_sockfd;
    }
    res = sendto(sock, frame, frameLen, flags, (struct sockaddr *)&dst_info, sizeof(dst_info));
    return res;
}
