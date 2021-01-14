/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "supplementaryservice.h"
#include "rillog.h"
#include "protocolcallbuilder.h"
#include "protocolcalladapter.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define writeRilEvent(format1, format2, ...)   CRilEventLog::writeRilEvent(RIL_LOG_CAT_CALL, format1, format2, ##__VA_ARGS__)

SupplementaryService::SupplementaryService(RilContext *pRilContext)
    : Service(pRilContext, RIL_SERVICE_SUPPLEMENTARY)
{
    for (unsigned int i = 0; i < sizeof(mRespCallForward)/sizeof(RIL_CallForwardInfo *); i++)
    {
        mRespCallForward[i] = NULL;
    }

    memset(&mRespCallForwardData, 0x00, sizeof(mRespCallForwardData));

    for (unsigned int i = 0; i < sizeof(mRespUssd)/sizeof(char *); i++)
    {
        mRespUssd[i] = NULL;
    }

    memset(mResp2Ints, 0x00, sizeof(mResp2Ints));

    mRespInt = 0;

    memset(&mRespSuppSvcNoti, 0x00, sizeof(mRespSuppSvcNoti));

    m_ussdUserInitiated = true;

    mCardState = RIL_CARDSTATE_ABSENT;
}

SupplementaryService::~SupplementaryService()
{
}

int SupplementaryService::OnCreate(RilContext *pRilContext)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
    return 0;
}

BOOL SupplementaryService::OnHandleRequest(Message *pMsg)
{
    INT32 nRet = -1;
    if(NULL == pMsg)
    {
        return FALSE;
    }

    switch (pMsg->GetMsgId())
    {
    case MSG_SS_CALL_BARRING_PWD:
        nRet = DoChangeCallBarringPwd();
        break;
    case MSG_SS_SET_CALL_WAITING:
        nRet = DoSetCallWaiting();
        break;
    case MSG_SS_QUERY_CALL_WAITING:
        nRet = DoQueryCallWaiting();
        break;
    case MSG_SS_SET_CALL_FORWARDING:
        nRet = DoSetCallForwarding();
        break;
    case MSG_SS_QUERY_CALL_FORWARDING:
        nRet = DoQueryCallForwarding();
        break;
    case MSG_SS_SET_CLIR:
        nRet = DoSetClir();
        break;
    case MSG_SS_GET_CLIR:
        nRet = DoGetClir();
        break;
    case MSG_SS_GET_CLIP:
        nRet = DoGetClip();
        break;
    case MSG_SS_CANCEL_USSD:
        nRet = DoCancelUssd();
        break;
    case MSG_SS_SEND_USSD:
        nRet = DoSendUssd();
        break;
    case MSG_SS_QUERY_COLP:
        nRet = DoQueryColp();
        break;
    case MSG_SS_QUERY_COLR:
        nRet = DoQueryColr();
        break;
    case MSG_SS_SEND_ENCODED_USSD:
        nRet = DoSendEncodedUssd();
        break;
    default:
        break;
    }

    if (0 == nRet)
        return TRUE;
    else
        return FALSE;
}

BOOL SupplementaryService::OnHandleSolicitedResponse(Message *pMsg)
{
    INT32 nRet = -1;

    if(NULL == pMsg)
        return FALSE;

    switch (pMsg->GetMsgId())
    {
    case MSG_SS_CALL_BARRING_PWD_DONE:
        nRet = DoChangeCallBarringPwdDone(pMsg);
        break;
    case MSG_SS_SET_CALL_WAITING_DONE:
        nRet = DoSetCallWaitingDone(pMsg);
        break;
    case MSG_SS_QUERY_CALL_WAITING_DONE:
        nRet = DoQueryCallWaitingDone(pMsg);
        break;
    case MSG_SS_SET_CALL_FORWARDING_DONE:
        nRet = DoSetCallForwardingDone(pMsg);
        break;
    case MSG_SS_QUERY_CALL_FORWARDING_DONE:
        nRet = DoQueryCallForwardingDone(pMsg);
        break;
    case MSG_SS_GET_CLIR_DONE:
        nRet = DoGetClirDone(pMsg);
        break;
    case MSG_SS_GET_CLIP_DONE:
        nRet = DoGetClipDone(pMsg);
        break;
    case MSG_SS_CANCEL_USSD_DONE:
        nRet = DoCancelUssdDone(pMsg);
        break;
    case MSG_SS_SEND_USSD_DONE:
        nRet = DoSendUssdDone(pMsg);
        break;
    case MSG_SS_QUERY_COLP_DONE:
        nRet = DoQueryColpDone(pMsg);
        break;
    case MSG_SS_QUERY_COLR_DONE:
        nRet = DoQueryColrDone(pMsg);
        break;
    default:// it should add its exception handling
        RilLogE("[SupplementaryService::%s] solicited response handler is not specified. error", __FUNCTION__);
        break;
    }

    if (0 == nRet)
        return TRUE;
    else
        return FALSE;
}

