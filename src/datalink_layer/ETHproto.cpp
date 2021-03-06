#include <net/if.h> // if_nametoindex()
#include <linux/if_packet.h> // struct sockaddr_ll
#include <netinet/ip.h>
#include <netinet/ether.h>
#include "Print.h"
#include "ETHproto.h"
#include "HostBase.h"
#include "SockSend.h"

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
void CETHproto::refreshHostParam()
{
    m_nic = CHostBase::instance()->getHostNIC();
    m_src_mac = CHostBase::instance()->getHostMAC();
    m_mtu = CHostBase::instance()->getHostMTU();
    CSockSend::instance()->setNIC(m_nic);
}

void CETHproto::setNIC(string nic)
{
    m_nic = nic;
    CSockSend::instance()->setNIC(nic);
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

    /*printf("CETHproto::sendData, data pkt: ");
    for(uint16_t i = 0; i < frameLen; i++)
    {
        printf("0x%.2x ", *(frame+i));
    }
    printf("\n");*/
    
    res = CSockSend::instance()->sendData(sock, frame, frameLen, flags);
    delete frame;
    return res;
}

void CETHproto::setDstMAC(string dst_mac)
{
    m_dst_mac = dst_mac;
}

int16_t CETHproto::receiveData(uint8_t* &buf, uint16_t code, uint8_t type, int32_t timeout)
{
    struct timeval tv_start;
    struct timeval tv;
    gettimeofday(&tv_start, NULL);
    uint8_t* buf_eth = NULL;
    while(1)
    {
        int16_t len = CSockRecv::instance()->popBuffer(buf_eth, code, type);
        if((len == ERR_NO_PORT)||(len == ERR_NO_PROTOCOL)||(len == ERR_NO_TYPE)) // 查不到端口、协议或是查找类型出错，要马上返回
        {
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
                return ERR_TIMEOUT;
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
