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

#define LOG_TAG "LibSITRil-eMBMS"
#include <utils/Log.h>

#include "RileMbms.h"
#include "sitril-client.h"

#define LogD(format, ...)    ALOGD("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogE(format, ...)    ALOGE("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogW(format, ...)    ALOGW("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogI(format, ...)    ALOGI("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogV(format, ...)    ALOGI("%s() " format, __FUNCTION__, ##__VA_ARGS__)

#define ENTER_FUNC()        { ALOGI("%s() [<-- ", __FUNCTION__); }
#define LEAVE_FUNC()        { ALOGI("%s() [--> ", __FUNCTION__); }

// #### Defintions ####
#define SITRIL_CLIENT_LIB_PATH "libsitril-client-system.so"

#define MAX_LOOP_NUM_FOR_CALLBACK   (300)   // 3 sec
#define MAX_LOOP_NUM_FOR_MODEM      (3500)  // 35 sec
#define TIME_INTERVAL (10*1000)  // 10ms
#define SOCKET_0            (0)
#define SOCKET_1            (1)
#define SIM_COUNT           (2)

// define request or response data
#pragma pack(1)
#pragma pack()

enum {
    EMBMS_IDX_CHECK_AVAIABLE_EMBMS = 0,
    EMBMS_IDX_ENABLE_SERVICE,
    EMBMS_IDX_DISABLE_SERVICE,
    EMBMS_IDX_SET_SESSION,
    EMBMS_IDX_GET_SESSION_LIST,
    EMBMS_IDX_GET_SIGNAL_STRENGTH,
    EMBMS_IDX_GET_NETWORK_TIME,
    EMBMS_IDX_MAX
};

EMBMS_CONTEXT *gEMBMSHandle = NULL;

typedef struct tagSecureElementResponse {
    int nMsgID;
    int bResponse;
    int bSuccess;
    int nRilErrNo;
    int nValueLength;
    void *pValue;

    void Initialize(void)
    {
        bResponse = false;
        bSuccess = false;
        nRilErrNo = 0;
        nValueLength = 0;
        if (pValue != NULL)
        {
            switch (nMsgID)
            {
                break;
                pValue = NULL;
            }
        }
    }
} EwLTECON_Response;

typedef struct tagSitRilClientInterface {
    void *pHandle;
    void *(*Open)(void);
    int(*Close)(void* client);
    int(*Reconnect)(void* client);
    int(*RegisterUnsolicitedHandler)(void* client, Rilc_OnUnsolicitedResponse handler);
    int(*Send)(void* client, unsigned msgId, void* data, size_t length, Rilc_OnResponse handler, unsigned int channel);

    void Initialize()
    {
        pHandle = NULL;
        Open = NULL;
        Close = NULL;
        Reconnect = NULL;
        RegisterUnsolicitedHandler = NULL;
        Send = NULL;
    }

    void Dump()
    {
        LogV("[SitRilClientInterface] pHandle(%p), Open(%p), Close(%p), Reconnect(%p), RegisterUnsolicitedHandler(%p), Send(%p)",
            pHandle, Open, Close, Reconnect, RegisterUnsolicitedHandler, Send);
    }
} SitRilClientInterface;

// #### Global Variables ####
static EwLTECON_Response g_aSeResponse[EMBMS_IDX_MAX] = {
    { RILC_REQ_EMBMS_CHECK_AVAIABLE_EMBMS, false, false, 0, 0, NULL },
    { RILC_REQ_EMBMS_ENABLE_SERVICE, false, false, 0, 0, NULL },
    { RILC_REQ_EMBMS_DISABLE_SERVICE, false, false, 0, 0, NULL },
    { RILC_REQ_EMBMS_SET_SESSION, false, false, 0, 0, NULL },
    { RILC_REQ_EMBMS_GET_SESSION_LIST, false, false, 0, 0, NULL },
    { RILC_REQ_EMBMS_GET_SIGNAL_STRENGTH, false, false, 0, 0, NULL },
    { RILC_REQ_EMBMS_GET_NETWORK_TIME, false, false, 0, 0, NULL }
};

void *g_hLibrary = NULL;
SitRilClientInterface g_SitRilClientIf = { NULL, NULL, NULL, NULL, NULL, NULL };

// #### Internal Functions ####
int GetMessageIndex(unsigned int nMsgId)
{
    switch (nMsgId)
    {
    case RILC_REQ_EMBMS_CHECK_AVAIABLE_EMBMS: return EMBMS_IDX_CHECK_AVAIABLE_EMBMS;
    case RILC_REQ_EMBMS_ENABLE_SERVICE: return EMBMS_IDX_ENABLE_SERVICE;
    case RILC_REQ_EMBMS_DISABLE_SERVICE: return EMBMS_IDX_DISABLE_SERVICE;
    case RILC_REQ_EMBMS_SET_SESSION: return EMBMS_IDX_SET_SESSION;
    case RILC_REQ_EMBMS_GET_SESSION_LIST: return EMBMS_IDX_GET_SESSION_LIST;
    case RILC_REQ_EMBMS_GET_SIGNAL_STRENGTH: return EMBMS_IDX_GET_SIGNAL_STRENGTH;
    case RILC_REQ_EMBMS_GET_NETWORK_TIME: return EMBMS_IDX_GET_NETWORK_TIME;
    }

    return -1;
}

