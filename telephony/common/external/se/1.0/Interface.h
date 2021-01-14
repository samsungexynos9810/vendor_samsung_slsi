/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef RIL_INTEFACE_H
#define RIL_INTEFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Types.h"
#define RIL_CLIENT_LIBPATH "libsitril-se.so"

struct ril_handle {
    void *handle;
    void *client;
};

// SitRil Secure Element Errors
typedef enum _SitRilSecureElementError {
    SITRIL_SE_ERROR_NONE,
    SITRIL_SE_ERROR_LIB_LOAD_FAIL,
    SITRIL_SE_ERROR_NO_SYMBOL,
    SITRIL_SE_ERROR_OPEN_FAIL,
    SITRIL_SE_ERROR_SEND_FAIL,
    SITRIL_SE_ERROR_INVALID_PARAM,
    SITRIL_SE_ERROR_REGISTRATION_FAIL,
    SITRIL_SE_ERROR_ALREADY_REGISTERD,
    SITRIL_SE_ERROR_TIMEOUT,
    SITRIL_SE_ERROR_SIM_CARD_ABSENT,
    SITRIL_SE_ERROR_MAX
} SitRilSeError;

/* Function prototypes */
extern E_STATUS rilOpen(void);
extern E_STATUS rilClose(void);
extern E_STATUS rilSetClient(int clientId);
extern E_STATUS rilOpenLogicalChannel(openChannelResponse *pResponse, lengthData *pAid, int p2);
extern E_STATUS rilTransmitApduLogicalChannel(transmitApduChannelResponse *pRespone, int channel, int cla, int instruction, int p1, int p2, int p3, trasmitLengthData *pData);
extern E_STATUS rilTransmitApduBasicChannel(transmitApduBasicResponse *pRespone, int cla, int instruction, int p1, int p2, int p3, trasmitLengthData *pData);
extern E_STATUS rilCloseLogicalChannel(int channel);
extern E_STATUS rilGetAtr(lengthData *pAtr);
extern bool rilIsCardPresent(int socketId);

#ifdef __cplusplus
};
#endif

#endif  // RIL_INTEFACE_H

