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
 * protocolcalladapter.h
 *
 *  Created on: 2014. 6. 27.
 *      Author: jhdaniel.kim
 */

#ifndef __PROTOCOL_CALL_ADAPTER_H__
#define __PROTOCOL_CALL_ADAPTER_H__

#include "protocoladapter.h"

class UusInfo;
class CallInfo;
class EmcInfo;

#define MAX_MATCH_TABLE 20
#ifdef SUPPORT_CDMA
#define SIGNAL_INFO_REC_PRESENT 1 //non-zero if signal information record is present
#endif
typedef struct
{
    int SitVal;
    int RilVal;
}sit_ril_match_item;

typedef struct
{
    int Index;
    int DefSitValue;
    int DefRilValue;
    sit_ril_match_item match_table[MAX_MATCH_TABLE];
}sit_ril_match_table;

typedef enum
{
    SIT_CALL_TB_IDX_STATE,
    SIT_CALL_TB_IDX_PRESENTATION,
    SIT_CALL_TB_IDX_UUS_TYPE,
    SIT_CALL_TB_IDX_UUS_DCS,
    SIT_CALL_TB_IDX_TYPE,
    SIT_CALL_TB_IDX_CLIR,
    //SIT_CALL_TB_IDX_RELEASECAUSE,
    SIT_CALL_TB_IDX_GET_CLIP_STATE,
    SIT_CALL_TB_IDX_SERVICE_CLASS_CF_SET,
    SIT_CALL_TB_IDX_SERVICE_CLASS_CF_GET,
    SIT_CALL_TB_IDX_SERVICE_STATUS_CALL_WAITING,
    SIT_CALL_TB_IDX_USSD_STATUS,
    SIT_CALL_TB_IDX_SSNOTI_TYPE,

    SIT_CALL_TB_IDX__MAX
}sit_call_tb_idx_e_type;

//static int ConvertSitDefineToRilDefine(int TableIndex, int SitValue);

class ProtocolGetCurrentCallAdapter : public ProtocolRespAdapter {
public:
    ProtocolGetCurrentCallAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    bool HasValidLength();
    int GetCallNum();
    int GetCallInfo(CallInfo* pCallInfo, int index);
    void DebugPrintCallInfo(CallInfo* pCallInfo);

private:
    int ConvertSitToUusInfo(UusInfo* pUusInfo, int uusType, int uus_dcs, int len, BYTE* pData);
};

class ProtocolGetLastCallFailCauseAdapter : public ProtocolRespAdapter {
public:
    ProtocolGetLastCallFailCauseAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetLastCallFailCause();
};

class ProtocolGetClipAdapter : public ProtocolRespAdapter {
public:
    ProtocolGetClipAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetClipStatus();
};

class ProtocolGetClirAdapter : public ProtocolRespAdapter {
public:
    ProtocolGetClirAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetClirStatus();
};

class ProtocolGetCallForwardingStatusAdapter : public ProtocolRespAdapter {
public:
    ProtocolGetCallForwardingStatusAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetCfNum();
    bool HasValidLength();
    int GetCfInfo(RIL_CallForwardInfo* pCfInfo, int index);
    void DebugPrintCfInfo(RIL_CallForwardInfo* pCfInfo);
};

class ProtocolGetCallWaitingAdapter : public ProtocolRespAdapter {
public:
    ProtocolGetCallWaitingAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetServiceStatus();
    int GetServiceClass();
};

class ProtocolUssdIndAdapter : public ProtocolRespAdapter {
public:
    ProtocolUssdIndAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetDecodedUssd(char* decodedUssd, size_t buf_size, int& dcs);
    int GetUssdStatus();
};

class ProtocolSsSvcIndAdapter : public ProtocolRespAdapter {
public:
    ProtocolSsSvcIndAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetNotificationType();
    int GetCode();
    int GetCugIndex();
    UINT GetSSType();
    int GetNumberLength();
    char* GetNumber();
};

class ProtocolGetColpAdapter : public ProtocolRespAdapter {
public:
    ProtocolGetColpAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetColpStatus();
};

class ProtocolGetColrAdapter : public ProtocolRespAdapter {
public:
    ProtocolGetColrAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetColrStatus();
};

class ProtocolSetCallConfirmRespAdapter : public ProtocolRespAdapter {
public:
    ProtocolSetCallConfirmRespAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetResult() const;
};

class ProtocolSendCallConfirmRespAdapter : public ProtocolRespAdapter {
public:
    ProtocolSendCallConfirmRespAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetResult() const;
};

#ifdef SUPPORT_CDMA
class ProtocolGetPreferredVoicePrivacyModeAdapter : public ProtocolRespAdapter {
public:
    ProtocolGetPreferredVoicePrivacyModeAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}
public:
    int GetPreferredVoicePrivacyStatus();
};

class ProtocolCdmaCallWaitingIndAdapter : public ProtocolRespAdapter {
public:
    ProtocolCdmaCallWaitingIndAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetCwInfo(RIL_CDMA_CallWaiting_v6 *pCwInfo);
};

class ProtocolCdmaInfoListIndAdapter : public ProtocolRespAdapter {
public:
    ProtocolCdmaInfoListIndAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    int GetCdmaInfo(RIL_CDMA_InformationRecord &cdmaInfo, int index) const;
    int GetNumberOfInfoRecs() const;
};

class ProtocolCdmaOtaProvisionStatusIndAdapter : public ProtocolIndAdapter {
public:
    ProtocolCdmaOtaProvisionStatusIndAdapter(const ModemData *pModemData)  : ProtocolIndAdapter(pModemData) {}

public:
    int GetOtaProvisionStatus() const;
};
#endif

class ProtocolEmergencyCallListIndAdapter : public ProtocolRespAdapter {
private:
    char m_mcc[MAX_MCC_LEN + 1];
    char m_mnc[MAX_MNC_LEN + 1];
public:
    ProtocolEmergencyCallListIndAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}

public:
    bool HasValidLength();
    const char* GetMcc();
    const char* GetMnc();
    int GetNum() const;
    bool GetEmcInfo(EmcInfo &emcInfo, int idx) const;
};

class ProtocolEmergencySupportRatModeIndAdapter : public ProtocolIndAdapter {
public:
    ProtocolEmergencySupportRatModeIndAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) {}

public:
    int GetSupportRatMode() const;
};

class ProtocolExitEmergencyCbModeRespAdapter : public ProtocolRespAdapter {
public:
    ProtocolExitEmergencyCbModeRespAdapter(const ModemData *pModemData)  : ProtocolRespAdapter(pModemData) {}

public:
    bool GetResult() const;
    RIL_Errno GetRilErrorCode() const;
};

class ProtocolUnsolOnSSAdapter : public ProtocolIndAdapter {
public:
    ProtocolUnsolOnSSAdapter(const ModemData *pModemData)  : ProtocolIndAdapter(pModemData) {}

public:
    int GetServiceType() const;
    int GetRequestType() const;
    int GetTeleServiceType() const;
    int GetServiceClass() const;
    int GetResult() const;
    int GetDataType() const;
    bool GetData(void *pData) const;
};
#endif /* __PROTOCOL_CALL_ADAPTER_H__ */
