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
    RIL-Client library (implementation)
*/
#include "sitril-client.h"
#include <slsi/sitril-client_proto.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <signal.h>
#include <cutils/sockets.h>
#include <utils/Log.h>
#include <sys/types.h>

#include <vendor/samsung_slsi/telephony/hardware/radioExternal/1.0/IOemSlsiRadioExternal.h>
#include <vendor/samsung_slsi/telephony/hardware/radioExternal/1.0/IOemSlsiRadioExternalInd.h>
#include <vendor/samsung_slsi/telephony/hardware/radioExternal/1.0/IOemSlsiRadioExternalRes.h>
#include <hwbinder/IPCThreadState.h>
#include <hwbinder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/SystemClock.h>
#include <inttypes.h>

using namespace vendor::samsung_slsi::telephony::hardware::radioExternal::V1_0;
using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::hardware::Return;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using ::android::sp;
using ::android::wp;

/*
    Used defines
*/
#undef LOG_TAG
#define LOG_TAG             "RILClient"
#define LogD(...)           ((void)ALOG(LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LogE(...)           ((void)ALOG(LOG_ERROR, LOG_TAG, __VA_ARGS__))

// Enable verbose logging
#define SITRIL_CLIENT_VDBG (0)
#define ENABLE_LOG_DUMP (0)

#define RILC_TRANSACTION_MAX (255)
#define RILC_DEATH_HANDLING_RECORD_MAX (10)

#define MAX_RADIO_DATA_SIZE (1 << 11)    // set 2's multiplier
#define MAX_RADIO_DATA_MOD (MAX_RADIO_DATA_SIZE - 1)

#define MAX_NUM_IND_MULTIFRAME_RAW_DATA (5)
#define MAX_NUM_RSP_MULTIFRAME_RAW_DATA (5)
/*
    Used types
*/
/* history record */
struct rilc_record {
    unsigned int         msgId;
    unsigned int         slotId;
    Rilc_OnResponse      handler;
};

/* client */
struct rilc_client {
    int clientId;
    int closing;
    pthread_mutex_t               lock;
    Rilc_OnUnsolicitedResponse    unsolicited_handler;
    unsigned int                  last;
    struct rilc_record            records[RILC_TRANSACTION_MAX];
};


struct RecRawSegmentData{
    uint8_t *pData = NULL;
    int32_t serial;
    int32_t error;
    int32_t rilcMsgId;
    int32_t slotId;
    int32_t totalLen;
    int32_t accLen;

    void reset() {
        delete[] pData;
        pData = NULL;
        serial = -1;
        error = 0;
        rilcMsgId = -1;
        slotId = -1;
        totalLen = 0;
        accLen = 0;
    }
}indRawSegData[MAX_NUM_IND_MULTIFRAME_RAW_DATA], rspRawSegData[MAX_NUM_RSP_MULTIFRAME_RAW_DATA];

struct OemSlsiRadioExternalResImpl : public IOemSlsiRadioExternalRes {
    struct rilc_client *pClient = NULL;

    Return<void> sendRequestRawResponse(const RadioExternalResponseInfo& info, const hidl_vec<uint8_t>& data);
    Return<void> sendRequestRawResponseSeg(const RadioExternalResponseInfo& info, const hidl_vec<uint8_t>& data, int32_t segIndex, int32_t totalLen);

private:
    void sendErrorResponse(int serial, int rilcMsgId, int slotId);
};

struct OemSlsiRadioExternalIndImpl : public IOemSlsiRadioExternalInd {
    struct rilc_client *pClient = NULL;

    Return<void> rilExternalRawIndication(int32_t rilcMsgId, int32_t slotId, const hidl_vec<uint8_t>& rawBytes, int32_t dataLength);
    Return<void> rilExternalRawIndicationSeg(int32_t rilcMsgId, int32_t slotId, const hidl_vec<uint8_t>& rawBytes, int32_t dataLength, int32_t segIndex, int32_t totalLen);
};

class SitRilClient
{
private:
    struct RadioExternalProxyDeathRecipient : hidl_death_recipient {
        RadioExternalProxyDeathRecipient(SitRilClient *rilClient) {
            mSitRilClient = rilClient;
        }
        ~RadioExternalProxyDeathRecipient() = default;
        virtual void serviceDied(uint64_t cookie, const wp<OemSlsiRadioExternalIndImpl::IBase>& who) {
            (void) who;
            this->mSitRilClient->processRadioExternalProxyDeath(cookie);
        };

        SitRilClient *mSitRilClient;
    };

private:
    int mClientId;
    pthread_mutex_t mClientLock;
    sp<IOemSlsiRadioExternal> mRadioExternalProxy;
    sp<OemSlsiRadioExternalResImpl> mRadioExternalRes;
    sp<OemSlsiRadioExternalIndImpl> mRadioExternalInd;

    sp<RadioExternalProxyDeathRecipient> mRadioExternalProxyDeathRecipient;
    uint64_t mExternalProxyCookie;

public:
    SitRilClient();
    virtual ~SitRilClient();

    sp<IOemSlsiRadioExternal> getRadioExternalProxy(struct rilc_client *p);

    int getClientId(void);
    void updateClientInfo(struct rilc_client *p);
    void resetRadioExternalProxy(void);
    void processRadioExternalProxyDeath(uint64_t cookie);
    void closeConnection();
public:
    static void checkReturnStatus(Return<void>& ret) {
        if (!ret.isOk()) {
            // Remote process hosting the callbacks must be dead.
            // HAL proxy and callback objects will be reset by
            // processRadioExternalProxyDeath wihch is callback by hidl_death_recipient
            LogD("[checkReturnStatus] remote process may be died");
        }
    }
};


/* static instance */
static SitRilClient sInstance;
static SitRilClient *pInstance = &sInstance;

/* SitRilClient */
SitRilClient::SitRilClient()
{
    mClientId = (int)RadioExternalClientId::RADIO_EXTERNAL_CLIENT_NUM;
    pthread_mutex_init(&mClientLock, NULL);
    mRadioExternalProxy = NULL;
    mRadioExternalRes = new OemSlsiRadioExternalResImpl();
    mRadioExternalInd = new OemSlsiRadioExternalIndImpl();
    mRadioExternalProxyDeathRecipient = new RadioExternalProxyDeathRecipient(this);
    mExternalProxyCookie = 0;
}

