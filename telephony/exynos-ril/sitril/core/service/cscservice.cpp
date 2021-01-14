/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "cscservice.h"
#include "rilapplication.h"
#include "rillog.h"
#include "rildatabuilder.h"
#include "protocolcallbuilder.h"
#include "protocolcalladapter.h"
#include "protocolsoundbuilder.h"
#include "protocolsoundadapter.h"
#include "mcctable.h"
#include "networkservice.h"
#include "EccListLoader.h"
#include "customproductfeature.h"

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

CallId::CallId()
{
    Init();
}

void CallId::Init()
{
    for (int i = 0; i < MAX_CALL_ID_COUNT; i++)
    {
        m_CallId[i].call_id = -1;
        m_CallId[i].state = CALL_ID_STATE_INACTIVE;
    }
}

int CallId::GetCpIndex(int ApId)
{
    if (0 < ApId && ApId <= MAX_CALL_ID_COUNT)
    {
        return m_CallId[ApId-1].call_id;
    }
    return -1;
}

int CallId::GetApIndex(int CpId)
{
    for (int i = 0; i < MAX_CALL_ID_COUNT; i++)
    {
        if (m_CallId[i].call_id == CpId)
        {
            return (i + 1);
        }
    }
    return -1;
}

void CallId::SyncReady()
{
    for (int i = 0; i < MAX_CALL_ID_COUNT; i++)
    {
        m_CallId[i].state = CALL_ID_STATE_INACTIVE;
    }
}

int CallId::AddCallId(int CpId)
{
    for (int i = 0; i < MAX_CALL_ID_COUNT; i++)
    {
        if (m_CallId[i].call_id == CpId)
        {
            m_CallId[i].state = CALL_ID_STATE_ACTIVE;
            return (i + 1);
        }
    }

    for (int i = 0; i < MAX_CALL_ID_COUNT; i++)
    {
        if (m_CallId[i].call_id == -1)
        {
            m_CallId[i].call_id = CpId;
            m_CallId[i].state = CALL_ID_STATE_ACTIVE;
            return (i + 1);
        }
    }
    return -1;
}

void CallId::SyncDone()
{
    for (int i = 0; i < MAX_CALL_ID_COUNT; i++)
    {
        RilLogV("[%d]th Call ID : %d, state(%d)", i + 1, m_CallId[i].call_id, m_CallId[i].state);
        if (m_CallId[i].state == CALL_ID_STATE_INACTIVE)
        {
            m_CallId[i].call_id = -1;
        }
    }
}

CscService::CscService(RilContext *pRilContext)
    : Service(pRilContext, RIL_SERVICE_CSC)
{
    m_bCallConfirm = false;
    m_currCallList = NULL;

    for (unsigned int i = 0; i < sizeof(mRespCallsData) / sizeof(RIL_Call); i++)
    {
        memset(&mRespCallsData[i], 0x00, sizeof(mRespCallsData[i]));
    }
    for (unsigned int i = 0; i < sizeof(mRespCallsData_V1_2) / sizeof(RIL_Call_V1_2); i++)
    {
        memset(&mRespCallsData_V1_2[i], 0x00, sizeof(mRespCallsData_V1_2[i]));
    }


    mRespInt = 0;

    for (unsigned int i = 0; i < sizeof(mRespCalls)/sizeof(RIL_Call *); i++)
    {
        mRespCalls[i] = NULL;
    }
    for (unsigned int i = 0; i < sizeof(mRespCalls_V1_2)/sizeof(RIL_Call_V1_2 *); i++)
    {
        mRespCalls_V1_2[i] = NULL;
    }


    mCardState = RIL_CARDSTATE_ABSENT;
    mAppState = RIL_APPSTATE_UNKNOWN;

#ifdef SUPPORT_CDMA
    memset(&mRespCdmaCallWaitingNoti, 0, sizeof(mRespCdmaCallWaitingNoti));
    memset(&mRespCdmaInfoRecsNoti, 0, sizeof(mRespCdmaInfoRecsNoti));
#endif // SUPPORT_CDMA

    m_emcInfoListFromRadio.clear();
}

CscService::~CscService()
{
    if (m_currCallList)
    {
        delete m_currCallList;
        m_currCallList = NULL;
    }
}

int CscService::OnCreate(RilContext *pRilContext)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);

    m_currCallList = new CallList();
    m_currCallList->Clear();

    m_CallId.Init();

    return 0;
}

BOOL CscService::OnHandleRequest(Message *pMsg)
{
    INT32 nRet = -1;
    if (NULL == pMsg)
    {
        return FALSE;
    }

    switch (pMsg->GetMsgId())
    {
    /* CS : Call */
    case MSG_CS_CALL_LIST:
        nRet = DoGetCallList();
        break;
    case MSG_CS_CALL_DIAL:
        nRet = DoDial();
        break;
    case MSG_CS_CALL_EMERGENCY_DIAL:
        nRet = DoEmergencyDial();
        break;
    case MSG_CS_CALL_ANSWER:
        nRet = DoAnswer();
        break;
    case MSG_CS_SS_EXPLICIT_CALL_TRANSFER:
        nRet = DoExplicitCallTransfer();
        break;
    case MSG_CS_CALL_HANGUP:
        nRet = DoHangup(pMsg);
        break;
    case MSG_CS_LAST_CALL_FAIL_CAUSE:
        nRet = DoLastCallFailCause();
        break;
#ifdef SUPPORT_CDMA
    case MSG_CS_CDMA_BURST_DTMF:
        nRet = DoCdmaBurstDtmf();
        break;
    case MSG_CS_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE:
        nRet = DoCdmaSetPreferredVoicePrivacyMode();
        break;
    case MSG_CS_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE:
        nRet = DoCdmaQueryPreferredVoicePrivacyMode();
        break;
#endif

    /* CS : Supplementary Service */
    case MSG_CS_SS_UDUB:
        nRet = DoUdub();
        break;
    case MSG_CS_SS_HANGUP_FOREGROUND_RESUME_BACKGROUND:
        nRet = DoHangupFgResumeBg();
        break;
    case MSG_CS_SS_HANGUP_WAITING_OR_BACKGROUND:
        nRet = DoHangupWaitOrBg();
        break;
    case MSG_CS_SS_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
        nRet = DoSwitchWaitOrHoldAndActive();
        break;
    case MSG_CS_SS_CONFERENCE:
        nRet = DoConference();
        break;

#ifdef SUPPORT_CDMA
    case MSG_CS_SS_CDMA_FLASH:
        nRet = DoCdmaFlash();
        break;
#endif

    case MSG_CS_SS_SEPARATE_CONNECTION:
        nRet = DoSeparateConnection();
        break;
    case MSG_CS_SOUND_SWITCH_VOICE_CALL:
        nRet = DoSwitchVoiceCallAudio();
        break;
    case MSG_CS_SET_CALL_CONFIRM:
        nRet = DoSetCallConfirm();
        break;
    case MSG_CS_SEND_CALL_CONFIRM:
        nRet = DoSendCallConfirm();
        break;
    case MSG_CS_EXIT_EMERGENCY_CB_MODE:
        nRet = DoExitEmergencyCbMode(pMsg);
        break;

    /* CS : Sound */
    case MSG_CS_SOUND_SET_MUTE:
        nRet = DoSetMute();
        break;
    case MSG_CS_SOUND_GET_MUTE:
        nRet = DoGetMute();
        break;
    default:
        break;
    }

    if (0 == nRet)
        return TRUE;
    else
        return FALSE;
}

