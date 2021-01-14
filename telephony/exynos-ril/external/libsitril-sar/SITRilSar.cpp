/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <utils/Log.h>
#include <unistd.h>

#include "SITRilSar.h"
#include "sitril-client.h"

// ####  Logs Seleection ####
//#define ENABLE_LOGS_FUNC_ENTER_EXIT
#define ENABLE_ANDROID_LOG
#define LogE    ALOGE
#define LogW    ALOGW
#define LogN    ALOGI
#ifdef ENABLE_ANDROID_LOG
#define LogI    ALOGI
#define LogV    ALOGI
#endif // end  of ENABLE_ANDROID_LOG

// #### Defintions ####
#define SITRIL_CLIENT_LIB_PATH "libsitril-client.so"

#define TAG                 "sar"
#define ENTER_FUNC()        { LogV("[%s]%s() [<-- ", TAG, __FUNCTION__); }
#define LEAVE_FUNC()        { LogV("[%s]%s() [--> ", TAG, __FUNCTION__); }

#define MAX_LOOP_NUM (500)
#define TIME_INTERVAL (100*100)
#define RETVAL_LEN (200)

enum
{
    SET_STATE = 0,
    GET_STATE = 1
};

typedef struct __response_manager {
    int nMsgID;
    int bSuccess;
    int bResponse;
    int retvalue;
} rspData;

static rspData g_returnval[2] = {
    {SET_STATE, false, false, 0},
    {GET_STATE, false, false, 0}
};

// #### Global Variables ####
void *gdlHandle = NULL;
void *gSITRILC_Handle = NULL;
void *(*gSITRILC_Open)(void);
int (*gSITRILC_Close)(void* client);
int (*gSITRILC_Reconnect)(void* client);
int (*gSITRILC_RegisterUnsolicitedHandler)(void* client, Rilc_OnUnsolicitedResponse handler);
int (*gSITRILC_Send)(void* client, unsigned msgId, void* data, size_t length, Rilc_OnResponse handler, unsigned int channel);
int (*gSITRilSarUnsolCallback)(unsigned msgId, void *data, size_t datalen, unsigned int channel);

// #### Internal Functions ####
void SITRilClient_OnResponse(unsigned msgId, int status, void* data, size_t length, unsigned int channel)
{
    ENTER_FUNC();
    LogV("[%s]%s(msgId(%d), status(%d), data(0x%p), length(%zu), channel(%d)++",TAG,__FUNCTION__,msgId,status,data,length,channel);

    switch(msgId)
    {
        case RILC_REQ_SAR_SET_SAR_STATE:
            LogV("[%s]%s() RILC_REQ_SAR_SET_SAR_STATE = %d", TAG, __FUNCTION__, msgId);
            if (status == 0) {
                g_returnval[SET_STATE].bSuccess = true;
            } else {
                g_returnval[SET_STATE].bSuccess = false;
            }
            g_returnval[SET_STATE].bResponse = true;
            break;
        case RILC_REQ_SAR_GET_SAR_STATE:
            LogV("[%s]%s() RILC_REQ_SAR_GET_SAR_STATE = %d", TAG, __FUNCTION__, msgId);
            if (status == 0) {
                g_returnval[GET_STATE].bSuccess = true;
                int *rsp = (int *)data;
                if (rsp != NULL && length > 0) {
                    g_returnval[GET_STATE].retvalue = rsp[0];
                }
            } else {
                g_returnval[GET_STATE].bSuccess = false;
            }
            g_returnval[GET_STATE].bResponse = true;
            break;
        default:
            LogV("[%s]%s() Unkonwn msgId = %d",TAG,__FUNCTION__,msgId);
            break;
    }

    LEAVE_FUNC();
}

void SITRilClient__OnUnsolicitedResponse(unsigned msgId, void* data, size_t length, unsigned int channel)
{
    ENTER_FUNC();

    LogV("[%s]%s() msgId(%d), data(0x%p), length(%zu)",TAG,__FUNCTION__,msgId,data,length);

    do
    {
        if(gSITRilSarUnsolCallback == NULL) {
            LogE("[%s]%s() gSITRilSarUnsolCallback == NULL",TAG,__FUNCTION__);
            break;
        }

        // Check sar msgId
        switch(msgId)
        {
            case RILC_UNSOL_SAR_RF_CONNECTION:
                LogV("[%s]%s() RILC_UNSOL_SAR_RF_CONNECTION = %d",TAG,__FUNCTION__,msgId);
                break;
            default:
                LogV("[%s]%s() Unkonwn msgId = %d",TAG,__FUNCTION__,msgId);
                return;
        }

        // Call Callback Function
        gSITRilSarUnsolCallback(msgId, data, length, channel);

    }while(0);

    LEAVE_FUNC();
}

