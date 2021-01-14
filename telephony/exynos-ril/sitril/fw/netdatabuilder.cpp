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
 * netdatabuilder.cpp
 *
 *  Created on: 2014. 6. 30.
 *      Author: sungwoo48.choi
 */

#include <telephony/librilutils.h>
#include "netdatabuilder.h"
#include "operatortable.h"
#include "rilapplication.h"
#include "rillog.h"

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

const RilData *NetworkDataBuilder::BuildOperatorResponse(const char *plmn, const char *lplmn, const char *splmn)
{
    RilDataStrings *rildata = new RilDataStrings(3);
    if (rildata != NULL) {
        rildata->SetString(0, lplmn);
        rildata->SetString(1, splmn);
        rildata->SetString(2, plmn);
    }

    return rildata;
}

const RilData *NetworkDataBuilder::BuildNetSelectModeResponse(int mode)
{
    RilDataInts *rildata = new RilDataInts(1);
    if (rildata != NULL) {
        rildata->SetInt(0, mode);
    }

    return rildata;
}

const RilData *NetworkDataBuilder::BuildNetAvailableBandModeResponse(const int *bandMode, int count)
{
    RilDataInts *rildata = NULL;
    if (count > 0 && bandMode != NULL) {
        rildata = new RilDataInts(count + 1);
        if (rildata != NULL) {
            // Test IRadio.getAvailableBandModes() for the response returned.
            // Automatic mode selection must be supported
            rildata->SetInt(0, BAND_MODE_UNSPECIFIED);
            for (int i = 1; i <= count; i++) {
                rildata->SetInt(i+1, bandMode[i]);
            } // end for i
        }
    }

    return rildata;
}

const RilData *NetworkDataBuilder::BuildNetAvailableNetweorkResponse(const NetworkInfo *networks, int count)
{
    RilDataStrings *rildata = NULL;
    if (networks == NULL || count == 0) {
        rildata = new RilDataStrings(0);
    }
    else {
        rildata = new RilDataStrings(count * 4);

        for (int i = 0; i < count; i++) {
            const NetworkInfo *p = networks + i;
            rildata->SetString(i*4, p->longPlmn);
            rildata->SetString(i*4+1, p->shortPlmn);
            rildata->SetString(i*4+2, p->plmn);
            rildata->SetString(i*4+3, p->status);
        } // end for i ~
    }

    return rildata;
}

const RilData *NetworkDataBuilder::BuildNetCurrentNetworkResponse(RIL_RadioTechnology nDataRat, string strPlmn)
{
    const char *status = STR_NETWORK_STATUS_CURRENT;
    char plmn[MAX_PLMN_LEN+1] = {0,};
    const char *longPlmn = NULL;
    const char *shortPlmn = NULL;

    if ( strPlmn.length() > 0 )
    {
        int cp_len = strPlmn.length() < (MAX_PLMN_LEN + 1) ? strPlmn.length() : MAX_PLMN_LEN + 1;
        strncpy(plmn, strPlmn.c_str(), cp_len);
    }
    OperatorNameProvider *provider = OperatorNameProvider::GetInstance();
    if (provider != NULL) {
        if (provider->Contains(plmn)) {
            OperatorContentValue *opname = provider->Find(plmn);
            if (opname != NULL) {
                longPlmn = opname->GetLongPlmn();
                shortPlmn = opname->GetShortPlmn();
            }
        }
    }

    // if longPlmn(or shortPlmn) is NULL, update it to PLMN string.
    if(longPlmn == NULL || (*longPlmn) == 0) {
        longPlmn = plmn;
    }
    if(shortPlmn == NULL || (*shortPlmn) == 0) {
        shortPlmn = plmn;
    }

    RilDataStrings *rildata = new RilDataStrings(4);
    if (rildata != NULL) {
        rildata->SetString(0, longPlmn);
        rildata->SetString(1, shortPlmn);
        rildata->SetString(2, plmn);
        rildata->SetString(3, status);
        //rildata->SetString(4, nDataRat);
    }

    return rildata;
}

