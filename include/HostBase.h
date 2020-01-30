#ifndef __HOST_BASE_H__
#define __HOST_BASE_H__

#include <iostream> // std, int16_t

class CHostBase
{
public:
    static CHostBase* instance();
    uint16_t getHostMTU(std::string nic);
    std::string getHostMAC(std::string nic);
    std::string getHostIP(std::string nic);
private:
    CHostBase();
    ~CHostBase();
};
#endif
