#ifndef __UDP_PROTO_H__
#define __UDP_PROTO_H__

#include <iostream> // std
#include "IPproto.h"
#include "NetProtocolBase.h"

// udp接收缓存默认4MB空间
// 这4MB不是连续地址空间，不用担心申请不到内存
// 这4MB也不是一次性分配好，而是每次接收到数据都会new出一块内存，所有内存块总量不超过4MB
#define DEFAULT_RECV_BUF 4*1024*1024 

class CUDPproto : public CNetProtocolBase
{
public:
    CUDPproto();
    ~CUDPproto();
    int8_t connectToHost(uint16_t port, uint32_t memSize = DEFAULT_RECV_BUF);
    std::string getNIC();
    int8_t connectToRemote(std::string ip, uint16_t port);
    int16_t sendData(std::string msg);
    void closePort();
    // timeout设置超时时间，单位ms，负数为阻塞模式，０为非阻塞模式
    int16_t receiveData(uint8_t* &buf, int32_t timeout = 0);
private:
    uint8_t udpPacketCheck(struct udphdr* udpHdr, uint32_t ip_src, uint32_t ip_dst);
private:
    int16_t m_sockfd;
    std::string m_dst_mac;
    std::string m_dst_ip;
    uint16_t m_dst_port;
    uint16_t m_src_port;
    uint16_t m_pkt_id;
    CIPproto* m_networkLayer;
};

#endif