static const char *GetMessageName(unsigned int nMsgId)
{
    switch (nMsgId)
    {
    case RILC_REQ_EMBMS_CHECK_AVAIABLE_EMBMS: return "RILC_REQ_EMBMS_CHECK_AVAIABLE_EMBMS";
    case RILC_REQ_EMBMS_ENABLE_SERVICE: return "RILC_REQ_EMBMS_ENABLE_SERVICE";
    case RILC_REQ_EMBMS_DISABLE_SERVICE: return "RILC_REQ_EMBMS_DISABLE_SERVICE";
    case RILC_REQ_EMBMS_SET_SESSION: return "RILC_REQ_EMBMS_SET_SESSION";
    case RILC_REQ_EMBMS_GET_SESSION_LIST: return "RILC_REQ_EMBMS_GET_SESSION_LIST";
    case RILC_REQ_EMBMS_GET_SIGNAL_STRENGTH: return "RILC_REQ_EMBMS_GET_SIGNAL_STRENGTH";
    case RILC_REQ_EMBMS_GET_NETWORK_TIME: return "RILC_REQ_EMBMS_GET_NETWORK_TIME";
    case RILC_UNSOL_EMBMS_MODEM_STATUS: return "RILC_UNSOL_EMBMS_MODEM_STATUS";
    case RILC_UNSOL_EMBMS_COVERAGE: return "RILC_UNSOL_EMBMS_COVERAGE";
    case RILC_UNSOL_EMBMS_GLOBAL_CELL_ID: return "RILC_UNSOL_EMBMS_GLOBAL_CELL_ID";
    case RILC_UNSOL_EMBMS_SAI_LIST: return "RILC_UNSOL_EMBMS_SAI_LIST";
    case RILC_UNSOL_EMBMS_SESSION_LIST: return "RILC_UNSOL_EMBMS_SESSION_LIST";
    case RILC_UNSOL_EMBMS_SESSION_CONTROL: return "RILC_UNSOL_EMBMS_SESSION_CONTROL";
    case RILC_UNSOL_EMBMS_SIGNAL_STRENGTH: return "RILC_UNSOL_EMBMS_SIGNAL_STRENGTH";
    case RILC_UNSOL_EMBMS_NETWORK_TIME: return "RILC_UNSOL_EMBMS_NETWORK_TIME";
    case RILC_UNSOL_EMBMS_RADIO_STATE_CHANGED: return "RILC_UNSOL_EMBMS_RADIO_STATE_CHANGED";
    }

    return "Unknown";
}

static void SitRilClient_UnsolicitedResponse(unsigned int msgId, void* data, size_t length, unsigned int channel)
{
    ENTER_FUNC();
    switch(msgId)
    {
    case RILC_UNSOL_EMBMS_MODEM_STATUS:
        eMbmsModemStatusNoti(data);
        break;
    case RILC_UNSOL_EMBMS_COVERAGE:
        eMbmsCoverageNoti(data);
        break;
    case RILC_UNSOL_EMBMS_GLOBAL_CELL_ID:
        eMbmsNetworkInformationNoti(data);
        break;
    case RILC_UNSOL_EMBMS_SAI_LIST:
        eMbmsServiceAreaListUpdatedNoti(data);
        break;
    case RILC_UNSOL_EMBMS_SESSION_LIST:
        eMbmsSessionListUpdatedNoti(data);
        break;
    case RILC_UNSOL_EMBMS_SESSION_CONTROL:
        eMbmsSessionControlNoti(data);
        break;
    case RILC_UNSOL_EMBMS_SIGNAL_STRENGTH:
        eMbmsSignalInformationNoti(data);
        break;
    case RILC_UNSOL_EMBMS_NETWORK_TIME:
        eMbmsTimeNoti(data);
        break;
    case RILC_UNSOL_EMBMS_RADIO_STATE_CHANGED:
        eMbmsRadioStateChanged(data);
        break;
    default:
        LogV("Unkonwn msgId = %d", msgId);
        return;
    }
    LEAVE_FUNC();
}

static void SitRilClient_OnResponse(unsigned int msgId, int status, void* data, size_t length, unsigned int channel)
{
    ENTER_FUNC();

    LogV("%s(%d), status(%d), data(0x%p), length(%zu)", GetMessageName(msgId), msgId, status, data, length);

    void *pResponse = NULL;
    int nRspLength = 0;
    int nIndex = GetMessageIndex(msgId);

    switch (msgId)
    {
    case RILC_REQ_EMBMS_CHECK_AVAIABLE_EMBMS:
    case RILC_REQ_EMBMS_GET_NETWORK_TIME:
    case RILC_REQ_EMBMS_ENABLE_SERVICE:
    case RILC_REQ_EMBMS_DISABLE_SERVICE:
    case RILC_REQ_EMBMS_SET_SESSION:
        if(status==RILC_STATUS_SUCCESS) g_aSeResponse[nIndex].bSuccess = true;
        break;
    default:
        LogE("Error:  Unkonwn msgId(%d)", msgId);
        LEAVE_FUNC();
        return;
    }

    if (pResponse != NULL)
    {
        memset(pResponse, 0, nRspLength);
        memcpy(pResponse, data, length);

        g_aSeResponse[nIndex].pValue = (void *)pResponse;
        memcpy(g_aSeResponse[nIndex].pValue, data, length);
        g_aSeResponse[nIndex].nValueLength = nRspLength;
        g_aSeResponse[nIndex].nRilErrNo = status;
    }

    g_aSeResponse[nIndex].bResponse = true;

    LEAVE_FUNC();
}

// #### Initialization Functions ####