BOOL CscService::OnHandleSolicitedResponse(Message *pMsg)
{
    INT32 nRet = -1;

    if (NULL == pMsg)
        return FALSE;

    switch (pMsg->GetMsgId())
    {
    /* CS : Call */
    case MSG_CS_CALL_LIST_DONE:
        nRet = DoGetCallListDone(pMsg);
        break;
    case MSG_CS_CALL_DIAL_DONE:
        nRet = DoDialDone(pMsg);
        break;
    case MSG_CS_CALL_EMERGENCY_DIAL_DONE:
        nRet = DoEmergencyDialDone(pMsg);
        break;
    case MSG_CS_CALL_ANSWER_DONE:
        nRet = DoAnswerDone(pMsg);
        break;
    case MSG_CS_SS_EXPLICIT_CALL_TRANSFER_DONE:
        nRet = DoExplicitCallTransferDone(pMsg);
        break;
    case MSG_CS_CALL_HANGUP_DONE:
        nRet = DoHangupDone(pMsg);
        break;
    case MSG_CS_LAST_CALL_FAIL_CAUSE_DONE:
        nRet = DoLastCallFailCauseDone(pMsg);
        break;

#ifdef SUPPORT_CDMA
    case MSG_CS_CDMA_BURST_DTMF_DONE:
        nRet = DoCdmaBurstDtmfDone(pMsg);
        break;
    case MSG_CS_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE_DONE:
        nRet = DoCdmaSetPreferredVoicePrivacyModeDone(pMsg);
        break;
    case MSG_CS_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE_DONE:
        nRet = DoCdmaQueryPreferredVoicePrivacyModeDone(pMsg);
        break;
#endif

    /* CS : Supplementary Service */
    case MSG_CS_SS_UDUB_DONE:
        nRet = DoUdubDone(pMsg);
        break;
    case MSG_CS_SS_HANGUP_FOREGROUND_RESUME_BACKGROUND_DONE:
        nRet = DoHangupFgResumeBgDone(pMsg);
        break;
    case MSG_CS_SS_HANGUP_WAITING_OR_BACKGROUND_DONE:
        nRet = DoHangupWaitOrBgDone(pMsg);
        break;
    case MSG_CS_SS_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE_DONE:
        nRet = DoSwitchWaitOrHoldAndActiveDone(pMsg);
        break;
    case MSG_CS_SS_CONFERENCE_DONE:
        nRet = DoConferenceDone(pMsg);
        break;

#ifdef SUPPORT_CDMA
    case MSG_CS_SS_CDMA_FLASH_DONE:
        nRet = DoCdmaFlashDone(pMsg);
        break;
#endif

    case MSG_CS_SS_SEPARATE_CONNECTION_DONE:
        nRet = DoSeparateConnectionDone(pMsg);
        break;
    case MSG_CS_SET_CALL_CONFIRM_DONE:
        nRet = DoSetCallConfirmDone(pMsg);
        break;
    case MSG_CS_SEND_CALL_CONFIRM_DONE:
        nRet = DoSendCallConfirmDone(pMsg);
        break;
    case MSG_CS_EXIT_EMERGENCY_CB_MODE_DONE:
        nRet = DoExitEmergencyCbModeDone(pMsg);
        break;

    /* CS : Sound */
    case MSG_CS_SOUND_SET_MUTE_DONE:
        nRet = DoSetMuteDone(pMsg);
        break;
    case MSG_CS_SOUND_GET_MUTE_DONE:
        nRet = DoGetMuteDone(pMsg);
        break;

    default:// it should add its exception handling
        RilLogE("[CscService::%s] solicited response handler is not specified. error", __FUNCTION__);
        break;
    }

    if (0 == nRet)
        return TRUE;
    else
        return FALSE;
}

BOOL CscService::OnHandleUnsolicitedResponse(Message *pMsg)
{
    if (NULL == pMsg)
        return FALSE;

    switch (pMsg->GetMsgId())
    {
    /* CS : Call */
    case MSG_CS_SOUND_RINGBACKTONE_NTF:
        OnRingbackTone(pMsg);
        break;
    case MSG_CS_CALL_STATE_CHANGE_NTF:
        OnCallStateChanged(pMsg);
        break;
    case MSG_CS_CALL_RINGING_NTF:
        OnCallRinging(pMsg);
        break;
    case MSG_NET_EMERGENCY_CALL_LIST_RECEIVED:
        OnNetEmergencyCallListReceived(pMsg);
        break;
    case MSG_CS_IND_EMERGENCY_SUPPORT_RAT_MODE:
        OnEmergencySupportRatModeNtf(pMsg);
        break;

#ifdef SUPPORT_CDMA
    case MSG_CS_IND_CDMA_INFO_REC:
        OnCdmaInfoRec(pMsg);
        break;
#endif

    /* CS : Supplementary Service */
    case MSG_CS_CALL_PRESENT_NTF:
        OnCallPresent(pMsg);
        break;

#ifdef SUPPORT_CDMA
    case MSG_CS_SS_IND_CDMA_CALL_WAITING:
        OnCdmaCallWaitingNtf(pMsg);
        break;
    case MSG_CS_IND_CDMA_OTA_PROVISION_STATUS:
        OnUnsolCdmaOtaProvisionStatus(pMsg);
        break;
#endif

    case MSG_CS_IND_ENTER_EMERGENCY_CB_MODE_CHANGED:
        OnEnterEmergencyCallbackModeChanged(pMsg);
        break;
    case MSG_CS_IND_EXIT_EMERGENCY_CB_MODE_CHANGED:
        OnExitEmergencyCallbackModeChanged(pMsg);
        break;
    case MSG_CS_IND_RESEND_IN_CALL_MUTE:
        OnUnsolResendInCallMute(pMsg);
        break;
    default:
        RilLogE("[CscService::%s] unsolicited response handler is not specified. error", __FUNCTION__);
        break;
    }

    return TRUE;
}

BOOL CscService::OnHandleInternalMessage(Message* pMsg)
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

    RilLogV("[CscService]received internal message,id=%d", nMsgId);
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

void CscService::OnImsiUpdated(const char *imsi)
{
    if (imsi == NULL || *imsi == 0 || !(strlen(imsi) >= 5)) {
        RilLogE("Invalid IMSI value. Please check it");
        return ;
    }

    // update current carrier(mcc/mnc)
    if (!MccTable::FetchCarrierForImsi(imsi, m_carrier, sizeof(m_carrier))) {
        m_carrier[0] = 0;
    }

    RilLogV("[%s] Carrier(MCC/MNC)=%s", __FUNCTION__, m_carrier);
}

void CscService::OnSimStatusChanged(int cardState, int appState)
{
    mCardState = cardState;
    mAppState = appState;

    RilLogV("[%s] Card state: %d, App state: %d", __FUNCTION__, cardState, appState);
}