const RilData *NetworkDataBuilder::BuildNetFemtoCellSrchResponse(const int srch_result, const char *plmn)
{
    RilDataStrings *rildata = new RilDataStrings(2);
    if (rildata != NULL) {
        rildata->SetString(0, srch_result);
        rildata->SetString(1, plmn);
    }

    return rildata;
}

/**
 * NetworkDataAvailableNetworkListBuilder
 */
void NetworkDataAvailableNetworkListBuilder::AddNetworkInfo(const NetworkInfo &nwkInfo)
{
    // consider as same network even if RAT is different
    int size = mList.size();
    for (int i = 0; i < size; i++) {
        NetworkInfo &value = mList[i];
        if (strcmp(value.plmn, nwkInfo.plmn) == 0) {
            // Curret state shall be updated because only one RAT is current and other RATs are available.
            if (strcmp(nwkInfo.status, STR_NETWORK_STATUS_CURRENT) == 0) {
                value.status = STR_NETWORK_STATUS_CURRENT;
            }
            return ;
        }
    }
    mList.push_back(nwkInfo);
}

const RilData *NetworkDataAvailableNetworkListBuilder::Build()
{
    int size = mList.size();
    int colSize = 4;

    RilDataStrings *rildata = new RilDataStrings(size * colSize);
    for (int i = 0; i < size; i++) {
        NetworkInfo &nwkInfo = mList[i];
        rildata->SetString(i*colSize, nwkInfo.longPlmn);
        rildata->SetString(i*colSize+1, nwkInfo.shortPlmn);
        rildata->SetString(i*colSize+2, nwkInfo.plmn);
        rildata->SetString(i*colSize+3, nwkInfo.status);
    } // end for i ~
    return rildata;
}

/**
 * NetworkDataBplmnListBuilder
 */
void NetworkDataBplmnListBuilder::AddNetworkInfo(const NetworkInfo &nwkInfo)
{
    mList.push_back(nwkInfo);
}

const RilData *NetworkDataBplmnListBuilder::Build()
{
    int size = mList.size();
    int colSize = 5;

    RilDataStrings *rildata = new RilDataStrings(size * colSize);
    for (int i = 0; i < size; i++) {
        NetworkInfo &nwkInfo = mList[i];
        rildata->SetString(i*colSize, nwkInfo.longPlmn);
        rildata->SetString(i*colSize+1, nwkInfo.shortPlmn);
        rildata->SetString(i*colSize+2, nwkInfo.plmn);
        rildata->SetString(i*colSize+3, nwkInfo.status);
        rildata->SetString(i*colSize+4, nwkInfo.rat);
    } // end for i ~
    return rildata;
}

void NetworkDataUplmnListBuilder::AddPreferredPlmn(const PreferredPlmn &preferredPlmn)
{
    mList.push_back(preferredPlmn);
}

const RilData *NetworkDataUplmnListBuilder::Build(int maxNum)
{
    int size = mList.size();
    RilDataStrings *rildata = new RilDataStrings(size * 3 + 1);
    rildata->SetString(0, maxNum);
    if (rildata != NULL) {
        for (int i = 0; i < size; i++) {
            PreferredPlmn &preferredPlmn = mList[i];
            rildata->SetString(i*3+1, preferredPlmn.index);
            rildata->SetString(i*3+2, preferredPlmn.plmn);
            rildata->SetString(i*3+3, preferredPlmn.act);
        } // end for i ~
    }
    return rildata;
}

