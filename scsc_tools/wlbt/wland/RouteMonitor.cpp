#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <cstdio>
#include <cstring>
#include <cstdint>

#define LOG_TAG "ROUTE"
#include <utils/Log.h>

#include "RouteMonitor.h"

RouteMonitor *RouteMonitor::m_Instance = new RouteMonitor;

RouteMonitor *RouteMonitor::Instance()
{
    return m_Instance;
}

void RouteMonitor::MonitorRoutingUpdate()
{
    int received_bytes = 0;
    struct nlmsghdr *nlh;
    struct rtmsg *route_entry;      /* This struct represent a route entry in the routing table */
    struct rtattr *route_attribute; /* This struct contain route attributes (route type) */
    int route_attribute_len = 0;
    uint8_t *destination;
    uint8_t *gateway;
    int oif = 0;
    struct timeval rx_to = {1, 0};
    int max_fd = -1;
    fd_set read_fd;

    if(m_Sock == -1)
    {
        // Fail to open socket in constructor.
        // Retry to open socket.
        // If it fails to open socket, sleep 1 sec and retry.
        struct sockaddr_nl addr;
        m_Sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
        if(m_Sock == -1)
        {
            usleep(1000000);
            return;
        }

        memset(&addr, 0x0, sizeof(addr));
        addr.nl_family = AF_NETLINK;
        addr.nl_groups = RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE;

        if(bind(m_Sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            close(m_Sock);
            m_Sock = -1;
            usleep(1000000);
            return;
        }
    }

    FD_ZERO(&read_fd);
    FD_SET(m_Instance->m_Sock, &read_fd);
    max_fd = m_Instance->m_Sock + 1;

    const int state = select(max_fd + 1, &read_fd, NULL, NULL, &rx_to);
    if (state <= 0)
    {
        return;
    }

    /* Receiving netlink socket data */
    memset(m_Instance->m_Buffer, 0x0, sizeof(m_Instance->m_Buffer));
    received_bytes = recv(m_Instance->m_Sock, m_Instance->m_Buffer, sizeof(m_Instance->m_Buffer), 0);
    if (received_bytes < 0)
    {
        return;
    }
    /* cast the received buffer */
    nlh = (struct nlmsghdr *)m_Instance->m_Buffer;

    /* Reading netlink socket data */
    /* Loop through all entries */
    /* For more informations on some functions :
     * http://www.kernel.org/doc/man-pages/online/pages/man3/netlink.3.html
     * http://www.kernel.org/doc/man-pages/online/pages/man7/rtnetlink.7.html
     */
    for (; NLMSG_OK(nlh, (uint32_t)received_bytes); nlh = NLMSG_NEXT(nlh, received_bytes))
    {
        char destiation_str[128] = {0};
        char gateway_str[128] = {0};
        char ifname[32] = {0};

        /* Get the route data */
        route_entry = (struct rtmsg *)NLMSG_DATA(nlh);

        /* We are just intrested in main routing table */
        if (route_entry->rtm_table != RT_TABLE_MAIN)
            continue;

        /* Get attributes of route_entry */
        route_attribute = (struct rtattr *)RTM_RTA(route_entry);

        /* Get the route atttibutes len */
        route_attribute_len = RTM_PAYLOAD(nlh);
        /* Loop through all attributes */
        destination = nullptr;
        gateway = nullptr;
        oif = 0;
        for (; RTA_OK(route_attribute, route_attribute_len); route_attribute = RTA_NEXT(route_attribute, route_attribute_len))
        {
            /* Get the destination address */
            if (route_attribute->rta_type == RTA_DST)
            {
                destination = (uint8_t *)RTA_DATA(route_attribute);
            }
            /* Get the gateway (Next hop) */
            if (route_attribute->rta_type == RTA_GATEWAY)
            {
                gateway = (uint8_t *)RTA_DATA(route_attribute);
            }
            /* Get out interface index */
            if (route_attribute->rta_type == RTA_OIF)
            {
                oif = *(int *)RTA_DATA(route_attribute);
            }
        }

        if (destination)
        {
            inet_ntop(route_entry->rtm_family, destination, destiation_str, sizeof(destiation_str));
        }
        if (gateway)
        {
            inet_ntop(route_entry->rtm_family, gateway, gateway_str, sizeof(gateway_str));
        }
        ALOGD("%s Route Update : %s %s/%hhu via %s %s",
                (route_entry->rtm_family == AF_INET ? "IPv4" : "IPv6"),
                ((uint32_t)nlh->nlmsg_type == RTM_DELROUTE ? "DELETE" : "ADD"),
                destiation_str,
                route_entry->rtm_dst_len,
                (gateway ? gateway_str : ""),
                if_indextoname(oif, ifname));
    }
    return;
}

RouteMonitor::RouteMonitor()
{
    struct sockaddr_nl addr;
    m_Sock = -1;
    m_Sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if(m_Sock == -1)
    {
        return;
    }

    memset(&addr, 0x0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE;

    if(bind(m_Sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(m_Sock);
        m_Sock = -1;
    }
}

RouteMonitor::~RouteMonitor()
{
    if(m_Sock != -1)
    {
        close(m_Sock);
    }
}