BOOL SupplementaryService::OnHandleUnsolicitedResponse(Message *pMsg)
{
    if(NULL == pMsg)
        return FALSE;

    switch (pMsg->GetMsgId())
    {
    case MSG_SS_USSD_NTF:
        OnUssd(pMsg);
        break;
    case MSG_SS_SVC_NTF:
        OnSsSvc(pMsg);
        break;
    case MSG_SS_IND:
        OnUnsolOnSS(pMsg);
        break;
    default:
        RilLogE("[SupplementaryService::%s] unsolicited response handler is not specified. error", __FUNCTION__);
        break;
    }

    return TRUE;
}


BOOL SupplementaryService::OnHandleInternalMessage(Message* pMsg)
{
    //int nMsgId = 0;
    //char *pData = NULL;
    if (NULL == pMsg)
    {
        RilLogE("Received null internal message");
    }

#if 0
    nMsgId = pMsg->GetMsgId();
    //pData = pMsg->GetInternalData();

    RilLogV("[SupplementaryService]received internal message,id=%d", nMsgId);
    switch(nMsgId)
    {
    case MSG_INTER_SIM_STATE_CHANGED:
    {
        DoSimStateChanged();
        break;
    }
    }
#endif
    return TRUE;
}

void SupplementaryService::OnSimStatusChanged(int cardState, int appState)
{
    mCardState = cardState;

    RilLogV("[%s] Card state: %d, App state: %d", __FUNCTION__, cardState, appState);
}

INT32 SupplementaryService::DoChangeCallBarringPwd()
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);

    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }
    // On Sim absent, requesting command into CP cause IPC TIMEOUT
    // So return early,
    if (mCardState != RIL_CARDSTATE_PRESENT) {
        RilLogI("[%s] %s mCardState(%d) is not Present(%d)", m_szSvcName, __FUNCTION__, mCardState, RIL_CARDSTATE_PRESENT);
        OnRequestComplete(RIL_E_MODEM_ERR);
        return 0;
    }

    StringsRequestData* pReq = (StringsRequestData*) m_pCurReqMsg->GetRequestData();

    ProtocolCallBuilder builder;

    // if barring facility string code (eg. "AO" barr all outgoing calls) is needed, use CBarringPwdReqData::GetFacilityStringCode() function

    // If 3rd para is null, this is AOSP scheme.
    // If not, this is samsung scheme that request is sent even though the new password is mismatch.
    const char *str = pReq->GetString(3) != NULL ? pReq->GetString(3) : pReq->GetString(2);

    RilLogV("[%s] oldpw:%s, newpw:%s, newpwagain:%s", __FUNCTION__, pReq->GetString(1), pReq->GetString(2), str);
    ModemData *pModemData = builder.BuildChangeBarringPwd(pReq->GetString(1), pReq->GetString(2), str);

    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_CALL_BARRING_PWD_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 SupplementaryService::DoChangeCallBarringPwdDone(Message* pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
   Allowed RadioError on CardState::ABSENT is
 *   RadioError::NONE
 *   RadioError::MODEM_ERR
 *   RadioError::INVALID_ARGUMENTS
 *   RadioError::FDN_CHECK_FAILURE
 *   GeneralErrors on VTS-HIDL
 *    RadioError::RADIO_NOT_AVAILABLE (radio resetting)
 *    RadioError::NO_MEMORY
 *    RadioError::INTERNAL_ERR
 *    RadioError::SYSTEM_ERR
 *    RadioError::REQUEST_NOT_SUPPORTED
 *    RadioError::CANCELLED
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:SS_MODIFIED_TO_DIAL
 *   RadioError:SS_MODIFIED_TO_USSD
 *   RadioError:SS_MODIFIED_TO_SS
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:NO_MEMORY
 *   RadioError:MODEM_ERR
 *   RadioError:INTERNAL_ERR
 *   RadioError:FDN_CHECK_FAILURE
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:NO_RESOURCES
 *   RadioError:CANCELLED
 */
    RilLogI("[%s] %s mCardState(%d) errorCode=%d", m_szSvcName, __FUNCTION__, mCardState, errorCode);
    switch (errorCode) {
      // Expected Error Code
      case RIL_E_SUCCESS:
      case RIL_E_RADIO_NOT_AVAILABLE:
      case RIL_E_SS_MODIFIED_TO_DIAL:
      case RIL_E_SS_MODIFIED_TO_USSD:
      case RIL_E_SS_MODIFIED_TO_SS:
      case RIL_E_INVALID_ARGUMENTS:
      case RIL_E_NO_MEMORY:
      case RIL_E_MODEM_ERR:
      case RIL_E_INTERNAL_ERR:
      case RIL_E_SYSTEM_ERR:
      case RIL_E_FDN_CHECK_FAILURE:
      case RIL_E_REQUEST_NOT_SUPPORTED:
      case RIL_E_NO_RESOURCES:
      case RIL_E_CANCELLED:
        OnRequestComplete(errorCode);
        break;
        // These ErrorCodes are no more expected
      case RIL_E_PASSWORD_INCORRECT:
      case RIL_E_SUBSCRIPTION_NOT_AVAILABLE:
        OnRequestComplete(RIL_E_CANCELLED);
        break;
      default:
        OnRequestComplete(RIL_E_INTERNAL_ERR);
    }
    return 0;
}

