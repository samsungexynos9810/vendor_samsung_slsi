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
#include <utils/Log.h>

#include "SitRilSe.h"
#include "sitril-client.h"

#undef LOG_TAG
#define LOG_TAG "LibSitRil-se"

#define LogD(format, ...)    ALOGD("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogE(format, ...)    ALOGE("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogW(format, ...)    ALOGW("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogI(format, ...)    ALOGI("%s() " format, __FUNCTION__, ##__VA_ARGS__)
#define LogV(format, ...)    ALOGI("%s() " format, __FUNCTION__, ##__VA_ARGS__)

#define ENTER_FUNC()        { ALOGI("%s() [<-- ", __FUNCTION__); }
#define LEAVE_FUNC()        { ALOGI("%s() [--> ", __FUNCTION__); }

// #### Defintions ####
#define SITRIL_CLIENT_LIB_PATH "libsitril-client.so"
#define INVALID_SOCKET_ID   (-1)

// wait 30 secs
#define MAX_LOOP_NUM (3000)
#define TIME_INTERVAL (10*1000)  // 10ms

#define MAX_SIT_SIM_DATA_SIZE           512
#define MAX_SIT_SIM_AID_LEN             16
#define MAX_SIT_ATR_LENGTH              (256*2)

#pragma pack(1)
typedef struct tagSitSimOpenChannelRequest
{
    unsigned char aid_len;
    unsigned char aid[MAX_SIT_SIM_AID_LEN];
    unsigned char p2;
} SitSimOpenChannelReq;

typedef struct tagSitSimOpenChannelResponse
{
    int session_id;
    unsigned char sw1;
    unsigned char sw2;
    short response_len;
    unsigned char response[MAX_OPEN_CHANNEL_SIZE];
} SitSimOpenChannelRsp;

typedef struct tagSitSimTransmitApduChannelRequest
{
    int session_id;
    int cla;
    int instruction;
    int p1;
    int p2;
    int p3;
    unsigned short data_len;
    unsigned char data[0];
} SitSimTransmitApduChannelReq;

typedef struct tagSitSimTransmitApduChannelResponse
{
    unsigned char sw1;
    unsigned char sw2;
    unsigned short apdu_len;
    unsigned char apdu[0];
} SitSimTransmitApduChannelRsp;

typedef struct tagSitSimApdu {
    unsigned char cla;
    unsigned char instruction;
    unsigned char p1;
    unsigned char p2;
    unsigned char p3;
    unsigned char data[0];
} SitSimApdu;

typedef struct tagSitSimTransmitApduBasicRequest
{
    int session_id;
    unsigned short apdu_len;
    union {
        SitSimApdu apdu_detail;
        unsigned char apdu[0];
    };
} SitSimTransmitApduBasicReq;

typedef struct tagSitSimTransmitApduBasicResponse
{
    unsigned short apdu_len;
    unsigned char apdu[0];
} SitSimTransmitApduBasicRsp;

typedef struct tagSitAtrResponse
{
    uint8_t atr[MAX_SIT_ATR_LENGTH];
} SitAtrRsp;

#pragma pack()

enum {
    SE_IDX_OPEN_CHANNEL,
    SE_IDX_TRANSMIT_APDU_LOGICAL,
    SE_IDX_TRANSMIT_APDU_BASIC,
    SE_IDX_CLOSE_CHANNEL,
    SE_IDX_GET_ICC_ATR,
    SE_IDX_GET_CARD_PRESENT,
    SE_IDX_MAX
};

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
        if(pValue!=NULL)
        {
            switch(nMsgID)
            {
            case RILC_REQ_SE_OPEN_CHANNEL:
                {
                SitSimOpenChannelRsp *p = (SitSimOpenChannelRsp *) pValue;
                delete p;
                }
                break;
            case RILC_REQ_SE_TRANSMIT_APDU_LOGICAL:
                {
                unsigned char *p = (unsigned char *) pValue;
                delete [] p;
                }
                break;
            case RILC_REQ_SE_TRANSMIT_APDU_BASIC:
                {
                unsigned char *p = (unsigned char *) pValue;
                delete [] p;
                }
                break;
            case RILC_REQ_SE_GET_ICC_ATR:
                {
                SitAtrRsp *p = (SitAtrRsp *) pValue;
                delete p;
                }
                break;
            case RILC_REQ_SE_CLOSE_CHANNEL:
            case RILC_REQ_SE_GET_CARD_PRESENT:
                {
                int *p = (int *) pValue;
                delete p;
                }
                break;
            }

            pValue = NULL;
        }
    }
} SecureElementResponse;

