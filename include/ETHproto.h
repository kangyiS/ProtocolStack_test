#ifndef __ETH_PROTO_H__
#define __ETH_PROTO_H__

#include <iostream> // std
#include "NetProtocolBase.h"
#include "SockRecv.h"

#define ETHER_FRAME_MIN 60
#define ETHER_FRAME_MAX 1514
class CETHproto : public CNetProtocolBase
{
public:
    CETHproto();
    ~CETHproto();
	void setNIC(std::string nic);
    std::string getNIC();
    std::string getHostMAC(uint8_t refresh = 0);
    uint16_t getMTU(uint8_t refresh = 0);
    int16_t sendData(int16_t sock, void* data, uint16_t dataLen, int16_t flags, uint16_t proto);
    void setDstMAC(std::string dst_mac);
    int16_t receiveData(uint8_t* &buf, uint16_t code, uint8_t type, int32_t timeout);
private:
    std::string m_nic;
    std::string m_src_mac;
    std::string m_dst_mac;
    uint16_t m_mtu;
};

#endif