INT32 SupplementaryService::DoSetCallWaiting()
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    IntsRequestData* pReq = (IntsRequestData*) m_pCurReqMsg->GetRequestData();

    ProtocolCallBuilder builder;
    RilLogV("[%s] SS Mode : %d, SS Class:%d", __FUNCTION__, pReq->GetInt(0)/*Mode*/, pReq->GetInt(1)/*Service Class*/);
    ModemData *pModemData = builder.BuildSetCallWaiting((SsModeType)pReq->GetInt(0)/*Mode*/, (SsClassX)pReq->GetInt(1)/*Service Class*/);
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_SET_CALL_WAITING_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 SupplementaryService::DoSetCallWaitingDone(Message* pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = GetValidErrors(adapter.GetErrorCode());

/*
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 */
    OnRequestComplete(errorCode);

    return 0;
}

INT32 SupplementaryService::DoQueryCallWaiting()
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    IntsRequestData* pReq = (IntsRequestData*) m_pCurReqMsg->GetRequestData();

    ProtocolCallBuilder builder;

    RilLogV("[%s] SS Class:%d", __FUNCTION__, pReq->GetInt(0)/*Service Class*/);
    SsClassX serviceClass = (SsClassX)pReq->GetInt(0) == RIL_SS_CLASS_UNKNOWN
        ? RIL_SS_CLASS_VOICE :(SsClassX)pReq->GetInt(0);
    ModemData *pModemData = builder.BuildGetCallWaiting(serviceClass);
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_QUERY_CALL_WAITING_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 SupplementaryService::DoQueryCallWaitingDone(Message* pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolGetCallWaitingAdapter adapter(pModemData);
    int errorCode = GetValidErrors(adapter.GetErrorCode());

/*
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        mResp2Ints[0] = adapter.GetServiceStatus();
        RilLogV("[SupplementaryService::%s] Service Status : %d", __FUNCTION__, mResp2Ints[0]);

        mResp2Ints[1] = adapter.GetServiceClass();
        RilLogV("[SupplementaryService::%s] Service Class : %d", __FUNCTION__, mResp2Ints[1]);
        OnRequestComplete(RIL_E_SUCCESS, mResp2Ints, 2 * sizeof(int));
    }
    else
    {
        OnRequestComplete(errorCode);
    }

    return 0;
}

INT32 SupplementaryService::DoSetCallForwarding()
{
    RilLogI("[SupplementaryService] %s",__FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    CallForwardReqData* pReq = (CallForwardReqData*) m_pCurReqMsg->GetRequestData();

    ProtocolCallBuilder builder;

    RilLogV("[SupplementaryService] %s: Status: %d", __FUNCTION__, pReq->GetStatus());
    RilLogV("[SupplementaryService] %s: Reason: %d", __FUNCTION__, pReq->GetReason());
    RilLogV("[SupplementaryService] %s: classType: %d", __FUNCTION__, pReq->GetSsClassType());
    RilLogV("[SupplementaryService] %s: Toa: %d", __FUNCTION__, pReq->GetToa());
    RilLogV("[SupplementaryService] %s: Number: %s", __FUNCTION__, pReq->GetNumber());
    RilLogV("[SupplementaryService] %s: TimeSeconds: %d", __FUNCTION__, pReq->GetTimeSeconds());

    SsClassX serviceClass = pReq->GetSsClassType();
    if(IsOperatorUsingUnknownServiceClass() && serviceClass == RIL_SS_CLASS_VOICE)
    {
        serviceClass = RIL_SS_CLASS_UNKNOWN;
        RilLogV("[SupplementaryService] %s: classType: %d", __FUNCTION__, serviceClass);
    }
    ModemData *pModemData = builder.BuildSetCallForwarding(pReq->GetStatus(), pReq->GetReason(), serviceClass,
                                                pReq->GetToa(), pReq->GetNumber(), pReq->GetTimeSeconds());
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_SET_CALL_FORWARDING_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 SupplementaryService::DoSetCallForwardingDone(Message* pMsg)
{
    RilLogI("[SupplementaryService] %s",__FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = GetValidErrors(adapter.GetErrorCode());

/*
 * Valid errors:
 *  SUCCESS
 *  FDN_CHECK_FAILURE
 *  RADIO_NOT_AVAILABLE (radio resetting)
 */
    if ( errorCode != RIL_E_SUCCESS )
    {
        RilLogE("[supplementaryService::::%s] Set Call forwarding status fail (error code: %d)", __FUNCTION__, errorCode);
        OnRequestComplete(errorCode);
        return 0;
    }
    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

INT32 SupplementaryService::DoQueryCallForwarding()
{
    RilLogI("[SupplementaryService] %s",__FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    CallForwardReqData* pReq = (CallForwardReqData*) m_pCurReqMsg->GetRequestData();

    ProtocolCallBuilder builder;

    RilLogV("[SupplementaryService] %s: Status: %d", __FUNCTION__, pReq->GetStatus());
    RilLogV("[SupplementaryService] %s: Reason: %d", __FUNCTION__, pReq->GetReason());
    RilLogV("[SupplementaryService] %s: classType: %d", __FUNCTION__, pReq->GetSsClassType());
    RilLogV("[SupplementaryService] %s: Toa: %d", __FUNCTION__, pReq->GetToa());
    RilLogV("[SupplementaryService] %s: Number: %s", __FUNCTION__, pReq->GetNumber());
    RilLogV("[SupplementaryService] %s: TimeSeconds: %d", __FUNCTION__, pReq->GetTimeSeconds());

    SsClassX serviceClass = pReq->GetSsClassType();
    if(IsOperatorUsingUnknownServiceClass() && serviceClass == RIL_SS_CLASS_VOICE)
    {
        serviceClass = RIL_SS_CLASS_UNKNOWN;
        RilLogV("[SupplementaryService] %s: classType: %d", __FUNCTION__, serviceClass);
    }
    ModemData *pModemData = builder.BuildGetCallForwardingStatus((SsStatusType)pReq->GetStatus(), pReq->GetReason(), serviceClass,
                                                pReq->GetToa(), pReq->GetNumber(), pReq->GetTimeSeconds());
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_QUERY_CALL_FORWARDING_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 SupplementaryService::DoQueryCallForwardingDone(Message* pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);

    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolGetCallForwardingStatusAdapter adapter(pModemData);
    int errorCode = GetValidErrors(adapter.GetErrorCode());

/*
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 */
    if (adapter.HasValidLength() == false) {
        RilLogE("[CCscService::::%s] Get Query Call forwarding status fail invalid length", __FUNCTION__);
        errorCode = RIL_E_INTERNAL_ERR;
    }

    if ( errorCode != RIL_E_SUCCESS )
    {
        RilLogE("[CCscService::::%s] Get Query Call forwarding status fail (error code: %d)", __FUNCTION__, adapter.GetErrorCode());
        OnRequestComplete(errorCode);
        return 0;
    }

    int nCfNum = adapter.GetCfNum();
    if (nCfNum >= MAX_CALL_FORWARD_STATUS_NUM)
    {
        RilLogE("[SupplementaryService::::%s] Call forwarding status Count invalid (%d)", __FUNCTION__, nCfNum);
        OnRequestComplete(RIL_E_INTERNAL_ERR);
        return 0;
    }

    RilLogV("Call forwarding status Count: %d", nCfNum);

    for (int i = 0; i < nCfNum; i++)
    {
        if (0 > adapter.GetCfInfo(&mRespCallForwardData[i], i))
        {
            break;
        }
        RilLogV("< %d CallInfo >", i);
        adapter.DebugPrintCfInfo(&mRespCallForwardData[i]);

        mRespCallForward[i] = &mRespCallForwardData[i];
        if(IsOperatorUsingUnknownServiceClass()
                && mRespCallForward[i]->serviceClass == (int)RIL_SS_CLASS_UNKNOWN)
        {
            mRespCallForward[i]->serviceClass = (int)RIL_SS_CLASS_VOICE;
            RilLogV("[SupplementaryService] %s: classType: %d", __FUNCTION__, mRespCallForward[i]->serviceClass);
        }

    }

    OnRequestComplete(RIL_E_SUCCESS, mRespCallForward, nCfNum * sizeof(RIL_CallForwardInfo *));
    return 0;
}

INT32 SupplementaryService::DoSetClir()
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    IntRequestData *rildataInt = (IntRequestData *) m_pCurReqMsg->GetRequestData();
    if (rildataInt == NULL) {
        RilLogE("rildataInt is NULL");
        return -1;
    }

    m_clirInfo.SetClirAoc(rildataInt->GetInt(), GetRilSocketId());

    //jhdaniel.kim 20140930 no meaning to send CLIR set command to CP.
    //CP just ignore
    // Any side can handle CLIR value and store CLIR previous set
    // After discussing with Venkaeswar(k.venkatesh), Vendor RIL implements CLIR handling
    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

INT32 SupplementaryService::DoGetClir()
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildGetClir();
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_GET_CLIR_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 SupplementaryService::DoGetClirDone(Message* pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolGetClirAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
   Allowed RadioError on CardState::ABSENT is
 *   RadioError::NONE
 *   RadioError::MODEM_ERR
 *   GeneralErrors on VTS-HIDL
 *    RadioError::RADIO_NOT_AVAILABLE (radio resetting)
 *    RadioError::NO_MEMORY
 *    RadioError::INTERNAL_ERR
 *    RadioError::SYSTEM_ERR
 *    RadioError::REQUEST_NOT_SUPPORTED
 *    RadioError::CANCELLED
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:SS_MODIFIED_TO_DIAL
 *   RadioError:SS_MODIFIED_TO_USSD
 *   RadioError:SS_MODIFIED_TO_SS
 *   RadioError:NO_MEMORY
 *   RadioError:MODEM_ERR
 *   RadioError:INTERNAL_ERR
 *   RadioError:FDN_CHECK_FAILURE
 *   RadioError:SYSTEM_ERR
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:NO_RESOURCES
 *   RadioError:CANCELLED
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        switch (adapter.GetClirStatus())
        {
        case CLIR_PROVISIONED:
            mResp2Ints[0] = CLIR_INVOCATION;
            m_clirInfo.SetClirAoc(CLIR_INVOCATION, GetRilSocketId());
            break;
        case CLIR_NOT_PROVISIONED:
        case CLIR_UNKNOWN:
            mResp2Ints[0] = CLIR_DEFAULT;
            m_clirInfo.SetClirAoc(CLIR_DEFAULT, GetRilSocketId());
            break;
        case CLIR_TEMP_RESTRICTED:
        case CLIR_TEMP_ALLOWED:
            mResp2Ints[0] = m_clirInfo.GetClirAoc(GetRilSocketId());
            break;
        }
        mResp2Ints[1] = adapter.GetClirStatus();

        RilLogV("[SupplementaryService::%s] CLIR AOC : %d", __FUNCTION__, mResp2Ints[0]);
        RilLogV("[SupplementaryService::%s] CLIR Status : %d", __FUNCTION__, mResp2Ints[1]);

        OnRequestComplete(RIL_E_SUCCESS, mResp2Ints, 2 * sizeof(int));
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_MODEM_ERR);
    }
    return 0;
}

