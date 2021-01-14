/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_RILC_IF_CLIENT                      3

// sitril-interface support start ---------------------
#define RILC_REQ_IF_BASE                        80
#define RILC_REQ_IF_EXECUTE_AM                  (RILC_REQ_IF_BASE)

// sitril-interface errors
typedef enum _SITRilIfError {
    SITRIL_IF_ERROR_NONE,
    SITRIL_IF_ERROR_NO_FILE,
    SITRIL_IF_ERROR_NO_LIB_OPEN_FAIL,
    SITRIL_IF_ERROR_NO_LIB_SEND_FAIL,
    SITRIL_IF_ERROR_NO_DEVICE,
    SITRIL_IF_ERROR_INVALID_PARAM,
    SITRIL_IF_ERROR_REGISTERATION_FAIL,
    SITRIL_IF_ERROR_ALREADY_REGISTERD,
    SITRIL_IF_ERROR_MAX
} SITRilIfError;

typedef enum _SITRilResponseError
{
    SITRIL_IF_SUCCESS = 0,
    SITRIL_IF_FAILURE,
}SITRilResponseError;

typedef enum _SITRilIfClientId {
    SITRIL_IF_DAEMON = 0,
    SITRIL_IF_RIL,
    SITRIL_IF_OTHER,
} SITRilIfClientId;

/* Callbacks */
typedef void (*SITRilIfOnResponse)(unsigned int msgId, int status, void* data, size_t length, unsigned int channel);
typedef int (*SITRilIfUnsolCallback)(unsigned int msgId, void* data, size_t length, unsigned int socket_id);

// #### Initialization Functions ####

// ----------------------------------------------------
// API Name : IfOpen
// Description
//     : Open Interaface of SIT Ril.
// Params
//    - void
// Results
//    - int error value of SITRilIfError
// ----------------------------------------------------
int IfOpen(int client_id);

// ----------------------------------------------------
// API Name : IfClose
// Description
//     : Close Interaface of SIT Ril.
// Params
//    - void
// Results
//    - int error value of SITRilIfError
// ----------------------------------------------------
int IfClose(int client_id);

//IF support start ---------------------
int ExecuteAm(int client_id, unsigned char *data, int data_len, SITRilIfOnResponse callback);

//IF support end ---------------------

#ifdef __cplusplus
};
#endif
