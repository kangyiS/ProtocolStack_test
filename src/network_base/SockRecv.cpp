#include <list> //list
#include <unistd.h> // close()
#include <string.h> // strlen(), memset()
#include <net/if.h>
#include <sys/ioctl.h> // ioctl()
#include <linux/if_packet.h> // struct sockaddr_ll
#include <netinet/ip.h>
#include <netinet/ether.h> // struct ether_header
#include <arpa/inet.h>
#include <linux/udp.h> // struct udphdr
#include "ETHproto.h"
#include "SockRecv.h"
#include "Print.h"
#include "HostBase.h"
#include "NetProtocolBase.h"

using namespace std;

CSockRecv::CSockRecv()
{
    m_tid = 0;
    m_sockfd = -1;
    m_host_ip = "";
    m_host_mac = "";
}

CSockRecv::~CSockRecv()
{
    m_portMap.clear();
    m_idPortMap.clear();
}

CSockRecv* CSockRecv::instance()
{
    static CSockRecv* ptr = new CSockRecv();
    return ptr;
}

uint8_t CSockRecv::createRecv(string nic)
{
    CGuard guard(mutex_createRecv);
    if(m_tid != 0)
    {
        WARN("receive thread exists, return\n"); 
        return 1;
    }
    m_sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(m_sockfd == -1)
    {
        ERRORNO("create receive socket failed\n");
        return 0;
    }
    if(listenNIC(nic) == 0)
    {
        close(m_sockfd);
        ERROR("listen NIC failed\n");
        return 0;
    }
    m_host_ip = CHostBase::instance()->getHostIP(nic);
    m_host_mac = CHostBase::instance()->getHostMAC(nic);
    pthread_create(&m_tid, NULL, recvThread, this);//　线程放在最后建立
    INFO("receive thread is built\n");

    return 1;
}

void* CSockRecv::recvThread(void* param)
{
    CSockRecv* parent = (CSockRecv*)param;
    uint8_t buf[ETHER_FRAME_MAX] = {0};

    while(1)
    {
        int16_t res = recvfrom(parent->m_sockfd, buf, sizeof(buf), 0, NULL, NULL);
        if(res > 0)
        {
            parent->parseData(buf, res);
        }
    }
}

void CSockRecv::pushBufferByPort(uint16_t port, uint8_t* buf, uint16_t len)
{
    CGuard guard(mutex_pushBufferByPort);
    std::map<uint16_t, CRecvBuf*>::iterator it;
    it = m_portMap.find(port);
    if(it == m_portMap.end())
    {
        return;
    }
    it->second->pushBack(buf, len);
}

void CSockRecv::pushBufferByProto(uint16_t proto, uint8_t* buf, uint16_t len)
{
    CGuard guard(mutex_pushBufferByProto);
    std::map<uint16_t, CRecvBuf*>::iterator it;
    it = m_protocolMap.find(proto);
    if(it == m_protocolMap.end())
    {
        WARN("protocol 0x%x is not listened\n", proto);
        return;
    }
    it->second->pushBack(buf, len);
}