static void fillCellIdentity(RIL_CellIdentity_V1_2& cellIdentity, int cellInfoType,
        int mcc, int mnc, const char *alphaLong = NULL, const char *alphaShort = NULL)
{
    switch (cellInfoType) {
    case RIL_CELL_INFO_TYPE_GSM: {
        cellIdentity.cellIdentityGsm.mcc = mcc;
        cellIdentity.cellIdentityGsm.mnc = mnc;
        memset(cellIdentity.cellIdentityGsm.operatorNames.alphaLong,
                0, MAX_ALPHA_OPERATOR_NAME_LEN);
        if (!TextUtils::IsEmpty(alphaLong)) {
            strncpy(cellIdentity.cellIdentityGsm.operatorNames.alphaLong,
                    alphaLong, MAX_ALPHA_OPERATOR_NAME_LEN - 1);
        }

        memset(cellIdentity.cellIdentityGsm.operatorNames.alphaShort,
                0, MAX_ALPHA_OPERATOR_NAME_LEN);
        if (!TextUtils::IsEmpty(alphaShort)) {
            strncpy(cellIdentity.cellIdentityGsm.operatorNames.alphaShort,
                    alphaShort, MAX_ALPHA_OPERATOR_NAME_LEN - 1);
        }
        break;
    }
    case RIL_CELL_INFO_TYPE_WCDMA: {
        cellIdentity.cellIdentityWcdma.mcc = mcc;
        cellIdentity.cellIdentityWcdma.mnc = mnc;
        memset(cellIdentity.cellIdentityWcdma.operatorNames.alphaLong,
                0, MAX_ALPHA_OPERATOR_NAME_LEN);
        if (!TextUtils::IsEmpty(alphaLong)) {
            strncpy(cellIdentity.cellIdentityWcdma.operatorNames.alphaLong,
                    alphaLong, MAX_ALPHA_OPERATOR_NAME_LEN - 1);
        }

        memset(cellIdentity.cellIdentityWcdma.operatorNames.alphaShort,
                0, MAX_ALPHA_OPERATOR_NAME_LEN);
        if (!TextUtils::IsEmpty(alphaShort)) {
            strncpy(cellIdentity.cellIdentityWcdma.operatorNames.alphaShort,
                    alphaShort, MAX_ALPHA_OPERATOR_NAME_LEN - 1);
        }
        break;
    }
    case RIL_CELL_INFO_TYPE_LTE: {
        cellIdentity.cellIdentityLte.mcc = mcc;
        cellIdentity.cellIdentityLte.mnc = mnc;
        memset(cellIdentity.cellIdentityLte.operatorNames.alphaLong,
                0, MAX_ALPHA_OPERATOR_NAME_LEN);
        if (!TextUtils::IsEmpty(alphaLong)) {
            strncpy(cellIdentity.cellIdentityLte.operatorNames.alphaLong,
                    alphaLong, MAX_ALPHA_OPERATOR_NAME_LEN - 1);
        }

        memset(cellIdentity.cellIdentityLte.operatorNames.alphaShort,
                0, MAX_ALPHA_OPERATOR_NAME_LEN);
        if (!TextUtils::IsEmpty(alphaShort)) {
            strncpy(cellIdentity.cellIdentityLte.operatorNames.alphaShort,
                    alphaShort, MAX_ALPHA_OPERATOR_NAME_LEN - 1);
        }
        break;
    }
    case RIL_CELL_INFO_TYPE_TD_SCDMA: {
        cellIdentity.cellIdentityTdscdma.mcc = mcc;
        cellIdentity.cellIdentityTdscdma.mnc = mnc;
        memset(cellIdentity.cellIdentityTdscdma.operatorNames.alphaLong,
                0, MAX_ALPHA_OPERATOR_NAME_LEN);
        if (!TextUtils::IsEmpty(alphaLong)) {
            strncpy(cellIdentity.cellIdentityTdscdma.operatorNames.alphaLong,
                    alphaLong, MAX_ALPHA_OPERATOR_NAME_LEN - 1);
        }

        memset(cellIdentity.cellIdentityTdscdma.operatorNames.alphaShort,
                0, MAX_ALPHA_OPERATOR_NAME_LEN);
        if (!TextUtils::IsEmpty(alphaShort)) {
            strncpy(cellIdentity.cellIdentityTdscdma.operatorNames.alphaShort,
                    alphaShort, MAX_ALPHA_OPERATOR_NAME_LEN - 1);
        }
        break;
    }
    default:
        break;
    } // end switch ~
}