INT32 CscService::DoGetCallList()
{
    RilLogI("[CscService] %s", __FUNCTION__);

    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildGetCallList();
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_CALL_LIST_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

/*
   Allowed RadioError on CardState::ABSENT is
     RadioError::NONE
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE (radio resetting)
 *   RadioError:NO_MEMORY
 *   RadioError:INTERNAL_ERR
 *   RadioError:SYSTEM_ERR
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:NO_RESOURCES
 *   RadioError:CANCELLED
 */
INT32 CscService::DoGetCallListDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);

    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    m_currCallList->Clear();
    ModemData *pModemData = pMsg->GetModemData();
    ProtocolGetCurrentCallAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    //update ACTIVE CALL cid
    RilProperty *property = GetRilContextProperty();
    if (property != NULL)
    {
        property->Put(RIL_CONTEXT_CS_ACTIVE_CID, -1);
    }

    if (errorCode != RIL_E_SUCCESS || adapter.HasValidLength() == false)
    {
        RilLogE("[CscService::::%s] Get Call List fail(errorCode:%d) or invalid length", __FUNCTION__, errorCode);
        m_currCallList->Clear();
        m_currCallList->m_nCount = 0;

        if (mCardState == RIL_CARDSTATE_ABSENT) {
            // on CardStateAbsent return should be RadioError:NONE
            // For Emergency call this has some call list
            RilLogI("[%s] %s mCardState(%d) is not Present", m_szSvcName, __FUNCTION__, mCardState);
            // Null Response is not allowed
            int nLength = 0;
            char *response = BuildCallListResponse(m_currCallList, &nLength);
            OnRequestComplete(RIL_E_SUCCESS, response, nLength);
        } else
            OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
        return 0;
    }

    m_currCallList->m_nCount = adapter.GetCallNum();
    if (m_currCallList->m_nCount >= MAX_CALL_LIST_COUNT)
    {
        RilLogE("[CscService::::%s] Call List Count invalid (%d)", __FUNCTION__, m_currCallList->m_nCount);
        m_currCallList->Clear();
        m_currCallList->m_nCount = 0;
        if (mCardState == RIL_CARDSTATE_ABSENT) {
            // on CardStateAbsent return should be RadioError:NONE
            // For Emergency call this has some call list
            RilLogI("[%s] %s mCardState(%d) is not Present", m_szSvcName, __FUNCTION__, mCardState);
            // Null Response is not allowed
            int nLength = 0;
            char *response = BuildCallListResponse(m_currCallList, &nLength);
            OnRequestComplete(RIL_E_SUCCESS, response, nLength);
        } else
            OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        return 0;
    }

    RilLogV("Current Call Count: %d", m_currCallList->m_nCount);
    writeRilEvent(m_szSvcName, __FUNCTION__, "Current Call Count: %d", m_currCallList->m_nCount);

    CallInfo *pCallInfo = m_currCallList->GetCallInfo();
    int callIdx;
    for (callIdx = 0; callIdx < m_currCallList->m_nCount; callIdx++, pCallInfo++)
    {
        if (0 >  adapter.GetCallInfo(pCallInfo, callIdx))
        {
            //TEST1:callIdx--;
            break;
        }
        RilLogV("< %d CallInfo >", callIdx);
        adapter.DebugPrintCallInfo(pCallInfo);

        //update ACTIVE CALL cid
        if (pCallInfo != NULL && pCallInfo->m_state == RIL_CALL_ACTIVE)
        {
            if (property != NULL)
            {
                RilLogV("Set Active CID=%d in Context for socket_id(%d)", callIdx, m_pRilContext->GetRilSocketId());
                property->Put(RIL_CONTEXT_CS_ACTIVE_CID, callIdx);
            }
        }
    }

    // TEST1: if there are not enough call info, just return with valid(existing) count
    /*if (callIdx != m_currCallList->m_nCount)
    {
        m_currCallList->m_nCount = callIdx;
    }*/

    m_CallId.SyncReady();

    int id = 0;
    pCallInfo = m_currCallList->GetCallInfo();
    for (int i = 0; i < adapter.GetCallNum(); i++, pCallInfo++)
    {
        id = pCallInfo->m_nIndex;
        pCallInfo->m_nIndex = m_CallId.AddCallId(id);
    }

    m_CallId.SyncDone();

    int nLength = 0;
    char *response = BuildCallListResponse(m_currCallList, &nLength);
    //char *response = BuildCallListResponseTest(m_currCallList, &nLength);
    OnRequestComplete(RIL_E_SUCCESS, response, nLength);

    return 0;
}

const char* CALL_STATE[] = {"ACTIVE", "HOLDING", "DIALING", "ALERTING", "INCOMING", "WAITING", "UNKNOWN"};

inline const char*CallStateToString(int state)
{
    if (RIL_CALL_ACTIVE <= state && state <= RIL_CALL_WAITING)
    {
        return CALL_STATE[state];
    }
    else
    {
        return CALL_STATE[RIL_CALL_WAITING+1];
    }
}

#if 0
char *testbuff = "1234";
char * CscService::BuildCallListResponseTest(CallList *data, int *len)
{
    RilLogV("[CscService] %s",__FUNCTION__);


    if (data->m_nCount <= 0)
    {
        *len = 0;
        return NULL;
    }
    else
    {
        memset(mRespCalls, 0, sizeof(mRespCalls));
        memset(mRespCallsData, 0, sizeof(mRespCallsData));

        for (int i = 0; i < 2; i++)
        {
            mRespCallsData[i].state = (RIL_CallState)data->m_szCallInfo[0].m_state;//(RIL_CallState)(data->m_pCallInfo[i].m_state);
            mRespCallsData[i].index = i==1?2:data->m_szCallInfo[i].m_nIndex;
            mRespCallsData[i].toa = data->m_szCallInfo[0].m_toa;
            mRespCallsData[i].isMpty = (data->m_szCallInfo[0].m_isMParty == true)? 1: 0;
            mRespCallsData[i].isMT = (data->m_szCallInfo[0].m_isMt == true)? 1: 0;
            mRespCallsData[i].als = data->m_szCallInfo[0].m_als;
            mRespCallsData[i].isVoice = (data->m_szCallInfo[0].m_isVoice == true)? 1: 0;
            mRespCallsData[i].isVoicePrivacy = (data->m_szCallInfo[0].m_isVoicePrivacy == true)? 1: 0;
            mRespCallsData[i].number = i==1?testbuff:data->m_szCallInfo[0].m_number;
            mRespCallsData[i].numberPresentation = data->m_szCallInfo[0].m_numPresent;
            if (data->m_szCallInfo[i].m_name[0] != '\0')
            {
                mRespCallsData[i].name = data->m_szCallInfo[i].m_name;
            }
            mRespCallsData[i].namePresentation = data->m_szCallInfo[0].m_namePresent;
            mRespCallsData[i].uusInfo = NULL;

            mRespCalls[i] = &mRespCallsData[i];

            RilLogV("-------- %d / %d --------", i+1, data->m_nCount);
            RilLogV("Call list %d: index %d, state %s(%d), TOA %d, isMpty %d",
                i, mRespCallsData[i].index, CallStateToString(mRespCallsData[i].state), mRespCallsData[i].state, mRespCallsData[i].toa, mRespCallsData[i].isMpty);
            RilLogV("isMT %d, als %d, isVoice %d, isVoicePrivacy %d, ",
                mRespCallsData[i].isMT, mRespCallsData[i].als, mRespCallsData[i].isVoice, mRespCallsData[i].isVoicePrivacy);
            RilLogV("number %s, numberPresent %d, name %s, namePresent %d, uusInfo ...",
                mRespCallsData[i].number, mRespCallsData[i].numberPresentation, mRespCallsData[i].name,
                mRespCallsData[i].namePresentation);
            RilLogV("----------------------");
        }
        *len = 2 * sizeof(RIL_Call *);
        return (char *)mRespCalls;

    }
}
#endif

