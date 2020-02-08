#include <netinet/ether.h>
#include <arpa/inet.h>
#include <linux/udp.h>
#include "SockRecv.h"
#include "SockSend.h"
#include "Print.h"
#include "UDPproto.h"
#include "ARPproto.h"

using namespace std;

CUDPproto::CUDPproto()
{
    m_sockfd = -1;
    m_dst_mac = "";
    m_dst_ip = "";
    m_dst_port = 0;
    m_src_port = 0;
    m_pkt_id = 0;
    m_networkLayer = new CIPproto();
}

CUDPproto::~CUDPproto()
{
    delete m_networkLayer;
    m_networkLayer = NULL;
}

int8_t CUDPproto::connectToHost(uint16_t port, uint32_t memSize)
{
    m_sockfd = CSockSend::instance()->createSocket();
    if(m_sockfd == -1)
    {
        ERRORNO("create socket failed\n");
        return 0;
    }
    m_networkLayer->refreshHostParam();
    // 本地端口
    m_src_port = port;
    CSockRecv::instance()->addPort(port, memSize);
    INFO("src port: %d\n",m_src_port);

    return 1;
}

void CUDPproto::closePort()
{
    CSockRecv::instance()->closePort(m_src_port);
    m_src_port = 0;
}

string CUDPproto::getNIC()
{
    return m_networkLayer->getNIC();
}

int8_t CUDPproto::connectToRemote(string dst_ip, uint16_t dst_port)
{
    int16_t res = CARPproto::instance()->sendData(ARP_TYPE_REQUEST, dst_ip);
    if(res == -1)
    {
        WARN("send arp request failed\n");
        return 0;
    }
    string dst_mac = CARPproto::instance()->recvResponse();
    if(dst_mac == "")
    {
        WARN("receive arp response failed\n");
        return 0;
    }
    INFO("dst ip: %s\n", dst_ip.c_str());
    INFO("dst mac: %s\n", dst_mac.c_str());
    m_networkLayer->setDstMAC(dst_mac);
    m_networkLayer->setDstIP(dst_ip);
    m_dst_port = dst_port;
}

int16_t CUDPproto::sendData(string msg)
{
    uint16_t dataLen = strlen(msg.c_str());
    if(dataLen % 2 == 1)
    {
        msg.append("\0");
        dataLen++;
    }
    INFO("data length: %d\n", dataLen);

    struct udphdr udpHeader;
    struct psdhdr
    {
        struct in_addr src_ip;
        struct in_addr dst_ip;
        uint8_t padding;
        uint8_t proto;
        uint16_t len;
    }psdHeader;
    uint8_t udpHdrLen = sizeof(udpHeader);
    uint8_t psdHdrLen = sizeof(psdHeader);

    string src_ip = m_networkLayer->getHostIP();
    if(src_ip == "")
    {
        ERROR("get host ip failed\n");
        return 0;
    }
    string dst_ip = m_networkLayer->getDstIP();
    if(dst_ip == "")
    {
        ERROR("get dst ip failed\n");
        return 0;
    }
    uint32_t l_ip_src = inet_addr(src_ip.c_str());
    uint32_t l_ip_dst = inet_addr(dst_ip.c_str());
    // udp头
    udpHeader.source = htons(m_src_port);// 2 bytes
    udpHeader.dest = htons(m_dst_port);// 2 bytes
    udpHeader.len = htons(udpHdrLen+dataLen);// 2 bytes
    udpHeader.check = 0;// 2 bytes

    // 伪首部  udp
    memcpy(&psdHeader.src_ip, &l_ip_src, 4);// 4 bytes
    memcpy(&psdHeader.dst_ip, &l_ip_dst, 4);// 4 bytes
    psdHeader.padding = 0;// 1 byte
    psdHeader.proto = UDP_PROTO_ID;// 1 byte
    psdHeader.len = htons(udpHdrLen+dataLen);// 2 bytes

    // udp数据校验
    uint8_t* udp_check = new uint8_t[psdHdrLen+udpHdrLen+dataLen];
    memcpy(udp_check, &psdHeader, psdHdrLen);
    memcpy(udp_check+psdHdrLen, &udpHeader, udpHdrLen);
    memcpy(udp_check+psdHdrLen+udpHdrLen, msg.c_str(), dataLen);
    uint16_t udp_sum = htons(checkSum((uint16_t*)udp_check, (psdHdrLen+udpHdrLen+dataLen)/2));
    udpHeader.check = udp_sum;

    uint8_t* udp_packet = new uint8_t[udpHdrLen+dataLen];
    memset(udp_packet, 0, udpHdrLen+dataLen);
    memcpy(udp_packet, &udpHeader, udpHdrLen);
    memcpy(udp_packet+udpHdrLen, msg.c_str(), dataLen);

    int16_t res = m_networkLayer->sendData(m_sockfd, udp_packet, udpHdrLen+dataLen, 0, UDP_PROTO_ID);
    delete udp_check;
    delete udp_packet;
    return res;
}

// 没有数据，返回0，查不到端口，返回-1，超时，返回-2，ip头校验失败，返回-3，ip包偏移量（ip包顺序）错误，返回-4，udp校验失败，返回-5
int16_t CUDPproto::receiveData(uint8_t* &buf, int32_t timeout)
{
    uint8_t* buf_udp = NULL;
    uint32_t ip_src = 0;
    uint32_t ip_dst = 0;
    int16_t len = m_networkLayer->receiveData(buf_udp, ip_src, ip_dst, m_src_port, timeout);
    if(len <= 0)
    {
        ERROR("receive data failed, error no: %d\n", len);
        buf = NULL;
        return len;
    }
    INFO("udp packet length: %d\n", len);
    struct udphdr* udpHeader = (struct udphdr*)buf_udp;
    uint8_t udpHeader_len = sizeof(struct udphdr);
    if(udpPacketCheck(udpHeader, ip_src, ip_dst) == 0)
    {
        buf = NULL;
        WARN("udp packet check failed\n");
        return ERR_UDP_HEAD_CHECK;
    }
    buf = buf_udp + udpHeader_len;
    return len - udpHeader_len;
}

uint8_t CUDPproto::udpPacketCheck(struct udphdr* udpHdr, uint32_t ip_src, uint32_t ip_dst)
{
    struct psdhdr
    {
        struct in_addr src_ip;
        struct in_addr dst_ip;
        uint8_t padding;
        uint8_t proto;
        uint16_t len;
    }psdHeader;
    
    uint8_t psdHdrLen = sizeof(psdHeader);
    // 伪首部  udp
    memcpy(&psdHeader.src_ip, &ip_src, 4);// 4 bytes
    memcpy(&psdHeader.dst_ip, &ip_dst, 4);// 4 bytes
    psdHeader.padding = 0;// 1 byte
    psdHeader.proto = 0x11;// 1 byte
    psdHeader.len = udpHdr->len;// 2 bytes

    uint16_t udpLen = ntohs(udpHdr->len);
    if(udpLen % 2 == 1)// 要注意这里长度不能为奇数
    {
        udpLen++;
    }
    uint16_t udp_sum = udpHdr->check;
    udpHdr->check = 0;
    uint8_t* udp_packet = new uint8_t[psdHdrLen+udpLen];
    memset(udp_packet, 0, psdHdrLen+udpLen);
    memcpy(udp_packet, &psdHeader, psdHdrLen);
    memcpy(udp_packet+psdHdrLen, udpHdr, udpLen);
    uint16_t check_udp_sum = checkSum((uint16_t*)udp_packet, (psdHdrLen+udpLen)/2);
    delete udp_packet;
    return udp_sum == htons(check_udp_sum);
}