INT32 SupplementaryService::DoGetClip()
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildGetClip();
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_GET_CLIP_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 SupplementaryService::DoGetClipDone(Message* pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolGetClipAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
   Allowed RadioError on CardState::ABSENT are
     RadioError::NONE
     RadioError::MODEM_ERR
     GeneralErrors
 *    RadioError::RADIO_NOT_AVAILABLE (radio resetting)
 *    RadioError::NO_MEMORY
 *    RadioError::INTERNAL_ERR
 *    RadioError::SYSTEM_ERR
 *    RadioError::REQUEST_NOT_SUPPORTED
 *    RadioError::CANCELLED
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:NO_MEMORY
 *   RadioError:SYSTEM_ERR
 *   RadioError:MODEM_ERR
 *   RadioError:INTERNAL_ERR
 *   RadioError:FDN_CHECK_FAILURE
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:NO_RESOURCES
 *   RadioError:CANCELLED
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        mRespInt = adapter.GetClipStatus();
        RilLogV("[SupplementaryService::%s] CLIP Status : %d", __FUNCTION__, mRespInt);
        OnRequestComplete(RIL_E_SUCCESS, &mRespInt, sizeof(int));
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_MODEM_ERR);
    }

    return 0;
}

INT32 SupplementaryService::DoCancelUssd()
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildCancelUssd();
    m_ussdUserInitiated = true;
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_CANCEL_USSD_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 SupplementaryService::DoCancelUssdDone(Message* pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
        OnUnsolicitedResponse(RIL_UNSOL_USSD_CANCELED);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }
    return 0;
}

