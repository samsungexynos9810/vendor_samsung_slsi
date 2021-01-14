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
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>

#define LOG_TAG "LibSITRil-WLan"
#include <utils/Log.h>

#include "SITRilWLan.h"
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

#define ENTER_FUNC()        { LogV("%s() [<-- ", __FUNCTION__); }
#define LEAVE_FUNC()        { LogV("%s() [--> ", __FUNCTION__); }

// #### Global Variables ####

void *gdlHandle = NULL;

void *gSITRILC_Handle = NULL;
void *(*gSITRILC_Open)(void);
int (*gSITRILC_Close)(void* client);
int (*gSITRILC_Reconnect)(void* client);
int (*gSITRILC_RegisterUnsolicitedHandler)(void* client, Rilc_OnUnsolicitedResponse handler);
int (*gSITRILC_Send)(void* client, unsigned msgId, void* data, size_t length, Rilc_OnResponse handler, unsigned int channel);

int (*gSITRilWLanUnsolCallback)(unsigned msgId, const void *data, size_t datalen, unsigned int channel);

#define RETVAL_LEN  (200)

#define MAX_LOOP_NUM (500)
#define TIME_INTERVAL (100*1000)  // 100ms

int HexString2Value(unsigned char *pHexDecOut, const char *pszHexStrIn);

enum
{
    WLAN_GET_IMSI = 0,
    WLAN_SIM_AUTH
};

typedef struct __response_manager {
    int nMsgID;
    int bSuccess;
    int bResponse;
    char retvalue[RETVAL_LEN];
} rspData;

static rspData g_returnval[2] = {
    {WLAN_GET_IMSI, false, false, {0,}},
    {WLAN_SIM_AUTH, false, false, {0,}}
};

// #### Internal Functions ####
void SITRilClient_OnResponse(unsigned msgId, int status, void* data, size_t length, unsigned int channel)
{
    ENTER_FUNC();

    LogV("%s(msgId(%d), status(%d), data(0x%p), length(%zu)++",__FUNCTION__,msgId,status,data,length);

    switch(msgId)
    {
        case RILC_REQ_WLAN_GET_IMSI :
            if (status == 0) {
                g_returnval[WLAN_GET_IMSI].bSuccess = true;
                memcpy(g_returnval[WLAN_GET_IMSI].retvalue, (const char *)data, length);
            } else {
                g_returnval[WLAN_GET_IMSI].bSuccess = false;
            }
            g_returnval[WLAN_GET_IMSI].bResponse = true;
            break;

        case RILC_REQ_WLAN_SIM_AUTHENTICATE :
            if (status == 0) {
                g_returnval[WLAN_SIM_AUTH].bSuccess = true;
                HexString2Value((unsigned char*)g_returnval[WLAN_SIM_AUTH].retvalue, (const char *)data);
            } else {
                g_returnval[WLAN_SIM_AUTH].bSuccess = false;
            }
            g_returnval[WLAN_SIM_AUTH].bResponse = true;
            break;
        default:
            LogE("%s() !! ERROR !!, Unkonwn msgId = %d",__FUNCTION__,msgId);
            break;
    }

    LEAVE_FUNC();

}

/*
void SITRilClient_OnUnsolicitedResponse(unsigned msgId, void* data, size_t length, unsigned int channel)
{
    int newRilstateforaudio= 0;

    ENTER_FUNC();

    do
    {
        if(gSITRilWLanUnsolCallback == NULL)
            break;

        switch(msgId)
        {
            default:
                LogE("%s() !! ERROR !!, Unkonwn msgId = %d", __FUNCTION__, msgId);
                return;
        }

        gSITRilWLanUnsolCallback(msgId, data, length, channel);
    } while(0);

    LEAVE_FUNC();

}*/


// #### Initialization Functions ####

