/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "tetheroffload@1.0-service"

#include <utils/Log.h>
#include <pthread.h>
#include <poll.h>

#include <arpa/inet.h>
#include <net/if.h>

#define DEBUG


#include "Tetheroffload.h"
#include "Listener.h"
#include "exynos-dit-ioctl.h"

using ::android::hardware::tetheroffload::control::V1_0::ITetheringOffloadCallback;
using ::android::hardware::tetheroffload::control::V1_0::OffloadCallbackEvent;

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace tetheroffload {
namespace V1_0 {
namespace implementation {


Tetheroffload::Tetheroffload()
{
    mFd1 = nullptr;
	mFd2 = nullptr;
	mUpstream.iface = "";
	mCtrlCb.clear();
	was_interrupted = false;
}

Tetheroffload::~Tetheroffload()
{
    mFd1.setTo(nullptr, true);
    mFd2.setTo(nullptr, true);

	mLocalPrefixes.clear();
	mDownstreams.clear();
	threads.clear();
}

status_t Tetheroffload::registerService()
{
	status_t ret = 0;

	ret = IOffloadControl::registerAsService();
	if (ret != 0) {
		ALOGE("Failed to register IOffloadControl (%d)", ret);
		return ret;
	}

	ret = IOffloadConfig::registerAsService();
	if (ret != 0) {
		ALOGE("Failed to register IOffloadControl (%d)", ret);
		return ret;
	}
	return 0;
}

void Tetheroffload::StartThreads(void)
{
	int ret;
	pthread_t tid;

	ALOGI("%s called", __func__);

#if 0
	conntrack_cond_wait_init();
	ret = pthread_create(&tid, 0, conntrack_thread, 0);
	if(0 != ret) {
		ALOGE("%s fail to create UDP_GROUPS thread", __func__);
		return;
	}
	pthread_setname_np(tid, "udp_ct_listener");
	threads.push_back(tid);

	ret = pthread_create(&tid, 0, conntrack_thread, 1);
	if(0 != ret) {
		ALOGE("%s fail to create TCP_GROUPS thread", __func__);
		return;
	}
	pthread_setname_np(tid, "tcp_ct_listener");
	threads.push_back(tid);
#endif

	ret = pthread_create(&tid, 0, drv_event_thread, NULL);
	if(0 != ret) {
		ALOGE("%s fail to create drv_event_thread", __func__);
		return;
	}
	pthread_setname_np(tid, "drv_event_thread");
	threads.push_back(tid);
#if 0
	ret = pthread_create(&tid, 0, netlink_event_thread, NULL);
	if(0 != ret) {
		ALOGE("%s fail to create netlink_event_thread", __func__);
		return;
	}
	pthread_setname_np(tid, "netlink_event_thread");
	threads.push_back(tid);
#endif
	return;
}

status_t Tetheroffload::ThreadJoin()
{
	for (unsigned int i = 0; i < threads.size(); i++) {
		pthread_t tid = threads[i];
		pthread_join(tid, NULL);
	}

	ALOGI("%s done", __func__);

	return 0;
}

bool Tetheroffload::offloadstopped()
{
	return was_interrupted;
}

void Tetheroffload::offload_callback_event(int32_t event)
{
	if(mCtrlCb)
		mCtrlCb->onEvent((OffloadCallbackEvent)event);

	return;
}

bool Tetheroffload::isInitialized() {
	return mCtrlCb.get() != nullptr;
}

int Tetheroffload::parseAddress(string in, string &out, string &netmask) {
	int af = AF_UNSPEC;
	size_t pos = in.find(":");

	/* address family */
	if (pos != string::npos)
		af = AF_INET6;
	else
		af = AF_INET;

	/* extract address and netmask */
	pos = in.find("/");

	if (pos != string::npos && (pos >= 1 && pos < in.size())) {
		out = in.substr(0, pos);
		netmask = in.substr(pos + 1);
	} else if (pos == string::npos) {
		out = in;
		netmask = "";
	} else {
		return -1;
	}

	/* validate address */
	if (af == AF_INET6) {
		struct sockaddr_in6 sa;

		if (inet_pton(AF_INET6, out.c_str(), &(sa.sin6_addr)) <= 0)
			return -1;
	}
	else {
		struct sockaddr_in sa;

		if (inet_pton(AF_INET, out.c_str(), &(sa.sin_addr)) <= 0)
			return -1;
	}

	return af;
}

int Tetheroffload::checkInterfaceStat(string name)
{
	int fd;
	int if_index;
	struct ifreq ifr;

	ALOGI("interface name (%s)\n", name.c_str());

	if(name.length() >= sizeof(ifr.ifr_name)) {
		ALOGE("wrong strnlen");
		return -1;
	}

	if((name.find("rmnet") != string::npos)
		|| (name.find("dummy") != string::npos)) {
		return 1;
	}

	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		ALOGE("failed create socket (errno=%d", errno);
		return -1;
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name.c_str(), sizeof(ifr.ifr_name));

	if(ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
		ALOGE("failed SIOCGIFINDEX");
		close(fd);
		return -1;
	}

	if_index = ifr.ifr_ifindex;
	ALOGI("interface index %d\n", if_index);
	close(fd);
	return 1;
}

int Tetheroffload::ioctlOffload(int cmd, void *arg)
{
	int ret = 0, fd = -1;
	void *param = arg;

	if ((fd = open(DEVICE_OFFLOAD, O_RDWR | O_CLOEXEC)) < 0) {
		ALOGE("Failed opening %s.\n", DEVICE_OFFLOAD);
		return -1;
	}

#if 0
	switch(cmd) {
	case OFFLOAD_IOCTL_INIT_OFFLOAD:
		break;
	case OFFLOAD_IOCTL_STOP_OFFLOAD:
		break;
	case OFFLOAD_IOCTL_SET_LOCAL_PRFIX:
		break;
	case OFFLOAD_IOCTL_GET_FORWD_STATS:
		break;
	case OFFLOAD_IOCTL_SET_DATA_LIMIT:
		break;
	case OFFLOAD_IOCTL_SET_UPSTRM_PARAM:
		break;
	case OFFLOAD_IOCTL_ADD_DOWNSTREAM:
		break;
	case OFFLOAD_IOCTL_REMOVE_DOWNSTRM:
		break;
	case OFFLOAD_IOCTL_CONF_SET_HANDLES:
		break;
	default:
		break;
	}
#endif

	if (ioctl(fd, cmd, param) < 0) {
		ret = -1;
	}

	close(fd);
	return ret;
}

/* -------------------------- IOffloadConfig -------------------------------- */
Return<void> Tetheroffload::setHandles(const hidl_handle &fd1, const hidl_handle &fd2,
	setHandles_cb hidl_cb)
{
	ALOGI("%s called", __func__);

	if (fd1->numFds != 1) {
		hidl_cb(false, "Failed Input Checks");
		return Void();
	}

	if (fd1->numFds != 1) {
		hidl_cb(false, "Failed Input Checks");
		return Void();
	}

	mFd1 = fd1; mFd2 = fd2;

	if (ioctlOffload(OFFLOAD_IOCTL_CONF_SET_HANDLES, NULL) < 0) {
		hidl_cb(false, "can't connect offload hw");
		return Void();
	}

	if (passHandleConntrack(mFd1->data[0], mFd2->data[0]) < 0) {
		hidl_cb(false, "Failed Input Checks");
		return Void();
	}

	hidl_cb(true, NULL);
	return Void();
}

/* -------------------------- IOffloadControl ------------------------------- */
Return<void> Tetheroffload::initOffload(const sp<ITetheringOffloadCallback>& cb,
	initOffload_cb hidl_cb)
{
	ALOGI("%s called", __func__);

	if (isInitialized()) {
		ALOGI("Already initialized");
		hidl_cb(false, "Already initialized");
		return Void();
	}

	mCtrlCb = cb;

	if (ioctlOffload(OFFLOAD_IOCTL_INIT_OFFLOAD, NULL) < 0) {
		hidl_cb(false, "can't connect offload hw");
		return Void();
	}

	hidl_cb(true, NULL);
	return Void();
}

Return<void> Tetheroffload::stopOffload(stopOffload_cb hidl_cb)
{
	ALOGI("%s called", __func__);

	if (!isInitialized()) {
		hidl_cb(false, "Not initialized");
		ALOGI("Not initialized");
		return Void();
	}

	mCtrlCb.clear();

	if (mUpstream.iface.empty()) {
		hidl_cb(false, "Upstream empty");
		ALOGI("Upstream empty");
		return Void();
	}
#if 0
	if (checkInterfaceStat(mUpstream.iface) < 0) {
		ALOGE("Failed get interface stat %s\n", mUpstream.iface.c_str());
		hidl_cb(false, "Failed get interface stat");
		return Void();
	}
#endif
	if (ioctlOffload(OFFLOAD_IOCTL_STOP_OFFLOAD, NULL) < 0) {
		hidl_cb(false, "can't connect offload hw");
		return Void();
	}

	hidl_cb(true, NULL);
	return Void();
}

Return<void> Tetheroffload::setLocalPrefixes(const hidl_vec<hidl_string>& prefixes,
	setLocalPrefixes_cb hidl_cb)
{
	stringstream ss;

	ALOGI("%s called", __func__);

	if (!isInitialized()) {
		ALOGI("Not initialized");
		hidl_cb(false, "Not initialized");
		return Void();
	} else if(prefixes.size() < 1) {
		ALOGI("Failed Input Checks");
		hidl_cb(false, "Failed Input Checks");
		return Void();
	}

    for (size_t i = 0; i < prefixes.size(); i++) {
        string prfix = prefixes[i];
		string addr, netmask;

		if (!prfix.empty() && (parseAddress(prfix, addr, netmask) < 0)) {
			ALOGI("Failed Input Checks(prfix)");
			hidl_cb(false, "Failed Input Checks(prfix)");
			return Void();
		}

        mUpstream.uinfo.v6Gws.push_back(addr);
		ss << prefixes[i];
		ss << " ";
    }

	ALOGI("%s prefixes: [%s]", __func__, ss.str().c_str());
	ss.str("");

	/* update only delta */
	/* [ff00::/8,192.168.42.129/32,192.168.49.0/24,127.0.0.0/8,169.254.0.0/16,fe80::/64,fc00::/7,::/3] */
	/* [ff00::/8,100.90.249.41/32,192.168.42.129/32,....] */
	for (size_t i = 0; i < prefixes.size(); i++) {
		size_t m;
		string prfix = prefixes[i];
		string addr, netmask;

		if (parseAddress(prfix, addr, netmask) < 0) {
			ALOGI("Failed Input Checks");
			hidl_cb(false, "Failed Input Checks");
			return Void();
		}
		ss << prfix;
		ss << " ";

		for (m = 0; m < mLocalPrefixes.size(); m++) {
			if (mLocalPrefixes[m] == addr)
				break;
		}

		if (m >= mLocalPrefixes.size()) {
			mLocalPrefixes.push_back(addr);
			ALOGI("%s added", addr.c_str());
		}
	}

	ALOGI("%s prefixes: [%s]", __func__, ss.str().c_str());

	hidl_cb(true, NULL);

	return Void();
}

Return<void> Tetheroffload::getForwardedStats(const hidl_string& upstream,
	getForwardedStats_cb hidl_cb)
{
	struct forward_stat stat;

	if (!isInitialized()) {
		ALOGI("Not initialized");
		hidl_cb(0, 0);
		return Void();
	}

	if (strcmp(mUpstream.iface.c_str(), upstream.c_str()) != 0) {
		ALOGI("Failed Input Checks");
		hidl_cb(0, 0);
		return Void();
	}

	if (checkInterfaceStat(upstream) < 0) {
		ALOGE("Failed get interface stat %s\n", upstream.c_str());
		hidl_cb(0, 0);
		return Void();
	}

	if (ioctlOffload(OFFLOAD_IOCTL_GET_FORWD_STATS, &stat) < 0) {
		hidl_cb(0, 0);
		return Void();
	}

	mRxBytes = stat.rxBytes;
	mTxBytes = stat.txBytes;

	ALOGI("%s %s mRxBytes=%lu, mTxBytes=%lu", __func__, upstream.c_str(), mRxBytes, mTxBytes);

	hidl_cb(mRxBytes, mTxBytes);
	return Void();
}

Return<void> Tetheroffload::setDataLimit(const hidl_string& upstream, uint64_t limit,
	setDataLimit_cb hidl_cb)
{
	ALOGI("%s %s %lu", __func__, upstream.c_str(), limit);

	if (!isInitialized()) {
		hidl_cb(false, "Not initialized");
		ALOGI("Not initialized");
		return Void();
	}

	if (mUpstream.iface != upstream) {
		hidl_cb(0, 0);
		return Void();
	}

	if (checkInterfaceStat(upstream) < 0) {
		ALOGE("Failed get interface stat %s\n", upstream.c_str());
		hidl_cb(0, 0);
		return Void();
	}

	if (strcmp(mUpstream.iface.c_str(), upstream.c_str()) == 0)
		mUpstream.uinfo.limit = limit;

	if (ioctlOffload(OFFLOAD_IOCTL_SET_DATA_LIMIT, &limit) < 0) {
		hidl_cb(false, "can't connect offload hw");
		return Void();
	}

	hidl_cb(true, NULL);
	return Void();
}

Return<void> Tetheroffload::setUpstreamParameters(const hidl_string& iface,
	const hidl_string& v4Addr, const hidl_string& v4Gw, const hidl_vec<hidl_string>& v6Gws,
	setUpstreamParameters_cb hidl_cb)
{
	string addr, netmask;
	struct iface_info param;
	stringstream ss;

	if (!isInitialized()) {
		ALOGI("Not initialized");
		hidl_cb(false, "Not initialized");
		return Void();
	}

	mUpstream.uinfo.v6Gws.clear();

    for (size_t i = 0; i < v6Gws.size(); i++) {
        string v6gw = v6Gws[i];
		string addr, netmask;

		ALOGI("%s v6Gws: %s\n", __func__, v6Gws[i].c_str());

		if (v6gw.empty() || (parseAddress(v6gw, addr, netmask) != AF_INET6)) {
			ALOGI("Failed Input Checks(v6Gws)");
			hidl_cb(false, "Failed Input Checks(v6Gws)");
			return Void();
		}

        mUpstream.uinfo.v6Gws.push_back(addr);
		ss << v6Gws[i];
		ss << " ";
    }

	ALOGI("%s iface: %s, v4Addr: %s, v4Gw: %s, [%s]", __func__, iface.c_str(), v4Addr.c_str(),
		v4Gw.c_str(), ss.str().c_str());

	if (iface.empty()) {// || (v4Gw.empty() && (v6Gws.size() == 0))) {
		ALOGI("Failed Input Checks(empty)");
		hidl_cb(false, "Failed Input Checks");
		return Void();
	}

	mUpstream.iface = iface;

	memcpy(param.iface, mUpstream.iface.c_str(), IFNAMSIZ);
	param.iface[IFNAMSIZ-1] = '\0';
	ALOGI("param.iface=%s", param.iface);

	if (checkInterfaceStat(iface) < 0) {
		ALOGE("Failed get interface stat %s\n", iface.c_str());
		hidl_cb(false, "Failed get interface stat");
		return Void();
	}

	if (ioctlOffload(OFFLOAD_IOCTL_SET_UPSTRM_PARAM, &param) < 0) {
		ALOGI("can't connect offload hw");
		hidl_cb(false, "can't connect offload hw");
		return Void();
	}

	if (!v4Addr.empty()) {
		if (parseAddress(v4Addr, addr, netmask) != AF_INET) {
			ALOGI("Failed Input Checks(v4Addr)");
			hidl_cb(false, "Failed Input Checks(v4Addr)");
			return Void();
		}
		mUpstream.uinfo.v4Addr = addr;
	}

	if (!v4Gw.empty()) {
		if (parseAddress(v4Gw, addr, netmask) != AF_INET) {
			ALOGI("Failed Input Checks(v4Gw)");
			hidl_cb(false, "Failed Input Checks(v4Gw)");
			return Void();
		}
		mUpstream.uinfo.v4Gw = addr;
	}

	hidl_cb(true, NULL);
	return Void();
}

Return<void> Tetheroffload::addDownstream(const hidl_string& iface,
	const hidl_string& prefix, addDownstream_cb hidl_cb)
{
	string addr, netmask;
	struct iface_info param;

	/* rndis0, 192.168.42.0/24 */
	ALOGI("%s iface: %s, prefix: %s", __func__, iface.c_str(), prefix.c_str());

	if (!isInitialized()) {
		hidl_cb(false, "Not initialized");
		ALOGI("Not initialized");
		return Void();
	}

	if (iface.empty()) {
		ALOGI("Failed Input Checks");
		hidl_cb(false, "Failed Input Checks");
		return Void();
	}

	if (checkInterfaceStat(iface) < 0) {
		ALOGE("Failed get interface stat %s\n", iface.c_str());
		hidl_cb(0, 0);
		return Void();
	}

	if (parseAddress(prefix, addr, netmask) < 0) {
		ALOGI("Failed Input Checks(prefix)");
		hidl_cb(false, "Failed Input Checks(prefix)");
		return Void();
	}

	mDownstreams.push_back(dIfaceinfo());
	dIfaceinfo& ds = mDownstreams.back();
	ds.iface = iface;
	ds.dinfo.prefix = prefix;

	memcpy(param.iface, ds.iface.c_str(), IFNAMSIZ);
	param.iface[IFNAMSIZ-1] = '\0';
	memcpy(param.prefix, ds.dinfo.prefix.c_str(), INET6_ADDRSTRLEN);
	param.prefix[INET6_ADDRSTRLEN-1] = '\0';

	if (ioctlOffload(OFFLOAD_IOCTL_ADD_DOWNSTREAM, &param) < 0) {
		hidl_cb(false, "can't connect offload hw");
		return Void();
	}

	hidl_cb(true, NULL);

	return Void();
}

Return<void> Tetheroffload::removeDownstream(const hidl_string& iface,
	const hidl_string& prefix, removeDownstream_cb hidl_cb)
{
	size_t i;
	string addr, netmask;
	struct iface_info param;

	ALOGI("%s %s %s", __func__, iface.c_str(), prefix.c_str());

	if (!isInitialized()) {
		hidl_cb(false, "Not initialized");
		ALOGI("Not initialized");
		return Void();
	}

	if (iface.empty()) {
		ALOGI("Failed Input Checks");
		hidl_cb(false, "Failed Input Checks");
		return Void();
	}

	if (checkInterfaceStat(iface) < 0) {
		ALOGE("Failed get interface stat %s\n", iface.c_str());
		hidl_cb(0, 0);
		return Void();
	}

	if (parseAddress(prefix, addr, netmask) < 0) {
		ALOGI("Failed Input Checks(prefix)");
		hidl_cb(false, "Failed Input Checks(prefix)");
		return Void();
	}

    for (i = 0; i < mDownstreams.size(); i++) {
		ALOGI("mDownstreams[%lu] %s", i, mDownstreams[i].iface.c_str());
		if (strcmp(mDownstreams[i].iface.c_str(), iface.c_str()) == 0) {
			memcpy(param.iface, mDownstreams[i].iface.c_str(), IFNAMSIZ);
			param.iface[IFNAMSIZ-1] = '\0';
			if (ioctlOffload(OFFLOAD_IOCTL_REMOVE_DOWNSTRM, &param) < 0) {
				hidl_cb(false, 0);
				return Void();
			}

			mDownstreams.erase(mDownstreams.begin() + i);
			hidl_cb(true, NULL);
			return Void();
		}
    }

	hidl_cb(false, "NOT found downstream");
	return Void();
}


//ITetheroffload* HIDL_FETCH_ITetheroffload(const char* /* name */) {
    //return new Tetheroffload();
//}
//
}  // namespace implementation
}  // namespace V1_0
}  // namespace tetheroffload
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor
