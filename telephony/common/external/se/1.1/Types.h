/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#ifndef SE_TYPES_H
#define SE_TYPES_H

// Definitions
#define MAX_CHANNEL_NUM     (20)
#define MAX_ATR_LEN         (255)
#define MAX_COMMON_DATA     (256)
#define MAX_APDU_SIZE       (64*1024)       // 64K
#define MIN_APDU_SIZE       (4)
#define MAX_BUFFER_SIZE     (MAX_APDU_SIZE+14)      // 14 = sizeof(tagSecureElementTransmitApduChannelResponse)
#define BASIC_CHANNEL       (0)

typedef struct tagLengthData {
    int length;                   /*!< length of the buffer */
    uint8_t data[MAX_COMMON_DATA];   /*!< pointer to a buffer */
} lengthData;

typedef struct tagTrasmitLengthData {
    int length;                   /*!< length of the buffer */
    uint8_t data[MAX_APDU_SIZE];   /*!< pointer to a buffer */
} trasmitLengthData;

typedef struct tagSecureElementOpenChannelResponse {
    int rilErrno;
    int sessionId;
    int sw1;
    int sw2;
    uint16_t nResponseCount;
    uint8_t aResponse[MAX_COMMON_DATA];
} openChannelResponse;

typedef struct tagSecureElementTransmitApduChannelResponse {
    int rilErrno;
    int sw1;
    int sw2;
    uint16_t nResponseCount;
    uint8_t aResponse[0];
} transmitApduChannelResponse;

typedef struct tagSecureElementTransmitApduBasicResponse {
    int rilErrno;
    uint16_t nResponseCount;
    uint8_t aResponse[0];
} transmitApduBasicResponse;

typedef enum {
    E_STATUS_SUCCESS = 0,
    E_STATUS_FAIL,
    E_STATUS_SIM_ABSENT,
    E_STATUS_SECURE_EXCEPTION,
    E_STATUS_ILLEGAL_ARGUEMENT_EXCEPTION,
    E_STATUS_IO_EXCEPTION
} E_STATUS;

typedef enum {
    RIL_E_REQUEST_NOT_SUPPORTED = 6,
    RIL_E_CANCELLED = 7,
    RIL_E_MISSING_RESOURCE = 16,
    RIL_E_NO_SUCH_ELEMENT = 17,
    RIL_E_NO_MEMORY = 37,
    RIL_E_INTERNAL_ERR = 38,
    RIL_E_NO_RESOURCES = 42,
    RIL_E_SIM_ERR = 43,
    RIL_E_INVALID_SIM_STATE = 45,
} RIL_Errno;
#endif // #ifndef SE_TYPES_H