SitRilClient::~SitRilClient()
{
    mRadioExternalRes = NULL;
    mRadioExternalInd = NULL;
    mRadioExternalProxyDeathRecipient = NULL;
}

sp<IOemSlsiRadioExternal> SitRilClient::getRadioExternalProxy(struct rilc_client *p)
{
    int newClientId, oldClientId;
    if (mRadioExternalProxy != NULL) return mRadioExternalProxy;

    newClientId = -1;
    oldClientId = -1;

    mRadioExternalProxy = IOemSlsiRadioExternal::getService("rilExternal");
    if (mRadioExternalProxy == NULL) {
        LogD("[OemClient] %s mRadioExternalProxy is NULL", __FUNCTION__);
        // if service is not up, try to get service again after some time.
        // need to implement

    } else {
        if (mRadioExternalRes == NULL || mRadioExternalInd == NULL) {
            LogE("[OemClient] %s Res/Ind is NULL ", __FUNCTION__);
        }

        mExternalProxyCookie++;
        mRadioExternalProxy->linkToDeath(mRadioExternalProxyDeathRecipient, mExternalProxyCookie);
        newClientId = mRadioExternalProxy->setResponseFunctions(mRadioExternalRes, mRadioExternalInd);

        pthread_mutex_lock(&mClientLock);
        mClientId = newClientId;
        pthread_mutex_unlock(&mClientLock);

        if ( p != NULL) {
            pthread_mutex_lock(&p->lock);
            oldClientId = p->clientId;
            p->clientId = newClientId;
            pthread_mutex_unlock(&p->lock);
        }

        LogD("[OemClient] %s clientId: old(%d)->new(%d) ", __FUNCTION__, oldClientId, newClientId);
    }

    return mRadioExternalProxy;
}

int SitRilClient::getClientId(void)
{
    return mClientId;
}

void SitRilClient::updateClientInfo(struct rilc_client *p)
{
    pthread_mutex_lock(&mClientLock);
    if (mRadioExternalRes != NULL) mRadioExternalRes->pClient = p;
    if (mRadioExternalInd != NULL) mRadioExternalInd->pClient = p;
    pthread_mutex_unlock(&mClientLock);
}

void SitRilClient::resetRadioExternalProxy(void)
{
    mRadioExternalProxy = NULL;
    mExternalProxyCookie++;

    pthread_mutex_lock(&mClientLock);
    mClientId = (int)RadioExternalClientId::RADIO_EXTERNAL_CLIENT_NUM;
    pthread_mutex_unlock(&mClientLock);
}

void SitRilClient::processRadioExternalProxyDeath(uint64_t cookie)
{
    rilc_record waitRecord[RILC_DEATH_HANDLING_RECORD_MAX];
    int waitRecordCnt = 0;
    Rilc_OnResponse cb = 0;
    unsigned int msgId = RILC_TRANSACTION_NONE;

    memset(waitRecord, 0, sizeof(waitRecord));
    if (SITRIL_CLIENT_VDBG) {
        LogD("[OemClient] serviceDied cookie[%lu], mExternalProxyCookie[%lu]",
                (unsigned long)cookie, (unsigned long)mExternalProxyCookie);
    }

    if (mExternalProxyCookie == cookie) {
        int clientId;
        int i, ret;
        struct rilc_client *p = NULL;

        pthread_mutex_lock(&mClientLock);
        clientId = mClientId;
        pthread_mutex_unlock(&mClientLock);

        LogD("[OemClient] processRadioExternalProxyDeath PID(%d) clientId=%d", getpid(), clientId);
        // need to reconsider whether the below code is needed or not.
        if (clientId != (int)RadioExternalClientId::RADIO_EXTERNAL_CLIENT_NUM) {
            p = mRadioExternalRes->pClient;
            if (p != NULL) {
                pthread_mutex_lock(&p->lock);
                for (i = 0; i < RILC_TRANSACTION_MAX; i++) {
                    if (p->records[i].msgId != RILC_TRANSACTION_NONE && waitRecordCnt < RILC_DEATH_HANDLING_RECORD_MAX) {
                        waitRecord[waitRecordCnt].msgId = p->records[i].msgId;
                        waitRecord[waitRecordCnt].slotId = p->records[i].slotId;
                        waitRecord[waitRecordCnt].handler = p->records[i].handler;
                        waitRecordCnt++;
                    }
                    p->records[i].msgId = RILC_TRANSACTION_NONE;
                }
                p->last = 0;
                pthread_mutex_unlock(&p->lock);
            }

            if (waitRecordCnt >= RILC_DEATH_HANDLING_RECORD_MAX) {
                LogE("[OemClient] need to increase waitRecord size");
            }

            for (i = 0; i < waitRecordCnt; i++) {
                cb = waitRecord[i].handler;
                msgId = waitRecord[i].msgId;
                if (cb != NULL && msgId != RILC_TRANSACTION_NONE) {
                    cb(msgId, RILC_STATUS_FAIL, NULL, 0, waitRecord[i].slotId);
                }
            }

            resetRadioExternalProxy();

            //need to call the below fuction with some delay.
            ret = sleep(3);
            LogD("[OemClient] serviceDied cookie[%lu] current clientId: %d", (unsigned long)cookie, clientId);
            getRadioExternalProxy(p);
        }

    }
}

void SitRilClient::closeConnection()
{
    if (mRadioExternalProxy != NULL) {
        Return<void> retStatus = mRadioExternalProxy->clearResponseFunctions(mClientId);
        checkReturnStatus(retStatus);
        mRadioExternalProxy->unlinkToDeath(mRadioExternalProxyDeathRecipient);
    }

    updateClientInfo(NULL);
    resetRadioExternalProxy();
}

