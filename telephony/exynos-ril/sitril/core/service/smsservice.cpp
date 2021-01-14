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
 * smsservice.cpp
 *
 *  Created on: 2014. 7. 21.
 *      Author: sungwoo48.choi
 */
#include "smsservice.h"
#include "rilcontext.h"
#include "modemdata.h"
#include "servicemonitorrunnable.h"
#include "servicemgr.h"
#include "rillog.h"
#include "protocolsmsbuilder.h"
#include "protocolsmsadapter.h"
#include "smsdata.h"
#include "callreqdata.h"
#include "smsdatabuilder.h"
#include "simservice.h"
#include "util.h"
#include "sms_util.h"
#include "protocolimsbuilder.h"

BYTE g_nLastTpidNewSms = 0;

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define writeRilEvent(format1, format2, ...)   CRilEventLog::writeRilEvent(RIL_LOG_CAT_SMS, format1, format2, ##__VA_ARGS__)

SmsService::SmsService(RilContext* pRilContext) : Service(pRilContext, RIL_SERVICE_SMS)
{
    memset(m_sca, 0, sizeof(m_sca));
    m_scaLen = 0;

    memset(m_index, 0x00, sizeof(m_index));
    m_smsclass= 0;
    m_simsms_full = 0;
    m_nLastTpid = 0;
}
SmsService::~SmsService()
{

}

int SmsService::OnErrorRequestComplete(int errorCode)
{
    RilLogI("%s::%s()", m_szSvcName, __FUNCTION__);
    int nResult = errorCode;

    switch(errorCode)
    {
    case RIL_E_RADIO_NOT_AVAILABLE:
        RilLogV("%s::%s() RIL_E_RADIO_NOT_AVAILABLE(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_GENERIC_FAILURE:
        RilLogV("%s::%s() RIL_E_GENERIC_FAILURE(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        nResult = errorCode;
        break;
    case RIL_E_PASSWORD_INCORRECT:
        RilLogV("%s::%s() RIL_E_PASSWORD_INCORRECT(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_SIM_PIN2:
        RilLogV("%s::%s() RIL_E_SIM_PIN2(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_SIM_PUK2:
        RilLogV("RIL_E_SIM_PUK2(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_REQUEST_NOT_SUPPORTED:
        RilLogV("%s::%s() RIL_E_REQUEST_NOT_SUPPORTED(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_CANCELLED:
        RilLogV("%s::%s() RIL_E_CANCELLED(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL:
        RilLogV("%s::%s() RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW:
        RilLogV("%s::%s() RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_SIM_ABSENT:
        RilLogV("%s::%s() RIL_E_SIM_ABSENT(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_SUBSCRIPTION_NOT_AVAILABLE:
        RilLogV("%s::%s() RIL_E_SUBSCRIPTION_NOT_AVAILABLE(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_MODE_NOT_SUPPORTED:
        RilLogV("%s::%s() RIL_E_MODE_NOT_SUPPORTED(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_FDN_CHECK_FAILURE:
        RilLogV("%s::%s() RIL_E_FDN_CHECK_FAILURE(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_ILLEGAL_SIM_OR_ME:
        RilLogV("%s::%s() RIL_E_ILLEGAL_SIM_OR_ME(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_MISSING_RESOURCE:
        RilLogV("%s::%s() RIL_E_MISSING_RESOURCE(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    case RIL_E_NO_SUCH_ELEMENT:
        RilLogV("%s::%s() RIL_E_NO_SUCH_ELEMENT(%d=0x%02X)", m_szSvcName, __FUNCTION__, errorCode, errorCode);
        break;
    default:
        RilLogV("Error code is not predefined!");
        break;
    }

    return OnRequestComplete(nResult, NULL, 0);
}

bool SmsService::IsPossibleToPassInRadioOffState(int request_id)
{
    switch (request_id) {
        case RIL_REQUEST_OEM_AIMS_SEND_SMS:
        case RIL_REQUEST_OEM_AIMS_SEND_EXPECT_MORE:
        case RIL_REQUEST_GET_SMSC_ADDRESS:
        case RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG:
        case RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION:
#ifdef SUPPORT_CDMA
        case RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG:
        case RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION:
#endif // SUPPORT_CDMA
            break;
        default:
            return false;
    }
    return true;
}

int SmsService::OnCreate(RilContext *pRilContext)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    return 0;
}

BOOL SmsService::OnHandleRequest(Message* pMsg)
{
    int ret = -1;
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_SMS_SEND:
        ret = DoSendSms(pMsg, FALSE);
        break;
    case MSG_SMS_SEND_MORE:
        ret = DoSendSms(pMsg, TRUE);
        break;
    case MSG_SMS_ACKNOWLEDGE:
        ret = DoSmsAck(pMsg);
        break;
    case MSG_SMS_WRITE_SMS_TO_SIM:
        ret = DoWriteSmsToSim(pMsg);
        break;
    case MSG_SMS_DELETE_SMS_ON_SIM:
        ret = DoDeleteSmsOnSim(pMsg);
        break;
    case MSG_SMS_GET_BROADCAST_SMS_CONFIG:
        ret = DoGetBroadcastSmsConfig(pMsg);
        break;
    case MSG_SMS_SET_BROADCAST_SMS_CONFIG:
        ret = DoSetBroadcastSmsConfig(pMsg);
        break;
    case MSG_SMS_BROADCAST_ACTIVATION:
        ret = DoSmsBroadcastActivation(pMsg);
        break;
    case MSG_SMS_GET_SMSC_ADDRESS:
        ret = DoGetSmscAddress(pMsg);
        break;
    case MSG_SMS_SET_SMSC_ADDRESS:
        ret = DoSetSmscAddress(pMsg);
        break;
    case MSG_SMS_REPORT_SMS_MEMORY_STATUS:
        ret = DoReportSmsMemoryStatus(pMsg);
        break;
    case MSG_SMS_ACK_WITH_PDU:
        ret = DoSmsAckWithPdu(pMsg);
        break;
#ifdef SUPPORT_CDMA
    case MSG_SMS_CDMA_SEND:
        ret = DoSendCdmaSms(pMsg);
        break;
    case MSG_SMS_CDMA_WRITE_SMS_TO_RUIM:
        ret = DoWriteCdmaSmsToRuim(pMsg);
        break;
    case MSG_SMS_CDMA_DELETE_SMS_ON_RUIM:
        ret = DoDeleteCdmaSmsOnRuim(pMsg);
        break;
    case MSG_SMS_CDMA_GET_BROADCAST_SMS_CONFIG:
        ret = DoGetCdmaBroadcastSmsConfig(pMsg);
        break;
    case MSG_SMS_CDMA_SET_BROADCAST_SMS_CONFIG:
        ret = DoSetCdmaBroadcastSmsConfig(pMsg);
        break;
    case MSG_SMS_CDMA_BROADCAST_ACTIVATION:
        ret = DoSmsCdmaBroadcastActivation(pMsg);
        break;
#endif //SUPPORT_CDMA
    case MSG_SMS_AIMS_SEND_SMS:
    case MSG_SMS_AIMS_SEND_EXPECT_MORE:
        ret = DoSendAimsSms(pMsg);
        break;
    case MSG_SMS_GET_STORAGE_CAPACITY:
        ret = DoGetSmsCapacityOnSim(pMsg);
        break;
    default:
        return FALSE;
    }
    return (ret >= 0)? TRUE : FALSE;
}

BOOL SmsService::OnHandleSolicitedResponse(Message* pMsg)
{
    int ret = -1;
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_SMS_SEND_DONE:
        ret = OnSendSmsDone(pMsg);
        break;
    case MSG_SMS_ACKNOWLEDGE_DONE:
        ret = OnSmsAckDone(pMsg);
        break;
    case MSG_SMS_WRITE_SMS_TO_SIM_DONE:
        ret = OnWriteSmsToSimDone(pMsg);
        break;
    case MSG_SMS_DELETE_SMS_ON_SIM_DONE:
        ret = OnDeleteSmsOnSimDone(pMsg);
        break;
    case MSG_SMS_GET_BROADCAST_SMS_CONFIG_DONE:
        ret = OnGetBroadcastSmsConfigDone(pMsg);
        break;
    case MSG_SMS_SET_BROADCAST_SMS_CONFIG_DONE:
        ret = OnSetBroadcastSmsConfigDone(pMsg);
        break;
    case MSG_SMS_BROADCAST_ACTIVATION_DONE:
        ret = OnSmsBroadcastActivationDone(pMsg);
        break;
    case MSG_SMS_GET_SMSC_ADDRESS_DONE:
        ret = OnGetSmscAddressDone(pMsg);
        break;
    case MSG_SMS_SET_SMSC_ADDRESS_DONE:
        ret = OnSetSmscAddressDone(pMsg);
        break;
    case MSG_SMS_REPORT_SMS_MEMORY_STATUS_DONE:
        ret = OnReportSmsMemoryStatusDone(pMsg);
        break;
    case MSG_SMS_ACK_WITH_PDU_DONE:
        ret = OnSmsAckWithPduDone(pMsg);
        break;
#ifdef SUPPORT_CDMA
    case MSG_SMS_CDMA_SEND_DONE:
        ret = OnSendCdmaSmsDone(pMsg);
        break;
    case MSG_SMS_CDMA_WRITE_SMS_TO_RUIM_DONE:
        ret = OnWriteCdmaSmsToRuimDone(pMsg);
        break;
    case MSG_SMS_CDMA_DELETE_SMS_ON_RUIM_DONE:
        ret = OnDeleteCdmaSmsOnRuimDone(pMsg);
        break;
    case MSG_SMS_CDMA_GET_BROADCAST_SMS_CONFIG_DONE:
        ret = OnGetCdmaBroadcastSmsConfigDone(pMsg);
        break;
    case MSG_SMS_CDMA_SET_BROADCAST_SMS_CONFIG_DONE:
        ret = OnSetCdmaBroadcastSmsConfigDone(pMsg);
        break;
    case MSG_SMS_CDMA_BROADCAST_ACTIVATION_DONE:
        ret = OnSmsCdmaBroadcastActivationDone(pMsg);
        break;
#endif //SUPPORT_CDMA
    case MSG_SMS_AIMS_SEND_SMS_DONE:
    case MSG_SMS_AIMS_SEND_EXPECT_MORE_DONE:
        ret = OnSendAimsSmsDone(pMsg);
        break;
    case MSG_SMS_GET_STORAGE_CAPACITY_DONE:
        ret = OnGetSmsCapacityOnSimDone(pMsg);
        break;
    default:
        return FALSE;
    }
    return (ret >= 0)? TRUE : FALSE;
}

BOOL SmsService::OnHandleUnsolicitedResponse(Message* pMsg)
{
    int ret = -1;
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_SMS_INCOMING_NEW_SMS:
        ret = OnIncomingNewSms(pMsg);
        break;
    case MSG_SMS_INCOMING_NEW_SMS_STATUS_REPORT:
        ret = OnIncomingNewSmsStatusReport(pMsg);
        break;
    case MSG_SMS_INCOMING_NEW_SMS_ON_SIM:
        ret = OnIncomingNewSmsOnSim(pMsg);
        break;
    case MSG_SMS_SIM_SMS_STORAGE_FULL:
        ret = OnSimSmsStorageFull(pMsg);
        break;
    case MSG_SMS_INCOMING_NEW_BROADCAST_SMS:
        ret = OnIncomingNewBroadcastSms(pMsg);
        break;
#ifdef SUPPORT_CDMA
    case MSG_SMS_CDMA_INCOMING_NEW_SMS:
        ret = OnIncomingNewCdmaSms(pMsg);
        break;
    case MSG_SMS_CDMA_RUIM_SMS_STORAGE_FULL:
        ret = OnRuimSmsStorageFull(pMsg);
        break;
    case MSG_SMS_CDMA_VOICE_MSG_WAITING_INFO:
        ret = OnVoiceMsgWaitingInfo(pMsg);
        break;
    default:
#endif //SUPPORT_CDMA
        return FALSE;
    }
    return (ret >= 0)? TRUE : FALSE;
}

void SmsService::OnSimStatusChanged(int cardState, int appState)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if ((cardState == RIL_CARDSTATE_PRESENT) && (appState == RIL_APPSTATE_READY)) {
        RequestData *pData = new RequestData(RIL_REQUEST_GET_SMSC_ADDRESS, 0);

        if (pData != NULL) {
            Message *msg = Message::ObtainMessage(pData, RIL_SERVICE_SMS, MSG_SMS_GET_SMSC_ADDRESS);

            if (m_pRilContext->GetServiceManager()->SendMessage(msg) < 0) {
                if (msg) {
                    delete msg;
                }
            }
        }
    }
}


/*
    MMS configuration for SKT LTE
=============================================
    APN -------  lte.sktelecom.com
    MMSC ------ http://omms.nate.com:9082/oma_mms
    MMS Port ---  9093
=============================================
    MMS Proxy -- Not Set
    MCC ------- 450
    MNC ------- 05
=============================================
*/
int SmsService::DoSendSms(Message *pMsg, bool bExpectMore)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    const char *smsc;
    BYTE smscLen;

    if (pMsg == NULL) {
        RilLogE("%s() pMsg is NULL", __FUNCTION__);
        return -1;
    }

    GsmSmsMessage *rildata = (GsmSmsMessage *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("%s() rildata is NULL", __FUNCTION__);
        return -1;
    }

    if (rildata->mSmsc->UseRilSmsc() && m_scaLen != 0)
    {
        // len:09
        // data:08  91  28 01  00  99  01  02  89
        smsc = m_sca;
        smscLen = m_scaLen;
    }
    else
    {
        //rildata->mSmsc = "test smsc";
        smsc = rildata->mSmsc->mAddr;
        smscLen = rildata->mSmsc->mLen;
    }

    const char *pdu = rildata->mPdu->mData;
    int pduSize = rildata->mPdu->mLen;

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSendSms(smsc, smscLen, pdu, pduSize, bExpectMore);
    //writeRilEvent(m_szSvcName, __FUNCTION__, "smsc(%s), smscLen(0x%x), pdu(%s), pduSize(%d), bExpectMore(%s)", smsc, smscLen, pdu, pduSize, bExpectMore == 1? "true" : "false");

    if (SendRequest(pModemData, SMS_SEND_TIMEOUT, MSG_SMS_SEND_DONE) < 0) {
        RilLogE("[%s] %s() - Error", m_szSvcName, __FUNCTION__);
        return -1;
    }

    return 0;
}

int SmsService::OnSendSmsDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    const ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolSendSmsRespAdapter adapter(pModemData);
    int error = adapter.GetErrorCode();
    int tp_errcause = adapter.GetSmsRspErrorCode();
    int messageRef = adapter.GetRef();
    //const char *ackPdu = adapter.GetPdu();    //not used

    RIL_SMS_Response resp;
    resp.ackPDU = NULL;
    resp.messageRef = messageRef;
    resp.errorCode = tp_errcause;        //resp.errorCode = 0; error_code was -1 despite a successful response --> It's changed to '0' for success case now, but it seems to be whatever...

    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%d), tp_errcause(%d), messageRef(%d)", error, tp_errcause, messageRef);
    if (error == RIL_E_SUCCESS) {
        RilLogV("%s::%s() RIL_E_SUCCESS(%d=0x%02X)", m_szSvcName, __FUNCTION__, tp_errcause, tp_errcause);
        if(tp_errcause == 0){
            OnRequestComplete(RIL_E_SUCCESS, &resp, sizeof(RIL_SMS_Response));
        }
        else
        {
            if (tp_errcause == 42) { // 42(0x2a): RP_CONGESTION
                OnRequestComplete(RIL_E_SMS_SEND_FAIL_RETRY, &resp, sizeof(RIL_SMS_Response));
            } else {
                OnRequestComplete(RIL_E_GENERIC_FAILURE, &resp, sizeof(RIL_SMS_Response));
            }
        }
    }
    else if (error == RIL_E_SMS_SEND_FAIL_RETRY) {
        RilLogV("%s::%s() RCM_E_SMS_SEND_FAIL_RETRY(%d=0x%02X)", m_szSvcName, __FUNCTION__, tp_errcause, tp_errcause);
        OnRequestComplete(RIL_E_SMS_SEND_FAIL_RETRY, &resp, sizeof(RIL_SMS_Response));
    }
    else {
        OnErrorRequestComplete(error);
    }
    return 0;
}

int SmsService::OnIncomingNewSmsStatusReport(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    const ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolNewSmsIndAdapter adapter(pModemData);
    m_nLastTpid = adapter.GetTpid();
    g_nLastTpidNewSms = m_nLastTpid;

    OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT, adapter.GetPdu(), adapter.GetPduSize());
    return 0;
}

int SmsService::DoSmsAck(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    const SmsAcknowledge *rildata = (const SmsAcknowledge *)pMsg->GetRequestData();
    if (rildata == NULL) {
        return -1;
    }

    int result = rildata->mResult;
    int failcause = rildata->mFailureCause;

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSmsAck(result, m_nLastTpid, failcause);
    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_SMS_ACKNOWLEDGE_DONE) < 0 ) {
        RilLogE("[%s] %s() : Failed(%d)", m_szSvcName, __FUNCTION__, failcause);
        return -1;
    }
    return 0;
}

int SmsService::OnSmsAckDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);

    return 0;
}

int SmsService::DoWriteSmsToSim(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL)
    {
        RilLogE("%s() pMsg is NULL", __FUNCTION__);
        return -1;
    }

    SimSmsMessage* data= (SimSmsMessage *)pMsg->GetRequestData();
    if (data == NULL) {
        RilLogE("%s() data is NULL", __FUNCTION__);
        return -1;
    }

    int pduSize = data->GetLength();

    char *pdu = NULL;
    pdu = new char[pduSize];
    memcpy(&pdu[0], data->GetRawByte(), pduSize);

    if (SMS_DBG)
    {
        char *pdu_tmp = NULL;
        pdu_tmp = new char[(pduSize*2)+1];
        Value2HexString(pdu_tmp, (BYTE *)pdu, pduSize);
        RilLogV("[8] PDU (ALL) = %s, pdusize=%d", pdu_tmp, pduSize);
        delete [] pdu_tmp;
    }

    int status = data->mStatus;        // Needs to be confirmed
    int index = data->mSimIndex;    //  0xFFFF: Default value all the time, except for a test.

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildWriteSmsToSim(status, index, pduSize, pdu);

    delete [] pdu;

    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_SMS_WRITE_SMS_TO_SIM_DONE) < 0) {
        RilLogE("[%s] %s() : Error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int SmsService::OnWriteSmsToSimDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        return -1;
    }

    const ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }
    ProtocolWriteSmsToSimRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    m_index[0] = adapter.GetIndex();

    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(errorCode, m_index, sizeof(int));
    }
    else
    {
        OnErrorRequestComplete(errorCode);
    }

    if (m_smsclass == MESSAGE_CLASS_2)
    {
        if (errorCode == RIL_E_SUCCESS)
        {
            // RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM issuing to F/W
            OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM, m_index, sizeof(int));
        } else {
            // SEND_SMS_ACK issuing to Network
            int data[2];
            data[0] = FAILED_RECEIPT;
            data[1] = TP_CAUSE_UNSPECIFIED_ERROR;
            RequestData *req = RilParser::CreateSmsAck(RIL_REQUEST_SMS_ACKNOWLEDGE, 0, (char *)(&data[0]), (sizeof(data[0])*2));
            if (req != NULL)
            {
                Message *msg = Message::ObtainMessage(req, RIL_SERVICE_MISC, MSG_MISC_SMS_ACKNOWLEDGE);
                if (m_pRilContext->GetServiceManager()->SendMessage(msg) < 0)
                {
                    delete msg;
                }
            }
        }
        // Reset the class variable so that next write to sim operation will not be effected
        m_smsclass = MESSAGE_CLASS_1;
    }

    return 0;
}

