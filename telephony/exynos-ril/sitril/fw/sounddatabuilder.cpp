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
 * sounddatabuilder.cpp
 *
 *  Created on: 2014. 6. 30.
 *      Author: sungwoo48.choi
 */

#include "sounddatabuilder.h"
#include "rillog.h"

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SOUND, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SOUND, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SOUND, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SOUND, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/**
 * SoundDataBuilder
 */
const RilData *SoundDataBuilder::BuildGetMuteResponse(int muteMode)
{
    RilDataInts *rildata = new RilDataInts(1);
    if (rildata != NULL) {
        rildata->SetInt(0, (muteMode > 0) ? 1 : 0);
    }
    return rildata;
}

const RilData *SoundDataBuilder::BuildWBAMRReportUnsolResponse(int status, int calltype)
{
    RilDataInts *rildata = new RilDataInts(2);
    if (rildata != NULL) {
        rildata->SetInt(0, status);
        rildata->SetInt(1, calltype);
    }
    return rildata;
}
