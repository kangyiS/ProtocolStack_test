#ifndef __IGMP_PROTO_H__
#define __IGMP_PROTO_H__

#include <iostream> // std
#include "IPproto.h"
#include "NetProtocolBase.h"

#define IGMP_TYPE_QUERY 0x11
#define IGMP_TYPE_JOIN  0x16
#define IGMP_TYPE_LEAVE 0x17
class CIGMPproto : public CNetProtocolBase
{
public:
    static CIGMPproto* instance();
    int8_t init(std::string version);
    int16_t joinMultiGroup(std::string multiIP);
    int16_t leaveMultiGroup(std::string multiIP);
    uint8_t isMultiGroupAddr(std::string ip);
    std::list<std::string> getMultiGroupAddrList();
private:
    CIGMPproto();
    ~CIGMPproto();
    int16_t buildDatagram_v2(uint8_t type, uint8_t rspTime, std::string multiIP);
public:
    struct igmp_v2
    {
        uint8_t type;
        uint8_t maxRspTime;
        uint16_t chksum;
        uint32_t group_ip;
    };
private:
    int16_t m_sockfd;
    std::string m_version;
    CIPproto* m_networkLayer;
    std::list<std::string> m_multiGroupAddr_query;
    std::list<std::string> m_multiGroupAddr_report;
};
#endif
