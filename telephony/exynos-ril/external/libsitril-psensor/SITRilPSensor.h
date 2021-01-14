/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef SIT_RIL_PSENSOR_H
#define SIT_RIL_PSENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#define RILC_REQ_PSENSOR_BASE                            400
#define RILC_REQ_PSENSOR_SET_STATUS                      (RILC_REQ_PSENSOR_BASE + 1)

#define RILC_UNSOL_PSENSOR_BASE                          6000
#define RILC_UNSOL_PSENSOR_CONTROL_STATE                 6001

/* SITRil P-Sensor Errors */
typedef enum {
    SITRIL_PSENSOR_ERROR_NONE,
    SITRIL_PSENSOR_ERROR_NO_FILE,
    SITRIL_PSENSOR_ERROR_NO_LIB_OPEN_FAIL,
    SITRIL_PSENSOR_ERROR_NO_LIB_SEND_FAIL,
    SITRIL_PSENSOR_ERROR_NO_DEVICE,
    SITRIL_PSENSOR_ERROR_INVALID_PARAM,
    SITRIL_PSENSOR_ERROR_REGISTERATION_FAIL,
    SITRIL_PSENSOR_ERROR_ALREADY_REGISTERD,
    SITRIL_PSENSOR_ERROR_MAX
} SITRilPSensorError;

/* Callbacks */
typedef void (*SITRilPSensorOnResponse)(unsigned int msgId, int status, void* data, size_t length, unsigned int channel);
typedef int (*SITRilPSensorUnsolCallback)(unsigned int msgId, void* data, size_t length, unsigned int socket_id);
int RegisterCallback(SITRilPSensorUnsolCallback callback);

// #### API's ####
/* ----------------------------------------------------
// Description
//     : Open P-Sensor Interaface of SIT Ril.
// Results
//    - int error value of SITRilPSensorError
// ----------------------------------------------------*/
int PSensorOpen(SITRilPSensorUnsolCallback callback);

/* ----------------------------------------------------
// Description
//     : Close P-Sensor Interaface of SIT Ril.
// Results
//    - int error value of SITRilPSensorError
// ----------------------------------------------------*/
int PSensorClose();

/* ----------------------------------------------------
// SIT CMD Name : SIT_SET_SENSOR_STATUS
// RCM ID : 0x092B
// Description
//     : This solicited message is to set P-sensor status.
// ----------------------------------------------------*/
int SetPSensorStatus(int status, int channel);

#ifdef __cplusplus
};
#endif

#endif  /* SIT_RIL_PSENSOR_H */
