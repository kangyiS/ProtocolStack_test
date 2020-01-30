#ifndef __IGMP_PROTO_H__
#define __IGMP_PROTO_H__

#include <iostream> // std
#include "IPproto.h"
#include "NetProtocolBase.h"

class CIGMPproto : public CNetProtocolBase
{
public:
    CIGMPproto(std::string version);
    ~CIGMPproto();
    int8_t createSocket();
    int16_t joinMultiGroup(std::string multiIP);
    int16_t leaveMultiGroup(std::string multiIP);
    int8_t connectToHost(std::string nic, uint16_t port);
private:
    uint8_t getHostParam();
    int16_t buildDatagram_v2(uint8_t type, uint8_t rspTime, std::string multiIP);
private:
    int16_t m_sockfd;
    uint16_t m_src_port;
    std::string m_version;
    CIPproto* m_networkLayer;
};
#endif