int SmsService::DoDeleteSmsOnSim(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL)
    {
        return -1;
    }

    IntRequestData* data= (IntRequestData *)pMsg->GetRequestData();
    int index = data->GetInt();

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildDeleteSmsOnSim(index);

    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_SMS_DELETE_SMS_ON_SIM_DONE) < 0) {
        RilLogE("[%s] %s() : Error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int SmsService::OnDeleteSmsOnSimDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
            RilLogE("pModemData is NULL");
            return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);
    return 0;
}

int SmsService::DoGetSmsCapacityOnSim(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL)
    {
        return -1;
    }

    IntRequestData* data= (IntRequestData *)pMsg->GetRequestData();
    int simId = data->GetInt();

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildGetStoredSmsCount(simId);

    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_SMS_GET_STORAGE_CAPACITY_DONE) < 0) {
        RilLogE("[%s] %s() : Error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int SmsService::OnGetSmsCapacityOnSimDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        return -1;
    }

    const ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolSmsCapacityOnSimRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    RIL_StorageStatus resp;
    resp.sim_id = adapter.GetSimId();
    resp.total_num = adapter.GetTotalNum();
    resp.used_num = adapter.GetUsedNum();

    RilLogI("[%s] %s() sim id = %d", m_szSvcName, __FUNCTION__, resp.sim_id);
    RilLogI("[%s] %s() total no = %d", m_szSvcName, __FUNCTION__, resp.total_num);
    RilLogI("[%s] %s() used no = %d", m_szSvcName, __FUNCTION__, resp.used_num);

    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(errorCode, &resp, sizeof(RIL_StorageStatus));
    }
    else
    {
        OnErrorRequestComplete(errorCode);
    }

    return 0;
}


