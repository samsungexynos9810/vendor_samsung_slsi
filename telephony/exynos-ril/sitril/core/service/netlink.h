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
 * netlink.h
 *
 *  Created on: 2014. 10. 29.
 *      Author: sungwoo48.choi
 */

#ifndef __NETLINK_H__
#define __NETLINK_H__

#include "types.h"
#include <map>

typedef pair <int, struct in6_addr > ipv6_index_pair;
typedef struct {
    struct in6_addr prefix;
    map<int, struct in6_addr> gateway_addr_map;
    struct in6_addr gateway_addr;
    int index;
} Ipv6Prefix;

class RilContext;
class Thread;
class NetLinkRunnable;

class NetLinkMonitor {
    DECLARE_MODULE_TAG()
public:
    static const char HANDLE_REQ = 100;
    static const char REQ_FINISHED_NOTI = 101;
    static const char QUIT = 102;

private:
    RilContext *m_pContext;
    Thread *m_pThread;
    NetLinkRunnable *m_pRunnable;
    int m_iTestCount;
    Ipv6Prefix m_prefix;
private:
    int HandleNewPrefix(struct nlmsghdr *nh);
    int FakeNewPrefix(struct nlmsghdr *nh, int period);
    void SendPrefix(Ipv6Prefix &data);
    void StoreGateway(in6_addr &gateway, int oif);
    void TagExitCode(int exitcode);

public:
    explicit NetLinkMonitor(RilContext *pContext);
    virtual ~NetLinkMonitor();

    int Start();
    int Stop();

    friend class NetLinkRunnable;
};


#endif /* __NETLINK_H__ */