static void fillCdmaCellIdentity(RIL_CellIdentity_V1_2& cellIdentity, int cellInfoType,
        int basestationId, int latitude, int longitude, int systemId, int networkId)
{
    switch (cellInfoType) {
    case RIL_CELL_INFO_TYPE_CDMA: {
        cellIdentity.cellIdentityCdma.basestationId = basestationId;
        cellIdentity.cellIdentityCdma.latitude = latitude;
        cellIdentity.cellIdentityCdma.longitude = longitude;
        cellIdentity.cellIdentityCdma.systemId = systemId;
        cellIdentity.cellIdentityCdma.networkId = networkId;
        break;
    }
    default:
        break;
    }
}

static void fillCellIdentity(RIL_CellIdentity_V1_2& cellIdentity, int cellInfoType,
        int lac, int cid, int psc,
        int tac, int pcid, int eci, int channel)
{
    switch (cellInfoType) {
    case RIL_CELL_INFO_TYPE_GSM:
        cellIdentity.cellIdentityGsm.lac = lac;
        cellIdentity.cellIdentityGsm.cid = cid;
        cellIdentity.cellIdentityGsm.arfcn = channel;
        cellIdentity.cellIdentityGsm.bsic = 0xFF;
        break;
    case RIL_CELL_INFO_TYPE_WCDMA:
        cellIdentity.cellIdentityWcdma.lac = lac;
        cellIdentity.cellIdentityWcdma.cid = cid;
        cellIdentity.cellIdentityWcdma.psc = psc;
        cellIdentity.cellIdentityWcdma.uarfcn = channel;
        break;
    case RIL_CELL_INFO_TYPE_LTE:
        cellIdentity.cellIdentityLte.tac = tac;
        cellIdentity.cellIdentityLte.ci = eci;
        cellIdentity.cellIdentityLte.pci = pcid;
        cellIdentity.cellIdentityLte.earfcn = channel;
        break;
    case RIL_CELL_INFO_TYPE_TD_SCDMA:
        cellIdentity.cellIdentityTdscdma.lac = lac;
        cellIdentity.cellIdentityTdscdma.cid = cid;
        cellIdentity.cellIdentityTdscdma.cpid = INT_MAX;
        break;
    } // end switch ~
}

CellIdentityBuilder::CellIdentityBuilder()
{
    memset(&mCellIdentity, 0, sizeof(mCellIdentity));
    memset(&mCellIdentity_v16, 0, sizeof(mCellIdentity_v16));
    SetCellInfoType(RIL_CELL_INFO_TYPE_NONE);
}

CellIdentityBuilder::CellIdentityBuilder(int cellInfoType)
{
    memset(&mCellIdentity, 0, sizeof(mCellIdentity));
    memset(&mCellIdentity_v16, 0, sizeof(mCellIdentity_v16));
    SetCellInfoType(cellInfoType);
}

void CellIdentityBuilder::SetCellInfoType(int cellInfoType)
{
    mCellInfoType = cellInfoType;
    mCellIdentity.cellInfoType = (RIL_CellInfoType)cellInfoType;
}

void CellIdentityBuilder::SetCellIdentity(int mcc, int mnc,
        const char *alphaLong, const char *alphaShort)
{
    fillCellIdentity(mCellIdentity, mCellInfoType, mcc, mnc, alphaLong, alphaShort);
}

void CellIdentityBuilder::SetCellIdentity(const char *numeric,
        const char *alphaLong, const char *alphaShort)
{
    if (TextUtils::IsDigitsOnly(numeric)) {
        int mcc = NetworkUtils::fetchMcc(numeric);
        int mnc = NetworkUtils::fetchMnc(numeric);
        if (mcc == 0 || mcc == INT_MAX) {
            mnc = INT_MAX;
        }
        SetCellIdentity(mcc, mnc, alphaLong, alphaShort);
    }
}

void CellIdentityBuilder::SetCellIdentity(int lac, int cid, int psc,
        int tac, int pcid, int eci, int channel)
{
    fillCellIdentity(mCellIdentity, mCellInfoType,
            lac, cid, psc, tac, pcid, eci, channel);
}
void CellIdentityBuilder::SetCdmaCellIdentity(int basestationId, int latitude,
        int longitude, int systemId, int networkId)
{
    fillCdmaCellIdentity(mCellIdentity, mCellInfoType,
            basestationId, latitude, longitude, systemId, networkId);
}