int Open(void)
{
    ENTER_FUNC();

    int ret = SITRIL_EMBMS_ERROR_NONE;
    const char *pszSitRilClientLibPath = SITRIL_CLIENT_LIB_PATH;

    do {
        // Loading SIT RIL Client Library
        if (g_hLibrary != NULL)
        {
            LogD("SitRilClient(%s) already is opened ...", pszSitRilClientLibPath);
            ret = SITRIL_EMBMS_ERROR_ALREADY_LIB_LOADED;
            break;
        }

        g_hLibrary = dlopen(pszSitRilClientLibPath, RTLD_NOW);
        if (g_hLibrary == NULL)
        {
            LogE("Error:  Fail in dlopen()");
            ret = SITRIL_EMBMS_ERROR_LIB_LOAD_FAIL;
            break;
        }

        g_SitRilClientIf.Initialize();

        g_SitRilClientIf.Open = (void *(*)(void))dlsym(g_hLibrary, "RILC_Open");
        if (g_SitRilClientIf.Open == NULL)
        {
            ret = SITRIL_EMBMS_ERROR_NO_SYMBOL;
            LogE("Error:  Fail to load symbol 'RILC_Open' using dlsym()");
            break;
        }

        g_SitRilClientIf.Close = (int(*)(void*))dlsym(g_hLibrary, "RILC_Close");
        if (g_SitRilClientIf.Close == NULL)
        {
            ret = SITRIL_EMBMS_ERROR_NO_SYMBOL;
            LogE("Error:  Fail to load symbol 'RILC_Close' using dlsym()");
            break;
        }

        g_SitRilClientIf.Reconnect = (int(*)(void*))dlsym(g_hLibrary, "RILC_Reconnect");
        if (g_SitRilClientIf.Reconnect == NULL)
        {
            ret = SITRIL_EMBMS_ERROR_NO_SYMBOL;
            LogE("Error:  Fail to load symbol 'RILC_Reconnect' using dlsym()");
            break;
        }

        g_SitRilClientIf.RegisterUnsolicitedHandler = (int(*)(void*, Rilc_OnUnsolicitedResponse))dlsym(g_hLibrary, "RILC_RegisterUnsolicitedHandler");
        if (g_SitRilClientIf.RegisterUnsolicitedHandler == NULL)
        {
            ret = SITRIL_EMBMS_ERROR_NO_SYMBOL;
            LogE("Error:  Fail to load symbol 'RILC_RegisterUnsolicitedHandler' using dlsym()");
            break;
        }

        g_SitRilClientIf.Send = (int(*)(void*, unsigned, void*, size_t, Rilc_OnResponse, unsigned int))dlsym(g_hLibrary, "RILC_Send");
        if (g_SitRilClientIf.Send == NULL)
        {
            ret = SITRIL_EMBMS_ERROR_NO_SYMBOL;
            LogE("Error:  Fail to load symbol 'RILC_Send' using dlsym()");
            break;
        }

        // Open SITRil Client Library
        g_SitRilClientIf.pHandle = g_SitRilClientIf.Open();
        if (g_SitRilClientIf.pHandle == NULL)
        {
            LogE("Error:  Fail in g_SitRilClientIf.Open()");
            ret = SITRIL_EMBMS_ERROR_OPEN_FAIL;
            break;
        }
        LogV("g_SitRilClientIf.Open ... OK.");

        // Reigister Callback
        ret = g_SitRilClientIf.RegisterUnsolicitedHandler(g_SitRilClientIf.pHandle, SitRilClient_UnsolicitedResponse);
        if (ret)
        {
            LogE("Error:  Fail in g_SitRilClientIf.RegisterUnsolicitedHandler()");
            ret = SITRIL_EMBMS_ERROR_REGISTRATION_FAIL;
            break;
        }
        LogV("g_SitRilClientIf.RegisterUnsolicitedHandler() ... OK.");
    } while (0);

    if (ret != SITRIL_EMBMS_ERROR_NONE)
    {
        Close();
        g_SitRilClientIf.Initialize();
    }

    LEAVE_FUNC();
    return ret;
}

void Close(void)
{
    ENTER_FUNC();

    if (g_hLibrary)
    {
        if (g_SitRilClientIf.pHandle)
        {
            g_SitRilClientIf.RegisterUnsolicitedHandler(g_SitRilClientIf.pHandle, NULL);
            g_SitRilClientIf.Close(g_SitRilClientIf.pHandle);
            LogV("g_SitRilClientIf.Close() ... OK.");
            g_SitRilClientIf.Initialize();
        }

        dlclose(g_hLibrary);
        g_hLibrary = NULL;
    }

    LEAVE_FUNC();
}

int SendRequest(int socketId, unsigned int msgId, void *data, int data_len, bool synchronized)
{
    ENTER_FUNC();
    int ret = SITRIL_EMBMS_ERROR_NONE;

    do
    {
        // Check loaded library
        if (g_hLibrary == NULL)
        {
            LogE("Error: g_hLibrary = NULL");
            ret = SITRIL_EMBMS_ERROR_NOT_OPENED_LIB;
            break;
        }

        // Check opened library
        if (g_SitRilClientIf.pHandle == NULL)
        {
            LogE("Error: g_SitRilClientIf.pHandle = NULL");
            ret = SITRIL_EMBMS_ERROR_NOT_OPENED_LIB;
            break;
        }

        // Sending A message
        // LogV("Socket id : %d", socketId);
        ret = g_SitRilClientIf.Send(g_SitRilClientIf.pHandle, msgId, data, data_len, SitRilClient_OnResponse, socketId);
        if (ret)
        {
            LogE("Error:  Fail in g_SitRilClientIf.Send()");
            ret = SITRIL_EMBMS_ERROR_SEND_FAIL;
            break;
        }

        // Monitoring Receiving
        int nRspIdx = GetMessageIndex(msgId);
        int i = 0;

        if (synchronized) {
            while (i++ < MAX_LOOP_NUM_FOR_MODEM) {
                usleep(TIME_INTERVAL);
                if (g_aSeResponse[nRspIdx].bResponse == true) {
                    if (g_aSeResponse[nRspIdx].bSuccess != true) {
                        ret = SITRIL_EMBMS_ERROR_FAILURE;
                    }
                    break;
                }
            }

            if (MAX_LOOP_NUM_FOR_MODEM <= i) {
                ret = SITRIL_EMBMS_ERROR_TIMEOUT;
                LogE("Error:  TIMEOUT");
            }
        }
    } while (0);

    int nRspIdx = GetMessageIndex(msgId);
    g_aSeResponse[nRspIdx].Initialize();
    LEAVE_FUNC();
    return ret;
}

