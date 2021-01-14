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
    RIL-AIMS Interface
*/

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define LOG_TAG "RIL-AIMS"
#include <utils/Log.h>
#include <telephony/ril.h>

#include "RilAimsInterface.h"
#include "sitril-client.h"


// ####  Logs Selection ####
//#define ENABLE_LOGS_FUNC_ENTER_EXIT
#define ENABLE_ANDROID_LOG
#define LogE    ALOGE
#define LogW    ALOGW
#define LogN    ALOGI
#ifdef ENABLE_ANDROID_LOG
#define LogI    ALOGI
#define LogV    ALOGI
#endif // end  of ENABLE_ANDROID_LOG

// #### Definitions ####
#define SITRIL_CLIENT_LIB_PATH "libsitril-client.so"

#define RIL_AIMS_CHANNEL_UNKWON RIL_SOCKET_UNKNOWN

#define ENTER_FUNC()        { LogV("%s() [<-- ", __FUNCTION__); }
#define LEAVE_FUNC()        { LogV("%s() [--> ", __FUNCTION__); }

// #### Global Variables ####
void *gdlHandle[MAX_RILC_IMS_CLIENT];

void *gSITRILC_Handle[MAX_RILC_IMS_CLIENT];
void *(*gSITRILC_Open[MAX_RILC_IMS_CLIENT])(void);
int (*gSITRILC_Close[MAX_RILC_IMS_CLIENT])(void* client);
int (*gSITRILC_Reconnect[MAX_RILC_IMS_CLIENT])(void* client);
int (*gSITRILC_RegisterUnsolicitedHandler[MAX_RILC_IMS_CLIENT])(void* client, Rilc_OnUnsolicitedResponse handler);
int (*gSITRILC_Send[MAX_RILC_IMS_CLIENT])(void* client, unsigned msgId, void* data, size_t length, Rilc_OnResponse handler, unsigned int channel);

static RilAimsUnsolResponse gSITRilImsUnsolCallback[MAX_RILC_IMS_CLIENT];

#if 0
// #### Internal Functions ####
static void SITRilClient_OnResponse(unsigned msgId, int status, void* data, size_t length, unsigned int channel)
{
    ENTER_FUNC();
    LogV("%s(msgId(%d), status(%d), data(0x%p), length(%zu) channel(%d)",__FUNCTION__, msgId, status,data, length, channel);

    switch(msgId)
    {
        //example
        //case RILC_REQ_IMS_SETUP:
        //    LogN("%s() msgId(%d) is sent",__FUNCTION__,msgId);
        //    break;

        default:
            LogE("%s() !! ERROR !!, Unknown msgId = %d",__FUNCTION__,msgId);
            break;
    }

    LEAVE_FUNC();
}
#endif

