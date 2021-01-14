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
 * simdatabuilder.h
 *
 *  Created on: 2014. 11. 24.
 *      Author: sungwoo48.choi
 */

#ifndef __SIM_DATA_BUILDER_H__
#define __SIM_DATA_BUILDER_H__

#include "rildatabuilder.h"

class SimDataBuilder : public RilDataBuilder {
public:
    static const int EF_TYPE_TRANSPARENT = 0;
    static const int EF_TYPE_LINEAR_FIXED = 1;
    static const int EF_TYPE_CYCLIC = 3;

    const RilData *BuildSimPinPukResponse(int nRemainCount);
    const RilData *BuildSimNetworkLockResponse(int nRemainCount);
    const RilData *BuildSimIoResponse(BYTE sw1, BYTE sw2, int nLength, BYTE *pResponse);
    const RilData *BuildSimIoFcpTemplateResponse(BYTE sw1, BYTE sw2, int nLength, BYTE *pResponse);
    const RilData *BuildSimGetFacilityLockResponse(int nServiceClass);
    const RilData *BuildSimSetFacilityLockResponse(int nRemainCount);
    const RilData *BuildSimGetIsimAuthResponse(const int nAuthLen, const BYTE *pAuthResp);
    const RilData *BuildSimGetSimAuthResponse(int nAuthType, const int nParameterLength, const int nAuthLen, const BYTE *pAuthResp);
    const RilData *BuildSimGetGbaAuthResponse(const int nAuthLen, const BYTE *pAuthResp);
    const RilData *BuildSimOpenChannelResponse(const int sessionid, BYTE sw1, BYTE sw2, int nLength, BYTE *pResponse);
    const RilData *BuildSimCloseChannelResponse();
    const RilData *BuildSimTransmitApduBasicResponse(int nLength, BYTE *pResponse);
    const RilData *BuildSimTransmitApduChannelResponse(BYTE sw1, BYTE sw2, int nLength, BYTE *pResponse);
    const RilData *BuildGetImsiResponse(const char *imsi);
    const RilData *BuildGetATRResponse(const char *atr, int nLength);
    const RilData *BuildGet3GPbCapaResponse(int entryNum, int *pb);
    const RilData *BuildIccidInfoIndicate(int nLength, const BYTE *pIccId);
    const RilData *BuildGetCarrierRestrictionsResponse(int lenAllowed, int lenExcluded, void *pAllowed, void *pExcluded);
    const RilData *BuildGetSimLockInfoResponse(int policy, int status, int lockType,
            int maxRetryCount, int remainCount, int lockCodeCount, const char *lockCode);
};


#endif /* __SIM_DATA_BUILDER_H__ */
