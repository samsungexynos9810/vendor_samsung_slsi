/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "open_carrier.h"
#include "protocoladapter.h"
#include "rillog.h"
#include "reset_util.h"
#include "rilapptoken.h"
#include "service.h"
#include "servicestate.h"
#include "util.h"
#include "cscservice.h"
#include "networkservice.h"

static bool debug = true;
static bool VDBG = false;

#define SIGTIMER (SIGRTMAX)

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__);} while(0)
#undef RilLogV
#define RilLogV(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__);} while(0)
#undef RilLogW
#define RilLogW(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__);} while(0)
#undef RilLogE
#define RilLogE(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__);} while(0)

#define writeRilEvent(format1, format2, ...)   CRilEventLog::writeRilEvent(RIL_LOG_CAT_NET, format1, format2, ##__VA_ARGS__)

/**
 * RilDataResp
 */
class RilDataResp : public RilData {
private:
    char *m_resp;
    int m_respLen;
    int m_id;
    int m_result;
public:
    RilDataResp(int id, int result, const void *resp, int respLen) : m_resp(0), m_respLen(0) {
        m_id = id;
        m_result = result;
        if (resp != NULL && respLen > 0) {
            m_resp = new char[respLen];
            m_respLen = respLen;
            memcpy(m_resp, resp, respLen);
        }
    }

    virtual ~RilDataResp() {
        if (m_resp != NULL) {
            delete m_resp;
        }
    }

public:
    int GetId() const { return m_id; }
    int GetResult() const { return m_result; }
    void *GetData() const { return m_resp; }
    unsigned int GetDataLength() const { return m_respLen; }
};

/**
 * RilResponseInternalListener
 */
class RilResponseInternalListener : public RilApplicationContext
{
private:
    Service *service;
    int requestId;
public:
    RilResponseInternalListener(Service *service, int requestId) {
        this->requestId = requestId;
        this->service = service;
    }
    virtual ~RilResponseInternalListener() {}

public:
    // response
    void OnRequestComplete(RIL_Token t, RIL_Errno e, void *response, unsigned int responselen) {
        RilLogV("OnRequestInternalComplete(id=%d result=%d data=%p length=%d)", requestId, e, response, responselen);
        if (service != NULL) {
            service->OnRequestInternalComplete(t, requestId, e, response, responselen);
        }
        else {
            RilLogW("No target to be called for response");
        }
    }
    void OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen) {}
    void OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen, RIL_SOCKET_ID socket_id) {}
    void OnRequestAck(RIL_Token t) {}

public:
    static RilResponseInternalListener *NewInstance(Service *service, int requestId) {
        if (requestId < 0) {
            return NULL;
        }

        if (service == NULL) {
            return NULL;
        }

        return new RilResponseInternalListener(service, requestId);
    }
};



IMPLEMENT_MODULE_TAG(Service, Service)

const char *Service::GetServiceName(UINT nServiceId)
{
    switch (nServiceId) {
    case RIL_SERVICE_UNKNOWN:
        return "Service";
    case RIL_SERVICE_CSC:
        return "CsService";
    case RIL_SERVICE_PS:
        return "PsService";
    case RIL_SERVICE_SIM:
        return "SimService";
    case RIL_SERVICE_MISC:
        return "MiscService";
    case RIL_SERVICE_NETWORK:
        return "NetworkService";
    case RIL_SERVICE_SMS:
        return "SmsService";
    case RIL_SERVICE_AUDIO:
        return "AudioService";
    case RIL_SERVICE_IMS:
        return "ImsService";
    case RIL_SERVICE_GPS:
        return "GpsService";
    case RIL_SERVICE_WLAN:
        return "WLanService";
    case RIL_SERVICE_VSIM:
        return "VSimService";
    case RIL_SERVICE_STK:
        return "StkService";
    case RIL_SERVICE_TEST:
        return "TestService";
    case RIL_SERVICE_SUPPLEMENTARY:
        return "SupplementaryService";
    case RIL_SERVICE_EMBMS:
        return "EmbmsService";
    default:
        return "SERVICE_UNSUPPORTED";
    } // end switch ~
}

const char *Service::GetServiceName(const Service *pService)
{
    if (pService != NULL) {
        return GetServiceName(pService->GetServiceId());
    }
    return "RIL_SERVICE_UNKNOWN";
}

Service::Service(RilContext* pRilContext, UINT nServiceId/* = RIL_SERVICE_UNKNOWN*/)
    : m_pRilContext(pRilContext)
    , m_nMsgPipeR(-1)
    , m_nMsgPipeW(-1)
{
    m_nServiceState = UNAVAILABLE;
    m_pPoll = NULL;
    m_pServiceMonitorRunnable = NULL;
    m_bIsInited = false;
    memset(&m_tvStart, 0, sizeof(m_tvStart));
    m_pMutex = NULL;
    m_pCurReqMsg = NULL;
    m_nVoiceNetworkState = NOT_REGISTERED;
    m_nDataNetworkState = NOT_REGISTERED;
    m_nOverallRadioState = RADIO_STATE_UNAVAILABLE;
    m_nr_count = 0;
    m_nCurrentReqId = 0;

    // Priority Req Interface
    m_nAsycMsgReqToken = TOKEN_INVALID;
    m_pAsyncMsgHistory = NULL;

    SetServiceId(nServiceId);
    mModemState = MS_OFFLINE;
}

Service::~Service()
{
    ClearMessageQueue();
    AsycMsgReqClear();

    if(m_pPoll)
    {
        delete m_pPoll;
        m_pPoll = NULL;
    }

    if (-1 != m_nMsgPipeR)
    {
        RilLogW("Stop error: m_nMsgPipeR is -1");
        close(m_nMsgPipeR);
        m_nMsgPipeR = -1;
    }

    if (-1 != m_nMsgPipeW)
    {
        RilLogW("Stop error: m_nMsgPipeW is -1");
        close(m_nMsgPipeW);
        m_nMsgPipeW = -1;
    }

    if(m_pMutex)
    {
        delete m_pMutex;
        m_pMutex = NULL;
    }
}

