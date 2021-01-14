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
 * protocolsimadapter.h
 *
 *  Created on: 2014. 6. 28.
 *      Author: MOX
 */

#ifndef __PROTOCOL_SIM_ADAPTER_H__
#define __PROTOCOL_SIM_ADAPTER_H__

#include "protocoladapter.h"

class ProtocolSimResponseAdapter : public ProtocolRespAdapter {
public:
    ProtocolSimResponseAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
};

class ProtocolSimStatusAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimStatusAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }

public:
    int GetCardState() const { return (int)m_tSimStatusRsp.card_state; }
    int GetUniversalPinState() const { return (int)m_tSimStatusRsp.universal_pin_state; }
    int GetApplicationCount() const { return (int)m_tSimStatusRsp.application_num; }
    // Applications Status Information
    int GetAppsType(int nIndex) const { return (int)m_tSimStatusRsp.apps_status_info[nIndex].apps_type; }
    int GetAppsState(int nIndex) const { return (int)m_tSimStatusRsp.apps_status_info[nIndex].apps_state; }
    int GetPersonalSubstate(int nIndex) const { return (int)m_tSimStatusRsp.apps_status_info[nIndex].perso_substate; }
    char *GetAID(int nIndex) const;
    int GetApplicationLabel(int nIndex, BYTE *pAppLabel) const;
    int GetPin1Replaced(int nIndex) const { return (int)m_tSimStatusRsp.apps_status_info[nIndex].pin1_replaced; }
    int GetPinState(int nIndex, int nPinIndex) const;
    int GetPinRemainCount(int nIndex, int nPinIndex) const;
    int GetPukRemainCount(int nIndex, int nPukIndex) const;
    // Extension (1.2)
    bool GetEsimNoProfile() const { return (m_tSimStatusRspExt.esim_no_profile==1); }
    int GetPhysicalSlotId() const { return (int)m_tSimStatusRspExt.physical_slot_id; }
    int GetAtrLength() const { return (int)m_tSimStatusRspExt.atr_length; }
    char *GetAtr() const { return (char *)m_tSimStatusRspExt.atr; }
    int GetIccidLength() const { return (int)m_tSimStatusRspExt.iccid_length; }
    char *GetIccid() const { return (char *)m_tSimStatusRspExt.iccid; }

    // Extension (1.4)
    int GetEidLength() const { return (int)m_tSimStatusRspExt.eid_length; }
    char *GetEid() const { return (char *)m_tSimStatusRspExt.eid; }


private:
    sit_sim_get_sim_status_rsp m_tSimStatusRsp;
    sit_sim_get_sim_status_rsp_ext m_tSimStatusRspExt;

    virtual void Init();
};

class ProtocolSimVerifyPinAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimVerifyPinAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }

private:
    int m_nPinIndex;
    int m_nRemainCount;

    virtual void Init();

public:
    int GetPinIndex() const { return m_nPinIndex; }
    int GetRemainCount() const { return m_nRemainCount; }
};

class ProtocolSimVerifyPukAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimVerifyPukAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }

private:
    int m_nPukIndex;
    int m_nRemainCount;

    virtual void Init();

public:
    int GetPukIndex() const { return m_nPukIndex; }
    int GetRemainCount() const { return m_nRemainCount; }
};

class ProtocolSimChangePinAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimChangePinAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }

private:
    int m_nPinIndex;
    int m_nRemainCount;

    virtual void Init();

public:
    int GetPinIndex() const { return m_nPinIndex; }
    int GetRemainCount() const { return m_nRemainCount; }
};

class ProtocolSimVerifyNetLockAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimVerifyNetLockAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }

    int GetRemainCount() const;
};

class ProtocolSimIOAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimIOAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }

public:
    BYTE GetSw1() const;
    BYTE GetSw2() const;
    BYTE *GetResponse() const;
    int GetResponseLength() const;
};

class ProtocolSimGetFacilityLockAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimGetFacilityLockAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }

