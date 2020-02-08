#include "ARPproto.h"
#include "HostBase.h"
#include "SockSend.h"
#include <arpa/inet.h> // inet_addr()
#include <netinet/ether.h>
#include "SockRecv.h"
#include "Print.h"

using namespace std;

CARPproto::CARPproto()
{
    m_sockfd = -1;
    m_src_ip = "";
    m_dataLinkLayer = new CETHproto();
}

CARPproto::~CARPproto()
{

}

CARPproto* CARPproto::instance()
{
    static CARPproto* ptr = new CARPproto();
    return ptr;
}

void CARPproto::init()
{
    m_sockfd = CSockSend::instance()->createSocket();
    if(m_sockfd == -1)
    {
        ERRORNO("create socket failed\n");
        return;
    }
    m_src_ip = CHostBase::instance()->getHostIP();
    m_dataLinkLayer->refreshHostParam();
}

int16_t CARPproto::sendData(uint8_t type, string dst_ip, string dst_mac)
{
    string src_mac = m_dataLinkLayer->getHostMAC();
    uint32_t l_src_ip = inet_addr(m_src_ip.c_str());
    uint32_t l_dst_ip = inet_addr(dst_ip.c_str());
    
    struct ether_arp ea;//arp包数据结构
    // arp数据包
    ea.arp_hrd = htons(ARPHRD_ETHER);//硬件类型  2 bytes
    ea.arp_pro = htons(ETHERTYPE_IP);//协议类型  2 bytes
    ea.arp_hln = ETH_ALEN;//MAC地址长度6字节     1 byte
    ea.arp_pln = 4;//IP地址长度                  1 byte
    ea.arp_op = htons(type);//操作码，ARP请求1, ARP响应2  2 bytes
    memcpy(ea.arp_sha, ether_aton(src_mac.c_str()), ETH_ALEN);//源MAC   6 bytes
    memcpy(ea.arp_spa, &l_src_ip, 4);//源IP     4 bytes
    memcpy(ea.arp_tha, ether_aton(dst_mac.c_str()), ETH_ALEN);//目的MAC  6 bytes
    memcpy(ea.arp_tpa, &l_dst_ip, 4); // 目的ip  4 bytes
    m_dataLinkLayer->setDstMAC(dst_mac);
    int16_t res = m_dataLinkLayer->sendData(m_sockfd, &ea, sizeof(ea), 0, ARP_PROTO_ID);
    return res;
}

string CARPproto::recvResponse()
{
    int16_t len = 0;
    uint8_t* buf_arp = NULL;
    len = m_dataLinkLayer->receiveData(buf_arp, ARP_PROTO_ID, CSockRecv::BUFF_PROTOCOL, 3*1000);
    if(len <= 0)
    {
        ERROR("receive data failed, error no: %d\n", len);
        return "";
    }
    else
    {
        string dst_mac;
        struct ether_arp* ea = (struct ether_arp*)buf_arp;//arp包数据结构
        dst_mac = ether_ntoa((struct ether_addr*)ea->arp_sha);
        return dst_mac;
    }
}
