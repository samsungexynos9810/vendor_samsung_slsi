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
 * protocolsmsadapter.cpp
 *
 *  Created on: 2014. 7. 3.
 *      Author: sungwoo48.choi
 */

#include "protocolsmsadapter.h"
#include "rillog.h"
#include "util.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/**
 * ProtocolSendSmsRespAdapter
 */

int ProtocolSendSmsRespAdapter::GetRef() const
{
    if (m_pModemData != NULL) {
        sit_sms_send_sms_rsp *data = (sit_sms_send_sms_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_SEND_SMS || data->hdr.id == SIT_SEND_SMS_EXPECT_MORE)) {
            return (data->msg_ref & 0xFF);
        }
    }
    return -1;
}
int ProtocolSendSmsRespAdapter::GetSmsRspErrorCode() const
{
    if (m_pModemData != NULL) {
        sit_sms_send_sms_rsp *data = (sit_sms_send_sms_rsp *) m_pModemData->GetRawData();

#if 0
        for(int i=0; i < sizeof(sit_sms_send_sms_rsp) ; i++)
        {
            RilLogV("data[%d]: %02x", i, data[i]);
        }
#endif

        if (data != NULL && (data->hdr.id == SIT_SEND_SMS || data->hdr.id == SIT_SEND_SMS_EXPECT_MORE)) {
            return data->error_code;
        }
    }

    return -1;
}
int ProtocolSendSmsRespAdapter::GetPduSize() const
{
    if (m_pModemData != NULL) {
        sit_sms_send_sms_rsp *data = (sit_sms_send_sms_rsp *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_SEND_SMS || data->hdr.id == SIT_SEND_SMS_EXPECT_MORE)) {
            return (data->ack_pdu_len & 0xFF);
        }
    }
    return 0;
}
const char *ProtocolSendSmsRespAdapter::GetPdu() const
{
    char *pdu = NULL;
    if (m_pModemData != NULL) {
        sit_sms_send_sms_rsp *data = (sit_sms_send_sms_rsp *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_SEND_SMS || data->hdr.id == SIT_SEND_SMS_EXPECT_MORE)) {
            pdu = (char *)data->ack_pdu;
            if (data->ack_pdu_len == 0 || *data->ack_pdu == 0) {
                pdu = NULL;
            }
        }
    }
    return pdu;
}

int ProtocolWriteSmsToSimRespAdapter::GetIndex() const
{
    if (m_pModemData != NULL) {
        sit_sms_write_sms_to_sim_rsp *data = (sit_sms_write_sms_to_sim_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_WRITE_SMS_TO_SIM)) {
            return (data->index);
        }
    }
    return -1;
}

int ProtocolWriteSmsToSimRespAdapter::GetIndexLen() const
{
    if (m_pModemData != NULL) {
        sit_sms_write_sms_to_sim_rsp *data = (sit_sms_write_sms_to_sim_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_NEW_SMS_ON_SIM)) {
            return (sizeof(data->index));
        }
    }
    return -1;
}

BYTE* ProtocolNewBcstSmsAdapter::GetBcst() const
{
    BYTE *msg = NULL;

    if (m_pModemData != NULL) {
        sit_sms_new_bcst_sms_ind *data = (sit_sms_new_bcst_sms_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_NEW_BCST_SMS)) {
            msg = data->bcst_msg;
            if (data->bcst_msg_len == 0) {
                msg = NULL;
            }
        }
    }
    return msg;
}

int ProtocolNewBcstSmsAdapter::GetBcstLen() const
{
    if (m_pModemData != NULL) {
        sit_sms_new_bcst_sms_ind *data = (sit_sms_new_bcst_sms_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_NEW_BCST_SMS)) {
            return (data->bcst_msg_len);
        }
    }
    return -1;
}

/**
 * ProtocolSmscAddrRespAdapter
 */
ProtocolSmscAddrRespAdapter::ProtocolSmscAddrRespAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
    m_len = 0;
    memset(m_smsc, 0x0, sizeof(m_smsc));

    Init();
}