int RequestSar(unsigned int msgId, int state, int data_len, SITRilSarOnResponse callback, int channel)
{
    int ret = SITRIL_SAR_ERROR_NONE;

    ENTER_FUNC();
    LogI("[%s]%s: msgId=%d state=%d data_len=%d callback=%p",TAG, __FUNCTION__, msgId, state, data_len, callback);

    if(gSITRILC_Handle == NULL)
    {
        LogE("[%s]%s() gSITRILC_Handle == NULL",TAG,__FUNCTION__);
        ret = SITRIL_SAR_ERROR_NO_LIB_OPEN_FAIL;
        goto exit;
    }

    // Sending A message
    ret = gSITRILC_Send(gSITRILC_Handle, msgId, (void*)(&state), data_len, callback, channel);
    if(ret)
    {
        LogE("[%s]%s() Fail in gSITRILC_Send() ",TAG,__FUNCTION__);
        ret = SITRIL_SAR_ERROR_NO_LIB_SEND_FAIL;
        goto exit;
    }

exit:
    LEAVE_FUNC();
    return ret;
}

int RegisterCallback(SITRilSarUnsolCallback callback)
{
    int ret = SITRIL_SAR_ERROR_NONE;

    ENTER_FUNC();

    do
    {
        if(gSITRilSarUnsolCallback != NULL)
        {
            LogE("%s() gSITRilSarUnsolCallback(%p)", __FUNCTION__, gSITRilSarUnsolCallback);
            ret = SITRIL_SAR_ERROR_ALREADY_REGISTERD;
            break;
        }

        gSITRilSarUnsolCallback = callback;
        LogV("%s() new gSITRilSarUnsolCallback(%p)", __FUNCTION__, gSITRilSarUnsolCallback);

    }while(0);

    LEAVE_FUNC();

    return ret;
}

/* ----------------------------------------------------
// Description
//     : Close sar Interaface of SIT Ril.
// Results
//    - int error value of SITRilSarError
// ----------------------------------------------------*/
int SarClose()
{
    int ret = SITRIL_SAR_ERROR_NONE;

    ENTER_FUNC();

    if(gSITRILC_Send)
        gSITRILC_Send = NULL;

    if(gSITRILC_RegisterUnsolicitedHandler)
        gSITRILC_RegisterUnsolicitedHandler = NULL;

    if(gSITRILC_Reconnect)
        gSITRILC_Reconnect = NULL;

    if(gSITRILC_Close) {
        if(gSITRILC_Handle) {
            gSITRILC_Close(gSITRILC_Handle);
            LogI("[%s]%s() gSITRILC_Close() ... OK.",TAG, __FUNCTION__);
            gSITRILC_Handle = NULL;
        }
        gSITRILC_Close = NULL;
    }

    if(gSITRILC_Open)
        gSITRILC_Open = NULL;

    if(gdlHandle) {
        dlclose(gdlHandle);
        gdlHandle = NULL;
    }

    LEAVE_FUNC();

    return ret;
}

