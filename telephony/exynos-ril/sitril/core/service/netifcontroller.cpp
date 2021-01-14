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
 * netifcontroller.cpp
 *
 *  Created on: 2014. 10. 23.
 *      Author: sungwoo48.choi
 *
 *  reference code : kernel/exynos/Documentation/networking/ifenslave.c
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
//#include <linux/route.h>
//#include <linux/ipv6_route.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <net/if_arp.h>

#include "netifcontroller.h"
#include "pdpcontext.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_NETIF, CRilLog::E_RIL_INFO_LOG, format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_NETIF, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_NETIF, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_NETIF, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define MAX_FILE_LEN 64
#ifndef IPV6_ADDR_LINKLOCAL
#define IPV6_ADDR_LINKLOCAL     0x0020U
#endif

#ifndef IPV6_ADDR_GLOBAL
#define IPV6_ADDR_GLOBAL        0x0000U
#endif

#define PROC_NET_IF_INET6       "/proc/net/if_inet6"
#define PROC_NET_IF_INET6_ROUTE "/proc/net/ipv6_route"
#define IN6_FMT "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
#define IN6_ADDR_EXP(__a, __op) \
             __op(__a[0]), __op(__a[1]), \
             __op(__a[2]), __op(__a[3]), \
             __op(__a[4]), __op(__a[5]), \
             __op(__a[6]), __op(__a[7]), \
             __op(__a[8]), __op(__a[9]), \
             __op(__a[10]), __op(__a[11]), \
             __op(__a[12]), __op(__a[13]), \
             __op(__a[14]), __op(__a[15]) \

static unsigned char nullIpv4[MAX_IPV4_ADDR_LEN] = { 0, };
static unsigned char nullIpv6[MAX_IPV6_ADDR_LEN] = { 0, };

static bool debug = true;
IMPLEMENT_MODULE_TAG(NetIfController, NetIfController)
const char *TAG = "[NetIfController]";

static int last_errno = 0;
static int get_if_sock();
static short get_if_flags(int skfd, char *ifname);
static int set_if_flags(int skfd, char *ifname, short flags);
static int set_if_up(int skfd, char *ifname, short flags);
static int set_if_down(int skfd, char *ifname, short flags);
static int set_if_addr(int skfd, char *ifname, unsigned char *addr);
static int clear_if_addr(int skfd, char *ifname);

static int get_if_sock()
{
    RilLogI("%s %s", TAG, __FUNCTION__);
    int skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (skfd < 0) {
        last_errno = errno;
        RilLogE("%s %s error %d", TAG, __FUNCTION__, last_errno);
    }

    return skfd;
}

static short get_if_flags(int skfd, char *ifname)
{
    RilLogI("%s %s", TAG, __FUNCTION__);
    struct ifreq ifr;
    int res = 0;

    SECURELIB::strncpy(ifr.ifr_name, sizeof(ifr.ifr_name), ifname, SECURELIB::strlen(ifname));
    res = ioctl(skfd, SIOCGIFFLAGS, &ifr);
    if (res < 0) {
        last_errno = errno;
        RilLogE("%s %s error %d", TAG, __FUNCTION__, last_errno);
        return 0;
    }
    RilLogI("%s %s returns %x", TAG, __FUNCTION__, ifr.ifr_flags);

    return ifr.ifr_flags;
}

static int set_if_flags(int skfd, char *ifname, short flags)
{
    RilLogI("%s %s %x", TAG, __FUNCTION__, flags);
    struct ifreq ifr;
    int res = 0;

    ifr.ifr_flags = flags;
    SECURELIB::strncpy(ifr.ifr_name, sizeof(ifr.ifr_name), ifname, SECURELIB::strlen(ifname));

    res = ioctl(skfd, SIOCSIFFLAGS, &ifr);
    if (res < 0) {
        last_errno = errno;
        RilLogE("%s %s error %d", TAG, __FUNCTION__, last_errno);
    }

    return res;
}

static int set_if_up(int skfd, char *ifname, short flags)
{
    RilLogI("%s %s", TAG, __FUNCTION__);
    return set_if_flags(skfd, ifname, flags | IFF_UP);
}

static int set_if_down(int skfd, char *ifname, short flags)
{
    RilLogI("%s %s", TAG, __FUNCTION__);
    return set_if_flags(skfd, ifname, flags & ~IFF_UP);
}