char * CscService::BuildCallListResponse(CallList *data, int *len)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    unsigned int nSkipCallCnt = 0;

    if (data->m_nCount <= 0)
    {
        *len = 0;
        return NULL;
    }
    else
    {
        if (RilApplication::RIL_HalVersionCode >= HAL_VERSION_CODE(1, 2)) {
            memset(mRespCalls_V1_2, 0, sizeof(mRespCalls_V1_2));
            memset(mRespCallsData_V1_2, 0, sizeof(mRespCallsData_V1_2));

            for (int i = 0; i < data->m_nCount; i++)
            {
                mRespCallsData_V1_2[i].state = (RIL_CallState)data->m_szCallInfo[i].m_state;//(RIL_CallState)(data->m_pCallInfo[i].m_state);
                mRespCallsData_V1_2[i].index = data->m_szCallInfo[i].m_nIndex;
                mRespCallsData_V1_2[i].toa = data->m_szCallInfo[i].m_toa;
                mRespCallsData_V1_2[i].isMpty = (data->m_szCallInfo[i].m_isMParty == true)? 1: 0;
                mRespCallsData_V1_2[i].isMT = (data->m_szCallInfo[i].m_isMt == true)? 1: 0;
                mRespCallsData_V1_2[i].als = data->m_szCallInfo[i].m_als;
                mRespCallsData_V1_2[i].isVoice = (data->m_szCallInfo[i].m_isVoice == true)? 1: 0;
                mRespCallsData_V1_2[i].isVoicePrivacy = (data->m_szCallInfo[i].m_isVoicePrivacy == true)? 1: 0;
                mRespCallsData_V1_2[i].number = data->m_szCallInfo[i].m_number;
                mRespCallsData_V1_2[i].numberPresentation = data->m_szCallInfo[i].m_numPresent;
                if (data->m_szCallInfo[i].m_name[0] != '\0')
                {
                    mRespCallsData_V1_2[i].name = data->m_szCallInfo[i].m_name;
                }
                mRespCallsData_V1_2[i].namePresentation = data->m_szCallInfo[i].m_namePresent;
                mRespCallsData_V1_2[i].uusInfo = NULL;
                mRespCallsData_V1_2[i].audioQuality = (AudioQuality)data->m_szCallInfo[i].m_audioQuality;

                mRespCalls_V1_2[i] = &mRespCallsData_V1_2[i];

                RilLogV("-------- %d / %d --------", i+1, data->m_nCount);
                RilLogV("Call list %d: index %d, state %s(%d), TOA %d, isMpty %d",
                    i, mRespCallsData_V1_2[i].index, CallStateToString(mRespCallsData_V1_2[i].state), mRespCallsData_V1_2[i].state, mRespCallsData_V1_2[i].toa, mRespCallsData_V1_2[i].isMpty);
                RilLogV("isMT %d, als %d, isVoice %d, isVoicePrivacy %d, ",
                    mRespCallsData_V1_2[i].isMT, mRespCallsData_V1_2[i].als, mRespCallsData_V1_2[i].isVoice, mRespCallsData_V1_2[i].isVoicePrivacy);
                RilLogV("number %s, numberPresent %d, name %s, namePresent %d, uusInfo ...",
                    mRespCallsData_V1_2[i].number, mRespCallsData_V1_2[i].numberPresentation, mRespCallsData_V1_2[i].name,
                    mRespCallsData_V1_2[i].namePresentation);
                RilLog("----------------------");
                writeRilEvent(m_szSvcName, __FUNCTION__, "-------- %d / %d --------", i+1, data->m_nCount);
                writeRilEvent(m_szSvcName, __FUNCTION__, "Call list %d: index %d, state %s(%d), TOA %d, isMpty %d", i, mRespCallsData_V1_2[i].index, CallStateToString(mRespCallsData_V1_2[i].state), mRespCallsData_V1_2[i].state, mRespCallsData_V1_2[i].toa, mRespCallsData_V1_2[i].isMpty);
                writeRilEvent(m_szSvcName, __FUNCTION__, "isMT %d, als %d, isVoice %d, isVoicePrivacy %d, ", mRespCallsData_V1_2[i].isMT, mRespCallsData_V1_2[i].als, mRespCallsData_V1_2[i].isVoice, mRespCallsData_V1_2[i].isVoicePrivacy);
                writeRilEvent(m_szSvcName, __FUNCTION__, "number %s, numberPresent %d, name %s, namePresent %d, uusInfo ...", mRespCallsData_V1_2[i].number, mRespCallsData_V1_2[i].numberPresentation, mRespCallsData_V1_2[i].name, mRespCallsData_V1_2[i].namePresentation);
                writeRilEvent(m_szSvcName, __FUNCTION__, "----------------------");
            }
            *len = (data->m_nCount - nSkipCallCnt) * sizeof(RIL_Call_V1_2 *);
            return (char *)mRespCalls_V1_2;
        } else {
            memset(mRespCalls, 0, sizeof(mRespCalls));
            memset(mRespCallsData, 0, sizeof(mRespCallsData));

            for (int i = 0; i < data->m_nCount; i++)
            {
                mRespCallsData[i].state = (RIL_CallState)data->m_szCallInfo[i].m_state;//(RIL_CallState)(data->m_pCallInfo[i].m_state);
                mRespCallsData[i].index = data->m_szCallInfo[i].m_nIndex;
                mRespCallsData[i].toa = data->m_szCallInfo[i].m_toa;
                mRespCallsData[i].isMpty = (data->m_szCallInfo[i].m_isMParty == true)? 1: 0;
                mRespCallsData[i].isMT = (data->m_szCallInfo[i].m_isMt == true)? 1: 0;
                mRespCallsData[i].als = data->m_szCallInfo[i].m_als;
                mRespCallsData[i].isVoice = (data->m_szCallInfo[i].m_isVoice == true)? 1: 0;
                mRespCallsData[i].isVoicePrivacy = (data->m_szCallInfo[i].m_isVoicePrivacy == true)? 1: 0;
                mRespCallsData[i].number = data->m_szCallInfo[i].m_number;
                mRespCallsData[i].numberPresentation = data->m_szCallInfo[i].m_numPresent;
                if (data->m_szCallInfo[i].m_name[0] != '\0')
                {
                    mRespCallsData[i].name = data->m_szCallInfo[i].m_name;
                }
                mRespCallsData[i].namePresentation = data->m_szCallInfo[i].m_namePresent;
                mRespCallsData[i].uusInfo = NULL;

                mRespCalls[i] = &mRespCallsData[i];

                RilLogV("-------- %d / %d --------", i+1, data->m_nCount);
                RilLogV("Call list %d: index %d, state %s(%d), TOA %d, isMpty %d",
                    i, mRespCallsData[i].index, CallStateToString(mRespCallsData[i].state), mRespCallsData[i].state, mRespCallsData[i].toa, mRespCallsData[i].isMpty);
                RilLogV("isMT %d, als %d, isVoice %d, isVoicePrivacy %d, ",
                    mRespCallsData[i].isMT, mRespCallsData[i].als, mRespCallsData[i].isVoice, mRespCallsData[i].isVoicePrivacy);
                RilLogV("number %s, numberPresent %d, name %s, namePresent %d, uusInfo ...",
                    mRespCallsData[i].number, mRespCallsData[i].numberPresentation, mRespCallsData[i].name,
                    mRespCallsData[i].namePresentation);
                RilLog("----------------------");
                writeRilEvent(m_szSvcName, __FUNCTION__, "-------- %d / %d --------", i+1, data->m_nCount);
                writeRilEvent(m_szSvcName, __FUNCTION__, "Call list %d: index %d, state %s(%d), TOA %d, isMpty %d", i, mRespCallsData[i].index, CallStateToString(mRespCallsData[i].state), mRespCallsData[i].state, mRespCallsData[i].toa, mRespCallsData[i].isMpty);
                writeRilEvent(m_szSvcName, __FUNCTION__, "isMT %d, als %d, isVoice %d, isVoicePrivacy %d, ", mRespCallsData[i].isMT, mRespCallsData[i].als, mRespCallsData[i].isVoice, mRespCallsData[i].isVoicePrivacy);
                writeRilEvent(m_szSvcName, __FUNCTION__, "number %s, numberPresent %d, name %s, namePresent %d, uusInfo ...", mRespCallsData[i].number, mRespCallsData[i].numberPresentation, mRespCallsData[i].name, mRespCallsData[i].namePresentation);
                writeRilEvent(m_szSvcName, __FUNCTION__, "----------------------");
            }
            *len = (data->m_nCount - nSkipCallCnt) * sizeof(RIL_Call *);
            return (char *)mRespCalls;
        }
    }
}