int SmsService::DoGetBroadcastSmsConfig(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if(pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildGetBroadcastSmsConfig();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_SMS_GET_BROADCAST_SMS_CONFIG_DONE) < 0) {
        RilLogE("[%s] %s() Error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int SmsService::OnGetBroadcastSmsConfigDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if(pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolGetBcstSmsConfRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS, adapter.GetConfigsInfoPointers(),
                adapter.GetConfigsNumber() * sizeof(RIL_GSM_BroadcastSmsConfigInfo *));
    } else {
        OnErrorRequestComplete(errorCode);
    }
    return 0;
}

int SmsService::DoSetBroadcastSmsConfig(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if(pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    BroadcastSmsConfigsRequestData *rildata = (BroadcastSmsConfigsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSetBroadcastSmsConfig(rildata->GetConfigsInfo(),
            rildata->GetConfigsNumber());
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_SMS_SET_BROADCAST_SMS_CONFIG_DONE) < 0) {
        RilLogE("Error");
        return -1;
    }
    return 0;
}

int SmsService::OnSetBroadcastSmsConfigDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    OnRequestComplete(adapter.GetErrorCode());
    return 0;
}

int SmsService::DoSmsBroadcastActivation(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntRequestData* rildata= (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int act = rildata->GetInt();
    if (act != 0/*SMS_BCST_ACT_ACTIVATE*/ && act != 1/*SMS_BCST_ACT_DEACTIVATE*/) {
        RilLogE("Undefined activation code(%d)!!!",act);
        return -1;
    }

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSmsBroadcastActivation(act);
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_SMS_BROADCAST_ACTIVATION_DONE) < 0) {
        RilLogE("[%s] %s() Error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int SmsService::OnSmsBroadcastActivationDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    OnRequestComplete(adapter.GetErrorCode());
    return 0;
}

int SmsService::DoGetSmscAddress(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if(pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSmscAddress();
    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_SMS_GET_SMSC_ADDRESS_DONE) < 0) {
        RilLogE("[%s] %s() Error", m_szSvcName, __FUNCTION__);
        return -1;
    }

    return 0;
}

