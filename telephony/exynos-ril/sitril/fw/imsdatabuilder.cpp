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
 * imsdatabuilder.cpp
 *
 *  Created on: 2014. 11. 27.
 *      Author: Martin
 */

#include <stdio.h>
#include "rildatabuilder.h"
#include "imsdatabuilder.h"
#include "util.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define ENTER_FUNC()        { RilLogV("ImsDataBuilder::%s() [<-- ", __FUNCTION__); }
#define LEAVE_FUNC()        { RilLogV("ImsDataBuilder::%s() [--> ", __FUNCTION__); }
#define BYTEBIT(byte, idx)        ((byte >> (8-idx)) & 0x01)

const RilData *ImsDataBuilder::BuildImsGetConfigResponse(const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *ImsDataBuilder::BuildImsGetFrameSyncResponse(const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *ImsDataBuilder::BuildImsGbaSimAuth(const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

//AIMS support start ---------------------
const RilData *ImsDataBuilder::BuildAImsGetFrameTimeResponse(const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *ImsDataBuilder::BuildAImsCallModifyResponse(const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *ImsDataBuilder::BuildAImsGetDataResponse(const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *ImsDataBuilder::BuildAImsGetCallForwardResponse(const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *ImsDataBuilder::BuildAImsGetCallWaitingResponse(const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *ImsDataBuilder::BuildAImsGetCallBarringResponse(const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *ImsDataBuilder::BuildAImsSendSMSResponse(const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *ImsDataBuilder::BuildAImsSendExpectMoreResponse(const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

//AIMS support end ---------------------
