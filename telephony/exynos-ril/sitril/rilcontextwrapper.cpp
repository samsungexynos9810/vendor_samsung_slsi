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
 * rilcontextwrapper.cpp
 *
 *  Created on: 2014. 11. 12.
 *      Author: sungwoo48.choi
 */
#include "rilapplication.h"
#include "rilcontextwrapper.h"
#include "productfactory.h"
#include "servicefactory.h"
#include "rilparser.h"
#include "modemstatemonitor.h"
#include "servicemgr.h"
#include "rcmmgr.h"
#include "modemdata.h"
#include "requestdata.h"
#include "rildata.h"
#include "rillog.h"
#include "service.h"
#include "open_carrier.h"
#include "cscservice.h"

static bool VDBG = true;

IMPLEMENT_MODULE_TAG(RilContextWrapper, RilContext)

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

// Don't use old version RIL_Data_Call_Response anymore
typedef RIL_Data_Call_Response_v11  RIL_Data_Call_Response;

RilContextWrapper::RilContextWrapper(RilApplication *rilApp, const RilContextParam *param)
    : m_pRilApp(rilApp)
{
    memset(&m_Param, 0, sizeof(RilContextParam));
    m_rilSocketId = (RIL_SOCKET_ID)-1;
    if (param != NULL) {
        memcpy(&m_Param, param, sizeof(RilContextParam));
        m_rilSocketId =  m_Param.socket_id;
    }

    m_pIoChannel = NULL;
    m_pRcmMgr = NULL;
    m_pRilParser = NULL;
    m_pServiceMgr = NULL;
    mModemState = MS_OFFLINE;
    m_RadioState = RADIO_STATE_UNAVAILABLE;
    m_nOpenCarrierIndex = (unsigned int)OC_UNKNOWN;
    memset(m_bDataCallStateArray, 0, MAX_DATA_CALL_SIZE*2+1);
}

RilContextWrapper::~RilContextWrapper()
{
    OnDestroy();
}

int RilContextWrapper::OnCreate()
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    if (m_pRilApp == NULL) {
        return -1;
    }

    m_RilContextProperty.SetSocketId((int)m_rilSocketId);

    RilLogV("[%s_%d] %s RilContextParam{socket_id=%d,iochannel_name=%s,ifprefix=%s,ifstart=%d,ifmaxsize=%d,productFactory=%p,serviceFactory=%p}",
            TAG, m_rilSocketId, __FUNCTION__,
            m_Param.socket_id, m_Param.iochannel_name,
            m_Param.ifprefix, m_Param.ifstart, m_Param.ifmaxsize,
            m_Param.productFactory, m_Param.serviceFactory);

    // check parameter
    if (*m_Param.iochannel_name == 0 ) {
        return -1;
    }

    if (*m_Param.ifprefix == 0 ) {
        return -1;
    }

    if (m_Param.ifmaxsize == 0 ) {
        return -1;
    }

    RilLogV("[%s_%d] %s Get ProductFactory", TAG, m_rilSocketId, __FUNCTION__);
    ProductFactory *pProductFactory = m_Param.productFactory;
    if (pProductFactory == NULL) {
        // use default product factory
        RilLogV("[%s_%d] %s Use Default ProductFactory", TAG, m_rilSocketId, __FUNCTION__);
        pProductFactory = ProductFactory::GetDefaultProductFactory(this);
    }

    RilLogV("[%s_%d] %s Get RIL Request Parser", TAG, m_rilSocketId, __FUNCTION__);
    // initialize RIL Request Parser instance
    m_pRilParser = pProductFactory->GetRequestParser(this);
    if (m_pRilParser == NULL) {
        RilLogE("[%s_%d] %s Fail to init RIL Request Parser", TAG, m_rilSocketId, __FUNCTION__);
        return -1;
    }

    // initialize IoChannel instance
    RilLogV("[%s_%d] %s Init IoChannel name=%s", TAG, m_rilSocketId, __FUNCTION__, m_Param.iochannel_name);
    m_pIoChannel = pProductFactory->GetIoChannel(this, m_Param.iochannel_name);
    if (m_pIoChannel == NULL) {
        RilLogE("[%s_%d] %s Fail to init IoChannel", TAG, m_rilSocketId, __FUNCTION__);
        return -1;
    }

    RilLogV("[%s_%d] %s Get ServiceFactory", TAG, m_rilSocketId, __FUNCTION__);
    // initialize ServiceMgr and Service instances
    ServiceFactory *pServiceFactory = m_Param.serviceFactory;
    if (pServiceFactory == NULL) {
        RilLogW("[%s_%d] %s Use Default ServiceFactory", TAG, m_rilSocketId, __FUNCTION__);
        pServiceFactory = ServiceFactory::GetDefaultServiceFactory();
    }

    m_pServiceMgr = pProductFactory->GetServiceManager(this);
    if (m_pServiceMgr == NULL || m_pServiceMgr->Init(pServiceFactory) < 0) {
        RilLogE("[%s_%d] %s Fail to init ServiceManager", TAG, m_rilSocketId, __FUNCTION__);
        return -1;
    }

    m_RequestWaitList.Clear();

    return 0;
}

