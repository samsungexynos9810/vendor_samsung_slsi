#define LOG_TAG "tetheroffload@1.0-service"

#include <hidl/LegacySupport.h>
#include <pthread.h>
#include <poll.h>

#include "Tetheroffload.h"
#include "Listener.h"
#include "exynos-dit-ioctl.h"

extern "C"
{
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>
}


const unsigned int UDP_GROUPS = NF_NETLINK_CONNTRACK_NEW
		| NF_NETLINK_CONNTRACK_DESTROY;
const unsigned int TCP_GROUPS = NF_NETLINK_CONNTRACK_UPDATE
		| NF_NETLINK_CONNTRACK_DESTROY;


pthread_mutex_t wait_fd_mutex;
pthread_cond_t  wait_fd_cond;

int UDP_fd, TCP_fd;

void conntrack_cond_wait_init(void)
{
	pthread_mutex_init(&wait_fd_mutex, NULL);
	pthread_cond_init(&wait_fd_cond, NULL);
}

int passHandleConntrack(int fd1, int fd2)
{
	int on = 1;

	if (setsockopt(fd1, SOL_NETLINK, NETLINK_NO_ENOBUFS, &on, sizeof(int)) < 0)
		return -1;

	if (setsockopt(fd2, SOL_NETLINK, NETLINK_NO_ENOBUFS, &on, sizeof(int)) < 0)
		return -1;

	UDP_fd = fd1;
	TCP_fd = fd2;

	pthread_mutex_lock(&wait_fd_mutex);
	pthread_cond_signal(&wait_fd_cond);
	pthread_mutex_unlock(&wait_fd_mutex);

	return 0;
}

int init_conntrack_filter(struct nfct_filter *filter, int groups)
{
	int ret = 0;
	struct nfct_filter_proto fstate;

	ALOGI("%s", __func__);

	if (TCP_GROUPS == groups) {
		ret = nfct_filter_set_logic(filter,
				NFCT_FILTER_L4PROTO,
				NFCT_FILTER_LOGIC_POSITIVE);

		if(ret == -1) {
			ALOGE("fail to set filter for L4PROTO (err=%d)", ret);
			return -1;
		}

		nfct_filter_add_attr_u32(filter, NFCT_FILTER_L4PROTO, IPPROTO_TCP);

		ret = nfct_filter_set_logic(filter,
				NFCT_FILTER_L4PROTO_STATE,
				NFCT_FILTER_LOGIC_POSITIVE);

		if(ret == -1) {
			ALOGE("fail to set filter for L4PROTO_STATE (err=%d)", ret);
			return -1;
		}

		fstate.proto = IPPROTO_TCP;
		fstate.state = TCP_CONNTRACK_ESTABLISHED;

		nfct_filter_add_attr(filter,
				NFCT_FILTER_L4PROTO_STATE,
				&fstate);

		fstate.proto = IPPROTO_TCP;
		fstate.state = TCP_CONNTRACK_FIN_WAIT;

		nfct_filter_add_attr(filter,
				NFCT_FILTER_L4PROTO_STATE,
				&fstate);
	}else {
		ret = nfct_filter_set_logic(filter,
				NFCT_FILTER_L4PROTO,
				NFCT_FILTER_LOGIC_POSITIVE);
		if(ret == -1) {
			ALOGE("fail to set filter for L4PROTO (err=%d)", ret);
			return -1;
		}

		nfct_filter_add_attr_u32(filter, NFCT_FILTER_L4PROTO, IPPROTO_UDP);
	}

	return 0;
}

void UpdateForwardInfo(struct nf_conntrack *ct, enum nf_conntrack_msg_type type)
{
	u_int8_t l4proto = 0;

	ALOGI("%s type=%d", __func__, type);

	l4proto = nfct_get_attr_u8(ct, ATTR_ORIG_L4PROTO);

	if(IPPROTO_UDP == l4proto) {
		ALOGI("%s UPD ct", __func__);
	} else if (IPPROTO_TCP == l4proto) {
		ALOGI("%s TCP ct", __func__);
	}

	return;
}

int conntrack_event_handler(enum nf_conntrack_msg_type type,
	struct nf_conntrack *ct, void *data)
{
	uint8_t ip_type = 0;

	ALOGI("%s", __func__);

	data = NULL;

	/* Retrieve ip type */
	ip_type = nfct_get_attr_u8(ct, ATTR_REPL_L3PROTO);

	if(AF_INET == ip_type) {
		ALOGI("%s AF_INET", __func__);

		UpdateForwardInfo(ct, type);
	}

	nfct_destroy(ct);
	return NFCT_CB_STOLEN;

}

void *conntrack_thread(void *arg)
{
	struct nfct_filter *filter;
	struct nfct_handle *cth;
	unsigned int groups = arg ? TCP_GROUPS : UDP_GROUPS;

	int fd;
	int fd_nf;
	int ret;
	int on = 1;

	pthread_cond_wait(&wait_fd_cond, &wait_fd_mutex);

	fd = arg ? TCP_fd : UDP_fd;

	ALOGI("%s %s", __func__, arg ? "TCP": "UDP");

	filter = nfct_filter_create();

	fd_nf = dup(fd);
	cth = nfct_open2(CONNTRACK, groups, fd_nf);

	setsockopt(fd_nf, SOL_NETLINK, NETLINK_NO_ENOBUFS, &on, sizeof(int));

	init_conntrack_filter(filter, groups);

	ret = nfct_filter_attach(nfct_fd(cth), filter);
	if(ret == -1)
	{
		ALOGE("unable to attach filter (%s)\n", strerror(errno));
		goto exit;
	}

	if (TCP_GROUPS == groups) {
		nfct_callback_register(cth,
				(nf_conntrack_msg_type) (NFCT_T_UPDATE | NFCT_T_DESTROY | NFCT_T_NEW),
				conntrack_event_handler, gHAL);
	} else {
		nfct_callback_register(cth,
				(nf_conntrack_msg_type)(NFCT_T_NEW | NFCT_T_DESTROY),
				conntrack_event_handler, gHAL);
	}

wait:
	ret = nfct_catch(cth);
	if(ret == -1) {
		ALOGE("%s %s\n", __func__, strerror(errno));
		goto exit;
	}
	else {
		if (!gHAL->offloadstopped())
			goto wait;
	}

exit:
	nfct_filter_destroy(filter);
	nfct_callback_unregister(cth);
	nfct_close2(cth, true);

	ALOGI("%s exit", __func__);

	return NULL;
}
