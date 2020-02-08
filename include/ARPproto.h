#ifndef __ARP_PROTO_H__
#define __ARP_PROTO_H__

#include <iostream> // std
#include "ETHproto.h"
#include "NetProtocolBase.h"

#define ARP_TYPE_REQUEST 1
#define ARP_TYPE_RESPONSE 2

class CARPproto : public CNetProtocolBase
{
public:
    void init();
    static CARPproto* instance();
    int16_t sendData(uint8_t type, std::string dst_ip, std::string dst_mac = "ff:ff:ff:ff:ff:ff");
    std::string recvResponse();
private:
    CARPproto();
    ~CARPproto();
private:
    int16_t m_sockfd;
    std::string m_src_ip;
    CETHproto* m_dataLinkLayer;
};

#endif
