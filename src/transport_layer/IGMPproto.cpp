#include "IGMPproto.h"
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <list>
#include "Print.h"
#include "SockSend.h"

using namespace std;

CIGMPproto::CIGMPproto()
{
    m_sockfd = -1;
    m_networkLayer = new CIPproto();
    m_version = "";
    m_multiGroupAddr_query.push_back("224.0.0.1");//本地子网的所有系统，igmp查询指令中的目的ip地址
}

CIGMPproto::~CIGMPproto()
{
    delete m_networkLayer;
    m_networkLayer = NULL;
    m_multiGroupAddr_query.clear();
    m_multiGroupAddr_report.clear();
}

CIGMPproto* CIGMPproto::instance()
{
    static CIGMPproto* ptr = new CIGMPproto();
    return ptr;
}

int8_t CIGMPproto::init(string version)
{
    m_sockfd = CSockSend::instance()->createSocket();
    if(m_sockfd == -1)
    {
        ERRORNO("create socket failed\n");
        return 0;
    }
    m_version = version;
    m_networkLayer->refreshHostParam();

    return 1;
}

int16_t CIGMPproto::joinMultiGroup(string multiIP)
{
    int16_t res = -1;
    
    if(m_version == "v2")
    {
        INFO("joinMultiGroup: %s\n", multiIP.c_str());
        res = buildDatagram_v2(IGMP_TYPE_JOIN, 0, multiIP);
    }
    if(res > 0)
    {
        list<string>::iterator it;
        for(it = m_multiGroupAddr_report.begin(); it != m_multiGroupAddr_report.end(); it++)
        {
            if(*it == multiIP)
            {
                WARN("multiGroupAddr:%s has existed\n", multiIP.c_str());
                return res;
            }
        }
        m_multiGroupAddr_report.push_back(multiIP);
    }
    return res;
}

int16_t CIGMPproto::leaveMultiGroup(string multiIP)
{
    int16_t res = -1;
    if(m_version == "v2")
    {
        INFO("leaveMultiGroup: %s\n", multiIP.c_str());
        res = buildDatagram_v2(IGMP_TYPE_LEAVE, 0, multiIP);
    }
    if(res > 0)
    {
        uint8_t addrExist = 0;
        list<string>::iterator it;
        for(it = m_multiGroupAddr_report.begin(); it != m_multiGroupAddr_report.end(); it++)
        {
            if(*it == multiIP)
            {
                addrExist = 1;
                break;
            }
        }
        if(addrExist == 0)
        {
            WARN("multiGroupAddr:%s does not exist\n", multiIP.c_str());
            return res;
        }
        m_multiGroupAddr_report.erase(it);
    }
    return res;
}

int16_t CIGMPproto::buildDatagram_v2(uint8_t type, uint8_t rspTime, string multiIP)
{
    int16_t res = -1;
    uint32_t ipAddr_net = inet_addr(multiIP.c_str());
    uint8_t ipAry[4] = {0}; 
    uint32_t ipAddr_host = 0;
    for(uint8_t i = 0; i < 4; i++)
    {
        ipAry[i] = (ipAddr_net>>(i*8)) & 0xff;
        ipAddr_host |= ipAry[i]<<((3-i)*8);
    }
    ipAddr_host &= 0x007fffff;
    uint64_t macAddr_host = 0x01005e000000L | ipAddr_host;
    struct ether_addr macAddr_ary;
    for(uint8_t i = 0; i < 6; i++)
    {
        macAddr_ary.ether_addr_octet[i] = macAddr_host >> ((5-i)*8) & 0xff;
    }
    string dst_mac = ether_ntoa(&macAddr_ary);
    INFO("dst ip: %s\n", multiIP.c_str());
    INFO("dst mac: %s\n", dst_mac.c_str());
    
    m_networkLayer->setDstMAC(dst_mac);
    m_networkLayer->setDstIP(multiIP);

    struct igmp_v2 igmpPkt;
    uint16_t igmpPkt_len = sizeof(igmpPkt);
    igmpPkt.type = type; // v2的报告报文
    igmpPkt.maxRspTime = rspTime; // 最大响应时间
    igmpPkt.chksum = 0;
    igmpPkt.group_ip = inet_addr(multiIP.c_str());
    uint16_t igmp_sum = htons(checkSum((uint16_t*)&igmpPkt, igmpPkt_len/2));
    igmpPkt.chksum = igmp_sum;
    uint8_t* igmp_packet = new uint8_t[igmpPkt_len];
    memcpy(igmp_packet, &igmpPkt, igmpPkt_len);
    res = m_networkLayer->sendData(m_sockfd, igmp_packet, igmpPkt_len, 0, IGMP_PROTO_ID);
    delete igmp_packet;

    return res;
}

uint8_t CIGMPproto::isMultiGroupAddr(string ip)
{
    uint8_t addrExist = 0;
    list<string>::iterator it;
    for(it = m_multiGroupAddr_query.begin(); it != m_multiGroupAddr_query.end(); it++)
    {
        if(*it == ip)
        {
            addrExist = 1;
            break;
        }
    }
    for(it = m_multiGroupAddr_report.begin(); it != m_multiGroupAddr_report.end(); it++)
    {
        if(*it == ip)
        {
            addrExist = 1;
            break;
        }
    }
    return addrExist;
}

list<string> CIGMPproto::getMultiGroupAddrList()
{
    return m_multiGroupAddr_report;
}
