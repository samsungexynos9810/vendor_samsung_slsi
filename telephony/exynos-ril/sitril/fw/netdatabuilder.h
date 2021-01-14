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
 * netdatabuilder.h
 *
 *  Created on: 2014. 11. 24.
 *      Author: sungwoo48.choi
 */

#ifndef __NET_DATA_BUILDER_H__
#define __NET_DATA_BUILDER_H__

#include "networkutils.h"
#include "rildatabuilder.h"
#include <vector>
#include <list>

using namespace std;

int getRadioTechToCellInfoType(int rat);

/**
 * NetworkDataBuilder
 */
class NetworkDataBuilder : public RilDataBuilder {
public:
    const RilData *BuildOperatorResponse(const char *plmn, const char *lplmn, const char *splmn);
    const RilData *BuildNetSelectModeResponse(int mode);
    const RilData *BuildNetAvailableBandModeResponse(const int *bandMode, int count);
    const RilData *BuildNetAvailableNetweorkResponse(const NetworkInfo *networks, int count);
    const RilData *BuildNetCurrentNetworkResponse(RIL_RadioTechnology nDataRat, string strPlmn);
    const RilData *BuildNetFemtoCellSrchResponse(const int srch_result, const char *plmn);
};

class NetworkDataAvailableNetworkListBuilder : public RilDataBuilder {
private:
    vector<NetworkInfo> mList;

public:
    void AddNetworkInfo(const NetworkInfo &nwkInfo);
    const RilData *Build();
};

class NetworkDataBplmnListBuilder : public RilDataBuilder {
private:
    vector<NetworkInfo> mList;

public:
    void AddNetworkInfo(const NetworkInfo &nwkInfo);
    const RilData *Build();
};

/**
 * NetworkDataUplmnListBuilder
 */
class NetworkDataUplmnListBuilder : public RilDataBuilder {
private:
    vector<PreferredPlmn> mList;
public:
    void AddPreferredPlmn(const PreferredPlmn &preferredPlmn);
    const RilData *Build(int maxNum);
};

/**
 * CellIdentityBuilder
 */
class CellIdentityBuilder {
protected:
    int mCellInfoType;
private:
    RIL_CellIdentity_V1_2 mCellIdentity;
    RIL_CellIdentity_v16 mCellIdentity_v16;
public:
    CellIdentityBuilder();
    CellIdentityBuilder(int cellInfoType);
    void SetCellInfoType(int cellInfoType);
    virtual ~CellIdentityBuilder() {}
    void SetCellIdentity(int mcc, int mnc, const char *alphaLong = NULL, const char *alphaShort = NULL);
    void SetCellIdentity(const char *numeric, const char *alphaLong = NULL, const char *alphaShort = NULL);
    void SetCellIdentity(int lac, int cid, int psc, int tac, int pcid, int eci, int channel);
    void SetCdmaCellIdentity(int basestationId, int latitude,
            int longitude, int systemId, int networkId);
    void *cellIdentity(int halVer);
};

/**
 * DataRegStateResultBuilder
 */
class DataRegStateResultBuilder : public RilDataBuilder {
private:
    CellIdentityBuilder mCellIdentityBuilder;
    int mHalVer;
    RIL_DataRegistrationStateResponse_V1_4 mDataRegResult;
public:
    DataRegStateResultBuilder(int halVer = HAL_VERSION_CODE(1, 0)) {
        mHalVer = halVer;
        memset(&mDataRegResult, 0, sizeof(mDataRegResult));
    }
    void SetRegistrationState(int regState, int rat, int rejectCause, int sdc);
    void SetCellIdentity(int mcc, int mnc, const char *alphaLong = NULL, const char *alphaShort = NULL);
    void SetCellIdentity(const char *numeric, const char *alphaLong = NULL, const char *alphaShort = NULL);
    void SetCellIdentity(int lac, int cid, int psc, int tac, int pcid, int eci, int channel);
    void SetLteVopsInfo(bool isVopsSupported, bool isEmcBearerSupported);
    void SetNrIndicators(bool isEndcAvailable, bool isDcNrRestricted, bool isNrAvailable);
    const RilData *Build();
};

/**
 * VoiceRegStateResultBuilder
 */
class VoiceRegStateResultBuilder : public RilDataBuilder {
private:
    CellIdentityBuilder mCellIdentityBuilder;
    int mHalVer;
    RIL_VoiceRegistrationStateResponse_V1_2 mVoiceRegResult;
public:
    VoiceRegStateResultBuilder(int halVer = HAL_VERSION_CODE(1, 0)) {
        mHalVer = halVer;
        memset(&mVoiceRegResult, 0, sizeof(mVoiceRegResult));
    }
    void SetRegistrationState(int regState, int rat, int rejectCause);
    void SetCdmaState(int cssSupported, int roamingIndicator, int systemIsInPrl, int defaultRoamingIndicator);
    void SetCellIdentity(int mcc, int mnc, const char *alphaLong = NULL, const char *alphaShort = NULL);
    void SetCellIdentity(const char *numeric, const char *alphaLong = NULL, const char *alphaShort = NULL);
    void SetCellIdentity(int lac, int cid, int psc, int tac, int pcid, int eci, int channel);
    void SetCdmaCellIdentity(int basestationId, int latitude,
            int longitude, int systemId, int networkId);
    const RilData *Build();
};

/**
 * CellInfoBuilder
 */
class CellInfoListBuilder : public RilDataBuilder {
private:
    int mHalVer;
public:
    CellInfoListBuilder(int halVer = HAL_VERSION_CODE(1,0)) { mHalVer = halVer; }
    const RilData *Build(list<RIL_CellInfo_V1_4>& cellInfoList);
};

/**
 * NetworkScanResultBuilder
 */
class NetworkScanResultBuilder : public RilDataBuilder {
private:
    int mHalVer;
public:
    NetworkScanResultBuilder(int halVer = HAL_VERSION_CODE(1,0)) {
        mHalVer = halVer;
    }
    virtual ~NetworkScanResultBuilder() {}
    const RilData *Build(int status, int result, list<RIL_CellInfo_V1_4>& cellInfoList);
};

/**
 * PhysicalChannelConfigs
 */
class PhysicalChannelConfigsBuilder : public RilDataBuilder {
private:
    int mHalVer;
public:
    PhysicalChannelConfigsBuilder(int halVer = HAL_VERSION_CODE(1,2));
    const RilData* Build(const RIL_PhysicalChannelConfig_V1_4 *configs, int size);
};

#endif /* __NET_DATA_BUILDER_H__ */
