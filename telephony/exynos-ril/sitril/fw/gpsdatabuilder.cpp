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
 * gpsdatabuilder.cpp
 *
 *  Created on: 2015. 1. 23.
 *      Author: m.afzal
 */

#include "gpsdatabuilder.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

const RilData *GpsDataBuilder::BuildFreqAidingResponse( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildLPPSuplEcidInfoResponse( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildRRLPSuplEcidInfoResponse( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildMOLocationResponse( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildLPPServingCellInfoResponse( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildSuplNIReadyResponse( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildMeasurePositionIndication( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildAssistDataIndication( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildMTLocReqIndication( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildLppReqCapIndication( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildLppProvideAssistDataIndication( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildLppReqLocInfoIndication( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildLppErrorIndication( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildSuplLppDataInfoIndication( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}

const RilData *GpsDataBuilder::BuildSuplNIMessageIndication( const char * pResponse, int Length)
{
    return new RilDataRaw(pResponse, Length);
}