static int convertToRILCErrorNo(int error) {
    int rilcErrorNo = RILC_STATUS_FAIL;

    switch (error) {
        case (int)RadioExternalError::RADIO_EXTERNAL_NONE:
            rilcErrorNo = RILC_STATUS_SUCCESS;
            break;
        case (int)RadioExternalError::RADIO_EXTERNAL_MISSING_RESOURCE:
            rilcErrorNo = RILC_STATUS_MISSING_RESOURCE;
            break;
        case (int)RadioExternalError::RADIO_EXTERNAL_NO_SUCH_ELEMENT:
            rilcErrorNo = RILC_STATUS_NO_SUCH_ELEMENT;
            break;
        case (int)RadioExternalError::RADIO_EXTERNAL_INTERNAL_ERR:
            rilcErrorNo = RILC_STATUS_INTERNAL_ERR;
            break;
        case (int)RadioExternalError::RADIO_EXTERNAL_NO_MEMORY:
            rilcErrorNo = RILC_STATUS_NO_MEMORY;
            break;
        case (int)RadioExternalError::RADIO_EXTERNAL_NO_RESOURCES:
            rilcErrorNo = RILC_STATUS_NO_RESOURCES;
            break;
        case (int)RadioExternalError::RADIO_EXTERNAL_CANCELLED:
            rilcErrorNo = RILC_STATUS_CANCELLED;
            break;
        case (int)RadioExternalError::RADIO_EXTERNAL_SIM_ERROR:
            rilcErrorNo = RILC_STATUS_SIM_ERR;
            break;
        case (int)RadioExternalError::RADIO_EXTERNAL_INVALID_SIM_STATUS:
            rilcErrorNo = RILC_STATUS_INVALID_SIM_STATE;
            break;
        case (int)RadioExternalError::RADIO_EXTERNAL_REQUEST_NOT_SUPPORTED:
            rilcErrorNo = RILC_STATUS_REQUEST_NOT_SUPPORTED;
            break;
        default : rilcErrorNo = RILC_STATUS_FAIL;
            break;
    }

    return rilcErrorNo;
}

void OemSlsiRadioExternalResImpl::sendErrorResponse(int serial, int rilcMsgId, int slotId) {
    if(pClient == NULL) return;

    Rilc_OnResponse cb = NULL;
    pthread_mutex_lock(&pClient->lock);
    if (pClient->records[serial].msgId == (unsigned int)rilcMsgId) {
        cb = pClient->records[serial].handler;
        pClient->records[serial].msgId = RILC_TRANSACTION_NONE;
    }
    pthread_mutex_unlock(&pClient->lock);
    if (cb != NULL) cb(rilcMsgId, RILC_STATUS_FAIL, NULL, 0, slotId);
}

/* OemSlsiRadioExternalResImpl */
Return<void> OemSlsiRadioExternalResImpl::sendRequestRawResponse(const RadioExternalResponseInfo& info, const hidl_vec<uint8_t>& rawBytes) {
    if (pClient == NULL) {
        LogE("[OemClient]RSP: pClient is NULL, (t = %d, msgId = %d, len = %d, channel = %d)",
                info.serial, info.rilcMsgId, info.length, info.slotId);
        return Void();
    }

    LogD("[OemClient]RSP: (clientId = %d, t = %d, msgId = %d, len = %d, channel = %d, status = %d)",
            pClient->clientId, info.serial, info.rilcMsgId, info.length, info.slotId, (int)info.error);

    pthread_mutex_lock(&pClient->lock);
    if (pClient->records[info.serial].msgId != (unsigned int)info.rilcMsgId) {
        pthread_mutex_unlock(&pClient->lock);
        LogE("[OemClient]RSP: rilcMsgId mismatch. Was = %d, received = %d", pClient->records[info.serial].msgId, info.rilcMsgId);
        return Void();
    }
    Rilc_OnResponse cb = pClient->records[info.serial].handler;
    pClient->records[info.serial].msgId = RILC_TRANSACTION_NONE;
    pthread_mutex_unlock(&pClient->lock);

    if (info.length > MAX_RADIO_DATA_SIZE) {
        LogE("[OemClient]RSP: response length(%d) is too big <= bigger than MAX(%d)", info.length, MAX_RADIO_DATA_SIZE);
        if (cb != NULL) cb(info.rilcMsgId, RILC_STATUS_FAIL, NULL, 0, info.slotId);
        return Void();
    }

    /* call */
    if (cb != NULL) {
        const uint8_t *uData = rawBytes.data();
        int rilcErrorNo = convertToRILCErrorNo((int)info.error);
        cb(info.rilcMsgId, rilcErrorNo, (rawBytes.size() > 0) ? (void *)uData : NULL, info.length, info.slotId);
    }

    return Void();
}