typedef struct tagSitRilClientInterface {
    void *pHandle;
    void *(*Open)(void);
    int (*Close)(void* client);
    int (*Reconnect)(void* client);
    int (*RegisterUnsolicitedHandler)(void* client, Rilc_OnUnsolicitedResponse handler);
    int (*Send)(void* client, unsigned msgId, void* data, size_t length, Rilc_OnResponse handler, unsigned int channel);

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
static SecureElementResponse g_aSeResponse[SE_IDX_MAX] = {
    {RILC_REQ_SE_OPEN_CHANNEL, false, false, 0, 0, NULL},
    {RILC_REQ_SE_TRANSMIT_APDU_LOGICAL, false, false, 0, 0, NULL},
    {RILC_REQ_SE_TRANSMIT_APDU_BASIC, false, false, 0, 0, NULL},
    {RILC_REQ_SE_CLOSE_CHANNEL, false, false, 0, 0, NULL},
    {RILC_REQ_SE_GET_ICC_ATR, false, false, 0, 0, NULL},
    {RILC_REQ_SE_GET_CARD_PRESENT, false, false, 0, 0, NULL}
};

void *g_hLibrary = NULL;
SitRilClientInterface g_SitRilClientIf = { NULL, NULL, NULL, NULL, NULL, NULL };
int g_SocketId = INVALID_SOCKET_ID;

int HexString2Value(unsigned char *pHexDecOut, const char *pszHexStrIn);
int Value2HexString(char *pszHexStrOut, const unsigned char *pHexDecIn, int nLength);


// #### Internal Functions ####
int GetMessageIndex(unsigned int nMsgId)
{
    switch(nMsgId)
    {
    case RILC_REQ_SE_OPEN_CHANNEL: return SE_IDX_OPEN_CHANNEL;
    case RILC_REQ_SE_TRANSMIT_APDU_LOGICAL: return SE_IDX_TRANSMIT_APDU_LOGICAL;
    case RILC_REQ_SE_TRANSMIT_APDU_BASIC: return SE_IDX_TRANSMIT_APDU_BASIC;
    case RILC_REQ_SE_CLOSE_CHANNEL: return SE_IDX_CLOSE_CHANNEL;
    case RILC_REQ_SE_GET_ICC_ATR: return SE_IDX_GET_ICC_ATR;
    case RILC_REQ_SE_GET_CARD_PRESENT: return SE_IDX_GET_CARD_PRESENT;
    }

    return -1;
}

static const char *GetMessageName(unsigned int nMsgId)
{
    switch(nMsgId)
    {
    case RILC_REQ_SE_OPEN_CHANNEL: return "RILC_REQ_SE_OPEN_CHANNEL";
    case RILC_REQ_SE_TRANSMIT_APDU_LOGICAL: return "RILC_REQ_SE_TRANSMIT_APDU_LOGICAL";
    case RILC_REQ_SE_TRANSMIT_APDU_BASIC: return "RILC_REQ_SE_TRANSMIT_APDU_BASIC";
    case RILC_REQ_SE_CLOSE_CHANNEL: return "RILC_REQ_SE_CLOSE_CHANNEL";
    case RILC_REQ_SE_GET_ICC_ATR: return "RILC_REQ_SE_GET_ICC_ATR";
    case RILC_REQ_SE_GET_CARD_PRESENT: return "RILC_REQ_SE_GET_CARD_PRESENT";
    }

    return "Unknown";
}

#if 0
static void SitRilClient_UnsolicitedResponse(unsigned int msgId, void* data, size_t length, unsigned int channel)
{
    ENTER_FUNC();

    LEAVE_FUNC();
}
#endif

static void SitRilClient_OnResponse(unsigned int msgId, int status, void* data, size_t length, unsigned int channel)
{
    ENTER_FUNC();

    LogV("%s(%d), status(%d), data(%p), length(%zu)", GetMessageName(msgId), msgId, status, data, length);
    // Print Data Hex String
    char *pszHex = new char[length*2+1];
    memset(pszHex, 0, length*2+1);
    int nDataLen = Value2HexString(pszHex, (const unsigned char *) data, length);
    pszHex[nDataLen] = '\0';
    LogD("Data:%s", pszHex);
    delete [] pszHex;

    void *pResponse = NULL;
    int nRspLength = 0;
    int nIndex = GetMessageIndex(msgId);

    switch(msgId)
    {
    case RILC_REQ_SE_OPEN_CHANNEL:
        pResponse = (void *) new SitSimOpenChannelRsp();
        nRspLength = sizeof(SitSimOpenChannelRsp);
        if(pResponse!=NULL) g_aSeResponse[nIndex].bSuccess = true;
        break;
    case RILC_REQ_SE_TRANSMIT_APDU_LOGICAL:
        nRspLength = sizeof(SitSimTransmitApduChannelRsp) + length;
        pResponse = (void *) new unsigned char[nRspLength];
        if(pResponse!=NULL) g_aSeResponse[nIndex].bSuccess = true;
        break;
    case RILC_REQ_SE_TRANSMIT_APDU_BASIC:
        nRspLength = sizeof(SitSimTransmitApduBasicRsp) + length;
        pResponse = (void *) new unsigned char[nRspLength];
        if(pResponse!=NULL) g_aSeResponse[nIndex].bSuccess = true;
        break;
    case RILC_REQ_SE_CLOSE_CHANNEL:
        if(status==RILC_STATUS_SUCCESS) g_aSeResponse[nIndex].bSuccess = true;
        break;
    case RILC_REQ_SE_GET_ICC_ATR:
        pResponse = (void *) new SitAtrRsp();
        nRspLength = length;
        if(pResponse!=NULL && status==RILC_STATUS_SUCCESS) g_aSeResponse[nIndex].bSuccess = true;
        break;
    case RILC_REQ_SE_GET_CARD_PRESENT:
        pResponse = (void *) new int;
        nRspLength = sizeof(int);
        if(pResponse!=NULL && status==RILC_STATUS_SUCCESS) g_aSeResponse[nIndex].bSuccess = true;
        break;
    default:
        LogE("Error:  Unkonwn msgId(%d)", msgId);
        LEAVE_FUNC();
        return;
    }

    LogV("nRspLength(%d), length(%zu)", nRspLength, length);

    if(pResponse!=NULL)
    {
        memset(pResponse, 0, nRspLength);
        memcpy(pResponse, data, length);

        g_aSeResponse[nIndex].pValue = (void *) pResponse;
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

    int ret = SITRIL_SE_ERROR_NONE;
    const char *pszSitRilClientLibPath = SITRIL_CLIENT_LIB_PATH;

    do {
        // Loading SIT RIL Client Library
        if(g_hLibrary != NULL)
        {
            LogD("SitRilClient(%s) already is opened ...", pszSitRilClientLibPath);
            ret = SITRIL_SE_ERROR_ALREADY_LIB_LOADED;
            break;
        }

        g_hLibrary = dlopen(pszSitRilClientLibPath, RTLD_NOW);
        if(g_hLibrary == NULL)
        {
            LogE("Error:  Fail in dlopen()");
            ret = SITRIL_SE_ERROR_LIB_LOAD_FAIL;
            break;
        }

        g_SitRilClientIf.Initialize();

        g_SitRilClientIf.Open = (void *(*)(void))dlsym(g_hLibrary, "RILC_Open");
        if(g_SitRilClientIf.Open==NULL)
        {
            ret = SITRIL_SE_ERROR_NO_SYMBOL;
            LogE("Error:  Fail to load symbol 'RILC_Open' using dlsym()");
            break;
        }

        g_SitRilClientIf.Close = (int (*)(void*))dlsym(g_hLibrary, "RILC_Close");
        if(g_SitRilClientIf.Close==NULL)
        {
            ret = SITRIL_SE_ERROR_NO_SYMBOL;
            LogE("Error:  Fail to load symbol 'RILC_Close' using dlsym()");
            break;
        }

        g_SitRilClientIf.Reconnect = (int (*)(void*))dlsym(g_hLibrary, "RILC_Reconnect");
        if(g_SitRilClientIf.Reconnect==NULL)
        {
            ret = SITRIL_SE_ERROR_NO_SYMBOL;
            LogE("Error:  Fail to load symbol 'RILC_Reconnect' using dlsym()");
            break;
        }

        g_SitRilClientIf.RegisterUnsolicitedHandler = (int (*)(void*, Rilc_OnUnsolicitedResponse))dlsym(g_hLibrary, "RILC_RegisterUnsolicitedHandler");
        if(g_SitRilClientIf.RegisterUnsolicitedHandler==NULL)
        {
            ret = SITRIL_SE_ERROR_NO_SYMBOL;
            LogE("Error:  Fail to load symbol 'RILC_RegisterUnsolicitedHandler' using dlsym()");
            break;
        }

        g_SitRilClientIf.Send = (int (*)(void*, unsigned, void*, size_t, Rilc_OnResponse, unsigned int))dlsym(g_hLibrary, "RILC_Send");
        if(g_SitRilClientIf.Send==NULL)
        {
            ret = SITRIL_SE_ERROR_NO_SYMBOL;
            LogE("Error:  Fail to load symbol 'RILC_Send' using dlsym()");
            break;
        }

        // Open SITRil Client Library
        g_SitRilClientIf.pHandle = g_SitRilClientIf.Open();
        if(g_SitRilClientIf.pHandle == NULL)
        {
            LogE("Error:  Fail in g_SitRilClientIf.Open()");
            ret = SITRIL_SE_ERROR_OPEN_FAIL;
            break;
        }
        LogV("g_SitRilClientIf.Open ... OK.");

        // Reigister Callback
        ret = g_SitRilClientIf.RegisterUnsolicitedHandler(g_SitRilClientIf.pHandle, NULL);
        if(ret)
        {
            LogE("Error:  Fail in g_SitRilClientIf.RegisterUnsolicitedHandler()");
            ret = SITRIL_SE_ERROR_REGISTRATION_FAIL;
            break;
        }
        LogV("g_SitRilClientIf.RegisterUnsolicitedHandler() ... OK.");
    } while(0);

    if(ret!=SITRIL_SE_ERROR_NONE)
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

    if(g_hLibrary)
    {
        if(g_SitRilClientIf.pHandle)
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

int SetSocketId(int socketId)
{
    ENTER_FUNC();
    int ret = SITRIL_SE_ERROR_NONE;

    if(g_hLibrary == NULL)
    {
        ret = SITRIL_SE_ERROR_NOT_OPENED_LIB;
    }
    else
    {
        g_SocketId = socketId;
    }
    LEAVE_FUNC();

    return ret;
}

int SendRequest(int socketId, unsigned int msgId, void *data, int data_len)
{
    ENTER_FUNC();
    int ret = SITRIL_SE_ERROR_NONE;

    do
    {
        // Check loaded library
        if (g_hLibrary == NULL)
        {
            LogE("Error: g_hLibrary = NULL");
            ret = SITRIL_SE_ERROR_NOT_OPENED_LIB;
            break;
        }

        // Check opened library
        if (g_SitRilClientIf.pHandle == NULL)
        {
            LogE("Error: g_SitRilClientIf.pHandle = NULL");
            ret = SITRIL_SE_ERROR_NOT_OPENED_LIB;
            break;
        }

        // Sending A message
        LogV("socket id : %d", socketId);
        ret = g_SitRilClientIf.Send(g_SitRilClientIf.pHandle, msgId, data, data_len, SitRilClient_OnResponse, socketId);
        if (ret)
        {
            LogE("Error:  Fail in g_SitRilClientIf.Send()");
            ret = SITRIL_SE_ERROR_SEND_FAIL;
            break;
        }

        // Monitoring Receiving
        int nRspIdx = GetMessageIndex(msgId);
        int i = 0;
        while(i++ < MAX_LOOP_NUM) {
            usleep(TIME_INTERVAL);
            if (g_aSeResponse[nRspIdx].bResponse == true) {
                if(g_aSeResponse[nRspIdx].bSuccess != true) {
                    ret = SITRIL_SE_ERROR_SEND_FAIL;
                }
                break;
            }
        }

        if(MAX_LOOP_NUM <= i) {
            ret = SITRIL_SE_ERROR_TIMEOUT;
            LogE("%s() Error:  TIMEOUT",__FUNCTION__);
        }
    } while (0);

    LEAVE_FUNC();
    return ret;
}

int OpenLogicalChannel(SitRilSeOpenChannelResponse *pResponse, LengthData *pAid, int p2)
{
    ENTER_FUNC();
    int ret = SITRIL_SE_ERROR_NONE;

    // Check Socket ID
    if(g_SocketId==INVALID_SOCKET_ID)
    {
        LogE("Error:  Invalid Socket ID(%d)", g_SocketId);
        ret = SITRIL_SE_ERROR_INVALID_SOCKET_ID;
    }
    // Check parameters
    else if(pResponse==NULL || pAid==NULL)
    {
        LogE("Error:  pResponse(0x%p), pAid(0x%p)", pResponse, pAid);
        ret = SITRIL_SE_ERROR_INVALID_PARAM;
    }
    else
    {
        // Set Sending Data
        SitSimOpenChannelReq tReq;
        memset(&tReq, 0, sizeof(SitSimOpenChannelReq));
        tReq.aid_len = pAid->length;
        memcpy(&tReq.aid, pAid->data, pAid->length);
        tReq.p2 = p2;
        ret = SendRequest(g_SocketId, RILC_REQ_SE_OPEN_CHANNEL, &tReq, sizeof(SitSimOpenChannelReq));
        if(ret == SITRIL_SE_ERROR_NONE)
        {
            memset(pResponse, 0, sizeof(SitRilSeOpenChannelResponse));
            SecureElementResponse *pSeRsp = (SecureElementResponse *) &g_aSeResponse[SE_IDX_OPEN_CHANNEL];
            SitSimOpenChannelRsp *pValue = (SitSimOpenChannelRsp *) pSeRsp->pValue;
            if(pValue!=NULL)
            {
                //memcpy(pResponse, pValue, sizeof(SecureElementResponse));
                pResponse->rilErrNo = pSeRsp->nRilErrNo;
                pResponse->sessionId = pValue->session_id;
                pResponse->sw1 = (int) pValue->sw1;
                pResponse->sw2 = (int) pValue->sw2;
                pResponse->nResponseCount = (int) pValue->response_len;
                memcpy(pResponse->aResponse, pValue->response, pValue->response_len);
                pResponse->aResponse[pResponse->nResponseCount++] = (uint8_t)pResponse->sw1;
                pResponse->aResponse[pResponse->nResponseCount++] = (uint8_t)pResponse->sw2;
            }
            else ret = SITRIL_SE_ERROR_FAILURE;

            pSeRsp->Initialize();
        }
    }

    LEAVE_FUNC();
    return ret;
}

int TransmitApduLogicalChannel(SitRilSeTransmitApduChannelRsp *pResponse, int channel, int cla, int instruction, int p1, int p2, int p3, LengthData *pData)
{
    ENTER_FUNC();
    int ret = SITRIL_SE_ERROR_NONE;

    // Check Socket ID
    if(g_SocketId==INVALID_SOCKET_ID)
    {
        LogE("Error:  Invalid Socket ID(%d)", g_SocketId);
        ret = SITRIL_SE_ERROR_INVALID_SOCKET_ID;
    }
    // Check parameters
    else if(pResponse==NULL)
    {
        LogE("Error:  pResponse(0x%p)", pResponse);
        ret = SITRIL_SE_ERROR_INVALID_PARAM;
    }
    else
    {
        // Set Sending Data
        int nDataLen = (pData == NULL) ? 0 : pData->length;
        int nReqLength = sizeof(SitSimTransmitApduChannelReq) + nDataLen;
        unsigned char *pBuffer = new unsigned char[nReqLength];
        SitSimTransmitApduChannelReq *pReq = (SitSimTransmitApduChannelReq *) pBuffer;
        memset(pReq, 0, nReqLength);
        pReq->session_id = channel;
        pReq->cla = cla;
        pReq->instruction = instruction;
        pReq->p1 = p1;
        pReq->p2 = p2;
        pReq->p3 = p3;

        if(0 < nDataLen)
        {
            memcpy(pReq->data, pData->data, pData->length);
            pReq->data_len = pData->length;
        }
        ret = SendRequest(g_SocketId, RILC_REQ_SE_TRANSMIT_APDU_LOGICAL, pReq, nReqLength);
        if(ret == SITRIL_SE_ERROR_NONE)
        {
            memset(pResponse, 0, sizeof(SitRilSeTransmitApduChannelRsp));
            SecureElementResponse *pSeRsp = (SecureElementResponse *) &g_aSeResponse[SE_IDX_TRANSMIT_APDU_LOGICAL];
            SitSimTransmitApduChannelRsp *pValue = (SitSimTransmitApduChannelRsp *) pSeRsp->pValue;
            if(pValue!=NULL)
            {
                pResponse->rilErrNo = pSeRsp->nRilErrNo;
                pResponse->sw1 = (int) pValue->sw1;
                pResponse->sw2 = (int) pValue->sw2;
                pResponse->apdu_len = pValue->apdu_len;
                memcpy(pResponse->apdu, pValue->apdu, pValue->apdu_len);
                pResponse->apdu[pResponse->apdu_len++] = pValue->sw1;
                pResponse->apdu[pResponse->apdu_len++] = pValue->sw2;
            }
            else ret = SITRIL_SE_ERROR_FAILURE;

            delete [] pBuffer;
            pSeRsp->Initialize();
        }
    }

    LEAVE_FUNC();
    return ret;
}

int TransmitApduBasicChannel(SitRilSeTransmitApduBasicRsp *pResponse, int cla, int instruction, int p1, int p2, int p3, LengthData *pData)
{
    ENTER_FUNC();
    int ret = SITRIL_SE_ERROR_NONE;

    // Check Socket ID
    if(g_SocketId==INVALID_SOCKET_ID)
    {
        LogE("Error:  Invalid Socket ID(%d)", g_SocketId);
        ret = SITRIL_SE_ERROR_INVALID_SOCKET_ID;
    }
    // Check parameters
    else if(pResponse==NULL)
    {
        LogE("Error:  pResponse(0x%p)", pResponse);
        ret = SITRIL_SE_ERROR_INVALID_PARAM;
    }
    else
    {
        // Set Sending Data
        int nDataLen = (pData == NULL) ? 0 : pData->length;
        int nReqLength = sizeof(SitSimTransmitApduBasicReq) + nDataLen;
        unsigned char *pBuffer = new unsigned char[nReqLength];
        SitSimTransmitApduBasicReq *pReq = (SitSimTransmitApduBasicReq *) pBuffer;
        memset(pReq, 0, nReqLength);
        pReq->apdu_detail.cla = cla;
        pReq->apdu_detail.instruction = instruction;
        pReq->apdu_detail.p1 = p1;
        pReq->apdu_detail.p2 = p2;
        pReq->apdu_detail.p3 = p3;
        pReq->apdu_len += sizeof(SitSimApdu);

        if(0 < nDataLen)
        {
            memcpy(pReq->apdu_detail.data, pData->data, pData->length);
            pReq->apdu_len += pData->length;
        }

        ret = SendRequest(g_SocketId, RILC_REQ_SE_TRANSMIT_APDU_BASIC, pReq, nReqLength);
        if(ret == SITRIL_SE_ERROR_NONE)
        {
            memset(pResponse, 0, sizeof(SitRilSeTransmitApduBasicRsp));
            SecureElementResponse *pSeRsp = (SecureElementResponse *) &g_aSeResponse[SE_IDX_TRANSMIT_APDU_BASIC];
            SitSimTransmitApduBasicRsp *pValue = (SitSimTransmitApduBasicRsp *) pSeRsp->pValue;
            if(pValue!=NULL)
            {
                pResponse->rilErrNo = pSeRsp->nRilErrNo;
                pResponse->apdu_len = pValue->apdu_len;
                memcpy(pResponse->apdu, pValue->apdu, pValue->apdu_len);
            }
            else ret = SITRIL_SE_ERROR_FAILURE;

            delete [] pBuffer;
            pSeRsp->Initialize();
        }
    }

    LEAVE_FUNC();
    return ret;
}

int CloseLogicalChannel(int channel)
{
    ENTER_FUNC();
    int ret = SITRIL_SE_ERROR_NONE;

    // Check Socket ID
    if(g_SocketId==INVALID_SOCKET_ID)
    {
        LogE("Error:  Invalid Socket ID(%d)", g_SocketId);
        ret = SITRIL_SE_ERROR_INVALID_SOCKET_ID;
    }
    else
    {
        // Set Sending Data
        int nReq = channel;
        ret = SendRequest(g_SocketId, RILC_REQ_SE_CLOSE_CHANNEL, &nReq, sizeof(int));
        if(ret == SITRIL_SE_ERROR_NONE)
        {
            SecureElementResponse *pSeRsp = (SecureElementResponse *) &g_aSeResponse[SE_IDX_CLOSE_CHANNEL];
            pSeRsp->Initialize();
        }
    }

    LEAVE_FUNC();
    return ret;
}

int GetAtr(LengthData *pAtr)
{
    ENTER_FUNC();
    int ret = SITRIL_SE_ERROR_NONE;

    // Check Socket ID
    if(g_SocketId==INVALID_SOCKET_ID)
    {
        LogE("Error:  Invalid Socket ID(%d)", g_SocketId);
        ret = SITRIL_SE_ERROR_INVALID_SOCKET_ID;
    }
    // Check parameters
    else if(pAtr==NULL)
    {
        LogE("Error:  pAtr(0x%p)", pAtr);
        ret = SITRIL_SE_ERROR_INVALID_PARAM;
    }
    else
    {
        // Set Sending Data
        ret = SendRequest(g_SocketId, RILC_REQ_SE_GET_ICC_ATR, NULL, 0);
        if(ret == SITRIL_SE_ERROR_NONE)
        {
            //memset(pAtr, 0, sizeof(LengthData));
            SecureElementResponse *pSeRsp = (SecureElementResponse *) &g_aSeResponse[SE_IDX_GET_ICC_ATR];
            SitAtrRsp *pValue = (SitAtrRsp *) pSeRsp->pValue;
            if(pValue!=NULL)
            {
                pAtr->length = HexString2Value((unsigned char*)pAtr->data, (const char *)pValue->atr);
            }
            else ret = SITRIL_SE_ERROR_FAILURE;

            pSeRsp->Initialize();
        }
    }

    LEAVE_FUNC();
    return ret;
}

int IsCardPresent(int socketId, int *pCardState)
{
    ENTER_FUNC();
    int ret = SITRIL_SE_ERROR_NONE;

    // Check Socket ID
    if(g_SocketId==INVALID_SOCKET_ID)
    {
        LogE("Error:  Invalid Socket ID(%d)", g_SocketId);
        ret = SITRIL_SE_ERROR_INVALID_SOCKET_ID;
    }
    else if(pCardState==NULL)
    {
        LogE("Error:  pCardState(0x%p)", pCardState);
        ret = SITRIL_SE_ERROR_INVALID_PARAM;
    }
    else
    {
        ret = SendRequest(socketId, RILC_REQ_SE_GET_CARD_PRESENT, NULL, 0);
        if(ret == SITRIL_SE_ERROR_NONE)
        {
            SecureElementResponse *pSeRsp = (SecureElementResponse *) &g_aSeResponse[SE_IDX_GET_CARD_PRESENT];
            int *pValue = (int *) pSeRsp->pValue;
            if(pValue!=NULL)
            {
                int cardState;
                memcpy(&cardState, pValue, sizeof(int));
                *pCardState = (cardState==RIL_CARDSTATE_ABSENT)? 0: 1;
            }
            else ret = SITRIL_SE_ERROR_FAILURE;

            pSeRsp->Initialize();
        }
    }

    LEAVE_FUNC();
    return ret;
}

int Value2HexString(char *pszHexStrOut, const unsigned char *pHexDecIn, int nLength)
{
    int nResult = -1;
    if(pszHexStrOut && pHexDecIn && nLength>0)
    {
        int nSrcOffset = 0;
        for(int i=0; i<nLength; i++)
        {
            nSrcOffset += snprintf(&pszHexStrOut[nSrcOffset], 3, "%02X", (unsigned int) pHexDecIn[i]);
        }

        pszHexStrOut[nSrcOffset] = '\0';
        nResult = nSrcOffset;
    }

    return nResult;
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