void ProtocolSmscAddrRespAdapter::Init()
{
    if (m_pModemData != NULL) {
        sit_sms_get_smsc_addr_rsp *data = (sit_sms_get_smsc_addr_rsp *) m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_SMSC_ADDR && data->hdr.ext.rsp.error == RIL_E_SUCCESS && data->hdr.length > sizeof(RCM_HEADER))
        {
            m_len = (int)(data->sca_len & 0xFF);
            if (m_len > MAX_GSM_SMS_SERVICE_CENTER_ADDR) {
                m_len = MAX_GSM_SMS_SERVICE_CENTER_ADDR;
            }
            memcpy(m_smsc, data->sca, m_len);
        }
    }
}

const char *ProtocolSmscAddrRespAdapter::GetPdu() const
{
    if (m_len == 0) {
        return NULL;
    }
    return m_smsc;
}

int ProtocolSmscAddrRespAdapter::GetPduLength() const
{
    return m_len;
}

/**
 * ProtocolNewSmsIndAdapter
 */
ProtocolNewSmsIndAdapter::ProtocolNewSmsIndAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
    Init();

    int i = 0;
    unsigned int len = 0;

    if (m_pModemData != NULL) {
        sit_sms_new_sms_ind *data = (sit_sms_new_sms_ind *) m_pModemData->GetRawData();
        if (data != NULL && ((data->hdr.id == SIT_IND_NEW_SMS) || (data->hdr.id == SIT_IND_NEW_SMS_STATUS_REPORT))) {
            m_len = data->tpdu_len & 0xFF;
            if (m_len > 0) {
                for (i=0; i < m_len; i++) {
                    if (len < sizeof(m_tpdu)) {
                        len += snprintf(m_tpdu + len, sizeof(m_tpdu) - len - 1, "%02X", data->tpdu[i]);
                    }
                }
            }
            m_len = len;
            m_tpid = data->msg_tpid & 0xFF;
        }
    }
}

void ProtocolNewSmsIndAdapter::Init()
{
    m_len = 0;
    m_tpid = -1;
    memset(m_tpdu, 0, sizeof(m_tpdu));
}

const char *ProtocolNewSmsIndAdapter::GetPdu() const
{
    if (m_len == 0 || *m_tpdu == 0)
        return NULL;
    return m_tpdu;
}

/**
 * ProtocolGetBcstSmsConfRespAdapter
 */
ProtocolGetBcstSmsConfRespAdapter::ProtocolGetBcstSmsConfRespAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData), m_pCBcsc(NULL)
{
    if (m_pModemData != NULL) {
        sit_sms_get_bcst_sms_cfg_rsp *data =
            (sit_sms_get_bcst_sms_cfg_rsp *) m_pModemData->GetRawData();
        if (data != NULL  && (data->hdr.id == SIT_GET_BCST_SMS_CFG)) {
            int num = data->bcst_info_num;
            if (num == 0) {
                RilLogW("Number of BCST is 0.");
            } else {
                if (num > MAX_BCST_INFO_NUM) {
                    RilLogW("Number of BCST(%d) is larger than MAX_BCST_INFO_NUM(%d).",\
                            num , MAX_BCST_INFO_NUM);
                    num = MAX_BCST_INFO_NUM;
                }
                RIL_GSM_BroadcastSmsConfigInfo *rgbsci;
                rgbsci = new RIL_GSM_BroadcastSmsConfigInfo[num];
                for (int i = 0; i < num; i++) {
                    rgbsci[i].fromServiceId = data->cfgitem[i].from_svc_id;
                    rgbsci[i].toServiceId = data->cfgitem[i].to_svc_id;
                    rgbsci[i].fromCodeScheme = data->cfgitem[i].from_code_scheme;
                    rgbsci[i].toCodeScheme = data->cfgitem[i].to_code_scheme;
                    rgbsci[i].selected = data->cfgitem[i].selected;
                }
                m_pCBcsc = new BroadcastSmsConfigs(rgbsci, num);
                delete[] rgbsci;
            }
        }
    }
}

ProtocolGetBcstSmsConfRespAdapter::~ProtocolGetBcstSmsConfRespAdapter()
{
    if (m_pCBcsc) {
        delete m_pCBcsc;
    }
}