Return<void> OemSlsiRadioExternalResImpl::sendRequestRawResponseSeg(const RadioExternalResponseInfo& info,
                                                                        const hidl_vec<uint8_t>& rawBytes,
                                                                        int32_t segIndex, int32_t totalLen){
    if (pClient == NULL) {
        LogE("[OemClient]RSP(seg): pClient is NULL, (t = %d, msgId = %d, len = %d, channel = %d)",
                info.serial, info.rilcMsgId, info.length, info.slotId);
        return Void();
    }

    if (SITRIL_CLIENT_VDBG) { // need to confirm it alwyas comes in seq form 0 1, 2    not mixed.
        LogD("[OemClient]RSP(seg): (clientId = %d, t = %d, msgId = %d, len = %d, channel = %d, status = %d, segIndex = %d, totalLen = %d)",
                pClient->clientId, info.serial, info.rilcMsgId, info.length, info.slotId, (int)info.error, segIndex, totalLen);
    }

    if (totalLen > RILC_MAX_RESPONSE_PAYLOAD_SIZE) {
        LogE("[OemClient]RSP(seg): response(segIndex = %d), total length(%d) is too big <= bigger than MAX(%d)", segIndex, totalLen, RILC_MAX_RESPONSE_PAYLOAD_SIZE);
        sendErrorResponse(info.serial, info.rilcMsgId, info.slotId);
        return Void();
    }

    // It asumed that data always comes in sequence. There is no corruption in order.
    // If not, it should be redesinged.
    // total lenth(16KB for RCS) shall be supported.
    // If several rsp Msg shall be handled at the same time, code change is needed.
    int queIndex = 0;
    struct RecRawSegmentData *accData = NULL;

    // It asumed that data always comes in sequence. There is no corruption in order.
    // If not, it should be redesinged.
    // total lenth(16KB for RCS) shall be supported.
    if (segIndex == 0) {
        for (queIndex = 0; queIndex < MAX_NUM_RSP_MULTIFRAME_RAW_DATA; ++queIndex){
            accData = &rspRawSegData[queIndex];
            if (accData->pData == NULL) {
                accData->pData = new uint8_t[totalLen];
                accData->serial= info.serial;
                accData->error = (int)info.error;
                accData->rilcMsgId = info.rilcMsgId;
                accData->slotId = info.slotId;
                accData->totalLen = totalLen;
                accData->accLen = 0;
                break;
            }
        }

        if (queIndex >= MAX_NUM_RSP_MULTIFRAME_RAW_DATA) {
            RLOGE("[OemClient]RSP(seg): full: clientId(%d), serial(%d), msgId(%d)", pClient->clientId, info.serial, info.rilcMsgId);

            // If necessary, reset all of reqRawSegData
            for(int i = 0; i < MAX_NUM_RSP_MULTIFRAME_RAW_DATA; ++i) {
                accData = &rspRawSegData[i];
                sendErrorResponse(accData->serial, accData->rilcMsgId, accData->slotId);
                accData->reset();
            }

            sendErrorResponse(info.serial, info.rilcMsgId, info.slotId);
            return Void();
        }
    } else {
        for (queIndex = 0; queIndex < MAX_NUM_RSP_MULTIFRAME_RAW_DATA; ++queIndex){
            accData = &rspRawSegData[queIndex];
            if (accData->pData != NULL && accData->serial == info.serial && accData->rilcMsgId == info.rilcMsgId && accData->slotId == info.slotId) break ;
        }

        if (queIndex >= MAX_NUM_RSP_MULTIFRAME_RAW_DATA) {
            RLOGE("[OemClient]RSP(seg): not found for segment 0: clientId(%d), serial(%d), msgId(%d) slotId(%d) segIndex(%d)",
                pClient->clientId, info.serial, info.rilcMsgId, info.slotId, segIndex);
            return Void();
        }
    }

    if(accData != NULL && accData->pData != NULL){
        uint8_t *dst = accData->pData + segIndex * MAX_RADIO_DATA_SIZE;
        const uint8_t *src = rawBytes.data();
        const uint32_t dataLength = rawBytes.size();
        memcpy(dst, src, dataLength);
        accData->accLen += dataLength;

        if (accData->accLen >= accData->totalLen) {
            pthread_mutex_lock(&pClient->lock);
            if (pClient->records[accData->serial].msgId != (unsigned int)accData->rilcMsgId) {
                pthread_mutex_unlock(&pClient->lock);
                LogE("[OemClient]RSP(seg): rilcMsgId mismatch. serial = %d, Was = %d, received = %d", accData->serial, pClient->records[accData->serial].msgId, accData->rilcMsgId);
                accData->reset();
                return Void();
            }
            Rilc_OnResponse cb = pClient->records[accData->serial].handler;
            pClient->records[accData->serial].msgId = RILC_TRANSACTION_NONE;
            pthread_mutex_unlock(&pClient->lock);

            /* call */
            if (cb != NULL) {
                const uint8_t *uData = accData->pData;
                int rilcErrorNo = convertToRILCErrorNo(accData->error);
                cb(accData->rilcMsgId, rilcErrorNo, (accData->totalLen > 0) ? (void *)uData : NULL, accData->totalLen, accData->slotId);
            }

            LogD("[OemClient]RSP(seg): (clientId = %d, t = %d, msgId = %d, len = %d, channel = %d, status = %d, Final)",
                    pClient->clientId, accData->serial, accData->rilcMsgId, accData->totalLen, accData->slotId, accData->error);

            accData->reset();
        }
    }

    return Void();
}

/* OemSlsiRadioExternalIndImpl */
Return<void> OemSlsiRadioExternalIndImpl::rilExternalRawIndication(int32_t rilcMsgId, int32_t slotId, const hidl_vec<uint8_t>& rawBytes, int32_t dataLength) {
    if (pClient == NULL) {
        LogE("[OemClient]IND: pClient is NULL, (msgId = %d, channel = %d)", rilcMsgId, slotId);
        return Void();
    }

    LogD("[OemClient]IND: (clientId = %d, msgId = %d, dataLength = %d, channel = %d)", pClient->clientId, rilcMsgId, dataLength, slotId);

    if (dataLength > RILC_MAX_RESPONSE_PAYLOAD_SIZE) {
        LogE("[OemClient] indication length(%d) is too big <= bigger than MAX(%d)", dataLength, RILC_MAX_RESPONSE_PAYLOAD_SIZE);
        return Void();
    }

    pthread_mutex_lock(&pClient->lock);
    Rilc_OnUnsolicitedResponse usdcb = pClient->unsolicited_handler;
    pthread_mutex_unlock(&pClient->lock);

    if (pClient->closing == false && usdcb != NULL) {
        const uint8_t *uData = rawBytes.data();
        usdcb(rilcMsgId, rawBytes.size() > 0 ? (void *)uData : NULL, dataLength, slotId);
    }

    return Void();
}

