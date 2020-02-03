#include <net/if.h> // struct ifreq
#include <sys/ioctl.h> // ioctl()
#include <netinet/ether.h> // ether_ntoa()
#include <arpa/inet.h> // inet_ntoa()
#include "Print.h"
#include "SockTemp.h"
#include "SockRecv.h"
#include "NetProtocolBase.h"
#include "HostBase.h"

using namespace std;

CHostBase::CHostBase()
{
    m_mtu = 0;
    m_mac = "";
    m_ip = "";
    m_nic = "";
}

CHostBase::~CHostBase()
{

}

CHostBase* CHostBase::instance()
{
    static CHostBase* ptr = new CHostBase();
    return ptr;
}

void CHostBase::init(string nic)
{
    if(nic == "")
    {
        ERROR("Please set a NIC\n");
        return;
    }
    m_nic = nic;
    getHostMTU(nic);
    getHostMAC(nic);
    getHostIP(nic);

    if(CSockRecv::instance()->createRecv(nic) == 0)
    {
        ERROR("start receive thread failed\n");
        return;
    }
    // 初始化的时候添加arp协议，可以接收arp请求数据包
    // 还需要添加igmp协议，用来应答igmp请求
    CSockRecv::instance()->addProtocol(ARP_PROTO_ID, 4*1024);
}

uint16_t CHostBase::getHostMTU(string nic)
{   
    if(nic != "")
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
        m_mtu = ifr_localhost.ifr_mtu;
        INFO("host mtu: %d\n", m_mtu);
    }

    return m_mtu;
}

string CHostBase::getHostMAC(string nic)
{
    if(nic != "")
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
        m_mac = ether_ntoa((struct ether_addr*)ifr_localhost.ifr_hwaddr.sa_data);
        INFO("host mac: %s\n", m_mac.c_str());
    }

    return m_mac;
}

string CHostBase::getHostIP(string nic)
{
    if(nic != "")
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
        m_ip = inet_ntoa(sin->sin_addr);
        INFO("host ip: %s\n", m_ip);
    }

    return m_ip;
}

string CHostBase::getHostNIC()
{
    return m_nic;
}