INT32 SupplementaryService::DoSendUssd()
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    if (mCardState == RIL_CARDSTATE_ABSENT) {
        RilLogI("[%s] %s mCardState(%d) is not Present", m_szSvcName, __FUNCTION__, mCardState);
        OnRequestComplete(RIL_E_INVALID_STATE);
        return 0;
    }

    StringRequestData*pReq = (StringRequestData*) m_pCurReqMsg->GetRequestData();

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildSendUssd(pReq->GetString(), m_ussdUserInitiated);
    m_ussdUserInitiated = true;
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_SEND_USSD_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 SupplementaryService::DoSendUssdDone(Message* pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * Allowed RadioError on CardState::ABSENT are
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:INVALID_STATE
 *   RadioError:MODEM_ERR
 *   GeneralErrors
 *    RadioError::RADIO_NOT_AVAILABLE (radio resetting)
 *    RadioError::NO_MEMORY
 *    RadioError::INTERNAL_ERR
 *    RadioError::SYSTEM_ERR
 *    RadioError::REQUEST_NOT_SUPPORTED
 *    RadioError::CANCELLED
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:SS_MODIFIED_TO_DIAL
 *   RadioError:SS_MODIFIED_TO_USSD
 *   RadioError:SS_MODIFIED_TO_SS
 *   RadioError:SIM_BUSY
 *   RadioError:OPERATION_NOT_ALLOWED
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:NO_MEMORY
 *   RadioError:MODEM_ERR
 *   RadioError:INTERNAL_ERR
 *   RadioError:ABORTED
 *   RadioError:SYSTEM_ERR
 *   RadioError:INVALID_STATE
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:INVALID_MODEM_STATE
 *   RadioError:NO_RESOURCES
 *   RadioError:CANCELLED
*/
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        RilLogE("[SupplementaryService] %s errorCode=%d", __FUNCTION__, errorCode);
        switch (errorCode) {
          case RIL_E_FDN_CHECK_FAILURE:
          case RIL_E_RADIO_NOT_AVAILABLE:
              OnRequestComplete(errorCode);
              break;
          default:
              OnRequestComplete(RIL_E_MODEM_ERR);
        }
    }
    return 0;
}


