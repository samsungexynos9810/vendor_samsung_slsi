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
 * SITRilAudioEx.cpp
 *
 *  Created on: 2015. 4. 30.
 *      Author: sungwoo48.choi
 */
#include "SITRilAudio.h"
#include "rilclienthelper.h"
#include "customproductfeature.h"
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

static RilAudioApi g_RilAudioApi;
IMPLEMENT_MODULE_TAG(RilAudioApi, RilAudioApi)

RilAudioApi *RilAudioApi::GetInstance()
{
    return &g_RilAudioApi;
}

void RilAudioApi::OnSolicitedResponse(unsigned msgId, int status, void* data, size_t length, unsigned int channel)
{
    RilAudioApi *instance = RilAudioApi::GetInstance();
    if (instance != NULL) {
        instance->HandleSolicitedResponse(msgId, status, data, (unsigned int)length, channel);
    }
}

void RilAudioApi::OnUnsolicitedResponse(unsigned msgId, void* data, size_t length, unsigned int channel)
{
    RilAudioApi *instance = RilAudioApi::GetInstance();
    if (instance != NULL) {
        instance->HandleUnsolicitedResponse(msgId, data, (unsigned int)length, channel);
    }
}

RilAudioApi::RilAudioApi()
{
    RLOGI("%s::%s create RilAudioApi instance", TAG, __FUNCTION__);
    m_pRilClientHelper = NULL;
    m_client = NULL;
    m_adev = NULL;
    m_fpEventHandler = NULL;
    memset(m_transaction, 0, sizeof(m_transaction));
    m_OnClosing = false;
}

RilAudioApi::~RilAudioApi()
{
    RLOGI("%s::%s release RilAudioApi instance", TAG, __FUNCTION__);
    if (m_client != NULL) {
        Close();
    }

    if (m_pRilClientHelper != NULL) {
        delete m_pRilClientHelper;
        m_pRilClientHelper = NULL;
    }
}

void RilAudioApi::HandleSolicitedResponse(unsigned int msgId, int status, void* data, unsigned int length, unsigned int channel)
{
    unsigned int size = sizeof(m_transaction) / sizeof(m_transaction[0]);
    if (msgId < size) {
        RLOGV("%s::%s Received:msgId=%d", TAG, __FUNCTION__, msgId);
        m_transaction[msgId] = false;
    }
    else {
        RLOGW("%s::%s Unexpected msgId=%d", TAG, __FUNCTION__, msgId);
    }
}

void RilAudioApi::HandleUnsolicitedResponse(unsigned msgId, void *data, unsigned int length, unsigned int channel)
{
    RLOGI("%s::%s", TAG, __FUNCTION__);
    int event = RILAUDIO_EVENT_BASE;
    switch (msgId) {
    case RILC_UNSOL_IMS_SRVCC_HO:
        RLOGI("%s::%s RILC_UNSOL_IMS_SRVCC_HO(%d)", TAG, __FUNCTION__, msgId);
        event = RILAUDIO_EVENT_IMS_SRVCC_HANDOVER;
        break;
    case RILC_UNSOL_AUDIO_RINGBACK:
        RLOGI("%s::%s ignore RILC_UNSOL_AUDIO_RINGBACK(%d)", TAG, __FUNCTION__, msgId);
        break;
    case RILC_UNSOL_AUDIO_RINGBACK_BY_NETWORK:
        RLOGI("%s::%s RILC_UNSOL_AUDIO_RINGBACK_BY_NETWORK(%d)", TAG, __FUNCTION__, msgId);
        if (data != NULL && length >= sizeof(int)) {
            RLOGI("  - status=%d", ((int *)data)[0]);
            if (length >= sizeof(int) * 2) {
                RLOGI("  - flag=%d", ((int *)data)[1]);
            }
        }
        event = RILAUDIO_EVENT_RINGBACK_STATE_CHANGED;
        break;
    case RILC_UNSOL_WB_AMR_REPORT:
         RLOGI("%s::%s RILC_UNSOL_WB_AMR_REPORT(%d)", TAG, __FUNCTION__, msgId);
        if (length >= sizeof(int)) {
            RLOGI(" - status=%d", ((int *)data)[0]);
            if (length >= sizeof(int) * 2) {
                RLOGI(" - calltype=%d", ((int *)data)[1]);
            }
        }
        else {
            RLOGW("%s::%s RILC_UNSOL_WB_AMR_REPORT(%d) Invalid data length(%u)", TAG, __FUNCTION__, msgId, length);
            return ;
        }
        event = RILAUDIO_EVENT_WB_AMR_REPORT;
        break;
    default:
        RLOGI("%s::%s filtered-out: msgId=%d", TAG, __FUNCTION__, msgId);
        return;
    }

    if (m_fpEventHandler != NULL && event != RILAUDIO_EVENT_BASE) {
        RLOGV("%s::%s Invoke callback: m_adev=%p event=%d data=%p length=%d", TAG, __FUNCTION__,
                m_adev, event, data, length);
        m_fpEventHandler(m_adev, event, data, length);
    } else {
        RLOGE("%s::%s did not send (%d) m_fpEventHandler=0x%p", TAG, __FUNCTION__, event, m_fpEventHandler);
    }
}