EwLTECON_Status_t EwLTECON_Initialize(EwLTECON_Handle_t *pEMBMSHandle, const EwLTECON_Configuration_t *pConfig)
{
    int ret = SITRIL_EMBMS_ERROR_FAILURE;
    ENTER_FUNC();

    if (gEMBMSHandle != NULL)
    {
        LogI("Handle is initialized already.");
        *pEMBMSHandle = gEMBMSHandle;
    }
    else
    {
        gEMBMSHandle = new EMBMS_CONTEXT;
        if (gEMBMSHandle == NULL)
        {
            LogE("Fail to new handle");
            return EWLTECON_ERROR;
        }
        memset(gEMBMSHandle, 0x00, sizeof(gEMBMSHandle));
        *pEMBMSHandle = gEMBMSHandle;
    }

    ret = Open();
    if (ret != SITRIL_EMBMS_ERROR_NONE)
    {
        LogE("Fail in eMbmsOpen()");
        return EWLTECON_ERROR;
    }
    else
    {
        gEMBMSHandle->mClient = ret;
    }

    gEMBMSHandle->mModemStatus = EWLTECON_MS_EMBMS_DISABLE;
    gEMBMSHandle->mConfig = new EwLTECON_Configuration_t;
    memcpy(gEMBMSHandle->mConfig, pConfig, sizeof(EwLTECON_Configuration_t));
    gEMBMSHandle->mSocketId = SOCKET_0;

    EwLTECON_ModemStatus_t ModemStatus = EWLTECON_MS_EMBMS_DISABLE;

    int retSendRequest = SITRIL_EMBMS_ERROR_NONE;
    int socketId = SOCKET_0;
    for (socketId = SOCKET_0; socketId < SIM_COUNT; socketId++) {
        retSendRequest = SendRequest(socketId, RILC_REQ_EMBMS_CHECK_AVAIABLE_EMBMS, NULL, 0, true);
        if (SITRIL_EMBMS_ERROR_NONE != retSendRequest) {
            if (SITRIL_EMBMS_ERROR_FAILURE == retSendRequest) {
                LogI("sokcetid(%d) dose not support", socketId);
                continue;
            } else {
                LogE("ERROR : retSendRequest : %d", retSendRequest);
                ModemStatus = EWLTECON_MS_EMBMS_DISABLE;
                eMbmsModemStatusNoti((void*)&ModemStatus);
                LEAVE_FUNC();
                return EWLTECON_ERROR;
            }
        }
        break;
    }

    if (socketId == SIM_COUNT) {
        ModemStatus = EWLTECON_MS_EMBMS_DISABLE;
        eMbmsModemStatusNoti((void*)&ModemStatus);
        LEAVE_FUNC();
        return EWLTECON_ERROR;
    }

    gEMBMSHandle->mSocketId = socketId;
    ModemStatus = EWLTECON_MS_READY;
    eMbmsModemStatusNoti((void*)&ModemStatus);

    /* For debugging
    LogI("EwLTECON_Initialize() pModemStatusCB: 0x%X", gEMBMSHandle->mConfig->stCallbacks.pModemStatusCB);
    LogI("EwLTECON_Initialize() pCoverageCB: 0x%X", gEMBMSHandle->mConfig->stCallbacks.pCoverageCB);
    LogI("EwLTECON_Initialize() pNetworkInformationCB: 0x%X", gEMBMSHandle->mConfig->stCallbacks.pNetworkInformationCB);
    LogI("EwLTECON_Initialize() pServiceAreaListUpdatedCB: 0x%X", gEMBMSHandle->mConfig->stCallbacks.pServiceAreaListUpdatedCB);
    LogI("EwLTECON_Initialize() pSessionListUpdatedCB: 0x%X", gEMBMSHandle->mConfig->stCallbacks.pSessionListUpdatedCB);
    LogI("EwLTECON_Initialize() pSessionControlCB: 0x%X", gEMBMSHandle->mConfig->stCallbacks.pSessionControlCB);
    LogI("EwLTECON_Initialize() pSignalInformationCB: 0x%X", gEMBMSHandle->mConfig->stCallbacks.pSignalInformationCB);
    LogI("EwLTECON_Initialize() pTimeCB: 0x%X", gEMBMSHandle->mConfig->stCallbacks.pTimeCB);

    LogI("EwLTECON_Initialize() szRemoteModemNetworkInterface: %s", gEMBMSHandle->mConfig->szRemoteModemNetworkInterface);
    LogI("EwLTECON_Initialize() uRemoteModemNetworkInterfacePort: %d", gEMBMSHandle->mConfig->uRemoteModemNetworkInterfacePort);
    */

    LEAVE_FUNC();
    return EWLTECON_SUCCESS;
}