INT32 SupplementaryService::OnUssd(Message *pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }
    int len;
    char status[4];
    int dcs_val = 0;
    char dcs[4];
    char decodedUssd[MAX_USSD_DATA_LEN + 26];    //received ussd can be 182, if it is gsm7bit, it can be 182*8/7=208.
    memset(decodedUssd, 0x00, sizeof(decodedUssd));
/*
 * "data" is const char **
 * ((const char **)data)[0] points to a type code, which is
 *  one of these string values:
 *      "0"   USSD-Notify -- text in ((const char **)data)[1]
 *      "1"   USSD-Request -- text in ((const char **)data)[1]
 *      "2"   Session terminated by network
 *      "3"   other local client (eg, SIM Toolkit) has responded
 *      "4"   Operation not supported
 *      "5"   Network timeout
 *
 * The USSD session is assumed to persist if the type code is "1", otherwise
 * the current session (if any) is assumed to have terminated.
 *
 * ((const char **)data)[1] points to a message string if applicable, which
 * should always be in UTF-8.
 */
    ModemData *pModemData = pMsg->GetModemData();
    ProtocolUssdIndAdapter adapter(pModemData);

    RilLogV("[%s] ussd status : %d", __FUNCTION__, adapter.GetUssdStatus());
    if (adapter.GetUssdStatus() == RIL_USSD_REQUEST)
    {
        m_ussdUserInitiated = false;
    }
    else
    {
        m_ussdUserInitiated = true;
    }
    RilLogV("[%s] user initiated : %d", __FUNCTION__, m_ussdUserInitiated);

    len = adapter.GetDecodedUssd(decodedUssd, sizeof(decodedUssd), dcs_val);

    snprintf(status, sizeof(status)-1, "%d", adapter.GetUssdStatus());
    memset(dcs, 0x00, sizeof(dcs));
    snprintf(dcs, sizeof(dcs)-1, "%x", dcs_val);
    mRespUssd[0] = status;
    mRespUssd[1] = decodedUssd;
