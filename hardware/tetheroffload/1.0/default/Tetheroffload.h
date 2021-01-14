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

#ifndef VENDOR_SAMSUNG_SLSI_HARDWARE_TETHEROFFLOAD_V1_0_TETHEROFFLOAD_H
#define VENDOR_SAMSUNG_SLSI_HARDWARE_TETHEROFFLOAD_V1_0_TETHEROFFLOAD_H

#include <android/hardware/tetheroffload/config/1.0/IOffloadConfig.h>
#include <android/hardware/tetheroffload/control/1.0/IOffloadControl.h>
#include <hidl/HidlTransportSupport.h>

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <string.h>
#include <vector>
#include <thread>

#include <utils/Looper.h>

extern "C"
{
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>
}


static const char *DEVICE_OFFLOAD = "/dev/dit";

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace tetheroffload {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_handle;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::android::status_t;
using ::android::Looper;

using ::android::hardware::tetheroffload::config::V1_0::IOffloadConfig;
using ::android::hardware::tetheroffload::control::V1_0::IOffloadControl;

using ::android::hardware::tetheroffload::control::V1_0::ITetheringOffloadCallback;

using ::std::string;
using ::std::vector;
using ::std::stringstream;
using ::std::thread;
using ::std::move;

struct _upstreaminfo {
	uint64_t limit; /* bytes */
	string v4Addr;
	string v4Gw;
	vector<string> v6Gws;
};

struct _downstreaminfo {
	string prefix;
};

struct uIfaceinfo {
	string iface;
	struct _upstreaminfo uinfo;
};

struct dIfaceinfo {
	string iface;
	struct _downstreaminfo dinfo;
};

struct ConnTrackContext {
	int fd;
	unsigned int groups;
	pthread_t tid;
};

class Tetheroffload : public IOffloadControl, IOffloadConfig {
public:
	Tetheroffload();
	~Tetheroffload();

	status_t registerService();
	void StartThreads(void);
	status_t ThreadJoin();

	bool offloadstopped();
	void offload_callback_event(int32_t event);
	bool isInitialized();
	int parseAddress(string in, string &out, string &netmask);
	int checkInterfaceStat(string name);
	int ioctlOffload(int cmd, void *arg);

	/* IOffloadConfig */
	Return<void> setHandles(const hidl_handle& fd1, const hidl_handle& fd2,
		setHandles_cb hidl_cb);

	/* IOffloadControl */
	Return<void> initOffload(const ::android::sp<ITetheringOffloadCallback>& cb,
		initOffload_cb hidl_cb);
	Return<void> stopOffload(stopOffload_cb hidl_cb);
	Return<void> setLocalPrefixes(const hidl_vec<hidl_string>& prefixes,
		setLocalPrefixes_cb hidl_cb);
	Return<void> getForwardedStats(const hidl_string& upstream,
		getForwardedStats_cb hidl_cb);
    Return<void> setDataLimit(const hidl_string& upstream, uint64_t limit,
		setDataLimit_cb hidl_cb);
	Return<void> setUpstreamParameters(const hidl_string& iface,
		const hidl_string& v4Addr, const hidl_string& v4Gw,
		const hidl_vec<hidl_string>& v6Gws, setUpstreamParameters_cb hidl_cb);
	Return<void> addDownstream(const hidl_string& iface, const hidl_string& prefix,
		addDownstream_cb hidl_cb);
	Return<void> removeDownstream(const hidl_string& iface,
		const hidl_string& prefix, removeDownstream_cb hidl_cb);

private:
	vector<pthread_t> threads;

	bool was_interrupted;
	hidl_handle mFd1;
	hidl_handle mFd2;

	sp<ITetheringOffloadCallback> mCtrlCb;

	vector<string> mLocalPrefixes;
	uIfaceinfo mUpstream;
	vector<dIfaceinfo> mDownstreams;

	uint64_t mRxBytes;
	uint64_t mTxBytes;

//	struct ConnTrackContext ctc1;
//	struct ConnTrackContext ctc2;

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" ITetheroffload* HIDL_FETCH_ITetheroffload(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace tetheroffload
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor

#endif  // VENDOR_SAMSUNG_SLSI_HARDWARE_TETHEROFFLOAD_V1_0_TETHEROFFLOAD_H