public:
    int GetLockMode() const;
    int GetServiceClass() const;
};

class ProtocolSimSetFacilityLockAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimSetFacilityLockAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }

public:
    int GetRemainCount() const;
};

class ProtocolSimGetSimAuthAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimGetSimAuthAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }

public:
    int GetPayloadLength() const;

    int GetAuthType() const;
    int GetAuthLength() const;
    BYTE *GetAuth() const;
};

class ProtocolSimTransmitApduBasicAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimTransmitApduBasicAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }

public:
    BYTE GetSw1() const;
    BYTE GetSw2() const;
    int GetApduLength() const;
    BYTE *GetApdu() const;
};

class ProtocolSimOpenChannelAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimOpenChannelAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }

public:
    int GetSessionID() const;
    BYTE GetSw1() const;
    BYTE GetSw2() const;
    BYTE *GetResponse() const;
    int GetResponseLength() const;
};

class ProtocolSimTransmitApduChannelAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimTransmitApduChannelAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }

public:
    BYTE GetSw1() const;
    BYTE GetSw2() const;
    int GetApduLength() const;
    BYTE *GetApdu() const;
};

class ProtocolSimImsiAdapter : public ProtocolSimResponseAdapter {
private:
    char m_imsi[MAX_IMSI_LEN + 1];
public:
    ProtocolSimImsiAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }
protected:
    void Init();
public:
    const char *GetImsi() const;
};

class ProtocolSimGetGbaAuthAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimGetGbaAuthAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }
public:
    int GetGbaAuthLength() const;
    BYTE *GetGbaAuth() const;
};

class ProtocolSimATRAdapter : public ProtocolSimResponseAdapter {
private:
    BYTE m_result;
    BYTE m_atrlen;
    char m_atr[MAX_ATR_LEN];
public:
    ProtocolSimATRAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }
protected:
    void Init();
public:
    BYTE GetResult() const;
    BYTE GetATRLength() const;
    const char *GetATR() const;
};

class ProtocolSimReadPbEntry: public ProtocolSimResponseAdapter {
private:
    int m_pbType;
    int m_index;
    int m_dataLen;
    char m_entryData[MAX_PB_ENTRY_LEN];
public:
    ProtocolSimReadPbEntry(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }
    void Init();
    int GetIndex() const { return m_index; };
    int GetPbType() const { return m_pbType; };
    int GetDataLength() const { return m_dataLen; };
    char *GetEntryData() { return m_entryData; };
};

class ProtocolSimUpdatePbEntry: public ProtocolSimResponseAdapter {
private:
    BYTE m_mode;
    BYTE m_pbtype;
    BYTE m_index;
public:
    ProtocolSimUpdatePbEntry(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }
    void Init();
    BYTE GetMode() { return m_mode; };
    BYTE GetPbtype() { return m_pbtype; };
    BYTE GetIndex() { return m_index; };
};

class ProtocolSimPbStorageInfoAdapter:public ProtocolSimResponseAdapter {
private:
    int m_pbType;
    int m_totalCount;
    int m_usedCount;
public:
    ProtocolSimPbStorageInfoAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }
    void Init();
    int GetPbType() const { return m_pbType; };
    int GetTotalCount() const { return m_totalCount; };
    int GetUsedCount() const { return m_usedCount; }
};

class ProtocolSimPbStorageListAdapter:public ProtocolSimResponseAdapter {
public:
    ProtocolSimPbStorageListAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }
public:
    int GetPbList();
};

class ProtocolSimPbEntryInfoAdapter:public ProtocolSimResponseAdapter {
private:
    int m_pbType;
    int m_indexMin;
    int m_indexMax;
    int m_numMax;
    int m_textMax;
public:
    ProtocolSimPbEntryInfoAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }
    void Init();
    int GetPbType() const { return m_pbType; };
    int GetIndexMin() const { return m_indexMin; };
    int GetIndexMax() const { return m_indexMax; }
    int GetNumMax() const { return m_numMax; };
    int GetTextMax() const { return m_textMax; };
};

class ProtocolSimPbCapaAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimPbCapaAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }
    virtual ~ProtocolSimPbCapaAdapter() {}
public:
    int GetEntryNum() const;
    bool GetPbCapa(int *pb, int entryNum);
};

class ProtocolSimPbReadyAdapter:public ProtocolSimResponseAdapter {
private:
    int m_pbReady;
public:
    ProtocolSimPbReadyAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }
    void Init();
    int GetPbReady() const { return m_pbReady; };
};

class ProtocolSimIccidInfoAdapter : public ProtocolSimResponseAdapter {
private:
    int m_nIccidLen;
    BYTE m_szIccId[MAX_ICCID_LEN+1];

public:
    ProtocolSimIccidInfoAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }
    void Init();
    int GetIccIdLen() const { return m_nIccidLen; }
    const BYTE *GetIccId() const;
};

class ProtocolSimSetCarrierRestrictionsAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimSetCarrierRestrictionsAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }
    int GetLenAllowedCarriers() const;
};

class ProtocolSimGetCarrierRestrictionsAdapter : public ProtocolSimResponseAdapter {
private:
    int m_nLenAllowedCarriers;
    int m_nLenExcludedCarriers;
    void *m_pAllowedCarriers;
    void *m_pExcludedCarriers;

public:
    ProtocolSimGetCarrierRestrictionsAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { Init(); }
    void Init();
    int GetLenAllowedCarriers() const { return m_nLenAllowedCarriers; }
    int GetLenExcludedCarriers() const { return m_nLenExcludedCarriers; }
    int GetAllowedCarriers(RIL_Carrier *pCarriers, int nSize) const;
    int GetExcludedCarriers(RIL_Carrier *pCarriers, int nSize) const;
};

class ProtocolUiccSubStatusChangeAdapter : public ProtocolIndAdapter {
public:
    ProtocolUiccSubStatusChangeAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) { }
    int GetState() const;
};

class ProtocolSimLockInfoAdapter : public ProtocolRespAdapter {
public:
    ProtocolSimLockInfoAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetPolicy() const;
    int GetStatus() const;
    int GetLockType() const;
    int GetMaxRetryCount() const;
    int GetRemainCount() const;
    int GetLockCodeCount() const;
    const char *GetLockCode() const;
    int GetLockCodeSize() const;
};

/* Radio Config */
class ProtocolSimSlotStatusAdapter : public ProtocolSimResponseAdapter {
public:
    ProtocolSimSlotStatusAdapter(const ModemData *pModemData) : ProtocolSimResponseAdapter(pModemData) { }

public:
    int GetNumOfSlotStatus() const;
    int GetCardState(int phy_slotId) const;
    int GetSlotState(int phy_slotId) const;
    int GetAtrSize(int phy_slotId) const;
    char *GetAtr(int phy_slotId) const;
    int GetLogicalSlotId(int phy_slotId) const;
    int GetIccIdSize(int phy_slotId) const;
    string GetIccId(int phy_slotId) const;
    int GetEidSize(int phy_slotId) const;
    char *GetEid(int phy_slotId) const;
};

class ProtocolSlotStatusChangedAdapter : public ProtocolIndAdapter {
public:
    ProtocolSlotStatusChangedAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) { }

public:
    int GetNumOfSlotStatus() const;
    int GetCardState(int phy_slotId) const;
    int GetSlotState(int phy_slotId) const;
    int GetAtrSize(int phy_slotId) const;
    char *GetAtr(int phy_slotId) const;
    int GetLogicalSlotId(int phy_slotId) const;
    int GetIccIdSize(int phy_slotId) const;
    string GetIccId(int phy_slotId) const;
    int GetEidSize(int phy_slotId) const;
    char *GetEid(int phy_slotId) const;
};

#endif /* __PROTOCOL_SIM_ADAPTER_H__ */
