#ifndef __NET_PROTOCOL_BASE_H__
#define __NET_PROTOCOL_BASE_H__

#include <iostream> // std,int16_t

#define ARP_PROTO_ID   0x0806
#define IP_PROTO_ID    0x0800
#define IGMP_PROTO_ID  0x02
#define UDP_PROTO_ID   0x11

class CNetProtocolBase
{
public:
    CNetProtocolBase();
    virtual ~CNetProtocolBase();
    virtual int16_t sendData() {return -1;}
    virtual int16_t receiveData() {return -1;}
    uint16_t checkSum(uint16_t* buf, uint16_t len);
};
#endif
