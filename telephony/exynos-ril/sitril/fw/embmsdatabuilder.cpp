/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "embmsdatabuilder.h"
#include "util.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define ENTER_FUNC()        { RilLogV("EmbmsDataBuilder::%s() [<-- ", __FUNCTION__); }
#define LEAVE_FUNC()        { RilLogV("EmbmsDataBuilder::%s() [--> ", __FUNCTION__); }

enum EMBMS_CoverageState_t {
    EMBMS_NO_COVERAGE,
    EMBMS_UNICAST_COVERAGE,
    EMBMS_FULL_COVERAGE,
    EMBMS_TMGI_CONFIRMED,
    EMBMS_UNKNOW_STATE_COVERAGE = 0xFF
};

const RilData *EmbmsDataBuilder::BuildEmbmsCoverageIndicate(int nCoverage)
{
    ENTER_FUNC();
    EMBMS_CoverageState_t coverage = EMBMS_NO_COVERAGE;

    switch (nCoverage) {
        case 0: // Full Coverage (LTE Unicast and eMBMS available)
            coverage = EMBMS_FULL_COVERAGE;
            break;
        case 1: // No Coverage or Unicast Coverage
            coverage = EMBMS_NO_COVERAGE;
            break;
        default:
            break;
    }

    RilDataInts *rildata = new RilDataInts(1);
    if(rildata != NULL) {
        rildata->SetInt(0, coverage);
    }

    LEAVE_FUNC();
    return rildata;
}

uint64_t EmbmsDataBuilder::getLongTypeTmgi(const BYTE* pTmgi) {
    uint64_t tmgi = 0;

    if (pTmgi == NULL) {
        return -1UL;
    }

    // MCC, MNC
    tmgi |= ((((uint64_t) pTmgi[0]) & 0x00000000000000FF) << 40);
    tmgi |= ((((uint64_t) pTmgi[1]) & 0x00000000000000FF) << 32);
    tmgi |= ((((uint64_t) pTmgi[2]) & 0x00000000000000FF) << 24);
    tmgi |= ((((uint64_t) pTmgi[3]) & 0x00000000000000FF) << 16);
    tmgi |= ((((uint64_t) pTmgi[4]) & 0x00000000000000FF) << 8);
    tmgi |= (((uint64_t) pTmgi[5]) & 0x00000000000000FF);

    return tmgi;
}

const RilData *EmbmsDataBuilder::BuildEmbmsSessionListResponse(int nState, int nOosReason, int nRecordNum, const BYTE *pTmgi)
{
    ENTER_FUNC();
    BYTE tmgi[MAX_TMGI_LEN];

    RIL_SessionListInfo listInfo;
    memset(&listInfo, 0x0, sizeof(RIL_SessionListInfo));

    RilDataRaw *rildata = new RilDataRaw();
    if(rildata != NULL) {
        listInfo.state = nState;
        listInfo.count = nRecordNum;
        for( int i = 0; i < nRecordNum; i++) {
            memcpy(tmgi, &pTmgi[i*MAX_TMGI_LEN], MAX_TMGI_LEN);
            listInfo.tmgi[i] = getLongTypeTmgi((const BYTE*)tmgi);
        }

        rildata->SetData(&listInfo, sizeof(RIL_SessionListInfo));
    }

    LEAVE_FUNC();
    return rildata;
}

const RilData *EmbmsDataBuilder::BuildEmbmsSignalStrengthResponse(UINT32 count, const uint32_t *pSnrList)

{
    ENTER_FUNC();
    RilDataRaw *rildata = new RilDataRaw();

    if (rildata != NULL) {
        RIL_SignalInfoList signalInfoList;
        signalInfoList.count = count;
        for(int i = 0; i < (int)count; i++) {
            signalInfoList.Info[i].type = 2; /* EMBMSAL_SI_SNR */
            signalInfoList.Info[i].value = *pSnrList;
        }

        rildata->SetData(&signalInfoList, sizeof(RIL_SignalInfoList));
    }

    LEAVE_FUNC();
    return rildata;
}

const RilData *EmbmsDataBuilder::BuildEmbmsNetworkTimeResponse(const uint64_t networkTime)
{
    ENTER_FUNC();

    uint64_t microTime = networkTime * (uint64_t)1000000; // input is sec so change to microsec.
    if (networkTime == (uint64_t)(-1) || networkTime == (uint64_t)(0)) {
        microTime = (uint64_t)(-1);
    }

    RilDataRaw *rildata = new RilDataRaw();
    if (rildata != NULL) {
        rildata->SetData(&microTime, sizeof(uint64_t));
    }

    LEAVE_FUNC();
    return rildata;
}

const RilData *EmbmsDataBuilder::BuildEmbmsSaiIndicate(int nIntraNum, int nInterNum, const UINT16 *pIntraSaiList, const EMBMS_InterSaiList *pInterSaiList)
{
    ENTER_FUNC();
    RIL_SaiList saiList;

    saiList.IntraSaiListNum = nIntraNum;
    saiList.InterSaiListNum = nInterNum;
    memcpy(saiList.IntraSaiList, pIntraSaiList, sizeof(UINT16)*nIntraNum);
    memcpy(saiList.InterSaiList, pInterSaiList, sizeof(RIL_InterSaiList)*nInterNum);

    RilDataRaw *rildata = new RilDataRaw();
    if (rildata != NULL) {
        rildata->SetData(&saiList, sizeof(saiList));
    }

    LEAVE_FUNC();
    return rildata;
}

const RilData *EmbmsDataBuilder::BuildEmbmsGlobalCellIdIndicate(const char *mcc, const char *mnc, uint32_t cellId)
{
    ENTER_FUNC();
    RIL_GlobalCellIdType globalCellId;

    globalCellId.mcc = atoi(mcc);
    globalCellId.mnc = atoi(mnc);
    globalCellId.cellid = cellId;

    RilLogV("%s() mcc : %s, mnc : %s, cellid : %d", __FUNCTION__, mcc, mnc, cellId);

    RilDataRaw *rildata = new RilDataRaw();
    if (rildata != NULL) {
        rildata->SetData(&globalCellId, sizeof(globalCellId));
    }

    LEAVE_FUNC();
    return rildata;
}

const RilData *EmbmsDataBuilder::BuildEmbmsSessionControlIndicate(uint32_t controlType, int status, uint64_t uTmgi)
{
    ENTER_FUNC();
    RIL_SessionControlType sessionControl;

    sessionControl.type = controlType;
    sessionControl.status = status;
    sessionControl.tmgi = uTmgi;

    RilLogV("%s() controlType : %d, status : %d, tmgi : %012llX", __FUNCTION__, controlType, status, uTmgi);

    RilDataRaw *rildata = new RilDataRaw();
    if (rildata != NULL) {
        rildata->SetData(&sessionControl, sizeof(sessionControl));
    }

    LEAVE_FUNC();
    return rildata;
}

