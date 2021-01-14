#define LOG_TAG "tetheroffload@1.0-service"

#include <hidl/LegacySupport.h>

#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <net/if.h>

#include "Tetheroffload.h"
#include "Listener.h"
#include "exynos-dit-ioctl.h"


int nl_handle_msg(struct msghdr *msgh, unsigned int len)
{
	struct iovec *iov = msgh->msg_iov;
	struct nlmsghdr *nlh = (struct nlmsghdr *)iov->iov_base;
	char name[IFNAMSIZ];
	char address[128];

	while ((NLMSG_OK(nlh, len)) && (nlh->nlmsg_type != NLMSG_DONE))
	{
		switch(nlh->nlmsg_type)
		{
			case RTM_NEWLINK:
			{
				struct ifinfomsg  *pinfo = (struct ifinfomsg *)NLMSG_DATA(nlh);
				len -= sizeof(struct nlmsghdr);

				ALOGI("RTM_NEWLINK, ifi_change:%d\n", pinfo->ifi_change);
				ALOGI("RTM_NEWLINK, ifi_flags:%d\n", pinfo->ifi_flags);
				ALOGI("RTM_NEWLINK, ifi_index:%d\n", pinfo->ifi_index);
				ALOGI("RTM_NEWLINK, family:%d\n", pinfo->ifi_family);
				break;
			}
			case RTM_DELLINK:
			{
				struct ifinfomsg  *pinfo = (struct ifinfomsg *)NLMSG_DATA(nlh);
				len -= sizeof(struct nlmsghdr);

				ALOGI("RTM_DELLINK, ifi_change:%d\n", pinfo->ifi_change);
				ALOGI("RTM_DELLINK, ifi_flags:%d\n", pinfo->ifi_flags);
				ALOGI("RTM_DELLINK, ifi_index:%d\n", pinfo->ifi_index);
				ALOGI("RTM_DELLINK, family:%d\n", pinfo->ifi_family);
				break;
			}
			case RTM_NEWADDR:
			{
				struct ifaddrmsg  *ifa = (struct ifaddrmsg *)NLMSG_DATA(nlh);
				len -= sizeof(struct nlmsghdr);

				struct rtattr *rth = IFA_RTA(ifa);
				int rtl = IFA_PAYLOAD(nlh);

				while (rtl && RTA_OK(rth, rtl)) {
					if (rth->rta_type == IFA_ADDRESS) {
						memset(address, 0, sizeof(address));
						inet_ntop(ifa->ifa_family, RTA_DATA(rth), address, sizeof(address));
						if_indextoname(ifa->ifa_index, name);
						ALOGI("%s is now %s\n", name, address);
					}
					rth = RTA_NEXT(rth, rtl);
				}
				break;
			}
			case RTM_NEWROUTE:
			{
				struct rtattr *rtah = RTM_RTA(NLMSG_DATA(nlh));
				len -= sizeof(struct nlmsghdr);

				while(RTA_OK(rtah, len))
				{
					memset(address, 0, sizeof(address));
					switch(rtah->rta_type)
					{
					case RTA_DST:
						inet_ntop(AF_INET, rtah, address, sizeof(address));
						ALOGI("%s %s\n", "RTA_DST", address);
						break;

					case RTA_SRC:
						inet_ntop(AF_INET, rtah, address, sizeof(address));
						ALOGI("%s %s\n", "RTA_SRC", address);
						break;

					case RTA_GATEWAY:
						inet_ntop(AF_INET, rtah, address, sizeof(address));
						ALOGI("%s %s\n", "RTA_GATEWAY", address);
						break;

					case RTA_IIF:
						ALOGI("%s %s\n", "RTA_IIF", address);
						break;

					case RTA_OIF:
						ALOGI("%s %s\n", "RTA_OIF", address);
						break;

					case RTA_PRIORITY:
						ALOGI("%s %s\n", "RTA_PRIORITY", address);
						break;

					default:
						break;

					}

					/* Advance to next attribute */
					rtah = RTA_NEXT(rtah, len);
				}

				break;
			}
			case RTM_DELROUTE:
			{
				break;
			}
			case RTM_NEWNEIGH:
			{
				break;
			}
			case RTM_DELNEIGH:
			{
				break;
			}
			default:
				break;
		}
		nlh = NLMSG_NEXT(nlh, len);

	}
	return 0;
}