UINT8 ProtocolGetBcstSmsConfRespAdapter::GetConfigsNumber()
{
    if (m_pCBcsc) {
        return m_pCBcsc->GetConfigsNumber();
    } else {
        return 0;
    }
}

RIL_GSM_BroadcastSmsConfigInfo** ProtocolGetBcstSmsConfRespAdapter::GetConfigsInfoPointers()
{
    if (m_pCBcsc) {
        return m_pCBcsc->GetConfigsInfoPointers();
    } else {
        return NULL;
    }
}

int ProtocolSmsCapacityOnSimRespAdapter::GetSimId() const
{
    if (m_pModemData != NULL) {
        sit_sms_get_stored_sms_count_rsp *data = (sit_sms_get_stored_sms_count_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_STORED_SMS_COUNT)) {
            return (data->sim_id);
        }
    }
    // return invalid sim id
    return -1;
}

int ProtocolSmsCapacityOnSimRespAdapter::GetTotalNum() const
{
    if (m_pModemData != NULL) {
        sit_sms_get_stored_sms_count_rsp *data = (sit_sms_get_stored_sms_count_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_STORED_SMS_COUNT)) {
            return (data->total_num);
        }
    }
    return 0;
}

int ProtocolSmsCapacityOnSimRespAdapter::GetUsedNum() const
{
    if (m_pModemData != NULL) {
        sit_sms_get_stored_sms_count_rsp *data = (sit_sms_get_stored_sms_count_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_STORED_SMS_COUNT)) {
            return (data->used_num);
        }
    }
    return 0;
}

#ifdef SUPPORT_CDMA
/**
 * ProtocolCdmaSendSmsRespAdapter
 */
int ProtocolCdmaSendSmsRespAdapter::GetRef()
{
    if (m_pModemData != NULL) {
        sit_sms_cdma_send_sms_rsp *data = (sit_sms_cdma_send_sms_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_CDMA_SEND_SMS) {
            return data->msg_ref;
        }
    }
    return -1;
}
int ProtocolCdmaSendSmsRespAdapter::GetSmsRspErrorClass()
{
    if (m_pModemData != NULL) {
        sit_sms_cdma_send_sms_rsp *data = (sit_sms_cdma_send_sms_rsp *) m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_CDMA_SEND_SMS) {
            return data->error_class;
        }
    }
    return -1;
}
int ProtocolCdmaSendSmsRespAdapter::GetSmsRspCauseCode()
{
    if (m_pModemData != NULL) {
        sit_sms_cdma_send_sms_rsp *data = (sit_sms_cdma_send_sms_rsp *) m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_CDMA_SEND_SMS) {
            return data->error_code;
        }
    }
    return -1;
}

/**
 * ProtocolCdmaNewSmsIndAdapter
 */
ProtocolCdmaNewSmsIndAdapter::ProtocolCdmaNewSmsIndAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData), m_pCCsm(NULL), m_nTpId(-1)
{
    if (m_pModemData != NULL) {
        sit_sms_cdma_new_sms_ind *data = (sit_sms_cdma_new_sms_ind *) m_pModemData->GetRawData();
        if (data != NULL && data->msg_len > 0 && (data->hdr.id == SIT_IND_CDMA_NEW_SMS)) {
            m_pCCsm = new CCdmaSmsMessage((BYTE *)data->msg, data->msg_len);
            m_nTpId = data->msg_tpid;
        }
    }
}

ProtocolCdmaNewSmsIndAdapter::~ProtocolCdmaNewSmsIndAdapter()
{
    if (m_pCCsm) {
        delete m_pCCsm;
    }
}

const RIL_CDMA_SMS_Message* ProtocolCdmaNewSmsIndAdapter::GetRilCdmaSmsMsg() const
{
    if (m_pCCsm) {
        return m_pCCsm->GetRilCdmaSmsMsg();
    } else {
        return NULL;
    }
}

/**
 * ProtocolCdmaWriteSmsToRuimRespAdapter
 */
