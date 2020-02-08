#include <map>
#include <linux/if_packet.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <arpa/inet.h> // inet_addr()
#include "SockTemp.h"
#include "SockSend.h"
#include "Print.h"
#include "IPproto.h"
#include "HostBase.h"

using namespace std;

CIPproto::CIPproto()
{
    m_src_ip = "";
    m_dst_ip = "";
    // 用随机数初始化数据包id
    srand((int16_t)time(0));
    m_pkt_id = rand()%10000;
    m_dataLinkLayer = new CETHproto();
}

CIPproto::~CIPproto()
{
    delete m_dataLinkLayer;
    m_dataLinkLayer = NULL;
}

string CIPproto::getHostIP(uint8_t refresh)
{
    if(refresh == 1)
    {
        string host_nic  = m_dataLinkLayer->getNIC();
        if(host_nic == "")
        {
            ERROR("get NIC failed, please set NIC first\n");
            return "";
        }
	m_src_ip = CHostBase::instance()->getHostIP(host_nic);
    }
    return m_src_ip;
}

void CIPproto::refreshHostParam()
{
    m_src_ip = CHostBase::instance()->getHostIP();
    m_dataLinkLayer->refreshHostParam();
}

void CIPproto::setNIC(string nic)
{
    m_dataLinkLayer->setNIC(nic);
}

string CIPproto::getNIC()
{
    return m_dataLinkLayer->getNIC();
}

string CIPproto::getHostMAC(uint8_t refresh)
{
    return m_dataLinkLayer->getHostMAC(refresh);
}

uint16_t CIPproto::getMTU(uint8_t refresh)
{
    return m_dataLinkLayer->getMTU(refresh);
}

void CIPproto::setDstMAC(string dst_mac)
{
    m_dataLinkLayer->setDstMAC(dst_mac);
}

void CIPproto::setDstIP(string dst_ip)
{
    m_dst_ip = dst_ip;
}

string CIPproto::getDstIP()
{
    return m_dst_ip;
}

int16_t CIPproto::sendData(int16_t sock, void* data, uint16_t dataLen, int16_t flags, uint16_t proto)
{
    struct ether_header ethHeader;
    struct ip_hdr
    {
#if BYTE_ORDER == LITTLE_ENDIAN
        uint8_t ip_hl:4,
                ip_v:4;
#endif
#if BYTE_ORDER == BIG_ENDIAN
        uint8_t ip_v:4,
                ip_hl:4;
#endif
        uint8_t ip_tos;
        uint16_t ip_len;
        uint16_t ip_id;
        uint16_t ip_off;
        uint8_t ip_ttl;
        uint8_t ip_p;
        uint16_t ip_sum;
        struct in_addr ip_src, ip_dst;
        uint8_t ip_optType;
        uint8_t ip_optLen;
        uint16_t ip_optRAV;
    }ipHeader;
    uint8_t ethHdrLen = sizeof(ethHeader);

    uint32_t l_ip_src = inet_addr(m_src_ip.c_str());
    uint32_t l_ip_dst = inet_addr(m_dst_ip.c_str());

    // ip头
    ipHeader.ip_v = 4;// 0.5 byte
    if(proto == IGMP_PROTO_ID)
    {
        ipHeader.ip_hl = 6;// 0.5 byte
        ipHeader.ip_ttl = 1;// 1 byte
        ipHeader.ip_optType = 0x94;
        ipHeader.ip_optLen = 4;
        ipHeader.ip_optRAV = 0;
    }
    else
    {
        ipHeader.ip_hl = 5;// 0.5 byte
        ipHeader.ip_ttl = 64;// 1 byte
    }
    ipHeader.ip_tos = 0;// 1 byte
    ipHeader.ip_len = 0;// 2 byte
    ipHeader.ip_id = htons(m_pkt_id);// 2 bytes
    ipHeader.ip_off = 0;// 2 bytes
    ipHeader.ip_p = proto; // 1 byte
    ipHeader.ip_sum = 0;// 2 bytes     
    memcpy(&ipHeader.ip_src, &l_ip_src, 4);// 4 bytes
    memcpy(&ipHeader.ip_dst, &l_ip_dst, 4);// 4 bytes

    uint8_t ipHdrLen = ipHeader.ip_hl * 4;
    uint32_t pktLen = ipHdrLen+dataLen;
    uint16_t mtu = getMTU();
    uint16_t mtu_by_eth = 0;
    if(mtu > ETHER_FRAME_MAX-ethHdrLen)
    {
        mtu_by_eth = ETHER_FRAME_MAX-ethHdrLen;
    }
    else
    {
        mtu_by_eth = mtu;
    }

    uint16_t mtuReal = mtu_by_eth - (mtu_by_eth-ipHdrLen) % 8;// 如果分片，每一个包的数据量都是８的倍数
    INFO("real mtu: %d\n", mtuReal);
    uint16_t pkt_num = dataLen / (mtuReal-ipHdrLen) + 1;// 发送的包数量
    INFO("packet number: %d\n", pkt_num);
    
    int16_t res = -1;
    for(uint16_t i=1; i<=pkt_num; i++)
    {
        uint16_t ipData_len = 0;//　数据报中ip头之后的数据长度（不含ip头）
        uint16_t ip_off = 0;
        if(i == pkt_num)// 最后一包
        {
            ipData_len = dataLen % (mtuReal-ipHdrLen);
            ip_off &= 0xdfff;// MF = 0
        }
        else
        {
            ipData_len = mtuReal - ipHdrLen;
            ip_off |= 0x2000;// MF = 1
        }
        // 计算ip数据包长度
        ipHeader.ip_len = htons(ipData_len+ipHdrLen);

        // 计算偏移量
        uint16_t offset = (i-1) * (mtuReal-ipHdrLen) >> 3;
        ip_off |= offset;
        //ip_off |= 0x4000; // DF=1
        ipHeader.ip_off = htons(ip_off);
        // ip头校验
        ipHeader.ip_sum = 0;// 这里一定要先置0
        uint16_t ip_sum = checkSum((uint16_t*)&ipHeader, ipHdrLen/2);
        ipHeader.ip_sum = htons(ip_sum);

        uint16_t ipPktLen = ipHdrLen+ipData_len;
        uint8_t* ip_packet = new uint8_t[ipPktLen];
        memset(ip_packet, 0, ipPktLen);
        memcpy(ip_packet, &ipHeader, ipHdrLen);
        memcpy(ip_packet+ipHdrLen, (uint8_t*)data+(offset<<3), ipData_len);

        res = m_dataLinkLayer->sendData(sock, ip_packet, ipPktLen, flags, IP_PROTO_ID);
        delete ip_packet;
        if(res == -1)
        {
            break;
        }
    }
    return res;
}

