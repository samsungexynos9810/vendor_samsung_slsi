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
 * protocolnetadapter.h
 *
 *  Created on: 2014. 6. 25.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_NET_ADAPTER_H__
#define __PROTOCOL_NET_ADAPTER_H__

#include "protocoladapter.h"
#include <vector>
using namespace std;

class UplmnSelector;

class ProtocolNetVoiceRegStateAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetVoiceRegStateAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetRegState() const;
    int GetRejectCause() const;
    int GetRadioTech() const;
    int GetLAC() const;
    int GetCellId() const;
    int GetPSC() const;
    int GetTAC() const;
    int GetPCID() const;
    int GetECI() const;
#ifdef SUPPORT_CDMA
    int GetStationId() const;
    int GetStationLat() const;
    int GetStationLong() const;
    int GetConCurrent() const;
    int GetSystemId() const;
    int GetNetworkId() const;
    int GetRoamingInd() const;
    int GetRegPrl() const;
    int GetRoamingIndPrl() const;
#endif
    int getChannelNumber() const;
};

class ProtocolNetDataRegStateAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetDataRegStateAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetRegState() const;
    int GetRejectCause() const;
    int GetMaxSDC() const;
    int GetRadioTech() const;
    int GetLAC() const;
    int GetCellId() const;
    int GetPSC() const;
    int GetTAC() const;
    int GetPCID() const;
    int GetECI() const;
    int GetCSGID() const;
    int GetTADV() const;
    bool IsVolteServiceAvailabe() const;
    bool IsEmergencyCallServiceAvailable() const;
    int getChannelNumber() const;
    bool IsEndcAvailable() const;
    bool IsDcNrRestricted() const;
    bool IsNrAvailable() const;
};

class ProtocolNetOperatorAdapter : public ProtocolRespAdapter {
private:
    char m_szPlmn[MAX_PLMN_LEN+1];
    char m_szShortPlmn[MAX_SHORT_NAME_LEN+1];
    char m_szLongPlmn[MAX_FULL_NAME_LEN+1];
    int m_regState;
    int m_lac;
public:
    ProtocolNetOperatorAdapter(const ModemData *pModemData);
private:
    void Init();
public:
    const char *GetPlmn() const;
    const char *GetShortPlmn() const;
    const char *GetLongPlmn() const;
    int GetRegState() const;
    int GetLac() const;
};

class ProtocolNetSelModeAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetSelModeAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetNetworkSelectionMode() const;
};

class ProtocolNetRadioStateRespAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetRadioStateRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetRadioState() const;
    int ConvertRilRadioState(int radioState) const;
};

class ProtocolRadioStateAdapter : public ProtocolIndAdapter {
public:
    ProtocolRadioStateAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) { }
public:
    int GetRadioState() const;
    int ConvertRilRadioState(int radioState) const;
};

class ProtocolNetPrefNetTypeAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetPrefNetTypeAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetPreferredNetworkType() const;
};

class ProtocolNetBandModeAdapter : public ProtocolRespAdapter {
private:
    int m_bandMode[SIT_NET_BAND_MAX];
    int m_count;
public:
    ProtocolNetBandModeAdapter(const ModemData *pModemData);
protected:
    void Init();
public:
    int GetCount() const { return m_count; }
    const int * GetAvialableBandMode() const { return (m_count == 0) ? NULL : m_bandMode; }
};

class ProtocolNetAvailableNetworkAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetAvailableNetworkAdapter(const ModemData *pModemData);
    virtual ~ProtocolNetAvailableNetworkAdapter();

public:
    int GetCount();
    bool GetNetwork(NetworkInfo &nwkInfo, int index, const char *simPlmn, char *simSpn);
private:
    void UpdateNetworkNameOfNetworkScanResult(NetworkInfo &nwkInfo,
            const char *simOperatorNumeric, const char *operatorNumeric, const char *simSpn);
};


class ProtocolNetGetPsServiceAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetGetPsServiceAdapter(const ModemData *pModemData);
    virtual ~ProtocolNetGetPsServiceAdapter() {}