INT32 CscService::DoDial()
{
    RilLogI("[CscService] %s", __FUNCTION__);

    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    CallDialReqData* pCallReqData = (CallDialReqData*)m_pCurReqMsg->GetRequestData();

    CallType cType = CALL_TYPE_VOICE;
    int currentRat = RADIO_TECH_UNKNOWN;
    RilProperty *property = GetRilContextProperty();
    if (property != NULL)
    {
        currentRat = property->GetInt(RIL_CONTEXT_NET_VOICE_RADIO_TECH);
        if(isCdmaVoice(currentRat))
        {
            cType = CALL_TYPE_CDMA_VOICE;
        }
    }

    RilLogV("[CscService] Dial To : %s , Type(%s)", pCallReqData->GetNumber(),
            GetCallTypeString(cType));
    ProtocolCallBuilder builder;

    UINT eccCat = 0;    //default emergency center

    ClirType dialClirType = pCallReqData->GetClirType();
    if (dialClirType == CLIR_DEFAULT)
    {
        dialClirType = (ClirType)m_clirInfo.GetClirAoc(GetRilSocketId());
    }
    UusInfo uusInfo = pCallReqData->GetUusInfo();
    ModemData *pModemData = builder.BuildDial(pCallReqData->GetNumber(), dialClirType, uusInfo, cType, eccCat);

    writeRilEvent(m_szSvcName, __FUNCTION__, "num(%s), dialClirType(%s), calltype(%s), eccCat(%d)", pCallReqData->GetNumber(), dialClirType == CLIR_DEFAULT? "Default" : dialClirType == CLIR_INVOCATION ? "Invocation" : "Supperssion");

    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_CALL_DIAL_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoDialDone(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);

    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
   Allowed RadioError on CardState::ABSENT is
 *   RadioError::INVALID_ARGUMENTS
 *   RadioError::INVALID_STATE
 *   RadioError::MODEM_ERR
 *   RadioError::FDN_CHECK_FAILURE
 *   RadioError::NO_SUBSCRIPTION
 *   RadioError::NO_NETWORK_FOUND
 *   RadioError::INVALID_CALL_ID
 *   RadioError::DEVICE_IN_USE
 *   RadioError::OPERATION_NOT_ALLOWED
 *   RadioError::INVALID_MODEM_STATE
 *   RadioError::CANCELLED
 *   GeneralErrors on VTS-HIDL
 *    RadioError::RADIO_NOT_AVAILABLE (radio resetting)
 *    RadioError::NO_MEMORY
 *    RadioError::INTERNAL_ERR
 *    RadioError::SYSTEM_ERR
 *    RadioError::REQUEST_NOT_SUPPORTED
 *    RadioError::CANCELLED
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE (radio resetting)
 *   RadioError:DIAL_MODIFIED_TO_USSD
 *   RadioError:DIAL_MODIFIED_TO_SS
 *   RadioError:DIAL_MODIFIED_TO_DIAL
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:NO_MEMORY
 *   RadioError:INVALID_STATE
 *   RadioError:NO_RESOURCES
 *   RadioError:INTERNAL_ERR
 *   RadioError:FDN_CHECK_FAILURE
 *   RadioError:MODEM_ERR
 *   RadioError:NO_SUBSCRIPTION
 *   RadioError:NO_NETWORK_FOUND
 *   RadioError:INVALID_CALL_ID
 *   RadioError:DEVICE_IN_USE
 *   RadioError:OPERATION_NOT_ALLOWED
 *   RadioError:ABORTED
 *   RadioError:SYSTEM_ERR
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:INVALID_MODEM_STATE
 *   RadioError:CANCELLED
 */
    RilLogI("[%s] %s mCardState(%d) errorCode=%d", m_szSvcName, __FUNCTION__, mCardState, errorCode);

    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%s)", errorCode == RIL_E_SUCCESS ? "SUCCESS" : errorCode == RIL_E_RADIO_NOT_AVAILABLE? "RADIO_NOT_AVAILABLE" : "INTERNAL_ERR");
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}

bool CscService::isCdmaVoice(int rat)
{
    switch(rat) {
        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
        case RADIO_TECH_1xRTT:
        case RADIO_TECH_EVDO_0:
        case RADIO_TECH_EVDO_A:
        case RADIO_TECH_EVDO_B:
        case RADIO_TECH_EHRPD:
            break;
        default:
            return false;
    }
    return true;
}

INT32 CscService::DoEmergencyDial()
{
    RilLogI("[CCscService] %s",__FUNCTION__);

    if(IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    CallEmergencyDialReqData* pECallReqData = (CallEmergencyDialReqData*)m_pCurReqMsg->GetRequestData();

    char* number = pECallReqData->GetNumber();
    CallType cType = CALL_TYPE_EMERGENCY;
    int currentRat = RADIO_TECH_UNKNOWN;
    RilProperty *property = GetRilContextProperty();
    if (property != NULL)
    {
        currentRat = property->GetInt(RIL_CONTEXT_NET_VOICE_RADIO_TECH);
        if(isCdmaVoice(currentRat))
        {
            cType = CALL_TYPE_CDMA_EMERGENCY;
        }
    }

    ProtocolCallBuilder builder;

    UINT eccCat = pECallReqData->GetCategories();
    RilLogV("[%s] EmergencyDial To : %s, Type(%s), category (%d)", __FUNCTION__,
            number, GetCallTypeString(cType), eccCat);

    ClirType dialClirType = pECallReqData->GetClirType();
    if ( dialClirType == CLIR_DEFAULT )
    {
        dialClirType = (ClirType)m_clirInfo.GetClirAoc(GetRilSocketId());
    }
    UusInfo uusInfo = pECallReqData->GetUusInfo();
    ModemData *pModemData = builder.BuildDial(number, dialClirType, uusInfo, cType, eccCat);

    writeRilEvent(m_szSvcName, __FUNCTION__, "num(%s), dialClirType(%s)", number,
     dialClirType == CLIR_DEFAULT? "Default" : dialClirType == CLIR_INVOCATION ? "Invocation" : "Supperssion");

    if ( SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_CALL_EMERGENCY_DIAL_DONE) < 0 )
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoEmergencyDialDone(Message *pMsg)
{
    RilLogI("[CCscService] %s",__FUNCTION__);

    if(IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    RilLogI("[%s] %s mCardState(%d) errorCode=%d",
            m_szSvcName, __FUNCTION__, mCardState, errorCode);

    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%s)", errorCode == RIL_E_SUCCESS ? "SUCCESS" : errorCode == RIL_E_RADIO_NOT_AVAILABLE? "RADIO_NOT_AVAILABLE" : "INTERNAL_ERR");
    if ( errorCode == RIL_E_SUCCESS )
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}

UINT CscService::ReturnEmergencyCategory(const char* pNumber)
{
    if (pNumber == NULL || strlen(pNumber) == 0)
    {
        return 0;
    }

    // get carrier
    RilLogV("get carrier = %s", m_carrier);
    if (strncmp(m_carrier, "460", 3) == 0) //China network
    {
        RilLogV("checking ecc category for China");
        if (strncmp(pNumber, "110", 3) == 0)
        {
            return 0x01; // CALL_EMERGENCY_CALL_SUBTYPE_POLICE
        }
        else if (strncmp(pNumber, "120", 3) == 0)
        {
            return 0x02; // CALL_EMERGENCY_CALL_SUBTYPE_AMBULANCE
        }
        else if (strncmp(pNumber, "119", 3) == 0)
        {
            return 0x04; // CALL_EMERGENCY_CALL_SUBTYPE_FIRE_BRIGADE
        }
    } else if (strncmp(m_carrier, "730", 3) == 0) { // CL
        RilLogV("checking ecc category for CL");
        if (strncmp(pNumber, "131", 3) == 0)
        {
            return 0x02; // CALL_EMERGENCY_CALL_SUBTYPE_AMBULANCE
        }
        else if (strncmp(pNumber, "132", 3) == 0)
        {
            return 0x04; // CALL_EMERGENCY_CALL_SUBTYPE_FIRE_BRIGADE
        }
        else if (strncmp(pNumber, "133", 3) == 0)
        {
            return 0x01; // CALL_EMERGENCY_CALL_SUBTYPE_POLICE
        }
    }

    return 0;
}

INT32 CscService::DoAnswer()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildAnswer();
    writeRilEvent(m_szSvcName, __FUNCTION__);
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_CALL_ANSWER_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoAnswerDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
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
 */
    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%s)", errorCode == RIL_E_SUCCESS ? "SUCCESS" : errorCode == RIL_E_RADIO_NOT_AVAILABLE ? "RADIO_NOT_AVAILABLE" : "INTERNAL_ERR");
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}

INT32 CscService::DoExplicitCallTransfer()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }
    // On Sim absent, requesting command into CP cause IPC TIMEOUT
    // So return early,
    if (mCardState != RIL_CARDSTATE_PRESENT) {
        RilLogI("[%s] %s mCardState(%d) is not Present(%d)", m_szSvcName, __FUNCTION__, mCardState, RIL_CARDSTATE_PRESENT);
        OnRequestComplete(RIL_E_INVALID_STATE);
        return 0;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildExplicitCallTransfer();
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_CS_SS_EXPLICIT_CALL_TRANSFER_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoExplicitCallTransferDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * Allowed RadioError on CardState::ABSENT is
 *   RadioError::INVALID_STATE
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
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:INVALID_STATE
 *   RadioError:NO_RESOURCES
 *   RadioError:NO_MEMORY
 *   RadioError:SYSTEM_ERR
 *   RadioError:MODEM_ERR
 *   RadioError:INTERNAL_ERR
 *   RadioError:INVALID_CALL_ID
 *   RadioError:OPERATION_NOT_ALLOWED
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
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}

