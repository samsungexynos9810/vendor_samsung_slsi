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
    SITRil-Ims library (implementation)
*/

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#define LOG_TAG "LibSITRil-If"
#include <utils/Log.h>
#include <telephony/ril.h>

#include "sitril-if.h"
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
void *gdlHandle[MAX_RILC_IF_CLIENT];

void *gSITRILC_Handle[MAX_RILC_IF_CLIENT];
void *(*gSITRILC_Open[MAX_RILC_IF_CLIENT])(void);
int (*gSITRILC_Close[MAX_RILC_IF_CLIENT])(void* client);
int (*gSITRILC_Reconnect[MAX_RILC_IF_CLIENT])(void* client);
int (*gSITRILC_RegisterUnsolicitedHandler[MAX_RILC_IF_CLIENT])(void* client, Rilc_OnUnsolicitedResponse handler);
int (*gSITRILC_Send[MAX_RILC_IF_CLIENT])(void* client, unsigned msgId, void* data, size_t length, Rilc_OnResponse handler, unsigned int channel);
SITRilIfUnsolCallback gSITRilIfUnsolCallback[MAX_RILC_IF_CLIENT];


// IF default request function
static int RequestIF(int client_id, unsigned int msgId, unsigned char *data, int data_len, SITRilIfOnResponse callback)
{
    int ret = SITRIL_IF_ERROR_NONE;

    ENTER_FUNC();
    LogI("%s: msdId=%d data=0x0%p data_len=%d callback=0x%p", __FUNCTION__, msgId, data, data_len, callback);

    if(gSITRILC_Handle[client_id] == NULL)
    {
        LogE("%s() !! ERROR !!, gSITRILC_Handle[%d] == NULL",__FUNCTION__, client_id);
        ret = SITRIL_IF_ERROR_NO_LIB_OPEN_FAIL;
        goto exit;
    }

    // Sending A message
    ret = gSITRILC_Send[client_id](gSITRILC_Handle[client_id], msgId, (void*)(data), data_len, callback, RIL_SOCKET_UNKNOWN);
    if(ret)
    {
        LogE("%s() !! ERROR !!, Fail in gSITRILC_Send[%d]() ",__FUNCTION__, client_id);
        ret = SITRIL_IF_ERROR_NO_LIB_SEND_FAIL;
        goto exit;
    }

exit:
    LEAVE_FUNC();
    return ret;
}

// #### Internal Functions ####
void SITRilClient_OnResponse(unsigned msgId, int status, void* data, size_t length, unsigned int channel)
{
    ENTER_FUNC();
    LogN("%s(msgId(%d), status(%d), data(0x%p), length(%zu) channel(%d)",__FUNCTION__, msgId, status,data, length, channel);

    switch(msgId)
    {
        case RILC_REQ_IF_EXECUTE_AM:
            LogN("%s() msgId(%d) is sent",__FUNCTION__,msgId);
            break;

        default:
            LogE("%s() !! ERROR !!, Unknown msgId = %d",__FUNCTION__,msgId);
            break;
    }

    LEAVE_FUNC();
}