void *CellIdentityBuilder::cellIdentity(int halVer)
{
    if (halVer < HAL_VERSION_CODE(1, 2)) {
        NetworkUtils::convertCellIdentity(mCellIdentity_v16, mCellIdentity);
        return &mCellIdentity_v16;
    }
    return &mCellIdentity;
}

void DataRegStateResultBuilder::SetRegistrationState(
        int regState, int rat, int rejectCause, int sdc)
{
    mDataRegResult.regState = (RIL_RegState)regState;
    mDataRegResult.rat = (RIL_RadioTechnology)rat;
    mDataRegResult.reasonDataDenied = rejectCause;
    mDataRegResult.maxDataCalls = sdc;
    int cellInfoType = NetworkUtils::getCellInfoTypeRadioTechnology(rat);
    mCellIdentityBuilder.SetCellInfoType(cellInfoType);
}

void DataRegStateResultBuilder::SetCellIdentity(
        int mcc, int mnc, const char *alphaLong, const char *alphaShort)
{
    mCellIdentityBuilder.SetCellIdentity(mcc, mnc, alphaLong, alphaLong);
}

void DataRegStateResultBuilder::SetCellIdentity(const char *numeric,
        const char *alphaLong, const char *alphaShort)
{
    mCellIdentityBuilder.SetCellIdentity(numeric, alphaLong, alphaShort);
}

void DataRegStateResultBuilder::SetCellIdentity(
        int lac, int cid, int psc, int tac, int pcid, int eci, int channel)
{
    mCellIdentityBuilder.SetCellIdentity(lac, cid, psc, tac, pcid, eci, channel);
}

void DataRegStateResultBuilder::SetLteVopsInfo(bool isVopsSupported,
        bool isEmcBearerSupported)
{
    mDataRegResult.lteVopsInfo.isVopsSupported = isVopsSupported;
    mDataRegResult.lteVopsInfo.isEmcBearerSupported = isEmcBearerSupported;
}

void DataRegStateResultBuilder::SetNrIndicators(bool isEndcAvailable,
        bool isDcNrRestricted, bool isNrAvailable)
{
    mDataRegResult.nrIndicators.isEndcAvailable = isEndcAvailable;
    mDataRegResult.nrIndicators.isDcNrRestricted = isDcNrRestricted;
    mDataRegResult.nrIndicators.isNrAvailable = isNrAvailable;
}

const RilData *DataRegStateResultBuilder::Build()
{
    mDataRegResult.cellIdentity =
                *((RIL_CellIdentity_V1_2 *)mCellIdentityBuilder.cellIdentity(HAL_VERSION_CODE(1, 2)));

    if (mHalVer < HAL_VERSION_CODE(1, 2)) {
        RIL_DataRegistrationStateResponse dataRegResult;
        NetworkUtils::convertDataRegistrationStateResult(dataRegResult, mDataRegResult);
        return new RilDataRaw(&dataRegResult, sizeof(RIL_DataRegistrationStateResponse));
    }
    else if (mHalVer == HAL_VERSION_CODE(1, 2)) {
        RIL_DataRegistrationStateResponse_V1_2 dataRegResult;
        NetworkUtils::convertDataRegistrationStateResult(dataRegResult, mDataRegResult);
        return new RilDataRaw(&mDataRegResult, sizeof(RIL_DataRegistrationStateResponse_V1_2));
    }

    // HAL_VERSION_CODE(1, 4) or higher
    return new RilDataRaw(&mDataRegResult, sizeof(RIL_DataRegistrationStateResponse_V1_4));
}

