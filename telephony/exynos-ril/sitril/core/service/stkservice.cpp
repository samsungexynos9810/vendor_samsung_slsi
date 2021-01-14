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
 * StkService.cpp
 *
 *  Created on: 2018.03.09.
 *      Author: MOX
 */

#include "stkservice.h"
#include "rillog.h"
#include "protocolstkbuilder.h"
#include "protocolstkadapter.h"
#include "protocolsimadapter.h"
#include "stkdata.h"
#include "stkdatabuilder.h"
#include "util.h"
#include "systemproperty.h"
#include "servicemgr.h"
#include "rilparser.h"
#include "simservice.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define writeRilEvent(format1, format2, ...)   CRilEventLog::writeRilEvent(RIL_LOG_CAT_STK, format1, format2, ##__VA_ARGS__)

#define PARAM_NULL(msg)        { if(msg==NULL) { RilLogE("%s::%s() Parameter = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define NULL_REQ(msg)        { if(msg==NULL || msg->GetRequestData()==NULL) { RilLogE("%s::%s() RequestData = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define NULL_RSP(msg)        { if(msg==NULL || msg->GetModemData()==NULL) { RilLogE("%s::%s() ModemData = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define ENTER_FUNC()        { RilLogI("%s::%s() [<-- ", m_szSvcName, __FUNCTION__); }
#define LEAVE_FUNC()        { RilLogI("%s::%s() [--> ", m_szSvcName, __FUNCTION__); }
#define NOT_IMPLEMENT()        { RilLogE("%s::%s() Not Implemented", m_szSvcName, __FUNCTION__); }

#define LOGI(format, ...)        RilLogI("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)
#define LOGV(format, ...)        RilLogV("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)
#undef LOGD
#define LOGD(format, ...)        RilLog("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)
#define LOGE(format, ...)        RilLogE("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)

StkService::StkService(RilContext* pRilContext)
    : Service(pRilContext, RIL_SERVICE_STK)
{
    m_bStkRunning = FALSE;
    m_pProactiveCmd = NULL;
    m_pStkEfidList = NULL;
    m_pStkModule = NULL;
    m_nCardState = RIL_CARDSTATE_ABSENT;
}

StkService::~StkService()
{
    if(m_pProactiveCmd!=NULL)
    {
        delete m_pProactiveCmd;
        m_pProactiveCmd = NULL;
    }

    if(m_pStkEfidList!=NULL)
    {
        delete m_pStkEfidList;
        m_pStkEfidList = NULL;
    }

    // For STK Module
    if(STK_MODULE_ENABLED==true && m_pStkModule!=NULL)
    {
        m_pStkModule->Finalize();
        delete m_pStkModule;
        m_pStkModule = NULL;
    }
}

int StkService::OnCreate(RilContext *pRilContext)
{
    ENTER_FUNC();

    m_pProactiveCmd = NULL;

    // For STK Module
    if(STK_MODULE_ENABLED==true)
    {
        if(m_pStkModule!=NULL)
        {
            if(m_pStkModule->IsStarted()) m_pStkModule->Finalize();
            delete m_pStkModule;
            m_pStkModule = NULL;
        }

        m_pStkModule = new CStkModule(this, GetRilSocketId());
        if(m_pStkModule==NULL)
        {
            RilLogE("%s::%s() STK Module %d Creation Failed !!!", m_szSvcName, __FUNCTION__, GetRilSocketId());
            return -1;
        }
        RilLogE("%s::%s() STK Module %d Created", m_szSvcName, __FUNCTION__, GetRilSocketId());

        if(m_pStkModule->OnCreate()<0)
        {
            LOGE("m_pStkModule->OnCreate() Error !!!");
            return -1;
        }
    }

    LEAVE_FUNC();
    return 0;
}