public:
       int GetState();
};

class ProtocolNetUplmnListAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetUplmnListAdapter(const ModemData *pModemData);
    virtual ~ProtocolNetUplmnListAdapter() {}
public:
    int GetSize() const;
    bool GetPreferrecPlmn(PreferredPlmn &preferredPlmn, int index);
};

class ProtocolNetDuplexModeRespAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetDuplexModeRespAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int Get4gDuplexMode() const;
    int Get3gDuplexMode() const;
    int GetDuplexMode() const;
};

class ProtocolNetEmergencyActInfoAdapter : public ProtocolIndAdapter {
public:
    ProtocolNetEmergencyActInfoAdapter(const ModemData *pModemData);
    virtual ~ProtocolNetEmergencyActInfoAdapter() {}
public:
    int GetRat() const;
    int GetActStatus() const;
};

class ProtocolNetMcSrchRespAdapter : public ProtocolRespAdapter {
private:
    char m_szPlmn[MAX_PLMN_LEN+1];
public:
    ProtocolNetMcSrchRespAdapter(const ModemData *pModemData);
    virtual ~ProtocolNetMcSrchRespAdapter() {}

public:
    int GetMcSrchResult() const;
    const char *GetMcSrchPlmn() const;
};

class ProtocolSetNetworkRCRespAdapter : public ProtocolRespAdapter {
public:
    ProtocolSetNetworkRCRespAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}
};

class ProtocolGetNetworkRCRespAdapter : public ProtocolRespAdapter {
public:
    ProtocolGetNetworkRCRespAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetVersion() const;
    int GetSession() const;
    int GetPhase() const;
    int GetRafType() const;
    BYTE *GetUuid() const;
    int GetStatus() const;
};

class ProtocolNetworkRCIndAdapter : public ProtocolIndAdapter {
public:
    ProtocolNetworkRCIndAdapter(const ModemData *pModemData)  : ProtocolIndAdapter(pModemData) {}

public:
    //need to implement based on SIT definition when it is available
    int GetVersion() const;
    int GetSession() const;
    int GetPhase() const;
    int GetRafType() const;
    BYTE *GetUuid() const;
    int GetStatus() const;
};

class ProtocolNetworkSgcBearerAllocIndAdapter : public ProtocolIndAdapter {
private:
    int mRat;
    int mConnectionStatus;
public:
    ProtocolNetworkSgcBearerAllocIndAdapter(const ModemData *pModemData);

public:
    int GetRat() const { return mRat; }
    int GetConnectionStatus() const { return mConnectionStatus; }
};

#ifdef SUPPORT_CDMA
class ProtocolNetCdmaQueryRoamingTypeAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetCdmaQueryRoamingTypeAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int QueryRoamingType() const;
};

class ProtocolNetCdmaHybridModeAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetCdmaHybridModeAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetCdmaHybridMode() const;
};
#endif

class ProtocolNetTotalOosAdapter : public ProtocolIndAdapter {
public:
    ProtocolNetTotalOosAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) { }
public:
    int GetCurrentPrefNetworkMode() const;
};

class ProtocolNetMccAdapter : public ProtocolIndAdapter {
private:
    char mMcc[4];
public:
    ProtocolNetMccAdapter(const ModemData *pModemData);
public:
    int GetCurrentPrefNetworkMode() const;
    const char* GetMcc() const { return mMcc; }
};

/**
 * ProtocolNetCellInfoListAdapter
 */
class ProtocolNetCellInfoListAdapter : public ProtocolRespAdapter {
private:
    list<RIL_CellInfo_V1_4> mCellInfoList;
public:
    ProtocolNetCellInfoListAdapter(const ModemData *pModemData);
    virtual ~ProtocolNetCellInfoListAdapter() {}
public:
    list<RIL_CellInfo_V1_4>& GetCellInfoList();
};

/**
 * ProtocolNetCellInfoListIndAdapter
 */
