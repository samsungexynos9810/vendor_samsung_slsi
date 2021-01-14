/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __PROTOCOL_EMBMS_ADAPTER_H__
#define __PROTOCOL_EMBMS_ADAPTER_H__

#include "protocoladapter.h"

class ProtocolEmbmsCoverageAdapter : public ProtocolRespAdapter {
public:
    ProtocolEmbmsCoverageAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }
public:
    int GetCoverage() const;
};

class ProtocolEmbmsSessionListAdapter : public ProtocolRespAdapter {
public:
    ProtocolEmbmsSessionListAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }

private:
    int m_state;
    int m_oos_reason;
    int m_record_num;
    BYTE m_tmgi[EMBMS_MAX_INTRA_SAILIST_NUMBER*EMBMS_TMGI_LEN];

    virtual void Init();

public:
    int GetState() const { return m_state; }
    int GetOosReason() const { return m_oos_reason; }
    int GetRecordNum() const { return m_record_num; }
    const BYTE *GetTMGI() const;
};

class ProtocolEmbmsSessionListIndAdapter : public ProtocolRespAdapter {
public:
    ProtocolEmbmsSessionListIndAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }

private:
    int m_state;
    int m_oos_reason;
    int m_record_num;
    BYTE m_tmgi[EMBMS_MAX_INTRA_SAILIST_NUMBER*EMBMS_TMGI_LEN];

    virtual void Init();

public:
    int GetState() const { return m_state; }
    int GetOosReason() const { return m_oos_reason; }
    int GetRecordNum() const { return m_record_num; }
    const BYTE *GetTMGI() const;
};

class ProtocolEmbmsNetworkTimeAdapter : public ProtocolRespAdapter {
public:
    ProtocolEmbmsNetworkTimeAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}

public:
    uint64_t GetNetworkTime() const;
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
};

class ProtocolEmbmsNetworkTimeIndAdapter : public ProtocolRespAdapter {
public:
    ProtocolEmbmsNetworkTimeIndAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}

public:
    uint64_t GetNetworkTime() const;
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
};

class ProtocolEmbmsSignalStrengthAdapter : public ProtocolRespAdapter {
public:
    ProtocolEmbmsSignalStrengthAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init();}

private:
    int m_number_record;
    UINT32 m_arrSnrList[8];
    virtual void Init();

public:
    int GetCount() const { return m_number_record; }
    const UINT32 *GetSnrList() const;
};

class ProtocolEmbmsSignalStrengthIndAdapter : public ProtocolRespAdapter {
public:
    ProtocolEmbmsSignalStrengthIndAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init();}

private:
    int m_number_record;
    UINT32 m_arrSnrList[8];
    virtual void Init();

public:
    int GetCount() const { return m_number_record; }
    const UINT32 *GetSnrList() const;
};


class ProtocolEmbmsSaiListAdapter : public ProtocolRespAdapter {
public:
    ProtocolEmbmsSaiListAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }

private:
    EMBMS_SaiList m_saiList;

    virtual void Init();
public:
    int GetIntraSaiListLen() const {return m_saiList.IntraSaiListNum;}
    int GetInterSaiListLen() const {return m_saiList.InterSaiListNum;}
    const UINT16 *GetIntraSaiList() const;
    const EMBMS_InterSaiList *GetInterSaiList() const;
};

class ProtocolEmbmsGlobalCellIdAdapter : public ProtocolRespAdapter {
public:
    ProtocolEmbmsGlobalCellIdAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }

private:
    char m_mcc[MAX_MCC_LEN+1];
    char m_mnc[MAX_MNC_LEN+1];
    UINT32 m_cellId;

    virtual void Init();

public:
    const char* GetMcc() const;
    const char* GetMnc() const;
    UINT32 GetCellId() const;
};

#endif /* __PROTOCOL_EMBMS_ADAPTER_H__ */