Return<void> OemSlsiRadioExternalIndImpl::rilExternalRawIndicationSeg(int32_t rilcMsgId, int32_t slotId,
                                                                        const hidl_vec<uint8_t>& rawBytes, int32_t dataLength,
                                                                        int32_t segIndex, int32_t totalLen){

    if (pClient == NULL) {
        LogE("[OemClient]IND(seg): pClient is NULL, (msgId = %d, channel = %d)", rilcMsgId, slotId);
        return Void();
    }

    if (SITRIL_CLIENT_VDBG) { // need to confirm it alwyas comes in seq form 0 1, 2    not mixed.
        LogD("[OemClient]IND(seg): (clientId = %d, msgId = %d, dataLength = %d, channel = %d, segIndex = %d, totoalLen = %d)",
            pClient->clientId, rilcMsgId, dataLength, slotId, segIndex, totalLen);
    }

    int queIndex = 0;
    struct RecRawSegmentData *accData = NULL;

    // It asumed that data always comes in sequence. There is no corruption in order.
    // If not, it should be redesinged.
    // total lenth(16KB for RCS) shall be supported.
    if (segIndex == 0) {
        for (queIndex = 0; queIndex < MAX_NUM_IND_MULTIFRAME_RAW_DATA; ++queIndex){
            accData = &indRawSegData[queIndex];
            if (accData->pData == NULL) {
                accData->pData = new uint8_t[totalLen];
                accData->rilcMsgId = rilcMsgId;
                accData->slotId = slotId;
                accData->totalLen = totalLen;
                accData->accLen = 0;
                break;
            }
        }

        if (queIndex >= MAX_NUM_IND_MULTIFRAME_RAW_DATA) {
            RLOGE("[OemClient]IND(seg): full: clientId(%d), msgId(%d)", pClient->clientId, rilcMsgId);

            // If necessary, reset all of reqRawSegData
            for(int i = 0; i < MAX_NUM_IND_MULTIFRAME_RAW_DATA; ++i) {
                accData = &indRawSegData[i];
                accData->reset();
            }

            return Void();
        }
    } else {
        for (queIndex = 0; queIndex < MAX_NUM_IND_MULTIFRAME_RAW_DATA; ++queIndex){
            accData = &indRawSegData[queIndex];
            if (accData->pData != NULL && accData->rilcMsgId == rilcMsgId && accData->slotId == slotId) break ;
        }

        if (queIndex >= MAX_NUM_IND_MULTIFRAME_RAW_DATA) {
            RLOGE("[OemClient]IND(seg): not found for segment 0: clientId(%d), msgId(%d) slotId(%d) segIndex(%d)",
                pClient->clientId, rilcMsgId, slotId, segIndex);
            return Void();
        }
    }

    if(accData != NULL && accData->pData != NULL){
        uint8_t *dst = accData->pData + segIndex * MAX_RADIO_DATA_SIZE;
        const uint8_t *src = rawBytes.data();
        memcpy(dst, src, dataLength);
        accData->accLen += dataLength;

        if (accData->accLen >= accData->totalLen) {
            pthread_mutex_lock(&pClient->lock);
            Rilc_OnUnsolicitedResponse usdcb = pClient->unsolicited_handler;
            pthread_mutex_unlock(&pClient->lock);

            if (pClient->closing == false && usdcb != NULL) {
                usdcb(rilcMsgId, accData->totalLen > 0 ? (void *)(accData->pData) : NULL, accData->totalLen, accData->slotId);
            }

            LogD("[OemClient]IND(seg): (clientId = %d, msgId = %d, totalLength = %d, channel = %d, Final",
                pClient->clientId, accData->rilcMsgId, accData->totalLen, accData->slotId);

            accData->reset();
        }
    }

    return Void();
}

/*
    Open client

    input : clientId
    output: pointer to new client structure or NULL
*/
extern "C" void* RILC_Open()
{
    /* create client */
    struct rilc_client *p = (struct rilc_client*)malloc(sizeof(struct rilc_client));
    if (p == NULL) {
        LogE("[OemClient] %s Failed to allocate memory for client",  __FUNCTION__);
        return NULL;
    }
    memset(p, 0, sizeof(struct rilc_client));
    for (int i = 0; i < RILC_TRANSACTION_MAX; i++) {
        p->records[i].msgId = RILC_TRANSACTION_NONE;
    }
    pthread_mutex_init(&p->lock, NULL);
    p->closing = false;
    p->last = 0;
    p->clientId = (int)RadioExternalClientId::RADIO_EXTERNAL_CLIENT_NUM;

    struct RecRawSegmentData *accData = NULL;
    for(int i = 0; i < MAX_NUM_IND_MULTIFRAME_RAW_DATA; ++i) {
        accData = &indRawSegData[i];
        accData->reset();
    }
    for(int i = 0; i < MAX_NUM_RSP_MULTIFRAME_RAW_DATA; ++i) {
        accData = &rspRawSegData[i];
        accData->reset();
    }

    /* connect */
    sp<IOemSlsiRadioExternal> radioExternalProxy = pInstance->getRadioExternalProxy(p);
    if (radioExternalProxy == NULL) {
        free(p);
        return NULL;
    }
    pInstance->updateClientInfo(p);

    LogD("[OemClient] %s Client(%d) created ", __FUNCTION__, p->clientId);
    return p;
}

/*
    Re-connect to RIL

    input : pointer to client structure
    output: 0 if re-connect was successful, error status otherwise
*/
extern "C" int RILC_Reconnect(void* client)
{
    LogD("[OemClient] %s", __FUNCTION__);

    struct rilc_client *p = (struct rilc_client*)client;
    if (p == NULL) {
        return -RILC_STATUS_INVALID_PARAM;
    }

    sp<IOemSlsiRadioExternal> radioExternalProxy = pInstance->getRadioExternalProxy(p);
    if (radioExternalProxy == NULL) {
        free(p);
        return -RILC_STATUS_FAIL;
    }

    pthread_mutex_lock(&p->lock);
    p->closing = false;
    pthread_mutex_unlock(&p->lock);

    struct RecRawSegmentData *accData = NULL;
    for(int i = 0; i < MAX_NUM_IND_MULTIFRAME_RAW_DATA; ++i) {
        accData = &indRawSegData[i];
        accData->reset();
    }
    for(int i = 0; i < MAX_NUM_RSP_MULTIFRAME_RAW_DATA; ++i) {
        accData = &rspRawSegData[i];
        accData->reset();
    }

    LogD("[OemClient] %s Client(%d) re-connected", __FUNCTION__, p->clientId);
    return RILC_STATUS_SUCCESS;
}