BOOL StkService::OnHandleRequest(Message* pMsg)
{
    if (pMsg == NULL) {
        return FALSE;
    }

    int ret = -1;
    switch (pMsg->GetMsgId()) {
    case MSG_SAT_STK_IS_RUNNING:
        ret = DoStkIsRunning(pMsg);
        break;
    case MSG_SAT_SEND_ENVELOPE_CMD:
        ret = DoSendEvelopeCommand(pMsg);
        break;
    case MSG_SAT_SEND_TERMINAL_RSP:
        ret = DoSendTerminalResponse(pMsg);
        break;
    case MSG_SAT_SEND_ENVELOPE_STATUS:
        ret = DoSendEnvelopeStatus(pMsg);
        break;
    case MSG_SAT_STK_HANDLE_CALL_SETUP_REQ_FROM_SIM:
        ret = DoStkHandleCallSetupReqFromSim(pMsg);
        break;
    default:
        // TODO log unsupported message id
        return FALSE;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL StkService::OnHandleSolicitedResponse(Message* pMsg)
{
    PARAM_NULL(pMsg);

    int ret = -1;
    switch (pMsg->GetMsgId()) {
    case MSG_SAT_SEND_ENVELOPE_CMD_DONE:
        ret = OnSendEvelopeCommandDone(pMsg);
        break;
    case MSG_SAT_SEND_TERMINAL_RSP_DONE:
        ret = OnSendTerminalResponseDone(pMsg);
        break;
    case MSG_SAT_SEND_ENVELOPE_STATUS_DONE:
        ret = OnSendEnvelopeStatusDone(pMsg);
        break;
    case MSG_SAT_STK_HANDLE_CALL_SETUP_REQ_FROM_SIM_DONE:
        ret = OnStkHandleCallSetupReqFromSimDone(pMsg);
        break;
    default:
        // TODO log unsupported message id
        return FALSE;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL StkService::OnHandleUnsolicitedResponse(Message* pMsg)
{
    PARAM_NULL(pMsg);

    int ret = -1;
    switch (pMsg->GetMsgId()) {
    case MSG_SAT_PROACTIVE_COMMAND:
        ret = OnProactiveCommand(pMsg);
        break;
    case MSG_SAT_STK_SESSION_END:
        ret = OnSessionEnd(pMsg);
        break;
    case MSG_SAT_SIM_REFRESH:
        ret = OnSimRefresh(pMsg);
        break;
    // SS
    case MSG_CS_SS_RETURN_RESULT_NTF:
        ret = OnSsReturnResult(pMsg);
        break;
    case MSG_SAT_STK_CC_ALPHA_NOTIFY:
        ret = OnStkCcAlphaNtf(pMsg);
        break;
    default:
        // TODO log unsupported message id
        return FALSE;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

void StkService::OnSimStatusChanged(int cardState, int appState)
{
    ENTER_FUNC();

    LOGV("current card state :%d, new card state :%d", m_nCardState, cardState);


    if(m_nCardState!=cardState)
    {
        // SIM Insertion
        if(m_nCardState==RIL_CARDSTATE_ABSENT && cardState==RIL_CARDSTATE_PRESENT) OnSimHotSwap(FALSE);
        // SIM Removal
        else if(m_nCardState==RIL_CARDSTATE_PRESENT && cardState==RIL_CARDSTATE_ABSENT) OnSimHotSwap(TRUE);

        m_nCardState = cardState;
    }

    LEAVE_FUNC();
}

void StkService::OnVoiceRegistrationStateChanged(int regState)
{

}

void StkService::OnDataRegistrationStateChanged(int regState)
{
    ENTER_FUNC();

    if(STK_MODULE_ENABLED==true && m_pStkModule!=NULL) m_pStkModule->OnDataRegistrationStateChanged(regState);

    LEAVE_FUNC();
}

void StkService::OnDataCallStateChanged(int nCid, bool bActive)
{
    ENTER_FUNC();

    if(STK_MODULE_ENABLED==true && m_pStkModule!=NULL) m_pStkModule->OnDataCallStateChanged(nCid, bActive);

    LEAVE_FUNC();
}

bool StkService::IsDefaultApnConnecting()
{
    ENTER_FUNC();

    bool bResult = false;
    Message *pMsg = GetCurrentMsg();
    if(pMsg->GetMsgId()==MSG_PS_SETUP_DATA_CALL) bResult = true;

    LEAVE_FUNC();
    return bResult;
}

int StkService::OnSimHotSwap(BOOL bRemoval)
{
    ENTER_FUNC();

    if(bRemoval)
    {
        m_bStkRunning = FALSE;
        if(m_pProactiveCmd!=NULL)
        {
            delete m_pProactiveCmd;
            m_pProactiveCmd = NULL;
        }
    }
    else
    {
    }

    LEAVE_FUNC();
    return 0;
}

int StkService::DoStkIsRunning(Message *pMsg)
{
    ENTER_FUNC();

    // For STK Module
    if(STK_MODULE_ENABLED==true && m_pStkModule!=NULL) m_pStkModule->Initialize();

    OnRequestComplete(RIL_E_SUCCESS);

    /* Test Code of Proactive Command
    char test[] = "D01C8103012180820281028D0F04546F6F6C6B697420546573742034AB00";
    OnUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND, test, strlen(test));
    return 0;
    */

    // Send Kept Proactive Command
    if(m_bStkRunning==FALSE)
    {
        m_bStkRunning = TRUE;

        if(m_pProactiveCmd!=NULL)
        {
            // For STK Module
            if(STK_MODULE_ENABLED==true)
            {
                //Message *pMessage = new Message();
                SendStk(m_pProactiveCmd->Clone());
            }
            else OnProactiveCommand(m_pProactiveCmd);

            delete m_pProactiveCmd;
            m_pProactiveCmd = NULL;
        }
    }

    LEAVE_FUNC();
    return 0;
}

int StkService::DoSendEvelopeCommand(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    int nResult = -1;

    StkData rildata;
    if(rildata.Parse((StringRequestData *) pMsg->GetRequestData())==FALSE
            || rildata.GetLength()==0 || rildata.GetData()==NULL) {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        LEAVE_FUNC();
        return 0;
    }

    if(m_nCardState==RIL_CARDSTATE_ABSENT)
    {
        OnRequestComplete(RIL_E_SIM_ABSENT);
        LEAVE_FUNC();
        return 0;
    }

    // For STK Module
    if(pMsg->GetRequestData()->GetReqType()==REQ_FW && STK_MODULE_ENABLED==true
            && m_pStkModule && m_pStkModule->GetCurrentProcessingCmd()!=TAG_NONE)
    {
        SendStk(pMsg->Clone());
    }
    else
    {
        ProtocolStkBuilder builder;
        ModemData *pModemData = builder.BuildStkEnvelopeCommand(rildata.GetLength(), rildata.GetData());
        nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_SAT_SEND_ENVELOPE_CMD_DONE);
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int StkService::OnSendEvelopeCommandDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    // For STK Module
    if(STK_MODULE_ENABLED==true && m_pStkModule && m_pStkModule->GetCurrentProcessingCmd()!=TAG_NONE)
    {
        SendStk(pMsg->Clone());
    }
    else
    {
        ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
        UINT uErrCode = adapter.GetErrorCode();
        if(uErrCode==RIL_E_SUCCESS)
        {
            ProtocolStkEnvelopeCommandAdapter adapter(pMsg->GetModemData());
            StkDataBuilder builder;
            const RilData *pRilData = builder.BuildStkEnvelopeCommandResponse(adapter.GetEnvelopeCmdLength(), adapter.GetEnvelopeCommand());
            if(pRilData) OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
            else OnRequestComplete(RIL_E_SUCCESS);
            delete pRilData;
        }
        else OnRequestComplete(uErrCode);
    }

    LEAVE_FUNC();
    return 0;
}

int StkService::DoSendTerminalResponse(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    int nResult = -1;

    StkData rildata;
    if(rildata.Parse((StringRequestData *) pMsg->GetRequestData())==FALSE
            || rildata.GetLength()==0 || rildata.GetData()==NULL) {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        LEAVE_FUNC();
        return 0;
    }

    if(m_nCardState==RIL_CARDSTATE_ABSENT)
    {
        OnRequestComplete(RIL_E_SIM_ABSENT);
        LEAVE_FUNC();
        return 0;
    }

    // For STK Module
    if(pMsg->GetRequestData()->GetReqType()==REQ_FW && STK_MODULE_ENABLED==true
            && m_pStkModule && m_pStkModule->GetCurrentProcessingCmd()!=TAG_NONE)
    {
        SendStk(pMsg->Clone());
    }
    else
    {
        ProtocolStkBuilder builder;
        ModemData *pModemData = builder.BuildStkTerminalResponse(rildata.GetLength(), rildata.GetData());
        nResult = SendRequest(pModemData, TIMEOUT_STK_DEFAULT, MSG_SAT_SEND_TERMINAL_RSP_DONE);
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int StkService::OnSendTerminalResponseDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    // For STK Module
    if(STK_MODULE_ENABLED==true && m_pStkModule && m_pStkModule->GetCurrentProcessingCmd()!=TAG_NONE)
    {
        SendStk(pMsg->Clone());
    }
    else
    {
        ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
        UINT uErrCode = adapter.GetErrorCode();
        OnRequestComplete(uErrCode);
    }

    LEAVE_FUNC();
    return 0;
}

int StkService::DoSendEnvelopeStatus(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    int nResult = -1;

    StkData rildata;
    if(rildata.Parse((StringRequestData *) pMsg->GetRequestData())==FALSE
            || rildata.GetLength()==0 || rildata.GetData()==NULL) {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        LEAVE_FUNC();
        return 0;
    }

    if(m_nCardState==RIL_CARDSTATE_ABSENT)
    {
        OnRequestComplete(RIL_E_SIM_ABSENT);
        LEAVE_FUNC();
        return 0;
    }

    // For STK Module
    if(pMsg->GetRequestData()->GetReqType()==REQ_FW && STK_MODULE_ENABLED==true
            && m_pStkModule && m_pStkModule->GetCurrentProcessingCmd()!=TAG_NONE)
    {
        SendStk(pMsg->Clone());
    }
    else
    {
        ProtocolStkBuilder builder;
        ModemData *pModemData = builder.BuildStkEnvelopeStatus(rildata.GetLength(), rildata.GetData());
        nResult = SendRequest(pModemData, TIMEOUT_STK_DEFAULT, MSG_SAT_SEND_ENVELOPE_STATUS_DONE);
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int StkService::OnSendEnvelopeStatusDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    // For STK Module
    if(STK_MODULE_ENABLED==true && m_pStkModule && m_pStkModule->GetCurrentProcessingCmd()!=TAG_NONE)
    {
        SendStk(pMsg->Clone());
    }
    else
    {
        ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
        UINT uErrCode = adapter.GetErrorCode();
        if(uErrCode==RIL_E_SUCCESS)
        {
            ProtocolStkEnvelopeStatusAdapter adapter(pMsg->GetModemData());
            StkDataBuilder builder;
            const RilData *pRilData = builder.BuildStkEnvelopeStatusResponse(adapter.GetSW1(), adapter.GetSW2(), adapter.GetEnvelopeRspLength(), adapter.GetEnvelopeResponse());
            if(pRilData) OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
            else OnRequestComplete(RIL_E_SUCCESS);
            delete pRilData;
        }
        else OnRequestComplete(uErrCode);
    }

    LEAVE_FUNC();
    return 0;
}

int StkService::DoStkHandleCallSetupReqFromSim(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    if(GetRadioState()==RADIO_STATE_ON)
    {
        int nResult = -1;

        IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
        const int nUserOper = rildata->GetInt();

        ProtocolStkBuilder builder;
        ModemData *pModemData = builder.BuildStkCallSetup(nUserOper);
        nResult = SendRequest(pModemData, TIMEOUT_STK_DEFAULT, MSG_SAT_STK_HANDLE_CALL_SETUP_REQ_FROM_SIM_DONE);
    }
    else OnRequestComplete(RIL_E_RADIO_NOT_AVAILABLE);

    LEAVE_FUNC();
    return 0;
}

int StkService::OnStkHandleCallSetupReqFromSimDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSimResponseAdapter adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();
    if (uErrCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else if(uErrCode == RIL_E_RADIO_NOT_AVAILABLE) OnRequestComplete(uErrCode);
    else OnRequestComplete(RIL_E_GENERIC_FAILURE);

    LEAVE_FUNC();
    return 0;
}

int StkService::OnProactiveCommand(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolStkProactiveCommandAdapter adapter(pModemData);

    if(m_pStkEfidList!=NULL)
    {
        delete m_pStkEfidList;
        m_pStkEfidList = NULL;
    }

    if(m_pStkEfidList!=NULL) delete m_pStkEfidList;
    m_pStkEfidList = NULL;

    if(adapter.GetEfidCount()>0)
    {
        m_pStkEfidList = new StkEfidList(adapter.GetEfidCount());
        if(m_pStkEfidList!=NULL) for(int i=0; i<adapter.GetEfidCount(); i++) m_pStkEfidList->Insert(i, adapter.GetEFID(i));
        else LOGE("m_pStkEfidList new operation failed");
    }

    if(adapter.GetAidLength()>0)
    {
        if(m_pStkEfidList==NULL) m_pStkEfidList = new StkEfidList(0);
        m_pStkEfidList->SetAID(adapter.GetAID(), adapter.GetAidLength());
    }

    if(m_bStkRunning==TRUE)
    {
        StkDataBuilder builder;
        const RilData *pRilData = builder.BuildStkProactiveCommandIndicate(adapter.GetProactiveCmdLength(), adapter.GetProactiveCommand());

        // For STK Module
        if(STK_MODULE_ENABLED==true) SendStk(pMsg->Clone());
        else OnUnsolicitedResponse(RIL_UNSOL_STK_PROACTIVE_COMMAND, pRilData->GetData(), pRilData->GetDataLength());

        delete pRilData;
    }
    // Keep the Proactive Command if STK application is not ready (STK_IS_RUNNING)
    else
    {
        if(m_pProactiveCmd!=NULL)
        {
            delete m_pProactiveCmd;
            m_pProactiveCmd = NULL;
        }
        m_pProactiveCmd = pMsg->Clone();
    }

    LEAVE_FUNC();
    return 0;
}

int StkService::OnSessionEnd(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        LOGE("pModemData is NULL");
        return -1;
    }

    OnUnsolicitedResponse(RIL_UNSOL_STK_SESSION_END);
    LEAVE_FUNC();
    return 0;
}
int StkService::OnSimRefresh(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolStkSimRefreshAdapter adapter(pMsg->GetModemData());
    StkDataBuilder builder;

    /* According to AOSP,
       EFID : the updated file if the result is SIM_FILE_UPDATE or 0 for any other result.
       AID(application ID) : the card application
                             See ETSI 102.221 8.1 and 101.220 4
       For SIM_FILE_UPDATE result it can be set to AID of application in which updated EF resides
           or it can be NULL if EF is outside of an application.
       For SIM_INIT result this field is set to AID of application that caused REFRESH
       For SIM_RESET result it is NULL. */

    int nAidLength = 0;
    BYTE *pAID = NULL;
    if(adapter.GetResult()!=SIM_RESET && m_pStkEfidList!=NULL && m_pStkEfidList->GetAidLength()>0)
    {
        nAidLength = m_pStkEfidList->GetAidLength();
        pAID = m_pStkEfidList->GetAID();
        //RilLogV("AID exists");
    }

    // Cached Proactive Command for SIM_REFRESH
    if(adapter.GetResult()==SIM_FILE_UPDATE && m_pStkEfidList!=NULL && m_pStkEfidList->GetCount()>0)
    {
        RilLogV("SIM_FILE_UPDATE, EFID count:%d", m_pStkEfidList->GetCount());
        // EFID exists
        for(int i=0; i<m_pStkEfidList->GetCount(); i++)
        {
            //RilLogV("SIM_FILE_UPDATE, %d.EFID:0x%04X", i, m_pStkEfidList->GetAt(i));
            const RilData *pRilData = builder.BuildStkSimRefreshIndicate(adapter.GetResult(), m_pStkEfidList->GetAt(i), nAidLength, pAID);
            if(pRilData)
            {
                OnUnsolicitedResponse(RIL_UNSOL_SIM_REFRESH, pRilData->GetData(), pRilData->GetDataLength());
                delete pRilData;
                pRilData = NULL;
            }
        }
    }
    else if(adapter.GetResult()!=SIM_RESET && pAID==NULL)
    {
        RilLogV("SIM_REFRESH(Not RESET) without AID");
        RilContext *pContext = GetRilContext();
        if(pContext)
        {
            SimService *pSimService = (SimService *) pContext->GetService(RIL_SERVICE_SIM);
            for(int i=0; pSimService && i<pSimService->mRilCardStatus.num_applications; i++)
            {
                BYTE tempAID[MAX_SIM_AID_LEN+1];
                memset(tempAID, 0, MAX_SIM_AID_LEN+1);
                int nAidLength = HexString2Value(tempAID, (const char *) pSimService->m_aszAID[i]);
                const RilData *pRilData = builder.BuildStkSimRefreshIndicate(adapter.GetResult(), 0, nAidLength, (BYTE *) tempAID);
                if(pRilData)
                {
                    OnUnsolicitedResponse(RIL_UNSOL_SIM_REFRESH, pRilData->GetData(), pRilData->GetDataLength());
                    delete pRilData;
                    pRilData = NULL;
                }
            }
        }
    }
    else
    {
        RilLogV("SIM_FILE_UPDATE without EFID, SIM_INIT or SIM_RESET");
        const RilData *pRilData = builder.BuildStkSimRefreshIndicate(adapter.GetResult(), 0, nAidLength, pAID);
        if(pRilData)
        {
            // Reset flag because CatService should restart.
            if(adapter.GetResult()==SIM_RESET) m_bStkRunning = FALSE;

            OnUnsolicitedResponse(RIL_UNSOL_SIM_REFRESH, pRilData->GetData(), pRilData->GetDataLength());
            delete pRilData;
            pRilData = NULL;
        }
    }

    if(adapter.GetResult()==SIM_INIT || adapter.GetResult()==SIM_RESET)
    {
        OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED);
    }

    if(m_pStkEfidList!=NULL)
    {
        delete m_pStkEfidList;
        m_pStkEfidList = NULL;
    }

    LEAVE_FUNC();
    return 0;
}

int StkService::OnSsReturnResult(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolSsReturnResultAdapter adapter(pMsg->GetModemData());
    StkDataBuilder builder;
    const RilData *pRilData = builder.BuildStkSsReturnResultIndicate(adapter.GetReturnResultLength(), adapter.GetReturnResult());

    OnUnsolicitedResponse(RIL_UNSOL_SUPP_SVC_RETURN_RESULT, pRilData->GetData(), pRilData->GetDataLength());
    delete pRilData;

    LEAVE_FUNC();
    return 0;
}

INT32 StkService::OnStkCcAlphaNtf(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolStkCcAlphaNtfAdapter adapter(pModemData);
    int lenAlpha = adapter.GetAlphaLength();
    BYTE *pAlpha = adapter.GetAlpha();

    StkDataBuilder builder;
    const RilData *rildata = builder.BuildStkCcAlphaNtf(lenAlpha, pAlpha);
    if (rildata != NULL){
        OnUnsolicitedResponse(RIL_UNSOL_STK_CC_ALPHA_NOTIFY, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }

    LEAVE_FUNC();
    return 0;
}

bool StkService::IsPossibleToPassInRadioOffState(int request_id)
{
    switch (request_id) {
        case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
        case RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND:
        case RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE:
        case RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS:
            break;
        default:
            return false;
    }
    return true;

}

// Internal Request
void StkService::OnRequestInternalComplete(RIL_Token t, int id, int result, void *data /* =NULL */, int datalen /* =0 */)
{
    ENTER_FUNC();

    if(m_pStkModule!=NULL)
    {
        switch(id)
        {
        case RIL_REQUEST_SETUP_DATA_CALL:
            m_pStkModule->OnOpenBipConnectionDone(result, data, datalen);
            break;
        case RIL_REQUEST_DEACTIVATE_DATA_CALL:
            m_pStkModule->OnCloseBipConnectionDone(result, data, datalen);
            break;
        }
    }
    else LOGE("m_pStkModule is NULL !!!");

    LEAVE_FUNC();
}

// For STK Module
int StkService::SendStk(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    switch(pMsg->GetDirection())
    {
    case REQUEST:
    case RESPONSE:
        if(m_pStkModule!=NULL) m_pStkModule->Enqueue(pMsg->Clone());
        else LOGE("m_pStkModule is NULL !!!");
        break;
    default:
        LOGE("Invalid Direction:%d", pMsg->GetDirection());
        break;
    }

    LEAVE_FUNC();
    return 0;
}

int StkService::OnStkModule(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    EnQueue(pMsg);
    NotifyNewMessage(pMsg);

    /*
    switch(pMsg->GetDirection())
    {
    case REQUEST:
        OnHandleRequest(pMsg);
        break;
    case RESPONSE:
        if(OnHandleUnsolicitedResponse(pMsg)==false) OnHandleSolicitedResponse(pMsg);
        break;
    default:
        LOGE("Invalid Direction:%d", pMsg->GetDirection());
        break;
    }
    */

    LEAVE_FUNC();
    return 0;
}