INT32 CscService::DoHangup(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(pMsg))
    {
        return -1;
    }

    IntRequestData *data = (IntRequestData *) pMsg->GetRequestData();
    writeRilEvent(m_szSvcName, __FUNCTION__, "CallCount(%d)", m_currCallList->m_nCount);

    if (m_currCallList->m_nCount < 1)
    {
        OnRequestComplete(RIL_E_INTERNAL_ERR);
        return 0;
    }
    else if (m_currCallList->m_nCount== 1)
    {
        return DoReleaseCall(m_CallId.GetCpIndex(data->GetInt()), pMsg);
    }
    else
    {
        return DoReleaseCallMulti(m_CallId.GetCpIndex(data->GetInt()), pMsg);
    }
}

INT32 CscService::DoReleaseCall(int callId, Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);

    if (IsNullRequest(pMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildHangup(callId);
    writeRilEvent(m_szSvcName, __FUNCTION__, "CallId(%d)", callId);
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_CALL_HANGUP_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoReleaseCallMulti(int callId, Message *pMsg)
{
    RilLogI("[CscService] %s",__FUNCTION__);

    if(IsNullRequest(pMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildHangupMulti(callId);
    writeRilEvent(m_szSvcName, __FUNCTION__, "CallId(%d)", callId);
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_CALL_HANGUP_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoHangupDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
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
 */
    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%s)", errorCode == RIL_E_SUCCESS ? "SUCCESS" : errorCode == RIL_E_RADIO_NOT_AVAILABLE? "RADIO_NOT_AVAILABLE" : "INTERNAL_ERR");
    if (errorCode == RIL_E_SUCCESS)
    {
        // clear released call information before updating by framework
        //
        // VTS test runtime doesn't guarantee calling getCurrentCalls
        // even if received RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED
        // VtsHalRadioV1_4Target#RadioHidlTest_v1_4.setPreferredNetwrokTypeBitmap
        // can be affected and the result become failed.
        IntRequestData *requestData = (IntRequestData *)GetCurrentReqeustData();
        if (requestData != NULL) {
            int callIdex = requestData->GetInt() - 1;
            RilLogV("[%d] clear released call info: callId=%d", GetRilSocketId(), callIdex);
            int numOfCalls = (m_currCallList != NULL) ? m_currCallList->GetCount() : 0;
            if (m_currCallList != NULL && numOfCalls > 0) {
                // clear CallInfo
                if (callIdex >= 0 && callIdex < numOfCalls) {
                    CallInfo &currentCall = m_currCallList->GetCallInfo()[callIdex];
                    currentCall.Clear();
                    // decrease active call count
                    m_currCallList->m_nCount--;
                }
            }
        }
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}


INT32 CscService::DoLastCallFailCause()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildLastCallFailCause();
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_LAST_CALL_FAIL_CAUSE_DONE) < 0 )
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoLastCallFailCauseDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolGetLastCallFailCauseAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
/*
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        mRespInt = adapter.GetLastCallFailCause();
        OnRequestComplete(RIL_E_SUCCESS, &mRespInt, sizeof(int));
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}

#ifdef SUPPORT_CDMA
INT32 CscService::DoCdmaBurstDtmf()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }
    // On Sim absent, requesting command into CP cause IPC TIMEOUT
    // So return early,
    if(mCardState != RIL_CARDSTATE_PRESENT) {
        RilLogI("[%s] %s mCardState(%d) is not Present(%d)", m_szSvcName, __FUNCTION__, mCardState, RIL_CARDSTATE_PRESENT);
        OnRequestComplete(RIL_E_INVALID_STATE);
        return 0;
    }

    StringsRequestData* pReq = (StringsRequestData*) m_pCurReqMsg->GetRequestData();

    ProtocolCallBuilder builder;

    RilLogV("[%s] DTMF string:%s, on_length:%s, off_length:%s", __FUNCTION__, pReq->GetString(0), pReq->GetString(1), pReq->GetString(2));
    ModemData *pModemData = builder.BuildCdmaBurstDtmf(strlen(pReq->GetString(0)), pReq->GetString(0),
            strlen(pReq->GetString(1)), pReq->GetString(1), strlen(pReq->GetString(2)), pReq->GetString(2));
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_CDMA_BURST_DTMF_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoCdmaBurstDtmfDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * Allowed RadioError on CardState::ABSENT are
 *   RadioError::NONE
 *   RadioError:INVALID_CALL_ID
 *   RadioError:MODEM_ERR
 *   RadioError:INVALID_MODEM_STATE
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
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:NO_RESOURCES
 *   RadioError:NO_MEMORY
 *   RadioError:MODEM_ERR
 *   RadioError:INVALID_CALL_ID
 *   RadioError:INTERNAL_ERR
 *   RadioError:SYSTEM_ERR
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:CANCELLED
 *   RadioError:INVALID_MODEM_STATE
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}
#endif

INT32 CscService::OnRingbackTone(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSoundRingbackToneIndAdapter adapter(pModemData);
    int ringback_state = adapter.GetRingbackToneState();
    // legacy if flags is negative value
    int flag = adapter.GetFlag();
    RilLogV("[%s] ringback_state=%d flag=%d", GetServiceName(), ringback_state, flag);

    // notify ringback tone status except call altering state
    if (!(flag == RINGBACK_FLAG_PLAY_INBAND_BY_NW &&
            ringback_state == RIL_SND_RINGBACK_TONE_START)) {
        mRespInt = ringback_state;
        OnUnsolicitedResponse(RIL_UNSOL_RINGBACK_TONE, (char *)&mRespInt, sizeof(mRespInt));
    }

    // notify ringback tone to OEM with flag
    if (flag == RINGBACK_FLAG_PLAY_INBAND_BY_NW) {
        int resp[] = {ringback_state, flag};
        OnUnsolicitedResponse(RIL_UNSOL_OEM_RINGBACK_TONE_BY_NETWORK, resp, sizeof(resp));
    }

    return 0;
}

INT32 CscService::OnCallStateChanged(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    writeRilEvent(m_szSvcName, __FUNCTION__);
    OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED);
    return 0;
}

INT32 CscService::OnCallRinging(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    writeRilEvent(m_szSvcName, __FUNCTION__);
    OnUnsolicitedResponse(RIL_UNSOL_CALL_RING);
    OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED);

    return 0;
}

int CscService::OnNetEmergencyCallListReceived(Message *pMsg)
{
    int i;
    RilLogI("[CscService] %s", __FUNCTION__);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    m_emcInfoListFromRadio.clear();

    ProtocolEmergencyCallListIndAdapter adapter(pModemData);

    if (adapter.HasValidLength() == false) {
        RilLogE("[CscService] %s  indication has invalid length", __FUNCTION__);
        return -1;
    }

    const char* mcc = adapter.GetMcc();
    const char* mnc = adapter.GetMnc();
    int num = adapter.GetNum();

    for (i=0; i<num; i++) {
        EmcInfo emcInfo;
        memset(&emcInfo, 0, sizeof(emcInfo));
        if (adapter.GetEmcInfo(emcInfo, i)) {
            RilLogV("[%s] EmcInfo: category=%d, number_len=%d, number=%s, source=%d", __FUNCTION__,
                    emcInfo.m_category, emcInfo.m_number_len, emcInfo.m_number, emcInfo.m_source);
            EmcInfo* emcInfoArray = m_emcInfoListFromRadio.getEmcInfoList();
            emcInfoArray[i].update(mcc, mnc, emcInfo.m_number, emcInfo.m_number_len,
                        emcInfo.m_category, SIT_EMERGENCY_CONDITION_ALWAYS, emcInfo.m_source);
        }
        if (i == num-1) break;
    }
    m_emcInfoListFromRadio.m_count = num;

    NetworkService* pNetworkService = (NetworkService*)GetCurrentService(RIL_SERVICE_NETWORK);
    if (pNetworkService != NULL) {
        pNetworkService->mergeAndUpdateEmergencyNumberList();
    }

    return 0;
}

INT32 CscService::OnEmergencySupportRatModeNtf(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolEmergencySupportRatModeIndAdapter adapter(pModemData);
    int supportRatMode = adapter.GetSupportRatMode();
    RilLogV("Support RAT Mode=%d", supportRatMode);
    writeRilEvent(m_szSvcName, __FUNCTION__, "Support RAT Mode=%d", supportRatMode);

    OnUnsolicitedResponse(RIL_UNSOL_EMERGENCY_SUPPORT_RAT_MODE, &supportRatMode, sizeof(int) * 1);

    return 0;
}

#ifdef SUPPORT_CDMA
INT32 CscService::DoCdmaSetPreferredVoicePrivacyMode()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    IntRequestData* pReq = (IntRequestData*) m_pCurReqMsg->GetRequestData();

    ProtocolCallBuilder builder;

    RilLogV("[%s] VPMode state: %s", __FUNCTION__, pReq->GetInt() == RIL_CALL_CDMA_VOICEPRIVACY_INACTIVE ? "Inactive" : "Active");
    ModemData *pModemData = builder.BuildCdmaSetPreferredVoicePrivacyMode(pReq->GetInt());
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoCdmaSetPreferredVoicePrivacyModeDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
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
 *  RADIO_NOT_AVAILABLE
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}

INT32 CscService::DoCdmaQueryPreferredVoicePrivacyMode()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildCdmaQueryPreferredVoicePrivacyMode();
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoCdmaQueryPreferredVoicePrivacyModeDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolGetPreferredVoicePrivacyModeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE
 */
    if(errorCode == RIL_E_SUCCESS)
    {
        mRespInt = adapter.GetPreferredVoicePrivacyStatus();
        RilLogV("[CscService] %s VoicePrivacyMode : %d", __FUNCTION__, mRespInt);
        OnRequestComplete(RIL_E_SUCCESS, &mRespInt, sizeof(int));
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}

INT32 CscService::OnCdmaInfoRec(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolCdmaInfoListIndAdapter adapter(pModemData);

    mRespCdmaInfoRecsNoti.numberOfInfoRecs = adapter.GetNumberOfInfoRecs();
    if(mRespCdmaInfoRecsNoti.numberOfInfoRecs > MAX_NUMBER_OF_INFO_RECS)
    {
        return -1;
    }

    RilLogV("CdmaInfoRecs Num: %d", mRespCdmaInfoRecsNoti.numberOfInfoRecs);

    for(int i = 0; i<mRespCdmaInfoRecsNoti.numberOfInfoRecs; i++)
    {
        memset(&mRespCdmaInfoRecsNoti.infoRec[i], 0x00, sizeof(mRespCdmaInfoRecsNoti.infoRec[i]));
        adapter.GetCdmaInfo(mRespCdmaInfoRecsNoti.infoRec[i], i);
    }

    OnUnsolicitedResponse(RIL_UNSOL_CDMA_INFO_REC, &mRespCdmaInfoRecsNoti, sizeof(RIL_CDMA_InformationRecords));

    return 0;
}
#endif

INT32 CscService::DoUdub()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildUdub();
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_CS_SS_UDUB_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoUdubDone(Message * pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
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
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}

INT32 CscService::DoHangupFgResumeBg()
{
    RilLogI("[CscService] %s", __FUNCTION__);

    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildHangupForegroundResumeBackground();
    writeRilEvent(m_szSvcName, __FUNCTION__);
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_SS_HANGUP_FOREGROUND_RESUME_BACKGROUND_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 CscService::DoHangupFgResumeBgDone(Message * pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
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
 */
    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%s)", errorCode == RIL_E_SUCCESS ? "SUCCESS" : errorCode == RIL_E_RADIO_NOT_AVAILABLE ? "RADIO_NOT_AVAILABLE" : "INTERNAL_ERR");
    if (errorCode == RIL_E_SUCCESS )
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }
    return 0;
}

INT32 CscService::DoHangupWaitOrBg()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildHangupWaitingOrBackground();
    writeRilEvent(m_szSvcName, __FUNCTION__);
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_SS_HANGUP_WAITING_OR_BACKGROUND_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 CscService::DoHangupWaitOrBgDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
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
 */
    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%s)", errorCode == RIL_E_SUCCESS ? "SUCCESS" : errorCode == RIL_E_RADIO_NOT_AVAILABLE ? "RADIO_NOT_AVAILABLE" : "INTERNAL_ERR");
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }
    return 0;
}

