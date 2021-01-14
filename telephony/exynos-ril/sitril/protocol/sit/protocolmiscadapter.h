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
 * protocolmiscadapter.h
 *
 *  Created on: 2014. 6. 30.
 *      Author: m.afzal
 */

#ifndef __PROTOCOL_MISC_ADAPTER_H__
#define __PROTOCOL_MISC_ADAPTER_H__

#include "protocoladapter.h"

class ProtocolMiscResponseAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscResponseAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
};

class ProtocolMiscVersionAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscVersionAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetMask() const;
    const char * GetSwVer() const;
    const char * GetHwVer() const;
    const char * GetRfCalDate() const;
    const char * GetProdCode() const;
    const char * GetModelID() const;
    int GetPrlNamNum() const;
    const BYTE * GetPrlVersion() const;
    int GetEriNamNum() const;
    const BYTE * GetEriVersion() const;
    const BYTE * GetCPChipSet() const;
};

class ProtocolMiscGetTtyAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscGetTtyAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetTtyMode() const;
};

class ProtocolMiscSigStrengthAdapter : public ProtocolRespAdapter {
private:
    RIL_SignalStrength_V1_4 mSignalStrength;
    RIL_SignalStrengthForVoWifi mSignalStrengthForVoWifi;
public:
    ProtocolMiscSigStrengthAdapter(const ModemData *pModemData);
public:
    RIL_SignalStrength_V1_4& GetSignalStrength();
    RIL_SignalStrengthForVoWifi& GetSignalStrengthForVowifi();
};

class ProtocolMiscSigStrengthIndAdapter : public ProtocolIndAdapter {
private:
    RIL_SignalStrength_V1_4 mSignalStrength;
    RIL_SignalStrengthForVoWifi mSignalStrengthForVoWifi;
public:
    ProtocolMiscSigStrengthIndAdapter(const ModemData *pModemData);
public:
    RIL_SignalStrength_V1_4& GetSignalStrength();
    RIL_SignalStrengthForVoWifi& GetSignalStrengthForVowifi();
};

class ProtocolMiscNITZTimeAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscNITZTimeAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) { }
public:
    int TimeInfoType() const;
    int DayLightValid() const;
    int Year() const;
    int Month() const;
    int Day() const;
    int Hour() const;
    int Minute() const;
    int Second() const;
    int TimeZone() const;
    int DayLightAdjust() const;
    int DayofWeek() const;
    int GetMMInfo() const;
    const BYTE * GetPLMN() const;

};

class ProtocolMiscIMEIAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscIMEIAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int IMEILen() const;
    const BYTE * GetIMEI() const;
};

class ProtocolMiscIMEISVAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscIMEISVAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int IMEISVLen() const;
    const BYTE * GetIMEISV() const;
};

class ProtocolMiscDeviceIDAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscDeviceIDAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int IMEILen() const;
    const BYTE * GetIMEI() const;
    int IMEISVLen() const;
    const BYTE * GetIMEISV() const;
    int MEIDLen() const;
    const BYTE * GetMEID() const;
    int ESNLen() const;
    const BYTE * GetESN() const;
};

class ProtocolMiscOemDisplayEngAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscOemDisplayEngAdapter(const ModemData *pModemData)  : ProtocolIndAdapter(pModemData){ }
};

class ProtocolMiscPhoneResetAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscPhoneResetAdapter(const ModemData *pModemData);
public:
    BYTE GetResetType();
    BYTE GetResetCause();
private:
    BYTE m_ResetType;
    BYTE m_ResetCause;
};

class ProtocolMiscDataStateChangeAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscDataStateChangeAdapter(const ModemData *pModemData);
public:
    BYTE GetExpectedState();
private:
    BYTE m_ExpectedState;
};

class ProtocolMiscNvReadItemAdapter : public ProtocolRespAdapter {
private:
#if 0   //removed
    char m_value[MAX_NV_ITEM_DATA_SIZE + 1];
#endif
public:
    ProtocolMiscNvReadItemAdapter(const ModemData *pModemData);
public:
    const char *GetValue();
};

class ProtocolMiscGetActivityInfoAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscGetActivityInfoAdapter(const ModemData *pModemData);
public:
    UINT32 GetSleepPeriod() const;
    UINT32 GetIdlePeriod() const;
    UINT32* GetTxPeriod() const;
    UINT32 GetRxPeriod() const;
};

class ProtocolMiscGetMslCodeAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscGetMslCodeAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    const char *getMslCode() const;
};

class ProtocolMiscPinControlAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscPinControlAdapter(const ModemData *pModemData);
public:
    BYTE GetSignal();
    BYTE GetStatus();
private:
    BYTE m_Signal;
    BYTE m_Status;
};

class ProtocolMiscGetPreferredCallCapability : public ProtocolRespAdapter {
public:
    ProtocolMiscGetPreferredCallCapability(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetMode() const;
};

class ProtocolMiscSetManualBandModeAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscSetManualBandModeAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    BYTE GetCause();
};

class ProtocolMiscSetRfDesenseModeAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscSetRfDesenseModeAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    BYTE GetCause();
};

class ProtocolMiscGetHwConfigAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscGetHwConfigAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetNum() const;
    int GetData(RIL_HardwareConfig *pRsp, int num) const;
};

class ProtocolMiscHwConfigChangeAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscHwConfigChangeAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) {}
public:
    int GetNum() const;
    int GetData(RIL_HardwareConfig *pRsp, int num) const;
};

class ProtocolMiscCdmaPrlChangeAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscCdmaPrlChangeAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) {}
public:
    int GetPrlVersion() const;
};

class ProtocolMiscStoreAdbSerialNumberAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscStoreAdbSerialNumberAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    BYTE GetRcmError();
};

class ProtocolMiscReadAdbSerialNumberAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscReadAdbSerialNumberAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    const char *GetAdbSerialNumber();
};

class ProtocolMiscLceAdapter : public ProtocolRespAdapter {
private:
    int m_lceStatus;
    unsigned int m_actualIntervalMs;
    unsigned int m_dlCapacityKbps;
    unsigned int m_ulCapacityKbps;
    int m_confidenceLevel;
    int m_lceSuspended;
public:
    ProtocolMiscLceAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }
    virtual void Init();
    int getLceStatus() { return m_lceStatus; }
    unsigned int getActualIntervalMs() { return m_actualIntervalMs; }
    unsigned int getDlCapacityKbps () { return m_dlCapacityKbps; }
    unsigned int getUlCapacityKbps () { return m_ulCapacityKbps; }
    int getConfidencelevel () { return m_confidenceLevel; }
    int getLceSuspended() { return m_lceSuspended; }
    RIL_Errno GetRilErrorCode() const;
};

class ProtocolMiscLceIndAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscLceIndAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) {}
public:
    int GetDLLc() const;
    int GetULLc() const;
    int GetConfLevel() const;
    int GetIsSuspended() const;
};

#ifdef SUPPORT_CDMA
class ProtocolCdmaSubscriptionAdapter:public ProtocolRespAdapter {
private:
    char m_szMdn[MAX_CDMA_MDN_LEN+1];
    char m_szMin[MAX_CDMA_MIN_LEN+1];
    WORD m_wSid;
    WORD m_wNid;
    UINT m_uPrlVersion;
public:
    ProtocolCdmaSubscriptionAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }
    virtual void Init();
    char *GetMdn() const { return (char *) m_szMdn; }
    char *GetMin() const { return (char *) m_szMin; }
    WORD GetSid() const { return m_wSid; }
    WORD GetNid() const { return m_wNid; }
    UINT GetPrlVersion() const { return m_uPrlVersion; }
};
#endif // SUPPORT_CDMA

class ProtocolMiscSarContolStateAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscSarContolStateAdapter(const ModemData *pModemData);
public:
    BYTE GetDeviceState();
private:
    BYTE m_DeviceState;
};

class ProtocolMiscSarRfConnectionAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscSarRfConnectionAdapter(const ModemData *pModemData);
public:
    BYTE GetRfState();
private:
    BYTE m_RfState;
};

class ProtocolMiscGetSarStateAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscGetSarStateAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetSarState() const;
};

class ProtocolMiscRssiScanResultAdapter : public ProtocolIndAdapter {
public:
    enum { DEFAULT_RSSI = -2040, INVALID_RSSI = 0x7FFFFFFF };

public:
    ProtocolMiscRssiScanResultAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) {}
public:
    int GetTotalPage();
    int GetCurrentPage();
    int GetStartFrequency();
    int GetEndFrequency();
    int GetStep();
    int GetScanResultSize();
    INT16* GetScanResult();
};