static int set_if_addr(int skfd, char *ifname, unsigned char *addr)
{
    RilLogI("%s %s %s %d", TAG, __FUNCTION__, (ifname) ? ifname : "", skfd );
    struct ifreq ifr;
    int res = 0;
    SECURELIB::strncpy(ifr.ifr_name, sizeof(ifr.ifr_name), ifname, SECURELIB::strlen(ifname));
    ifr.ifr_addr.sa_family = AF_INET;
    memset(ifr.ifr_addr.sa_data, 0, sizeof(ifr.ifr_addr.sa_data));
    ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr = *((in_addr_t *)addr);

    res = ioctl(skfd, SIOCSIFADDR, &ifr);
    if (res < 0) {
        if (errno == EEXIST) {
            // ignore
            return 0;
        }
        last_errno = errno;
        RilLogE("%s(SIOCSIFADDR) %s error %d", TAG, __FUNCTION__, last_errno);
        if(last_errno == 13)
        {
            RilLogE("%s(SIOCSIFADDR) %s uid %d, tid %d, pid %d, gid %d, egid%d", TAG, __FUNCTION__, getuid(), gettid(), getpid(), getgid(), getegid());
        }
    }
    else {
        // This will trigger Kernel main routing insertion,
        // but useless from AOSP N version, each interface's routing table will hold own default
        /*
        unsigned int mask = 24;
        mask = htonl(~(0) << (32 - mask));
        ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr = *((in_addr_t *)&mask);
        if (ioctl(skfd, SIOCSIFNETMASK, &ifr) < 0) {
            if (errno == EEXIST) {
                // ignore
            }
            last_errno = errno;
            RilLogE("%s(SIOCSIFNETMASK) %s error %d", TAG, __FUNCTION__, last_errno);
        }
        */
    }

    return res;
}

static int clear_if_addr(int skfd, char *ifname)
{
    RilLogI("%s %s %s %d", TAG, __FUNCTION__, (ifname) ? ifname : "", skfd);
    struct ifreq ifr;
    int res = 0;

    SECURELIB::strncpy(ifr.ifr_name, sizeof(ifr.ifr_name), ifname, SECURELIB::strlen(ifname));
    ifr.ifr_addr.sa_family = AF_INET;
    memset(ifr.ifr_addr.sa_data, 0, sizeof(ifr.ifr_addr.sa_data));

    res = ioctl(skfd, SIOCSIFADDR, &ifr);
    if (res < 0) {
        last_errno = errno;
        RilLogE("%s %s error %d", TAG, __FUNCTION__, last_errno);
    }

    return res;
}

static int set_if_enabled_IPv6(char *ifname, bool enabled)
{
    RilLogI("%s %s %s %d", TAG, __FUNCTION__, (ifname) ? ifname : "", enabled);
    char filepath[MAX_FILE_LEN];
    snprintf(filepath, sizeof(filepath)-1, "/proc/sys/net/ipv6/conf/%s/disable_ipv6", ifname);

    int fd = open(filepath, O_WRONLY | O_CLOEXEC);
    if (fd == -1) {
        RilLogE("%s %s error %d", TAG, __FUNCTION__, last_errno);
        return -1;
    }

    char c = (enabled ? 0 : 1) + '0';
    int n = write(fd, &c, sizeof(c));

    close(fd);

    if (n < 1) {
        RilLogE("Failed to set set_if_enabled_v6(%s, %s)", ifname, (enabled ? "enabled" : "disabled"));
        return -1;
    }

    RilLogV("IPv6 for %s is %s", ifname, (enabled ? "enabled" : "disabled"));
    return 0;
}

static int set_if_enabled_auto_conf_v6(char *ifname, bool enabled)
{
    RilLogI("%s %s", TAG, __FUNCTION__);
    char filepath[MAX_FILE_LEN];

    snprintf(filepath, sizeof(filepath)-1, "/proc/sys/net/ipv6/conf/%s/autoconf", ifname);

    int fd = open(filepath, O_WRONLY | O_CLOEXEC);
    if (fd == -1) {
        RilLogE("Failed to set set_if_enabled_auto_conf_v6(%s, %s)", ifname, (enabled ? "enabled" : "disabled"));
        return -1;
    }

    char c = (enabled ? '1' : '0');
    int n = write(fd, &c, sizeof(c));
    close(fd);

    if (n < 1) {
        RilLogE("Failed to set_if_enabled_auto_conf_v6(%s, %s)", ifname, (enabled ? "enable" : "disable"));
        return -1;
    }

    RilLogV("autoconf for %s is %s.", ifname, (enabled ? "enabled" : "disabled"));
    return 0;
}