uint8_t CSockRecv::dstIsHostMac(string mac)
{
    if((mac == m_host_mac)||(mac == "ff:ff:ff:ff:ff:ff"))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t CSockRecv::dstIsHostIP(string ip)
{
    if((ip == m_host_ip)||(ip == "255.255.255.255"))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint8_t CSockRecv::parseData(uint8_t* buf, uint16_t len)
{
    struct ether_header* ethHeader;
    struct ip* ipHeader;
    struct udphdr* udpHeader;
    uint8_t ethHdrLen = sizeof(struct ether_header);
    uint8_t udpHdrLen = sizeof(struct udphdr);

    ethHeader = (struct ether_header*)buf;
    string eth_dst = ether_ntoa((struct ether_addr*)ethHeader->ether_dhost);
    string eth_src = ether_ntoa((struct ether_addr*)ethHeader->ether_shost);
    if(!dstIsHostMac(eth_dst))
    {
        INFO("dst mac is not Host\n");
        return 0;
    }
    if(ntohs(ethHeader->ether_type) == 0x0800) // ip
    {
        ipHeader = (struct ip*)(buf+ethHdrLen);
        string ip_src = inet_ntoa(ipHeader->ip_src);
        string ip_dst = inet_ntoa(ipHeader->ip_dst);
        if(ip_src == m_host_ip)  // 本机发出去的包不用解析
        {
            INFO("this packet is from Host\n");
            return 0;
        }
        if(!dstIsHostIP(ip_dst))
        {
            INFO("dst ip is not Host\n");
            return 0;
        }
        INFO("ip: %s -> %s\n", ip_src.c_str(), ip_dst.c_str());
        uint16_t ipMF = (ntohs(ipHeader->ip_off) & 0x2000) > 0 ? 1 : 0;
        uint16_t ipID = ntohs(ipHeader->ip_id);
        if(ipHeader->ip_p == UDP_PROTO_ID) // udp
        {
            udpHeader = (struct udphdr*)(buf+ethHdrLen+ipHeader->ip_hl*4);
            uint16_t port_src = ntohs(udpHeader->source);
            uint16_t port_dst = ntohs(udpHeader->dest);
            map<uint16_t, uint16_t>::iterator it = m_idPortMap.find(ipID);
            // 当前这个包不是最后一个包
            if(ipMF == 1)
            {
                // 当前包的id没有记录，则说明这是第一个包，要记录下来
                if(it == m_idPortMap.end())
                {
                    m_idPortMap.insert(make_pair(ipID, port_dst));
                    INFO("port: %d -> %d\n", port_src, port_dst);
                }
                else//　如果有记录，则在记录中找到这个id对应的port，那才是真正的目的端口
                {
                    port_dst = it->second;
                }
            }
            else// 这个包是最后一个包
            {
                // 如果能在map中找到id，说明这个包是某一个完整包的最后一个包
                if(it != m_idPortMap.end())
                {
                    port_dst = it->second;
                    m_idPortMap.erase(it);
                }
            }
            /*for(uint16_t i=0; i<udpDataLen; i++)
            {
                printf("0x%.2x ", udpData[i]);
            }
            printf("\n");*/
            pushBufferByPort(port_dst, buf, len);
        }
    }
    else if(ntohs(ethHeader->ether_type) == ARP_PROTO_ID) //　arp协议
    {
        if(eth_dst == "ff:ff:ff:ff:ff:ff")//　暂时只接收arp应答协议
        {
            return 0;
        }
        /*INFO("received arp packet\n");
        for(uint16_t i=0; i<len; i++)
        {
            printf("0x%.2x ", *(buf+i));
        }
        printf("\n");*/
        pushBufferByProto(ARP_PROTO_ID, buf, len);
    }
    return 1;
}

void CSockRecv::closePort(uint16_t port)
{
    CGuard guard(mutex_closePort);
    std::map<uint16_t, CRecvBuf*>::iterator it;
    it = m_portMap.find(port);
    if(it == m_portMap.end())
    {
        WARN("the port %d dose not exist, cannot be closed\n", port);
        return;
    }
    m_portMap.erase(it);
    INFO("port %d is closed successfully\n", port);
}

void CSockRecv::addPort(uint16_t port, uint32_t memSize)
{
    CGuard guard(mutex_addPort);
    std::map<uint16_t, CRecvBuf*>::iterator it;
    it = m_portMap.find(port);
    if(it != m_portMap.end())
    {
        WARN("the port %d exists\n", port);
        return;
    }
    m_portMap.insert(make_pair(port, new CRecvBuf(memSize, port)));
    INFO("add port %d succeed\n", port);
}

void CSockRecv::addProtocol(uint16_t proto, uint32_t memSize)
{
    CGuard guard(mutex_addProtocol);
    std::map<uint16_t, CRecvBuf*>::iterator it;
    it = m_protocolMap.find(proto);
    if(it != m_protocolMap.end())
    {
        WARN("the protocol 0x%x exists\n", proto);
        return;
    }
    m_protocolMap.insert(make_pair(proto, new CRecvBuf(memSize, proto)));
    INFO("add protocol 0x%x successfully\n", proto);
}

uint8_t CSockRecv::listenNIC(string nic)
{
    struct sockaddr_ll sll;  
    struct ifreq ifstruct;

    memset(&sll, 0, sizeof(struct sockaddr_ll));  
    strncpy(ifstruct.ifr_name, nic.c_str(), sizeof(ifstruct.ifr_name));

    // 获取网卡接口信息，网卡索引号，每一个网卡名对应一个编号
    if(ioctl(m_sockfd, SIOCGIFINDEX, &ifstruct) == -1)
    {
        ERRORNO("get NIC number failed\n");
        return 0;
    }

    // 在监听时，只需设置下面三个参数就可以
    sll.sll_family   = PF_PACKET;// 协议族，监听底层协议，这个要和套接字类型对应  
    sll.sll_ifindex  = ifstruct.ifr_ifindex; //　指定监听的网卡，写０，监听所有网卡 
    sll.sll_protocol = htons(ETH_P_ALL);// 监听数据链路层所有协议，也可以单独选ip,arp,icmp等协议，这里要用网络字节序  

    // 设置混杂模式，用ifconfig可以看到对应网卡上多了PROMISC参数
    if(ioctl(m_sockfd, SIOCGIFFLAGS, &ifstruct) == -1)
    {
        ERRORNO("get NIC flags failed\n");
        return 0;
    }
    ifstruct.ifr_flags |= IFF_PROMISC;
    if(ioctl(m_sockfd, SIOCSIFFLAGS, &ifstruct) == -1)
    {
        ERRORNO("set NIC PROMISC failed\n");
        return 0;
    }

    if(bind(m_sockfd, (struct sockaddr *)&sll, sizeof(struct sockaddr_ll)) == -1)
    {
        ERRORNO("bind failed\n");
        return 0;
    }

    return 1;
}

int16_t CSockRecv::popBuffer(uint8_t* &buf, uint16_t code, uint8_t type)
{
    CGuard guard(mutex_popBuffer);
    std::map<uint16_t, CRecvBuf*>::iterator it;
    if(type == BUFF_PORT) // port
    {
        it = m_portMap.find(code);
        if(it == m_portMap.end())
        {
            WARN("port %d is not listened\n", code); 
            return ERR_NO_PORT;
        }
    }
    else if(type == BUFF_PROTOCOL) // protocol
    {
        it = m_protocolMap.find(code);
        if(it == m_protocolMap.end())
        {
            WARN("protocol %d is not listened\n", code); 
            return ERR_NO_PROTOCOL;
        }
    }
    else
    {
        ERROR("type %d is not exist\n", type);
        return ERR_NO_TYPE;
    }
    uint16_t len = it->second->popFront(buf);

    return len;
}

// ************************ class: CRecvBuf *********************** //
CRecvBuf::CRecvBuf(uint32_t memSize, uint16_t port)
{
    m_bufSize = 0;
    m_bufCount = 0;
    m_memSize = memSize;
    m_port = port;
}

CRecvBuf::~CRecvBuf()
{
    m_bufMap.clear();
}

uint8_t CRecvBuf::pushBack(uint8_t* buf, uint16_t len)
{
    CGuard guard(mutex_pushBack);
    if((m_bufSize + len) > m_memSize)
    {
        WARN("port %d recv buffer, out of memory\n", m_port);
        return 0;
    }

    uint8_t* buf_temp = new uint8_t[len];
    memcpy(buf_temp, buf, len);
    m_bufMap.insert(make_pair(len, buf_temp));
    /*printf("CRecvBuf::pushBack, ");
    for(uint16_t i = 0; i < len; i++)
    {
        printf("0x%.2x ", buf_temp[i]);
    }
    printf("\n");*/

    m_bufSize += len;
    m_bufCount++;
    INFO("port: %d, m_bufMap size: %d, memory used: %d\n", m_port, m_bufCount, m_bufSize);

    return 1;
}

int16_t CRecvBuf::popFront(uint8_t* &buf)
{
    CGuard guard(mutex_popFront);
    if(m_bufMap.size() == 0)
    {
        buf = NULL;
        return 0;
    }
    map<uint16_t, uint8_t*>::iterator it = m_bufMap.begin();
    uint16_t len = it->first;
    buf = it->second;
    m_bufMap.erase(it);
    m_bufSize -= len;
    m_bufCount--;

    return len;
}