struct msghdr* nl_alloc_msg(int msglen)
{
	unsigned char *buf = NULL;
	struct sockaddr_nl *nladdr = NULL;
	struct iovec *iov = NULL;
	struct msghdr *msgh;

	msgh = (struct msghdr *)malloc(sizeof(struct msghdr));
	if(msgh == NULL)
	{
		ALOGE("Failed malloc for msghdr\n");
		return NULL;
	}

	nladdr = (struct sockaddr_nl *)malloc(sizeof(struct sockaddr_nl));
	if(nladdr == NULL)
	{
		ALOGE("Failed malloc for sockaddr\n");
		free(msgh);
		return NULL;
	}

	iov = (struct iovec *)malloc(sizeof(struct iovec));
	if(iov == NULL)
	{
		ALOGE("Failed malloc for iovec");
		free(nladdr);
		free(msgh);
		return NULL;
	}

	buf = (unsigned char *)malloc(msglen);
	if(buf == NULL)
	{
		ALOGE("Failed malloc for mglen\n");
		free(iov);
		free(nladdr);
		free(msgh);
		return NULL;
	}

	memset(nladdr, 0, sizeof(struct sockaddr_nl));
	nladdr->nl_family = AF_NETLINK;

	memset(msgh, 0x0, sizeof(struct msghdr));
	msgh->msg_name = nladdr;
	msgh->msg_namelen = sizeof(struct sockaddr_nl);
	msgh->msg_iov = iov;
	msgh->msg_iovlen = 1;

	memset(iov, 0x0, sizeof(struct iovec));
	iov->iov_base = buf;
	iov->iov_len = msglen;

	return msgh;
}

void nl_release_msg(struct msghdr *msgh)
{
	unsigned char *buf = NULL;
	struct sockaddr_nl *nladdr = NULL;
	struct iovec *iov = NULL;

	if(NULL == msgh)
	{
		return;
	}

	nladdr = (struct sockaddr_nl *)msgh->msg_name;
	iov = msgh->msg_iov;
	if(msgh->msg_iov)
		buf = (unsigned char *)msgh->msg_iov->iov_base;

	if(buf)
		free(buf);

	if(iov)
		free(iov);

	if(nladdr)
		free(nladdr);

	if(msgh)
		free(msgh);
	return;
}

void *netlink_event_thread(void *arg)
{
	int fd;
	unsigned int nl_type = NETLINK_ROUTE;
	unsigned int nl_group = RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE | RTMGRP_LINK |
			RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR | RTMGRP_NEIGH |
			RTNLGRP_IPV6_PREFIX;
	struct sockaddr_nl nl_addr;
	fd_set fdset;
	struct msghdr *msgh;
	struct timeval tv;

	arg = NULL;

	if((fd = socket(AF_NETLINK, SOCK_RAW, nl_type)) < 0) {
	}

	nl_addr.nl_family = AF_NETLINK;
	nl_addr.nl_pid = getpid();
	nl_addr.nl_groups = nl_group;

	if(bind(fd, (struct sockaddr *)&nl_group, sizeof(struct sockaddr_nl)) < 0) {
	}

	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = 1;

	do {
		int len;

		if (select(1, &fdset, NULL, NULL, &tv) < 0) {
			break;
		}

		if (FD_ISSET(fd, &fdset)) {
			msgh = nl_alloc_msg(2048);
			len = recvmsg(fd, msgh, 0);

			if (len > 0)
				nl_handle_msg(msgh, len);

			nl_release_msg(msgh);
		}
	} while (!gHAL->offloadstopped());

	ALOGI("%s exit", __func__);

	return nullptr;
}