int RilContextWrapper::OnStart()
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    if (m_pServiceMgr != NULL && m_pServiceMgr->StartServices() < 0) {
        RilLogE("[%s_%d] %s Fail to start all service instance", TAG, m_rilSocketId, __FUNCTION__);
        return -1;
    }

    m_pRcmMgr = new RadioControlMessageMgr(this, m_pIoChannel);
    if (m_pRcmMgr == NULL || m_pRcmMgr->Start() < 0) {
        RilLogE("[%s_%d] %s Fail to start RadioControlMessageMgr", TAG, m_rilSocketId, __FUNCTION__);
        return -1;
    }

    return 0;
}

void RilContextWrapper::OnDestroy()
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    if (m_pRcmMgr != NULL) {
        delete m_pRcmMgr;
        m_pRcmMgr = NULL;
    }

    if (m_pIoChannel != NULL) {
        delete m_pIoChannel;
        m_pIoChannel = NULL;
    }

    if (m_pServiceMgr != NULL) {
        delete m_pServiceMgr;
        m_pServiceMgr = NULL;
    }

    if (m_pRilParser != NULL) {
        delete m_pRilParser;
        m_pRilParser = NULL;
    }
}

void RilContextWrapper::OnRequest(int request, void *data, unsigned int datalen, RIL_Token t)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    if (request < 0) {
        RilLogE("Invalid RIL Request ID");
        OnRequestComplete(t, request, RIL_E_REQUEST_NOT_SUPPORTED);
        return;
    }

    RequestData *pReqData = m_pRilParser->GetRequestData(request, t, data, datalen);
    if(pReqData == NULL || SendMessage(pReqData) < 0) {
        RilLogE("[%s_%d] %s Send message failed", TAG, m_rilSocketId, __FUNCTION__);
        OnRequestComplete(t, request, RIL_E_REQUEST_NOT_SUPPORTED);
        if (pReqData) {
            delete pReqData;
        }
    }
}

RIL_RadioState RilContextWrapper::OnRadioStateRequest()
{
    return GetCurrentRadioState();
}

void RilContextWrapper::OnRequestComplete(RIL_Token t, int request, int result, void *response/* = NULL*/, unsigned int responselen/* = 0*/)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    // TODO pre-process solicited response
    if (result == RIL_E_SUCCESS) {
        ProcessSolicitedResponse(request, response, responselen);
    }

    // notify the result
    if (m_pRilApp != NULL) {
        m_pRilApp->OnRequestComplete(this, t, (RIL_Errno)result, response, responselen);
    }
}

void RilContextWrapper::OnUnsolicitedResponse(int unsolResponse, const void *data/* = NULL*/, unsigned int datalen/* = 0*/)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);

    // pre-process unsolicited response
    ProcessUnsolicitedResponse(unsolResponse, data, datalen);

    // notify the result
    if (m_pRilApp != NULL) {
        m_pRilApp->OnUnsolicitedResponse(this, unsolResponse, data, datalen);
    }
}

void RilContextWrapper::OnRequestAck(RIL_Token t)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);

    if (m_pRilApp != NULL) {
        m_pRilApp->OnRequestAck(t);
    }
}

void RilContextWrapper::OnRequestComplete(RequestData*req, int result, void *response/* = NULL*/, unsigned int responselen/* = 0*/)
{
    if (req != NULL) {
        OnRequestComplete(req->GetToken(), req->GetReqId(), result, response, responselen);
    }
}

void RilContextWrapper::OnRequestTimeout(RequestData*req, unsigned int token)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);

    MessageHistoryRecord record = m_RequestWaitList.Find(token);
    if (record.IsValid()) {
        RemoveRequestHistory(token);
    }

    if (req != NULL) {
        OnRequestComplete(req, RIL_E_INTERNAL_ERR);
    }
}

RIL_SOCKET_ID RilContextWrapper::GetRilSocketId()
{
    return m_rilSocketId;
}

ServiceMgr *RilContextWrapper::GetServiceManager()
{
    return m_pServiceMgr;
}

RilProperty *RilContextWrapper::GetApplicationProperty()
{
    if (m_pRilApp != NULL)
        return m_pRilApp->GetProperty();
    return NULL;
}

void RilContextWrapper::OnModemStateChanged(int state)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    int oldState = mModemState;
    mModemState = state;

    // state not changed
    if (oldState == state) return;

    if (state == MS_CRASH_EXIT) {
        if (m_pRcmMgr != NULL) {
            m_pRcmMgr->StopRcmMonitoring();
        }
    }

    RilDataInts *rildata = new RilDataInts(1);
    if (rildata != NULL) {
        RilLogW("[%d] Broadcast new modem state(%d)", GetRilSocketId(), state);
        rildata->SetInt(0, state);
        BroadcastSystemMessage(MSG_SYSTEM_MODEM_STATE_CHANGED, rildata);
    }

    if (oldState == MS_ONLINE && state != MS_ONLINE) {
        RilLogW("[%d] Broadcast system reset", GetRilSocketId());
        BroadcastSystemMessage(MSG_SYSTEM_RESET);
    }
}

