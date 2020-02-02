// arp应答线程刚写好，需要验证
#include "UDPproto.h"
#include "IGMPproto.h"

CUDPproto mysock;
CIGMPproto igmpSock("v2");

using namespace std;

#define TEST 2
string send_msg;
uint16_t dataLen = 100;

int main()
{
    for(uint16_t i = 0; i < dataLen; i++)
    {
        send_msg.append("a");
    }
#if TEST == 1
    mysock.init("wlx00e02c3112e7");
    mysock.connectToHostPort(1234);
    mysock.connectToRemote("192.168.3.21", 1234);
    mysock.sendData(send_msg);
    while(1);
#elif TEST == 2
    mysock.init("wlx00e02c3112e7");
    mysock.listenHost(1234);
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
#elif TEST == 3
    igmpSock.createSocket();
    igmpSock.connectToHost("wlx00e02c3112e7", 1234);
    igmpSock.joinMultiGroup("224.132.1.1");
    mysock.listenHost("wlx00e02c3112e7", 1234);
    while(1);
#endif
    return 0;
}
