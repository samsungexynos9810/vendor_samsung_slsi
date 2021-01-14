/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef    __PROTOCOL_SMS_ADAPTER_H__
#define    __PROTOCOL_SMS_ADAPTER_H__

#include "protocoladapter.h"
#include "smsdata.h"

class ProtocolSendSmsRespAdapter : public ProtocolRespAdapter {

public:
    ProtocolSendSmsRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}

public:
    int GetRef() const;
    int GetSmsRspErrorCode() const;
    int GetPduSize() const;
    const char *GetPdu() const;
};

class ProtocolWriteSmsToSimRespAdapter : public ProtocolRespAdapter {

public:
    ProtocolWriteSmsToSimRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
    virtual ~ProtocolWriteSmsToSimRespAdapter() {};

public:
    int GetIndex() const;
    int GetIndexLen() const;
};

class ProtocolSmsCapacityOnSimRespAdapter : public ProtocolRespAdapter {

public:
    ProtocolSmsCapacityOnSimRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
    virtual ~ProtocolSmsCapacityOnSimRespAdapter() {};

public:
    int GetSimId() const;
    int GetTotalNum() const;
    int GetUsedNum() const;
};

class ProtocolSmscAddrRespAdapter : public ProtocolRespAdapter {
private:
    char m_smsc[MAX_GSM_SMS_SERVICE_CENTER_ADDR*2];
    int m_len;

public:
    ProtocolSmscAddrRespAdapter(const ModemData *pModemData);
    virtual ~ProtocolSmscAddrRespAdapter() {}

protected:
    void Init();

public:
    const char *GetPdu() const;
    int GetPduLength() const;
};

class ProtocolNewSmsIndAdapter : public ProtocolIndAdapter {
private:
    char m_tpdu[MAX_GSM_SMS_TPDU_SIZE *2 + 1];
    int m_tpid;
    int m_len;
public:
    ProtocolNewSmsIndAdapter(const ModemData *pModemData);
    virtual ~ProtocolNewSmsIndAdapter() {}

protected:
    void Init();

public:
    int GetPduSize() const { return m_len; }
    const char *GetPdu() const;
    int GetTpid() const { return m_tpid; }
};

typedef ProtocolNewSmsIndAdapter ProtocolSmsStatusReportIndAdapter;

class ProtocolGetBcstSmsConfRespAdapter : public ProtocolRespAdapter
{
    private:
        BroadcastSmsConfigs *m_pCBcsc;

    public:
        ProtocolGetBcstSmsConfRespAdapter(const ModemData *pModemData);
        virtual ~ProtocolGetBcstSmsConfRespAdapter();

        UINT8 GetConfigsNumber();
        RIL_GSM_BroadcastSmsConfigInfo** GetConfigsInfoPointers();
};

class ProtocolNewBcstSmsAdapter : public ProtocolIndAdapter {
public:
    ProtocolNewBcstSmsAdapter(const ModemData *pModemData): ProtocolIndAdapter(pModemData) {}
    virtual ~ProtocolNewBcstSmsAdapter() {}

public:
    BYTE *GetBcst() const;
    int GetBcstLen() const;
};

#ifdef SUPPORT_CDMA
#include "cdmasmsdata.h"
class ProtocolCdmaSendSmsRespAdapter : public ProtocolRespAdapter
{
    public:
        ProtocolCdmaSendSmsRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
        virtual ~ProtocolCdmaSendSmsRespAdapter() {}

        int GetRef();
        int GetSmsRspErrorClass();
        int GetSmsRspCauseCode();
};

class ProtocolCdmaNewSmsIndAdapter : public ProtocolIndAdapter
{
    private:
        CCdmaSmsMessage *m_pCCsm;
        int m_nTpId;

    public:
        ProtocolCdmaNewSmsIndAdapter(const ModemData *pModemData);
        virtual ~ProtocolCdmaNewSmsIndAdapter();

        int GetMessageLength() { return sizeof(RIL_CDMA_SMS_Message); }
        const RIL_CDMA_SMS_Message* GetRilCdmaSmsMsg() const;
        int GetTpid() const { return m_nTpId; }
};

class ProtocolCdmaWriteSmsToRuimRespAdapter : public ProtocolRespAdapter
{
    public:
        ProtocolCdmaWriteSmsToRuimRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}
        virtual ~ProtocolCdmaWriteSmsToRuimRespAdapter() {}

        int GetIndex();
};

class ProtocolGetCdmaBcstSmsConfRespAdapter : public ProtocolRespAdapter
{
    private:
        CCdmaBroadcastSmsConfigs *m_pCCbcsc;

    public:
        ProtocolGetCdmaBcstSmsConfRespAdapter(const ModemData *pModemData);
        virtual ~ProtocolGetCdmaBcstSmsConfRespAdapter();

        UINT8 GetConfigsNumber();
        RIL_CDMA_BroadcastSmsConfigInfo** GetConfigsInfoPointers();
};

class ProtocolCdmaVoiceMsgWaitingInfoIndAdapter : public ProtocolIndAdapter
{
    private:
        RIL_CDMA_SMS_Message *m_ptRcsm;

    public:
        ProtocolCdmaVoiceMsgWaitingInfoIndAdapter(const ModemData *pModemData);
        virtual ~ProtocolCdmaVoiceMsgWaitingInfoIndAdapter();

        int GetMessageLength() { return sizeof(RIL_CDMA_SMS_Message); }
        const RIL_CDMA_SMS_Message* GetRilCdmaSmsMsg() const { return m_ptRcsm; }
};
#endif // SUPPORT_CDMA
#endif // __PROTOCOL_SMS_ADAPTER_H__