/*
    Delete client

    input : pointer to client structure
    output: 0 if delete was successful, error status otherwise
*/
extern "C" int RILC_Close(void* client)
{
    int clientId;
    LogD("[OemClient] %s", __FUNCTION__);

    struct rilc_client *p = (struct rilc_client*)client;
    if (p == NULL) {
        return -RILC_STATUS_INVALID_PARAM;
    }

    pthread_mutex_lock(&p->lock);
    clientId = p->clientId;
    p->closing = true;
    pthread_mutex_unlock(&p->lock);

    struct RecRawSegmentData *accData = NULL;
    for(int i = 0; i < MAX_NUM_IND_MULTIFRAME_RAW_DATA; ++i) {
        accData = &indRawSegData[i];
        accData->reset();
    }
    for(int i = 0; i < MAX_NUM_RSP_MULTIFRAME_RAW_DATA; ++i) {
        accData = &rspRawSegData[i];
        accData->reset();
    }

    pInstance->closeConnection();

    free(p);
    LogD("[OemClient] %s, Client(%d) closed", __FUNCTION__, clientId);
    return RILC_STATUS_SUCCESS;
}

/*
    Register unsolicited handler

    input : pointer to client structure, pointer to handler or NULL
    output: 0 if register was successful, error status otherwise
*/
extern "C" int RILC_RegisterUnsolicitedHandler(void* client, Rilc_OnUnsolicitedResponse handler)
{
    LogD("[OemClient] %s", __FUNCTION__);

    struct rilc_client *p = (struct rilc_client*)client;
    if (p == NULL)
        return -RILC_STATUS_INVALID_PARAM;

    pthread_mutex_lock(&p->lock);
    int clientId = p->clientId;
    p->unsolicited_handler = handler;
    pthread_mutex_unlock(&p->lock);

    if (SITRIL_CLIENT_VDBG) LogD("[OemClient] %s, Client(%d) Unsol registered", __FUNCTION__, clientId);
    return RILC_STATUS_SUCCESS;
}

