#include <net/if.h> // struct ifreq
#include <sys/ioctl.h> // ioctl()
#include <netinet/ether.h> // ether_ntoa()
#include <arpa/inet.h> // inet_ntoa()
#include "Print.h"
#include "SockTemp.h"
#include "HostBase.h"

using namespace std;

CHostBase::CHostBase()
{

}

CHostBase::~CHostBase()
{

}

CHostBase* CHostBase::instance()
{
    static CHostBase* ptr = new CHostBase();
    return ptr;
}

uint16_t CHostBase::getHostMTU(string nic)
{   
    struct ifreq ifr_localhost;

    CSockTemp sockTemp;
    int16_t sock_localhost = sockTemp.createSocket(AF_INET, SOCK_STREAM, 0);
    if(sock_localhost == -1)
    {
        ERRORNO("create localhost socket failed\n");
        return 0;
    }

    memset(&ifr_localhost, 0, sizeof(ifr_localhost));
    strncpy(ifr_localhost.ifr_name, nic.c_str(), sizeof(ifr_localhost.ifr_name));
    if(ioctl(sock_localhost, SIOCGIFMTU, &ifr_localhost) == -1)
    {
        ERRORNO("cannot get local mtu\n");
        return 0;
    }
    uint16_t mtu = ifr_localhost.ifr_mtu;

    return mtu;
}

string CHostBase::getHostMAC(string nic)
{
    struct ifreq ifr_localhost;

    CSockTemp sockTemp;
    int16_t sock_localhost = sockTemp.createSocket(AF_INET, SOCK_STREAM, 0);
    if(sock_localhost == -1)
    {
        ERRORNO("create localhost socket failed\n");
        return "";
    }

    memset(&ifr_localhost, 0, sizeof(ifr_localhost));
    strncpy(ifr_localhost.ifr_name, nic.c_str(), sizeof(ifr_localhost.ifr_name));
    if(ioctl(sock_localhost, SIOCGIFHWADDR, &ifr_localhost) == -1)
    {
        ERRORNO("cannot get local mac\n");
        return "";
    }
    string host_mac = ether_ntoa((struct ether_addr*)ifr_localhost.ifr_hwaddr.sa_data);

    return host_mac;
}

string CHostBase::getHostIP(string nic)
{
    struct sockaddr_in *sin;
    struct ifreq ifr_localhost;

    CSockTemp sockTemp;
    int16_t sock_localhost = sockTemp.createSocket(AF_INET, SOCK_STREAM, 0);
    if(sock_localhost == -1)
    {
        ERRORNO("create localhost socket failed\n");
        return "";
    }
    
    memset(&ifr_localhost, 0, sizeof(ifr_localhost));
    strncpy(ifr_localhost.ifr_name, nic.c_str(), sizeof(ifr_localhost.ifr_name));

    if(ioctl(sock_localhost, SIOCGIFADDR, &ifr_localhost) == -1)
    {
        ERRORNO("cannot get local ip\n");
        return "";
    }
    sin = (struct sockaddr_in*)&ifr_localhost.ifr_addr;
    string host_ip = inet_ntoa(sin->sin_addr);

    return host_ip;
}