/* ----------------------------------------------------
// Description
//     : Open sar Interaface of SIT Ril.
// Results
//    - int error value of SITRilSarError
// ----------------------------------------------------*/
int SarOpen(SITRilSarUnsolCallback callback)
{
    int ret = SITRIL_SAR_ERROR_NONE;
    const char * SITRilClientLibPath = SITRIL_CLIENT_LIB_PATH;

    ENTER_FUNC();

    do {
        // Loading SITRil Client Library
        if(gdlHandle != NULL) {
            LogW("[%s]%s() SITRilClient(%s) already is opened ...",TAG,__FUNCTION__, SITRilClientLibPath);
            break;
        }

        gdlHandle = dlopen(SITRilClientLibPath, RTLD_NOW);
        if(gdlHandle == NULL) {
            LogE("[%s]%s() Fail in dlopen()",TAG, __FUNCTION__);
            ret = SITRIL_SAR_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }

        gSITRILC_Open = NULL;
        gSITRILC_Close = NULL;
        gSITRILC_Reconnect = NULL;
        gSITRILC_RegisterUnsolicitedHandler = NULL;
        gSITRILC_Send = NULL;

        gSITRILC_Open = (void *(*)(void))dlsym(gdlHandle, "RILC_Open");
        if(gSITRILC_Open == NULL) {
            LogE("[%s]%s() Fail in dlsym() for gSITRILC_Open",TAG, __FUNCTION__);
            ret = SITRIL_SAR_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("[%s]%s() gSITRILC_Open:0x%p ...",TAG,__FUNCTION__, gSITRILC_Open);

        gSITRILC_Close = (int (*)(void*))dlsym(gdlHandle, "RILC_Close");
        if(gSITRILC_Close == NULL) {
            LogE("[%s]%s() Fail in dlsym() for gSITRILC_Close",TAG,__FUNCTION__);
            ret = SITRIL_SAR_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("[%s]%s() gSITRILC_Close:0x%p ...",TAG,__FUNCTION__,gSITRILC_Close);

        gSITRILC_Reconnect = (int (*)(void*))dlsym(gdlHandle, "RILC_Reconnect");
        if(gSITRILC_Reconnect == NULL) {
            LogE("[%s]%s() Fail in dlsym() for gSITRILC_Reconnect",TAG,__FUNCTION__);
            ret = SITRIL_SAR_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("[%s]%s() gSITRILC_Reconnect:0x%p ...",TAG,__FUNCTION__,gSITRILC_Reconnect);

        gSITRILC_RegisterUnsolicitedHandler = (int (*)(void*, Rilc_OnUnsolicitedResponse))dlsym(gdlHandle, "RILC_RegisterUnsolicitedHandler");
        if(gSITRILC_RegisterUnsolicitedHandler == NULL) {
            LogE("[%s]%s() Fail in dlsym() for gSITRILC_RegisterUnsolicitedHandler",TAG,__FUNCTION__);
            ret = SITRIL_SAR_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("[%s]%s() gSITRILC_RegisterUnsolicitedHandler:0x%p ...",TAG,__FUNCTION__,gSITRILC_RegisterUnsolicitedHandler);

        gSITRILC_Send = (int (*)(void*, unsigned, void*, size_t, Rilc_OnResponse, unsigned int))dlsym(gdlHandle, "RILC_Send");
        if(gSITRILC_Send == NULL) {
            LogE("[%s]%s() Fail in dlsym() for gSITRILC_Send",TAG,__FUNCTION__);
            ret = SITRIL_SAR_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("[%s]%s() gSITRILC_Send:0x%p ...",TAG,__FUNCTION__,gSITRILC_Send);

        // Open SITRil Client Library
        gSITRILC_Handle = gSITRILC_Open();
        if(gSITRILC_Handle == NULL) {
            LogE("[%s]%s() Fail in gSITRILC_Open()",TAG,__FUNCTION__);
            ret = SITRIL_SAR_ERROR_NO_DEVICE;
            break;
        }

        // Reigister Callback
        ret = gSITRILC_RegisterUnsolicitedHandler(gSITRILC_Handle, SITRilClient__OnUnsolicitedResponse);
        if(ret) {
            LogE("[%s]%s() Fail in gSITRILC_Open()",TAG,__FUNCTION__);
            ret = SITRIL_SAR_ERROR_REGISTERATION_FAIL;
            break;
        }

        ret = RegisterCallback(callback);
        if (ret) {
            LogE("[%s]%s() Fail in registerCallback()",TAG,__FUNCTION__);
            break;
        }

        LogV("[%s]%s() gSITRILC_Open() ... OK.",TAG,__FUNCTION__);

    }while(0);

    if(ret != SITRIL_SAR_ERROR_NONE) {
        SarClose();
    }

    LEAVE_FUNC();

    return ret;
}

/* ----------------------------------------------------
// SIT CMD Name : SIT_OEM_SET_SAR_STATE
// RCM ID : 0x4101
// Description
//     : This solicited message is to set sar state.
// ----------------------------------------------------*/
int SetSarState(int state, int channel)
{
    ENTER_FUNC();
    int ret = 0;
    int i = 0;

    RequestSar(RILC_REQ_SAR_SET_SAR_STATE, state, sizeof(int), SITRilClient_OnResponse, channel);

    while(i++ < MAX_LOOP_NUM) {
        usleep(TIME_INTERVAL);
        if (g_returnval[SET_STATE].bResponse == true) {
            if (g_returnval[SET_STATE].bSuccess== true) {
                LogI("SAR_SET_STATE_SUCESS");
                ret = 1;
            } else {
                LogI("SAR_SET_STATE_FAIL");
                ret = 0;
            }
            break;
        }
    }
    g_returnval[SET_STATE].bSuccess = false;
    g_returnval[SET_STATE].bResponse = false;

    LEAVE_FUNC();
    return ret;
}

/* ----------------------------------------------------
// SIT CMD Name : SIT_OEM_GET_SAR_STATE
// RCM ID : 0x4100
// Description
//     : This solicited message is to get sar state.
// ----------------------------------------------------*/
int GetSarState(int channel)
{
    ENTER_FUNC();
    int ret = -1;
    int i = 0;

    RequestSar(RILC_REQ_SAR_GET_SAR_STATE, 0, 0, SITRilClient_OnResponse, channel);

    while(i++ < MAX_LOOP_NUM) {
        usleep(TIME_INTERVAL);
        if (g_returnval[GET_STATE].bResponse == true) {
            if (g_returnval[GET_STATE].bSuccess == true) {
                ret = g_returnval[GET_STATE].retvalue;
                LogI("SAR_GET_STATE_SUCESS ret(%d)", ret);
            } else {
                LogI("SAR_GET_STATE_FAIL");
            }
            break;
        }
    }
    g_returnval[GET_STATE].bSuccess = false;
    g_returnval[GET_STATE].bResponse = false;
    g_returnval[GET_STATE].retvalue = 0;

    LEAVE_FUNC();
    return ret;
}