int SmsService::OnGetSmscAddressDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if(pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    const ModemData *pModemData = pMsg->GetModemData();
    if ((pModemData == NULL) || (pModemData->GetRawData() == NULL)) {
        return -1;
    }

    ProtocolSmscAddrRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS)
    {
        const char *smscPdu = adapter.GetPdu();
        int smscPduLen = adapter.GetPduLength();
        char sca[MAX_GSM_SMS_SERVICE_CENTER_ADDR * 2] = {0, };
        int scaSize = (int)(sizeof(sca)/sizeof(sca[0]));
        int scaLen = 0;
        if (smscPdu != NULL) {
            m_scaLen = smscPduLen;
            memcpy(m_sca, smscPdu, smscPduLen);

            // Convert BCD to number format
            scaLen = ConvertSmscBcdToNumber(smscPdu, smscPduLen, sca, scaSize);
            if (scaLen > 0) {
                RilLogV("[%d] SMSC=%s", GetRilSocketId(), sca);
                const char *format = "vendor.ril.sms.sim%d.smsc";
                char buf[100] = {0, };
                snprintf(buf, sizeof(buf), format, GetRilSocketId());
                RilLogV("[%s] %s() property_set(%s,%s)", m_szSvcName, __FUNCTION__, buf, sca);
                property_set(buf, sca);
            }
            else {
                RilLogW("Failed to convert SMS PDU to String");
            }
        }
        else {
            RilLogW("SMSC is null");
        }

        OnRequestComplete(RIL_E_SUCCESS, sca, scaLen);
    }
    else
    {
        RilLogE("[%s] %s() %d error", m_szSvcName, __FUNCTION__, errorCode);
        OnRequestComplete(errorCode);
    }
    return 0;
}

