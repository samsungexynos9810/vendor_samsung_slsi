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
 * netlink.cpp
 *
 *  Created on: 2014. 10. 29.
 *      Author: sungwoo48.choi
 */

#include <unistd.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "netlink.h"
#include "rilcontext.h"
#include "service.h"
#include "servicemgr.h"
#include "thread.h"
#include "rildata.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_NETL, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_NETL, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_NETL, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_NETL, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define TEST_PREFIX_RECONFIGURATION (FALSE)
#define TEST_RA_TIMEOUT (FALSE)

class NetLinkRunnable : public Runnable {
    static __u32 nlgrp(__u32 group)
    {
        if (group > 31) {
            RilLogE("%s() invalid group", __FUNCTION__);
            return -1;
        }
        return group ? (1 << (group - 1)) : 0;
    }

    NetLinkMonitor *m_pManager;
private:
    int sock = -1;
    int buf_sz = 64 * 1024;
    struct sockaddr_nl nladdr;
    char buffer[64 * 1024];
    int len = 0;
    //int exitcode=0;

    struct sockaddr_nl sa;
    struct iovec iov = {buffer, sizeof(buffer)};
    struct msghdr msg = {(void *)&sa, sizeof(sa), &iov, 1, NULL, 0, 0};
    struct nlmsghdr *nh = NULL;
public:
    NetLinkRunnable(NetLinkMonitor *pManager) {
        m_pManager = pManager;
        memset (&nladdr, 0, sizeof (nladdr));
        memset(&sa, 0, sizeof(sa));
    }