EwLTECON_Status_t EwLTECON_Destroy(EwLTECON_Handle_t hHandle)
{
    ENTER_FUNC();

    EMBMS_CONTEXT *pHandle = (EMBMS_CONTEXT *)hHandle;
    if (pHandle == NULL) {
        LogI(" Handle is not initialized.");
        return EWLTECON_ERROR;
    }

    Close();

    if (gEMBMSHandle->mConfig != NULL) {
        delete gEMBMSHandle->mConfig;
        gEMBMSHandle->mConfig = NULL;
    }

    if (gEMBMSHandle != NULL) {
        delete gEMBMSHandle;
        gEMBMSHandle = NULL;
    }

    LEAVE_FUNC();
    return EWLTECON_SUCCESS;
}
EwLTECON_Status_t EwLTECON_EnableService(EwLTECON_Handle_t hHandle)
{
    ENTER_FUNC();
    int ret = SITRIL_EMBMS_ERROR_NONE;
    EMBMS_CONTEXT *pHandle = (EMBMS_CONTEXT *)hHandle;
    if (pHandle == NULL) {
        LogI("Handle is not initialized.");
        LEAVE_FUNC();
        return EWLTECON_ERROR;
    }

    int *data = new int(ENABLE_SERVICE);
    if (data == NULL) {
        LogE("!! ERROR !!, Fail to allocate");
        goto exit_fail;
    }

    ret = SendRequest(gEMBMSHandle->mSocketId, RILC_REQ_EMBMS_ENABLE_SERVICE, (void*)(data), sizeof(data), true);
    if (SITRIL_EMBMS_ERROR_NONE != ret) {
        LogE("!! ERROR !!, Fail in SendRequest() : %d", ret);
        goto exit_fail;
    }

    delete data;
    LEAVE_FUNC();
    return EWLTECON_SUCCESS;

exit_fail:

    EwLTECON_ModemStatus_t modemStatus = EWLTECON_MS_EMBMS_DISABLE;
    eMbmsModemStatusNoti((void *)&modemStatus);

    if (data != NULL) { delete data; }
    LEAVE_FUNC();
    return EWLTECON_ERROR;
}

EwLTECON_Status_t EwLTECON_DisableService(EwLTECON_Handle_t hHandle)
{
    ENTER_FUNC();

    EMBMS_CONTEXT *pHandle = (EMBMS_CONTEXT *)hHandle;
    if (pHandle == NULL) {
        LogI(" Handle is not initialized.");
        LEAVE_FUNC();
        return EWLTECON_ERROR;
    }

    int *data = new int(DISABLE_SERVICE);
    if (data == NULL) {
        LogE("!! ERROR !!, Fail to allocate");
        LEAVE_FUNC();
        return EWLTECON_ERROR;
    }

    if (SITRIL_EMBMS_ERROR_NONE != SendRequest(gEMBMSHandle->mSocketId, RILC_REQ_EMBMS_DISABLE_SERVICE, (void*)(data), sizeof(data), true)) {
        LogE(" !! ERROR !!, Fail in SendRequest()");
        delete data;
        LEAVE_FUNC();
        return EWLTECON_ERROR;
    }

    delete data;
    LEAVE_FUNC();
    return EWLTECON_SUCCESS;

}

EwLTECON_Status_t EwLTECON_ActivateSession(EwLTECON_Handle_t hHandle, uint8_t uPriority, EwLTECON_AvailablityInfo_t *pInfo)
{
    ENTER_FUNC();

    EMBMS_CONTEXT *pHandle = (EMBMS_CONTEXT *)hHandle;
    if (pHandle == NULL) {
        LogI("Handle is not initialized.");
        return EWLTECON_ERROR;
    }

    RIL_EMBMS_AvailablityInfo_t availablityInfo;
    memset(&availablityInfo, 0x00, sizeof(RIL_EMBMS_AvailablityInfo_t));

    availablityInfo.uSetActive = 1;     // Activate session
    availablityInfo.uPriority = uPriority;
    availablityInfo.uTMGI = pInfo->uTMGI;
    availablityInfo.uInfoBindCount = 1;
    if (0 < availablityInfo.uInfoBindCount) {
        availablityInfo.pInfoBind.uSAICount = pInfo->uSAICount;
        availablityInfo.pInfoBind.uFreqCount = pInfo->uFreqCount;
        LogI("uSAICount : %d, uFreqCount : %d", availablityInfo.pInfoBind.uSAICount, availablityInfo.pInfoBind.uFreqCount);
        for (int i = 0; i < (int)availablityInfo.pInfoBind.uSAICount; i++) {
            availablityInfo.pInfoBind.pSAIList[i] = pInfo->pSAIList[i];
        }
        for (int i = 0; i < (int)availablityInfo.pInfoBind.uFreqCount; i++) {
            availablityInfo.pInfoBind.pFreqList[i] = pInfo->pFreqList[i];
        }
    }

    if (SITRIL_EMBMS_ERROR_NONE != SendRequest(gEMBMSHandle->mSocketId, RILC_REQ_EMBMS_SET_SESSION, (void*)(&availablityInfo), sizeof(RIL_EMBMS_AvailablityInfo_t), true)) {
        LogE("!! ERROR !!, Fail in SendRequest()");
        return EWLTECON_ERROR;
    }

    LEAVE_FUNC();
    return EWLTECON_SUCCESS;
}

EwLTECON_Status_t EwLTECON_DeactivateSession(EwLTECON_Handle_t hHandle, EwLTECON_AvailablityInfo_t *pInfo)
{
    ENTER_FUNC();

    EMBMS_CONTEXT *pHandle = (EMBMS_CONTEXT *)hHandle;
    if (pHandle == NULL) {
        LogI("Handle is not initialized.");
        return EWLTECON_ERROR;
    }

    RIL_EMBMS_AvailablityInfo_t availablityInfo;
    memset(&availablityInfo, 0x00, sizeof(RIL_EMBMS_AvailablityInfo_t));

    availablityInfo.uSetActive = 0;     // Deactive session
    availablityInfo.uTMGI = pInfo->uTMGI;
    availablityInfo.uInfoBindCount = 1;
    if (0 < availablityInfo.uInfoBindCount) {
        availablityInfo.pInfoBind.uSAICount = pInfo->uSAICount;
        availablityInfo.pInfoBind.uFreqCount = pInfo->uFreqCount;
        LogI("uSAICount : %d, uFreqCount : %d", availablityInfo.pInfoBind.uSAICount, availablityInfo.pInfoBind.uFreqCount);
        for (int i = 0; i < (int)availablityInfo.pInfoBind.uSAICount; i++) {
            availablityInfo.pInfoBind.pSAIList[i] = pInfo->pSAIList[i];
        }
        for (int i = 0; i < (int)availablityInfo.pInfoBind.uFreqCount; i++) {
            availablityInfo.pInfoBind.pFreqList[i] = pInfo->pFreqList[i];
        }
    }

    if (SITRIL_EMBMS_ERROR_NONE != SendRequest(gEMBMSHandle->mSocketId, RILC_REQ_EMBMS_SET_SESSION, (void*)(&availablityInfo), sizeof(RIL_EMBMS_AvailablityInfo_t), true)) {
        LogE("!! ERROR !!, Fail in SendRequest()");
        LEAVE_FUNC();
        return EWLTECON_ERROR;
    }

    LEAVE_FUNC();
    return EWLTECON_SUCCESS;
}