int RilContextWrapper::Send(ModemData *pModemData, UINT nDstServiceId, UINT nResult)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    int nRet = -1;
    if (pModemData != NULL) {
        // TODO record Tx history
        RecordRequestHistory(nDstServiceId, nDstServiceId, pModemData->GetToken(), nResult);
        nRet = this->Send((char *)pModemData->GetRawData(), pModemData->GetLength());

        if (nRet < 0) {
            RemoveRequestHistory(pModemData->GetToken());
        }
    }

    return nRet;
}

int RilContextWrapper::Send(void *data, unsigned int datalen)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    if(m_pRcmMgr->Write((char *)data, datalen) != 0) {
        RilLogE("Fail to write to the IO Channel.");
        return -1;
    }
    return 0;
}

UINT RilContextWrapper::RecordRequestHistory(UINT nSrcServiceId, UINT nDestServiceId, UINT nToken, UINT nResult)
{
    RilLogV("[%s_%d] %s token=0x%04X", TAG, m_rilSocketId, __FUNCTION__, nToken);
    MessageHistoryRecord record = MessageHistoryRecord::newInstance(nSrcServiceId, nDestServiceId, nToken, nResult);
    if (record.IsValid()) {
        m_RequestWaitList.Push(record);
    }
    return 0;
}

void RilContextWrapper::RemoveRequestHistory(UINT nToken)
{
    RilLogV("[%s_%d] %s  token=0x%04X", TAG, m_rilSocketId, __FUNCTION__, nToken);
    m_RequestWaitList.Remove(nToken);
}

int RilContextWrapper::ProcessModemData(void *data, unsigned int datalen)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    if (data != NULL && datalen > 0) {
        ModemData *pModemData = new ModemData((char*)data, datalen, false/*tx*/);
        if (SendMessage(pModemData) < 0) {
            delete pModemData;
            pModemData = NULL;
        }
    }
    return 0;
}

void RilContextWrapper::ClearGarbage(UINT nToken)
{
    if (nToken != TOKEN_INVALID) {
        RemoveRequestHistory(nToken);
    }
}

bool RilContextWrapper::IsInRequestWaitHistory(UINT nToken)
{
    bool result = FALSE;
    MessageHistoryRecord record = m_RequestWaitList.Find(nToken);

    if (record.IsValid()) {
        result = TRUE;
    }

    return result;
}

int RilContextWrapper::SendMessage(RequestData *pRequestData)
{
    //RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    int nServiceId = -1;
    int nMessageId = 0;

    if (pRequestData == NULL) {
        RilLogE("[%s_%d] %s Invalid parameter : pRequestData", TAG, m_rilSocketId, __FUNCTION__);
        return -1;
    }

    if (m_pServiceMgr == NULL || m_pServiceMgr->RouteRequest(pRequestData, nServiceId, nMessageId) < 0) {
        RilLogE("[%s_%d] %s RIL Request Routing error - %d", TAG, m_rilSocketId, __FUNCTION__, pRequestData->GetReqId());
        return -1;
    }

    RilLogV("[%s_%d] %s RIL Request Routing : RCM ID=%d Service ID=%d Message ID=%04d",
                TAG, m_rilSocketId, __FUNCTION__,
                pRequestData->GetReqId(), nServiceId, nMessageId);
    Message *pMsg = Message::ObtainMessage(pRequestData, nServiceId, nMessageId);

    // Check Async message
    if (m_pServiceMgr->IsAsyncMessage(nServiceId, nMessageId) == true) {
        pMsg->SetDirection(ASYNC_REQUEST);
        pMsg->SetAsyncMsgReqStatus(ASYNC_MSG_STATUS_INITIATED);
    }

    if (m_pServiceMgr->SendMessage(pMsg) < 0) {
        RilLogE("[%s_%d] %s SendMessage error", TAG, m_rilSocketId, __FUNCTION__);
        if (pMsg) {
           delete pMsg;
        }
        OnRequestComplete(pRequestData, RIL_E_REQUEST_NOT_SUPPORTED);
        return 0;
    }

    return 0;
}