int ProtocolCdmaWriteSmsToRuimRespAdapter::GetIndex()
{
    if (m_pModemData != NULL) {
        sit_sms_cdma_write_sms_to_ruim_rsp *data =
            (sit_sms_cdma_write_sms_to_ruim_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_CDMA_WRITE_SMS_TO_RUIM)) {
            return data->index;
        }
    }
    return -1;
}

/**
 * ProtocolGetCdmaBcstSmsConfRespAdapter
 */
ProtocolGetCdmaBcstSmsConfRespAdapter::ProtocolGetCdmaBcstSmsConfRespAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData), m_pCCbcsc(NULL)
{
    if (m_pModemData != NULL) {
        sit_sms_cdma_get_bcst_sms_cfg_rsp *data =
            (sit_sms_cdma_get_bcst_sms_cfg_rsp *) m_pModemData->GetRawData();
        if (data != NULL  && (data->hdr.id == SIT_CDMA_GET_BCST_SMS_CFG)) {
            int num = data->bcst_info_num;
            if (num == 0) {
                RilLogW("Number of BCST is 0.");
            } else {
                if (num > MAX_CDMA_BCST_INFO_NUM) {
                    RilLogW("Number of BCST(%d) is larger than MAX_CDMA_BCST_INFO_NUM(%d).",\
                            num, MAX_CDMA_BCST_INFO_NUM);
                    num = MAX_CDMA_BCST_INFO_NUM;
                }
                RIL_CDMA_BroadcastSmsConfigInfo *rcbsci;
                rcbsci = new RIL_CDMA_BroadcastSmsConfigInfo[num];
                for (int i = 0; i < num; i++) {
                    rcbsci[i].service_category = data->cfgitem[i].svc_category;
                    rcbsci[i].language = data->cfgitem[i].language;
                    rcbsci[i].selected = true;
                }
                m_pCCbcsc = new CCdmaBroadcastSmsConfigs(rcbsci, num);
                delete[] rcbsci;
            }
        }
    }
}

ProtocolGetCdmaBcstSmsConfRespAdapter::~ProtocolGetCdmaBcstSmsConfRespAdapter()
{
    if (m_pCCbcsc) {
        delete m_pCCbcsc;
    }
}

UINT8 ProtocolGetCdmaBcstSmsConfRespAdapter::GetConfigsNumber()
{
    if (m_pCCbcsc) {
        return m_pCCbcsc->GetConfigsNumber();
    } else {
        return 0;
    }
}

RIL_CDMA_BroadcastSmsConfigInfo** ProtocolGetCdmaBcstSmsConfRespAdapter::GetConfigsInfoPointers()
{
    if (m_pCCbcsc) {
        return m_pCCbcsc->GetConfigsInfoPointers();
    } else {
        return NULL;
    }
}

/**
 * ProtocolCdmaVoiceMsgWaitingInfoIndAdapter
 */
ProtocolCdmaVoiceMsgWaitingInfoIndAdapter::ProtocolCdmaVoiceMsgWaitingInfoIndAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData), m_ptRcsm(NULL)
{
    if (m_pModemData != NULL) {
        sit_sms_cdma_voice_msg_waiting_info_ind *data =
                (sit_sms_cdma_voice_msg_waiting_info_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_CDMA_VOICE_MSG_WAITING_INFO)) {
            // Create RIL_CDMA_SMS_Message for MWI. Message Waiting Info Record defined in 3GPP2 C.S-0005, 3.7.5.6
            // It contains only an 8-bit number with the number of messages waiting.
            m_ptRcsm = new RIL_CDMA_SMS_Message;
            memset(m_ptRcsm, 0, sizeof(RIL_CDMA_SMS_Message));

            m_ptRcsm->uTeleserviceID = TELESERVICE_IDENTIFIER_MWI;
            m_ptRcsm->uBearerDataLen = 1;
            m_ptRcsm->aBearerData[0] = data->msg_count;
        }
    }
}

ProtocolCdmaVoiceMsgWaitingInfoIndAdapter::~ProtocolCdmaVoiceMsgWaitingInfoIndAdapter()
{
    if (m_ptRcsm) {
        delete m_ptRcsm;
    }
}
#endif // SUPPORT_CDMA
