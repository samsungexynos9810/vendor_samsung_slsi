/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef SIT_RIL_SAR_H
#define SIT_RIL_SAR_H

#ifdef __cplusplus
extern "C" {
#endif

#define RILC_REQ_SAR_BASE                            500
#define RILC_REQ_SAR_SET_SAR_STATE                   (RILC_REQ_SAR_BASE + 1)
#define RILC_REQ_SAR_GET_SAR_STATE                   (RILC_REQ_SAR_BASE + 2)

#define RILC_UNSOL_SAR_BASE                          7000
#define RILC_UNSOL_SAR_RF_CONNECTION                 (RILC_UNSOL_SAR_BASE + 1)

/* SITRil sar Errors */
typedef enum {
    SITRIL_SAR_ERROR_NONE,
    SITRIL_SAR_ERROR_NO_FILE,
    SITRIL_SAR_ERROR_NO_LIB_OPEN_FAIL,
    SITRIL_SAR_ERROR_NO_LIB_SEND_FAIL,
    SITRIL_SAR_ERROR_NO_DEVICE,
    SITRIL_SAR_ERROR_INVALID_PARAM,
    SITRIL_SAR_ERROR_REGISTERATION_FAIL,
    SITRIL_SAR_ERROR_ALREADY_REGISTERD,
    SITRIL_SAR_ERROR_MAX
} SITRilSarError;

/* Callbacks */
typedef void (*SITRilSarOnResponse)(unsigned int msgId, int status, void* data, size_t length, unsigned int channel);
typedef int (*SITRilSarUnsolCallback)(unsigned int msgId, void* data, size_t length, unsigned int socket_id);

int RegisterCallback(SITRilSarUnsolCallback callback);

// #### API's ####
/* ----------------------------------------------------
// Description
//     : Open sar Interaface of SIT Ril.
// Results
//    - int error value of SITRilSarError
// ----------------------------------------------------*/
int SarOpen(SITRilSarUnsolCallback callback);

/* ----------------------------------------------------
// Description
//     : Close sar Interaface of SIT Ril.
// Results
//    - int error value of SITRilSarError
// ----------------------------------------------------*/
int SarClose();

/* ----------------------------------------------------
// SIT CMD Name : SIT_OEM_SET_SAR_STATE
// RCM ID : 0x4101
// Description
//     : This solicited message is to set sar state.
// ----------------------------------------------------*/
int SetSarState(int state, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_OEM_GET_SAR_STATE
// RCM ID : 0x4100
// Description
//     : This solicited message is to set sar state.
// ----------------------------------------------------*/
int GetSarState(int channel);

#ifdef __cplusplus
};
#endif

#endif  /* SIT_RIL_SAR_H */