EwLTECON_Status_t EwLTECON_SwitchSession(EwLTECON_Handle_t hHandle, uint8_t uPriority,
    EwLTECON_AvailablityInfo_t *pActInfo, EwLTECON_AvailablityInfo_t *pDeactInfo)
{
    ENTER_FUNC();

    if (EWLTECON_SUCCESS != EwLTECON_DeactivateSession(hHandle, pDeactInfo)) {
        LogE("!! ERROR !!, Fail in EwLTECON_DeactivateSession()");
        LEAVE_FUNC();
        return EWLTECON_ERROR;
    }

    if (EWLTECON_SUCCESS != EwLTECON_ActivateSession(hHandle, uPriority, pActInfo)) {
        LogE("!! ERROR !!, Fail in EwLTECON_ActivateSession()");
        LEAVE_FUNC();
        return EWLTECON_ERROR;
    }

    LEAVE_FUNC();
    return EWLTECON_SUCCESS;
}

EwLTECON_Status_t EwLTECON_GetTime(EwLTECON_Handle_t hHandle)
{
    ENTER_FUNC();
    EMBMS_CONTEXT *pHandle = (EMBMS_CONTEXT *)hHandle;
    if (pHandle == NULL) {
        LogI(" Handle is not initialized.");
        return EWLTECON_ERROR;
    }

    int ret = SendRequest(gEMBMSHandle->mSocketId, RILC_REQ_EMBMS_GET_NETWORK_TIME, NULL, 0, false);
    if (ret != SITRIL_EMBMS_ERROR_NONE) {
        LogE(" !! ERROR !!, Fail in SendRequest");
        LEAVE_FUNC();
        return EWLTECON_ERROR;
    }

    LEAVE_FUNC();
    return EWLTECON_SUCCESS;
}

EwLTECON_Status_t EwLTECON_ConfigureHysteresisDelays(EwLTECON_Handle_t hHandle,
    EwLTECON_HysteresisDelays_t *pHysteresisDelays)
{
    LogI(" !! Not Supported");
    return EWLTECON_NOT_SUPPORTED;
}

EwLTECON_Status_t EwLTECON_ConfigureWatchdogDelay(EwLTECON_Handle_t hHandle, uint32_t uWatchdogDelayMs)
{
    LogI(" !! Not Supported");
    return EWLTECON_NOT_SUPPORTED;
}

int eMbmsModemStatusNoti(void *data)
{
    ENTER_FUNC();

    if (gEMBMSHandle == NULL) {
        LogE("Handle is not initialized.");
        return 0;
    }

    EwLTECON_ModemStatus_t modemStatus = *(EwLTECON_ModemStatus_t *)data;
    gEMBMSHandle->mModemStatus = modemStatus;

    char interfaceName[20] = {0,};
    snprintf(interfaceName, sizeof(interfaceName)-1, "embms%d", gEMBMSHandle->mSocketId);

    LogV("modemStatus : %s, interface name : %s, interface index : %d", SLSI_EMBMS_MODEM_STATUS(modemStatus), interfaceName, 1);

    if (gEMBMSHandle->mConfig->stCallbacks.pModemStatusCB != NULL) {
        gEMBMSHandle->mConfig->stCallbacks.pModemStatusCB(gEMBMSHandle->mConfig->pCallbacksUserData, modemStatus, interfaceName, 1);
    }

    LEAVE_FUNC();
    return 0;
}

int eMbmsCoverageNoti(void *data)
{
    ENTER_FUNC();

    if (gEMBMSHandle == NULL) {
        LogE("Handle is not initialized.");
        return 0;
    }

    int loopCount = 0;
    EwLTECON_CoverageState_t coverageState = *(EwLTECON_CoverageState_t*)data;
    LogV("Coverage : %s", SLSI_EMBMS_COVERAGE_STATE(coverageState));

    while(loopCount++ < MAX_LOOP_NUM_FOR_CALLBACK) {
        usleep(TIME_INTERVAL);
        if(gEMBMSHandle->mModemStatus == EWLTECON_MS_EMBMS_ENABLE) {
            if (gEMBMSHandle->mConfig->stCallbacks.pCoverageCB != NULL) {
                gEMBMSHandle->mConfig->stCallbacks.pCoverageCB(gEMBMSHandle->mConfig->pCallbacksUserData, coverageState);
                LogV("called callback function");
                LEAVE_FUNC();
                return 0;

            }
        }
    }

    LogE("TIME OUT!!! - ModemStatus : %s", SLSI_EMBMS_MODEM_STATUS(gEMBMSHandle->mModemStatus));
    LEAVE_FUNC();
    return 0;

}