    int initNetlinkSocket()
    {
        memset(&nladdr, 0, sizeof(nladdr));
        nladdr.nl_family = AF_NETLINK;
        nladdr.nl_pid    = gettid();
        nladdr.nl_groups = RTMGRP_IPV6_IFADDR | nlgrp(RTNLGRP_LINK) | nlgrp(RTNLGRP_IPV6_PREFIX) | nlgrp(RTNLGRP_NEIGH) | nlgrp(RTNLGRP_IPV6_ROUTE);

        if ((sock = socket(PF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE)) < 0) {
            RilLogE("%s(): error socket()", __FUNCTION__);
            return -1;
        }

        if (setsockopt(sock, SOL_SOCKET, SO_RCVBUFFORCE, &buf_sz, sizeof(buf_sz)) < 0) {
            RilLogE("%s(): error sockopt()", __FUNCTION__);
            close(sock);
            return -1;
        }

        if (::bind(sock, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
            RilLogE("%s(): error bind(%d)", __FUNCTION__, errno);
            close(sock);
            return -1;
        }

        RilLogV("NetLink binding PID:%d, tid:%d, nl_pid:%d sock:%d", getpid(), gettid(), nladdr.nl_pid, sock);
        return 0;
    }
    int recoverNetlinkSocket(int errcode)
    {
        if(errcode != 0){
            RilLogE("recover sock, Old sock = %d, errno=%d", sock, errno);
            close(sock);
        }
        if ( m_pManager != NULL ) {
            m_pManager->TagExitCode(errcode);
        }
        if(initNetlinkSocket() < 0){
            if(initNetlinkSocket() < 0){ // Just retry once
                RilLogV("Netlink Socket Creation failed, errno=%d", errno);
                return -1;
            }
        }
        return 0;
    }

    void Run() {
        if(recoverNetlinkSocket(0) < 0)
            return;

        while (1) {
            //RilLogV("%s(): Wait for netlink msg @ PID=%d,tid=%d", __FUNCTION__, getpid(), gettid());
            // Read the message from fd
            len = recvmsg(sock, &msg, 0);

            if (len < 0) {
                if (errno == EINTR || errno == EAGAIN) {
                    continue;
                }

                RilLogV("%s() netlink receive error %s (%d)",
                        __FUNCTION__, strerror(errno), errno);
                if (errno == ENOBUFS) {
                    continue;
                }

                if(recoverNetlinkSocket(4) < 0)
                    return;
                else continue;
            }

            if (len == 0) {
                RilLogV("%s() EOF on netlink", __FUNCTION__);
                if(recoverNetlinkSocket(5) < 0)
                    return;
                else continue;
            }

            int nlmsg_count=0;
            for (nh = (struct nlmsghdr *)buffer; NLMSG_OK(nh, (__u32)len); nh = NLMSG_NEXT(nh, len)) {
                //RilLogV("%s(): Received nlmsg len=%d, nlmsg_pid=%d, PID=%d, tid=%d", __FUNCTION__, len, nh->nlmsg_pid, getpid(), gettid());
                if (nh->nlmsg_type == NLMSG_DONE) {
                    RilLogV("%s(): detected no address change...", __FUNCTION__);
                    if(recoverNetlinkSocket(6) < 0)
                        return;
                    else continue;
                }

                if (nh->nlmsg_type == NLMSG_ERROR) {
                    RilLogV("%s(): Error in the message received", __FUNCTION__);
                    if(recoverNetlinkSocket(7) < 0)
                        return;
                    else continue;
                }

                switch (nh->nlmsg_type) {
                  case RTM_NEWADDR:
                      //RilLogV("%s(): RTM_NEWADDR nlmsg_len=%d, %d", __FUNCTION__, nh->nlmsg_len, nlmsg_count++);
                      break;
                  case RTM_DELADDR:
                      //RilLogV("%s(): RTM_DELADDR nlmsg_len=%d, %d", __FUNCTION__, nh->nlmsg_len, nlmsg_count++);
                      break;
                  case RTM_NEWLINK:
                      {
                          struct ifinfomsg *ifinfo = (struct ifinfomsg *) NLMSG_DATA(nh);
                          struct in6_addr *addr = NULL;
                          //int index = ifinfo->ifi_index;
                          int len = nh->nlmsg_len - NLMSG_LENGTH(sizeof(*ifinfo));
                          struct rtattr *rta;
                          char buf[100] = {0, };

                          if (ifinfo->ifi_family != AF_INET6) {
                              break;
                          }

                          rta = RTM_RTA(ifinfo);
                          while (RTA_OK(rta, len)) {
                              //RilLogV("%s(): rta len (%d), rta_type(%d)", __FUNCTION__, rta->rta_len, rta->rta_type);
                              switch(rta->rta_type){
                                case IFLA_UNSPEC:
                                    break;
                                case IFLA_ADDRESS:
                                    addr = (struct in6_addr *)RTA_DATA(rta);
                                    inet_ntop(AF_INET6, addr, buf, (socklen_t)sizeof(buf));
                                    //RilLogV("%s(): IFLA_ADDRESS addr:%x", __FUNCTION__, buf);
                                    break;
                                case IFLA_BROADCAST:
                                    break;
                                case IFLA_IFNAME:
                                    break;
                                case IFLA_MTU:
                                    break;
                                case IFLA_LINK:
                                    break;
                                case IFLA_QDISC:
                                    break;
                                case IFLA_STATS:
                                    break;
                              }
                              rta = RTA_NEXT(rta, len);
                          }

                          if (len) {
                              //RilLogV("%s(): Invalid length!", __FUNCTION__);
                              break;
                          }

                          //RilLogV("%s(): RTM_NEWLINK index=%d, nlmsg_len=%d, %d", __FUNCTION__, ifinfo->ifi_index, nh->nlmsg_len, nlmsg_count++);
                      }
                      break;
                  case RTM_DELROUTE:
                  case RTM_NEWROUTE:
                      {
                          char destination_address[128]={0,};
                          char gateway_address[128]={0,};
                          struct rtmsg *route_entry;
                          struct rtattr *route_attribute;
                          int route_attribute_len = 0;
                          in6_addr gateway_addr={{{0},},};
                          in6_addr dst_addr={{{0},},};
                          in6_addr null_addr={{{0},},};
                          map < int, in6_addr > gateway_addr_map;
                          int iif=-1,oif=-1;

                          /* Get the route data */
                          route_entry = (struct rtmsg *) NLMSG_DATA(nh);

                          if (route_entry->rtm_family != AF_INET6) {
                              break;
                          }

                          /* We have just interest in main routing table? : Nope
                          //if (route_entry->rtm_table != RT_TABLE_MAIN)
                          //    continue;

                          RilLogV("%s(): %s nlmsg_len=%d, %d :: protocol:%d, scope:%d, flag:%d,"
                                 "type:%d, table:%d src_len:%d, dst_len:%d, tos:%d",
                                 __FUNCTION__,
                                 (nh->nlmsg_type == RTM_NEWROUTE)?"RTM_NEWROUTE":"RTM_DELROUTE",
                                 nh->nlmsg_len, nlmsg_count++,
                                 route_entry->rtm_protocol,
                                 route_entry->rtm_scope,
                                 route_entry->rtm_flags,
                                 route_entry->rtm_type,
                                 route_entry->rtm_table,
                                 route_entry->rtm_src_len,
                                 route_entry->rtm_dst_len,
                                 route_entry->rtm_tos
                                 );
                                 */

                          /* Get attributes of route_entry */
                          route_attribute = (struct rtattr *) RTM_RTA(route_entry);

                          /* Get the route atttibutes len */
                          route_attribute_len = RTM_PAYLOAD(nh);
                          /* Loop through all attributes */
                          for ( ; RTA_OK(route_attribute, route_attribute_len); \
                                route_attribute = RTA_NEXT(route_attribute, route_attribute_len))
                          {
                              /* Get the destination address */
                              if (route_attribute->rta_type == RTA_DST)
                              {
                                  inet_ntop(AF_INET6, RTA_DATA(route_attribute), \
                                            destination_address, sizeof(destination_address));
                                  dst_addr = *((struct in6_addr *)RTA_DATA(route_attribute));
                              }
                              /* Get the Input interface index */
                              if (route_attribute->rta_type == RTA_IIF)
                              {
                                  iif = *(int *)RTA_DATA(route_attribute);
                              }
                              /* Get the Output interface index */
                              if (route_attribute->rta_type == RTA_OIF)
                              {
                                  oif = *(int *)RTA_DATA(route_attribute);
                              }
                              /* Get the gateway (Next hop) */
                              if (route_attribute->rta_type == RTA_GATEWAY)
                              {
                                  inet_ntop(AF_INET6, RTA_DATA(route_attribute), \
                                            gateway_address, sizeof(gateway_address));
                                  gateway_addr = *((struct in6_addr *)RTA_DATA(route_attribute));
                              }
                          }
                          if(*gateway_address!=0)
                          {
                              /* Now we can dump the routing attributes */
                              RilLogV("%s route to destination %s via gateway %s table %d",
                                     (nh->nlmsg_type == RTM_DELROUTE)? "Deleting" : "Adding",
                                     destination_address, gateway_address,
                                     route_entry->rtm_table);

                              if (m_pManager != NULL && IN6_ARE_ADDR_EQUAL(&dst_addr, &null_addr)) {
                                  RilLogV("%s(): Store Gateway address: %s of index: %d", __FUNCTION__, gateway_address, oif);
                                  m_pManager->StoreGateway(gateway_addr, oif);
                              }
                          }
                      }
                      break;
                  case RTM_NEWNEIGH:
                      {
                          struct ndmsg *ndinfo = (struct ndmsg *) NLMSG_DATA(nh);
                          //struct in6_addr *addr = NULL;
                          struct nda_cacheinfo *ndci = NULL;
                          char ifname[IFNAMSIZ];
                          int index = ndinfo->ndm_ifindex;
                          int len = nh->nlmsg_len - NLMSG_LENGTH(sizeof(*ndinfo));
                          struct rtattr *rta;
                          //char buf[100] = {0, };

                          if (ndinfo->ndm_family != AF_INET6) {
                              RilLogV("%s(): ndinfo has no AF_INET6, skip", __FUNCTION__);
                              break;
                          }
                          if (if_indextoname(index, ifname) == NULL)
                              snprintf(ifname, IFNAMSIZ, "if%d", index);

                          RilLogV("%s(): RTM_NEWNEIGH index=%d, nh=%x, nlmsg_len=%d, %d", __FUNCTION__, ndinfo->ndm_ifindex, nh, nh->nlmsg_len, nlmsg_count++);
                          RilLogV("%s(): ndmsg, ndm_family(%d), ndm_ifindex(%d,%s), ndm_state(%x), ndm_flags(%x), ndm_type(%d)",
                                  __FUNCTION__, ndinfo->ndm_family, ndinfo->ndm_ifindex, ifname, ndinfo->ndm_state, ndinfo->ndm_flags, ndinfo->ndm_type);

                          rta = RTM_RTA(ndinfo);
                          while (RTA_OK(rta, len)) {
                              RilLogV("%s(): ndmsg, rta len (%d), rta_type(%d)", __FUNCTION__, rta->rta_len, rta->rta_type);
                              switch(rta->rta_type){
                                case NDA_UNSPEC:
                                    RilLogV("%s(): NDA_UNSPEC", __FUNCTION__);
                                    break;
                                case NDA_DST:
                                    {
                                        char buf[INET6_ADDRSTRLEN];
                                        const char *valid_check;
                                        valid_check = inet_ntop(AF_INET6, RTA_DATA(rta), buf, INET6_ADDRSTRLEN);
                                        if( valid_check == buf )
                                            RilLogV("%s(): NDA_DST : %s", __FUNCTION__, buf);
                                        else
                                            RilLogV("%s(): incorrect address, errno = %d", __FUNCTION__, errno);
                                    }
                                    break;
                                case NDA_LLADDR:
                                    {
                                        // We don't know type here, about address decoding, refer external/iproute2  ll_addr_n2a function.
                                        //char buf[INET6_ADDRSTRLEN];
                                        //RilLogV("%s(): NDA_LLADDR : %s", __FUNCTION__,
                                        //        ll_addr_n2a((const unsigned char *)RTA_DATA(rta), (int)RTA_PAYLOAD(rta), -1, buf, sizeof(buf)));
                                    }
                                    break;
                                case NDA_CACHEINFO:
                                    {
                                        ndci = (struct nda_cacheinfo *)RTA_DATA(rta);
                                        //RilLogV("%s(): NDA_CACHEINFO, confirmed:%d, used:%d, updated:%d, refcnt:%d",
                                        //        __FUNCTION__, ndci->ndm_confirmed, ndci->ndm_used, ndci->ndm_updated, ndci->ndm_refcnt);
                                    }
                                    break;
                                case NDA_PROBES:
                                    RilLogV("%s(): NDA_PROBES", __FUNCTION__);
                                    break;
                                case NDA_VLAN:
                                    RilLogV("%s(): NDA_VLAN", __FUNCTION__);
                                    break;
                                case NDA_PORT:
                                    RilLogV("%s(): NDA_PORT", __FUNCTION__);
                                    break;
                                case NDA_VNI:
                                    RilLogV("%s(): NDA_VNI", __FUNCTION__);
                                    break;
                                case NDA_IFINDEX:
                                    RilLogV("%s(): NDA_IFINDEX", __FUNCTION__);
                                    break;
                                case NDA_MASTER:
                                    RilLogV("%s(): NDA_MASTER", __FUNCTION__);
                                    break;
                                case NDA_LINK_NETNSID:
                                    RilLogV("%s(): NDA_LINK_NETNSID", __FUNCTION__);
                                    break;
                                default:
                                    RilLogV("%s(): unknown rta_type:%d", __FUNCTION__, rta->rta_type);
                                    break;
                              }
                              rta = RTA_NEXT(rta, len);
                          }

                          if (len) {
                              //RilLogV("%s(): Invalid length!", __FUNCTION__);
                              break;
                          }
                      }
                      break;
                  case RTM_DELNEIGH:
                      RilLogV("%s(): RTM_DELNEIGH nh=%x, nlmsg_len=%d, %d", __FUNCTION__, nh, nh->nlmsg_len, nlmsg_count++);
                      break;
                  case RTM_GETNEIGH:
                      RilLogV("%s(): RTM_GETNEIGH nh=%x, nlmsg_len=%d, %d", __FUNCTION__, nh, nh->nlmsg_len, nlmsg_count++);
                      break;
                  case RTM_NEWPREFIX:
                      if (m_pManager != NULL) {
#if TEST_RA_TIMEOUT
                          static int ra_count=0;
                          RilLogV("%s(): Ignore Ra first 12 times(Two Deact for two APN should be occured) ra_count=%d", __FUNCTION__, ++ra_count);
                          if(ra_count>12)
                              m_pManager->HandleNewPrefix(nh);
#elif TEST_PREFIX_RECONFIGURATION
                          RilLogV("%s(): Call FakeNewPrefix nh=%x, nlmsg_len=%d, %d", __FUNCTION__, nh, nh->nlmsg_len, nlmsg_count++);
                          m_pManager->FakeNewPrefix(nh, 20); // For Test
#else
                          m_pManager->HandleNewPrefix(nh);
#endif
                      }
                      break;
                  default:
                      // Dump all other nlmsg
                      //RilLogV("%s(): type=%d, len=%d, %d\n", __FUNCTION__, nh->nlmsg_type, nh->nlmsg_len, nlmsg_count++);
                      break;
                }
            }
        }
    }
};

IMPLEMENT_MODULE_TAG(NetLinkMonitor, NetLink)

NetLinkMonitor::NetLinkMonitor(RilContext *pContext)
{
    m_pContext = pContext;
    m_pThread = NULL;
    m_pRunnable = NULL;
    m_iTestCount = 0;
}

NetLinkMonitor::~NetLinkMonitor()
{

}

int NetLinkMonitor::Start()
{
    m_pRunnable = new NetLinkRunnable(this);
    m_pThread = new Thread(m_pRunnable);
    if ( m_pThread == NULL ) {
        RilLogE("Can't create NetLinkMonitor Threadr");
        return -1;
    }
    if (m_pThread->Start() < 0) {
        RilLogE("Fail to start NetLinkMonitor");
        return -1;
    };
    return 0;
}

int NetLinkMonitor::Stop()
{
    if (m_pThread != NULL) {
        m_pThread->Stop();
    }
    return 0;
}

// This function will take new prefix and send original one once.
// Then after period, Send Another Modified Prefix, Then Send again Original One
#define FAKE_COUNT_MAX  (3)
int NetLinkMonitor::FakeNewPrefix(struct nlmsghdr *nh, int period)
{
    if(m_iTestCount==0 || m_iTestCount > FAKE_COUNT_MAX)
    {
        m_iTestCount++;
        return HandleNewPrefix(nh);
    }
    else
    {
        while(m_iTestCount <= FAKE_COUNT_MAX)
        {
            Ipv6Prefix fakePrefix;

            fakePrefix=m_prefix;
            fakePrefix.prefix.s6_addr[7]+=m_iTestCount++;
            SendPrefix(fakePrefix);

            sleep(period);
        }
        return HandleNewPrefix(nh);
    }
    return 0;
}

void NetLinkMonitor::StoreGateway(in6_addr &gateway, int oif)
{
    char gateway_address[128];

    m_prefix.gateway_addr = gateway;

    pair< map<int, in6_addr>::iterator, bool > Result;
    Result = m_prefix.gateway_addr_map.insert(ipv6_index_pair(oif, gateway));
    if(!Result.second)
    {
        char oldgateway[128]={0,};
        inet_ntop(AF_INET6, (const char *)&m_prefix.gateway_addr_map.find(oif)->second,
                  oldgateway, sizeof(oldgateway));

        RilLogV("%sThere is already stored Gateway address: %s with index:%d", __FUNCTION__, oldgateway,  oif);
    }

    inet_ntop(AF_INET6, &m_prefix.gateway_addr, \
              gateway_address, sizeof(gateway_address));
    RilLogV("%s(): Stored Gateway address: %s with index:%d", __FUNCTION__, gateway_address, oif);
}

void NetLinkMonitor::SendPrefix(Ipv6Prefix &data)
{
    char buf[100] = {0, };
    char buf2[100] = {0, };

    map<int, struct in6_addr>::iterator it = m_prefix.gateway_addr_map.find(data.index);
    if(it != m_prefix.gateway_addr_map.end())
    {
        data.gateway_addr = (m_prefix.gateway_addr_map.find(data.index)->second); // we just hope gateway address is filled
    }
    else
    {
        memset(&data.gateway_addr, 0, sizeof(data.gateway_addr));
    }

    inet_ntop(AF_INET6, data.prefix.s6_addr, buf, (socklen_t)sizeof(buf));
    inet_ntop(AF_INET6, data.gateway_addr.s6_addr, buf2, (socklen_t)sizeof(buf2));
    //RilLogV("%s() IPv6 Prefix=%s, Gateway=%s, ifindex=%d", __FUNCTION__, buf, buf2, data.index);

    if (m_pContext != NULL) {
        RilDataRaw *rildata = new RilDataRaw(&data, sizeof(data));
        Message *pMsg = Message::ObtainMessage(rildata, RIL_SERVICE_PS, MSG_PS_IPV6_CONFIGURED);
        ServiceMgr *pServigMrg = m_pContext->GetServiceManager();
        if (pServigMrg != NULL) {
            if (pServigMrg->SendMessage(pMsg) < 0) {
                RilLogE("%s(): SendMessage error", __FUNCTION__);
                delete pMsg;
            }
        }
        else {
            RilLogE("%s() unavailable ServiceMgr instance", __FUNCTION__);
            delete pMsg;
        }
    }
    else {
        RilLogE("%s() unavailable RilContext instance", __FUNCTION__);
    }
}

int NetLinkMonitor::HandleNewPrefix(struct nlmsghdr *nh)
{
    RilLogI("%s::%s()++", TAG, __FUNCTION__);
    struct prefixmsg *prefix = (struct prefixmsg *) NLMSG_DATA(nh);
    struct in6_addr *pfx = NULL;
    int index = prefix->prefix_ifindex;
    int len = nh->nlmsg_len - NLMSG_LENGTH(sizeof(*prefix));
    struct rtattr *rta;

    if (prefix->prefix_family != AF_INET6) {
        return -1;
    }

    rta = RTM_RTA(prefix);
    while (RTA_OK(rta, len)) {
        if (rta->rta_type == PREFIX_ADDRESS) {
            pfx = (struct in6_addr *)RTA_DATA(rta);
        }
        rta = RTA_NEXT(rta, len);
    }

    if (len) {
        RilLogE("%s(): Invalid length!", __FUNCTION__);
        return -1;
    }

    if (pfx == NULL) {
        RilLogE("%s(): No prefix info!", __FUNCTION__);
        return -1;
    }

    Ipv6Prefix data;
    memset(&data.prefix, 0, sizeof(struct in6_addr));
    memset(&data.gateway_addr, 0, sizeof(struct in6_addr));
    memcpy(&data.prefix, pfx, sizeof(data.prefix));
    data.index = index;

#if TEST_PREFIX_RECONFIGURATION
    // for Test, to remember original prefix
    m_prefix = data;
#endif

    SendPrefix(data);

    RilLogI("%s::%s()--", TAG, __FUNCTION__);

    return 0;
}

void NetLinkMonitor::TagExitCode(int exitcode)
{
    RilLogE("%s(): exit(%d) NetlinkMonitor Thread\n", __FUNCTION__, exitcode);
    // RilContext property
    if( NULL != m_pContext) {
        RilProperty *property = m_pContext->GetProperty();
        if (property != NULL) {
            property->Put("vendor.ril.context.ps.netlink.exitcode", exitcode);
        }
    } else {
        RilLogE("%s(): m_pContext is NULL. Something is corrupted\n", __FUNCTION__, exitcode);
    }
}