int RilContextWrapper::SendMessage(ModemData *pModemData)
{
    //RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);

    if (pModemData == NULL) {
        RilLogE("Invalid parameter : pModemData");
        return -1;
    }

    int nServiceId = -1;
    int nMessageId = 0;
    if (pModemData->IsSolicitedResponse()) {
        RilLogV("[%s_%d] %s RCM Type : Solicited Response", TAG, m_rilSocketId, __FUNCTION__);

        MessageHistoryRecord record = m_RequestWaitList.Find(pModemData->GetToken());
        if (record.IsValid()) {
            nServiceId = record.GetDestServiceId();
            nMessageId = record.GetResult();
        }
        m_RequestWaitList.Remove(pModemData->GetToken());
    }
    else if (pModemData->IsUnsolicitedResponse()) {
        RilLogV("[%s_%d] %s RCM Type : Indication", TAG, m_rilSocketId, __FUNCTION__);
        if (m_pServiceMgr->RouteProtocolInd(pModemData->GetMessageId(), nServiceId, nMessageId)) {
            return -1;
        }
    }
    else {
        RilLogE("[%s_%d] %s RCM Type : Unsupported(%d)", TAG, m_rilSocketId, __FUNCTION__, pModemData->GetType());
        return -1;
    }

    RilLogV("[%s_%d] %s RCM Response Routing : RCM ID=0x%04X Service ID=%d Message ID=%04d",
            TAG, m_rilSocketId, __FUNCTION__,
            pModemData->GetMessageId(), nServiceId, nMessageId);
    Message *pMsg = Message::ObtainMessage(pModemData, nServiceId, nMessageId);
    if (m_pServiceMgr->SendMessage(pMsg) < 0) {
        RilLogE("[%s_%d] %s SendMessage error", TAG, m_rilSocketId, __FUNCTION__);
        if (pMsg) {
            delete pMsg;
        }
        return 0;
    }
    return 0;
}

RIL_RadioState RilContextWrapper::GetCurrentRadioState() const
{
    return m_RadioState;
}

void RilContextWrapper::SetRadioState(RIL_RadioState radioState)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    switch (radioState) {
    case RADIO_STATE_UNAVAILABLE:
        mServiceState.setNullState();
        SetNoSim();
        break;
    case RADIO_STATE_OFF:
        mServiceState.setNullState();
        break;
    case RADIO_STATE_ON:
        break;
    default:
        RilLogV("Unsupported Radio State %d", radioState);
        return;
    } // end switch ~

    RilLogV("[%s_%d] %s Overall Radio State changed from %d to %d", TAG, m_rilSocketId, __FUNCTION__, m_RadioState, radioState);
    m_RadioState = radioState;
}

void RilContextWrapper::ProcessSolicitedResponse(int id, const void *data, unsigned int length)
{
    RilLogI("[%s_%d] %s Request ID=%d", TAG, m_rilSocketId, __FUNCTION__, id);
    if (data == NULL || length <= 0) {
        return ;
    }

    int requestId = DECODE_REQUEST(id);
    //int halVer = DECODE_HAL(id);

    switch (requestId) {
    case RIL_REQUEST_OPERATOR:
    case RIL_REQUEST_DATA_REGISTRATION_STATE:
    case RIL_REQUEST_VOICE_REGISTRATION_STATE:
    case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:
        ProcessNetworkServiceState(requestId, data, length);
        break;
    case RIL_REQUEST_GET_SIM_STATUS:
        ProcessIccCardStatus(data, length);
        break;
    case RIL_REQUEST_GET_IMSI:
        ProcessImsi(data, length);
        break;
    default:
        break;
    }
}

void RilContextWrapper::ProcessUnsolicitedResponse(int id, const void *data, unsigned int length)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    //Message *message = NULL;

    if (id == RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED) {
        RilLogV("[%s_%d] %s Process : RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED", TAG, m_rilSocketId, __FUNCTION__);
        if (data != NULL && length == sizeof(int)) {

            RIL_RadioState radioState = (RIL_RadioState)*(int *)data;
            if (m_RadioState != radioState) { //radio state sync with f/w (not the real RIL state)
                SetRadioState(radioState);
            }
        }
    }
    else if(id == RIL_UNSOL_DATA_CALL_LIST_CHANGED) {
        RilLogV("[%s_%d] %s Process : RIL_UNSOL_DATA_CALL_LIST_CHANGED", TAG, m_rilSocketId, __FUNCTION__);
        bool abDataState[MAX_DATA_CALL_SIZE*2+1];
        memset(abDataState, 0, MAX_DATA_CALL_SIZE*2+1);

        // Set CID's Active State
        if (data != NULL && length >= (int)sizeof(RIL_Data_Call_Response)) {
            int nNumList = length>0? length / sizeof(RIL_Data_Call_Response): 0;
            RIL_Data_Call_Response *pDataCall = length>0? (RIL_Data_Call_Response *) data: NULL;
            for(int i=0; i<nNumList; i++) abDataState[pDataCall[i].cid] = (pDataCall[i].active? true: false);
        }

        // Compare and Broadcast
        for(int i=1; i<(MAX_DATA_CALL_SIZE*2+1); i++) {
            if(m_bDataCallStateArray[i] != abDataState[i]) {
                RilLogV("[%s_%d] %s CID:%d state changed", TAG, m_rilSocketId, __FUNCTION__, i);
                m_bDataCallStateArray[i] = abDataState[i];
                RilDataInts *rildata = new RilDataInts(2);
                if (rildata != NULL) {
                    rildata->SetInt(0, i);
                    rildata->SetInt(1, m_bDataCallStateArray[i]);
                    BroadcastSystemMessage(MSG_SYSTEM_DATA_CALL_STATE_CHANGED, rildata);
                    RilLogV("[%s_%d] %s Broadcast System Message : MSG_SYSTEM_DATA_CALL_STATE_CHANGED", TAG, m_rilSocketId, __FUNCTION__);
                } else RilLogE("[%s_%d] %s rildata is NULL", TAG, m_rilSocketId, __FUNCTION__);
            }
        }
    }
}