int eMbmsNetworkInformationNoti(void *data)
{
    ENTER_FUNC();

    if (gEMBMSHandle == NULL) {
        LogI("Handle is not initialized.");
        return 0;
    }

    int loopcount = 0;
    RIL_EMBMS_NetworkInformation_t globalCellId;
    memcpy(&globalCellId, data, sizeof(RIL_EMBMS_NetworkInformation_t));

    LogV("mcc : %d, mnc : %d, cellid : %d", globalCellId.mcc, globalCellId.mnc, globalCellId.cellId);

    while(loopcount++ < MAX_LOOP_NUM_FOR_CALLBACK) {
        usleep(TIME_INTERVAL);
        if(gEMBMSHandle->mModemStatus == EWLTECON_MS_EMBMS_ENABLE) {
            usleep(TIME_INTERVAL);
            if (gEMBMSHandle->mConfig->stCallbacks.pNetworkInformationCB != NULL) {
                gEMBMSHandle->mConfig->stCallbacks.pNetworkInformationCB(gEMBMSHandle->mConfig->pCallbacksUserData, globalCellId.mcc, globalCellId.mnc, globalCellId.cellId);
                LogV("called callback function");
                goto success;
            }
        }
    }

    LogE("TIME OUT!!! - ModemStatus : %s", SLSI_EMBMS_MODEM_STATUS(gEMBMSHandle->mModemStatus));

success:
    LEAVE_FUNC();
    return 0;
}

int eMbmsServiceAreaListUpdatedNoti(void *data)
{
    ENTER_FUNC();
    if (gEMBMSHandle == NULL) {
        LogI("Handle is not initialized.");
        return 0;
    }

    int loopCount = 0;
    uint32_t uNbCampedSAI = 0;
    EwLTECON_SAI_t *pCampedSAI = NULL;
    uint32_t uNbGroup = 0;
    uint32_t *pNbSAIperGroup = NULL;
    uint32_t uNbAvailableSAI = 0;
    EwLTECON_SAI_t *pAvailableSAI = NULL;

    RIL_EMBMS_SaiList *pSaiList = (RIL_EMBMS_SaiList*)data;

    uNbCampedSAI = pSaiList->IntraSaiListNum;
    LogI("uNbCampedSAI : %d", uNbCampedSAI);
    if (0 < uNbCampedSAI) {
        pCampedSAI = new EwLTECON_SAI_t[uNbCampedSAI];
        if (pCampedSAI == NULL) {
            LogE("pCampedSAI is NULL");
            return 0;
        }
        for(int i = 0; i < (int)uNbCampedSAI; i++) {
            pCampedSAI[i] = (EwLTECON_SAI_t)pSaiList->IntraSaiList[i];
            LogI("pCampedSAI[%d] : %d", i, pCampedSAI[i]);
        }
    }

    uNbGroup = pSaiList->InterSaiListNum;
    LogI("uNbGroup : %d", uNbGroup);
    if (0 < uNbGroup) {
        pNbSAIperGroup = new uint32_t[uNbGroup];
        if (pNbSAIperGroup == NULL) {
            LogE("pNbSAIperGroup is NULL");
            delete pCampedSAI;
            return 0;
        }
        for(int i = 0; i < (int)uNbGroup; i++) {
            pNbSAIperGroup[i] = (uint32_t)pSaiList->InterSaiList[i].InterSaiNumber;
            uNbAvailableSAI += pNbSAIperGroup[i];
        }
    }

    LogI("uNbAvailableSAI : %d", uNbAvailableSAI);
    if (0 < uNbAvailableSAI) {
        pAvailableSAI = new EwLTECON_SAI_t[uNbAvailableSAI];
        if (pAvailableSAI == NULL) {
            LogE("pAvailableSAI is NULL");
            delete pCampedSAI;
            delete pNbSAIperGroup;
            return 0;
        }

        int y = 0; // index of pAvailableSAI
        for (int i = 0; i < (int)uNbGroup; i++) {    // index of gourp
            for(int j = 0; j < (int)pNbSAIperGroup[i]; j++) {    // index of pNbSAIperGroup
                pAvailableSAI[y] = (EwLTECON_SAI_t)pSaiList->InterSaiList[i].InterSaiInfo[j];
                LogI("pAvailableSAI[%d] : %d", y, pAvailableSAI[y]);
                y++;
            }
        }

        if (y != (int)uNbAvailableSAI) {
            LogE("FAIL: SAI number is incorrect!!!!");
            delete pCampedSAI;
            delete pNbSAIperGroup;
            delete pAvailableSAI;
            return 0;
        }
    }

    while (loopCount++ < MAX_LOOP_NUM_FOR_CALLBACK) {
        usleep(TIME_INTERVAL);
        if(gEMBMSHandle->mModemStatus == EWLTECON_MS_EMBMS_ENABLE) {
            if (gEMBMSHandle->mConfig->stCallbacks.pServiceAreaListUpdatedCB != NULL) {
                gEMBMSHandle->mConfig->stCallbacks.pServiceAreaListUpdatedCB(gEMBMSHandle->mConfig->pCallbacksUserData
                            , uNbCampedSAI, pCampedSAI, uNbGroup, pNbSAIperGroup, uNbAvailableSAI, pAvailableSAI);
                LogV("Called callback function");
                goto success;
            }
        }
    }

    LogE("TIME OUT!!! - ModemStatus : %s", SLSI_EMBMS_MODEM_STATUS(gEMBMSHandle->mModemStatus));
success:

    delete pCampedSAI;
    delete pNbSAIperGroup;
    delete pAvailableSAI;

    LEAVE_FUNC();
    return 0;
}