/*
    Send request to RIL

    input : pointer to client structure, message ID, pointer to message data or NULL,
        length of data, pointer to response handler or NULL
    output: 0 if sending was successful, error status otherwise
*/
extern "C" int RILC_Send(void* client, unsigned msgId, void* data, size_t length, Rilc_OnResponse handler, unsigned int channel)
{
    if (SITRIL_CLIENT_VDBG) LogD("[OemClient] %s msgId:%d lengh:%zu channel:%d", __FUNCTION__,msgId, length, channel);

    struct rilc_client *p = (struct rilc_client*)client;
    if (p == NULL) {
        return -RILC_STATUS_INVALID_PARAM;
    }

    if (length && (data == NULL)) {
        return -RILC_STATUS_INVALID_PARAM;
    }

    sp<IOemSlsiRadioExternal> radioExternalProxy = pInstance->getRadioExternalProxy(p);
    if (radioExternalProxy == NULL) {
         return -RILC_STATUS_NOT_CONNECTED;
    }

//    rilc_request_t *req = NULL;

    /* check length based on RILC req */
    switch (msgId) {
        case RILC_REQ_SYSTEM_MODEM_DUMP:
        case RILC_REQ_MISC_SET_ENG_MODE:
            break;
        case RILC_REQ_MISC_SCREEN_LINE:
        {
            if (length != RILC_REQ_SET_SCR_LINE_SIZE)
                return -RILC_STATUS_INVALID_PARAM;
            break;
        }
        case RILC_REQ_MISC_DEBUG_TRACE:
        {
            if (length != RILC_REQ_SET_DEBUG_TRACE_SIZE)
                return -RILC_STATUS_INVALID_PARAM;
            break;
        }
        case RILC_REQ_MISC_SET_CARRIER_CONFIG:
        case RILC_REQ_MISC_SET_ENG_STRING_INPUT:
        case RILC_REQ_MISC_APN_SETTINGS:
        case RILC_REQ_MISC_GET_MSL_CODE:
        case RILC_REQ_MISC_SET_PIN_CONTROL:
        case RILC_REQ_GET_AVAILABLE_NETWORKS:
            break;
        case RILC_REQ_AUDIO_SET_MUTE:
        {
            if (length != RILC_AUDIO_MUTE_SIZE)
                return -RILC_STATUS_INVALID_PARAM;
            break;
        }
        case RILC_REQ_AUDIO_SET_VOLUME:
        {
            if (length != RILC_AUDIO_VOLUME_SIZE)
                return -RILC_STATUS_INVALID_PARAM;
            break;
        }
        case RILC_REQ_AUDIO_SET_PATH:
        {
            if (length != RILC_AUDIO_PATH_SIZE)
                return -RILC_STATUS_INVALID_PARAM;
            break;
        }
        case RILC_REQ_AUDIO_SET_MIC:
        {
            if (length != RILC_AUDIO_MICCTL_SIZE)
                return -RILC_STATUS_INVALID_PARAM;
            break;
        }
        case RILC_REQ_AUDIO_GET_MUTE:
        case RILC_REQ_AUDIO_GET_VOLUME:
        case RILC_REQ_AUDIO_GET_PATH:
        case RILC_REQ_AUDIO_GET_MIC:
            break;
        case RILC_REQ_AUDIO_SET_AUDIO_CLOCK:
        {
            if (length != RILC_AUDIO_AUDIO_CLOCK_SIZE)
                return -RILC_STATUS_INVALID_PARAM;
            break;
        }
        case RILC_REQ_AUDIO_SET_AUDIO_LOOPBACK:
        {
            if (length != RILC_AUDIO_AUDIO_LOOPBACK_SIZE)
                return -RILC_STATUS_INVALID_PARAM;
            break;
        }
        case RILC_REQ_AUDIO_SET_TTY_MODE:
            break;
        //for IMS
        case RILC_REQ_IMS_SET_CONFIGURATION:
        case RILC_REQ_IMS_GET_CONFIGURATION:
        case RILC_REQ_IMS_SIM_AUTH:
        case RILC_REQ_IMS_SET_EMERGENCY_CALL_STATUS:
        case RILC_REQ_IMS_SET_SRVCC_CALL_LIST:
        case RILC_REQ_IMS_GET_GBA_AUTH:
        case RILC_REQ_IMS_SIM_IO:
        case RILC_REQ_NET_GET_IMS_SUPPORT_SERVICE:
        //CALL_CAPABILITY
        case RILC_REQ_MISC_SET_PREFERRED_CALL_CAPA:
        case RILC_REQ_MISC_GET_PREFERRED_CALL_CAPA:
        /* for GPS */
        case RILC_REQ_GPS_SET_FREQUENCY_AIDING:
        case RILC_REQ_GPS_GET_LPP_SUPL_REQ_ECID_INFO:
        case RILC_REQ_GPS_SET_RRLP_SUPL_REQ_ECID_INFO:
        case RILC_REQ_GPS_MO_LOCATION_REQUEST:
        case RILC_REQ_GPS_GET_LPP_REQ_SERVING_CELL_INFO:
        case RILC_REQ_GPS_SET_SUPL_NI_READY:
        case RILC_REQ_GPS_GET_GSM_EXT_INFO_MSG:
        case RILC_REQ_GPS_CONTROL_PLANE_ENABLE:
        case RILC_REQ_GPS_GNSS_LPP_PROFILE_SET:
        // Indication from AP, No resp
        case RILC_REQ_GPS_MEASURE_POS_RSP:
        case RILC_REQ_GPS_RELEASE_GPS:
        case RILC_REQ_GPS_MT_LOCATION_REQUEST:
        case RILC_REQ_GPS_LPP_PROVIDE_CAPABILITIES:
        case RILC_REQ_GPS_LPP_REQUEST_ASSIST_DATA:
        case RILC_REQ_GPS_LPP_PROVIDE_LOCATION_INFO:
        case RILC_REQ_GPS_LPP_GPS_ERROR_IND:
        case RILC_REQ_GPS_SUPL_LPP_DATA_INFO:
        case RILC_REQ_GPS_SUPL_NI_MESSAGE:
        case RILC_REQ_GPS_RETRIEVE_LOC_INFO:
        /* CDMA & HEDGE GANSS */
        case RILC_REQ_GPS_SET_GANSS_MEAS_POS_RSP:
        case RILC_REQ_GPS_SET_GPS_LOCK_MODE:
        case RILC_REQ_GPS_GET_REFERENCE_LOCATION:
        case RILC_REQ_GPS_SET_PSEUDO_RANGE_MEASUREMENTS:
        case RILC_REQ_GPS_GET_CDMA_PRECISE_TIME_AIDING_INFO:
        case RILC_REQ_GPS_CDMA_FREQ_AIDING:
        // Indication from AP, No resp
        case RILC_REQ_GPS_GANSS_AP_POS_CAP_RSP:
        // for WLan
        case RILC_REQ_WLAN_GET_IMSI:
        case RILC_REQ_WLAN_SIM_AUTHENTICATE:
        // for IF
        case RILC_REQ_IF_EXECUTE_AM:

        // Bandmode & RF desense
        case RILC_REQ_MISC_GET_MANUAL_BAND_MODE:
        case RILC_REQ_MISC_SET_MANUAL_BAND_MODE:
        case RILC_REQ_MISC_GET_RF_DESENSE_MODE:
        case RILC_REQ_MISC_SET_RF_DESENSE_MODE:
        case RILC_REQ_SCAN_RSSI:

        // Oem common
        case RILC_REQ_MISC_STORE_ADB_SERIAL_NUMBER:
        case RILC_REQ_MISC_READ_ADB_SERIAL_NUMBER:

        //AIMS support start ---------------------
        case RILC_REQ_AIMS_DIAL:
        case RILC_REQ_AIMS_ANSWER:
        case RILC_REQ_AIMS_HANGUP:
        case RILC_REQ_AIMS_DEREGISTRATION:
        case RILC_REQ_AIMS_HIDDEN_MENU:
        case RILC_REQ_AIMS_ADD_PDN_INFO:
        case RILC_REQ_AIMS_CALL_MANAGE:
        case RILC_REQ_AIMS_SEND_DTMF:
        case RILC_REQ_AIMS_SET_FRAME_TIME:
        case RILC_REQ_AIMS_GET_FRAME_TIME:
        case RILC_REQ_AIMS_CALL_MODIFY:
        case RILC_REQ_AIMS_RESPONSE_CALL_MODIFY:
        case RILC_REQ_AIMS_TIME_INFO:
        case RILC_REQ_AIMS_CONF_CALL_ADD_REMOVE_USER:
        case RILC_REQ_AIMS_ENHANCED_CONF_CALL:
        case RILC_REQ_AIMS_GET_CALL_FORWARD_STATUS:
        case RILC_REQ_AIMS_SET_CALL_FORWARD_STATUS:
        case RILC_REQ_AIMS_GET_CALL_WAITING:
        case RILC_REQ_AIMS_SET_CALL_WAITING:
        case RILC_REQ_AIMS_GET_CALL_BARRING:
        case RILC_REQ_AIMS_SET_CALL_BARRING:
        case RILC_REQ_AIMS_SEND_SMS:
        case RILC_REQ_AIMS_SEND_EXPECT_MORE:
        case RILC_REQ_AIMS_SEND_SMS_ACK:
        case RILC_REQ_AIMS_SEND_ACK_INCOMING_SMS:
        case RILC_REQ_AIMS_CHG_BARRING_PWD:
        case RILC_REQ_AIMS_SEND_USSD_INFO:
        case RILC_REQ_AIMS_GET_PRESENTATION_SETTINGS:
        case RILC_REQ_AIMS_SET_PRESENTATION_SETTINGS:
        case RILC_REQ_AIMS_SET_SELF_CAPABILITY:
        case RILC_REQ_AIMS_HO_TO_WIFI_READY:
        case RILC_REQ_AIMS_HO_TO_WIFI_CANCEL_IND:
        case RILC_REQ_AIMS_HO_PAYLOAD_IND:
        case RILC_REQ_AIMS_HO_TO_3GPP:
        case RILC_REQ_AIMS_SEND_ACK_INCOMING_CDMA_SMS:
        case RILC_REQ_AIMS_MEDIA_STATE_IND:
        case RILC_REQ_AIMS_DEL_PDN_INFO:
        case RILC_REQ_AIMS_STACK_START_REQ:
        case RILC_REQ_AIMS_STACK_STOP_REQ:
        case RILC_REQ_AIMS_XCAPM_START_REQ:
        case RILC_REQ_AIMS_XCAPM_STOP_REQ:
        case RILC_REQ_AIMS_RTT_SEND_TEXT:
        case RILC_REQ_AIMS_EXIT_EMERGENCY_CB_MODE:
        case RILC_REQ_AIMS_SET_GEO_LOCATION_INFO:
        case RILC_REQ_AIMS_CDMA_SEND_SMS:
        case RILC_REQ_AIMS_RCS_MULTI_FRAME:
        case RILC_REQ_AIMS_RCS_CHAT:
        case RILC_REQ_AIMS_RCS_GROUP_CHAT:
        case RILC_REQ_AIMS_RCS_OFFLINE_MODE:
        case RILC_REQ_AIMS_RCS_FILE_TRANSFER:
        case RILC_REQ_AIMS_RCS_COMMON_MESSAGE:
        case RILC_REQ_AIMS_RCS_CONTENT_SHARE:
        case RILC_REQ_AIMS_RCS_PRESENCE:
        case RILC_REQ_AIMS_XCAP_MANAGE:
        case RILC_REQ_AIMS_RCS_CONFIG_MANAGE:
        case RILC_REQ_AIMS_RCS_TLS_MANAGE:
        //WFC
        case RILC_REQ_WFC_MEDIA_CHANNEL_CONFIG:
        case RILC_REQ_WFC_DTMF_START:
        case RILC_REQ_WFC_SET_VOWIFI_HO_THRESHOLD:
        //SENSOR
        case RILC_REQ_PSENSOR_SET_STATUS:
        // VSIM
        case RILC_REQ_VSIM_NOTIFICATION:
        case RILC_REQ_VSIM_OPERATION:
        //SAR
        case RILC_REQ_SAR_SET_SAR_STATE:
        case RILC_REQ_SAR_GET_SAR_STATE:
            break;

        // Seceure Element
        case RILC_REQ_SE_OPEN_CHANNEL:
        case RILC_REQ_SE_TRANSMIT_APDU_LOGICAL:
        case RILC_REQ_SE_TRANSMIT_APDU_BASIC:
        case RILC_REQ_SE_CLOSE_CHANNEL:
        case RILC_REQ_SE_GET_ICC_ATR:
        case RILC_REQ_SE_GET_CARD_PRESENT:
            break;

        case RILC_REQ_EMBMS_ENABLE_SERVICE:
        case RILC_REQ_EMBMS_DISABLE_SERVICE:
        case RILC_REQ_EMBMS_SET_SESSION:
        case RILC_REQ_EMBMS_GET_SESSION_LIST:
        case RILC_REQ_EMBMS_GET_SIGNAL_STRENGTH:
        case RILC_REQ_EMBMS_GET_NETWORK_TIME:
        case RILC_REQ_EMBMS_CHECK_AVAIABLE_EMBMS:

        // EN-DC
        case RILC_REQ_SET_ENDC_MODE:
        case RILC_REQ_GET_ENDC_MODE:
            break;

        default:
            LogE("[OemClient] Unsupported msgId = %d", msgId);
            return -RILC_STATUS_INVALID_PARAM;
    }


    /* allocate transaction */
    int have_free = 0;
    int transaction = 0;

    pthread_mutex_lock(&p->lock);
    int clientId = p->clientId;
    for (int i = 0; i < RILC_TRANSACTION_MAX; i++) {
        if (p->records[p->last].msgId == RILC_TRANSACTION_NONE) {
            have_free = 1;
            break;
        }
        p->last = (p->last + 1) % RILC_TRANSACTION_MAX;
    }
    if (have_free) {
        /* transaction */
        transaction = p->last;

        /* save record */
        p->records[p->last].msgId = msgId;
        p->records[p->last].handler = handler;
        p->records[p->last].slotId = channel;
        p->last = (p->last + 1) % RILC_TRANSACTION_MAX;
    }
    pthread_mutex_unlock(&p->lock);

    if (!have_free) {
        LogE("[OemClient] No available transactions");
        return -RILC_STATUS_NO_RESOURCES;
    }

#if ENABLE_LOG_DUMP
    /* dump message */
    {
        char print_buffer[512];
        memset(print_buffer, 0, 512);
        unsigned char dump_data = (unsigned char)data;

        unsigned int len = (length > 16) ? 16 : length;
        unsigned int ofs = sprintf(print_buffer, "REQ (t = %d, id = %d, len = %zu)", transaction, msgId, length);
        if (length) {
            ofs += sprintf(print_buffer + ofs, " %s: ", "DATA");
            for (unsigned int i = 0; i < len; i++) {
                ofs += sprintf(print_buffer + ofs, "%02x ", dump_data[i]);
            }
        }
        LogD("[OemClient] %s", print_buffer);
    }
#endif

    LogD("[OemClient]REQ (clientId = %d, t = %d, msgId = %d, len = %zu channel = %d)",
            clientId, transaction, msgId, length, channel);

    /* send */
    if (length > MAX_RADIO_DATA_SIZE) {
        int index;
        int num = (length + MAX_RADIO_DATA_SIZE - 1) / MAX_RADIO_DATA_SIZE;
        uint8_t *proceesingData = (uint8_t *)data;

        for (index = 0; index < num; ++index) {
            hidl_vec<uint8_t> sendData;
            int segLength = (index == num-1) ? length&MAX_RADIO_DATA_MOD : MAX_RADIO_DATA_SIZE;
            sendData.setToExternal(proceesingData, segLength);
            Return<void> retStatus =
                    radioExternalProxy->sendRequestRawSegment(transaction, clientId, msgId, channel, segLength,
                            sendData, index, length);
            SitRilClient::checkReturnStatus(retStatus);
            proceesingData += segLength;
        }
    } else {
        hidl_vec<uint8_t> sendData;
        if (data != NULL) {
            sendData.setToExternal((uint8_t *)data, length);
        }

        Return<void> retStatus =
                radioExternalProxy->sendRequestRaw(transaction, clientId, msgId, channel, length, sendData);
        SitRilClient::checkReturnStatus(retStatus);
    }

    return RILC_STATUS_SUCCESS;
}
