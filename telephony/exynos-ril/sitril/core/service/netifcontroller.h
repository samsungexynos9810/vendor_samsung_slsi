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
 * netifcontroller.h
 *
 *  Created on: 2014. 10. 23.
 *      Author: sungwoo48.choi
 *
 *  reference code : kernel/exynos/Documentation/networking/ifenslave.c
 */

#ifndef    __NETIF_CONTROLLER_H__
#define    __NETIF_CONTROLLER_H__

#include "rildef.h"

class PdpContext;

class NetIfController {
    DECLARE_MODULE_TAG()
private:
    PdpContext *m_pPdpContext;
    char    m_ifname[256];
    int     m_skfd;
    int     m_skfdv6;

public:
    NetIfController(PdpContext *pPdpContext);
    ~NetIfController();

public:
    bool BringUp();
    bool TearDown();
    void Reset();
    void SetPdpContext(PdpContext *pPdpContext);

    // static
public:
    static int GetIfSock();
    static int GetIfSockV6();
    static short GetIfFlags(int skfd, char *ifname);
    static bool SetIfFlags(int skfd, char *ifname, short flags);
    static bool SetIfUp(int skfd, char *ifname, short flags);
    static bool SetIfDown(int skfd, char *ifname, short flags);
    static bool GetLastError();
    static int GetIfIndex(int skfd, const char *ifname);
    static bool SetIfAddr(int skfd, char *ifname, unsigned char *addr);
    static bool ClearIfAddr(int skfd, char *ifname);
    static bool SetIfAddrIpv6(int skfd, char *ifname, int index, unsigned char *addr);
    static bool SetIfAddrIpv6(int skfd, char *ifname, unsigned char *addr);
    static bool ClearIfGlobalAddress(int skfd, char *ifname, int index);
    static bool ClearIfLinkLocalAddress(int skfd, char *ifname, int index);
    static int SetIfMaxRsCount(const char *ifname, int count);
    static int GetIfMaxRsCount(const char *ifname);
    static int SetIfRsDelay(const char *ifname, int delay);
    static int GetIfRsDelay(const char *ifname);
    static int SetIfRsInterval(const char *ifname, int interval);
    static int GetIfRsInterval(const char *ifname);
};

#endif // __NET_CONTROLLER_H__