void VoiceRegStateResultBuilder::SetRegistrationState(int regState, int rat, int rejectCause)
{
    mVoiceRegResult.regState = (RIL_RegState)regState;
    mVoiceRegResult.rat = (RIL_RadioTechnology)rat;
    mVoiceRegResult.reasonForDenial = rejectCause;
    int cellInfoType = NetworkUtils::getCellInfoTypeRadioTechnology(rat);
    mCellIdentityBuilder.SetCellInfoType(cellInfoType);
}

void VoiceRegStateResultBuilder::SetCdmaState(int cssSupported, int roamingIndicator,
        int systemIsInPrl, int defaultRoamingIndicator)
{
    mVoiceRegResult.cssSupported = cssSupported;
    mVoiceRegResult.roamingIndicator = roamingIndicator;
    mVoiceRegResult.systemIsInPrl = systemIsInPrl;
    mVoiceRegResult.defaultRoamingIndicator = defaultRoamingIndicator;
}

void VoiceRegStateResultBuilder::SetCellIdentity(
        int mcc, int mnc, const char *alphaLong, const char *alphaShort)
{
    mCellIdentityBuilder.SetCellIdentity(mcc, mnc, alphaLong, alphaLong);
}
void VoiceRegStateResultBuilder::SetCellIdentity(const char *numeric,
        const char *alphaLong, const char *alphaShort)
{
    mCellIdentityBuilder.SetCellIdentity(numeric, alphaLong, alphaShort);
}

void VoiceRegStateResultBuilder::SetCellIdentity(
        int lac, int cid, int psc, int tac, int pcid, int eci, int channel)
{
    mCellIdentityBuilder.SetCellIdentity(lac, cid, psc, tac, pcid, eci, channel);
}

void VoiceRegStateResultBuilder::SetCdmaCellIdentity(
        int basestationId, int latitude,
        int longitude, int systemId, int networkId)
{
    mCellIdentityBuilder.SetCdmaCellIdentity(basestationId, latitude,
            longitude, systemId, networkId);
}

const RilData *VoiceRegStateResultBuilder::Build()
{
    mVoiceRegResult.cellIdentity =
                *((RIL_CellIdentity_V1_2 *)mCellIdentityBuilder.cellIdentity(HAL_VERSION_CODE(1, 2)));
    if (mHalVer < HAL_VERSION_CODE(1, 2)) {
        RIL_VoiceRegistrationStateResponse voiceRegResult;
        NetworkUtils::convertVoiceRegistrationStateResult(voiceRegResult, mVoiceRegResult);
        return new RilDataRaw(&voiceRegResult, sizeof(RIL_VoiceRegistrationStateResponse));
    }

    // HAL_VERSION_CODE(1, 2) or higher
    return new RilDataRaw(&mVoiceRegResult, sizeof(RIL_VoiceRegistrationStateResponse_V1_2));
}

/**
 * CellInfoBuilder
 */
const RilData *CellInfoListBuilder::Build(list<RIL_CellInfo_V1_4>& cellInfoList)
{
    int cellInfoSize = cellInfoList.size();
    int dataSize = 0;
    RilDataRaw *rildata = NULL;
    char *data = NULL;
    if (mHalVer < HAL_VERSION_CODE(1, 2)) {
        dataSize = sizeof(RIL_CellInfo_v12) * cellInfoSize;
    }
    else if (mHalVer == HAL_VERSION_CODE(1, 2)) {
        dataSize = sizeof(RIL_CellInfo_V1_2) * cellInfoSize;
    }
    else {
        dataSize = sizeof(RIL_CellInfo_V1_4) * cellInfoSize;
    }

    data = new char[dataSize];
    if (data != NULL) {
        memset(data, 0, dataSize);
        list<RIL_CellInfo_V1_4>::iterator iter;
        int i = 0;
        for (iter = cellInfoList.begin(); iter != cellInfoList.end(); iter++) {
            RIL_CellInfo_V1_4 &cur = *iter;
            if (mHalVer < HAL_VERSION_CODE(1, 2)) {
                NetworkUtils::convertCellInfo(((RIL_CellInfo_v12 *)data)[i], cur);
            }
            else if (mHalVer == HAL_VERSION_CODE(1, 2)) {
                NetworkUtils::convertCellInfo(((RIL_CellInfo_V1_2 *)data)[i], cur);
            }
            else {
                ((RIL_CellInfo_V1_4 *)data)[i] = cur;
            }
            i++;
        } // end iter ~
        rildata = new RilDataRaw(data, dataSize);
        delete[] data;
    }
    return rildata;
}