int RilContextWrapper::BroadcastSystemMessage(int messageId, RilData *data/* = NULL*/)
{
    RilLogI("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    if (m_pServiceMgr != NULL) {
        m_pServiceMgr->BroadcastSystemMessage(messageId, data);
    }
    else {
        delete data;
    }
    return 0;
}

RilContext *RilContextWrapper::GetRilContext(RIL_SOCKET_ID socket_id)
{
    RilLogV("[%s_%d] %s socket_id:%d", TAG, m_rilSocketId, __FUNCTION__, socket_id);
    if ( socket_id == m_rilSocketId )
    {
        RilLogV("[%s_%d] %s return myself", TAG, m_rilSocketId, __FUNCTION__);
        return this;
    }
    else
    {
        if ( m_pRilApp != NULL )
        {
            RilContext * pRilContext = m_pRilApp->GetRilContext(socket_id);
            RilLogV("[%s_%d] %s return socketid(%d)'s RilContext", TAG, m_rilSocketId, __FUNCTION__, socket_id);
            return pRilContext;
        }
        else
        {
            RilLogV("[%s_%d] %s: RilApplication == NULL ", TAG, m_rilSocketId, __FUNCTION__);
        }
    }
    return NULL;
}

RilContext* RilContextWrapper::GetOppositeRilContext(void)
{
    RIL_SOCKET_ID TargetSocketId = (RIL_SOCKET_ID)((int)RIL_SOCKET_NUM-1);
    if ( m_rilSocketId == RIL_SOCKET_NUM-1 )
    {
        TargetSocketId = RIL_SOCKET_1;
    }

    if ( TargetSocketId == m_rilSocketId )
    {
        RilLogW("%s Target socket ID(%d) is same as mine. Compile option could be for Single SIM solution.", __FUNCTION__, TargetSocketId);
        return NULL;
    }

    RilLogV("%s Get RilContext(%d)", __FUNCTION__, TargetSocketId);
    return GetRilContext(TargetSocketId);
}

Service* RilContextWrapper::GetService(int nServiceId)
{
    RilLogV("[%s_%d] %s ", TAG, m_rilSocketId, __FUNCTION__);
    if ( m_pServiceMgr != NULL )
    {
        RilLogV("[%s_%d] %s return service(%d)", TAG, m_rilSocketId, __FUNCTION__, nServiceId);
        return m_pServiceMgr->FindService(nServiceId);
    }
    RilLogV("[%s_%d] %s Cannot find service (%d)", TAG, m_rilSocketId, __FUNCTION__);
    return NULL;
}

unsigned int RilContextWrapper::GetOpenCarrierIndex()
{
    RilLogV("[%s_%d] %s return open carrier Index : 0x%x", TAG, m_rilSocketId, __FUNCTION__, m_nOpenCarrierIndex);
    return m_nOpenCarrierIndex;
}

void RilContextWrapper::SetOpenCarrierIndex(const char* mccmnc)
{
    char buf[128] = { 0, };
    m_nOpenCarrierIndex = GetOcNameByMccMnc(mccmnc);
    snprintf(buf, sizeof(buf), "%u", m_nOpenCarrierIndex);

    switch(m_rilSocketId)
    {
        case RIL_SOCKET_1:
            property_set(RIL_SIM1_OPEN_CARRIER_ID, buf);
            break;
#if (SIM_COUNT >= 2)
        case RIL_SOCKET_2:
            property_set(RIL_SIM2_OPEN_CARRIER_ID, buf);
            break;
#endif
        default:
            RilLogE("[%s_%d] %s invaild ril socket ID, open carrier index(0x%x)", TAG, m_rilSocketId, __FUNCTION__, m_nOpenCarrierIndex);
            break;
    }

    RilLogV("[%s_%d] %s set open carrier index : 0x%x", TAG, m_rilSocketId, __FUNCTION__, m_nOpenCarrierIndex);
}


// set card status into RilContextProperty
void RilContextWrapper::SetCardStatus(RIL_CardStatus_v6 *cardStatus)
{
    if (cardStatus != NULL) {
        // RilContext property
        if (cardStatus->card_state != RIL_CARDSTATE_PRESENT) {
            m_RilContextProperty.Put("gsm_umts_subscription_app_index", -1);
            m_RilContextProperty.Put("cdma_subscription_app_index", -1);
            m_RilContextProperty.Put("ims_subscription_app_index", -1);
            RilLog("[%d] %s CardState ABSENT or ERROR", GetRilSocketId(), __FUNCTION__);
        }
        else {
            m_RilContextProperty.Put("gsm_umts_subscription_app_index", cardStatus->gsm_umts_subscription_app_index);
            m_RilContextProperty.Put("cdma_subscription_app_index", cardStatus->cdma_subscription_app_index);
            m_RilContextProperty.Put("ims_subscription_app_index", cardStatus->ims_subscription_app_index);
            RilLog("[%d] %s SIM/USIM: %s RUIM/CSIM: %s ISIM: %s", GetRilSocketId(), __FUNCTION__,
                    cardStatus->gsm_umts_subscription_app_index >= 0 ? "exist" : "not exist",
                    cardStatus->cdma_subscription_app_index >= 0 ? "exist" : "not exist",
                    cardStatus->ims_subscription_app_index >= 0 ? "exist" : "not exist");
        }

        // RilApplication property
        if (GetApplicationProperty() != NULL) {
            RilProperty *pRilAppProperty = GetApplicationProperty();
            // overallCardState
            // bit 0 : SIM1 card state
            // bit 1 : SIM2 card state
            int overallCardState = pRilAppProperty->GetInt(RIL_SIM_OVERALL_CARDSTATE, 0);
            if (cardStatus->card_state == RIL_CARDSTATE_PRESENT) {
                overallCardState |= (0x01 << GetRilSocketId());
            }
            else {
                overallCardState &= ~(0x01 << GetRilSocketId());
            }
            pRilAppProperty->Put(RIL_SIM_OVERALL_CARDSTATE, overallCardState);
        }
    }
}

