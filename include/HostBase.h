#ifndef __HOST_BASE_H__
#define __HOST_BASE_H__

#include <iostream> // std, int16_t

class CHostBase
{
public:
    static CHostBase* instance();
    void init(std::string nic);
    uint16_t getHostMTU(std::string nic = "");
    std::string getHostMAC(std::string nic = "");
    std::string getHostIP(std::string nic = "");
    std::string getHostNIC();
private:
    CHostBase();
    ~CHostBase();
private:
    std::string m_nic;
    uint16_t m_mtu;
    std::string m_mac;
    std::string m_ip;
};
#endif
