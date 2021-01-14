/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __SITRILWLAN_H__
#define __SITRILWLAN_H__

#ifdef __cplusplus
extern "C" {
#endif

// SITRil WLan Errors
typedef enum _SITRilWLanError {
    SITRIL_WLAN_ERROR_NONE,
    SITRIL_WLAN_ERROR_NO_FILE,
    SITRIL_WLAN_ERROR_NO_LIB_OPEN_FAIL,
    SITRIL_WLAN_ERROR_NO_LIB_SEND_FAIL,
    SITRIL_WLAN_ERROR_NO_DEVICE,
    SITRIL_WLAN_ERROR_INVALID_PARAM,
    SITRIL_WLAN_ERROR_REGISTERATION_FAIL,
    SITRIL_WLAN_ERROR_ALREADY_REGISTERD,
    SITRIL_WLAN_ERROR_TIMEOUT,
    SITRIL_WLAN_ERROR_MAX
} SITRilWLanError;

#define MAX_AUTH_LEN 34
#define MAX_AUTH_RSP_LEN 69


typedef int (*SITRilWLanUnsolCallback)(unsigned msgId, const void *data, size_t datalen, unsigned int channel);
typedef void (*SITRilWLanOnResponse)(unsigned msgId, signed error, void* data, size_t datalen, unsigned int channel);

typedef struct __auth_request
{
    int socket_id;
    unsigned char auth_type;
    unsigned char auth_len;
    unsigned char auth[MAX_AUTH_LEN];
} auth_request_type;

typedef struct _auth_response
{
    unsigned char auth_type;
    unsigned char auth_rsp_len;
    unsigned char auth_rsp[MAX_AUTH_RSP_LEN];
} auth_response_type;


// #### Initialization Functions ####

// ----------------------------------------------------
// API Name : Open
// Description
//     : Open WLan Interaface of SIT Ril.
// Params
//    - void
// Results
//    - int error value of SITRilSITRilWLanError
// ----------------------------------------------------
int Open(void);

// ----------------------------------------------------
// API Name : Close
// Description
//     : Close WLan Interaface of SIT Ril.
// Params
//    - void
// Results
//    - int error value of SITRilSITRilWLanError
// ----------------------------------------------------
int Close(void);

// ----------------------------------------------------
// API Name : RegisterCallback
// Description
//     : Register Callback function to receive SIMRIL_STATE_FOR_AUDIO_XXX events of SITRil states for WLan
// Params
//    - (in) int interestingRilstateforaudio
//      : SIMRIL_STATE_FOR_AUDIO_XXX events
//    - (in) SITRilAudioCallback callback
//      : Callback function
// Results
//    - int error value of SITRilSITRilWLanError
// ----------------------------------------------------
int RegisterCallback(int interestingRilstateforwlan, SITRilWLanUnsolCallback callback);

// #### Operation Functions ####

// ----------------------------------------------------
// API Name : GetIMSI
// Description
//     : Get IMSI
// Params
//    - (in) char *imsi
//    - (in) int socket_id
// Results
//    - int error value of SITRilSITRilWLanError
// ----------------------------------------------------
int EAP_Get_IMSI(char *imsi, int socket_id);

// ----------------------------------------------------
// API Name : EAP_3G_Authenticate
// Description
//     : EAP 3G Authenticate
// Params
//    - (in) auth_request_type *request
//    - (out) auth_response_type *response
//    - (in) int socket_id
// Results
//    - int error value of SITRilSITRilWLanError
// ----------------------------------------------------
int EAP_3G_Authenticate(auth_request_type *request, auth_response_type *response, int socket_id);
// ----------------------------------------------------
// API Name : EAP_GSM_Authenticate
// Description
//     : EAP GSM Authenicate
// Params
//    - (in) auth_request_type *request
//    - (out) auth_response_type *response
//    - (in) int socket_id
// Results
//    - int error value of SITRilSITRilWLanError
// ----------------------------------------------------
int EAP_GSM_Authenticate(auth_request_type *request, auth_response_type *response, int socket_id);

#ifdef __cplusplus
};
#endif

#endif // #ifndef __SITRILWLAN_H__
