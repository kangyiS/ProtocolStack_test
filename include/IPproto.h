#ifndef __IP_PROTO_H__
#define __IP_PROTO_H__

#include <iostream> // std
#include "NetProtocolBase.h"
#include "ETHproto.h"

class CIPproto : public CNetProtocolBase
{
public:
    CIPproto();
    ~CIPproto();
    std::string getHostIP(uint8_t refresh = 0);
    void setNIC(std::string nic);
    std::string getNIC();
    std::string getHostMAC(uint8_t refresh = 0);
    uint16_t getMTU(uint8_t refresh = 0);
    int16_t sendArpRequest(std::string dst_ip);
    std::string recvArpResponse();
    void setDstMAC(std::string dst_mac);
    void setDstIP(std::string dst_ip);
    std::string getDstIP();
    int16_t sendData(int16_t sock, void* data, uint16_t dataLen, int16_t flags, uint16_t proto);
    int32_t receiveData(uint8_t* &buf, uint32_t& l_ip_src, uint32_t& l_ip_dst, uint16_t port, int32_t timeout);

private:
    uint8_t ipPacketCheck(struct ip* header);
private:
    std::string m_src_ip;
    std::string m_dst_ip;
    uint16_t m_pkt_id;
    CETHproto* m_dataLinkLayer;
};

#endif
