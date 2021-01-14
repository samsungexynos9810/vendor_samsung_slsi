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
    RIL-Client library (header)
*/
#ifndef _SITRIL_CLIENT_H
#define _SITRIL_CLIENT_H

#include <sys/types.h>
#include <telephony/ril.h>
#include <slsi/ril_client.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    Types
*/
#define RILC_AUDIO_MUTE_DISABLE       0        /* same as in ril.h */
#define RILC_AUDIO_MUTE_ENABLE        1        /* same as in ril.h */
#define RILC_AUDIO_MUTE_SIZE          sizeof(int)
#define RILC_AUDIO_VOLUME_SIZE        sizeof(int)
#define RILC_AUDIO_PATH_SIZE          sizeof(int)
#define RILC_AUDIO_MICCTL_SIZE        sizeof(int)
#define RILC_AUDIO_AUDIO_CLOCK_SIZE   sizeof(int)
#define RILC_AUDIO_AUDIO_LOOPBACK_SIZE        (sizeof(int)+sizeof(int))

#define RILC_REQ_IMS_SET_CONFIGURATION_SIZE             121
#define RILC_REQ_IMS_GET_CONFIGURATION_SIZE             25
#define RILC_REQ_IMS_SET_EMERGENCY_CALL_STATUS_SIZE     2
#define RILC_REQ_IMS_SET_SRVCC_CALL_LIST_SIZE           0

#define RILC_REQ_SET_ENG_MODE_SIZE           1
#define RILC_REQ_SET_SCR_LINE_SIZE           1
#define RILC_REQ_SET_DEBUG_TRACE_SIZE        1


/* Callbacks */
typedef void (*Rilc_OnResponse)(unsigned int msgId, int status, void* data, size_t length, unsigned int channel);
typedef void (*Rilc_OnUnsolicitedResponse)(unsigned int msgId, void* data, size_t length, unsigned int channel);

/*
    API
*/
/* Open client */
void* RILC_Open(void);

/* Re-connect to RIL */
int RILC_Reconnect(void* client);

/* Register unsolicited handler */
int RILC_RegisterUnsolicitedHandler(void* client, Rilc_OnUnsolicitedResponse handler);

/* Send request to RIL */
int RILC_Send(void* client, unsigned int msgId, void* data, size_t length, Rilc_OnResponse handler, unsigned int channel);

/* Delete client */
int RILC_Close(void* client);

#ifdef __cplusplus
};
#endif

#endif
