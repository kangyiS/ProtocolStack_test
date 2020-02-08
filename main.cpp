#include "UDPproto.h"
#include "IGMPproto.h"
#include "ARPproto.h"
#include "HostBase.h"

using namespace std;

#define TEST 2

CUDPproto mysock;
string send_msg;
uint16_t dataLen = 100;

const string g_host_nic = "wlx00e02c3112e7";
const uint16_t g_host_port = 1234;
const string g_dst_ip = "192.168.3.20";
const uint16_t g_dst_port = 1234;

int main()
{
    for(uint16_t i = 0; i < dataLen; i++)
    {
        send_msg.append("a");
    }
    CHostBase::instance()->init(g_host_nic);
    CARPproto::instance()->init();
    CIGMPproto::instance()->init("v2");
#if TEST == 1 //udp send
    mysock.connectToHost(g_host_port);
    mysock.connectToRemote(g_dst_ip, g_dst_port);
    mysock.sendData(send_msg);
#elif TEST == 2 //udp receive
    mysock.connectToHost(g_host_port);
    uint8_t* buf = NULL;
    while(1)
    {
        int16_t len = mysock.receiveData(buf, 2000);
        if(len > 0)
        {
            printf("main, len = %d\n", len);
            for(uint16_t i = 0; i < len; i++)
                printf("%d ", *(buf+i));
            printf("\n");
        }
    }
    mysock.closePort();
#elif TEST == 3 //igmp v2 join and response
    CIGMPproto::instance()->joinMultiGroup("224.132.1.1");
    CIGMPproto::instance()->joinMultiGroup("224.132.1.2");
    CIGMPproto::instance()->joinMultiGroup("224.132.1.3");
    CIGMPproto::instance()->joinMultiGroup("224.132.1.4");
    while(1);
#endif
    return 0;
}
