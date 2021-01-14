/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "message.h"
#include "modemstatemonitor.h"
#include "mutex.h"
#include "requestdata.h"
#include "rilcontext.h"
#include "rilparser.h"
#include "rilproperty.h"
#include "servicemgr.h"
#include "servicemonitorrunnable.h"
#include "timerlist.h"

#include <queue>
#include <map>

using namespace std;

#define    __WR_USING_STL__

typedef enum {
    RIL_SERVICE_UNKNOWN,
    RIL_SERVICE_CSC,           // 1
    RIL_SERVICE_PS,            // 2
    RIL_SERVICE_SIM,           // 3
    RIL_SERVICE_MISC,          // 4
    RIL_SERVICE_NETWORK,       // 5
    RIL_SERVICE_SMS,           // 6
    RIL_SERVICE_AUDIO,         // 7
    RIL_SERVICE_IMS,           // 8
    RIL_SERVICE_GPS,           // 9
    RIL_SERVICE_WLAN,          // 10
    RIL_SERVICE_VSIM,          // 11
    RIL_SERVICE_STK,           // 12
    RIL_SERVICE_TEST,          // 13
    RIL_SERVICE_SUPPLEMENTARY, // 14
    RIL_SERVICE_EMBMS,         // 15
    RIL_SERVICE_UNSUPPORT,
    RIL_SERVICE_MAX,
}RIL_SERVICE_TYPE;

#define    TIMEOUT_NO_WAIT         0
#define    TIMEOUT_INFINITE        0xFFFFFFFF
#define    ASYNC_MSG_TIMER_DELETE_REASON_NORMAL    (0)
#define    ASYNC_MSG_TIMER_DELETE_REASON_TIMEOUT   (1)
#define    ASYNC_MSG_TIMER_DELETE_REASON_OTHERS    (2)
#define    OCCUPY_RF_STATUS_PLMN_SEARCH    (1)
#define    OCCUPY_RF_STATUS_DIAL           (2)

class Thread;
class ServiceState;

class CAsyncMsgReqHistory
{
private:
    CMutex m_lock;
    map<UINT, Message *> m_asyncMsgReqMap;

public:
    CAsyncMsgReqHistory();
    virtual ~CAsyncMsgReqHistory();

public:
    UINT GetSize();
    bool Push(UINT token, Message *pMsg);
    UINT GetBegin();
    void Remove(UINT token);
    Message *Find(UINT token);
    bool IsReqInQueue(UINT nReqId);
    Message *GetReqMsgForEarlistTimeOut(struct timeval *pTv, UINT *pToken);
    void Clear();

protected:
    inline void Lock() { m_lock.lock(); }
    inline void Unlock() { m_lock.unlock(); }
};

class Service
{
    DECLARE_MODULE_TAG()

public:
    enum { UNAVAILABLE, CREATED, STARTED, DESTROYED };

    typedef struct _async_msg_timer_param
    {
        void *callback_handler;
        UINT token;
    } async_msg_timer_param;

protected:
    UINT m_nServiceId;
    UINT m_nServiceState;
    String m_strServiceName;
    UINT m_nCurrentReqId;
    void SetTransaction(UINT nReqId) { m_nCurrentReqId = nReqId; }
    void UnsetTransaction() { m_nCurrentReqId = 0; }
    BOOL IsInTransaction(UINT nReqId);

private:
    int m_nVoiceNetworkState;
    int m_nDataNetworkState;
    int m_nOverallRadioState;
    int mModemState;
    ServiceMonitorRunnable *m_pServiceMonitorRunnable;
    friend class ServiceMonitorRunnable;
    friend class ServiceMgr;

    /**
     * constructor
     */
public:
    Service(RilContext* pRilContext, UINT nServiceId = RIL_SERVICE_UNKNOWN);
    virtual ~Service();

protected:
    int Init();
    int Start();
    int Finalize();

    int EnQueue(Message *pMsg);
    int NotifyNewMessage(Message *pMsg);
    int NotifyNewMessage(MsgDirection direction);

    int HandleRequest();
    int HandleResponse();
    int HandleInternalMessage();
    int HandleRequestTimeout();
    int HandleAsycMsgRequest();

protected:
    virtual int OnCreate(RilContext *pRilContext);
    virtual void OnStart();
    virtual void OnDestroy();

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleInternalMessage(Message* pMsg);
    virtual BOOL OnHandleRequestTimeout(Message* pMsg);
    int OnRequestComplete(int result, void *data = NULL, int length = 0, Message *pMsg = NULL);
    int OnUnsolicitedResponse(int id, const void *data = NULL, int length = 0);
    int OnRequestAck();
    int SendRequest(ModemData *pModemData, UINT timeout);
    int SendRequest(ModemData *pModemData, UINT timeout, UINT nResult, Message *pMsg = NULL);
    int SendRequest(ModemData *pModemData);

    virtual void OnSimStatusChanged(int cardState, int appState);
    virtual void OnImsiUpdated(const char *imsi);
    virtual void OnImsiUpdated(const char *aid, const char *imsi);
    virtual void OnReset();
    virtual void OnVoiceRegistrationStateChanged(int regState);
    virtual void OnDataRegistrationStateChanged(int regState);
    virtual void OnDataCallStateChanged(int nCid, bool bActive);
    virtual void OnServiceStateChanged(const ServiceState& state);
    virtual bool IsPossibleToPassInRadioOffState(int request_id) { return false; }
    virtual bool IsPossibleToPassInRadioUnavailableState(int request_id) { return false; }