int SmsService::DoSetSmscAddress(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("%s() pMsg is NULL", __FUNCTION__);
        return -1;
    }

    StringRequestData *rildata = (StringRequestData *)pMsg->GetRequestData();

    if (rildata == NULL) {
        RilLogE("%s() rildata is NULL", __FUNCTION__);
        return -1;
    }

    const char *sca = rildata->GetString();
    if (sca == NULL) {
        RilLogE("%s() rildata->GetString() is NULL", __FUNCTION__);
        return -1;
    }
    int scaLen = strlen(sca);

    // Check SMSC Length
    if (!IsSmscLenValid(sca, scaLen)) {
        RilLogE("[%s] %s() : Error: SMSC length exceeded maximum size", m_szSvcName, __FUNCTION__);
        return -1;
    }

    char smscPdu[MAX_GSM_SMS_SERVICE_CENTER_ADDR] = {0, };
    int smscPduSize = (int)(sizeof(smscPdu)/sizeof(smscPdu[0]));

    // Convert number to BCD format
    int smscPduLen = ConvertSmscNumberToBcd(sca, scaLen, smscPdu, smscPduSize);
    if (smscPduLen < 0) {
        RilLogE("[%s] %s() : Error: ConvertSmscNumberToBcd Failed", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSmscAddress(smscPduLen, smscPdu);

    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_SMS_SET_SMSC_ADDRESS_DONE) < 0) {
        RilLogE("[%s] %s() : Error", m_szSvcName, __FUNCTION__);
        return -1;
    }

    return 0;
}

