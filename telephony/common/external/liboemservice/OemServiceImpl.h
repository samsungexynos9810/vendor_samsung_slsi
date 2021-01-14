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
 * OemServiceImpl.h
 *
 *  Created on: 2018. 5. 11.
 */

#ifndef __OEM_SERVICE_IMPL_H__
#define __OEM_SERVICE_IMPL_H__

#include <vendor/samsung_slsi/telephony/hardware/oemservice/1.0/IOemService.h>
#include <vendor/samsung_slsi/telephony/hardware/oemservice/1.0/IOemServiceCallback.h>
#include "oemservice.h"
#include "oem_internal.h"

using namespace vendor::samsung_slsi::telephony::hardware::oemservice::V1_0;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using android::sp;
using android::wp;

struct OemServiceDeathRecipient : hidl_death_recipient {
    OemServiceDeathRecipient(const sp<IOemService> oemServiceImpl) : mOemServiceImpl(oemServiceImpl) {
    }

    virtual void serviceDied(uint64_t /*cookie*/, const wp<::android::hidl::base::V1_0::IBase>& /*who*/) {
        dlog("OemServiceDeathRecipient::serviceDied");
        mOemServiceImpl->close();
    }
    sp<IOemService> mOemServiceImpl;
};

struct OemServiceImpl : public IOemService {
    OemServiceImpl();
    sp<IOemServiceCallback> mCallback;
    OEM_ServiceFunctions *mFunc;
    sp<OemServiceDeathRecipient> mDeathRecipient;

    Return<void> sendRequestRaw(int32_t type, int32_t id, const ::android::hardware::hidl_vec<uint8_t>& data);
    Return<void> setCallback(const ::android::sp<IOemServiceCallback>& callback);
    Return<void> close();
    void checkReturnStatus(Return<void>& ret);
};

class OemService {
private:
    sp<OemServiceImpl> mOemService;
    char mServiceName[MAX_SERVICE_NAMX + 1];
    OEM_ServiceFunctions mFunc;

public:
    OemService(const char *serviceName);

public:
    const char *getServiceName() const { return mServiceName; }
    int registerService();
    void setServiceFunction(OEM_ServiceFunctions *func);
    int onCallback(int type, int id, void *data, unsigned int datalen);
public:
    static OemService *makeInstance(const char *serviceName, OEM_ServiceFunctions *func);
};


#endif // __OEM_SERVICE_IMPL_H__