class NetworkScanResult : public RilData {
private:
    RIL_NetworkScanResult *mResult;
    RIL_NetworkScanResult_V1_4 *mResult_V1_4;
    unsigned int mSize;
    void *mData;
public:
    NetworkScanResult(int halVer) {
        mResult = NULL;
        mResult_V1_4 = NULL;
        mSize = 0;

        if (halVer == HAL_VERSION_CODE(1,4)) {
            mResult_V1_4 = new RIL_NetworkScanResult_V1_4;
            if (mResult_V1_4 != NULL) {
                mSize = sizeof(RIL_NetworkScanResult_V1_4);
                memset(mResult_V1_4, 0, mSize);
                mData = mResult_V1_4;
            }
        }
        else {
            mResult = new RIL_NetworkScanResult;
            if (mResult != NULL) {
                mSize = sizeof(RIL_NetworkScanResult);
                memset(mResult, 0, mSize);
                mData = mResult;
            }
        }
    }
    virtual ~NetworkScanResult() {
        if (mResult_V1_4 != NULL) {
            if (mResult_V1_4->network_infos != NULL) {
                delete[] mResult_V1_4->network_infos;
            }
            delete mResult_V1_4;
        }
        if (mResult != NULL) {
            if (mResult->network_infos != NULL) {
                delete[] mResult->network_infos;
            }
            delete mResult;
        }
    }

    void SetStatus(int status, int errorCode) {
        if (mResult_V1_4 != NULL) {
            mResult_V1_4->status = (RIL_ScanStatus)status;
            mResult_V1_4->error = (RIL_Errno)errorCode;
        }
        if (mResult != NULL) {
            mResult->status = (RIL_ScanStatus)status;
            mResult->error = (RIL_Errno)errorCode;
        }
    }

    void SetCellInfoList(RIL_CellInfo_v12 *cellInfos, unsigned int size) {
        if (mResult == NULL)
            return;

        if (cellInfos == NULL || size == 0) {
            return ;
        }

        if (mResult->network_infos != NULL) {
            delete[] mResult->network_infos;
            mResult->network_infos = NULL;
        }

        mResult->network_infos = new RIL_CellInfo_v12[size];
        if (mResult->network_infos != NULL) {
            memcpy(mResult->network_infos, cellInfos, sizeof(RIL_CellInfo_v12) * size);
            mResult->network_infos_length = size;
        }

        //RilLog("network_infos_length=%d network_infos=%p", mResult->network_infos_length, mResult->network_infos);
    }

    void SetCellInfoList(RIL_CellInfo_V1_4 *cellInfos, unsigned int size) {
        if (mResult_V1_4 == NULL)
            return;

        if (cellInfos == NULL || size == 0) {
            return ;
        }

        if (mResult_V1_4->network_infos != NULL) {
            delete[] mResult_V1_4->network_infos;
            mResult_V1_4->network_infos = NULL;
        }

        mResult_V1_4->network_infos = new RIL_CellInfo_V1_4[size];
        if (mResult_V1_4->network_infos != NULL) {
            memcpy(mResult_V1_4->network_infos, cellInfos, sizeof(RIL_CellInfo_V1_4) * size);
            mResult_V1_4->network_infos_length = size;
        }

        //RilLog("network_infos_length=%d network_infos=%p", mResult_V1_4->network_infos_length, mResult_V1_4->network_infos);
    }
    void *GetData() const { return mData; }
    unsigned int GetDataLength() const { return mSize; }
};

