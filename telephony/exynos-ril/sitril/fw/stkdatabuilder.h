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
 * stkdatabuilder.h
 *
 *  Created on: 2014. 11. 24.
 *      Author: sungwoo48.choi
 */

#ifndef __STK_DATA_BUILDER_H__
#define __STK_DATA_BUILDER_H__

#include "rildatabuilder.h"

class StkDataBuilder : public RilDataBuilder {
public:
    const RilData *BuildStkEnvelopeCommandResponse(int nLength, BYTE *pEnvelopeCommand);
    const RilData *BuildStkEnvelopeStatusResponse(int nSW1, int nSW2, int nLength, BYTE *pEnvelopeStatus);
    const RilData *BuildStkProactiveCommandIndicate(int nLength, BYTE *pProactiveCommand);
    const RilData *BuildStkSimRefreshIndicate(int nResult, int nEFID, int nAidLen, BYTE *pAID);
    const RilData *BuildStkSsReturnResultIndicate(int nLength, BYTE *pReturnResult);
    const RilData *BuildStkEventNotify(int nLength, BYTE *pSatUsatCommand);
    const RilData *BuildStkCallSetupIndicate(int nTimeout);
    const RilData *BuildStkCcAlphaNtf(int lenAlpha, const BYTE *pAlpha);
};

#endif /* __STK_DATA_BUILDER_H__ */
