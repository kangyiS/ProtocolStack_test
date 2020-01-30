#include <net/if.h> // if_nametoindex()
#include <linux/if_packet.h> // struct sockaddr_ll
#include <netinet/ip.h>
#include <netinet/ether.h>
#include "Print.h"
#include "SockRecv.h"
#include "ETHproto.h"
#include "HostBase.h"

using namespace std;

CETHproto::CETHproto()
{
    m_nic = "";
    m_src_mac = "";
    m_dst_mac = "";
    m_mtu = 0;
}

CETHproto::~CETHproto()
{

}

void CETHproto::setNIC(string nic)
{
    m_nic = nic;
}

string CETHproto::getNIC()
{
    return m_nic;
}

string CETHproto::getHostMAC(uint8_t refresh)
{
    if(refresh == 1)
    {
        if(m_nic == "")
        {
            ERROR("get NIC failed, please set NIC first, return\n");
            return "";
        }
        m_src_mac = CHostBase::instance()->getHostMAC(m_nic);
    }
    return m_src_mac;
}

uint16_t CETHproto::getMTU(uint8_t refresh)
{
    if(refresh == 1)
    {
        if(m_nic == "")
        {
            ERROR("get NIC failed, please set NIC first, return\n");
            return 0;
        }
        m_mtu = CHostBase::instance()->getHostMTU(m_nic);
    }
    return m_mtu;
}

int16_t CETHproto::sendData(int16_t sock, void* data, uint16_t dataLen, int16_t flags, uint16_t proto)
{
    int res = 0;

    struct sockaddr_ll dst_info;
    memset(&dst_info, 0, sizeof(dst_info));
    dst_info.sll_family = PF_PACKET;
    dst_info.sll_ifindex = if_nametoindex(m_nic.c_str());//返回对应接口名的编号

    struct ether_header eh;
    if(m_dst_mac == "")
    {
        ERROR("get dst mac failed, please set dst mac first, return\n");
        return -1;
    }
    memcpy(eh.ether_dhost, ether_aton(m_dst_mac.c_str()), ETH_ALEN);//  6 bytes
    memcpy(eh.ether_shost, ether_aton(m_src_mac.c_str()), ETH_ALEN);//  6 bytes
    eh.ether_type = htons(proto);// 2 bytes

    uint8_t frameLen = sizeof(eh) + dataLen;
    if(frameLen < ETHER_FRAME_MIN)
    {
        frameLen = ETHER_FRAME_MIN;
    }
    uint8_t* frame = new uint8_t[frameLen];
    memset(frame, 0, frameLen);
    memcpy(frame, &eh, sizeof(eh));
    memcpy(frame+sizeof(eh), (uint8_t*)data, dataLen);

    /*printf("CETHproto::sendFrame, frame pkt: ");
    for(uint16_t i = 0; i < frameLen; i++)
    {
        printf("0x%x ", *(frame+i));
    }
    printf("\n");*/
    res = sendto(sock, frame, frameLen, flags, (struct sockaddr *)&dst_info, sizeof(dst_info));
    delete frame;
    return res;
}

void CETHproto::setDstMAC(string dst_mac)
{
    m_dst_mac = dst_mac;
}

// 查不到端口，返回-1，超时，返回-2
int16_t CETHproto::receiveData(uint8_t* &buf, uint16_t port, int32_t timeout)
{
    struct timeval tv_start;
    struct timeval tv;
    gettimeofday(&tv_start, NULL);
    uint8_t* buf_eth = NULL;
    while(1)
    {
        int16_t len = CSockRecv::instance()->popBuffer(buf_eth, port);
        if(len == -1) // -1代表查不到端口，要马上返回
        {
            ERROR("the port: %d is not registered, return\n", port);
            buf = NULL;
            return len;
        }
        else if(len > 0)
        {
            uint8_t eh_len = sizeof(struct ether_header);
            buf = buf_eth + eh_len;// 剥掉以太网头，返回给ip层
            INFO("eth packet length: %d, return\n", len);
            return len - eh_len;       
        }
        if(timeout > 0)// -1代表阻塞模式，会一直查询数据，正数代表设置了超时时间，单位ms
        {
            gettimeofday(&tv, NULL);
            uint32_t time_ms = (tv.tv_sec - tv_start.tv_sec)*1000 + (tv.tv_usec - tv_start.tv_usec)/1000;
            if(time_ms > timeout)
            {
                WARN("receive data timeout:　%d ms, return\n", timeout);
                buf = NULL;                
                return -2;
            }
        }
        else if(timeout == 0)// 0代表非阻塞模式
        {
            WARN("there is no data, non blocking mode, return\n");
            buf = NULL;
            return 0;
        }
    }
}
