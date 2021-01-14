/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
*/

#ifndef RIL_EXTERNAL_SERVICE_H
#define RIL_EXTERNAL_SERVICE_H

#include <telephony/ril.h>
#include <telephony/ril_ext.h>
#include <slsi/ril_external.h>
#include <ril_internal.h>
#include <libril/ril_ex.h>

#define RIL_EXTERNAL_SERVICE_NAME "rilExternal"
typedef int (*radioExternal_responseFunction)(int, int, int, int, RIL_Errno, void *, size_t);

namespace radioExternal {
void registerService(const RIL_RadioExternalFunctions *callbacks);

pthread_rwlock_t * getRadioExternalServiceRwlock();

/**
 * solicited response
 */
int sendRequestRawResponse(int clientId, int rilcMsgId, int slotId, int serial, RIL_Errno e, void *response, size_t responseLen);
int sendRequestRawResponseSeg(int clientId, int rilcMsgId, int slotId, int serial, RIL_Errno e, void *response, size_t responseLen);

// response handler
int getAvailableNtworksResponse(int clientId, int rilcMsgId, int slotId, int serial,
        RIL_Errno e, void *response, size_t responseLen);
/**
 * unsolicited response
 */
int rilExternalRawIndication(int clientId, int rilcMsgId, int slotId, const void *indication, size_t indicationLen);
int rilExternalRawIndicationSeg(int clientId, int rilcMsgId, int slotId, const void *indication, size_t indicationLen);

}   // namespace radioExternal

#endif  // RIL_EXTERNAL_SERVICE_H