#ifdef RIL_FEATURE_NO_EXTENTION_USSD_DCS    //original AOSP compatibility
    OnUnsolicitedResponse(RIL_UNSOL_ON_USSD, (char *)mRespUssd, 2 * sizeof(char*));
#else
    mRespUssd[2] = dcs;
    OnUnsolicitedResponse(RIL_UNSOL_ON_USSD_WITH_DCS, (char *)mRespUssd, 3 * sizeof(char*));
#endif  //RIL_FEATURE_NO_EXTENTION_USSD_DCS

    return 0;
}

INT32 SupplementaryService::OnSsSvc(Message *pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }
/**
 * RIL_UNSOL_SUPP_SVC_NOTIFICATION
 *
 * Reports supplementary service related notification from the network.
 *
 * "data" is a const RIL_SuppSvcNotification *
 *
 */
    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSsSvcIndAdapter adapter(pModemData);

    memset(&mRespSuppSvcNoti, 0x00, sizeof(mRespSuppSvcNoti));
    mRespSuppSvcNoti.notificationType = adapter.GetNotificationType();
    mRespSuppSvcNoti.code = adapter.GetCode();
    mRespSuppSvcNoti.index = adapter.GetCugIndex();

    if (mRespSuppSvcNoti.notificationType == RIL_SSNOTI_TYPE_MT)    /* 1 = MT unsolicited result code */
    {
        mRespSuppSvcNoti.type = adapter.GetSSType();
        mRespSuppSvcNoti.number = adapter.GetNumber();
    }

    RilLogV("[%s] notificationType : %s", __FUNCTION__, mRespSuppSvcNoti.notificationType == 0 ? "MO intermediate result" : "MT unsolicited result");
    RilLogV("[%s] code : %d", __FUNCTION__, mRespSuppSvcNoti.code);
    RilLogV("[%s] index : %d", __FUNCTION__, mRespSuppSvcNoti.index);
    RilLogV("[%s] type : %d", __FUNCTION__, mRespSuppSvcNoti.type);
    RilLogV("[%s] number : %s", __FUNCTION__, mRespSuppSvcNoti.number);

    OnUnsolicitedResponse(RIL_UNSOL_SUPP_SVC_NOTIFICATION, &mRespSuppSvcNoti, sizeof(RIL_SuppSvcNotification));
    return 0;
}

INT32 SupplementaryService::DoQueryColp()
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildQueryColp();
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_QUERY_COLP_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 SupplementaryService::DoQueryColpDone(Message* pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolGetColpAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */
#if 0//for test in live network
    mRespInt = 2;    //unkown
    RilLogV("[SupplementaryService::%s] COLP Status : %d <-- for temporary test", __FUNCTION__, mRespInt);
    OnRequestComplete( RIL_E_SUCCESS, &mRespInt, sizeof(int));
#else
    if (errorCode == RIL_E_SUCCESS)
    {
        mRespInt = adapter.GetColpStatus();
        RilLogV("[SupplementaryService::%s] COLP Status : %d", __FUNCTION__, mRespInt);
        OnRequestComplete(RIL_E_SUCCESS, &mRespInt, sizeof(int));
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }
#endif
    return 0;
}

INT32 SupplementaryService::DoQueryColr()
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildQueryColr();
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_QUERY_COLR_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 SupplementaryService::DoQueryColrDone(Message* pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolGetColrAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */
#if 0//for test in live network
    mRespInt = 2;    //unkown
    RilLogV("[SupplementaryService::%s] COLR Status : %d <-- for temporary test", __FUNCTION__, mRespInt);
    OnRequestComplete( RIL_E_SUCCESS, &mRespInt, sizeof(int));
#else
    if (errorCode == RIL_E_SUCCESS)
    {
        mRespInt = adapter.GetColrStatus();
        RilLogV("[SupplementaryService::%s] COLR Status : %d", __FUNCTION__, mRespInt);
        OnRequestComplete(RIL_E_SUCCESS, &mRespInt, sizeof(int));
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }
#endif
    return 0;
}