int Service::Init()
{
    RilLogI("[%s] %s++", m_szSvcName, __FUNCTION__);
    if (m_bIsInited) {
        RilLogW("[%s] %s already initialized", m_szSvcName, __FUNCTION__);
        return -1;
    }

    // callback OnCreate() method
    // all initialization code for each Service instance should be here.
    // every Service instance should override.
    if (OnCreate(m_pRilContext) < 0) {
        RilLogE("[%s] %s OnCreate error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    m_nServiceState = Service::CREATED;

    m_pAsyncMsgHistory = new CAsyncMsgReqHistory();
    m_pMutex = new CMutex();
    if(m_pMutex == NULL) {
        RilLogE("[%s] %s Fail to create Mutex instance", m_szSvcName, __FUNCTION__);
        return -1;
    }

    if (InitMessageQueue() < 0) {
        RilLogE("[%s] %s Fail to initialize message queue", m_szSvcName, __FUNCTION__);
        return -1;
    }

    if (OpenMessagePipe() < 0)
    {
        RilLogE("[%s] %s Fail to open message pipe", m_szSvcName, __FUNCTION__);
        return -1;
    }

    m_bIsInited = true;
    RilLogI("[%s] %s--", m_szSvcName, __FUNCTION__);
    return 0;
}

int Service::Start()
{
    if (m_pServiceMonitorRunnable == NULL) {
        m_pServiceMonitorRunnable = new ServiceMonitorRunnable(this);
    }

    m_pPoll = new Thread(m_pServiceMonitorRunnable);
    if (m_pPoll != NULL && m_pPoll->Start() < 0) {
        RilLogE("[%s] %s Fail to start ServiceMonitor thread.", m_szSvcName, __FUNCTION__);
        return -1;
    }
    m_nServiceState = Service::STARTED;

    return 0;
}

int Service::Finalize()
{
    RilLogI("++[%s] %s", m_szSvcName, __FUNCTION__);
    if (m_bIsInited == false) {
        RilLogW("Service(base) finalize error");
        RilLogW("[%s] %s Finalize but not initialized", m_szSvcName, __FUNCTION__);
        return -1;
    }

    if (m_pPoll != NULL) {
        m_pPoll->Stop();

        if (m_pServiceMonitorRunnable != NULL) {
            delete m_pServiceMonitorRunnable;
            m_pServiceMonitorRunnable = NULL;
        }

        delete m_pPoll;
        m_pPoll = NULL;
    }

    if (m_nMsgPipeR > 0) {
        close(m_nMsgPipeR);
        m_nMsgPipeR = -1;
    }

    if (m_nMsgPipeW > 0) {
        close(m_nMsgPipeW);
        m_nMsgPipeW = -1;
    }

    ClearMessageQueue();
    AsycMsgReqClear();

    // callback OnDestroy()
    OnDestroy();
    m_nServiceState = Service::DESTROYED;

    m_bIsInited = false;
    RilLogI("--[%s] %s", m_szSvcName, __FUNCTION__);
    return 0;
}

void Service::SetServiceId(UINT nServiceId)
{
    m_nServiceId = nServiceId;
    if (m_pRilContext != NULL) {
        snprintf(m_szSvcName, sizeof(m_szSvcName)-1, "%s_%d",
                GetServiceName(nServiceId), m_pRilContext->GetRilSocketId());
    }
    else {
        snprintf(m_szSvcName, sizeof(m_szSvcName)-1, "%s", GetServiceName(nServiceId));
    }
}

void Service::SetServiceMonitorRunnable(ServiceMonitorRunnable *pServiceMonitorRunnable)
{
    if (m_bIsInited && m_nServiceState != Service::STARTED) {
        if (m_pServiceMonitorRunnable != NULL) {
            delete m_pServiceMonitorRunnable;
        }
        m_pServiceMonitorRunnable = pServiceMonitorRunnable;
    }
}

int Service::InitMessageQueue()
{
    return 0;
}

void Service::ClearMessageQueue()
{
    /*
    queue<Message *> m_requestQueue;
    queue<Message *> m_responseQueue;
    queue<Message *> m_internalQueue;
    */
    Message *message = NULL;
    while (!m_requestQueue.empty()) {
        message = m_requestQueue.front();
        if (message != NULL) {
            delete message;
        }
        m_requestQueue.pop();
    } // end while ~
    while (!m_responseQueue.empty()) {
        message = m_responseQueue.front();
        if (message != NULL) {
            delete message;
        }
        m_responseQueue.pop();
    } // end while ~
    while (!m_internalQueue.empty()) {
        message = m_internalQueue.front();
        if (message != NULL) {
            delete message;
        }
        m_internalQueue.pop();
    } // end while ~
    while (!m_asyncMsgReqQueue.empty()) {
        message = m_asyncMsgReqQueue.front();
        if (message != NULL) {
            delete message;
        }
        m_asyncMsgReqQueue.pop();
    }
}

int Service::OpenMessagePipe()
{
    if (m_nMsgPipeR != -1)
    {
        RilLogV("Open command pipe again(m_nMsgPipeR)");
        close(m_nMsgPipeR);
        m_nMsgPipeR = -1;
    }

    if (m_nMsgPipeW != -1)
    {
        RilLogV("Open command pipe again(m_nMsgPipeW)");
        close(m_nMsgPipeW);
        m_nMsgPipeW = -1;
    }

    int fds[2];
    int n = pipe(fds);
    if (n < 0)
    {
        RilLogE("Command pipe create fail");
        return -1;
    }

    m_nMsgPipeR = fds[0];
    m_nMsgPipeW = fds[1];
    return 0;
}

int Service::EnQueue(Message *pMsg)
{
    if(NULL == pMsg) {
        RilLogE("Message enqueue fail: message is NULL");
        return -1;
    }

    //Enqueue message due to different direction
    switch (pMsg->GetDirection()) {
    case REQUEST:
        m_pMutex->lock();
        m_requestQueue.push(pMsg);
        m_pMutex->unlock();
        break;
    case RESPONSE:
        m_pMutex->lock();
        m_responseQueue.push(pMsg);
        m_pMutex->unlock();
        break;
    case INTERNAL:
        m_pMutex->lock();
        m_internalQueue.push(pMsg);
        m_pMutex->unlock();
        break;
    case ASYNC_REQUEST:
        m_pMutex->lock();
        m_asyncMsgReqQueue.push(pMsg);
        m_pMutex->unlock();
        break;
    default:
        //these value shouldn't be set, report error
        RilLogE("[%s] %s(): message direction error", m_szSvcName, __FUNCTION__);
        return -1;
    } // end switch ~

    return 0;
}

Message* Service::reqDeQ()
{
    m_pMutex->lock();
    if (m_requestQueue.empty())
    {
        m_pMutex->unlock();
        return NULL;
    }
    Message *message = m_requestQueue.front();
    m_requestQueue.pop();
    m_pMutex->unlock();
    return message;
}

Message* Service::respDeQ()
{
    m_pMutex->lock();
    if (m_responseQueue.empty())
    {
        m_pMutex->unlock();
        return NULL;
    }
    Message *message = m_responseQueue.front();
    m_responseQueue.pop();
    m_pMutex->unlock();
    return message;
}

/* function: internalDeQ
    description: de-queue a message from internal event queue

    change log:
    13-06-05 songqiao.yin initial version
*/
Message* Service::internalDeQ()
{
    m_pMutex->lock();
    if (m_internalQueue.empty())
    {
        m_pMutex->unlock();
        return NULL;
    }
    Message *message = m_internalQueue.front();
    m_internalQueue.pop();
    m_pMutex->unlock();
    return message;
}

Message* Service::asycMsgReqDeQ()
{
    m_pMutex->lock();
    if (m_asyncMsgReqQueue.empty())
    {
        m_pMutex->unlock();
        return NULL;
    }
    Message *message = m_asyncMsgReqQueue.front();
    m_asyncMsgReqQueue.pop();
    m_pMutex->unlock();
    return message;
}

int Service::NotifyNewMessage(MsgDirection direction)
{
    if (m_nMsgPipeW != -1)
    {
        m_pMutex->lock();
        INT8 bVal = (INT8)direction;
        if (write(m_nMsgPipeW, &bVal, 1) > 0)
        {
            m_pMutex->unlock();
            return 0;
        }
        m_pMutex->unlock();
    }

    RilLogE("Notify new message fail: m_nMsgPipeW is -1");
    return -1;
}

int Service::NotifyNewMessage(Message *pMsg)
{
    return NotifyNewMessage(pMsg->GetDirection());
}

void Service::NotifyNextRequestMessage()
{
    //Before notify, delete current message
    if(m_pCurReqMsg != NULL) {
        delete m_pCurReqMsg;
        m_pCurReqMsg = NULL;
    }

    NotifyNewMessage(REQUEST);

    return;
}

bool Service::PostponeRequestMessage(unsigned int millis)
{
    // PostponeRequestMessage can be called both
    // HandleRequest and HandleResponse
    Message *postponeMsg = m_pCurReqMsg;
    if (postponeMsg != NULL) {
        RilLogV("[%s] PostponeRequestMessage Postpone Message:{id=%d}", GetServiceName(), postponeMsg->GetMsgId());
        m_pCurReqMsg = NULL;
        postponeMsg->SetModemData(NULL);
        postponeMsg->SetTimeout(0);

        if (m_pRilContext != NULL) {
            // try to remove request history record if garbage token is remained
            int token = postponeMsg->GetToken();
            if (token != TOKEN_INVALID) {
                m_pRilContext->ClearGarbage(token);
            }
        }

        // Unset Transaction ID
        RilLogV("[%s] Unset transaction (%d)", GetServiceName(), m_nCurrentReqId);
        UnsetTransaction();

        // postpone
        mTimerList.erase(postponeMsg);
        mTimerList.put(postponeMsg, millis);
    }

    NotifyNextRequestMessage();

    return true;
}

/**
 * An initializer of Service instance.
 * call-back by Init() method in Service
 *
 * @param pRilContext - a pointer of RilContext instance
 * @return return 0 if success, negative value if error
 */
int Service::OnCreate(RilContext *pRilContext)
{
    return 0;
}

void Service::OnStart()
{

}

/**
 * An end point of Service instance.
 * call-back by Finalize() method in Service
 *
 * @param pRilContext - a pointer of RilContext instance
 */
void Service::OnDestroy()
{
}


BOOL Service::OnHandleRequest(Message* pMsg)
{
    // DO NOT be set m_pCurReqMsg inside HandleReq() method
    return FALSE;
}

// @return must be TRUE when OnRequestComplete and PostponeRequestMessage
BOOL Service::OnHandleSolicitedResponse(Message* pMsg)
{
    return FALSE;
}
BOOL Service::OnHandleUnsolicitedResponse(Message* pMsg)
{
    return FALSE;
}

BOOL Service::OnHandleInternalMessage(Message* pMsg)
{
    return FALSE;
}

BOOL Service::OnHandleRequestTimeout(Message* pMsg)
{
    return FALSE;
}

int Service::StartRequestTimeout()
{
    if (m_pCurReqMsg != NULL) {
        if (m_pCurReqMsg->GetTimeout() <= 0) {
            m_pCurReqMsg->SetTimeout(60000);    // 1 min.
        }

        // timeout for the current transaction
        int elapse = m_pCurReqMsg->GetTimeout();
        mTimerList.erase(m_pCurReqMsg);
        mTimerList.put(m_pCurReqMsg, elapse);
    }
    return 0;
}

int Service::SetRequestTimeout(UINT timeout)
{
    if (m_pCurReqMsg == NULL) {
        return -1;
    }

    m_pCurReqMsg->SetTimeout(timeout);
    return 0;
}

int Service::ResetRequestTimeout(UINT timeout)
{
    if (SetRequestTimeout(timeout) < 0) {
        return -1;
    }

    // reset started time
    StartRequestTimeout();

    return 0;
}

int Service::CalcNextTimeout(struct timeval *tv)
{
    if (tv == NULL || mTimerList.size() == 0) {
        // infinite
        return -1;
    }

    TimerEvent front = mTimerList.front();
    struct timeval now = TimerList::getNow();
    TimerList::sub(front.timeout, now);
    tv->tv_sec = front.timeout.tv_sec;
    tv->tv_usec = front.timeout.tv_usec;

    if (front.msg == NULL) {
        tv->tv_sec = tv->tv_usec = 0;
    }

    return 0;
}

int Service::SendRequest(ModemData *pModemData, UINT nTimeout, UINT nResult, Message *pMsg/* = NULL*/)
{
    bool isAsycMsg = ( pMsg != NULL && pMsg->GetAsyncMsgReqStatus() != ASYNC_MSG_STATUS_NONE);

    if (pModemData == NULL) {
        RilLogE("%s::%s() : pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    if (m_pRilContext == NULL) {
        RilLogE("%s::%s() : m_pRilContext is NULL", m_szSvcName, __FUNCTION__);
        delete pModemData;
        return -1;
    }

    if (isAsycMsg == FALSE) {
        // normal sync msg case
        if (m_pCurReqMsg == NULL) {
            RilLogE("%s::%s() : m_pCurReqMsg is NULL", m_szSvcName, __FUNCTION__);
            delete pModemData;
            return -1;
        }

        m_pCurReqMsg->SetModemData(pModemData);
        m_pCurReqMsg->SetTimeout(nTimeout);
        if (m_pRilContext->Send(pModemData, m_nServiceId, nResult) < 0) {
            RilLogE("%s::%s() : m_pRilContext->Send2Modem error", m_szSvcName, __FUNCTION__);
            return -1;
        }
    } else {
        // async msg case
        pMsg->SetModemData(pModemData);
        pMsg->SetTimeout(nTimeout);
        if (m_pRilContext->Send(pModemData, m_nServiceId, nResult) < 0) {
            RilLogE("%s::%s() : m_pRilContext->Send2Modem error", m_szSvcName, __FUNCTION__);
            return -1;
        }

        m_nAsycMsgReqToken = pModemData->GetToken();
    }

    return 0;
}

int Service::SendRequest(ModemData *pModemData, UINT nTimeout)
{
    if (m_pCurReqMsg == NULL) {
        return -1;
    }
    return SendRequest(pModemData, nTimeout, m_pCurReqMsg->GetMsgId());
}

int Service::SendRequest(ModemData *pModemData)
{
    if (pModemData == NULL) {
        RilLogE("%s::%s() : pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    if (m_pRilContext != NULL && m_pRilContext->Send((void *)pModemData->GetRawData(), pModemData->GetLength()) < 0) {
        RilLogE("%s::%s() : m_pRilContext->Send2Modem error", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int Service::OnRequestComplete(int result, void *data/* = NULL*/, int length/* = 0*/, Message *pMsg/* = NULL*/)
{
    bool isAsycMsg = ( pMsg != NULL && pMsg->GetAsyncMsgReqStatus() != ASYNC_MSG_STATUS_NONE);

    if (isAsycMsg == FALSE) {
        // normal sync msg case
        if (m_pCurReqMsg != NULL) {
            RequestData *pRequest = m_pCurReqMsg->GetRequestData();
            int token = m_pCurReqMsg->GetToken();
            if (m_pRilContext != NULL) {
                RilLogV("[%s] %s() MSG_ID=%d", m_szSvcName, __FUNCTION__, m_pCurReqMsg->GetMsgId());
                m_pRilContext->OnRequestComplete(pRequest, result, data, length);

                // try to remove request history record if garbage token is remained
                m_pRilContext->ClearGarbage(token);
            }

            // remove timer
            mTimerList.erase(m_pCurReqMsg);
        }

        // Unset Transaction ID
        RilLogV("[%s] Unset transaction (%d)", m_szSvcName, m_nCurrentReqId);
        UnsetTransaction();
        NotifyNextRequestMessage();
    } else {
        // async msg case
        Message *pRequestMsg = NULL;
        RequestData *pRequest = NULL;
        int token = TOKEN_INVALID;
        AsyncMsgReqStatus asyncMsgReqStatus = pMsg->GetAsyncMsgReqStatus();

        if (pMsg->GetDirection() == ASYNC_REQUEST) {
            pRequestMsg = pMsg;
            pRequest = pRequestMsg->GetRequestData();

            if (asyncMsgReqStatus == ASYNC_MSG_STATUS_SENT2CP) {
                // case1: SIT time out case
                // it is called in OnHandleRequestTimeout(), pMsg is request message.
                token = pMsg->GetToken();
                if (m_pAsyncMsgHistory != NULL) m_pAsyncMsgHistory->Remove(token);
            } else {
                // unwanted scenario
                RilLogW("[%s] unwanted asyn for request msgId = %d ", m_szSvcName, pMsg->GetMsgId());
            }
        } else {
            if (asyncMsgReqStatus == ASYNC_MSG_STATUS_RESPONSE) {
                // case3: SIT respone is received. pMsg is response msg.
                token = pMsg->GetToken();
                if (m_pAsyncMsgHistory != NULL) {
                    pRequestMsg = m_pAsyncMsgHistory->Find(token);
                    if (pRequestMsg != NULL) pRequest = pRequestMsg->GetRequestData();
                    m_pAsyncMsgHistory->Remove(token);
                }
            } else {
                // unwanted scenario
                RilLogW("[%s] unwanted asyn for response msgId = %d ", m_szSvcName, pMsg->GetMsgId());
            }
        }

        RilLogV("[%s] %s() AsyncMsgReqStatus = %d, token = 0x%x", m_szSvcName, __FUNCTION__, asyncMsgReqStatus, token);
        if(m_pRilContext != NULL) {
            m_pRilContext->OnRequestComplete(pRequest, result, data, length);

            // try to remove request history record if garbage token is remained
            m_pRilContext->ClearGarbage(token);
        }

        // delete request message
        if (pRequestMsg != NULL) {
            delete pRequestMsg;
        }
    }

    return 0;
}

int Service::OnUnsolicitedResponse(int id, const void *data/* = NULL*/, int length/* = 0*/)
{
    if (m_pRilContext != NULL) {
        m_pRilContext->OnUnsolicitedResponse(id, data, length);
    }
    return 0;
}

int Service::OnRequestAck()
{
    if (m_pCurReqMsg != NULL) {
        RequestData *pRequest = m_pCurReqMsg->GetRequestData();

        if (m_pRilContext != NULL) {
            m_pRilContext->OnRequestAck(pRequest->GetToken());
        }
    }
    return 0;
}

int Service::HandleRequest()
{
    if (m_pCurReqMsg == NULL) {
        Message *msg = reqDeQ();
        if (msg == NULL) {
            //RilLogW("reqDeQ() : NULL");
            return -1;
        }
        RequestData *pReqData = msg->GetRequestData();

        if (pReqData == NULL) {
            RilLogE("%s::%s() : No RIL Request Data, msg(%p)", m_szSvcName, __FUNCTION__, msg);
#if 0
            RilLogV("%s : dequeued msg(%p)", __FUNCTION__, msg);
            Message *stackedQ = reqDeQ();
            while ( NULL != stackedQ )
            {
                RilLogV("%s : stacked msg(%p)", __FUNCTION__, msg);
                stackedQ = reqDeQ();
            }
#endif
            RilLogE("%s : before delete (%p)", __FUNCTION__, msg);
            delete msg;
            return -1;
        }


        int requestId = pReqData->GetReqId();
        bool allowedRadioUnavailable = IsPossibleToPassInRadioUnavailableState(requestId);
        bool allowedRadioOff = IsPossibleToPassInRadioOffState(requestId);
        if(m_nOverallRadioState == RADIO_STATE_UNAVAILABLE)
        {
            if (!allowedRadioUnavailable)
            {
                RilLogV("[%s_%d] %s Current Radio State is RADIO_STATE_UNAVAILABLE : RIL Request ID=%d",
                    m_szSvcName, m_pRilContext->GetRilSocketId(),__FUNCTION__, pReqData->GetReqId());
                m_pRilContext->OnRequestComplete(pReqData, RIL_E_RADIO_NOT_AVAILABLE);
                NotifyNextRequestMessage();
                return 0;
            }
        }

        if (m_nOverallRadioState == RADIO_STATE_OFF)
        {
            if (!(allowedRadioUnavailable || allowedRadioOff))
            {
                RilLogV("[%s_%d] %s Current Radio State is RADIO_STATE_OFF : RIL Request ID=%d",
                    m_szSvcName, m_pRilContext->GetRilSocketId(), __FUNCTION__, pReqData->GetReqId());
                m_pRilContext->OnRequestComplete(pReqData, RIL_E_RADIO_NOT_AVAILABLE);
                NotifyNextRequestMessage();
                return 0;
            }
        }

        // Set Transaction ID
        if ( msg != NULL )
        {
            RequestData* pReqData = msg->GetRequestData();
            RilLogV("[%s] Set transaction (%d)", m_szSvcName, pReqData->GetReqId());
            if ( pReqData != NULL )
            {
                SetTransaction((UINT)(pReqData->GetReqId()));
            }
        }

        // m_pCurReqMsg is only set by OnHandleRequest inside
        m_pCurReqMsg = msg;
        RilLogV("Enter %s::HandleReq()", m_szSvcName);
        BOOL ret = OnHandleRequest(msg);
        RilLogV("Exit %s::HandleReq() : ret=%s", m_szSvcName, ret ? "TRUE" : "FALSE");
        if (!ret) {
            RilLogE("HandleReq(msg) error : OnRequestComplete(RIL_E_GENERIC_FAILURE) to the RILD");
            m_pRilContext->OnRequestComplete(pReqData, RIL_E_GENERIC_FAILURE);

            // Unset Transaction ID
            RilLogV("[%s] Unset transaction (%d)", m_szSvcName, m_nCurrentReqId);
            UnsetTransaction();

            NotifyNextRequestMessage();
            return 0;
        }

        StartRequestTimeout();
    }
    else {
        if (debug) {
            RilLogV("[%s] Now on being IPC transaction : Message ID=%04d", m_szSvcName, m_pCurReqMsg->GetMsgId());
        }
    }

    return 0;
}

int Service::HandleResponse()
{
    m_nr_count= 0;    //reset no response count

    Message *msg = respDeQ();
    if (msg == NULL) {
        RilLogW("[%s] %s() : respDeQ() is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *modemData = NULL;
    if ((modemData = msg->GetModemData()) == NULL) {
        RilLogE("[%s] %s() : No Modem Data", m_szSvcName, __FUNCTION__);
        delete msg;
        return -1;
    }

    if (modemData->IsSolicitedResponse()) {
        bool isReqRspMatched = FALSE;
        AsyncMsgReqStatus asyncStatus = msg->GetAsyncMsgReqStatus();
        // sync msg check
        if (m_pCurReqMsg != NULL && m_pCurReqMsg->GetToken() == msg->GetToken()) {
            isReqRspMatched = TRUE;
        }

        // async msg check
        if (isReqRspMatched == FALSE && asyncStatus == ASYNC_MSG_STATUS_RESPONSE) {
            isReqRspMatched = TRUE;
        }

        if (isReqRspMatched == FALSE) {
            RilLogE("[%s] %s() : No Requested message. Drop response message.", m_szSvcName, __FUNCTION__);
            delete msg;
            return -1;
        }

        bool handled = false;
        RilLogV("Enter %s::%s()", m_szSvcName, __FUNCTION__);
        handled = OnHandleSolicitedResponse(msg);
        RilLogV("Exit %s::%s()", m_szSvcName, __FUNCTION__);
        if (!handled) {
            OnRequestComplete(RIL_E_GENERIC_FAILURE, NULL, 0, msg);
        }
    }
    else if (modemData->IsUnsolicitedResponse()){
        RilLogV("Enter %s::%s()", m_szSvcName, __FUNCTION__);
        OnHandleUnsolicitedResponse(msg);
        RilLogV("Exit %s::%s()", m_szSvcName, __FUNCTION__);
    }

    if (msg) {
        delete msg;
    }

    return 0;
}

void Service::CheckCountsForTimeOut(ModemData *pModemData)
{
    if ( pModemData == NULL) return;

    if ( ++m_nr_count > MAX_NR_COUNT )
    {
        RilLogE("There is NO RESPONSE from CP more than %d times", MAX_NR_COUNT);

        const BOOL bModemCrash = TRUE;
        if ( bModemCrash == TRUE /*&& RilProperty::IsUserMode() == false*/ )
        {
            RilLogE("Request to make CP Forced Crash");

            // Request CP Crash Dump
            int rcmId = pModemData->GetMessageId();
            RequestData *pData = RilParser::CreateRawData(RIL_REQUEST_OEM_MODEM_DUMP, 0, (char *)(&rcmId), sizeof(int));
            if (pData != NULL) {
                Message *msg = Message::ObtainMessage(pData, RIL_SERVICE_MISC, MSG_MISC_OEM_FCRASH_MNR_REQ);
                writeRilEvent(m_szSvcName, __FUNCTION__, "###################################################################################");
                writeRilEvent(m_szSvcName, __FUNCTION__, "####### CP crash due to %s(0x%04x) #######", rcmMsgToString(rcmId), pModemData->GetToken());
                writeRilEvent(m_szSvcName, __FUNCTION__, "###################################################################################");
                if (m_pRilContext->GetServiceManager()->SendMessage(msg) < 0) {
                    if (msg) {
                        delete msg;
                    }
                }
            }
        }
        else
        {
            RilErrorReset("IPC_TIMEOUT(MNR)");
        }
    }
}

int Service::HandleRequestTimeout()
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);

    TimerEvent te = mTimerList.pop();
    if (te.msg != NULL) {
        if (te.msg == m_pCurReqMsg) {
            RequestData *req = m_pCurReqMsg->GetRequestData();
            if (req != NULL) {
                RilLogE("[%s] %s RIL Request(%d) TIMEOUT", m_szSvcName, __FUNCTION__, req->GetReqId());
                writeRilEvent(m_szSvcName, __FUNCTION__, "TIMEOUT RIL Req info req(%d), srcId(%s), msgId(%d), timeout(%d)",
                    req->GetReqId(), GetServiceName(m_pCurReqMsg->GetSvcId()), m_pCurReqMsg->GetMsgId(), m_pCurReqMsg->GetTimeout());

                // keep token to be removed
                unsigned int token = m_pCurReqMsg->GetToken();

                ModemData *pModemData = m_pCurReqMsg->GetModemData();
                if ( pModemData != NULL ) {
                    RilLogE("[%s] TIMEOUT IPC Info : Id=0x%04x(%s), token=0x%04x",
                        m_szSvcName, pModemData->GetMessageId(), rcmMsgToString(pModemData->GetMessageId()),
                        pModemData->GetToken());
                    writeRilEvent(m_szSvcName, __FUNCTION__, "TIMEOUT IPC info %s(0x%04x), token(0x%04x)",
                        rcmMsgToString(pModemData->GetMessageId()), pModemData->GetMessageId(), pModemData->GetToken());
                }

                // if requested message was already handled in OnHandleRequestTimeout method, MUST be returned true.
                if (OnHandleRequestTimeout(m_pCurReqMsg)) {
                    return 0;
                }

                CheckCountsForTimeOut(pModemData);

                m_pRilContext->OnRequestTimeout(req, token);
                NotifyNextRequestMessage();
            }
        }
        else {
            // handle postpone messages
            // push at the end of request queue
            Message *postponeMsg = te.msg;
            RilLogI("[%s]%s handling postpone Message:{id=%d}", GetServiceName(), __FUNCTION__, postponeMsg->GetMsgId());
            EnQueue(postponeMsg);
            NotifyNewMessage(REQUEST);
        }
    }
    else {
        RilLogW("[%s]%s TimerEvent has NULL message.", GetServiceName(), __FUNCTION__);
    }
    return 0;
}

int Service::HandleInternalMessage()
{
    Message* msg = internalDeQ();
    if (msg == NULL) {
        //RilLogW("internalDeQ() : NULL");
        return -1;
    }

    switch (msg->GetMsgId()) {
    case MSG_SYSTEM_RADIO_STATE_CHANGED:
    {
        if (VDBG)
            RilLogV("[%s] OnRadioStateChanged()", m_szSvcName);
        int radioState = RADIO_STATE_UNAVAILABLE;
        RilDataInts *rildata = (RilDataInts *)msg->GetUserData();
        if (rildata != NULL) {
            radioState = rildata->GetInt(0);
        }
        else {
            radioState = m_pRilContext->GetCurrentRadioState();
            if (GetRilContextProperty()->GetInt(RIL_CONTEXT_UICC_STATUS, -1) == 0) {
                radioState = RADIO_STATE_OFF;
            }
        }
        SetRadioState(radioState, true);

        if ((m_nOverallRadioState == RADIO_STATE_OFF) && (m_pCurReqMsg != NULL)) {
            RequestData *pRequest = m_pCurReqMsg->GetRequestData();
            if (!IsPossibleToPassInRadioOffState(pRequest->GetReqId())) {
                RilLogV("[%s_%d] Pending Request Complete in RADIO_STATE_OFF. req=%d", m_szSvcName, m_pRilContext->GetRilSocketId(), pRequest->GetReqId());
                OnRequestComplete(RIL_E_RADIO_NOT_AVAILABLE);
            }
        } else if ((m_nOverallRadioState == RADIO_STATE_UNAVAILABLE) && (m_pCurReqMsg != NULL)) {
            RequestData *pRequest = m_pCurReqMsg->GetRequestData();
            if (!IsPossibleToPassInRadioUnavailableState(pRequest->GetReqId())) {
                RilLogV("[%s_%d] Pending Request Complete in RADIO_STATE_UNAVAILABLE. req=%d", m_szSvcName, m_pRilContext->GetRilSocketId(), pRequest->GetReqId());
                OnRequestComplete(RIL_E_RADIO_NOT_AVAILABLE);
            }
        }
        OnRadioStateChanged(m_nOverallRadioState);
        break;
    }
    case MSG_SYSTEM_SIM_STATUS_CHANGED:
    {
        if (VDBG)
            RilLogV("[%s] OnSimStatusChanged()", m_szSvcName);
        RilDataInts *rildata = (RilDataInts *)msg->GetUserData();
        if (rildata != NULL) {
            OnSimStatusChanged(rildata->GetInt(0), rildata->GetInt(1));
        }
        break;
    }
    case MSG_SYSTEM_IMSI_UPDATED:
    {
        // MSG_SYSTEM_IMSI_UPDATED is only broadcast when reading IMSI of (U)SIM subscription is done.
        // No IMSI erase event when howswap is occurred. (query from SIM card only)
        if (VDBG)
            RilLogV("[%s] OnImsiUpdated()", m_szSvcName);
        RilDataStrings *rildata = (RilDataStrings *)msg->GetUserData();
        if (rildata != NULL) {
            const char *aid = rildata->GetString(0);
            const char *imsi = rildata->GetString(1);
            // legacy. IMSI only
            OnImsiUpdated(imsi);
            // IMSI with AID
            OnImsiUpdated(aid, imsi);
        }
        break;
    }
    case MSG_SYSTEM_RESET:
        if (VDBG)
            RilLogV("[%s] OnReset()", m_szSvcName);
        OnReset();
        break;
    case MSG_SYSTEM_VOICE_REGISTRTION_STATE_CHANGED:
    {
        if (VDBG)
            RilLogV("[%s] OnVoiceRegistrationStateChanged()", m_szSvcName);
        RilDataInts *rildata = (RilDataInts *)msg->GetUserData();
        if (rildata != NULL) {
            m_nVoiceNetworkState = rildata->GetInt(0);
            OnVoiceRegistrationStateChanged(m_nVoiceNetworkState);
        }
        break;
    }
    case MSG_SYSTEM_DATA_REGISTRTION_STATE_CHANGED:
    {
        if (VDBG)
            RilLogV("[%s] OnDataRegistrationStateChanged()", m_szSvcName);
        RilDataInts *rildata = (RilDataInts *)msg->GetUserData();
        if (rildata != NULL) {
            m_nDataNetworkState = rildata->GetInt(0);
            OnDataRegistrationStateChanged(m_nDataNetworkState);
        }
        break;
    }
    case MSG_SYSTEM_DATA_CALL_STATE_CHANGED:
    {
        if (VDBG)
            RilLogV("[%s] OnDataCallStateChanged()", m_szSvcName);
        RilDataInts *rildata = (RilDataInts *)msg->GetUserData();
        if (rildata != NULL) {
            int nCid = rildata->GetInt(0);
            bool bActive = rildata->GetInt(1)? true: false;
            OnDataCallStateChanged(nCid, bActive);
        }
        break;
    }
    case MSG_SYSTEM_SERVICE_TATE_CHANGED: {
        if (VDBG)
            RilLogV("[%s] OnServiceStateChanged()", GetServiceName());
        if (GetRilContext() != NULL) {
            ServiceState state = GetRilContext()->GetServiceState();
            OnServiceStateChanged(state);
        }
        break;
    }
    case MSG_SYSTEM_MODEM_STATE_CHANGED: {
        if (VDBG)
            RilLogV("[%s] OnModemStateChanged()", m_szSvcName);
        RilDataInts *rildata = (RilDataInts *) msg->GetUserData();
        if (rildata != NULL) {
            int state = rildata->GetInt(0);
            int oldModemState = mModemState;
            mModemState = state;
            if (oldModemState != state) {
                OnModemStateChanged(state);

                if (state == MS_ONLINE) {
                    OnModemOnline();
                }

                if (state != MS_ONLINE) {
                    OnModemOffline();
                }
            }
        }
        break;
    }
    default:
        OnHandleInternalMessage(msg);
        break;
    } // end switch ~

    if (msg) {
        delete msg;
    }

    return 0;
}

void Service::SetRadioState(int radioState, bool forceNotify) {
    int oldRadioState = m_nOverallRadioState;
    m_nOverallRadioState = radioState;

    if (radioState == RADIO_STATE_UNAVAILABLE) {
        SetRadioStateUnavailable();
    }

    if (oldRadioState != radioState && forceNotify) {
        OnRadioStateChanged(radioState);
    }

    if (radioState != RADIO_STATE_UNAVAILABLE &&
            oldRadioState == RADIO_STATE_UNAVAILABLE) {
        OnRadioAvailable();
    }

    if (radioState == RADIO_STATE_UNAVAILABLE &&
            oldRadioState != RADIO_STATE_UNAVAILABLE) {
        OnRadioNotAvailable();
    }

    if (radioState == RADIO_STATE_ON &&
            oldRadioState != RADIO_STATE_ON) {
        OnRadioOn();
    }

    if ((radioState == RADIO_STATE_OFF || radioState == RADIO_STATE_ON) &&
            oldRadioState == RADIO_STATE_UNAVAILABLE) {
        OnRadioOffOrNotAvailable();
    }
}

void Service::SetRadioStateUnavailable() {
    m_nVoiceNetworkState = NOT_REGISTERED;
    m_nDataNetworkState = NOT_REGISTERED;
    m_nOverallRadioState = RADIO_STATE_UNAVAILABLE;
}

bool Service::IsVoiceNetworkAvailable() const
{
    return (m_nVoiceNetworkState == REGISTERED_HOME ||
            m_nVoiceNetworkState == REGISTERED_ROAMING);
}

bool Service::IsDataNetworkAvailable() const
{
    return (m_nDataNetworkState == REGISTERED_HOME ||
            m_nDataNetworkState == REGISTERED_ROAMING);
}

bool Service::IsRadioNotAvailable() const
{
    return (m_nOverallRadioState == RADIO_STATE_UNAVAILABLE);
}

bool Service::IsRadioAvailable() const
{
    return (m_nOverallRadioState != RADIO_STATE_UNAVAILABLE);
}

bool Service::IsRadioOn() const
{
    return (m_nOverallRadioState == RADIO_STATE_ON);
}

bool Service::IsRadioOffOrNotAvailable() const
{
    return (m_nOverallRadioState != RADIO_STATE_ON);
}

// System Broadcast Message Handler
void Service::OnRadioStateChanged(int radioState) {}
void Service::OnRadioNotAvailable() {}
void Service::OnRadioOffOrNotAvailable() {}
void Service::OnRadioAvailable() {}
void Service::OnRadioOn() {}
void Service::OnSimStatusChanged(int cardState, int appState) {}
void Service::OnImsiUpdated(const char *imsi) {}
void Service::OnImsiUpdated(const char *aid, const char *imsi) {}
void Service::OnReset() {}
void Service::OnVoiceRegistrationStateChanged(int regState) {}
void Service::OnDataRegistrationStateChanged(int regState) {}
void Service::OnDataCallStateChanged(int nCid, bool bActive) {}
void Service::OnServiceStateChanged(const ServiceState& state) {}
void Service::OnModemStateChanged(int state) {}
void Service::OnModemOnline() {}
void Service::OnModemOffline() {}

RilContext *Service::GetRilContext()
{
    if (m_pRilContext == NULL) {
        RilLogW("%s Warning no valid RilContext instance!!!", __FUNCTION__);
    }
    return m_pRilContext;
}

RIL_SOCKET_ID Service::GetRilSocketId()
{
    RilContext *pRilContext = GetRilContext();
    if (pRilContext != NULL) {
        return pRilContext->GetRilSocketId();
    }
    return RIL_SOCKET_1;
}

RilProperty *Service::GetRilContextProperty()
{
    RilContext *pRilContext = GetRilContext();
    if (pRilContext != NULL) {
        return pRilContext->GetProperty();
    }
    return NULL;
}

RilProperty *Service::GetRilApplicationProperty()
{
    RilContext *pRilContext = GetRilContext();
    if (pRilContext != NULL) {
        return pRilContext->GetApplicationProperty();
    }
    return NULL;
}

///TODO: consider not DSDS case - more than 2 ril context
Service* Service::GetOppositeService(int nServiceId)
{
    //RilLogI("[%s] %s() Find service id (%d)", m_szSvcName, __FUNCTION__, nServiceId);

    RilContext *pRilContext = GetRilContext();
    if ( pRilContext != NULL )
    {
        RilContext *pTargetRilContext = pRilContext->GetOppositeRilContext();
        if ( pTargetRilContext != NULL)
        {
            RilLogV("%s Get Service(%d) in RilContext(%d)", __FUNCTION__, nServiceId, (UINT)(pTargetRilContext->GetRilSocketId()));
            return pTargetRilContext->GetService(nServiceId);
        }
    }

    RilLogV("[%s] %s() : Failed to find opposite service(%d)", m_szSvcName, __FUNCTION__, nServiceId);
    return NULL;
}

UINT Service::IsOppsiteStackOccupyRF()
{
    //RilLogV("[%s] %s()", m_szSvcName, __FUNCTION__);

    NetworkService* pOppNetworkService = (NetworkService*)GetOppositeService(RIL_SERVICE_NETWORK);
    if ( pOppNetworkService != NULL )
    {
        if ( pOppNetworkService->IsPlmnSearching() == TRUE )
        {
            RilLogV("[%s] %s() : Oppsite RIL context is in PLMN Searching", m_szSvcName, __FUNCTION__);
            return OCCUPY_RF_STATUS_PLMN_SEARCH;
        }
    }

    CscService* pOppCsService = (CscService*)GetOppositeService(RIL_SERVICE_CSC);
    if ( pOppCsService != NULL )
    {
        if ( pOppCsService->IsInCallState() == TRUE )
        {
            RilLogV("[%s] %s() : Oppsite RIL context is in Call state", m_szSvcName, __FUNCTION__);
            return OCCUPY_RF_STATUS_DIAL;
        }
    }
    return 0;
}

Service* Service::GetCurrentService(int nServiceId)
{
    //RilLogI("[%s] %s() Find service id (%d)", m_szSvcName, __FUNCTION__, nServiceId);

    RilContext *pRilContext = GetRilContext();
    if ( pRilContext != NULL )
    {
        RilLogV("%s Get Service(%d) in RilContext(%d)", __FUNCTION__, nServiceId, (UINT)GetRilSocketId());
        return pRilContext->GetService(nServiceId);
    }

    RilLogE("[%s] %s() : Failed to find current service(%d)", m_szSvcName, __FUNCTION__, nServiceId);
    return NULL;
}

UINT Service::IsCurrentStackOccupyRF()
{
    //RilLogV("[%s] %s()", m_szSvcName, __FUNCTION__);

    NetworkService* pMyNetworkService = (NetworkService*)GetCurrentService(RIL_SERVICE_NETWORK);
    if ( pMyNetworkService != NULL )
    {
        if ( pMyNetworkService->IsPlmnSearching() == TRUE )
        {
            RilLogV("[%s] %s() : Current RIL context is in PLMN Searching", m_szSvcName, __FUNCTION__);
            return OCCUPY_RF_STATUS_PLMN_SEARCH;
        }
    }

    CscService* pMyCsService = (CscService*)GetCurrentService(RIL_SERVICE_CSC);
    if ( pMyCsService != NULL )
    {
        if ( pMyCsService->IsInCallState() == TRUE )
        {
            RilLogV("[%s] %s() : Oppsite RIL context is in Call state", m_szSvcName, __FUNCTION__);
            return OCCUPY_RF_STATUS_DIAL;
        }
    }
    return 0;
}

unsigned int Service::GetOpenCarrierIndex()
{
    RilContext *pRilContext = GetRilContext();
    if ( pRilContext != NULL )
    {
        return pRilContext->GetOpenCarrierIndex();
    }

    RilLogE("[%s] %s() : Failed to get open carrier index", m_szSvcName, __FUNCTION__);
    return (unsigned int)OC_UNKNOWN;
}

void Service::SetOpenCarrierIndex(const char* mccmnc)
{
    RilContext *pRilContext = GetRilContext();
    if ( pRilContext != NULL )
    {
        return pRilContext->SetOpenCarrierIndex(mccmnc);
    }

    RilLogE("[%s] %s() : Failed to set open carrier index", m_szSvcName, __FUNCTION__);
}

String Service::GetSimOperatorNumeric()
{
    const int phoneId = GetRilSocketId();
    string numeric = GetSimOperatorNum(phoneId);
    return numeric;
}

void Service::OnRequestInternal(int request, void *data/* = NULL*/, unsigned int datalen/* = 0*/)
{
    RilLogV("[%d]%s(id=%d data=%p length=%d", GetRilSocketId(), __FUNCTION__, request, data, datalen);
    if (request > 0) {
        RilAppToken *tok = RilAppToken::NewInstance(RilResponseInternalListener::NewInstance(this, request));
        GetRilContext()->OnRequest(request, data, datalen, (RIL_Token)tok);
    }
}

void Service::OnRequestInternal(int request, void *data, unsigned int datalen, RIL_Token t)
{
    RilLogV("[%d]%s(id=%d data=%p length=%d)", GetRilSocketId(), __FUNCTION__, request, data, datalen);
    if (request > 0) {
        RilAppToken *tok = RilAppToken::NewInstance(RilResponseInternalListener::NewInstance(this, request), t);
        GetRilContext()->OnRequest(request, data, datalen, (RIL_Token)tok);
    }

}

void Service::OnRequestInternalComplete(RIL_Token t, int id, int result, void *data/* = NULL*/, int datalen/* = 0*/)
{
    // TODO : override
}

void Service::AsycMsgReqClearHistory()
{
    UINT size, index, token;

    if (m_pAsyncMsgHistory == NULL) return;

    size = m_pAsyncMsgHistory->GetSize();
    for(index = 0; index < size; index++) {
        token = m_pAsyncMsgHistory->GetBegin();
        Message *pReqMsg = m_pAsyncMsgHistory->Find(token);
        if (pReqMsg != NULL) {
            RequestData *pRequest = pReqMsg->GetRequestData();
            if(m_pRilContext != NULL) {
                m_pRilContext->OnRequestTimeout(pRequest, token);
            }
        }
        AsyncMsgReqDeleteTimer(token, ASYNC_MSG_TIMER_DELETE_REASON_OTHERS);
        m_pAsyncMsgHistory->Remove(token);
        if(pReqMsg != NULL) delete pReqMsg;
    }
}

void Service::AsycMsgReqClear()
{
    AsycMsgReqClearHistory();

    // For double checking
    if (m_pAsyncMsgHistory != NULL)
    {
        m_pAsyncMsgHistory->Clear();
        delete m_pAsyncMsgHistory;
        m_pAsyncMsgHistory = NULL;
    }

    m_asyncMsgReqTimerMap.clear();
}

void Service::AsyncMsgReqPreProcessing(Message *pMsg)
{
    if(NULL == pMsg) {
        RilLogE("PreProcessing for priority fail: message is NULL");
        return;
    }

    //Enqueue message due to different direction
    switch (pMsg->GetDirection()) {
        case REQUEST:
        {
            break;
        }
        case RESPONSE:
        {
            int rspMsgId = -1;
            UINT token = pMsg->GetToken();
            rspMsgId = pMsg->GetMsgId();
            ModemData *pModemData = pMsg->GetModemData();

            if(token != TOKEN_INVALID && m_pAsyncMsgHistory != NULL && pModemData != NULL && pModemData->IsSolicitedResponse()) {
                Message *pReqMsg = m_pAsyncMsgHistory->Find(token);
                if(pReqMsg != NULL && pReqMsg->GetAsyncMsgReqStatus() == ASYNC_MSG_STATUS_SENT2CP) {
                    pMsg->SetAsyncMsgReqStatus(ASYNC_MSG_STATUS_RESPONSE);
                    AsyncMsgReqDeleteTimer(token, ASYNC_MSG_TIMER_DELETE_REASON_NORMAL);
                    RilLogV("%s::%s():RESPONSE token(0x%x), RSP msg id(%d)",
                            m_szSvcName, __FUNCTION__, token, rspMsgId);
                }
            }
            break;
        }
        case INTERNAL:
        {
            break;
        }
        case ASYNC_REQUEST:
        {
            break;
        }
        default:
        {
            //these value shouldn't be set, report error
            RilLogE("[%s] %s(): message direction error", m_szSvcName, __FUNCTION__);
        }
    } // end switch ~
}

void Service::AsyncMsgReqInsertHistory(UINT token, Message * pMsg)
{
    if (m_pAsyncMsgHistory != NULL) m_pAsyncMsgHistory->Push(token, pMsg);
}

void Service::AsyncMsgReqStartTimer(UINT token, Message * pMsg)
{
    timer_t timerId;
    int relativeTimeOut;

    if (pMsg != NULL && token != TOKEN_INVALID) {
        relativeTimeOut = pMsg->GetTimeout();
        timerId = StartTimer(token, relativeTimeOut, Service::AsyncMsgReqTimerHandlerWrapper);
        RilLogV("%s::%s() : token(0x%x), timerID(0x%x), timeout(%d), total(%d)",
            m_szSvcName, __FUNCTION__, token, timerId, relativeTimeOut, m_asyncMsgReqTimerMap.size());
    }
}

int Service::HandleAsycMsgRequest()
{
    Message *pMsg = asycMsgReqDeQ();
    if (pMsg == NULL) {
        //RilLogW("asycMsgReqDeQ() : NULL");
        return -1;
    }

    RequestData *pReqData = pMsg->GetRequestData();
    if (pReqData == NULL) {
        RilLogE("%s::%s() : No RIL Request Data, msg(%p)", m_szSvcName, __FUNCTION__, pMsg);
        delete pMsg;
        return -1;
    }

    if (m_nOverallRadioState == RADIO_STATE_OFF)
    {
        if ( false == IsPossibleToPassInRadioOffState(pReqData->GetReqId()) )
        {
            RilLogV("[%s_%d] %s Current Radio State is RADIO_STATE_OFF : RIL Request ID=%d",
                m_szSvcName, m_pRilContext->GetRilSocketId(), __FUNCTION__, pReqData->GetReqId());
            if(m_pRilContext != NULL) m_pRilContext->OnRequestComplete(pReqData, RIL_E_RADIO_NOT_AVAILABLE);
            delete pMsg;
            //NotifyNewMessage(ASYNC_REQUEST);
            return 0;
        }
    }

    if(m_nOverallRadioState == RADIO_STATE_UNAVAILABLE)
    {
        if ( false == IsPossibleToPassInRadioUnavailableState(pReqData->GetReqId()) )
        {
            RilLogV("[%s_%d] %s Current Radio State is RADIO_STATE_UNAVAILABLE : RIL Request ID=%d",
                m_szSvcName, m_pRilContext->GetRilSocketId(),__FUNCTION__, pReqData->GetReqId());
            if(m_pRilContext != NULL) m_pRilContext->OnRequestComplete(pReqData, RIL_E_RADIO_NOT_AVAILABLE);
            delete pMsg;
            //NotifyNewMessage(ASYNC_REQUEST);
            return 0;
        }
    }

    RilLogV("Enter %s::%s()", m_szSvcName, __FUNCTION__);
    pMsg->SetAsyncMsgReqStatus(ASYNC_MSG_STATUS_PROCESSING);
    m_nAsycMsgReqToken = TOKEN_INVALID;
    BOOL ret = OnHandleRequest(pMsg);
    RilLogV("Exit %s::%s() ret=%s, RIL Request ID=%d", m_szSvcName, __FUNCTION__, ret ? "TRUE" : "FALSE", pReqData->GetReqId());
    if (!ret) {
        RilLogE("%s::%s() OnRequestComplete(RIL_E_GENERIC_FAILURE) to the RILD", m_szSvcName, __FUNCTION__);
        if(m_pRilContext != NULL) m_pRilContext->OnRequestComplete(pReqData, RIL_E_GENERIC_FAILURE);
        delete pMsg;
        m_nAsycMsgReqToken = TOKEN_INVALID;
        //NotifyNewMessage(ASYNC_REQUEST);
        return 0;
    }

    // if msg is not Null and m_nAsycMsgReqToken has vaild value, IPC is sent to CP in OnHandleRequest()
    // there is case which msg is deleted while processing OnHandleRequest().
    if(pMsg != NULL) {
        UINT priorityReqToken = pMsg->GetToken();

        // (m_pRilContext != NULL && m_pRilContext->IsInRequestWaitHistory(priorityReqToken) == TRUE) can be considered
        if(priorityReqToken != TOKEN_INVALID && m_nAsycMsgReqToken == priorityReqToken){
            pMsg->SetAsyncMsgReqStatus(ASYNC_MSG_STATUS_SENT2CP);
            pMsg->SetAsyncMsgReqStartTime();
            AsyncMsgReqInsertHistory(priorityReqToken, pMsg);

            // After getting earliest expiration time in priority request list, then update timer.
            AsyncMsgReqStartTimer(priorityReqToken, pMsg);
        } else {
            RilLogV("%s::%s() : token is different (0x%x != 0x%x)", m_szSvcName, __FUNCTION__, m_nAsycMsgReqToken, priorityReqToken);
        }
    }
    m_nAsycMsgReqToken = TOKEN_INVALID;

    return 0;
}

void Service::AsyncMsgReqDeleteTimer(UINT token, int reason)
{
    timer_t t_id = (timer_t)-1;
    bool isTimerDeleteError = FALSE;

    m_pMutex->lock();
    map<UINT, timer_t>::iterator iter = m_asyncMsgReqTimerMap.find(token);
    if (iter != m_asyncMsgReqTimerMap.end()) {
        t_id = m_asyncMsgReqTimerMap.find(token)->second;

        if(timer_delete(t_id) == -1){
            isTimerDeleteError = TRUE;
        }
        m_asyncMsgReqTimerMap.erase(token);

    }
    m_pMutex->unlock();
    RilLogV("[%s] %s() : token(0x%x), isTimerDeleteError(%d) reason(%d)",
            m_szSvcName, __FUNCTION__, token, isTimerDeleteError, reason);
}

void Service::AsyncMsgReqTimerCallback(int sigNo, siginfo_t *info, UINT token)
{
    AsyncMsgReqDeleteTimer(token, ASYNC_MSG_TIMER_DELETE_REASON_TIMEOUT);

    if (m_pAsyncMsgHistory == NULL) return;

    Message *pReqMsg = m_pAsyncMsgHistory->Find(token);
    if (pReqMsg != NULL) {
        RequestData *req = pReqMsg->GetRequestData();
        if (req != NULL) {
            // if requested message was already handled in OnHandleRequestTimeout method, MUST be returned true.
            // if it is true, pReqMsg is deleted and  m_pAsyncMsgHistory->Remove is called in OnHandleRequestTimeout.
            if (OnHandleRequestTimeout(pReqMsg)) {
                return;
            }

            CheckCountsForTimeOut(pReqMsg->GetModemData());
            m_pRilContext->OnRequestTimeout(req, token);
        } else {
            RilLogE("[%s] %s() : No REQ data for token(0x%x) in the async msg req list", m_szSvcName, __FUNCTION__, token);
        }

        m_pAsyncMsgHistory->Remove(token);
        delete pReqMsg;
    }
    else {
        RilLogE("[%s] %s() : No MSG for token(0x%x) in the async msg req list", m_szSvcName, __FUNCTION__, token);
    }
}

void Service::AsyncMsgReqTimerHandlerWrapper(int sigNo, siginfo_t *info, void *uc)
{
    if(info->si_code != SI_TIMER && sigNo != SIGTIMER){
        RilLog("non SI_TIMER signal is received");
        return;
    }

    sigval_t val = info->si_value;
    async_msg_timer_param *object = (async_msg_timer_param *) val.sival_ptr;

    ((Service *) object->callback_handler)->AsyncMsgReqTimerCallback(sigNo, info, object->token);
    delete object;
}

timer_t Service::StartTimer(UINT token, int timeout, void (pAction)(int , siginfo_t *, void *))
{
    struct sigaction sigAct;
    struct sigevent sigEvt;
    struct itimerspec val;
    timer_t temp_t_id;

    sigemptyset(&sigAct.sa_mask);
    //sigaddset(&sigAct.sa_mask, SIGTIMER)
    //sigfillset(&sigAct.sa_mask);
    sigAct.sa_flags = SA_SIGINFO;
    sigAct.sa_sigaction = pAction;
    if (sigaction(SIGTIMER, &sigAct, 0) == -1) {
        RilLogE("sigaciton error");
        return (timer_t)-1;
    }

    async_msg_timer_param * t_param = new(async_msg_timer_param);
    t_param->callback_handler = (void *) this;
    t_param->token = token;

    sigEvt.sigev_notify = SIGEV_SIGNAL;
    sigEvt.sigev_signo = SIGTIMER;
    sigEvt.sigev_value.sival_ptr = (void *) t_param;
    sigEvt.sigev_notify_thread_id = gettid();

    if(timer_create(CLOCK_REALTIME, &sigEvt, &temp_t_id) == 0){
        //RilLogV("timer is created with %x", temp_t_id);

        //The unit for timeout value is ms
        val.it_value.tv_sec = (timeout/1000);
        val.it_value.tv_nsec = (long)(timeout % 1000) * (1000000L);
        val.it_interval.tv_sec = 0;
        val.it_interval.tv_nsec = 0;

        if(timer_settime(temp_t_id, 0, &val, NULL) != 0){
            RilLogE("%s() timer_settimer() error", __FUNCTION__);
            if(timer_delete(temp_t_id) == -1){
                RilLogE("%s() timer_delete() error", __FUNCTION__);
            }
            delete t_param;
            return (timer_t)-1;
        }
        m_pMutex->lock();
        m_asyncMsgReqTimerMap.insert( map<UINT, timer_t>::value_type(token, temp_t_id));
        m_pMutex->unlock();
    }
    else{
        RilLogE("timer_create() error");
        delete t_param;
        return (timer_t)-1;
    }

    return temp_t_id;

}

BOOL Service::IsInTransaction(UINT nReqId)
{
    BOOL ret = FALSE;

    // Check Normal Msg
    ret = (m_nCurrentReqId != 0 && m_nCurrentReqId == nReqId);

    // Check Async Msg History
    if(ret == FALSE && m_pAsyncMsgHistory != NULL && m_pAsyncMsgHistory->GetSize() != 0) {
        ret = m_pAsyncMsgHistory->IsReqInQueue(nReqId);
    }

    return ret;
}


/////////////////////////////////////////////////////////////////////////////////////////
CAsyncMsgReqHistory::CAsyncMsgReqHistory()
{
}

CAsyncMsgReqHistory::~CAsyncMsgReqHistory()
{
    Clear();
}

UINT CAsyncMsgReqHistory::GetSize()
{
    UINT ret;
    Lock();
    ret = (UINT)m_asyncMsgReqMap.size();
    Unlock();
    return ret;
}

bool CAsyncMsgReqHistory::Push(UINT token, Message *pMsg)
{
    UINT mapSize = 0;
    bool result = FALSE;
    pair<map<UINT, Message *>::iterator,bool> ret;

    if (token != TOKEN_INVALID && pMsg != NULL) {
        Lock();
        ret = m_asyncMsgReqMap.insert(pair<UINT, Message *>(token, pMsg));
        mapSize = (UINT)m_asyncMsgReqMap.size();
        Unlock();

        if (ret.second == TRUE) {
            RilLogV("CAsyncMsgReqHistory::%s() element 0x%x is pushed. size = %d", __FUNCTION__, token, mapSize);
            result = true;
        }
        else {
            RilLogE("CAsyncMsgReqHistory::%s() element 0x%x is already exist. size = %d", __FUNCTION__, token, mapSize);
        }
    }
    else {
        RilLogE("CAsyncMsgReqHistory::%s() token 0x%x is invalid or msg is null", __FUNCTION__, token);
    }

    return result;
}

UINT CAsyncMsgReqHistory::GetBegin()
{
    UINT token = TOKEN_INVALID;

    Lock();
    if ( m_asyncMsgReqMap.size() > 0 ) {
        map<UINT, Message *>::iterator iter = m_asyncMsgReqMap.begin();
        token = iter->first;
    }
    Unlock();
    return token;
}

void CAsyncMsgReqHistory::Remove(UINT token)
{
    Lock();
    if ( m_asyncMsgReqMap.size() > 0 && token != TOKEN_INVALID) {
        m_asyncMsgReqMap.erase(token);
    }
    Unlock();
}

Message *CAsyncMsgReqHistory::Find(UINT token)
{
    Message *pResultMsg = NULL;
    Lock();
    if ( m_asyncMsgReqMap.size() > 0 && token != TOKEN_INVALID ) {

        map<UINT, Message *>::iterator iter = m_asyncMsgReqMap.find(token);
        if (iter != m_asyncMsgReqMap.end()) {
             pResultMsg = iter->second;
        }
    }
    Unlock();

    return pResultMsg;
}

bool CAsyncMsgReqHistory::IsReqInQueue(UINT nReqId)
{
    bool ret = false;
    Message *pMessage = NULL;
    Lock();
    if ( m_asyncMsgReqMap.size() > 0 ) {
        RequestData *pReqData;
        for ( map<UINT, Message *>::iterator iter = m_asyncMsgReqMap.begin(); iter != m_asyncMsgReqMap.end(); ++iter) {
            pMessage = iter->second;
            if (pMessage != NULL) {
                pReqData = pMessage->GetRequestData();
                if(pReqData != NULL && (UINT)pReqData->GetReqId() == nReqId) {
                    ret = true;
                    break;
                }
            }
        }
    }
    Unlock();

    return ret;
}

Message *CAsyncMsgReqHistory::GetReqMsgForEarlistTimeOut(struct timeval *pTv, UINT *pToken)
{
    Message *pResultMsg = NULL;
    UINT earlistToken = TOKEN_INVALID;

    int64_t relativeTimeOut;
    struct timeval tv;
    struct timeval earlistTimeout;

    memset(&tv, 0, sizeof(tv));
    memset(&earlistTimeout, 0, sizeof(earlistTimeout));

    Lock();
    if ( m_asyncMsgReqMap.size() > 0 ) {
        for ( map<UINT, Message *>::iterator iter = m_asyncMsgReqMap.begin(); iter != m_asyncMsgReqMap.end(); ++iter) {
            Message *pMessage = iter->second;
            if (pMessage != NULL && pMessage->GetAsyncMsgReqStatus() == ASYNC_MSG_STATUS_SENT2CP ) {
                relativeTimeOut = pMessage->GetTimeout();
                tv = pMessage->GetAsyncMsgReqStartTime();
                tv.tv_sec = tv.tv_sec + relativeTimeOut /1000;
                tv.tv_usec = tv.tv_usec + ((relativeTimeOut % 1000) * 1000);

                if(pResultMsg == NULL) {
                    pResultMsg = pMessage;
                    earlistTimeout = tv;
                    earlistToken = iter->first;
                }
                else {
                    if(earlistTimeout.tv_sec > tv.tv_sec || (earlistTimeout.tv_sec == tv.tv_sec && earlistTimeout.tv_usec > tv.tv_usec)){
                        pResultMsg = pMessage;
                        earlistTimeout = tv;
                        earlistToken = iter->first;
                    }
                }
            }
        }
    }
    Unlock();

    if(pResultMsg != NULL) {
        pTv->tv_sec = earlistTimeout.tv_sec;
        pTv->tv_usec = earlistTimeout.tv_usec;
        *pToken = earlistToken;
    }

    return pResultMsg;
}

void CAsyncMsgReqHistory::Clear()
{
    Lock();
    if ( m_asyncMsgReqMap.size() > 0 ) {
        for ( map<UINT, Message *>::iterator iter = m_asyncMsgReqMap.begin(); iter != m_asyncMsgReqMap.end(); ++iter) {
            Message *pReqMsg = iter->second;
            if (pReqMsg != NULL) {
                delete pReqMsg;
            }
        }
    }

    m_asyncMsgReqMap.clear();
    Unlock();
}
