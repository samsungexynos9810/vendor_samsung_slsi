/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/*
 * socket.h
 *
 *  Created on: 2017.03.28.
 *      Author: MOX
 */

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stddef.h>
#include "types.h"
#include "thread.h"

using namespace std;
#ifdef DEFAULT_BUFFER_SIZE
#undef DEFAULT_BUFFER_SIZE
#define DEFAULT_BUFFER_SIZE 1400
#endif

#define DEFAULT_CHANNEL_ID 1

class CStkModule;

class CSocket : public Thread
{
    DECLARE_MODULE_TAG()

public:
    typedef enum SocketType {
        SOCK_UDP = 1,
        SOCK_TCP,
        SOCK_LOCAL,
    } SOCKET_TYPE;

    CSocket(CStkModule *pStkModule);
    virtual ~CSocket();

    // Not Used Yet
    virtual int OnCreate();
    // Not Used Yet
    virtual void OnStart();
    virtual void OnDestroy();

    bool IsStarted() { return m_bStarted; }

    int Initialize();
    int Finalize();
    virtual void Run();

    // Socket
    virtual int SetInterface(string &pstrIFname);
    virtual int SetDestination(string &strIp, int nPort);
    virtual int Create(int nSocketType);
    virtual int Connect();
    virtual int Bind();
    virtual int Listen();
    virtual int Accept();
    virtual int Send(const void *buf, size_t len, int flags);
    virtual int Sendto(const void *buf, size_t len, int flags);
    virtual int Receivefrom(const void *buf, size_t len, int flags);
    virtual int Receive(void *buf, size_t len, int flags);
    virtual int Close();

private:
    CStkModule *m_pStkModule;
    friend CStkModule;

    int m_nSocketType;
    int m_nSockFd;
    int m_nAcceptFd;
    string m_strIFName;
    string m_strIp;
    int m_nPort;
};
#endif // __SOCKET_H__