class ProtocolNetCellInfoListIndAdapter : public ProtocolIndAdapter {
private:
    list<RIL_CellInfo_V1_4> mCellInfoList;
public:
    ProtocolNetCellInfoListIndAdapter(const ModemData *pModemData);
    virtual ~ProtocolNetCellInfoListIndAdapter() {}
public:
    list<RIL_CellInfo_V1_4>& GetCellInfoList();
};

/**
 * ProtocolNetScanResultAdapter
 */
class ProtocolNetScanResultAdapter : public ProtocolIndAdapter {
private:
    list<RIL_CellInfo_V1_4> mCellInfoList;
public:
    ProtocolNetScanResultAdapter(const ModemData *pModemData);
    virtual ~ProtocolNetScanResultAdapter() {}

public:
    int GetScanStatus() const;
    int GetScanResult() const;
    list<RIL_CellInfo_V1_4>& GetCellInfoList();
};

class ProtocolNetSimFileInfoAdapter : public ProtocolIndAdapter {
private:
    int mSimFileId;
    int mRecordLen;
    int mNumRecords;
    BYTE **mppData;
public:
    ProtocolNetSimFileInfoAdapter(const ModemData *pModemData);
    virtual ~ProtocolNetSimFileInfoAdapter();

public:
    int GetSimFileId() const;
    int GetRecordLen() const;
    int GetNumOfRecords() const;
    BYTE **GetSimFileData() const;
};

class ProtocolNetPhysicalChannelConfigs : public ProtocolIndAdapter {
private:
    RIL_PhysicalChannelConfig_V1_4 mConfigs[MAX_PHYSICAL_CHANNEL_CONFIGS];
    int mSize;
private:
    void Init();
public:
    ProtocolNetPhysicalChannelConfigs(const ModemData *pModemData);
    virtual ~ProtocolNetPhysicalChannelConfigs();
    int GetSize() const { return mSize; }
    RIL_PhysicalChannelConfig_V1_4 *GetConfigs() { return mConfigs; }
};

class ProtocolNetGetManualRatModeAdapter : public ProtocolRespAdapter {
private:
    int m_manual_rat_mode_set;
    int m_rat;
public:
    ProtocolNetGetManualRatModeAdapter(const ModemData *pModemData);
public:
    void GetManualRatMode(void *data);
};

class ProtocolNetSetManualRatModeAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetSetManualRatModeAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetCause() const;
};

class ProtocolNetGetFreqLockAdapter : public ProtocolRespAdapter {
private:
    int m_freq_mode_set;
    int m_rat;
    int m_lte_pci;
    int m_lte_earfcn;
    int m_gsm_arfcn;
    int m_wcdma_psc;
    int m_wcdma_uarfcn;
public:
    ProtocolNetGetFreqLockAdapter(const ModemData *pModemData);
public:
    void GetFrequencyLock(void *data);
};

class ProtocolNetSetFreqLockAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetSetFreqLockAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetResult() const;
};

class ProtocolNetGetEndcModeAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetGetEndcModeAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetEndcMode() const;
};

class ProtocolNetworkFrequencyInfoIndAdapter : public ProtocolIndAdapter {
public:
    ProtocolNetworkFrequencyInfoIndAdapter(const ModemData *pModemData)  : ProtocolIndAdapter(pModemData) {}

public:
    int GetRat() const;
    int GetBand() const;
    int GetFrequency() const;
};

class ProtocolNetAcBarringInfo : public ProtocolIndAdapter {
private:
    AC_BARRING_INFO mAcBarringInfo;
public:
    ProtocolNetAcBarringInfo(const ModemData *pModemData);
public:
    void GetAcBarringInfo(void *data, unsigned int size);
};

class ProtocolNetGetFrequencyInfoAdapter : public ProtocolRespAdapter {
public:
    ProtocolNetGetFrequencyInfoAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetRat() const;
    int GetBand() const;
    int GetFrequency() const;
};

#endif /* __PROTOCOL_NET_ADAPTER_H__ */