int SmsService::OnSetSmscAddressDone(Message *pMsg)
{
    RilLogI("[%s] %s() N/A", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
            RilLogE("pModemData is NULL");
            return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);

    if(errorCode == RIL_E_SUCCESS){
        /*  SMSC consistency management btwn RIL and Frameworks    */
        RequestData *req = new RequestData(RIL_REQUEST_GET_SMSC_ADDRESS, 0);

        if (req != NULL) {
            Message *msg = Message::ObtainMessage(req, RIL_SERVICE_SMS, MSG_SMS_GET_SMSC_ADDRESS);

            if (m_pRilContext->GetServiceManager()->SendMessage(msg) < 0) {
                if (msg) {
                    delete msg;
                }
            }
        }
    }
    return 0;
}

int SmsService::DoReportSmsMemoryStatus(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL)
    {
        return -1;
    }

    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    int mem_status = rildata->GetInt();

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSmsMemoryStatus(mem_status);

    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_SMS_REPORT_SMS_MEMORY_STATUS_DONE) < 0) {
        RilLogE("[%s] %s() : Error", m_szSvcName, __FUNCTION__);
        return -1;
    }

    return 0;
}

int SmsService::OnReportSmsMemoryStatusDone(Message *pMsg)
{
    RilLogI("[%s] %s() N/A", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
            RilLogE("pModemData is NULL");
            return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);

    return 0;
}

int SmsService::DoSmsAckWithPdu(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    const SmsAcknowledgePdu *rildata = (const SmsAcknowledgePdu *)pMsg->GetRequestData();
    if (rildata == NULL) {
        return -1;
    }

#if 0
    const char *pdu = rildata->mPdu->ToHexString();
    int pduSize = strlen(pdu);
#else
    const char *pdu = rildata->mPdu->mData;
    int pduSize = rildata->mPdu->mLen;
#endif

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSmsAck(rildata->mResult, m_nLastTpid, pdu, pduSize);

    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_SMS_ACKNOWLEDGE_DONE) < 0 ) {
        RilLogE("[%s] %s() : Failed", m_szSvcName, __FUNCTION__);
        return -1;
    }

    return 0;
}

int SmsService::OnSmsAckWithPduDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
            RilLogE("pModemData is NULL");
            return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);

    return 0;
}

int SmsService::OnIncomingNewSms(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    const ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolNewSmsIndAdapter adapter(pModemData);
    char *pdu_tmp = NULL;
    pdu_tmp = (char *)adapter.GetPdu();
    m_nLastTpid = adapter.GetTpid();
    g_nLastTpidNewSms = m_nLastTpid;

    PduParser  SmsParser;
    m_smsclass = SmsParser.GetSmsClass(pdu_tmp);

    writeRilEvent(m_szSvcName, __FUNCTION__, "pdu_tmp(%s), m_nLastTpid(0x%x), g_nLastTpidNewSms(0x%x), m_smsclass(%d)", pdu_tmp, m_nLastTpid, g_nLastTpidNewSms, m_smsclass);

    if (m_smsclass == MESSAGE_CLASS_2)
    {
        RIL_SMS_WriteArgs data;
        data.smsc = NULL;
        int sca_len;

        data.status = 0x00/*SIM_STATUS_RECEIVED_UNREAD*/;
        sca_len = (SmsParser.GetScaLen(pdu_tmp)+1)*2;

        data.smsc = new char[sca_len+1];
        memset(&(data.smsc)[0], 0, sca_len+1);
        memcpy(&(data.smsc)[0], pdu_tmp, sca_len);
        data.pdu = pdu_tmp + sca_len ;

        if (SMS_DBG)
        {
            RilLogV("[7-1] PDU = %s", data.pdu);
            RilLogV("[7-2] PDU_SMSC = %s, strlen(data.smsc)=%d", data.smsc, strlen(data.smsc));
        }

        RequestData *req = RilParser::CreateSimSmsData(RIL_REQUEST_WRITE_SMS_TO_SIM, 0, (char *)(&data), sizeof(data));
        delete [] data.smsc;

        if (req != NULL)
        {
            Message *msg = Message::ObtainMessage(req, RIL_SERVICE_SMS, MSG_SMS_WRITE_SMS_TO_SIM);
            if (m_pRilContext->GetServiceManager()->SendMessage(msg) < 0)
            {
                delete msg;
            }
        }
    }
    else
    {
        OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_NEW_SMS, adapter.GetPdu(), adapter.GetPduSize());
    }
    return 0;
}

