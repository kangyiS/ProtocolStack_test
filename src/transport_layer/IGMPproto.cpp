#include "IGMPproto.h"
#include <arpa/inet.h>
#include <netinet/ether.h>
#include "Print.h"
#include "SockSend.h"

using namespace std;

CIGMPproto::CIGMPproto(string version)
{
    m_sockfd = -1;
    m_src_port = 0;
    m_networkLayer = new CIPproto();
    m_version = version;
}

CIGMPproto::~CIGMPproto()
{
    delete m_networkLayer;
    m_networkLayer = NULL;
}

int8_t CIGMPproto::createSocket()
{
    m_sockfd = CSockSend::instance()->createSocket();
    if(m_sockfd == -1)
    {
        ERRORNO("create socket failed\n");
        return 0;
    }
    return 1;
}

int8_t CIGMPproto::connectToHost(string nic, uint16_t port)
{
    m_networkLayer->setNIC(nic);
    if(!getHostParam())
    {
        return 0;
    }
    // 本地端口
    m_src_port = port;
    INFO("src port: %d\n",m_src_port);

    return 1;
}

uint8_t CIGMPproto::getHostParam()
{
    // 获取本地mtu
    uint16_t mtu = m_networkLayer->getMTU(1);
    if(mtu == 0)
    {
        ERROR("get mtu failed\n");
        return 0;
    }
    INFO("localhost MTU: %d\n", mtu);

    // 获取本地ip
    string src_ip = m_networkLayer->getHostIP(1);
    if(src_ip == "")
    {
        ERROR("get host ip failed\n");
        return 0;
    }
    INFO("src ip: %s\n",src_ip.c_str());

    // 获取本地mac
    string src_mac = m_networkLayer->getHostMAC(1);
    if(src_mac == "")
    {
        ERROR("get host mac failed\n");
        return 0;
    }
    INFO("src mac: %s\n", src_mac.c_str());

    return 1;
}

int16_t CIGMPproto::joinMultiGroup(string multiIP)
{
    int16_t res = -1;
    if(m_version == "v2")
    {
        INFO("joinMultiGroup: %s\n", multiIP.c_str());
        res = buildDatagram_v2(0x16, 0, multiIP);
    }
    return res;
}

int16_t CIGMPproto::leaveMultiGroup(string multiIP)
{
    int16_t res = -1;
    if(m_version == "v2")
    {
        INFO("leaveMultiGroup: %s\n", multiIP.c_str());
        res = buildDatagram_v2(0x17, 0, multiIP);
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

    struct igmp_v2
    {
        uint8_t type;
        uint8_t maxRspTime;
        uint16_t chksum;
        uint32_t group_ip;
    }igmpPkt;
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