int eMbmsSessionListUpdatedNoti(void *data)
{
    ENTER_FUNC();

    if (gEMBMSHandle == NULL) {
        LogI("Handle is not initialized.");
        LEAVE_FUNC();
        return 0;
    }

    int loopCount = 0;
    RIL_EMBMS_SessionListInfo_t *pData = (RIL_EMBMS_SessionListInfo_t*)data;
    EwLTECON_TMGI_t *pTmgiList = NULL;

    if (0 < pData->count) {
        pTmgiList = new EwLTECON_TMGI_t[pData->count];

        for(int i = 0; i < pData->count; i++) {
            pTmgiList[i] = pData->tmgi[i];
            LogI("pList[%d].TMGI : %012llX, pList[%d].eTMGIStatus : %s", i, pTmgiList[i], i, SLSI_EMBMS_TMGI_STATUS(pData->state));
        }

        LogI("State: %d", pData->state);
    } else {
        LogV("TMGI COUNT is zero");
    }

    while(loopCount++ < MAX_LOOP_NUM_FOR_CALLBACK) {
        usleep(TIME_INTERVAL);
        if(gEMBMSHandle->mModemStatus == EWLTECON_MS_EMBMS_ENABLE) {
            if (gEMBMSHandle->mConfig->stCallbacks.pSessionListUpdatedCB != NULL) {
                gEMBMSHandle->mConfig->stCallbacks.pSessionListUpdatedCB(gEMBMSHandle->mConfig->pCallbacksUserData, (EwLTECON_TMGIStatus_t)pData->state, pData->count, pTmgiList);
                LogV("Called callback function");
                goto success;
            }
        }
    }

    LogE("TIME OUT!!! - ModemStatus : %s", SLSI_EMBMS_MODEM_STATUS(gEMBMSHandle->mModemStatus));

success:
    if (pTmgiList != NULL) delete []pTmgiList;
    LEAVE_FUNC();
    return 0;

}


int eMbmsSessionControlNoti(void *data)
{
    ENTER_FUNC();

    if (gEMBMSHandle == NULL) {
        LogI("Handle is not initialized.");
        return 0;
    }

    int loopcount = 0;
    RIL_EMBMS_SessionControl sessionControl;
    memcpy(&sessionControl, data, sizeof(RIL_EMBMS_SessionControl));

    LogV("type : %d, status : %d, tmgi : %012llX", sessionControl.type, sessionControl.status, sessionControl.tmgi);

    while(loopcount++ < MAX_LOOP_NUM_FOR_CALLBACK) {
        usleep(TIME_INTERVAL);
        if(gEMBMSHandle->mModemStatus == EWLTECON_MS_EMBMS_ENABLE) {
            usleep(TIME_INTERVAL);
            if (gEMBMSHandle->mConfig->stCallbacks.pSessionControlCB != NULL) {
                gEMBMSHandle->mConfig->stCallbacks.pSessionControlCB(gEMBMSHandle->mConfig->pCallbacksUserData,
                    sessionControl.type, sessionControl.status, sessionControl.tmgi);
                LogV("called callback function");
                goto success;
            }
        }
    }

    LogE("TIME OUT!!! - ModemStatus : %s", SLSI_EMBMS_MODEM_STATUS(gEMBMSHandle->mModemStatus));

success:
    LEAVE_FUNC();
    return 0;
}

int eMbmsSignalInformationNoti(void *data)
{
    ENTER_FUNC();

    if (gEMBMSHandle == NULL) {
        LogE("Handle is not initialized.");
        return 0;
    }

    int loopcount = 0;
    RIL_SignalInfoList *pData = (RIL_SignalInfoList*)data;
    EwLTECON_SignalInformation_t *pList = new EwLTECON_SignalInformation_t[pData->count];
    if (pList == NULL) {
        LogE("alloc failed");
        return 0;
    }

    for(int i = 0; i < (int)pData->count; i++) {
        pList[i].eSignalType = (EwLTECON_SignalInformationType_t)pData->info[i].type;
        pList[i].value.int64Val = (int32_t)pData->info[i].value;
    }

    while(loopcount++ < MAX_LOOP_NUM_FOR_CALLBACK) {
        usleep(TIME_INTERVAL);
        if (gEMBMSHandle->mConfig->stCallbacks.pSignalInformationCB != NULL) {
            gEMBMSHandle->mConfig->stCallbacks.pSignalInformationCB(gEMBMSHandle->mConfig->pCallbacksUserData, pData->count, pList);
            LogV("Called callback function");
            goto success;
        }
    }

    LogE("TIME OUT!!! - ModemStatus : %s", SLSI_EMBMS_MODEM_STATUS(gEMBMSHandle->mModemStatus));

success:
    delete []pList;
    LEAVE_FUNC();
    return 0;
}

int eMbmsTimeNoti(void *data)
{
    ENTER_FUNC();

    if (gEMBMSHandle == NULL) {
        LogE("Handle is not initialized.");
        return 0;
    }

    int loopcount = 0;
    uint64_t uTime = *(uint64_t *)data;
    LogV("Time %lld", uTime);

    while(loopcount++ < MAX_LOOP_NUM_FOR_CALLBACK) {
        usleep(TIME_INTERVAL);
        if (gEMBMSHandle->mConfig->stCallbacks.pTimeCB != NULL) {
            gEMBMSHandle->mConfig->stCallbacks.pTimeCB(gEMBMSHandle->mConfig->pCallbacksUserData, uTime);
            LogV("Called callback function");
            goto success;
        }
    }

    LogE("TIME OUT!!! - ModemStatus : %s", SLSI_EMBMS_MODEM_STATUS(gEMBMSHandle->mModemStatus));

success:
    LEAVE_FUNC();
    return 0;
}

int eMbmsRadioStateChanged(void *data)
{
    ENTER_FUNC();

    if (gEMBMSHandle == NULL) {
        LogE("Handle is NULL.");
        return 0;
    }

    int radioState = *(int *)data;
    EwLTECON_ModemStatus_t modemStatus = EWLTECON_MS_DEVICE_OFF;
    switch (radioState) {
        case RADIO_STATE_UNAVAILABLE:
        case RADIO_STATE_OFF:
            //Flight mode Activated
            if (gEMBMSHandle->mModemStatus != EWLTECON_MS_DEVICE_OFF) {
                eMbmsModemStatusNoti(&modemStatus);
            } else {
                LogI("Already device is off");
            }
            break;
        default:
            //Flight mode Deactivated
            modemStatus = EWLTECON_MS_READY;
            eMbmsModemStatusNoti(&modemStatus);
            // EwLTECON_EnableService(gEMBMSHandle);
            break;
    } // end switch ~
    LEAVE_FUNC();
    return 0;
}