void RilContextWrapper::SetNoSim()
{
    RIL_CardStatus_v6 dummyCardStatus;
    memset(&dummyCardStatus, 0, sizeof(dummyCardStatus));
    dummyCardStatus.card_state = RIL_CARDSTATE_ABSENT;
    dummyCardStatus.gsm_umts_subscription_app_index = -1;
    dummyCardStatus.cdma_subscription_app_index = -1;
    dummyCardStatus.ims_subscription_app_index = -1;
    SetCardStatus(&dummyCardStatus);
}

/**
 * static
 */
RilContext *RilContextWrapper::NewInstance(RilApplication *rilApp, const RilContextParam *param)
{
    RilLogI("%s::%s", TAG, __FUNCTION__);
    RilContextWrapper *rilcontext = new RilContextWrapper(rilApp, param);
    return rilcontext;
}

void RilContextWrapper::ProcessNetworkServiceState(int requestId, const void *data, unsigned int  length)
{
    bool hasOperatorNumericChanged = false;
    bool hasVoiceRegStateChanged = false;
    bool hasVoiceRadioTechChanged = false;
    bool hasDataRegStateChanged = false;
    bool hasDataRadioTechChanged = false;
    bool hasLteVopsStatusChanged = false;
    bool hasNrStatusChanged = false;

    ServiceState newState = mServiceState;
    ServiceState &state = mServiceState;
    switch (requestId) {
    case RIL_REQUEST_OPERATOR: {
        ProcessOperatorInfo(newState, data, length);
        hasOperatorNumericChanged = !TextUtils::Equals(state.getOperatorNumeric(), newState.getOperatorNumeric());
        break;
    }
    case RIL_REQUEST_DATA_REGISTRATION_STATE: {
        ProcessNetworkRegStateResult(newState, NETWORK_DOMAIN_PS, data, length);
        hasDataRegStateChanged = (state.getDataRegState() != newState.getDataRegState());
        hasDataRadioTechChanged = (state.getDataRadioTechnology() != newState.getDataRadioTechnology()) ||
                                (state.isUsingCarrierAggregation() != newState.isUsingCarrierAggregation());
        hasLteVopsStatusChanged = (state.getLteVopsSupport() != newState.getLteVopsSupport()) ||
                                    (state.getLteEmcBearerSupport() != newState.getLteEmcBearerSupport());
        hasNrStatusChanged = (state.getNrStatus() != newState.getNrStatus());
        break;
    }
    case RIL_REQUEST_VOICE_REGISTRATION_STATE: {
        ProcessNetworkRegStateResult(newState, NETWORK_DOMAIN_CS, data, length);
        hasVoiceRegStateChanged = (state.getVoiceRegState() != newState.getVoiceRegState());
        hasVoiceRadioTechChanged = (state.getVoiceRadioTechnology() != newState.getVoiceRadioTechnology());
        break;
    }
    case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE: {
        ProcessNetworkSelectionMode(newState, data, length);
        break;
    }
    default:
        return;
    }

    // support legacy. Service::OnVoiceRegistrationStateChanged()
    if (hasVoiceRegStateChanged) {
        RilDataInts *rildata = new RilDataInts(1);
        if (rildata != NULL) {
            rildata->SetInt(0, newState.getVoiceRegState());
            BroadcastSystemMessage(MSG_SYSTEM_VOICE_REGISTRTION_STATE_CHANGED, rildata);
            RilLogV("[%s_%d] %s Broadcast System Message : MSG_SYSTEM_VOICE_REGISTRTION_STATE_CHANGED", TAG, m_rilSocketId, __FUNCTION__);
        }
    }

    // support legacy. Service::OnDataRegistrationStateChanged()
    if (hasDataRegStateChanged) {
        RilDataInts *rildata = new RilDataInts(1);
        if (rildata != NULL) {
            rildata->SetInt(0, newState.getDataRegState());
            BroadcastSystemMessage(MSG_SYSTEM_DATA_REGISTRTION_STATE_CHANGED, rildata);
            RilLogV("[%s_%d] %s Broadcast System Message : MSG_SYSTEM_DATA_REGISTRTION_STATE_CHANGED", TAG, m_rilSocketId, __FUNCTION__);
        }
    }

    bool hasChanged = (hasVoiceRegStateChanged || hasVoiceRadioTechChanged ||
                       hasDataRegStateChanged || hasDataRadioTechChanged ||
                       hasLteVopsStatusChanged || hasNrStatusChanged ||
                       hasOperatorNumericChanged);
    if (hasChanged) {
        RilLog("[%d] hasChanged(%s)\n"
                "    hasOperatorNumericChanged:%d\n"
                "    hasVoiceRegStateChanged:%d\n"
                "    hasVoiceRadioTechChanged:%d\n"
                "    hasDataRegStateChanged:%d\n"
                "    hasLteVopsStatusChanged:%d\n"
                "    hasNrStatusChanged:%d",
                GetRilSocketId(), hasChanged ? "true" : "false",
                hasOperatorNumericChanged,
                hasVoiceRegStateChanged, hasVoiceRadioTechChanged,
                hasDataRegStateChanged, hasLteVopsStatusChanged, hasNrStatusChanged);
        if (VDBG) {
            RilLogV("[%d] OldState=%s \n      NewState=%s",
                    GetRilSocketId(), state.toString().c_str(), newState.toString().c_str());
        }
        BroadcastSystemMessage(MSG_SYSTEM_SERVICE_TATE_CHANGED);
    }

    mServiceState = newState;
}

