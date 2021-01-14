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
 * protocolpsadapter.h
 *
 *  Created on: 2014. 6. 25.
 *      Author: sungwoo48.choi
 */

#ifndef __PROTOCOL_PS_ADAPTER_H__
#define __PROTOCOL_PS_ADAPTER_H__

#include "protocoladapter.h"

enum {
    PDP_UNKNOWN = 0x00,
    PDP_IPV4 = 0x01,
    PDP_IPV6 = 0x02,
    PDP_IPV4V6 = 0x03,
    PDP_PPP = 0x04,
};

enum dc_type {
    DC_V10 = 0, DC_V11 = 1, DC_EXTENSION = 2, DC_RETRYTIME =3, DC_AMBR = 4,
};

static UINT dc_item_size_table[5] =
    { LEN_DC_V10, LEN_DC_V11, LEN_DC_V11_PCSCF_EXT, LEN_DC_V11_LP_RETRY, LEN_DC_V11_AMBR };

class DataCallUtil
{

};

class ProtocolPsSetupDataCallAdapter : public ProtocolRespAdapter, DataCallUtil {
private:
    DataCall m_dataCall;

public:
    ProtocolPsSetupDataCallAdapter(const ModemData *pModemData);
    virtual ~ProtocolPsSetupDataCallAdapter() {}
protected:
    void Init();

public:
    virtual UINT GetErrorCode() const;
    int GetStatus() const { return m_dataCall.status; }
    int GetCid() const { return m_dataCall.cid; }
    int GetActiveStatus() const { return m_dataCall.active; }
    int GetPdpType() const { return m_dataCall.pdpType; }
    int GetAddrInfo(PDP_ADDR *pAddr, int pdpType) const;
    int GetAddrInfo(PDP_ADDR *pAddr) const;
    int GetMTU() const { return m_dataCall.mtu_size; }
    int GetPCO() const { return m_dataCall.pco; }
    int GetSuggestedRetryTime() const { return m_dataCall.suggestedRetryTime; }
    const DataCall *GetDataCall() const { return &m_dataCall; }
};

class ProtocolPsDataCallListAdapter : public ProtocolRespAdapter, DataCallUtil {
private:
    DataCall m_dataCallList[MAX_DATA_CALL_SIZE];
    unsigned int m_dataCallNum;
public:
    ProtocolPsDataCallListAdapter(const ModemData *pModemData);
protected:
    void Init();
public:
    unsigned int GetDataCallNum() const { return m_dataCallNum; }
    const DataCall *GetDataCallList() const;
    const DataCall *GetDataCallByIndex(unsigned int index) const;
    const DataCall *GetDataCallByCid(int cid) const;
private:
    void FillDataCallListFromModemPayload(const sit_pdp_get_data_call_list_rsp *data, dc_type dct);
};

class ProtocolPsDataCallListChangedAdapter : public ProtocolIndAdapter, DataCallUtil {
private:
    DataCall m_dataCallList[MAX_DATA_CALL_SIZE];
    unsigned int m_dataCallNum;
public:
    ProtocolPsDataCallListChangedAdapter(const ModemData *pModemData);
protected:
    void Init();
public:
    unsigned int GetDataCallNum() const { return m_dataCallNum; }
    const DataCall *GetDataCallList() const;
    const DataCall *GetDataCallByIndex(unsigned int index) const;
    const DataCall *GetDataCallByCid(int cid) const;
private:
    void FillDataCallListFromModemPayload(const sit_pdp_data_call_list_changed_ind *data, dc_type dct);
};

class ProtocolPsDedicatedBearInfoAdapter : public ProtocolIndAdapter {
public:
    ProtocolPsDedicatedBearInfoAdapter(const ModemData *pModemData);
public:
    const DedicatedBearerInfo *GetDedicatedBearerInfo();
};

class ProtocolPsNasTimerStatusAdapter : public ProtocolIndAdapter {
public:
    ProtocolPsNasTimerStatusAdapter(const ModemData *pModemData);
public:
    const SitNasTimerStatus *GetNasTimerStatus();
};

class ProtocolPsStartKeepAliveAdapter : public ProtocolRespAdapter {
public:
    ProtocolPsStartKeepAliveAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
public:
    unsigned int getSessionHandle() const;
    int getCode() const;
};

class ProtocolPsKeepAliveStatusAdapter : public ProtocolIndAdapter {
public:
    ProtocolPsKeepAliveStatusAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) {}
public:
    unsigned int getSessionHandle() const;
    int getCode() const;
};


class ProtocolPsPcoDataAdapter : public ProtocolIndAdapter {
public:
    ProtocolPsPcoDataAdapter(const ModemData *pModemData);
    int remainPcoBlocks;
    int nextPcoBlockPos;
public:
    // Per CID and SIT_IND
    int GetCid() const;
    int GetPdpType() const;
    int GetPcoNum() const;

    // Per PCO Block
    int GetPcoData(sit_pdp_pco_data_entry &);
};
#endif /* __PROTOCOL_PS_ADAPTER_H__ */