INT32 CscService::DoSwitchWaitOrHoldAndActive()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildSwitchWaitingOrHoldingAndActive();
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_SS_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 CscService::DoSwitchWaitOrHoldAndActiveDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
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
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }
    return 0;
}

INT32 CscService::DoConference()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildConference();
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_CS_SS_CONFERENCE_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 CscService::DoConferenceDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
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
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }
    return 0;
}

#ifdef SUPPORT_CDMA
INT32 CscService::DoCdmaFlash()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    StringRequestData* pReq = (StringRequestData*) m_pCurReqMsg->GetRequestData();
    const char *flash = pReq->GetString();
    ProtocolCallBuilder builder;

    RilLogV("[%s] FLASH string:%s", __FUNCTION__, flash);
    ModemData *pModemData = builder.BuildCdmaFlash(flash);
    if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_CS_SS_CDMA_FLASH_DONE) < 0)
    {
        return -1;
    }

    return 0;
}

INT32 CscService::DoCdmaFlashDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
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
 *  RADIO_NOT_AVAILABLE
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}
#endif

INT32 CscService::DoSeparateConnection()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        return 0;
    }

    IntRequestData *data = (IntRequestData *) m_pCurReqMsg->GetRequestData();

    if (m_currCallList->m_nCount < 2)    /* if call is less than 2, call cannot be separated */
    {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        return 0;
    }
    else
    {
        ProtocolCallBuilder builder;
        ModemData *pModemData = builder.BuildSeparateConnection(m_CallId.GetCpIndex(data->GetInt()));
        if (SendRequest(pModemData, SUPPLEMENTARY_DEFAULT_TIMEOUT, MSG_CS_SS_SEPARATE_CONNECTION_DONE) < 0)
        {
            OnRequestComplete(RIL_E_MODEM_ERR);
            return 0;
        }
    }

    return 0;
}

INT32 CscService::DoSeparateConnectionDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * Allowed RadioError on CardState::ABSENT are
 *   RadioError::INVALID_ARGUMENTS
 *   RadioError::INVALID_STATE
 *   RadioError::MODEM_ERR
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
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:INVALID_STATE
 *   RadioError:NO_RESOURCES
 *   RadioError:NO_MEMORY
 *   RadioError:MODEM_ERR
 *   RadioError:SYSTEM_ERR
 *   RadioError:INTERNAL_ERR
 *   RadioError:INVALID_CALL_ID
 *   RadioError:OPERATION_NOT_ALLOWED
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:INVALID_MODEM_STATE
 *   RadioError:CANCELLED
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_MODEM_ERR);
    }

    return 0;
}

INT32 CscService::DoSwitchVoiceCallAudio()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    IntRequestData* pMuteReqData = (IntRequestData *) m_pCurReqMsg->GetRequestData();

    ProtocolSoundBuilder builder;
    ModemData *pModemData = builder.BuildSwitchVoiceCallAudio((BYTE)(pMuteReqData->GetInt()));
    RilLogV("[CscService] <req> SIM #: %d", pMuteReqData->GetInt());
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_SOUND_SWITCH_VOICE_CALL_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 CscService::DoSwitchVoiceCallAudioDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * ///TODO: check acceptable return error code
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

INT32 CscService::DoSetCallConfirm()
{
    RilLogI("[CscService] %s",__FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }


    IntRequestData *pSetCallConfirmReqData = (IntRequestData *) m_pCurReqMsg->GetRequestData();
    ProtocolCallBuilder builder;
    int iSetMode = pSetCallConfirmReqData->GetInt();
    ModemData *pModemData = builder.BuildSetCallConfirm(iSetMode);
    RilLogV("[CscService] <req> call confirm state: %d (%s)", iSetMode, iSetMode == RIL_CALL_CONFIRM_ENABLE ? "Enabled" : "Disabled");
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_SET_CALL_CONFIRM_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 CscService::DoSetCallConfirmDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolSetCallConfirmRespAdapter adapter(pModemData);
    int mRespInt = adapter.GetResult();
    int errorCode = adapter.GetErrorCode();

/*
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS, &mRespInt, sizeof(mRespInt));
        m_bCallConfirm = true; // approved by CP
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
        m_bCallConfirm = false; // disapproved by CP
    }

    return 0;
}

INT32 CscService::DoSendCallConfirm()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildSendCallConfirm();
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_SEND_CALL_CONFIRM_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 CscService::DoSendCallConfirmDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolSendCallConfirmRespAdapter adapter(pModemData);
    int mRespInt = adapter.GetResult();
    int errorCode = adapter.GetErrorCode();

/*
 * Valid errors:
 *  SUCCESS
 *  RADIO_NOT_AVAILABLE (radio resetting)
 *  GENERIC_FAILURE
 */
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS, &mRespInt, sizeof(mRespInt));
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
        if (m_bCallConfirm == true) {
            RilLogW("[CscService] %s. UNSOL_CALL_PRESENT_IND was neglected.", __FUNCTION__);
        }
    }

    return 0;
}