static int set_if_enabled_dad_v6(char *ifname, bool enabled)
{
    RilLogI("%s %s", TAG, __FUNCTION__);
    char filepath[MAX_FILE_LEN];

    snprintf(filepath, MAX_FILE_LEN, "/proc/sys/net/ipv6/conf/%s/accept_dad", ifname);

    int fd = open(filepath, O_WRONLY | O_CLOEXEC);
    if (fd == -1) {
        RilLogE("Failed to set set_if_enabled_dad_v6(%s, %s)", ifname, (enabled ? "enabled" : "disabled"));
        return -1;
    }

    char c = (enabled ? '1' : '0');
    int n = write(fd, &c, sizeof(c));
    close(fd);

    if (n < 1) {
        RilLogE("Failed to set_if_enabled_dad_v6(%s, %s)", ifname, (enabled ? "enable" : "disable"));
        return -1;
    }

    RilLogV("Dad for %s is %s.", ifname, (enabled ? "enabled" : "disabled"));
    return 0;
}

static int set_if_max_rs_count_v6(const char *ifname, int count)
{
    RilLogI("%s %s", TAG, __FUNCTION__);
    char filepath[MAX_FILE_LEN];

    snprintf(filepath, MAX_FILE_LEN, "/proc/sys/net/ipv6/conf/%s/router_solicitations", ifname);

    int fd = open(filepath, O_WRONLY | O_CLOEXEC);
    if (fd == -1) {
        RilLogE("set_if_max_rs_count_v6(%s, %d) Failed to open %s due to %s",
                ifname, count, filepath, strerror(errno));
        return -1;
    }

    char c = count + '0';
    int n = write(fd, &c, sizeof(c));
    close(fd);

    if (n < 1) {
        RilLogE("Failed to set_if_max_rs_count_v6(%s, %d)", ifname, count);
        return -1;
    }

    RilLogV("MaxRsCount for %s is set to %d.", ifname, count);
    return 0;
}

const char* rs_propname[3] = { "router_solicitations",
                               "router_solicitation_delay",
                               "router_solicitation_interval" };

typedef enum { RS_MAXCOUNT=0, RS_DELAY, RS_INTERVAL } _rs_propname;

static int get_if_ipv6conf_sysproc(const char *ifname, const char *prop_name)
{
    RilLogI("%s %s", TAG, __FUNCTION__);
    char filepath[MAX_FILE_LEN];

    snprintf(filepath, MAX_FILE_LEN, "/proc/sys/net/ipv6/conf/%s/%s", ifname, prop_name);

    int fd = open(filepath, O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        RilLog("%s(%s,%s) Failed to open %s due to %s",
                __func__, ifname, prop_name, filepath, strerror(errno));
        return -1;
    }

    char c[5] = {0,};
    int n = read(fd, &c, sizeof(c));
    close(fd);

    if (n < 0) {
        RilLog("Failed to %s(%s)", __func__, ifname);
        return -1;
    }
    n = atoi(c);
    RilLogV("%s for %s is %d.", prop_name, ifname, n);
    return n;
}

static int set_if_max_dad_count_v6(const char *ifname, int count)
{
    RilLogI("%s %s", TAG, __FUNCTION__);
    char filepath[MAX_FILE_LEN];

    snprintf(filepath, MAX_FILE_LEN, "/proc/sys/net/ipv6/conf/%s/dad_transmits", ifname);

    int fd = open(filepath, O_WRONLY | O_CLOEXEC);
    if (fd == -1) {
        RilLogE("set_if_max_dad_count_v6(%s, %d) Failed to open %s due to %s",
                ifname, count, filepath, strerror(errno));
        return -1;
    }

    char c = count + '0';
    int n = write(fd, &c, sizeof(c));
    close(fd);

    if (n < 1) {
        RilLogE("Failed to set_if_max_dad_count_v6(%s, %d)", ifname, count);
        return -1;
    }

    RilLogV("MaxDadCount for %s is set to %d.", ifname, count);
    return 0;
}