    virtual void OnModemStateChanged(int state);
    virtual void OnModemOnline();
    virtual void OnModemOffline();
    virtual void OnRadioStateChanged(int radioState);
    virtual void OnRadioNotAvailable();
    virtual void OnRadioOffOrNotAvailable();
    virtual void OnRadioAvailable();
    virtual void OnRadioOn();
    bool IsRadioNotAvailable() const;
    bool IsRadioAvailable() const;
    bool IsRadioOn() const;
    bool IsRadioOffOrNotAvailable() const;
    int GetRadioState() const { return m_nOverallRadioState; }

    int GetVoiceNetworkState() const { return m_nVoiceNetworkState; }
    int GetDataNetworkState() const { return m_nDataNetworkState; }
    bool IsVoiceNetworkAvailable() const;
    bool IsDataNetworkAvailable() const;
    int GetModemState() { return mModemState; }

    RilContext *GetRilContext();
    RIL_SOCKET_ID GetRilSocketId();
    RilProperty *GetRilContextProperty();
    RilProperty *GetRilApplicationProperty();

    Service* GetOppositeService(int nServiceId);
    UINT IsOppsiteStackOccupyRF();
    Service* GetCurrentService(int nServiceId);
    UINT IsCurrentStackOccupyRF();

    unsigned int GetOpenCarrierIndex();
    void SetOpenCarrierIndex(const char* mccmnc);
    String GetSimOperatorNumeric();

    void CheckCountsForTimeOut(ModemData *pModemData);
    timer_t StartTimer(UINT token, int timeout, void (pAction)(int , siginfo_t *, void *));

    // Async Msg Req Interface
    void AsyncMsgReqPreProcessing(Message *pMsg);
    void AsyncMsgReqInsertHistory(UINT token, Message * pMsg);
    void AsyncMsgReqStartTimer(UINT token, Message * pMsg);
    void AsyncMsgReqDeleteTimer(UINT token, int reason);
    void AsyncMsgReqTimerCallback(int sigNo, siginfo_t *info, UINT token);

    void AsycMsgReqClearHistory();
    void AsycMsgReqClear();

private:
    void SetRadioState(int radioState, bool forceNotify);
    void SetRadioStateUnavailable();

public:
    int OpenMessagePipe();
    int GetReadPipe() { return m_nMsgPipeR; }
    int StartRequestTimeout();
    int SetRequestTimeout(UINT timeout);
    int SetRequestTimeout(Message *msg, UINT timeout);
    int ResetRequestTimeout(UINT timeout);
    int CalculateTimeout(struct timeval *pTv);
    int CalcNextTimeout(struct timeval *tv);
    Message* reqDeQ();
    Message* respDeQ();
    Message* internalDeQ();
    Message* asycMsgReqDeQ();
    Message *GetCurrentMsg() { return m_pCurReqMsg; }
    RequestData *GetCurrentReqeustData() { return m_pCurReqMsg != NULL ? m_pCurReqMsg->GetRequestData() : NULL; }
    void NotifyNextRequestMessage();
    bool PostponeRequestMessage(unsigned int millis);

    UINT GetServiceId() const { return m_nServiceId; }
    const char *GetServiceName() { return m_szSvcName; }
    UINT GetServiceState() const { return m_nServiceState; };
    void SetServiceId(UINT nServiceId);
    void SetServiceMonitorRunnable(ServiceMonitorRunnable *pServiceMonitorRunnable);

    void OnRequestInternal(int request, void *data = NULL, unsigned int datalen = 0);
    void OnRequestInternal(int request, void *data, unsigned int datalen, RIL_Token t);
    virtual void OnRequestInternalComplete(RIL_Token t, int id, int result, void *data = NULL, int datalen = 0);

protected:
    Message* m_pCurReqMsg;
    RilContext* m_pRilContext;
    char m_szSvcName[32];

#ifdef HAVE_POSIX_CLOCKS
    struct timespec m_tvStart;
#else
    struct timeval m_tvStart;   //
#endif

//  private:
    CMutex *m_pMutex;

    int InitMessageQueue();
    void ClearMessageQueue();

private:
    int m_nMsgPipeR;
    int m_nMsgPipeW;
    Thread* m_pPoll;
    bool m_bIsInited;

    // 2015.06.20 : currently, no response is checked only inside of each service.
    // Need to be considered to switch to check NR regardless of service category
    static const unsigned int MAX_NR_COUNT = 3;
    unsigned int m_nr_count;

    // STD queue
    queue<Message *> m_requestQueue;
    queue<Message *> m_responseQueue;
    queue<Message *> m_internalQueue;

    // handling timeout and postpone messages
    TimerList mTimerList;

    // Async Message Req Interface
    UINT m_nAsycMsgReqToken;
    queue<Message *> m_asyncMsgReqQueue;
    CAsyncMsgReqHistory *m_pAsyncMsgHistory;
    map<UINT, timer_t> m_asyncMsgReqTimerMap;

    // static
public:
    static const char *GetServiceName(UINT nServiceId);
    static const char *GetServiceName(const Service *pService);
    static void AsyncMsgReqTimerHandlerWrapper(int sigNo, siginfo_t *info, void *uc);
};

#endif
