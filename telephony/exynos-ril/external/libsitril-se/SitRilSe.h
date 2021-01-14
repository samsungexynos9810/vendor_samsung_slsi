/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __SITRILSE_H__
#define __SITRILSE_H__

#ifdef __cplusplus
extern "C" {
#endif

// Definitions
#define MAX_LENGTH_DATA_SIZE    256
#define MAX_OPEN_CHANNEL_SIZE   256
#define MAX_APDU_SIZE           (1024*64)       // 64K

// SitRil Secure Element Errors
enum {
    SITRIL_SE_ERROR_NONE,
    SITRIL_SE_ERROR_FAILURE,
    SITRIL_SE_ERROR_NOT_OPENED_LIB,
    SITRIL_SE_ERROR_LIB_LOAD_FAIL,
    SITRIL_SE_ERROR_NO_SYMBOL,
    SITRIL_SE_ERROR_OPEN_FAIL,
    SITRIL_SE_ERROR_SEND_FAIL,
    SITRIL_SE_ERROR_INVALID_SOCKET_ID,
    SITRIL_SE_ERROR_INVALID_PARAM,
    SITRIL_SE_ERROR_REGISTRATION_FAIL,
    SITRIL_SE_ERROR_ALREADY_LIB_LOADED,
    SITRIL_SE_ERROR_ALREADY_REGISTERD,
    SITRIL_SE_ERROR_TIMEOUT,
    SITRIL_SE_ERROR_MAX
} SitRilSeError;

typedef struct tagSitRilSecureElementOpenChannelResponse {
    int rilErrNo;
    int sessionId;
    int sw1;
    int sw2;
    short nResponseCount;
    uint8_t aResponse[MAX_OPEN_CHANNEL_SIZE];
} SitRilSeOpenChannelResponse;

typedef struct tagSitRilSecureElementTransmitApduChannelResponse
{
    int rilErrNo;
    int sw1;
    int sw2;
    unsigned short apdu_len;
    unsigned char apdu[MAX_APDU_SIZE];
} SitRilSeTransmitApduChannelRsp;

typedef struct tagSitRilSecureElementTransmitApduBasicResponse
{
    int rilErrNo;
    unsigned short apdu_len;
    unsigned char apdu[MAX_APDU_SIZE];
} SitRilSeTransmitApduBasicRsp;

typedef struct tagLengthData {
    int length;
    uint8_t data[MAX_LENGTH_DATA_SIZE];
} LengthData;

// #### Initialization Functions ####

// ----------------------------------------------------
// API Name : Open
// Description
//     : Open Secure Element Interaface of SIT RIL.
// Params
//    - void
// Results
//    - int error value of below SitRilSeError
//          SITRIL_SE_ERROR_NONE,
//          SITRIL_SE_ERROR_FAILURE,
//          SITRIL_SE_ERROR_NOT_OPENED_LIB,
//          SITRIL_SE_ERROR_LIB_LOAD_FAIL,
//          SITRIL_SE_ERROR_NO_SYMBOL,
//          SITRIL_SE_ERROR_OPEN_FAIL,
// ----------------------------------------------------
int Open(void);

// ----------------------------------------------------
// API Name : Close
// Description
//     : Close Secure Element Interaface of SIT RIL.
// Params
//    - void
// ----------------------------------------------------
void Close(void);

// ----------------------------------------------------
// API Name : Close
// Description
//     : Close Secure Element Interaface of SIT RIL.
// Params
//    - (in) int socketId
// Results
//    - int error value of below SitRilSeError
//          SITRIL_SE_ERROR_NONE,
//          SITRIL_SE_ERROR_FAILURE,
//          SITRIL_SE_ERROR_NOT_OPENED_LIB,
//          SITRIL_SE_ERROR_INVALID_SOCKET_ID
// ----------------------------------------------------
int SetSocketId(int socketId);

// ----------------------------------------------------
// API Name : OpenLogicalChannel
// Description
//     : Open Logical Channel to transmit APDU
// Params
//    - (out) SitRilSeOpenChannelResponse &pResponse
//    - (in) char[] aid
//    - (in) int p2
// Results
//    - int error value of below SitRilSeError
//          SITRIL_SE_ERROR_NONE,
//          SITRIL_SE_ERROR_FAILURE,
//          SITRIL_SE_ERROR_NOT_OPENED_LIB,
//          SITRIL_SE_ERROR_LIB_LOAD_FAIL,
//          SITRIL_SE_ERROR_NO_SYMBOL,
//          SITRIL_SE_ERROR_OPEN_FAIL,
//          SITRIL_SE_ERROR_INVALID_SOCKET_ID,
//          SITRIL_SE_ERROR_INVALID_PARAM,
//          SITRIL_SE_ERROR_REGISTRATION_FAIL,
//          SITRIL_SE_ERROR_TIMEOUT,
// ----------------------------------------------------
int OpenLogicalChannel(SitRilSeOpenChannelResponse *pResponse, LengthData *pAid, int p2);

// ----------------------------------------------------
// API Name : TransmitApduLogicalChannel
// Description
//     : Transmit APDU on Logical Channel
// Params
//    - (out) LengthData &apdu
//    - (in) int channel
//    - (in) int cla
//    - (in) int instruction
//    - (in) int p1
//    - (in) int p2
//    - (in) int p3
//    - (in) LengthData data
// Results
//    - int error value of below SitRilSeError
//          SITRIL_SE_ERROR_NONE,
//          SITRIL_SE_ERROR_FAILURE,
//          SITRIL_SE_ERROR_NOT_OPENED_LIB,
//          SITRIL_SE_ERROR_INVALID_SOCKET_ID,
//          SITRIL_SE_ERROR_INVALID_PARAM,
//          SITRIL_SE_ERROR_TIMEOUT,
// ----------------------------------------------------
int TransmitApduLogicalChannel(SitRilSeTransmitApduChannelRsp *pResponse, int channel, int cla, int instruction, int p1, int p2, int p3, LengthData *pData);

// ----------------------------------------------------
// API Name : TransmitApduBasicChannel
// Description
//     : Transmit APDU on Basic Channel without Session ID
// Params
//    - (out) LengthData &apdu
//    - (in) int cla
//    - (in) int instruction
//    - (in) int p1
//    - (in) int p2
//    - (in) int p3
//    - (in) LengthData data
// Results
//    - int error value of below SitRilSeError
//          SITRIL_SE_ERROR_NONE,
//          SITRIL_SE_ERROR_FAILURE,
//          SITRIL_SE_ERROR_NOT_OPENED_LIB,
//          SITRIL_SE_ERROR_INVALID_SOCKET_ID,
//          SITRIL_SE_ERROR_INVALID_PARAM,
//          SITRIL_SE_ERROR_TIMEOUT,
// ----------------------------------------------------
int TransmitApduBasicChannel(SitRilSeTransmitApduBasicRsp *pResponse, int cla, int instruction, int p1, int p2, int p3, LengthData *pData);

// ----------------------------------------------------
// API Name : CloseLogicalChannel
// Description
//     : Close APDU on Basic Channel
// Params
//    - (in) int channel
// Results
//    - int error value of below SitRilSeError
//          SITRIL_SE_ERROR_NONE,
//          SITRIL_SE_ERROR_FAILURE,
//          SITRIL_SE_ERROR_NOT_OPENED_LIB,
//          SITRIL_SE_ERROR_INVALID_SOCKET_ID,
//          SITRIL_SE_ERROR_INVALID_PARAM,
//          SITRIL_SE_ERROR_TIMEOUT,
// ----------------------------------------------------
int CloseLogicalChannel(int channel);

// ----------------------------------------------------
// API Name : GetAtr
// Description
//     : Get ATR
// Params
//    - (out ) LengthData &atr
// Results
//    - int error value of below SitRilSeError
//          SITRIL_SE_ERROR_NONE,
//          SITRIL_SE_ERROR_FAILURE,
//          SITRIL_SE_ERROR_NOT_OPENED_LIB,
//          SITRIL_SE_ERROR_INVALID_SOCKET_ID,
//          SITRIL_SE_ERROR_INVALID_PARAM,
//          SITRIL_SE_ERROR_TIMEOUT,
// ----------------------------------------------------
int GetAtr(LengthData *pAtr);

// ----------------------------------------------------
// API Name : GetCardPresent
// Description
//     : Get Card Status
// Params
//    - (in) int socketId
//    - (out) int *pCardState
// Results
//    - int error value of below SitRilSeError
//          SITRIL_SE_ERROR_NONE,
//          SITRIL_SE_ERROR_FAILURE,
//          SITRIL_SE_ERROR_INVALID_SOCKET_ID,
// ----------------------------------------------------
int IsCardPresent(int socketId, int *pCardState);


#ifdef __cplusplus
};
#endif

#endif // #ifndef __SITRILSE_H__