static void SITRilClient__OnUnsolicitedResponse(unsigned msgId, void* data, size_t length, unsigned int channel)
{
    ENTER_FUNC();
    int i;

    LogV("%s() msgId(%d), data(0x%p), length(%zu) channel(%d)",__FUNCTION__, msgId, data, length, channel);

    do
    {
        for(i=0; i<MAX_RILC_IMS_CLIENT; i++)
        {
            if(gSITRilImsUnsolCallback[i] != NULL) break;
        }

        if(i == MAX_RILC_IMS_CLIENT)
        {
            LogE("%s() !! ERROR !!, gSITRilImsUnsolCallback == NULL",__FUNCTION__);
            break;
        }

        // Check IMS msgId
        switch(msgId)
        {
            case RILC_UNSOL_IMS_CONFIGURATION:
                LogV("%s() RILC_UNSOL_IMS_CONFIGURATION msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_IMS_DEDICATED_PDN_INFO:
                LogV("%s() RILC_UNSOL_IMS_DEDICATED_PDN_INFO msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_IMS_EMERGENCY_ACT_INFO:
                LogV("%s() RILC_UNSOL_IMS_EMERGENCY_ACT_INFO msgId = %d, len = %zu",__FUNCTION__,msgId, length);
                break;
            case RILC_UNSOL_IMS_SRVCC_HO:
                LogV("%s() RILC_UNSOL_IMS_SRVCC_HO msgId = %d, len = %zu",__FUNCTION__,msgId, length);
                break;
            case RILC_UNSOL_IMS_EMERGENCY_CALL_LIST:
                LogV("%s() RILC_UNSOL_IMS_EMERGENCY_CALL_LIST msgId = %d, len = %zu",__FUNCTION__,msgId, length);
                break;
            case RILC_UNSOL_IMS_SUPPORT_SERVICE:
                LogV("%s() RILC_UNSOL_IMS_SUPPORT_SERVICE msgId = %d, Volte %d, EMC %d, sim %d, rat %d",__FUNCTION__,msgId,*((char *)data),*((char *)data+1),*((char *)data+2),*((char *)data+3));
                break;
            case RILC_UNSOL_IMS_OPEN_CARRIER_INFO:
                LogV("%s() RILC_UNSOL_IMS_OPEN_CARRIER_INFO msgId = %d, len = %zu",__FUNCTION__, msgId, length);
                break;

        //AIMS support start ---------------------
            case RILC_UNSOL_AIMS_CALL_RING:
                LogV("%s() RILC_UNSOL_AIMS_CALL_RING msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_CALL_STATUS:
                LogV("%s() RILC_UNSOL_AIMS_CALL_STATUS msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_REGISTRATION:
                LogV("%s() RILC_UNSOL_AIMS_REGISTRATION msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_CALL_MODIFY:
                LogV("%s() RILC_UNSOL_AIMS_CALL_MODIFY msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_FRAME_TIME:
                LogV("%s() RILC_UNSOL_AIMS_FRAME_TIME msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_SUPP_SVC_NOTIFICATION:
                LogV("%s() RILC_UNSOL_AIMS_SUPP_SVC_NOTIFICATION msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_NEW_SMS:
                LogV("%s() RILC_UNSOL_AIMS_NEW_SMS msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_NEW_SMS_STATUS_REPORT:
                LogV("%s() RILC_UNSOL_AIMS_NEW_SMS_STATUS_REPORT msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_ON_USSD:
                LogV("%s() RILC_UNSOL_AIMS_ON_USSD msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_CONFERENCE_CALL_EVENT:
                LogV("%s() RILC_UNSOL_AIMS_CONFERENCE_CALL_EVENT msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_HO_PAYLOAD:
                LogV("%s() RILC_UNSOL_AIMS_HO_PAYLOAD msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_VOWIFI_HO_CALL_INFO:
                LogV("%s RILC_UNSOL_AIMS_VOWIFI_HO_CALL_INFO msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_NEW_CDMA_SMS:
                LogV("%s RILC_UNSOL_AIMS_NEW_CDMA_SMS msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_CALL_MANAGE:
                LogV("%s() RILC_UNSOL_AIMS_CALL_MANAGE msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_CONF_CALL_ADD_REMOVE_USER:
                LogV("%s() RILC_UNSOL_AIMS_CONF_CALL_ADD_REMOVE_USER msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_ENHANCED_CONF_CALL:
                LogV("%s() RILC_UNSOL_AIMS_ENHANCED_CONF_CALL msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_CALL_MODIFY_RSP:
                LogV("%s() RILC_UNSOL_AIMS_CALL_MODIFY_RSP msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_DTMF_EVENT:
                LogV("%s() RILC_UNSOL_AIMS_DTMF_EVENT msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RTT_NEW_TEXT:
                LogV("%s() RILC_UNSOL_AIMS_RTT_NEW_TEXT msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RTT_FAIL_SENDING_TEXT:
                LogV("%s() RILC_UNSOL_AIMS_RTT_FAIL_SENDING_TEXT msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RCS_MULTI_FRAME:
                LogV("%s() RILC_UNSOL_AIMS_RCS_MULTI_FRAME msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RCS_CHAT:
                LogV("%s() RILC_UNSOL_AIMS_RCS_CHAT msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RCS_GROUP_CHAT:
                LogV("%s() RILC_UNSOL_AIMS_RCS_GROUP_CHAT msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RCS_OFFLINE_MODE:
                LogV("%s() RILC_UNSOL_AIMS_RCS_OFFLINE_MODE msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RCS_FILE_TRANSFER:
                LogV("%s() RILC_UNSOL_AIMS_RCS_FILE_TRANSFER msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RCS_COMMON_MESSAGE:
                LogV("%s() RILC_UNSOL_AIMS_RCS_COMMON_MESSAGE msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RCS_CONTENT_SHARE:
                LogV("%s() RILC_UNSOL_AIMS_RCS_CONTENT_SHARE msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RCS_PRESENCE:
                LogV("%s() RILC_UNSOL_AIMS_RCS_PRESENCE msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RCS_XCAP_MANAGE:
                LogV("%s() RILC_UNSOL_AIMS_RCS_XCAP_MANAGE msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RCS_CONFIG_MANAGE:
                LogV("%s() RILC_UNSOL_AIMS_RCS_CONFIG_MANAGE msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_RCS_TLS_MANAGE:
                LogV("%s() RILC_UNSOL_AIMS_RCS_TLS_MANAGE msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_EXIT_EMERGENCY_CB_MODE:
                LogV("%s() RILC_UNSOL_AIMS_EXIT_EMERGENCY_CB_MODE msgId = %d",__FUNCTION__,msgId);
                break;
           case RILC_UNSOL_AIMS_DIALOG_INFO:
                LogV("%s() RILC_UNSOL_AIMS_DIALOG_INFO msgId = %d",__FUNCTION__,msgId);
                break;
            case RILC_UNSOL_AIMS_AC_BARRING_INFO:
                LogV("%s() RILC_UNSOL_AIMS_AC_BARRING_INFO msgId = %d",__FUNCTION__,msgId);
                break;
        //AIMS support end ----------------------

        //Audio support
            case RILC_UNSOL_AUDIO_RINGBACK:
                LogV("%s() RILC_UNSOL_AUDIO_RINGBACK msgId = %d",__FUNCTION__,msgId);
                break;

            case RILC_UNSOL_WFC_RTP_RTCP_TIMEOUT:
                LogV("%s() RILC_UNSOL_WFC_RTP_RTCP_TIMEOUT msgId = %d",__FUNCTION__,msgId);
                break;

            case RILC_UNSOL_WFC_FIRST_RTP:
                LogV("%s() RILC_UNSOL_WFC_FIRST_RTP msgId = %d",__FUNCTION__,msgId);
                break;

            case RILC_UNSOL_WFC_RTCP_RX_SR:
                LogV("%s() RILC_UNSOL_WFC_RTCP_RX_SR msgId = %d",__FUNCTION__,msgId);
                break;

            case RILC_UNSOL_WFC_RCV_DTMF_NOTI:
                LogV("%s() RILC_UNSOL_WFC_RCV_DTMF_NOTI msgId = %d",__FUNCTION__,msgId);
                break;

            case RILC_UNSOL_WFC_SIGNAL_STRENGTH:
                LogV("%s() RILC_UNSOL_WFC_SIGNAL_STRENGTH msgId = %d",__FUNCTION__,msgId);
                break;

            default:
                LogE("%s() !! ERROR !!, Unknown msgId = %d",__FUNCTION__,msgId);
                return;
        }

        // Call Callback Function
        for(i=0; i<MAX_RILC_IMS_CLIENT; i++)
        {
            if(gSITRilImsUnsolCallback[i])
            {
                LogV("%s() Send unsol msg msgId = %d, client id = %d, socket_id = %d",__FUNCTION__,msgId, i, channel);
                gSITRilImsUnsolCallback[i](msgId, data, length, channel);
            }
        }
    }while(0);

    LEAVE_FUNC();
}

/**
 *
 */
static int RequestAims(int client_id, unsigned int msgId, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAimsForChannelId(client_id, msgId, data, data_len, callback, RIL_AIMS_CHANNEL_UNKWON);
}


// #### Initialization Functions ####
/**
 * @deprecated
 */
int Open(int client_id)
{
    return RilAimsOpen(client_id);
}

/**
 * RilAimsOpen
 *
 * @desc Open RIL-AIMS Interaface.
 * @params
 * @Return an error value of RilAimsError
 */
int RilAimsOpen(int client_id)
{
    int ret = RIL_AIMS_ERROR_SUCCESS;
    int i;
    const char * SITRilClientLibPath = SITRIL_CLIENT_LIB_PATH;

    ENTER_FUNC();

    do
    {
        // Loading SITRil Client Library
        if(gdlHandle[client_id] != NULL)
        {
            LogW("%s() SITRilClient(%s) already is opened ...",__FUNCTION__,SITRilClientLibPath);
            break;
        }

        gdlHandle[client_id] = dlopen(SITRilClientLibPath, RTLD_NOW);
        if(gdlHandle[client_id] == NULL)
        {
            LogE("%s() client_id : %d !! ERROR !!, Fail in dlopen()",__FUNCTION__, client_id);
            ret = RIL_AIMS_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }

        gSITRILC_Open[client_id] = NULL;
        gSITRILC_Close[client_id] = NULL;
        gSITRILC_Reconnect[client_id] = NULL;
        gSITRILC_RegisterUnsolicitedHandler[client_id] = NULL;
        gSITRILC_Send[client_id] = NULL;

        gSITRILC_Open[client_id] = (void *(*)(void))dlsym(gdlHandle[client_id], "RILC_Open");
        if(gSITRILC_Open[client_id] == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_Open[%d]",__FUNCTION__, client_id);
            ret = RIL_AIMS_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("%s() Martin gSITRILC_Open[%d]:0x%p ...",__FUNCTION__,client_id, gSITRILC_Open);

        gSITRILC_Close[client_id] = (int (*)(void*))dlsym(gdlHandle[client_id], "RILC_Close");
        if(gSITRILC_Close[client_id] == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_Close[%d]",__FUNCTION__, client_id);
            ret = RIL_AIMS_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("%s() gSITRILC_Close[%d]:0x%p ...",__FUNCTION__,client_id, gSITRILC_Close);

        gSITRILC_Reconnect[client_id] = (int (*)(void*))dlsym(gdlHandle[client_id], "RILC_Reconnect");
        if(gSITRILC_Reconnect[client_id] == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_Reconnect",__FUNCTION__);
            ret = RIL_AIMS_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("%s() gSITRILC_Reconnect[%d]:0x%p ...",__FUNCTION__,client_id, gSITRILC_Reconnect);

        gSITRILC_RegisterUnsolicitedHandler[client_id] = (int (*)(void*, Rilc_OnUnsolicitedResponse))dlsym(gdlHandle[client_id], "RILC_RegisterUnsolicitedHandler");
        if(gSITRILC_RegisterUnsolicitedHandler[client_id] == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_RegisterUnsolicitedHandler[%d]",__FUNCTION__, client_id);
            ret = RIL_AIMS_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("%s() gSITRILC_RegisterUnsolicitedHandler[%d]:0x%p ...",__FUNCTION__,client_id, gSITRILC_RegisterUnsolicitedHandler);

        gSITRILC_Send[client_id] = (int (*)(void*, unsigned, void*, size_t, Rilc_OnResponse, unsigned int))dlsym(gdlHandle[client_id], "RILC_Send");
        if(gSITRILC_Send[client_id] == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_Send[%d]",__FUNCTION__, client_id);
            ret = RIL_AIMS_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("%s() gSITRILC_Send[%d]:0x%p ...",__FUNCTION__,client_id, gSITRILC_Send);

        // Open SITRil Client Library
        gSITRILC_Handle[client_id] = gSITRILC_Open[client_id]();
        if(gSITRILC_Handle[client_id] == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in gSITRILC_Open[%d]()",__FUNCTION__, client_id);
            ret = RIL_AIMS_ERROR_NO_DEVICE;
            break;
        }

        LogV("%s() gSITRILC_Open[%d]() ... OK.",__FUNCTION__, client_id);


        // Reigister Callback
        for(i=0; i<MAX_RILC_IMS_CLIENT; i++)
        {
            if(gSITRilImsUnsolCallback[i] != NULL) break;
        }

        if(i == MAX_RILC_IMS_CLIENT)
        {
            ret = gSITRILC_RegisterUnsolicitedHandler[client_id](gSITRILC_Handle[client_id], SITRilClient__OnUnsolicitedResponse);

            if(ret)
            {
                LogE("%s() !! ERROR !!, Fail in gSITRILC_RegisterUnsolicitedHandler[%d]()",__FUNCTION__, client_id);
                ret = RIL_AIMS_ERROR_REGISTERATION_FAIL;
                break;
            }
            LogV("%s() gSITRILC_RegisterUnsolicitedHandler[%d]() ... OK.",__FUNCTION__, client_id);
        }
        else
        {
            LogV("%s() Already registered gSITRILC_RegisterUnsolicitedHandler ... OK.",__FUNCTION__);
        }

    }while(0);

    if(ret != RIL_AIMS_ERROR_SUCCESS)
    {
        Close(client_id);
    }

    LEAVE_FUNC();

    return ret;
}

/**
 * @deprecated
 */
int Close(int client_id)
{
    return RilAimsClose(client_id);
}

/**
 * RilAimsClose
 *
 * @desc Close RIL-AIMS Interaface.
 * @params
 * @Return an error value of RilAimsError
 */
int RilAimsClose(int client_id)
{
    int ret = RIL_AIMS_ERROR_SUCCESS;

    ENTER_FUNC();

    if(gSITRILC_Send[client_id])
        gSITRILC_Send[client_id] = NULL;

    if(gSITRILC_RegisterUnsolicitedHandler[client_id])
        gSITRILC_RegisterUnsolicitedHandler[client_id] = NULL;

    if(gSITRILC_Reconnect[client_id])
        gSITRILC_Reconnect[client_id] = NULL;

    if(gSITRILC_Close[client_id])
    {
        if(gSITRILC_Handle[client_id])
        {
            gSITRILC_Close[client_id](gSITRILC_Handle[client_id]);
            LogV("%s() gSITRILC_Close[%d]() ... OK.",__FUNCTION__, client_id);
            gSITRILC_Handle[client_id] = NULL;
        }
        gSITRILC_Close[client_id] = NULL;
    }

    if(gSITRILC_Open[client_id])
        gSITRILC_Open[client_id] = NULL;

    if(gdlHandle[client_id])
    {
        dlclose(gdlHandle[client_id]);
        gdlHandle[client_id] = NULL;
    }

    LEAVE_FUNC();

    return ret;
}

int RilAimsRegisterCallback(int client_id, RilAimsUnsolResponse callback)
{
    int ret = RIL_AIMS_ERROR_SUCCESS;

    ENTER_FUNC();

    do
    {
        if(gSITRilImsUnsolCallback[client_id] != NULL)
        {
            LogE("%s() !! ERROR !!, gSITRilImsCallback(%p)",__FUNCTION__,gSITRilImsUnsolCallback);
            ret = RIL_AIMS_ERROR_ALREADY_REGISTERD;
            break;
        }

        gSITRilImsUnsolCallback[client_id] = callback;
        LogV("%s() new gSITRilImsCallback[%d](%p)",__FUNCTION__,client_id, gSITRilImsUnsolCallback[client_id]);

    }while(0);

    LEAVE_FUNC();

    return ret;
}

// #### Operation Functions ####
// ======================================================
// API Name : Set IMS Configuration
// ----------------------------------------------------
int SetImsConfiguration(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_IMS_SET_CONFIGURATION, data, data_len, callback);
}

// ======================================================
// API Name : Get IMS Configuration
// ----------------------------------------------------
int GetImsConfiguration(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_IMS_GET_CONFIGURATION, data, data_len, callback);
}

// ======================================================
// API Name : ISIM Authentication
// ----------------------------------------------------
int GetSimAuth(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_IMS_SIM_AUTH, data, data_len, callback);
}

// ======================================================
// API Name : Set IMS Emergency call status
// ----------------------------------------------------
int SetEmergencyCallStatus(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_IMS_SET_EMERGENCY_CALL_STATUS, data, data_len, callback);
}

// ======================================================
// API Name : Set Srvcc call list
// ----------------------------------------------------
int SetSrvccCallList(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_IMS_SET_SRVCC_CALL_LIST, data, data_len, callback);
}

// ======================================================
// API Name : Get GBA Auth
// ----------------------------------------------------
int GetGBAAuth(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_IMS_GET_GBA_AUTH, data, data_len, callback);
}

//AIMS support start ---------------------
int RequestAimsForChannelId(int client_id, unsigned int msgId, void *data, int data_len, RilAimsOnResponse callback, int channel)
{
    int ret = RIL_AIMS_ERROR_SUCCESS;

    ENTER_FUNC();
    LogI("%s: msgId=%d data=0x0%p data_len=%d callback=0x%p", __FUNCTION__, msgId, data, data_len, callback);
    if (client_id < 0 || client_id >= MAX_RILC_IMS_CLIENT) {
        LogE("%s() !! ERROR !!, array out of index. client_id=%d",__FUNCTION__, client_id);
        return RIL_AIMS_ERROR_INVALID_PARAM;
    }

    if(gSITRILC_Handle[client_id] == NULL)
    {
        LogE("%s() !! ERROR !!, gSITRILC_Handle[%d] == NULL",__FUNCTION__, client_id);
        ret = RIL_AIMS_ERROR_NO_LIB_OPEN_FAIL;
        goto exit;
    }

    if (!(channel == RIL_AIMS_CHANNEL_0 || channel == RIL_AIMS_CHANNEL_1 || channel == RIL_AIMS_CHANNEL_UNKWON)) {
        LogE("%s() !! ERROR !!, unsupported channel=%d",__FUNCTION__, channel);
        return RIL_AIMS_ERROR_INVALID_PARAM;
    }

    // Sending A message
    if (gSITRILC_Send[client_id] != NULL) {
        ret = gSITRILC_Send[client_id](gSITRILC_Handle[client_id], msgId, (void*)(data), data_len, callback, channel);
        if(ret)
        {
            LogE("%s() !! ERROR !!, Fail in gSITRILC_Send[%d]() ",__FUNCTION__, client_id);
            ret = RIL_AIMS_ERROR_NO_LIB_SEND_FAIL;
            goto exit;
        }
    }
    else {
        LogE("%s() !! ERROR !!, gSITRILC_Send[%d] == NULL",__FUNCTION__, client_id);
        return RIL_AIMS_ERROR_NO_LIB_SEND_FAIL;
    }

exit:
    LEAVE_FUNC();
    return ret;
}

int RequestAimsDial(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_DIAL, data, data_len, callback);
}

int RequestAimsAnswer(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_ANSWER, data, data_len, callback);
}

int ReqeustAimsHangup(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_HANGUP, data, data_len, callback);
}

int RequestAimsDegregistration(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_DEREGISTRATION, data, data_len, callback);
}

int RequestAimsHiddenMenu(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_HIDDEN_MENU, data, data_len, callback);
}

int RequestAimsPdnInfo(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_ADD_PDN_INFO, data, data_len, callback);
}

int RequestAimsCallManage(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_CALL_MANAGE, data, data_len, callback);
}

int RequestAimsSendDtmf(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SEND_DTMF, data, data_len, callback);
}

int RequestAimsSetFrameTime(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SET_FRAME_TIME, data, data_len, callback);
}

int RequestAimsGetFrameTime(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_GET_FRAME_TIME, data, data_len, callback);
}

int RequestAimsCallModify(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_CALL_MODIFY, data, data_len, callback);
}

int RequestAimsResponseCallModify(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_RESPONSE_CALL_MODIFY, data, data_len, callback);
}

int RequestAimsTimeInfo(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_TIME_INFO, data, data_len, callback);
}

int RequestAimsConfCallAddRemoveUser(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_CONF_CALL_ADD_REMOVE_USER, data, data_len, callback);
}

int RequestAimsEnhancedConfCall(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_ENHANCED_CONF_CALL, data, data_len, callback);
}

int RequestAimsGetCallForwardStatus(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_GET_CALL_FORWARD_STATUS, data, data_len, callback);
}

int RequestAimsSetCallForwardStatus(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SET_CALL_FORWARD_STATUS, data, data_len, callback);
}

int RequestAimsGetCallWaiting(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_GET_CALL_WAITING, data, data_len, callback);
}

int RequestAimsSetCallWaiting(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SET_CALL_WAITING, data, data_len, callback);
}

int RequestAimsGetCallBarring(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_GET_CALL_BARRING, data, data_len, callback);
}

int RequestAimsSetCallBarring(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SET_CALL_BARRING, data, data_len, callback);
}

int RequestAimsSendSms(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SEND_SMS, data, data_len, callback);
}

int RequestAimsSendSmsExpectMore(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SEND_EXPECT_MORE, data, data_len, callback);
}

int RequestAimsSendSmsAck(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SEND_SMS_ACK, data, data_len, callback);
}

int RequestAimsAckIncomingGsmSms(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SEND_ACK_INCOMING_SMS, data, data_len, callback);
}

int RequestAimsChangeBarringPassword(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_CHG_BARRING_PWD, data, data_len, callback);
}

int RequestAimsSendUssdInfo(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SEND_USSD_INFO, data, data_len, callback);
}

int RequestAimsGetPresentationSettings(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_GET_PRESENTATION_SETTINGS, data, data_len, callback);
}

int RequestAimsSetPresentationSettings(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SET_PRESENTATION_SETTINGS, data, data_len, callback);
}

int RequestAimsSetSelfCapability(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SET_SELF_CAPABILITY, data, data_len, callback);
}

int RequestAimsHoToWifiReady(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_HO_TO_WIFI_READY, data, data_len, callback);
}

int RequestAimsHoToWifiCancel(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_HO_TO_WIFI_CANCEL_IND, data, data_len, callback);
}

int RequestAimsHoPayloadInd(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_HO_PAYLOAD_IND, data, data_len, callback);
}

int RequestAimsHoTo3gpp(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_HO_TO_3GPP, data, data_len, callback);
}

int RequestAimsAckIncomingCdmaSms(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_SEND_ACK_INCOMING_CDMA_SMS, data, data_len, callback);
}

int RequestAimsMediaStateInd(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_MEDIA_STATE_IND, data, data_len, callback);
}

int RequestWfcSetVowifiHoThreshold(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_WFC_SET_VOWIFI_HO_THRESHOLD, data, data_len, callback);
}

int RequestAimsRttSendText(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_RTT_SEND_TEXT, data, data_len, callback);
}

int RequestAimsExitEmergencyCbMode(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback)
{
    return RequestAims(client_id, RILC_REQ_AIMS_RTT_SEND_TEXT, data, data_len, callback);
}
//AIMS support end ---------------------