int SmsService::OnIncomingNewSmsOnSim(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    int index;
    int indexLen;

    if (pMsg == NULL) {
        return -1;
    }

    const ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolWriteSmsToSimRespAdapter adapter(pModemData);
    index = adapter.GetIndex();
    indexLen = adapter.GetIndexLen();

    OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM, &index, indexLen);

    return 0;
}

int SmsService::OnSimSmsStorageFull(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        return -1;
    }

    m_simsms_full = 1;

    #if 0
    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
            RilLogE("pModemData is NULL");
            return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);
    #endif
    OnUnsolicitedResponse(RIL_UNSOL_SIM_SMS_STORAGE_FULL, NULL, 0);

    return 0;
}

int SmsService::OnIncomingNewBroadcastSms(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    const ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolNewBcstSmsAdapter adapter(pModemData);

    OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS, adapter.GetBcst(), adapter.GetBcstLen());
    return 0;
}

#ifdef SUPPORT_CDMA
#include "cdmasmsdata.h"

#define CDMA_SMS_DEFAULT_TIMEOUT    SMS_DEFAULT_TIMEOUT
#define CDMA_SMS_SEND_TIMEOUT       SMS_SEND_TIMEOUT
int SmsService::DoSendCdmaSms(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    CdmaSmsRequestData *rildata = (CdmaSmsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("[%s] %s() rildata is NULL", m_szSvcName,  __FUNCTION__);
        return -1;
    }

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSendCdmaSms((const char *)rildata->GetMessage(),
            rildata->GetMessageLength());

    if (SendRequest(pModemData, CDMA_SMS_SEND_TIMEOUT, MSG_SMS_CDMA_SEND_DONE) < 0) {
        RilLogE("[%s] %s() Error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int SmsService::OnSendCdmaSmsDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolCdmaSendSmsRespAdapter adapter(pModemData);
    int rilError = adapter.GetErrorCode();
    int errorClass = adapter.GetSmsRspErrorClass();
    int causeCode = adapter.GetSmsRspCauseCode();
    int messageRef = adapter.GetRef();

    RIL_SMS_Response resp;
    resp.ackPDU = NULL;
    resp.messageRef = messageRef;
    resp.errorCode = causeCode;

    if (rilError == RIL_E_SUCCESS) {
        RilLogV("[%s] %s() RCM_E_SUCCESS(Error class = %d, Cause code = %d)", m_szSvcName, __FUNCTION__,
                errorClass, causeCode);
        // TODO: Should set proper RIL Error based on Error Class.
        switch (errorClass) {
            case ERROR_CLASS_NO_ERROR:
                OnRequestComplete(RIL_E_SUCCESS, &resp, sizeof(RIL_SMS_Response));
                break;
            case ERROR_CLASS_TEMPORARY_ERROR:
                OnRequestComplete(RIL_E_NETWORK_NOT_READY, &resp, sizeof(RIL_SMS_Response));
                break;
            case ERROR_CLASS_PERMANENT_ERROR:
                OnRequestComplete(RIL_E_NETWORK_REJECT, &resp, sizeof(RIL_SMS_Response));
                break;
            default:
                OnErrorRequestComplete(RIL_E_GENERIC_FAILURE);
                break;
        }
    } else {
        OnErrorRequestComplete(rilError);
    }
    return 0;
}

int SmsService::OnIncomingNewCdmaSms(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolCdmaNewSmsIndAdapter adapter(pModemData);
    m_nLastTpid = adapter.GetTpid();
    g_nLastTpidNewSms = m_nLastTpid;

    OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, adapter.GetRilCdmaSmsMsg(),
            adapter.GetMessageLength());
    return 0;
}

int SmsService::DoWriteCdmaSmsToRuim(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }
    CdmaSmsWriteToRuimRequestData *rildata = (CdmaSmsWriteToRuimRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("[%s] %s() rildata is NULL", m_szSvcName,  __FUNCTION__);
        return -1;
    }

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildWriteCdmaSmsToRuim(rildata->GetStatus(),
            (const char *)rildata->GetMessage(), rildata->GetMessageLength());

    if (SendRequest(pModemData, CDMA_SMS_DEFAULT_TIMEOUT, MSG_SMS_CDMA_WRITE_SMS_TO_RUIM_DONE) < 0) {
        RilLogE("[%s] %s() Error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int SmsService::OnWriteCdmaSmsToRuimDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolCdmaWriteSmsToRuimRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    int index = adapter.GetIndex();

    if (errorCode == RIL_E_SUCCESS) {
        RilLogV("[%s] %s() RCM_E_SUCCESS(Index=%d)", m_szSvcName, __FUNCTION__, index);
        OnRequestComplete(RIL_E_SUCCESS, &index, sizeof(index));
    } else {
        OnErrorRequestComplete(errorCode);
    }
    return 0;
}

int SmsService::DoDeleteCdmaSmsOnRuim(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    IntRequestData* rildata= (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("[%s] %s() rildata is NULL", m_szSvcName,  __FUNCTION__);
        return -1;
    }

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildDeleteCdmaSmsOnRuim(rildata->GetInt());

    if (SendRequest(pModemData, CDMA_SMS_DEFAULT_TIMEOUT, MSG_SMS_CDMA_DELETE_SMS_ON_RUIM_DONE) < 0) {
        RilLogE("[%s] %s() Error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int SmsService::OnDeleteCdmaSmsOnRuimDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    OnRequestComplete(adapter.GetErrorCode());
    return 0;
}

int SmsService::OnRuimSmsStorageFull(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    OnUnsolicitedResponse(RIL_UNSOL_CDMA_RUIM_SMS_STORAGE_FULL, NULL, 0);
    return 0;
}

int SmsService::DoGetCdmaBroadcastSmsConfig(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildGetCdmaBroadcastSmsConfig();
    if (SendRequest(pModemData, CDMA_SMS_DEFAULT_TIMEOUT, MSG_SMS_CDMA_GET_BROADCAST_SMS_CONFIG_DONE) < 0) {
        RilLogE("[%s] %s() Error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int SmsService::OnGetCdmaBroadcastSmsConfigDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolGetCdmaBcstSmsConfRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS, adapter.GetConfigsInfoPointers(),
                adapter.GetConfigsNumber() * sizeof(RIL_CDMA_BroadcastSmsConfigInfo *));
    } else {
        OnErrorRequestComplete(errorCode);
        RilLogV("[%s] %s() Error(%d)", m_szSvcName, __FUNCTION__, errorCode);
    }
    return 0;
}

int SmsService::DoSetCdmaBroadcastSmsConfig(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    CdmaBroadcastSmsConfigsRequestData *rildata = (CdmaBroadcastSmsConfigsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("[%s] %s() rildata is NULL", m_szSvcName,  __FUNCTION__);
        return -1;
    }

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSetCdmaBroadcastSmsConfig(rildata->GetConfigsInfo(),
            rildata->GetConfigsNumber());
    if (SendRequest(pModemData, CDMA_SMS_DEFAULT_TIMEOUT, MSG_SMS_CDMA_SET_BROADCAST_SMS_CONFIG_DONE) < 0) {
        RilLogE("[%s] %s() Error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int SmsService::OnSetCdmaBroadcastSmsConfigDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    OnRequestComplete(adapter.GetErrorCode());
    return 0;
}

int SmsService::DoSmsCdmaBroadcastActivation(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    IntRequestData* rildata= (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("[%s] %s() rildata is NULL", m_szSvcName,  __FUNCTION__);
        return -1;
    }

    int act = rildata->GetInt();
    if (act != RIL_SMS_CDMA_BCST_ACT_ACTIVATE && act != RIL_SMS_CDMA_BCST_ACT_DEACTIVATE) {
        RilLogE("[%s] %s() Undefined activation code(%d)!!!", m_szSvcName, __FUNCTION__, act);
        return -1;
    }
    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildCdmaSmsBroadcastActivation(act);

    if (SendRequest(pModemData, CDMA_SMS_DEFAULT_TIMEOUT, MSG_SMS_CDMA_BROADCAST_ACTIVATION_DONE) < 0) {
        RilLogE("[%s] %s() Error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int SmsService::OnSmsCdmaBroadcastActivationDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    OnRequestComplete(adapter.GetErrorCode());
    return 0;
}

int SmsService::OnVoiceMsgWaitingInfo(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolCdmaVoiceMsgWaitingInfoIndAdapter adapter(pModemData);

    OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_CDMA_NEW_SMS, adapter.GetRilCdmaSmsMsg(),
            adapter.GetMessageLength());
    return 0;
}
#endif // SUPPORT_CDMA

int SmsService::DoSendAimsSms(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    RawRequestData *rildata =(RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolImsBuilder builder;
    ModemData *pModemData = builder.BuildAimsPDU(rildata->GetReqId(), rildata->GetRawData(), rildata->GetSize());

    if (SendRequest(pModemData, SMS_SEND_TIMEOUT, pMsg->GetMsgId() + 1) < 0) {
        RilLogE("[%s] %s() Error", m_szSvcName, __FUNCTION__);
        return -1;
    }

    return 0;
}

int SmsService::OnSendAimsSmsDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());
    }
    else {
        OnErrorRequestComplete(errorCode);
    }

    return 0;
}