bool RilAudioApi::IsTransactionExisted() const
{
    int size = sizeof(m_transaction) / sizeof(m_transaction[0]);
    for (int i = 0; i < size; i++) {
        if (m_transaction[i]) {
            return true;
        }
    } // end for i ~
    return false;
}

void RilAudioApi::WaitAllTransactionCompleted()
{
    const int TOTAL_TIMEOUT = 3000000;  // 3sec, micro second
    const int INTERVAL = 100000; // 100ms, micro second
    int count = TOTAL_TIMEOUT / INTERVAL;

    bool onTranscation = false;
    while ((onTranscation = IsTransactionExisted()) && count > 0) {
        usleep(INTERVAL);
        count--;
    } // end while ~
    RLOGI("%s::%s onTranscation=%d count=%d", TAG, __FUNCTION__, onTranscation, count);
}

int RilAudioApi::Open()
{
    ProductFeature::Init("sitril-audio");

    RLOGI("%s::%s", TAG, __FUNCTION__);
    if (m_client != NULL) {
        RLOGV("%s::%s RilAudioApi was already opened. m_client=%p", TAG, __FUNCTION__, m_client);
        return RILAUDIO_ERROR_NONE;
    }

    if (m_pRilClientHelper == NULL) {
        //RLOGV("%s::%s create new instance of RilClientHelper", TAG, __FUNCTION__);
        //m_pRilClientHelper = RilClientHelper::GetInstance();
#ifndef _USE_RILCLIENT_DL_
        m_pRilClientHelper = RilClientHelperFactory::CreateHelperInstance(RilClientHelperFactory::RILCLIENT_LINK_STATIC);
#else
        m_pRilClientHelper = RilClientHelperFactory::CreateHelperInstance(RilClientHelperFactory::RILCILENT_DYNAMIC_LOAD);
#endif
        if (m_pRilClientHelper == NULL) {
            RLOGE("%s::%s fail to initialize RilClientHelper", TAG, __FUNCTION__);
            return RILAUDIO_ERROR_NO_FILE;
        }
    }

    m_client = m_pRilClientHelper->Open();
    if (m_client == NULL) {
        RLOGE("%s::%s fail to open RIL client", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_NO_LIB_OPEN_FAIL;
    }

    if (m_pRilClientHelper != NULL) {
        if (m_pRilClientHelper->RegisterUnsolicitedResponseHandler(m_client, &RilAudioApi::OnUnsolicitedResponse) != RILC_STATUS_SUCCESS) {
            RLOGE("%s::%s fail to register UnsolicitedResponse handler", TAG, __FUNCTION__);
            RLOGW("%s::%s close RIL client connection", TAG, __FUNCTION__);
            Close();
            return RILAUDIO_ERROR_NO_LIB_OPEN_FAIL;
        }
    }

    m_OnClosing = false;
    memset(m_transaction, 0, sizeof(m_transaction));

    return RILAUDIO_ERROR_NONE;
}

int RilAudioApi::Close()
{
    RLOGI("%s::%s", TAG, __FUNCTION__);
    if (m_client == NULL) {
        RLOGW("%s::%s no RIL client connection to be closed", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_NO_DEVICE;
    }

    m_OnClosing = true;
    WaitAllTransactionCompleted();

    m_pRilClientHelper->RegisterUnsolicitedResponseHandler(m_client, NULL);
    m_pRilClientHelper->Close(m_client);
    m_client = NULL;
    m_adev = NULL;
    m_fpEventHandler = NULL;
    m_OnClosing = false;
    memset(m_transaction, 0, sizeof(m_transaction));

    return RILAUDIO_ERROR_NONE;
}

int RilAudioApi::RegisterEventCallback(HANDLE handle, RILAUDIO_EventCallback callback)
{
    RLOGI("%s::%s handle=%p callback=%p", TAG, __FUNCTION__, handle, callback);
    m_adev = handle;
    m_fpEventHandler = callback;
    return RILAUDIO_ERROR_NONE;
}

int RilAudioApi::SetAudioVolume(int volume)
{
    RLOGI("%s::%s volume=%d", TAG, __FUNCTION__, volume);
    if (m_pRilClientHelper == NULL || m_client == NULL) {
        RLOGW("%s::%s error: not initialized", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_NO_DEVICE;
    }

    if (m_OnClosing) {
        RLOGE("%s::%s On Closing", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }

    int Ret = RILC_STATUS_SUCCESS;
//#ifdef AUDIO_FEATURE_CUSTOM_VOLUME_START_FROM1
    if ( CustomProductFeature::SupportVolumeStartFrom1() == true )
    {
        int volume_CP = volume - 1; // AudioHAL sends the value from 1 and CP from 0.
        RLOGV("%s::%s volume:%d is transferred to CP as volume_CP:%d", TAG, __FUNCTION__,volume,volume_CP);

        if ( volume_CP < 0 )
        {
            volume_CP = 0;
        }

        Ret = m_pRilClientHelper->Send(m_client, RILC_REQ_AUDIO_SET_VOLUME, (void*)(&volume_CP), sizeof(volume_CP), OnSolicitedResponse);
    }
//#else
    else
    {
        if ( volume < 0 )
        {
            volume = 0;
        }

        Ret = m_pRilClientHelper->Send(m_client, RILC_REQ_AUDIO_SET_VOLUME, (void*)(&volume), sizeof(volume), OnSolicitedResponse);
    }
//#endif    //AUDIO_FEATURE_CUSTOM_VOLUME_START_FROM1

    if (Ret != RILC_STATUS_SUCCESS) {
        RLOGE("%s::%s error: send failure", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }

    m_transaction[RILC_REQ_AUDIO_SET_VOLUME] = true;

    return RILAUDIO_ERROR_NONE;
}

int RilAudioApi::SetAudioPath(int audioPath)
{
    RLOGI("%s::%s audioPath=%d", TAG, __FUNCTION__, audioPath);
    if (m_pRilClientHelper == NULL || m_client == NULL) {
        RLOGW("%s::%s error: not initialized", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_NO_DEVICE;
    }

    if (!((audioPath >= RILAUIDO_PATH_HANDSET && audioPath < RILAUIDO_PATH_MAX))) {
        RLOGW("%s::%s error: out of range. audioPath=%d", TAG, __FUNCTION__, audioPath);
        return RILAUDIO_ERROR_INVALID_PARAM;
    }

    if (m_OnClosing) {
        RLOGE("%s::%s On Closing", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }

    if (m_pRilClientHelper->Send(m_client, RILC_REQ_AUDIO_SET_PATH, (void*)(&audioPath), sizeof(audioPath), OnSolicitedResponse) != RILC_STATUS_SUCCESS) {
        RLOGE("%s::%s error: send failure", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }
    m_transaction[RILC_REQ_AUDIO_SET_PATH] = true;

    return RILAUDIO_ERROR_NONE;
}

int RilAudioApi::SetMultiMic(int mode)
{
    RLOGI("%s::%s mode=%d", TAG, __FUNCTION__, mode);
    if (m_pRilClientHelper == NULL || m_client == NULL) {
        RLOGW("%s::%s error: not initialized", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_NO_DEVICE;
    }

    if (!(mode == RILAUDIO_MULTI_MIC_OFF || mode == RILAUDIO_MULTI_MIC_ON)) {
        RLOGW("%s::%s error: out of range. mode=%d", TAG, __FUNCTION__, mode);
        return RILAUDIO_ERROR_INVALID_PARAM;
    }

    if (m_OnClosing) {
        RLOGE("%s::%s On Closing", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }

    if (m_pRilClientHelper->Send(m_client, RILC_REQ_AUDIO_SET_MIC, (void*)(&mode), sizeof(mode), OnSolicitedResponse) != RILC_STATUS_SUCCESS) {
        RLOGE("%s::%s error: send failure", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }
    m_transaction[RILC_REQ_AUDIO_SET_MIC] = true;

    return RILAUDIO_ERROR_NONE;
}

int RilAudioApi::SetMute(int mode)
{
    RLOGI("%s::%s mode=%d", TAG, __FUNCTION__, mode);
    if (m_pRilClientHelper == NULL || m_client == NULL) {
        RLOGW("%s::%s error: not initialized", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_NO_DEVICE;
    }

    if (!(mode == RILAUDIO_MUTE_DISABLED || mode == RILAUDIO_MUTE_ENABLED)) {
        RLOGW("%s::%s error: out of range. mode=%d", TAG, __FUNCTION__, mode);
        return RILAUDIO_ERROR_INVALID_PARAM;
    }

    if (m_OnClosing) {
        RLOGE("%s::%s On Closing", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }

    if (m_pRilClientHelper->Send(m_client, RILC_REQ_AUDIO_SET_MUTE, (void*)(&mode), sizeof(mode), OnSolicitedResponse) != RILC_STATUS_SUCCESS) {
        RLOGE("%s::%s error: send failure", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }
    m_transaction[RILC_REQ_AUDIO_SET_MUTE] = true;

    return RILAUDIO_ERROR_NONE;
}

int RilAudioApi::SetAudioClock(int mode)
{
    RLOGI("%s::%s mode=%d", TAG, __FUNCTION__, mode);
    if (m_pRilClientHelper == NULL || m_client == NULL) {
        RLOGW("%s::%s error: not initialized", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_NO_DEVICE;
    }

    if (!(mode == RILAUDIO_TURN_OFF_I2S || mode == RILAUDIO_TURN_ON_I2S)) {
        RLOGW("%s::%s error: out of range. mode=%d", TAG, __FUNCTION__, mode);
        return RILAUDIO_ERROR_INVALID_PARAM;
    }

    if (m_OnClosing) {
        RLOGE("%s::%s On Closing", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }

    if (m_pRilClientHelper->Send(m_client, RILC_REQ_AUDIO_SET_AUDIO_CLOCK, (void*)(&mode), sizeof(mode), OnSolicitedResponse) != RILC_STATUS_SUCCESS) {
        RLOGE("%s::%s error: send failure", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }
    m_transaction[RILC_REQ_AUDIO_SET_AUDIO_CLOCK] = true;

    return RILAUDIO_ERROR_NONE;
}

typedef struct tagLoopbackData{
    int onoff;
    int path;
}LoopbackData;

int RilAudioApi::SetAudioLoopback(int onoff, int path)
{
    RLOGI("%s::%s onoff=%d, path=%d", TAG, __FUNCTION__, onoff, path);
    if (m_pRilClientHelper == NULL || m_client == NULL) {
        RLOGW("%s::%s error: not initialized", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_NO_DEVICE;
    }

    LoopbackData data;
    data.onoff = onoff;
    data.path = path;

    if (!(onoff == RILAUDIO_LOOPBACK_STOP || onoff == RILAUDIO_LOOPBACK_START)) {
        RLOGW("%s::%s error: out of range. onoff=%d", TAG, __FUNCTION__, onoff);
        return RILAUDIO_ERROR_INVALID_PARAM;
    }

    if ( !( RILAUDIO_LOOPBACK_PATH_NA <= path && path < RILAUDIO_LOOPBACK_PATH_MAX)) {
        RLOGW("%s::%s error: out of range. path=%d", TAG, __FUNCTION__, path);
        return RILAUDIO_ERROR_INVALID_PARAM;
    }

    if (m_OnClosing) {
        RLOGE("%s::%s On Closing", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }

    if (m_pRilClientHelper->Send(m_client, RILC_REQ_AUDIO_SET_AUDIO_LOOPBACK, (void*)(&data), sizeof(LoopbackData), OnSolicitedResponse) != RILC_STATUS_SUCCESS) {
        RLOGE("%s::%s error: send failure", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }
    m_transaction[RILC_REQ_AUDIO_SET_AUDIO_LOOPBACK] = true;

    return RILAUDIO_ERROR_NONE;
}

int RilAudioApi::SetTtyMode(int ttyMode)
{
    RLOGI("%s::%s ttyMode=%d", TAG, __FUNCTION__, ttyMode);
    if (m_pRilClientHelper == NULL || m_client == NULL) {
        RLOGW("%s::%s error: not initialized", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_NO_DEVICE;
    }

    switch (ttyMode) {
    case TTY_MODE_OFF:
    case TTY_MODE_FULL:
    case TTY_MODE_VCO:
    case TTY_MODE_HCO:
        break;
    default:
        RLOGW("%s::%s unsupported tty mode", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_INVALID_PARAM;
    }

    if (m_OnClosing) {
        RLOGE("%s::%s On Closing", TAG, __FUNCTION__);
        return RILAUDIO_ERROR_SEND_FAIL;
    }

    int ret = RILAUDIO_ERROR_NONE;
    if (m_pRilClientHelper->Send(m_client, RILC_REQ_AUDIO_SET_TTY_MODE, (void*)(&ttyMode), sizeof(ttyMode),
            NULL, RIL_SOCKET_1) != RILC_STATUS_SUCCESS) {
        RLOGE("%s::%s error: send failure for phoneId %d", TAG, __FUNCTION__, RIL_SOCKET_1);
        ret =  RILAUDIO_ERROR_SEND_FAIL;
    }
    if (m_pRilClientHelper->Send(m_client, RILC_REQ_AUDIO_SET_TTY_MODE, (void*)(&ttyMode), sizeof(ttyMode),
                NULL, RIL_SOCKET_2) != RILC_STATUS_SUCCESS) {
        RLOGE("%s::%s error: send failure for phoneId %d", TAG, __FUNCTION__, RIL_SOCKET_2);
        ret =  RILAUDIO_ERROR_SEND_FAIL;
    }

    return ret;
}
