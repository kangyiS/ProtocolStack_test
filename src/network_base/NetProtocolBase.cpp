#include "NetProtocolBase.h"
#include <arpa/inet.h> // htons()

using namespace std;

CNetProtocolBase::CNetProtocolBase()
{

}

CNetProtocolBase::~CNetProtocolBase()
{

}

// 校验一定要用１６位的指针，求和时要把主机字节序转成网络字节序
uint16_t CNetProtocolBase::checkSum(uint16_t* buf, uint16_t len)
{
    uint32_t sum;
    for(sum=0; len>0; len--)
    {
        sum += htons(*buf);
        buf++;
    }
    sum = (sum>>16) + (sum&0xffff);
	sum += (sum>>16);
	return ~sum;
}