// 查不到端口，返回-1，超时，返回-2，ip头校验失败，返回-3，ip包偏移量错误，返回-4
int32_t CIPproto::receiveData(uint8_t* &buf, uint32_t& l_ip_src, uint32_t& l_ip_dst, uint16_t port, int32_t timeout)
{
    struct ip* ipHeader;
    uint8_t ipHeader_len = sizeof(struct ip);
    uint8_t* buf_ip = NULL;
    int16_t len = 0;
    int32_t len_sum = 0;
    int32_t offset = -1;
    map<uint8_t*, uint16_t> ip_pkt_map;
    while(1)
    {
        len = m_dataLinkLayer->receiveData(buf_ip, port, CSockRecv::BUFF_PORT, timeout);
        if(len <= 0)
        {
            ERROR("receive data failed, error no: %d\n", len);
            buf = NULL;
            return len;
        }
        ipHeader = (struct ip*)buf_ip;
        if(ipPacketCheck(ipHeader) == 0)
        {
            buf = NULL;
            WARN("ip packet check failed\n");
            return ERR_IP_HEAD_CHECK;
        }
        uint16_t ipOffset = ntohs(ipHeader->ip_off) & 0x1fff;
        uint16_t ipMF = (ntohs(ipHeader->ip_off) & 0x2000) > 0 ? 1 : 0;
        if((int32_t)ipOffset <= offset)//　可以考虑用数据长度和偏移量来校验
        {
            buf = NULL;
            ERROR("ip packet order chaos\n");
            return ERR_IP_PACKET_ORDER;
        }
        offset = ipOffset;
        // 这里保持网络字节序不变，方便udp层进行校验
        memcpy(&l_ip_src, &(ipHeader->ip_src), 4);
        memcpy(&l_ip_dst, &(ipHeader->ip_dst), 4);
        len_sum += (len - ipHeader_len);
        // 剥掉ip头存入map表中，方便之后组udp包
        ip_pkt_map.insert(make_pair(buf_ip+ipHeader_len, len-ipHeader_len));
        INFO("ip packet length: %d, in this time\n", len);
        if(ipMF == 0)
        {
            break;
        }
    }
    uint8_t* ip_pkt = new uint8_t[len_sum];// 这里的内存不能释放，上一层会使用这里的数据，但是有可能会有内存泄漏
    buf = ip_pkt;
    memset(ip_pkt, 0, len_sum);
    map<uint8_t*, uint16_t>::iterator it = ip_pkt_map.begin();
    for(; it != ip_pkt_map.end(); it++)
    {
        memcpy(ip_pkt, it->first, it->second);
        ip_pkt += it->second;
    }
    INFO("ip packet length: %d, in total\n", len_sum+ipHeader_len);
    return len_sum;
}

uint8_t CIPproto::ipPacketCheck(struct ip* header)
{
    uint8_t ipHeaderLen = header->ip_hl * 4;
    uint16_t ip_sum = header->ip_sum;
    header->ip_sum = 0;
    uint8_t* ip_packet = new uint8_t[ipHeaderLen];
    memset(ip_packet, 0, ipHeaderLen);
    memcpy(ip_packet, header, ipHeaderLen);
    uint16_t check_ip_sum = checkSum((uint16_t*)ip_packet, ipHeaderLen/2);
    delete ip_packet;
    return ip_sum == htons(check_ip_sum);
}
