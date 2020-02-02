#ifndef __SOCK_RECV_H__
#define __SOCK_RECV_H__

#include <iostream> // std
#include <map> // map
#include <list>
#include <pthread.h>
#include "Guard.h"

class CRecvBuf
{
public:
    CRecvBuf(uint32_t memSize, uint16_t port);
    ~CRecvBuf();
    uint8_t pushBack(uint8_t* buf, uint16_t len);
    int16_t popFront(uint8_t* &buf);
    uint32_t size();
private:
    std::list<std::pair<uint16_t, uint8_t*> > m_bufList; 
    uint32_t m_bufSize; // 当前缓存所占的大小：字节
    uint32_t m_bufCount;// 一共有多少个缓存包
    uint32_t m_memSize;//　一个CRecvBuf对象最多允许占多少内存：字节
    uint16_t m_port;// 该对象对应的端口
    pthread_mutex_t mutex_pushBack;
    pthread_mutex_t mutex_popFront;
};

class CSockRecv
{
public:
    enum NET_LAYER
    {
        DATALINK_LAYER = 0,
        NETWORK_LAYER = 8,
        TRANSPORT_LAYER = 24
    };
    enum BUFF_TYPE
    {
        BUFF_PORT,
        BUFF_PROTOCOL
    };
public:
    static CSockRecv* instance();
    uint8_t createRecv(std::string nic);
    void closePort(uint16_t port);
    void addPort(uint16_t port, uint32_t memSize);
    void addProtocol(uint16_t proto, uint32_t memSize);
    int16_t popBuffer(uint8_t* &buf, uint16_t code, uint8_t type);

private:
    CSockRecv();
    ~CSockRecv();
    static void* recvThread(void* param);
    static void* resProtoThread(void* param);
    void responseProto(uint16_t proto, uint8_t* buf, uint16_t len);
    void responseARP(uint8_t* buf, uint16_t len);
    uint8_t listenNIC(std::string nic);
    uint8_t parseData(uint8_t* buf, uint16_t len);
    void pushBufferByPort(uint16_t port, uint8_t* buf, uint16_t len);
    void pushBufferByProto(uint16_t proto, uint8_t* buf, uint16_t len, uint8_t signal);
    uint8_t dstIsHostIP(std::string ip);
    uint8_t dstIsHostMac(std::string mac);
private:
    uint16_t m_sockfd;
    pthread_t m_tid_recvData;
    pthread_t m_tid_resProto;
    std::map<uint16_t, CRecvBuf*> m_portMap;
    std::map<uint16_t, uint16_t> m_idPortMap;
    std::map<uint16_t, CRecvBuf*> m_protocolMap;// 数据长期保存，需要的时候从队列中获取
    std::map<uint16_t, CRecvBuf*> m_resProtoMap;// 需要马上处理的数据，比如说arp请求, igmp查询
    pthread_mutex_t mutex_createRecv;
    pthread_mutex_t mutex_pushBufferByPort;
    pthread_mutex_t mutex_pushBufferByProto;
    pthread_mutex_t mutex_closePort;
    pthread_mutex_t mutex_addPort;
    pthread_mutex_t mutex_addProtocol;
    pthread_mutex_t mutex_popBuffer;
    pthread_cond_t cond_resProto;
    std::string m_host_ip;
    std::string m_host_mac;
};


#endif