// #### Initialization Functions ####
int IfClose(int client_id)
{
    int ret = SITRIL_IF_ERROR_NONE;

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
            LogN("%s() gSITRILC_Close[%d]() ... OK.",__FUNCTION__, client_id);
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

int IfOpen(int client_id)
{
    int ret = SITRIL_IF_ERROR_NONE;
    int i;
    const char * SITRilClientLibPath = SITRIL_CLIENT_LIB_PATH;

    ENTER_FUNC();

    do
    {
        // Loading SITRil Client Library
        if(gdlHandle[client_id] != NULL)
        {
            LogE("%s() SITRilClient(%s) already is opened ...",__FUNCTION__,SITRilClientLibPath);
            break;
        }

        gdlHandle[client_id] = dlopen(SITRilClientLibPath, RTLD_NOW);
        if(gdlHandle[client_id] == NULL)
        {
            LogE("%s() client_id : %d !! ERROR !!, Fail in dlopen()",__FUNCTION__, client_id);
            ret = SITRIL_IF_ERROR_NO_LIB_OPEN_FAIL;
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
            ret = SITRIL_IF_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogI("%s() IF gSITRILC_Open[%d]:0x%p ...",__FUNCTION__,client_id, gSITRILC_Open);

        gSITRILC_Close[client_id] = (int (*)(void*))dlsym(gdlHandle[client_id], "RILC_Close");
        if(gSITRILC_Close[client_id] == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_Close[%d]",__FUNCTION__, client_id);
            ret = SITRIL_IF_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogI("%s() gSITRILC_Close[%d]:0x%p ...",__FUNCTION__,client_id, gSITRILC_Close);

        gSITRILC_Reconnect[client_id] = (int (*)(void*))dlsym(gdlHandle[client_id], "RILC_Reconnect");
        if(gSITRILC_Reconnect[client_id] == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_Reconnect",__FUNCTION__);
            ret = SITRIL_IF_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogI("%s() gSITRILC_Reconnect[%d]:0x%p ...",__FUNCTION__,client_id, gSITRILC_Reconnect);

        gSITRILC_RegisterUnsolicitedHandler[client_id] = (int (*)(void*, Rilc_OnUnsolicitedResponse))dlsym(gdlHandle[client_id], "RILC_RegisterUnsolicitedHandler");
        if(gSITRILC_RegisterUnsolicitedHandler[client_id] == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_RegisterUnsolicitedHandler[%d]",__FUNCTION__, client_id);
            ret = SITRIL_IF_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogI("%s() gSITRILC_RegisterUnsolicitedHandler[%d]:0x%p ...",__FUNCTION__,client_id, gSITRILC_RegisterUnsolicitedHandler);

        gSITRILC_Send[client_id] = (int (*)(void*, unsigned, void*, size_t, Rilc_OnResponse, unsigned int))dlsym(gdlHandle[client_id], "RILC_Send");
        if(gSITRILC_Send[client_id] == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in dlsym() for gSITRILC_Send[%d]",__FUNCTION__, client_id);
            ret = SITRIL_IF_ERROR_NO_LIB_OPEN_FAIL;
            break;
        }
        LogI("%s() gSITRILC_Send[%d]:0x%p ...",__FUNCTION__,client_id, gSITRILC_Send);

        // Open SITRil Client Library
        gSITRILC_Handle[client_id] = gSITRILC_Open[client_id]();
        if(gSITRILC_Handle[client_id] == NULL)
        {
            LogE("%s() !! ERROR !!, Fail in gSITRILC_Open[%d]()",__FUNCTION__, client_id);
            ret = SITRIL_IF_ERROR_NO_DEVICE;
            break;
        }

        LogN("%s() gSITRILC_Open[%d]() ... OK.",__FUNCTION__, client_id);


        // Reigister Callback
        for(i=0; i<MAX_RILC_IF_CLIENT; i++)
        {
            if(gSITRilIfUnsolCallback[i] != NULL) break;
        }

        if(i == MAX_RILC_IF_CLIENT)
        {
            ret = gSITRILC_RegisterUnsolicitedHandler[client_id](gSITRILC_Handle[client_id], NULL);

            if(ret)
            {
                LogE("%s() !! ERROR !!, Fail in gSITRILC_RegisterUnsolicitedHandler[%d]()",__FUNCTION__, client_id);
                ret = SITRIL_IF_ERROR_REGISTERATION_FAIL;
                break;
            }
            LogN("%s() gSITRILC_RegisterUnsolicitedHandler[%d]() ... OK.",__FUNCTION__, client_id);
        }
        else
        {
            LogN("%s() Already registered gSITRILC_RegisterUnsolicitedHandler ... OK.",__FUNCTION__);
        }

    }while(0);

    if(ret != SITRIL_IF_ERROR_NONE)
    {
        IfClose(client_id);
    }

    LEAVE_FUNC();

    return ret;
}

int RegisterCallback(int client_id, SITRilIfUnsolCallback callback)
{
    int ret = SITRIL_IF_ERROR_NONE;

    ENTER_FUNC();

    do
    {
        if(gSITRilIfUnsolCallback[client_id] != NULL)
        {
            LogE("%s() !! ERROR !!, gSITRilIfUnsolCallback(%p)",__FUNCTION__,gSITRilIfUnsolCallback);
            ret = SITRIL_IF_ERROR_ALREADY_REGISTERD;
            break;
        }

        gSITRilIfUnsolCallback[client_id] = callback;
        LogN("%s() new gSITRilIfUnsolCallback[%d](%p)",__FUNCTION__,client_id, gSITRilIfUnsolCallback[client_id]);

    }while(0);

    LEAVE_FUNC();

    return ret;
}


int ExecuteAm(int client_id, unsigned char *data, int data_len, SITRilIfOnResponse callback)
{
    return RequestIF(client_id, RILC_REQ_IF_EXECUTE_AM, data, data_len, callback);
}

//IF support end ---------------------