INT32 CscService::OnCallPresent(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    OnUnsolicitedResponse(RIL_UNSOL_CALL_PRESENT_IND);

    return 0;
}

#ifdef SUPPORT_CDMA
INT32 CscService::OnCdmaCallWaitingNtf(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolCdmaCallWaitingIndAdapter adapter(pModemData);

    memset(&mRespCdmaCallWaitingNoti, 0x00, sizeof(mRespCdmaCallWaitingNoti));
    adapter.GetCwInfo(&mRespCdmaCallWaitingNoti);

    RilLogV("[%s] number : %s", __FUNCTION__, mRespCdmaCallWaitingNoti.number);

    OnUnsolicitedResponse(RIL_UNSOL_CDMA_CALL_WAITING, &mRespCdmaCallWaitingNoti, sizeof(RIL_CDMA_CallWaiting_v6));

    return 0;
}

int CscService::OnUnsolCdmaOtaProvisionStatus(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);

    if (IsNullResponse(pMsg)) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolCdmaOtaProvisionStatusIndAdapter adapter(pModemData);
    int provisionStatus = adapter.GetOtaProvisionStatus();
    if (provisionStatus != -1) {
        OnUnsolicitedResponse(RIL_UNSOL_CDMA_OTA_PROVISION_STATUS, &provisionStatus, sizeof(provisionStatus));
    } else {
        RilLogW("[CscService] %s provisionStatus is error", __FUNCTION__);
    }

    return 0;
}
#endif

int CscService::OnEnterEmergencyCallbackModeChanged(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    OnUnsolicitedResponse(RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE, NULL, 0);

    return 0;
}

int CscService::OnExitEmergencyCallbackModeChanged(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    OnUnsolicitedResponse(RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE, NULL, 0);

    return 0;
}

int CscService::DoExitEmergencyCbMode(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(pMsg))
    {
        return -1;
    }

    ProtocolCallBuilder builder;
    ModemData *pModemData = builder.BuildExitEmergencyCbMode();
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_EXIT_EMERGENCY_CB_MODE_DONE) < 0)
    {
        return -1;
    }
    return 0;;
}

/*
   Allowed RadioError on CardState::ABSENT are
    RadioError::NONE
    RadioError::REQUEST_NOT_SUPPORTED
    RadioError::SIM_ABSENT
   No Return Result
*/
int CscService::DoExitEmergencyCbModeDone(Message *pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolExitEmergencyCbModeRespAdapter adapter(pModemData);
    int errorCode = adapter.GetRilErrorCode();

    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        RilLogE("[CscService] %s errorCode=%d", __FUNCTION__, errorCode);
        switch (errorCode) {
            case RIL_E_REQUEST_NOT_SUPPORTED:
            case RIL_E_RADIO_NOT_AVAILABLE:
                OnRequestComplete(errorCode);
                break;
            default:
                OnRequestComplete(RIL_E_INTERNAL_ERR);
                break;
        }
    }

    return 0;
}

int CscService::OnUnsolResendInCallMute(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    OnUnsolicitedResponse(RIL_UNSOL_RESEND_INCALL_MUTE, NULL, 0);

    return 0;
}

INT32 CscService::DoSetMute()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

   IntRequestData* pMuteReqData = (IntRequestData *) m_pCurReqMsg->GetRequestData();

    ProtocolSoundBuilder builder;
    ModemData *pModemData = builder.BuildSetMute(pMuteReqData->GetInt());
    RilLogV("[CscService] <req> Mute state: %s", pMuteReqData->GetInt() == RIL_MUTE_STATUS_UNMUTE ? "Unmute" : "Mute");
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_SOUND_SET_MUTE_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 CscService::DoSetMuteDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
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
    }
    else
    {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

INT32 CscService::DoGetMute()
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    ProtocolSoundBuilder builder;
    ModemData *pModemData = builder.BuildGetMute();
    if (SendRequest(pModemData, CALL_DEFAULT_TIMEOUT, MSG_CS_SOUND_GET_MUTE_DONE) < 0)
    {
        return -1;
    }
    return 0;
}

INT32 CscService::DoGetMuteDone(Message* pMsg)
{
    RilLogI("[CscService] %s", __FUNCTION__);
    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolSoundGetMuteRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    int mute_state = adapter.GetMuteState();

    RilLogV("[CscService] <resp> Mute state: %s", mute_state == RIL_MUTE_STATUS_UNMUTE ? "Unmute" : "Mute");

/*
 * Allowed RadioError on CardState::ABSENT are
 *   RadioError::NONE
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:SS_MODIFIED_TO_DIAL
 *   RadioError:SS_MODIFIED_TO_USSD
 *   RadioError:SS_MODIFIED_TO_SS
 *   RadioError:NO_MEMORY
 *   RadioError:REQUEST_RATE_LIMITED
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:INTERNAL_ERR
 *   RadioError:SYSTEM_ERR
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:NO_RESOURCES
 *   RadioError:CANCELLED
*/
    if(mCardState != RIL_CARDSTATE_PRESENT) {
        RilLogI("[%s] %s mCardState(%d) is not Present(%d)", m_szSvcName, __FUNCTION__, mCardState, RIL_CARDSTATE_PRESENT);
        // Ignore Error, this shall succeed always on SIM ABSENT
        errorCode = RIL_E_SUCCESS;
    }
    if (errorCode == RIL_E_SUCCESS)
    {
        OnRequestComplete(RIL_E_SUCCESS, &mute_state, sizeof(mute_state));
    }
    else
    {
        RilLogI("[%s] unexpected %s mCardState(%d) is not Present, errorCode=%d", m_szSvcName, __FUNCTION__, mCardState, errorCode);
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}

const char* CscService::GetCallTypeString(CallType callType)
{
    switch(callType)
    {
        case CALL_TYPE_VOICE:
            return "gsm voice";
        case CALL_TYPE_VIDEO:
            return "gsm video";
        case CALL_TYPE_EMERGENCY:
            return "gsm emergency";
#ifdef SUPPORT_CDMA
        case CALL_TYPE_CDMA_VOICE:
            return "cdma voice";
        case CALL_TYPE_CDMA_EMERGENCY:
            return "cdma emergency";
#endif
    }
    return "gsm voice";
}

BOOL CscService::IsNullRequest(Message *pMsg)
{
    //RilLogV("[CscService] %s",__FUNCTION__);
    if (NULL == pMsg || NULL == pMsg->GetRequestData())
    {
        RilLogE("[CscService] %s, pMsg or RequestData is NULL!!", __FUNCTION__);
        return TRUE;
    }
    return FALSE;
}

BOOL CscService::IsNullResponse(Message *pMsg)
{
    //RilLogV("[CscService] %s",__FUNCTION__);
    if (NULL == pMsg || NULL == pMsg->GetModemData())
    {
        RilLogE("[CscService] %s, pMsg or ModemData is NULL!!", __FUNCTION__);
        return TRUE;
    }
    return FALSE;
}

bool CscService::IsPossibleToPassInRadioOffState(int request_id)
{
    switch (request_id) {
        case RIL_REQUEST_GET_MUTE:
        case RIL_REQUEST_SET_MUTE:
            break;
        default:
            return false;
    }
    return true;
}

BOOL CscService::IsInCallState()
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (m_currCallList == NULL)
    {
        RilLogE("[%s] %s() Call list is NULL", m_szSvcName, __FUNCTION__);
        return FALSE;
    }
    else
    {
        RilLogV("[%s] %s() Call list Count == %d", m_szSvcName, __FUNCTION__, m_currCallList->GetCount());
        return (m_currCallList->GetCount() != 0);
    }
}
