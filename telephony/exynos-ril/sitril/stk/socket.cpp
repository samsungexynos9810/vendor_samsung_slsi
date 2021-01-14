/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <errno.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include "socket.h"
#include "rillog.h"
#include "stkmodule.h"
#include "types.h"

static bool debug = true;

// add category to display selective logs
#undef LOGV
#define LOGV(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_VERBOSE_LOG,  "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGI
#define LOGI(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_INFO_LOG, "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGW
#define LOGW(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_WARNING_LOG,  "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGE
#define LOGE(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_CRITICAL_LOG,  "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)

#undef ENTER_FUNC
#define ENTER_FUNC()        { LOGI("[<--"); }
#undef LEAVE_FUNC
#define LEAVE_FUNC()        { LOGI("[-->"); }

IMPLEMENT_MODULE_TAG(CSocket, Socket)

CSocket::CSocket(CStkModule *pStkModule)
    : m_pStkModule(pStkModule)
{
    m_nSocketType = -1;
    m_nSockFd = -1;
    m_nAcceptFd = -1;
    m_strIp = "";
    m_nPort = 0;
    m_strIFName = "";
}

CSocket::~CSocket()
{

}

int CSocket::OnCreate()
{
    ENTER_FUNC();

    LEAVE_FUNC();
    return 0;
}

void CSocket::OnStart()
{
    ENTER_FUNC();

    LEAVE_FUNC();
}

void CSocket::OnDestroy()
{
    ENTER_FUNC();

    LEAVE_FUNC();
}

int CSocket::Initialize()
{
    ENTER_FUNC();

    if(m_bStarted)
    {
        LOGI("Already Initialized !!!");
        return -1;
    }

    // Start Thread
    Start();

    LOGI("Socket Initialized...");
    LEAVE_FUNC();
    return 0;
}

int CSocket::Finalize()
{
    ENTER_FUNC();

    if(m_bStarted)
    {
        Stop();
    } else LOGI("Already Finalized !!!");

    LEAVE_FUNC();
    return 0;
}

void CSocket::Run()
{
    ENTER_FUNC();
    int nRxDataLen = 0;
    int nResult;
    fd_set readset;
    struct timeval tv;

    // Callback OnStart();
    OnStart();

    // Initialize time out struct, 10 sec
    tv.tv_sec = 60;
    tv.tv_usec = 10;

    FD_ZERO(&readset);
    FD_SET(m_nSockFd, &readset);

    nResult = select(m_nSockFd + 1, &readset, NULL, NULL,  &tv);

    if (nResult > 0)
    {
        if (FD_ISSET(m_nSockFd, &readset))
        {
            /* The socket_fd has data available to be read */
            if (m_nSocketType == SOCK_DGRAM)
                nRxDataLen = Receivefrom(m_pStkModule->m_szReceiveDataBuffer,1024,0);
            else if (m_nSocketType == SOCK_STREAM)
                nRxDataLen = Receive(m_pStkModule->m_szReceiveDataBuffer,1024,0);

            if(nRxDataLen < 0)
                LOGE("RX Thread running Failed(RxDataLen fail)");
            else
            {
                LOGI("RxDataLen :[%d]", nRxDataLen);
                m_pStkModule->SetReceiveDataLen(nRxDataLen);
                m_pStkModule->SendDataAvailableEvent();
            }
        }
    }
    else if (nResult < 0)
    {
        /* An error ocurred, just print it to stdout */
        LOGE("Error on select() : %s ",strerror(errno));
    }
    else if(nResult == 0)
    {
        LOGE("Timeout error : %s ",strerror(errno));
    }

    LEAVE_FUNC();
}

// Socket
int CSocket::SetDestination(string &strIp, int nPort)
{
    ENTER_FUNC();

    m_strIp = strIp;
    m_nPort = nPort;
    LOGI("IP: [%s], PORT: [%d]", m_strIp.c_str(), m_nPort);

    LEAVE_FUNC();
    return 0;
}

int CSocket::SetInterface(string &strIFname)
{
    ENTER_FUNC();

    m_strIFName = strIFname;
    LOGI("m_strIFName: [%s]", m_strIFName.c_str());

    LEAVE_FUNC();
    return 0;
}

//fdsocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP); // example
//getsockopt(fdsocket,SOL_SOCKET,SO_RCVBUF,(void *)&n, &m);
int CSocket::Create(int nSocketType)
{
    ENTER_FUNC();
    int snd_buf, rcv_buf;
    socklen_t len = 0;
    switch(nSocketType)
    {
    case SOCK_TCP: m_nSocketType = SOCK_STREAM; break;
    case SOCK_UDP: m_nSocketType = SOCK_DGRAM; break;
    default: m_nSocketType = -1; break;
    }

    if(m_nSocketType!=-1 && (m_nSockFd = socket(AF_INET, m_nSocketType, 0))==-1)
    {
        LOGE("Socket creation failed: %s", strerror(errno));
    }
    else
    {
        if (m_strIFName.length() > 0)
        {
            if (setsockopt(m_nSockFd, SOL_SOCKET, SO_BINDTODEVICE, m_strIFName.c_str(), m_strIFName.length()))
            {
                LOGE("Setsockopt() error");
            }
        }
        else
            LOGI("Setsockopt() : don't need setsockopt, maybe create socket for default APN ");

        len = sizeof(int);
        if(getsockopt(m_nSockFd, SOL_SOCKET, SO_SNDBUF, &snd_buf, &len))
        {
            LOGE("GetSockopt() error : SO_SNDBUF");
        }

        len = sizeof(int);
        if(getsockopt(m_nSockFd, SOL_SOCKET, SO_RCVBUF, &rcv_buf, &len))
        {
            LOGE("GetSockopt() error : SO_RCVBUF");
        }

        LOGI("send buffer size : %d", snd_buf);
        LOGI("receive buffer size : %d", rcv_buf);
    }

    LEAVE_FUNC();
    return m_nSockFd;
}

int CSocket::Connect()
{
    ENTER_FUNC();

    if(m_nSockFd<0)
    {
        LOGE("Socket is not created!!!");
        LEAVE_FUNC();
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(m_strIp.c_str());
    addr.sin_port = htons(m_nPort);

    if(connect(m_nSockFd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        LOGE("%s:%d Connection Failed: %s", m_strIp.c_str(), m_nPort, strerror(errno));
        LEAVE_FUNC();
        return errno;
    }

    LEAVE_FUNC();
    return 0;
}

int CSocket::Bind()
{
    ENTER_FUNC();

    LEAVE_FUNC();
    return 0;
}

int CSocket::Listen()
{
    ENTER_FUNC();

    LEAVE_FUNC();
    return 0;
}

int CSocket::Accept()
{
    ENTER_FUNC();

    LEAVE_FUNC();
    return 0;
}

int CSocket::Send(const void *buf, size_t len, int flags)
{
    ENTER_FUNC();

    if(m_nSockFd<0)
    {
        LOGE("Socket is not created!!!");
        LEAVE_FUNC();
        return -1;
    }

    int senddatalen = 0;
    senddatalen = send(m_nSockFd, buf, len, flags);
    if (senddatalen < 0)
        LOGE("%s:%d Send Failed: %s", strerror(errno));
    LEAVE_FUNC();
    return senddatalen;
}

// for UDP send
int CSocket::Sendto(const void *buf, size_t len, int flags)
{
    ENTER_FUNC();

    if(m_nSockFd<0)
    {
        LOGE("Socket is not created!!!");
        LEAVE_FUNC();
        return -1;
    }

    int senddatalen = 0;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(inet_addr(m_strIp.c_str()));
    addr.sin_port = htons(m_nPort);

    senddatalen = sendto(m_nSockFd, buf, len, flags, (struct sockaddr *) &addr, sizeof(addr));

    if (senddatalen < 0)
        LOGE("%s:%d Sendto Failed: %s", strerror(errno));

    LEAVE_FUNC();
    return senddatalen;
}

int CSocket::Receive(void *buf, size_t len, int flags)
{
    ENTER_FUNC();

    if(m_nSockFd==-1)
    {
        LOGE("Socket is not created!!!");
        LEAVE_FUNC();
        return -1;
    }

    int receivedatalen = 0;
    receivedatalen = recv(m_nSockFd, buf, len, flags);
    if (receivedatalen < 0)
        LOGE("%s:%d Receive Failed: %s", strerror(errno));
    LEAVE_FUNC();
    return receivedatalen;
}

int CSocket::Receivefrom(const void *buf, size_t len, int flags)
{
    ENTER_FUNC();

    if(m_nSockFd==-1)
    {
        LOGE("Socket is not created!!!");
        LEAVE_FUNC();
        return -1;
    }

    int receivedatalen = 0;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(m_strIp.c_str());
    addr.sin_port = htons(m_nPort);

    int addr_len = sizeof(addr);
    receivedatalen = recvfrom(m_nSockFd, (void *) buf,(size_t) len, (unsigned int)flags,(struct sockaddr *) &addr, (socklen_t*)&addr_len);

    if (receivedatalen < 0)
        LOGE("%s:%d Receivefrom Failed: %s", strerror(errno));

    LEAVE_FUNC();
    return receivedatalen;
}

int CSocket::Close()
{
    ENTER_FUNC();

    if(m_nSockFd==-1)
    {
        LOGI("Already closed...");
        LEAVE_FUNC();
        return 1;
    }

    close(m_nSockFd);
    LOGI("Socket %d closed...", m_nSockFd);
    m_nSockFd = -1;

    LEAVE_FUNC();
    return 0;
}