const RilData *NetworkScanResultBuilder::Build(int status, int error,
        list<RIL_CellInfo_V1_4>& cellInfoList)
{
    int halVer = (mHalVer == HAL_VERSION_CODE(1,4)) ? mHalVer : HAL_VERSION_CODE(1,1);
    NetworkScanResult *rildata = new NetworkScanResult(halVer);
    if (rildata != NULL) {
        rildata->SetStatus(status, error);
        CellInfoListBuilder cellInfoBuilder(halVer);
        const RilData *cellInfos = cellInfoBuilder.Build(cellInfoList);
        if (cellInfos != NULL) {
            if (mHalVer == HAL_VERSION_CODE(1,4)) {
                unsigned int size = cellInfos->GetDataLength() / sizeof(RIL_CellInfo_V1_4);
                rildata->SetCellInfoList((RIL_CellInfo_V1_4 *)cellInfos->GetData(), size);
            }
            else {
                unsigned int size = cellInfos->GetDataLength() / sizeof(RIL_CellInfo_v12);
                rildata->SetCellInfoList((RIL_CellInfo_v12 *)cellInfos->GetData(), size);
            }
            delete cellInfos;
        }
    }
    return rildata;
}

/*
 * PhysicalChannelConfigs
 */
class PhysicalChannelConfigs : public RilData {
private:
    RIL_PhysicalChannelConfig_V1_4 mConfigs_V1_4[MAX_PHYSICAL_CHANNEL_CONFIGS];
    RIL_PhysicalChannelConfig mConfigs[MAX_PHYSICAL_CHANNEL_CONFIGS];
    int mSize;
    int mDataLength;
    void *mData;
    int mHalVer;
public:
    PhysicalChannelConfigs(int halVer) : mHalVer(halVer) {
        memset(&mConfigs_V1_4, 0, sizeof(mConfigs_V1_4));
        memset(&mConfigs, 0, sizeof(mConfigs));
        mSize = 0;
        mDataLength = 0;
        mData = NULL;
    }

    virtual ~PhysicalChannelConfigs() {
        for (int i = 0; i < mSize; i++) {
            if (mConfigs_V1_4[i].contextIds != NULL) {
                delete[] mConfigs_V1_4[i].contextIds;
            }
        } // end for i ~
    }

    void SetConfigs(const RIL_PhysicalChannelConfig_V1_4 *configs, int size) {
        if (mHalVer < HAL_VERSION_CODE(1,2)) {
            return ;
        }

        if (configs == NULL || size <= 0) {
            return ;
        }

        mSize = size;
        if (mSize > MAX_PHYSICAL_CHANNEL_CONFIGS) {
            mSize = MAX_PHYSICAL_CHANNEL_CONFIGS;
        }

        for (int i = 0; i < size; i++) {
            if (mHalVer == HAL_VERSION_CODE(1,2)) {
                mData = mConfigs;
                NetworkUtils::convertPhysicalChannelConfig(mConfigs[i], configs[i]);
            }
            else {
                mData = mConfigs_V1_4;
                NetworkUtils::dupPhysicalChannelConfig(mConfigs_V1_4[i], configs[i]);
            }
        } // end for i ~

        if (mHalVer == HAL_VERSION_CODE(1,2)) {
            mData = mConfigs;
            mDataLength = (unsigned int)sizeof(RIL_PhysicalChannelConfig) * mSize;
        }
        else {
            mData = mConfigs_V1_4;
            mDataLength = (unsigned int)sizeof(RIL_PhysicalChannelConfig_V1_4) * mSize;
        }
    }

    void *GetData() const { return mData; }
    unsigned int GetDataLength() const { return mDataLength; }
};

/**
 * PhysicalChannelConfigsBuilder
 * - single PhysicalChannelConfigs(draft)
 */
PhysicalChannelConfigsBuilder::PhysicalChannelConfigsBuilder(int halVer)
{
    mHalVer = halVer;
}

const RilData* PhysicalChannelConfigsBuilder::Build(const RIL_PhysicalChannelConfig_V1_4 *configs, int size)
{
    PhysicalChannelConfigs *rildata = new PhysicalChannelConfigs(mHalVer);
    if (rildata != NULL) {
        rildata->SetConfigs(configs, size);
    }
    return rildata;
}
