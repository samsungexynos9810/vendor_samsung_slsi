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
 * miscdatabuilder.h
 *
 *  Created on: 2014. 11. 24.
 *      Author: sungwoo48.choi
 */

#ifndef __MISC_DATA_BUILDER_H__
#define __MISC_DATA_BUILDER_H__

#include "rildatabuilder.h"

class MiscDataBuilder : public RilDataBuilder {
public:
    const RilData *BuildBaseBandVersionResponse(const char *SwVer);
    const RilData *BuildGetTtyModeResponse(int TtyMode);
    const RilData *BuildIMEIResponse(int IMEILen, const BYTE *IMEI);
    const RilData *BuildIMEISVResponse(int IMEISVLen, const BYTE *IMEISV);
    const RilData *BuildDevIDResponse(int IMEILen, const BYTE *IMEI, int IMEISVLen, const BYTE *IMEISV, int MEIDLen, const BYTE *MEID,
            int ESNLen, const BYTE *ESN);
    const RilData *BuildNitzTimeIndication(int DayLightValid, int Year, int Month, int Day, int Hour,
            int Minute, int Second, int TimeZone, int DayLightAdjust, int DayofWeek);
    const RilData *BuildDisplayEngIndication(const char * pResponse, int Length);
    const RilData *BuildLceDataIndication(int dlLc, int ulLc, int confLevel, int isSuspended);
    const RilData *BuildCdmaSubscriptionSource(int nSubscriptionSource);
    const RilData *BuildCdmaSubscription(char *mdn, WORD sid, WORD nid, char *min, UINT prl_version);
    const RilData *BuildHardwareConfigNV(char *pszUUID, int nState, int nModel, UINT uRat, int nMaxCS, int nMaxPS, int nMaxStandby);
    const RilData *BuildHardwareConfigRuim(char *pszUUID, int nState, char *pszModemUUID);
    const RilData *BuildRssiScanResult(int total, int current, int startFrequency, int endFrequency, int step, INT16* result, int resultSize);
    const RilData *BuildATCommand(const char *command);
    const RilData *BuildGetModemStatus(int status);
    const RilData *BuildModemInfo(int type, void *data, unsigned int datalen);
};

/**
 * SignalStrengthBuilder
 */
class SignalStrengthBuilder : public RilDataBuilder {
private:
    int mHalVer;
public:
    SignalStrengthBuilder(int halVer = HAL_VERSION_CODE(1, 0));
    const RilData* Build(RIL_SignalStrength_V1_4& currentSignalStrength);
};

#endif /* __MISC_DATA_BUILDER_H__ */