int Close(void)
{
    int ret = SITRIL_WLAN_ERROR_NONE;

    ENTER_FUNC();

    if(gSITRILC_Send)
        gSITRILC_Send = NULL;

    if(gSITRILC_RegisterUnsolicitedHandler)
        gSITRILC_RegisterUnsolicitedHandler = NULL;

    if(gSITRILC_Reconnect)
        gSITRILC_Reconnect = NULL;

    if(gSITRILC_Close)
    {
        if(gSITRILC_Handle)
        {
            gSITRILC_Close(gSITRILC_Handle);
            LogV("%s() gSITRILC_Close() ... OK.",__FUNCTION__);
            gSITRILC_Handle = NULL;
        }
        gSITRILC_Close = NULL;
    }

    if(gSITRILC_Open)
        gSITRILC_Open = NULL;

    if(gSITRilWLanUnsolCallback)
        gSITRilWLanUnsolCallback = NULL;

    if(gdlHandle)
    {
        dlclose(gdlHandle);
        gdlHandle = NULL;
    }

    LEAVE_FUNC();

    return ret;
}

int Open(void)
{
    int ret = SITRIL_WLAN_ERROR_NONE;
    const char * SITRilClientLibPath = SITRIL_CLIENT_LIB_PATH;

    ENTER_FUNC();

    do
    {
        // Loading SITRil Client Library
        if(gdlHandle != NULL)
        {
            LogW("%s() SITRilClient(%s) already is opened ...",__FUNCTION__,SITRilClientLibPath);
            break;
        }

        gdlHandle = dlopen(SITRilClientLibPath, RTLD_NOW);
        if(gdlHandle == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlopen()",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }

        gSITRILC_Open = NULL;
        gSITRILC_Close = NULL;
        gSITRILC_Reconnect = NULL;
        gSITRILC_RegisterUnsolicitedHandler = NULL;
        gSITRILC_Send = NULL;

        gSITRILC_Open = (void *(*)(void))dlsym(gdlHandle, "RILC_Open");
        if(gSITRILC_Open == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_Open",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("%s() gSITRILC_Open:0x%p ...",__FUNCTION__,gSITRILC_Open);

        gSITRILC_Close = (int (*)(void*))dlsym(gdlHandle, "RILC_Close");
        if(gSITRILC_Close == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_Close",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("%s() gSITRILC_Close:0x%p ...",__FUNCTION__,gSITRILC_Close);

        gSITRILC_Reconnect = (int (*)(void*))dlsym(gdlHandle, "RILC_Reconnect");
        if(gSITRILC_Reconnect == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_Reconnect",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("%s() gSITRILC_Reconnect:0x%p ...",__FUNCTION__,gSITRILC_Reconnect);

        gSITRILC_RegisterUnsolicitedHandler = (int (*)(void*, Rilc_OnUnsolicitedResponse))dlsym(gdlHandle, "RILC_RegisterUnsolicitedHandler");
        if(gSITRILC_RegisterUnsolicitedHandler == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_RegisterUnsolicitedHandler",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("%s() gSITRILC_RegisterUnsolicitedHandler:0x%p ...",__FUNCTION__,gSITRILC_RegisterUnsolicitedHandler);

        gSITRILC_Send = (int (*)(void*, unsigned, void*, size_t, Rilc_OnResponse, unsigned int))dlsym(gdlHandle, "RILC_Send");
        if(gSITRILC_Send == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_Send",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogV("%s() gSITRILC_Send:0x%p ...",__FUNCTION__,gSITRILC_Send);

        // Open SITRil Client Library
        gSITRILC_Handle = gSITRILC_Open();
        if(gSITRILC_Handle == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in gSITRILC_Open()",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_DEVICE;
            break;
        }

        LogV("%s() gSITRILC_Open() ... OK.",__FUNCTION__);


        // Reigister Callback
        ret = gSITRILC_RegisterUnsolicitedHandler(gSITRILC_Handle, NULL);
        if(ret)
        {
            LogE("%s() !! ERROR !!, Fail in gSITRILC_Open()",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_REGISTERATION_FAIL;
            break;
        }

        LogV("%s() gSITRILC_Open() ... OK.",__FUNCTION__);


    }while(0);

    if(ret != SITRIL_WLAN_ERROR_NONE)
        Close();

    LEAVE_FUNC();

    return ret;
}

int RegisterCallback(SITRilWLanUnsolCallback callback)
{
    int ret = SITRIL_WLAN_ERROR_NONE;

    ENTER_FUNC();

    do
    {
        if(gSITRilWLanUnsolCallback != NULL)
        {
            LogE("%s() !! ERROR !!, gSITRilWLanUnsolCallback(%p)",__FUNCTION__,gSITRilWLanUnsolCallback);
            ret = SITRIL_WLAN_ERROR_ALREADY_REGISTERD;
            break;
        }

        gSITRilWLanUnsolCallback = callback;
        LogV("%s() new gSITRilWLanUnsolCallback(%p)", __FUNCTION__, gSITRilWLanUnsolCallback);

    } while(0);

    LEAVE_FUNC();

    return ret;
}

int EAP_Get_IMSI(char *imsi, int socket_id)
{
    int ret = SITRIL_WLAN_ERROR_NONE;
    int i = 0;

    ENTER_FUNC();

    if (imsi == NULL)
    {
        LogE("%s() !! ERROR !!, imsi == NULL", __FUNCTION__);
        LEAVE_FUNC();
        return SITRIL_WLAN_ERROR_INVALID_PARAM;
    }

    do
    {
        if (gSITRILC_Handle == NULL)
        {
            LogE("%s() !! ERROR !!, gSITRILC_Handle == NULL", __FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }

        // Sending A message
        LogV("EAP_Get_IMSI - socket id : %d", socket_id);
        ret = gSITRILC_Send(gSITRILC_Handle, RILC_REQ_WLAN_GET_IMSI, &socket_id, sizeof(int), SITRilClient_OnResponse, socket_id);
        if (ret)
        {
            LogE("%s() !! ERROR !!, Fail in gSITRILC_Send()", __FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_SEND_FAIL;
            break;
        }

        while(i++ < MAX_LOOP_NUM) {
            usleep(TIME_INTERVAL);
            if (g_returnval[WLAN_GET_IMSI].bResponse == true) {
                if (g_returnval[WLAN_GET_IMSI].bSuccess == true) {
                    memcpy(imsi, g_returnval[WLAN_GET_IMSI].retvalue, strlen(g_returnval[WLAN_GET_IMSI].retvalue));
                    LogV("%s() IMSI : %s", __FUNCTION__, imsi);
                } else {
                    LogV("GET IMSI ERROR");
                }
                memset(&g_returnval[WLAN_GET_IMSI], 0, sizeof(rspData));
                break;
            }
        }

        if(MAX_LOOP_NUM <= i) {
            ret = SITRIL_WLAN_ERROR_TIMEOUT;
            LogE("%s() !! ERROR !!, TIMEOUT",__FUNCTION__);
        }

    } while (0);

    LEAVE_FUNC();
    return ret;
}

int EAP_3G_Authenticate(auth_request_type *request, auth_response_type *response, int socket_id)
{
    auth_request_type RequestAuth;
    int ret = SITRIL_WLAN_ERROR_NONE;
    int i = 0;

    ENTER_FUNC();

    do
    {
        if(gSITRILC_Handle == NULL)
        {
            LogE("%s() !! ERROR !!, gSITRILC_Handle == NULL",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }

        if( socket_id < 0 || 1 < socket_id ) {
            LogE("%s() !! ERROR !!, socket_id is wrong(%d)",__FUNCTION__, socket_id);
            ret = SITRIL_WLAN_ERROR_INVALID_PARAM;
            break;
        }

        RequestAuth.socket_id = socket_id;
        RequestAuth.auth_type = request->auth_type;
        RequestAuth.auth_len = request->auth_len;
        if (MAX_AUTH_LEN < request->auth_len)
            request->auth_len = MAX_AUTH_LEN;
        memcpy(RequestAuth.auth, request->auth, request->auth_len);

        LogI("%s : request - socket id : %d, auth_type : %d",__FUNCTION__,  RequestAuth.socket_id, RequestAuth.auth_type);

        // Sending A message
        ret =  gSITRILC_Send(gSITRILC_Handle, RILC_REQ_WLAN_SIM_AUTHENTICATE, &RequestAuth, sizeof(auth_request_type), SITRilClient_OnResponse, socket_id);
        if(ret)
        {
            LogE("%s() !! ERROR !!, Fail in gSITRILC_Send()",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_SEND_FAIL;
            break;
        }
        LogI("call EAP_3G_Authenticate Done");

        while(i++ < MAX_LOOP_NUM) {
            usleep(TIME_INTERVAL);
            if (g_returnval[WLAN_SIM_AUTH].bResponse == true) {
                if (g_returnval[WLAN_SIM_AUTH].bSuccess== true) {
                    memcpy(response, g_returnval[WLAN_SIM_AUTH].retvalue, sizeof(auth_response_type));
                    memset(&g_returnval[WLAN_SIM_AUTH], 0, sizeof(rspData));
                    LogI("WLAN_SIM_AUTH SUCESS");
                } else {
                    LogI("WLAN_SIM_AUTH ERROR");
                }
                break;
            }
        }

        if(MAX_LOOP_NUM <= i) {
            ret = SITRIL_WLAN_ERROR_TIMEOUT;
            LogE("%s() !! ERROR !!, TIMEOUT",__FUNCTION__);
        }

    } while(0);

    LEAVE_FUNC();
    return ret;
}

int EAP_GSM_Authenticate(auth_request_type *request, auth_response_type *response, int socket_id)
{
    auth_request_type RequestAuth;
    int ret = SITRIL_WLAN_ERROR_NONE;
    int i = 0;

    ENTER_FUNC();

    do
    {
        if(gSITRILC_Handle == NULL)
        {
            LogE("%s() !! ERROR !!, gSITRILC_Handle == NULL",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }

        if( socket_id < 0 || 1 < socket_id ) {
            LogE("%s() !! ERROR !!, socket_id is wrong(%d)",__FUNCTION__, socket_id);
            ret = SITRIL_WLAN_ERROR_INVALID_PARAM;
            break;
        }

        RequestAuth.socket_id = socket_id;
        RequestAuth.auth_type = request->auth_type;
        RequestAuth.auth_len = request->auth_len;
        if (MAX_AUTH_LEN < request->auth_len)
            request->auth_len = MAX_AUTH_LEN;
        memcpy(RequestAuth.auth, request->auth, request->auth_len);

        LogV("%s : request - socket id : %d, auth_type : %d",__FUNCTION__,  RequestAuth.socket_id, RequestAuth.auth_type);

        // Sending A message
        ret =  gSITRILC_Send(gSITRILC_Handle, RILC_REQ_WLAN_SIM_AUTHENTICATE, &RequestAuth, sizeof(auth_request_type), SITRilClient_OnResponse, socket_id);
        if(ret)
        {
            LogE("%s() !! ERROR !!, Fail in gSITRILC_Send()",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_NO_LIB_SEND_FAIL;
            break;
        }

        while(i++ < MAX_LOOP_NUM) {
            usleep(TIME_INTERVAL);
            if (g_returnval[WLAN_SIM_AUTH].bResponse == true) {
                if (g_returnval[WLAN_SIM_AUTH].bSuccess== true) {
                    memcpy(response, g_returnval[WLAN_SIM_AUTH].retvalue, sizeof(auth_response_type));
                    memset(&g_returnval[WLAN_SIM_AUTH], 0, sizeof(rspData));
                    LogI("WLAN_SIM_AUTH SUCESS");
                } else {
                    LogI("WLAN_SIM_AUTH ERROR");
                }
                break;
            }
        }

        if(MAX_LOOP_NUM <= i) {
            LogE("%s() !! ERROR !!, TIMEOUT",__FUNCTION__);
            ret = SITRIL_WLAN_ERROR_TIMEOUT;
        }

    } while(0);

    LEAVE_FUNC();

    return ret;
}

int HexChar2Value(char ch)
{
    if (ch >= '0' && ch <= '9')
        return (int)(ch - '0');
    else if (ch >= 'a' && ch <= 'f')
        return (int)(ch - 'a') + 10;
    else if (ch >= 'A' && ch <= 'F')
        return (int)(ch - 'A') + 10;
    return -1;
}

int HexString2Value(unsigned char *pHexDecOut, const char *pszHexStrIn)
{
    int nResult = 0;
    if (pHexDecOut == NULL || pszHexStrIn == NULL)
        return nResult;

    int nLength = strlen(pszHexStrIn);
    if(pHexDecOut && pszHexStrIn && nLength%2==0)
    {
        int i;
        for (i=0; i<nLength && pszHexStrIn[i*2]!='\0'; i++)
        {
            int nVal = HexChar2Value(pszHexStrIn[i*2]) << 4;
            nVal |= HexChar2Value(pszHexStrIn[(i*2)+1]);
            pHexDecOut[i] = (unsigned char) (nVal & 0xFF);
        }

        nResult = i;
    }

    return nResult;
}