INT32 SupplementaryService::DoSendEncodedUssd()
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    StringsRequestData* pReq = (StringsRequestData*) m_pCurReqMsg->GetRequestData();

    RilLogV("[SupplementaryService] %s() Param1: %s", __FUNCTION__, pReq->GetString(0));
    BYTE dcs = (BYTE) strtol(pReq->GetString(0), NULL, 16);

    RilLogV("[SupplementaryService] %s() dcs: 0x%02X, Param2: %s", __FUNCTION__, dcs, pReq->GetString(1));
    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildSendEncodedUssd(dcs, pReq->GetString(1), m_ussdUserInitiated);
    m_ussdUserInitiated = true;
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_SS_SEND_USSD_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 SupplementaryService::OnUnsolOnSS(Message *pMsg)
{
    RilLogI("[SupplementaryService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolUnsolOnSSAdapter adapter(pModemData);
    RIL_StkCcUnsolSsResponse rsp;
    memset(&rsp, 0x0, sizeof(RIL_StkCcUnsolSsResponse));
    rsp.serviceType = (RIL_SsServiceType)adapter.GetServiceType();
    rsp.requestType = (RIL_SsRequestType)adapter.GetRequestType();
    rsp.teleserviceType = (RIL_SsTeleserviceType)adapter.GetTeleServiceType();
    rsp.serviceClass = adapter.GetServiceClass();
    rsp.result= (RIL_Errno)adapter.GetResult();
    if (adapter.GetData(rsp.ssInfo)) {
        RilLogI("[SupplementaryService] %s(), fail to get data", __FUNCTION__);
    }

    OnUnsolicitedResponse(RIL_UNSOL_ON_SS, &rsp, sizeof(RIL_StkCcUnsolSsResponse));
    return 0;
}

BOOL SupplementaryService::IsNullRequest(Message *pMsg)
{
    if (NULL == pMsg || NULL == pMsg->GetRequestData())
    {
        RilLogE("[SupplementaryService] %s, pMsg or RequestData is NULL!!", __FUNCTION__);
        return TRUE;
    }
    return FALSE;
}

BOOL SupplementaryService::IsNullResponse(Message *pMsg)
{
    if (NULL == pMsg || NULL == pMsg->GetModemData())
    {
        RilLogE("[SupplementaryService] %s, pMsg or ModemData is NULL!!", __FUNCTION__);
        return TRUE;
    }
    return FALSE;
}

bool SupplementaryService::IsPossibleToPassInRadioOffState(int request_id)
{
    switch (request_id) {
        default:
            return false;
    }
    return true;
}

/*
 *  Allowed RadioError on CardState::ABSENT is
 *   RadioError::NONE
 *   RadioError::INVALID_STATE
 *   RadioError::MODEM_ERR
 *  GeneralErrors on VTS-HIDL
 *   RadioError::RADIO_NOT_AVAILABLE (radio resetting)
 *   RadioError::REQUEST_NOT_SUPPORTED
 *   RadioError::CANCELLED
 *   RadioError::NO_MEMORY
 *   RadioError::INTERNAL_ERR
 *   RadioError::SYSTEM_ERR
 *   RadioError::SYSTEM_ERR
*/
int SupplementaryService::GetValidErrors(int errorCode)
{
    const static int allowErrors[] = {
        RIL_E_SUCCESS, RIL_E_RADIO_NOT_AVAILABLE, RIL_E_REQUEST_NOT_SUPPORTED, RIL_E_CANCELLED,
        RIL_E_NO_MEMORY, RIL_E_INTERNAL_ERR, RIL_E_SYSTEM_ERR, RIL_E_MODEM_ERR, RIL_E_INVALID_STATE};

    if (mCardState != RIL_CARDSTATE_ABSENT) return errorCode;

    int size = sizeof(allowErrors) / sizeof(allowErrors[0]);
    for (int i = 0 ; i < size ; i++)
    {
        if (errorCode == allowErrors[i])
        {
            return errorCode;
        }
    }
    return RIL_E_INTERNAL_ERR;
}

BOOL SupplementaryService::IsOperatorUsingUnknownServiceClass()
{
    String simOperator = GetSimOperatorNumeric();
    const char *simPlmn = simOperator.c_str();
    if(strncmp(simPlmn, "71610", 5) == 0)
    {
        return true;
    }
    return false;
}
