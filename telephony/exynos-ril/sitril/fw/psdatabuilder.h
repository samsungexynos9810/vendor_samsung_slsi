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
 * psdatabuilder.h
 *
 *  Created on: 2014. 11. 24.
 *      Author: sungwoo48.choi
 */

#ifndef __PS_DATA_BUILDER_H__
#define __PS_DATA_BUILDER_H__

#include "rildatabuilder.h"
#include "pdpcontext.h"
#include <vector>
using namespace std;

class PsDataBuilder : public RilDataBuilder {
private:
    char *ConvertPdpType(int nPdpType);
public:
    //const RilData *BuildSetupDataCallResponse(const DataCall *dataCall);
    const RilData *BuildSetupDataCallResponse(PdpContext *pPdpContext);
    const RilData *BuildSetupDataCallResponse(int errorCode, PdpContext *pPdpContext);
    const RilData *BuildSetupDataCallResponse(int errorCode, int status);
    const RilData *BuildPcoData(int cid, int nPdpType, int pcoId, int contentsLen, char *pContents);
};

class PsDataCallListBuilder : public RilDataBuilder {
private:
    vector<PdpContext *> m_PdpContextList;
public:
    int AddDataCall(PdpContext *pPdpContext);
    const RilData *Build();
    void Clear();
};
class PsDataNasTimerStatusBuilder : public RilDataBuilder {
public:
    const RilData *BuildNasTimerStatus(const SitNasTimerStatus *pNasTimerStatus);
};
#endif /* __PS_DATA_BUILDER_H__ */