static int get_if_index(int skfd, const char *ifname)
{
    RilLogI("%s %s skfd:%d, ifname:%s", TAG, __FUNCTION__, skfd, (ifname) ? ifname : "");
    struct ifreq ifr;
    SECURELIB::strncpy(ifr.ifr_name, sizeof(ifr.ifr_name), ifname, SECURELIB::strlen(ifname));
    if (ioctl(skfd, SIOCGIFINDEX, &ifr) < 0) {
        last_errno = errno;
        RilLogE("%s %s error %d", TAG, __FUNCTION__, last_errno);
        return -1;
    }
    RilLogV("%s %s returns index:%d", TAG, __FUNCTION__, ifr.ifr_ifindex);
    return ifr.ifr_ifindex;
}


static int get_if_sock_v6()
{
    RilLogI("%s %s", TAG, __FUNCTION__);
    int skfd = socket(AF_INET6, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (skfd < 0) {
        last_errno = errno;
        RilLogE("%s %s error %d", TAG, __FUNCTION__, last_errno);
    } else {

    }
    RilLogV("%s %s success return skfd:%d", TAG, __FUNCTION__, skfd);
    return skfd;
}

static int set_if_addr_v6(int skfd, char *ifname, int index, unsigned char *addr)
{
    RilLogI("%s %s skfd:%d, ifname:%s, index:%d", TAG, __FUNCTION__, skfd, (ifname) ? ifname : "", index);
    struct in6_ifreq ifr;
    int res = 0;

    memset(&ifr, 0, sizeof(struct in6_ifreq));
    ifr.ifr6_ifindex = 0;
    ifr.ifr6_prefixlen = 0;
    memcpy((void*)(ifr.ifr6_addr.s6_addr), (void *)addr, 16);
    ifr.ifr6_ifindex = index;
    ifr.ifr6_prefixlen = 64;

    res = ioctl(skfd, SIOCSIFADDR, &ifr);
    if (res < 0) {
        if (errno == EEXIST) {
            RilLogW("%s %s wanring %d address is already exist", TAG, __FUNCTION__, last_errno);
            // ignore
            return 0;
        }
        last_errno = errno;
        RilLogE("%s %s error %d", TAG, __FUNCTION__, last_errno);
    }
    else {

    }

    return res;
}

static int clear_if_addr_v6(int skfd, char *ifname, int index, unsigned char *addr)
{
    RilLogI("%s %s skfd:%d, ifname:%s, index:%d", TAG, __FUNCTION__, skfd, (ifname) ? ifname : "", index);
    struct in6_ifreq ifr;
    int res = 0;

    memset(&ifr, 0, sizeof(struct in6_ifreq));
    memcpy((void*)(ifr.ifr6_addr.s6_addr), (void *)addr, 16);
    ifr.ifr6_ifindex = index;
    ifr.ifr6_prefixlen = 64;
    res = ioctl(skfd, SIOCDIFADDR, &ifr);
    if (res < 0) {
        last_errno = errno;
        RilLogE("%s %s error %d", TAG, __FUNCTION__, last_errno);
    }
    else {

    }

    return res;
}

static int clear_if_global_addr_v6(int skfd, char *ifname, int index)
{
    RilLogI("%s %s skfd:%d, ifname:%s, index:%d", TAG, __FUNCTION__, skfd, (ifname) ? ifname : "", index);
    //struct in6_ifreq ifr;
    //int res = 0;

    FILE *fp;
    char tmp_ifname[IF_NAMESIZE];
    unsigned int tmp_index, unused2, tmp_scope, unused3;
    unsigned int tmp_addr[16];
    unsigned char addr[16];

    if ((fp = fopen(PROC_NET_IF_INET6, "r")) == NULL) {
        RilLogE("%s(): Couldn't open proc file %s", __FUNCTION__, PROC_NET_IF_INET6);
        return -1;
    }

    while (fscanf(fp, IN6_FMT" %02x %02x %02x %02x %15s\n",
                  IN6_ADDR_EXP(tmp_addr, &), &tmp_index, &unused2,
                  &tmp_scope, &unused3, tmp_ifname) != EOF) {
        if (strcmp(ifname, tmp_ifname) == 0 && tmp_scope == IPV6_ADDR_LINKLOCAL) {
            for (int i = 0; i < 16; i++) {
                addr[i] = (__u8)tmp_addr[i];
            }

            RilLogV("%s(): remove ipv6 addr with index(%d)", __FUNCTION__, tmp_index);
            clear_if_addr_v6(skfd, ifname, index, addr);
        }
    }
    fclose(fp);
    fp = NULL;

    return 0;
}

static int clear_if_linklocal_addr_v6(int skfd, char *ifname, int index)
{
    RilLogI("%s %s", TAG, __FUNCTION__);
    //struct in6_ifreq ifr;
    //int res = 0;

    FILE *fp;
    char tmp_ifname[IF_NAMESIZE];
    unsigned int tmp_index, unused2, tmp_scope, unused3;
    unsigned int tmp_addr[16];
    unsigned char addr[16];

    if ((fp = fopen(PROC_NET_IF_INET6, "r")) == NULL) {
        RilLogE("%s(): Couldn't open proc file %s", __FUNCTION__, PROC_NET_IF_INET6);
        return -1;
    }

    while (fscanf(fp, IN6_FMT" %02x %02x %02x %02x %15s\n",
                  IN6_ADDR_EXP(tmp_addr, &), &tmp_index, &unused2,
                  &tmp_scope, &unused3, tmp_ifname) != EOF) {
        if (strcmp(ifname, tmp_ifname) == 0 && tmp_scope == IPV6_ADDR_GLOBAL) {
            for (int i = 0; i < 16; i++) {
                addr[i] = (__u8)tmp_addr[i];
            }

            RilLogV("%s(): remove ipv6 addr with index(%d)", __FUNCTION__, tmp_index);
            clear_if_addr_v6(skfd, ifname, index, addr);
        }
    }
    fclose(fp);
    fp = NULL;

    return 0;
}

#if 0
static int clear_if_routes_v6(int skfd, char *ifname, int index, unsigned int fl, unsigned int pref)
{
    char tmp_ifname[64];
    unsigned int prefix, unused1, unused2, unused3, unused4, flags;
    unsigned int dst[16];
    unsigned int temp[16];
    unsigned int gw[16];
    struct in6_rtmsg rt;
    FILE *fp;
    int ifindex;
    char dst_str[64], gw_str[64];
    int i, result;

    if ((fp = fopen("/proc/net/ipv6_route", "r")) == NULL) {
        RilLogE("Failed to open route file.");
        return -1;
    }

    for (;;) {
        int nread =
                fscanf(fp,
                        IN6_FMT" %02x "IN6_FMT" %02x " IN6_FMT" \
                %08x %08x %08x %08x %15s\n",
                        IN6_ADDR_EXP(dst, &), &prefix, IN6_ADDR_EXP(temp, &),
                        &unused1, IN6_ADDR_EXP(gw, &), &unused2, &unused3,
                        &unused4, &flags, tmp_ifname);

        if (nread != (16 + 1 + 16 + 1 + 16 + 5)) {
            break;
        }

        if ((flags & (RTF_UP | fl)) != (RTF_UP | fl)
                || strcmp(tmp_ifname, ifname) != 0 || prefix != pref) {
            continue;
        }

        memset(&rt, 0, sizeof(rt));
        rt.rtmsg_dst_len = prefix;
        rt.rtmsg_ifindex = ifindex;
        rt.rtmsg_flags = flags;

        for (i = 0; i < 16; i++) {
            rt.rtmsg_gateway.s6_addr[i] = (__u8) gw[i];
            rt.rtmsg_dst.s6_addr[i] = (__u8) dst[i];
        }

        inet_ntop(AF_INET6, rt.rtmsg_gateway.s6_addr, gw_str, sizeof(gw_str));
        RilLogV("%s(): remove gw %s", __FUNCTION__, gw_str);

        if ((result = ioctl(skfd, SIOCDELRT, &rt)) < 0) {
            RilLogE("Failed to remove ipv6 route for %s to gw %s dst %s: %s",
                    tmp_ifname, gw_str,
                    inet_ntop(AF_INET6, rt.rtmsg_dst.s6_addr, dst_str, sizeof(dst_str)),
                    strerror(errno));
        }
    }

    fclose(fp);
    return 0;
}
#endif

/****************************************
 * Wrapper functions
 ****************************************/
int NetIfController::GetIfSock()
{
    return get_if_sock();
}

int NetIfController::GetIfSockV6()
{
    return get_if_sock_v6();
}

short NetIfController::GetIfFlags(int skfd, char *ifname)
{
    return get_if_flags(skfd, ifname);
}

bool NetIfController::SetIfFlags(int skfd, char *ifname, short flags)
{
    if (set_if_flags(skfd, ifname, flags) < 0)
        return false;
    return true;
}

bool NetIfController::SetIfUp(int skfd, char *ifname, short flags)
{
    if (set_if_up(skfd, ifname, flags) < 0)
        return false;
    return true;
}

bool NetIfController::SetIfDown(int skfd, char *ifname, short flags)
{
    if (set_if_down(skfd, ifname, flags) < 0)
        return false;
    return true;
}

bool NetIfController::SetIfAddr(int skfd, char *ifname, unsigned char *addr)
{
    if (set_if_addr(skfd, ifname, addr) < 0)
        return false;
    return true;
}

bool NetIfController::ClearIfAddr(int skfd, char *ifname)
{
    if (clear_if_addr(skfd, ifname) < 0)
        return false;
    return true;
}

bool NetIfController::GetLastError()
{
    return last_errno;
}

int NetIfController::GetIfIndex(int skfd, const char *ifname)
{
    return get_if_index(skfd, ifname);
}

bool NetIfController::SetIfAddrIpv6(int skfd, char *ifname, int index, unsigned char *addr)
{
    if (set_if_addr_v6(skfd, ifname, index, addr) < 0)
        return false;
    return true;
}

bool NetIfController::SetIfAddrIpv6(int skfd, char *ifname, unsigned char *addr)
{
    int index = get_if_index(skfd, ifname);
    if (index < 0) {
        return false;
    }

    if (set_if_addr_v6(skfd, ifname, index, addr) < 0)
        return false;
    return true;
}

bool NetIfController::ClearIfGlobalAddress(int skfd, char *ifname, int index)
{
    if (clear_if_global_addr_v6(skfd, ifname, index) < 0)
        return false;
    return true;
}

bool NetIfController::ClearIfLinkLocalAddress(int skfd, char *ifname, int index)
{
    if (clear_if_linklocal_addr_v6(skfd, ifname, index) < 0)
            return false;
    return true;
}

int NetIfController::SetIfMaxRsCount(const char *ifname, int count)
{
    return set_if_max_rs_count_v6(ifname, count);
}

int NetIfController::GetIfMaxRsCount(const char *ifname)
{
    return get_if_ipv6conf_sysproc(ifname, rs_propname[RS_MAXCOUNT]);
}

int NetIfController::SetIfRsDelay(const char *ifname, int delay)
{
    // TBD
    return 0;
}

int NetIfController::GetIfRsDelay(const char *ifname)
{
    return get_if_ipv6conf_sysproc(ifname, rs_propname[RS_DELAY]);
}

int NetIfController::SetIfRsInterval(const char *ifname, int interval)
{
    // TBD
    return 0;
}

int NetIfController::GetIfRsInterval(const char *ifname)
{
    return get_if_ipv6conf_sysproc(ifname, rs_propname[RS_INTERVAL]);
}

/****************************************
 * NetIfController
 ****************************************/
NetIfController::NetIfController(PdpContext *pPdpContext) : m_skfd(-1), m_skfdv6(-1)
{
    SetPdpContext(pPdpContext);
}

NetIfController::~NetIfController()
{
    SetPdpContext(NULL);
}

void NetIfController::SetPdpContext(PdpContext *pPdpContext)
{
    m_pPdpContext = pPdpContext;
    Reset();
}

bool NetIfController::BringUp()
{
    RilLogI("[%s] %s", TAG, __FUNCTION__);

    if (m_skfd <= 0 || m_skfdv6 <= 0) {
        if (debug) RilLogW("[%s] %s invalid socket, may be need to reset m_skfd:%d, m_skfdv6:%d, will return false", TAG, __FUNCTION__, m_skfd, m_skfdv6);
        return false;
    }

    DataCall *dc = m_pPdpContext->GetDataCallInfo();
    if (dc != NULL && dc->active >= ACTIVE_AND_LINKDOWN) {
        bool ipset = false;
        bool ipv6set = false;
        // TBD: Consider changing IP Check,
        //      IN RFC4193, fc00::/7 is unique local address,
        //      This address range can be assigned as IP by Network
        bool needToIpv6Rs = (dc->ipv6.valid && dc->ipv6.addr[0] == 0xFE && dc->ipv6.addr[1] == 0x80);

        // For RIL Reset case or any abnormal case, initialze interface
        TearDown();

//#define EMULATE_IPV4V6 // For on SKT-live test(IPV6 only network)
#ifdef EMULATE_IPV4V6
        static bool emulation_ipv4=false;
        if(emulation_ipv4 || (!dc->ipv4.valid && dc->ipv6.valid))
        {
            if(needToIpv6Rs){
                emulation_ipv4=true;
                dc->ipv4.valid=true;
                inet_pton(AF_INET, "192.168.0.224", (void *)&dc->ipv4.addr);
                RilLogV("emulated ipv4:%lu\n", (unsigned long)dc->ipv4.addr);
            }
            else{
                emulation_ipv4=false;
                dc->ipv4.valid=false;
                memset(&dc->ipv4.addr, 0, sizeof(dc->ipv4.addr));
                RilLogV("emulated ipv4:%lu\n", (unsigned long)dc->ipv4.addr);
            }
        }
#endif
        // IPv4 configuration
        if (dc->ipv4.valid) {
            if (debug) RilLogV("[%s] %s IPv4 address is valid", TAG, __FUNCTION__);
            if (memcmp(nullIpv4, dc->ipv4.addr, sizeof(nullIpv4)) != 0) {
                ipset = SetIfAddr(m_skfd, m_ifname, dc->ipv4.addr);
                if (ipset)
                    RilLogV("[%s] %s SetIfAddr success", TAG, __FUNCTION__);
            }
            else {
                RilLogW("[%s] %s IPv4 is valid but null address", TAG, __FUNCTION__);
            }
        }
        else {
            if (debug) RilLogV("[%s] %s IPv4 address is not valid", TAG, __FUNCTION__);
            ipset = ClearIfAddr(m_skfd, m_ifname);
            if (ipset){
                RilLogV("[%s] %s ClearIfAddr success", TAG, __FUNCTION__);
                ipset=false;
            }
        }

        // IPv6 configuration
        if (dc->ipv6.valid) {
            if (debug) RilLogV("[%s] %s IPv6 address is valid", TAG, __FUNCTION__);
            if (memcmp(nullIpv6, dc->ipv6.addr, sizeof(nullIpv6)) != 0) {
                // SetIfAddrIpv6 requires disable_ipv6=0
                set_if_enabled_IPv6(m_ifname, true);

                short flags = GetIfFlags(m_skfdv6, m_ifname);
                if (!SetIfUp(m_skfdv6, m_ifname, flags)) {
                    if (debug) RilLogW("[%s] %s fail to SetIfUp for skfdv6:%d", TAG, __FUNCTION__, m_skfdv6);
                }

                if (debug) RilLogW("[%s] %s no need to set Link Local", TAG, __FUNCTION__);
#if 0
                if (!needToIpv6Rs) {
                    // Need to add Link-Local Address first
                    if (debug) RilLogW("[%s] %s Set Link-Local address from Global address skfdv6:%d", TAG, __FUNCTION__, m_skfdv6);
                    unsigned char temp_ipv6addr[16];
                    memset(&temp_ipv6addr[0], 0, 8);
                    temp_ipv6addr[0] = 0xFE;
                    temp_ipv6addr[1] = 0x80;
                    memcpy(&temp_ipv6addr[8], &dc->ipv6.addr[8], sizeof(char)*8);

                    if(!SetIfAddrIpv6(m_skfdv6, m_ifname, temp_ipv6addr))
                        RilLogE("[%s] %s Set Link-Local address from Global address skfdv6:%d failed", TAG, __FUNCTION__, m_skfdv6);
                }
#endif
                // This trigger addrconf_dad_completed -> ndisc_send_rs in kernel, requires IF_UP(vnet_open: rmnet*)
                ipv6set = SetIfAddrIpv6(m_skfdv6, m_ifname, dc->ipv6.addr);
                if (ipv6set)
                    RilLogV("%s %s set_if_addr_v6 success", TAG, __FUNCTION__);

                set_if_enabled_auto_conf_v6(m_ifname, false);
                if (needToIpv6Rs) {
                    set_if_max_rs_count_v6(m_ifname, 3);
                }
                set_if_enabled_dad_v6(m_ifname, false);
                set_if_max_dad_count_v6(m_ifname, 0);
            }
            else {
                RilLogW("[%s] %s IPv6 is valid but null address", TAG, __FUNCTION__);
            }
        }
        else {
            if (debug) RilLogW("[%s] %s IPv6 address is not valid", TAG, __FUNCTION__);
            set_if_enabled_IPv6(m_ifname, false);
        }

        if (ipset){
            short flags = GetIfFlags(m_skfd, m_ifname);
            if (!SetIfUp(m_skfd, m_ifname, flags)) {
                if (debug) RilLogW("[%s] %s fail to SetIfUp for skfd:%d", TAG, __FUNCTION__, m_skfd);
            }
        }

        // Strict Fail Policy : If we get valid ip, we should set IP as Interface address
        bool needToSetIpAddr = (dc->ipv4.valid && !ipset) || (dc->ipv6.valid && !ipv6set);
        // Loose Fail Policy : If we can set at least one valid ip address, it's ok
        //bool needToSetIpAddr = (dc->ipv4.valid && !ipset) && (dc->ipv6.valid && !ipv6set);
        if (needToSetIpAddr) {
            if (debug) RilLogW("[%s] %s fail to SetIfAddr", TAG, __FUNCTION__);
            TearDown();
            return false;
        }
    }

    return true;
}

bool NetIfController::TearDown()
{
    RilLogI("[%s] %s", TAG, __FUNCTION__);

    if (m_skfd <= 0 || m_skfdv6 <= 0){
        if (debug) RilLogW("[%s] %s invalid socket, may be need to reset m_skfd:%d, m_skfdv6:%d, will return false", TAG, __FUNCTION__, m_skfd, m_skfdv6);
        return false;
    }

    DataCall *dc = m_pPdpContext->GetDataCallInfo();
    if (dc != NULL && dc->active >= ACTIVE_AND_LINKDOWN) {
        // clear IPv4 address
        if (!ClearIfAddr(m_skfd, m_ifname)) {
            if (debug) RilLogW("[%s] %s fail to ClearIfAddr", TAG, __FUNCTION__);
        }
        // TODO clear IPv6 address
        /*
        int index = GetIfIndex(m_skfdv6, m_ifname);
        if (index >= 0) {
            // Skip All address clear, This will be done by Kernel when Interface down
            ClearIfGlobalAddress(m_skfdv6, m_ifname, index);
            ClearIfLinkLocalAddress(m_skfdv6, m_ifname, index);
            //clear_if_routes_v6(m_skfdv6, m_ifname, index, 0, 0);
            //clear_if_routes_v6(m_skfdv6, m_ifname, index, RTF_GATEWAY|RTF_ADDRCONF, 0);
        }
        */

        short flags = GetIfFlags(m_skfd, m_ifname);
        if (SetIfDown(m_skfd, m_ifname, flags)) {
            return true;
        }
        else {
            if (debug) RilLogW("[%s] %s fail to SetIfDown", TAG, __FUNCTION__);
        }
    }

    return false;
}

void NetIfController::Reset()
{
    RilLogI("[%s] %s", TAG, __FUNCTION__);

    if (m_skfd > 0) {
        close(m_skfd);
    }

    if (m_skfdv6 > 0) {
        close(m_skfdv6);
    }

    m_skfd = m_skfdv6 = -1;
    memset(m_ifname, 0, sizeof(m_ifname));

    if (m_pPdpContext != NULL) {
        SECURELIB::strncpy(m_ifname, sizeof(m_ifname), m_pPdpContext->GetInterfaceName(), SECURELIB::strlen(m_pPdpContext->GetInterfaceName()));

        m_skfd = GetIfSock();
        m_skfdv6 = get_if_sock_v6();
    }
    RilLogV("[%s] %s success m_skfd:%d, m_skfdv6:%d", TAG, __FUNCTION__, m_skfd, m_skfdv6);
}