class ProtocolMiscATCommandAdapter : public ProtocolIndAdapter {
private:
    char *mCommand;
    unsigned int mCommandLength;
private:
    void Init();
public:
    ProtocolMiscATCommandAdapter(const ModemData *pModemData);
    virtual ~ProtocolMiscATCommandAdapter();
public:
    const char *GetCommand() const { return mCommand; }
    unsigned int GetCommandLength() const { return mCommandLength; }
};

class ProtocolMiscGetRadioNodeAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscGetRadioNodeAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    const char *GetValue() const;
};

class ProtocolMiscGetVoLteProvisionUpdateAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscGetVoLteProvisionUpdateAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetStatus();
};

class ProtocolMiscSetVoLteProvisionUpdateAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscSetVoLteProvisionUpdateAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetResult();
};

/*
 * ProtocolMiscGetStackStatusAdapter
 */
class ProtocolMiscGetStackStatusAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscGetStackStatusAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetMode();
};

/**
 * ProtocolOemModemInfoAdapter
 */
class ProtocolOemModemInfoAdapter : public ProtocolRespAdapter {
public:
    ProtocolOemModemInfoAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetCommandType() const;
    unsigned int GetSize() const;
    void *GetData();
};

/**
 * ProtocolOemModemInfoIndAdapter
 */
class ProtocolOemModemInfoIndAdapter : public ProtocolIndAdapter {
public:
    ProtocolOemModemInfoIndAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) {}
public:
    int GetCommandType() const;
    unsigned int GetSize() const;
    void *GetData();
};

/**
 * ProtocolOemSwitchModemFunctionAdapter
 */
class ProtocolOemSwitchModemFunctionAdapter : public ProtocolRespAdapter {
public:
    ProtocolOemSwitchModemFunctionAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    BYTE GetResult() const;
};

class ProtocolMiscSetSelflogAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscSetSelflogAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetSelflogResult();
};

class ProtocolMiscGetSelflogStatusAdapter : public ProtocolRespAdapter {
    public:
        ProtocolMiscGetSelflogStatusAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
    public:
        int GetSelflogStatus();
};

class ProtocolMiscSelflogStatusAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscSelflogStatusAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) {}
public:
    BYTE GetIndSelflogStatus();
};

/**
 * ProtocolOemGetCqiInfoAdapter
 */
class ProtocolOemGetCqiInfoAdapter : public ProtocolRespAdapter {
public:
    ProtocolOemGetCqiInfoAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    INT16 GetCqiType() const;
    INT16 GetCqiInfo0() const;
    INT16 GetCqiInfo1() const;
    INT16 GetRi() const;
};

/**
 * ProtocolMiscSetTcsFciAdapter
 */
class ProtocolMiscSetTcsFciAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscSetTcsFciAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetResult() const;
};

/**
 * ProtocolMiscGetTcsFciAdapter
 */
class ProtocolMiscGetTcsFciAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscGetTcsFciAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    const char * GetFci() const;
};

class ProtocolCaBandwidthFilterIndAdapter : public ProtocolIndAdapter {
public:
    ProtocolCaBandwidthFilterIndAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) {}
public:
    int GetCaConfig() const;
    int GetNRB() const;
};

/**
 * ProtocolMiscSetModemLogDumpAdapter
 */
class ProtocolMiscSetModemLogDumpAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscSetModemLogDumpAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetResult() const;
};

class ProtocolMiscCurrentLinkCapacityEstimate : public ProtocolIndAdapter {
    private:
        CURRENT_LINK_CAPA_ESTIMATE mCurLinkCapaEstimate;
    public:
        ProtocolMiscCurrentLinkCapacityEstimate(const ModemData *pModemData);
    public:
        void GetCurrentLinkCapaEstimate(void *data);
};

/**
 * ProtocolMiscEndcCapabilityIndAdapter
 */
class ProtocolMiscEndcCapabilityIndAdapter : public ProtocolIndAdapter {
public:
    ProtocolMiscEndcCapabilityIndAdapter(const ModemData *pModemData)  : ProtocolIndAdapter(pModemData) {}

public:
    int GetCapability() const;
    int GetCause() const;
};

/**
 * ProtocolMiscSetSelflogProfileAdapter
 */
class ProtocolMiscSetSelflogProfileAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscSetSelflogProfileAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetResult() const;
};

/**
 * ProtocolMiscSetForbidLteCellAdapter
 */
class ProtocolMiscSetForbidLteCellAdapter : public ProtocolRespAdapter {
public:
    ProtocolMiscSetForbidLteCellAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    int GetResult() const;
};

#endif /* __PROTOCOL_MISC_ADAPTER_H__ */