void RilContextWrapper::ProcessOperatorInfo(ServiceState& ss, const void *data, unsigned int datalen)
{
    if (data != NULL && datalen% sizeof(char *) == 0) {
        size_t size = datalen / sizeof(char *);
        if (size >= 3) {
            string alphaLong = (((char **)data)[0] != NULL) ?  ((char **)data)[0] : "";
            string alphaShort = (((char **)data)[1] != NULL) ?  ((char **)data)[1] : "";
            string numeric = (((char **)data)[2] != NULL) ?  ((char **)data)[2] : "";
            ss.setOperatorNumeric(numeric);
            ss.setOperatorName(alphaLong, alphaShort);
        }
    }
}

void RilContextWrapper::ProcessNetworkSelectionMode(ServiceState& ss, const void *data, unsigned int datalen)
{
    if (data != NULL && datalen == sizeof(int)) {
        bool isManualNetworkSelection = (((int *)data)[0] == 1);
        ss.setIsManualSelection(isManualNetworkSelection);
    }
}

void RilContextWrapper::ProcessNetworkRegStateResult(ServiceState& ss, int domain, const void *networkStateResult, unsigned int datalen)
{
    if (networkStateResult == NULL || datalen == 0) {
        return ;
    }

    if (domain == NETWORK_DOMAIN_CS) {
        // radio 1.0 ~ 1.2
        if (datalen == sizeof(RIL_VoiceRegistrationStateResponse)) {
            ProcessVoiceRegStateResult(ss, (RIL_VoiceRegistrationStateResponse *)networkStateResult);
        }
        else if (datalen == sizeof(RIL_VoiceRegistrationStateResponse_V1_2)) {
            ProcessVoiceRegStateResult(ss, (RIL_VoiceRegistrationStateResponse_V1_2 *)networkStateResult);
        }
    }
    else if (domain == NETWORK_DOMAIN_PS) {
        // radio 1.0 ~ 1.4
        if (datalen == sizeof(RIL_DataRegistrationStateResponse)) {
            ProcessDataRegStateResult(ss, (RIL_DataRegistrationStateResponse *)networkStateResult);
        }
        else if (datalen == sizeof(RIL_DataRegistrationStateResponse_V1_2)) {
            ProcessDataRegStateResult(ss, (RIL_DataRegistrationStateResponse_V1_2 *)networkStateResult);
        }
        else if (datalen == sizeof(RIL_DataRegistrationStateResponse_V1_4)) {
            ProcessDataRegStateResult(ss, (RIL_DataRegistrationStateResponse_V1_4 *)networkStateResult);
        }
    }
}

void RilContextWrapper::ProcessVoiceRegStateResult(ServiceState& ss, RIL_VoiceRegistrationStateResponse *voiceRegResult)
{
    //RilLogV("Process : RIL_REQUEST_VOICE_REGISTRATION_STATE");
    if (voiceRegResult != NULL) {
        ss.setVoiceRegState((int)voiceRegResult->regState);
        ss.setVoiceRadioTechnology((int)voiceRegResult->rat);

        GetProperty()->Put(RIL_CONTEXT_NET_VOICE_REGISTRATION_STATE, ss.getVoiceRegState());
        GetProperty()->Put(RIL_CONTEXT_NET_VOICE_RADIO_TECH, ss.getVoiceRadioTechnology());
    }
}

void RilContextWrapper::ProcessVoiceRegStateResult(ServiceState& ss, RIL_VoiceRegistrationStateResponse_V1_2 *voiceRegResult)
{
    // enough casting a pointer type of RIL_VoiceRegistrationStateResponse.
    return ProcessVoiceRegStateResult(ss, (RIL_VoiceRegistrationStateResponse *)voiceRegResult);
}

