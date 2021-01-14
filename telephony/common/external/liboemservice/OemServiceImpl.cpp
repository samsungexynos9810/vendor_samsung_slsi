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
 * OemServiceImpl.cpp
 *
 *  Created on: 2018. 5. 11.
 */

#include "OemServiceImpl.h"

OemServiceImpl::OemServiceImpl() {
    mFunc = NULL;
    mCallback = NULL;
    mDeathRecipient = new OemServiceDeathRecipient(this);
}

Return<void> OemServiceImpl::sendRequestRaw(int32_t type, int32_t id, const ::android::hardware::hidl_vec<uint8_t>& rawBytes) {
    dlog("sendRequestRaw");
    if (mFunc != NULL) {
        const uint8_t *uData = rawBytes.data();
        mFunc->onRequest(type, id, (void *)uData, rawBytes.size());
    }
    return Void();
}

Return<void> OemServiceImpl::setCallback(const ::android::sp<IOemServiceCallback>& callback) {
    dlog("setCallback");

    mCallback = callback;
    if (mCallback != NULL) {
        dlog("setCallback: linkToDeath");
        mCallback->linkToDeath(mDeathRecipient, 0 /*cookie*/);
    }
    return Void();
}
Return<void> OemServiceImpl::close() {
    dlog("close");

    if (mCallback != NULL) {
        dlog("close: unlinkToDeath");
        mCallback->unlinkToDeath(mDeathRecipient);
    }
    mCallback = NULL;
    return Void();
}

void OemServiceImpl::checkReturnStatus(Return<void>& ret) {
    if (ret.isOk() == false) {
        dlog("checkReturnStatus: unable to call callback. Client may be died.");
        this->close();
    }
}

OemService *OemService::makeInstance(const char *serviceName, OEM_ServiceFunctions *func) {
    if (serviceName == NULL || *serviceName == 0) {
        return NULL;
    }

    OemService *oemService = new OemService(serviceName);
    if (oemService != NULL) {
        oemService->setServiceFunction(func);
    }
    return oemService;
}

OemService::OemService(const char *serviceName) {
    memset(mServiceName, 0, sizeof(mServiceName));
    memset(&mFunc, 0, sizeof(mFunc));
    mOemService = NULL;
    if (serviceName != NULL) {
        strncpy(mServiceName, serviceName, MAX_SERVICE_NAMX);
        mOemService = new OemServiceImpl;
        mOemService->mFunc = NULL;
        mOemService->mCallback = NULL;
    }
}

int OemService::registerService() {
    dlog("[%s]registerService", mServiceName);
    android::status_t status = mOemService->registerAsService(mServiceName);
    return status;
}

void OemService::setServiceFunction(OEM_ServiceFunctions *func) {
    dlog("[%s]setServiceFunction", mServiceName);
    if (func != NULL) {
        memcpy(&mFunc, func, sizeof(OEM_ServiceFunctions));
        mOemService->mFunc = &mFunc;
    }
}

int OemService::onCallback(int type, int id, void *data, unsigned int datalen) {
    dlog("[%s]onCallback", mServiceName);
    if (mOemService != NULL && mOemService->mCallback != NULL) {
        if (data == NULL || datalen == 0) {
            dlog("invalid data");
            return -1;
        }

        hidl_vec<uint8_t> rawBytes;
        rawBytes.setToExternal((uint8_t *) data, datalen);
        Return<void> retStatus = mOemService->mCallback->onCallback(type, id, rawBytes);
        mOemService->checkReturnStatus(retStatus);
    }
    return 0;
}