void RilContextWrapper::ProcessDataRegStateResult(ServiceState& ss, RIL_DataRegistrationStateResponse *dataRegResult)
{
    //RilLogV("Process : RIL_REQUEST_DATA_REGISTRATION_STATE");
    if (dataRegResult != NULL) {
        ss.setDataRegState((int)dataRegResult->regState);
        ss.setDataRadioTechnology((int)dataRegResult->rat);
        GetProperty()->Put(RIL_CONTEXT_NET_DATA_REGISTRATION_STATE, ss.getDataRegState());
        GetProperty()->Put(RIL_CONTEXT_NET_DATA_RADIO_TECH, ss.getDataRadioTechnology());
    }
}

void RilContextWrapper::ProcessDataRegStateResult(ServiceState& ss, RIL_DataRegistrationStateResponse_V1_2 *dataRegResult)
{
    ProcessDataRegStateResult(ss, (RIL_DataRegistrationStateResponse *)dataRegResult);
}

void RilContextWrapper::ProcessDataRegStateResult(ServiceState& ss, RIL_DataRegistrationStateResponse_V1_4 *dataRegResult)
{
    if (dataRegResult != NULL) {
        // pre-precessing for legacy
        ProcessDataRegStateResult(ss, (RIL_DataRegistrationStateResponse *)dataRegResult);

        // update LTE VoPS and emergency available
        ss.setLteVopsSupport(dataRegResult->lteVopsInfo.isVopsSupported);
        ss.setLteEmcBearerSupport(dataRegResult->lteVopsInfo.isEmcBearerSupported);

        // update NR indicator state
        ss.updateNrStatus(dataRegResult->nrIndicators.isEndcAvailable,
                dataRegResult->nrIndicators.isDcNrRestricted,
                dataRegResult->nrIndicators.isNrAvailable);
    }
}

void RilContextWrapper::ProcessIccCardStatus(const void *data, unsigned int datalen)
{
    if (data == NULL || datalen < sizeof(RIL_CardStatus_v6)) {
        return ;
    }

    if (datalen >= sizeof(RIL_CardStatus_v6)) {
        // radio 1.0
        RIL_CardStatus_v6 *cardStatus = (RIL_CardStatus_v6 *)data;
        SetCardStatus(cardStatus);

        RilDataInts *rildata = new RilDataInts(2);
        if (rildata != NULL) {
            rildata->SetInt(0, cardStatus->card_state);    // RIL_CardState : Absent(0), Present(1), Error(2)

            if (cardStatus->gsm_umts_subscription_app_index >= 0) {
                RIL_AppStatus *gsmApplication = &cardStatus->applications[cardStatus->gsm_umts_subscription_app_index];
                rildata->SetInt(1, gsmApplication->app_state);    // RIL_AppState : Illegal(-1), Unknown(0), Detected(1), Ready(5)
            }
            else {
                rildata->SetInt(1, (int)-1);
            }

            BroadcastSystemMessage(MSG_SYSTEM_SIM_STATUS_CHANGED, rildata);
            RilLogV("[%s_%d] %s Broadcast System Message : MSG_SYSTEM_SIM_STATUS_CHANGED", TAG, m_rilSocketId, __FUNCTION__);
        }
    }

    if (datalen >= sizeof(RIL_CardStatus_V1_2)) {
        // ICCID, ATR, Physical SIM slot
    }

    if (datalen >= sizeof(RIL_CardStatus_V1_4)) {
        // EID
    }
}

void RilContextWrapper::ProcessImsi(const void *data, unsigned int datalen)
{
    if (data == NULL || datalen == 0) {
        return ;
    }

    string imsi = (const char *)data;
    // RIL_CONTEXT_GSM_SIM_AID and RIL_CONTEXT_GSM_SIM_IMSI are set by SIM service.
    // OnGetImsiDone()
    string gsmUmtsAid = m_RilContextProperty.GetString(RIL_CONTEXT_GSM_SIM_AID, "");
    string gsmUmtsImsi = m_RilContextProperty.GetString(RIL_CONTEXT_GSM_SIM_IMSI, "");

    if (!TextUtils::IsEmpty(gsmUmtsAid) && !TextUtils::IsEmpty(gsmUmtsImsi) &&
            TextUtils::Equals(gsmUmtsImsi, imsi)) {
        RilDataStrings *rildata = new RilDataStrings(2);
        if (rildata != NULL) {
            rildata->SetString(0, gsmUmtsAid.c_str());
            rildata->SetString(1, gsmUmtsImsi.c_str());
            BroadcastSystemMessage(MSG_SYSTEM_IMSI_UPDATED, rildata);
            RilLogV("[%d] Broadcast System Message : MSG_SYSTEM_IMSI_UPDATED", GetRilSocketId());
        }
    }
}

void RilContextWrapper::ResetModem(const char *reason)
{
    if (m_pRilApp != NULL) {
        m_pRilApp->ResetModem(reason);
    }
}
