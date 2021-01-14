/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <sqlite3.h>
#include <arpa/inet.h>
#include "psservice.h"
#include "servicemgr.h"
#include "rillog.h"
#include "datacallreqdata.h"
#include "protocolpsbuilder.h"
#include "protocolpsadapter.h"
#include "psdatabuilder.h"
#include "rilparser.h"
#include "netifcontroller.h"
#include "mcctable.h"
#include "telephonyprovider.h"
#include "util.h"

#include "customproductfeature.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define writeRilEvent(format1, format2, ...)   CRilEventLog::writeRilEvent(RIL_LOG_CAT_PDP, format1, format2, ##__VA_ARGS__)

typedef struct _timer_param
{
    void *callback_handler;
    int cid;
} timer_param;

void PsService::RSRATimeoutCallback(int sigNo, siginfo_t *evp, int cid)
{
    time_t tim = time(0);
    //If Already Completed just ignore event.
    //If not yet Completed
    //Sending RS/RA Fail Event, so complete SETUP_DATA_CALL
    timer_t t_id;
    t_id = map_timer_id.find(cid)->second;
    RilLog("Catched RSRA Timeout t_id:%p, tid:%d, pid:%d, sigNo: %d cid:%d @ %s", t_id, gettid(), evp->si_pid, sigNo, cid, ctime(&tim));
    if(timer_delete(t_id) == -1){
        RilLog("timer_delete() error");
    }
    else
    {
        t_id = 0;
        map_timer_id.erase(cid);
    }
    if(!(cid>0 && cid<9)){
        RilLog("Incorrect CID:%d, Timeout will not be processed", cid);
        return ;
    }

    PdpContext *pPdpContext = GetPdpContext(cid);
    DataCall *dc = pPdpContext->GetDataCallInfo();
    int state = pPdpContext->GetState();

    //Still in Confiugring, if IPv4 is valid, clear IPV6
    // if IPv6 only, Assume RS/RA is failed, deact PDP
    /* ApnSetting* pApnSetting = pPdpContext->GetApnSetting(); */
    /* NULL != pApnSetting && pApnSetting->CanHandleType(APN_TYPE_DEFAULT) &&*/
    bool needToClearIpv6 = false;
    if(dc->ipv6.valid && dc->ipv6.addr[0] == 0xFE && dc->ipv6.addr[1] == 0x80){
        if(!dc->ipv4.valid){ // IPV6 Only
            RilLog("RA Timeout, no available Interface address, deact PDP");
            OnSetupDataCallComplete(RIL_E_GENERIC_FAILURE, pPdpContext);
            return;
        }
        else{
            needToClearIpv6=true;
        }
    }
    if(needToClearIpv6 && dc->ipv4.valid)
    {
        // Clean IPV6, just complete with IPV4 only
        if(state == PDP_CONTEXT_IPV6_CONFIGURING)
        {
            dc->ipv6.valid = false;
            dc->pdpType = PDP_TYPE_IPV4;
            pPdpContext->SetState(PDP_CONTEXT_CONNECTED);
            OnSetupDataCallComplete(RIL_E_SUCCESS, pPdpContext);
            return;
        }else if(state == PDP_CONTEXT_CONNECTED){
            // Deferred Case
            dc->ipv6.valid = false;
            dc->pdpType = PDP_TYPE_IPV4;
            OnNotifyDataCallList();
            return;
        }
        // Else just leave ipv6 link-local address
        RilLog("PDP state:%d. ignore RSRA timeout. wait for RIL Request Timeout", state);
    }
    else
    {
        RilLog("IPV6 is configured successfully or setup_data_call was already failed. ignore timeout");
    }
}

void PsService::TimerHandler_wrapper(int sigNo, siginfo_t *evp, void *uc)
{
    if(evp->si_code != SI_TIMER && sigNo != SIGUSR1){
        RilLog("non SI_TIMER signal is received");
        return;
    }
    sigval_t val = evp->si_value;

    timer_param *object = (timer_param *) val.sival_ptr;
    ((PsService *) object->callback_handler)->RSRATimeoutCallback(sigNo, evp, object->cid);
    delete object;
}

void PsService::SetRSRATimeoutTimer(int cid, const char *ifname)
{
    struct sigaction sigv;
    struct sigevent sigx;
    struct itimerspec val;
    timer_t temp_t_id;

    sigemptyset(&sigv.sa_mask);
    sigv.sa_flags = SA_SIGINFO;
    //sigv.sa_handler = (sighandler_t)&PsService::TimerHandler_wrapper;
    sigv.sa_sigaction = &PsService::TimerHandler_wrapper;

    if (sigaction(SIGUSR1, &sigv, 0) == -1) {
        RilLog("sigaciton error");
    }

    sigx.sigev_notify       = SIGEV_SIGNAL;
    sigx.sigev_signo        = SIGUSR1;
    //sigx.sigev_value.sival_int = gettid();
    timer_param * t_param = new(timer_param);
    sigx.sigev_value.sival_ptr = (void *) t_param;
    t_param->callback_handler = (void *) this;
    t_param->cid = cid;
    //sigx.sigev_notify_function = &PsService::TimerHandler_wrapper;
    //sigx.sigev_notify_attributes = cid;
    sigx.sigev_notify_thread_id = gettid();

    if (map_timer_id.find(cid) == map_timer_id.end()){
        if(timer_create(CLOCK_REALTIME, &sigx, &temp_t_id) == -1){
            RilLogE("timer_create() error");
        }
        else{
            //RilLogV("timer is created with %x", temp_t_id);
            map_timer_id.insert( map<int, timer_t>::value_type(cid, temp_t_id) );
        }
    }
    else{
        temp_t_id = map_timer_id.find(cid)->second;
        RilLog("Reset Timer : %p", temp_t_id);
    }

    // Read Kernel RS Parameters
    int rs_retry = NetIfController::GetIfMaxRsCount(ifname);
    int rs_interval = NetIfController::GetIfRsInterval(ifname);
    int rs_delay = NetIfController::GetIfRsDelay(ifname);
    int rs_timeout = (rs_retry+1) * rs_interval + rs_delay;

    clock_gettime(CLOCK_REALTIME, &val.it_value);
    val.it_value.tv_sec += rs_timeout;
    RilLog("RSRATimeout Timer(%x) will go off at : %s", temp_t_id, ctime(&val.it_value.tv_sec));
    val.it_value.tv_sec = rs_timeout;
    val.it_value.tv_nsec = 0;
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_nsec = 0;

    if(timer_settime(temp_t_id, 0/*TIMER_ABSTIME*/, &val, NULL) == -1){
        RilLog("timer_settimer() error");
    }
}

PsService::PsService(RilContext* pRilContext)
    : Service(pRilContext, RIL_SERVICE_PS)
{
    m_pAttachApn = NULL;
    m_pPreferredApn = NULL;
    m_nPdpContextSize = 0;
    m_bIsAttachDone = false;
    m_pActivatingPdpContext = NULL;
    m_pDeactivatingPdpContext = NULL;

    memset(m_imsi, 0, sizeof(m_imsi));

    m_pNetLinkMonitor = NULL;
    m_nAttachCid = 0;

    for ( unsigned int i = 0; i < MAX_DATA_CALL_SIZE; i++ )
    {
        m_PdpContext[i] = NULL;
    }

    map_timer_id.clear();
    m_bForceFailAfterSuccess = false;
}

PsService::~PsService()
{
    if (m_pAttachApn != NULL) {
        delete m_pAttachApn;
        m_pAttachApn = NULL;
    }

    if (m_pPreferredApn != NULL) {
        delete m_pPreferredApn;
        m_pPreferredApn = NULL;
    }

    if (m_pNetLinkMonitor != NULL) {
        delete m_pNetLinkMonitor;
        m_pNetLinkMonitor = NULL;
    }
}

int PsService::OnCreate(RilContext *pRilContext)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);

    if(InitPdpContext() != TRUE)
    {
        RilLogE("[%s] %s Fail to InitPdpContext", m_szSvcName, __FUNCTION__);
        return -1;
    }

    m_pNetLinkMonitor = new NetLinkMonitor(pRilContext);
    if (m_pNetLinkMonitor != NULL) {
        m_pNetLinkMonitor->Start();
    }

    return 0;
}

void PsService::OnDestroy()
{
    ResetPdpContext();
}

BOOL PsService::OnHandleRequest(Message* pMsg)
{
    int ret = -1;
    if (pMsg == NULL) {
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_PS_SETUP_DATA_CALL:
        ret = DoSetupDataCall(pMsg);
        break;
    case MSG_PS_DEACT_DATA_CALL:
        ret = DoDeactDataCall(pMsg);
        break;
    case MSG_PS_GET_DATA_CALL_LIST:
        ret = DoGetDataCallList(pMsg);
        break;
    case MSG_PS_SET_INITIAL_ATTACH_APN:
        ret = DoSetInitialAttachApn(pMsg);
        break;
    case MSG_PS_REFRESH_INITIAL_ATTACH_APN:
        ret = DoRefreshInitialAttachApn(pMsg);
        break;
    case MSG_PS_SET_DATA_PROFILE:
        ret = DoSetDataProfile(pMsg);
        break;
        /*
    case MSG_PS_SET_DATA_PROFILE_INITIAL_ATTACH:
        ret = DoSetDataProfileInitialAttachApnAsync(pMsg);
        break;
        */
    case MSG_PS_START_KEEPALIVE:
        ret = DoStartKeepAlive(pMsg);
        break;
    case MSG_PS_STOP_KEEPALIVE:
        ret = DoStopKeepAlive(pMsg);
        break;
    case MSG_PS_SET_PREFERRED_DATA_MODEM:
        ret = DoSetPreferredDataModem(pMsg);
        break;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL PsService::OnHandleSolicitedResponse(Message* pMsg)
{
    if (pMsg == NULL) {
        return FALSE;
    }

    int ret = -1;
    switch (pMsg->GetMsgId()) {
    case MSG_PS_SETUP_DATA_CALL_DONE:
        ret = OnSetupDataCallDone(pMsg);
        break;
    case MSG_PS_DEACT_DATA_CALL_DONE:
        ret = OnDeactDataCallDone(pMsg);
        break;
    case MSG_PS_GET_DATA_CALL_LIST_DONE:
        ret = OnGetDataCallListDone(pMsg);
        break;
    case MSG_PS_SET_INITIAL_ATTACH_APN_DONE:
        ret = OnSetInitialAttachApnDone(pMsg);
        break;
    case MSG_PS_SET_DATA_PROFILE_DONE:
        ret = OnSetDataProfileDone(pMsg);
        break;
    case MSG_PS_START_KEEPALIVE_DONE:
        ret = OnStartKeepAliveDone(pMsg);
        break;
    case MSG_PS_STOP_KEEPALIVE_DONE:
        ret = OnStopKeepAliveDone(pMsg);
        break;
    case MSG_PS_SET_PREFERRED_DATA_MODEM_DONE:
        ret = OnSetPreferredDataModemDone(pMsg);
        break;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL PsService::OnHandleUnsolicitedResponse(Message* pMsg)
{
    if (pMsg == NULL) {
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_PS_DATA_CALL_LIST_CHANGED:
        OnDataCallListChanged(pMsg);
        break;
    case MSG_PS_DEDICATED_BEARER_INFO_UPDATED:
        OnDedicatedBearerInfoUpdated(pMsg);
        break;
    case MSG_PS_NAS_TIMER_STATUS_CHANGED:
        OnNasTimerStatusChanged(pMsg);
        break;
    case MSG_PS_IND_KEEPALIVE_STATUS:
        OnUnsolKeepaliveStatus(pMsg);
        break;
    case MSG_PS_IND_PCO_DATA:
        OnUnsolPcoData(pMsg);
        break;
    } // end switch ~

    return TRUE;
}

BOOL PsService::OnHandleInternalMessage(Message* pMsg)
{
    if (pMsg == NULL) {
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_PS_IPV6_CONFIGURED:
        OnSetupDataCallIPv6Configured(pMsg);
        break;
    case MSG_PS_SIM_OPERATOR_INFO_UPDATED:
        OnSimOperatorInfoUpdated(pMsg);
        break;
    } // end switch ~
    return TRUE;
}

BOOL PsService::OnHandleRequestTimeout(Message* pMsg)
{
    if (pMsg == NULL) {
        return FALSE;
    }

    int ret = -1;
    switch (pMsg->GetMsgId()) {
    case MSG_PS_SETUP_DATA_CALL:
        ret = OnSetupDataCallTimeout(pMsg);
        break;
    case MSG_PS_DEACT_DATA_CALL:
        ret = OnDeactDataCallTimeout(pMsg);
        break;
    } // end switch ~
    return (ret < 0 ? FALSE : TRUE);
}

void PsService::OnRadioStateChanged(int radioState)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    switch (radioState) {
    case RADIO_STATE_OFF:
    case RADIO_STATE_UNAVAILABLE:
        m_pActivatingPdpContext = NULL;
        m_pDeactivatingPdpContext = NULL;
        m_bIsAttachDone = false;
        ResetPdpContext(true);
        break;

    } // end switch ~
}

void PsService::OnSimStatusChanged(int cardState, int appState)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if ( cardState == RIL_CARDSTATE_ABSENT ) {

        //notify prev. setup data call failure
        if ( m_pActivatingPdpContext != NULL )
        {
            RilLogW("In Pdp activating state : notify setup data failure");
            OnSetupDataCallComplete(RIL_E_GENERIC_FAILURE, m_pActivatingPdpContext);
            m_pActivatingPdpContext = NULL;
        }

        if ( m_pDeactivatingPdpContext != NULL ) {
            RilLogW("In Pdp deactivating state : notify deact. success anyway");
            OnDeactDataCallComplete(m_pDeactivatingPdpContext);
            m_pDeactivatingPdpContext = NULL;
        }

        m_bIsAttachDone = false;
        ResetPdpContext();
        OnNotifyDataCallList();
    }
}

void PsService::OnImsiUpdated(const char *imsi)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);
    if (TextUtils::IsEmpty(imsi) || !(strlen(imsi) >= 5)) {
        RilLogE("Invalid IMSI value. Please check it");
        return ;
    }

    string strOldImsi = m_imsi;
    string strNewImsi = imsi;
    memset(m_imsi, 0, sizeof(m_imsi));
    strncpy(m_imsi, imsi, MAX_IMSI_LEN);

    if (!TextUtils::IsEmpty(strOldImsi) && !TextUtils::Equals(strOldImsi, strNewImsi)) {
        RilLog("[%s] IMSI switched : %s -> %s", GetServiceName(), strOldImsi.c_str(), strNewImsi.c_str());
        PdpContext *pPdpContext = GetAttachPdpContext();
        if (pPdpContext != NULL) {
            pPdpContext->SetApnSetting(NULL);
        }
    }

}

const char *PsService::GetAttachApnType()
{
    string strSimPlmn = GetSimOperatorNumeric();
    const char *carrier = strSimPlmn.c_str();
    return TelephonyProvider::GetInstance()->GetAttachApnType(carrier);
}

int PsService::GetAttachCid() const
{
    // SIM 1 : Attach CID may be 1
    // SIM 2 : Attach CID may be 5
    return m_nAttachCid;
}

BOOL PsService::InitPdpContext()
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
    const char *ifprefix = NULL;
    int istart = 0;
    m_nPdpContextSize = sizeof(m_PdpContext) / sizeof(m_PdpContext[0]);
    const RilContextParam *param = m_pRilContext->GetRilContextParam();
    if (param != NULL) {
        ifprefix = param->ifprefix;
        istart = param->ifstart;
        if ((unsigned int)m_nPdpContextSize > param->ifmaxsize) {
            m_nPdpContextSize = param->ifmaxsize;
        }
        m_nAttachCid = istart + 1;
    }

    RilLogV("Max PDP Context size=%d", m_nPdpContextSize);
    for (int i = 0; i < m_nPdpContextSize; i++, istart++) {
        int cid = istart + 1;
        m_PdpContext[i] = new PdpContext(cid, ifprefix, istart);
        if (m_PdpContext[i] != NULL) {
            m_PdpContext[i]->Init();
        }
    } // end for i ~
    return TRUE;
}

BOOL PsService::ResetPdpContext(bool bKeepAttachApn)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
    for (int i = 0; i < m_nPdpContextSize; i++) {
        PdpContext *pPdpContext = GetPdpContextByIndex(i);
        if (pPdpContext != NULL) {
            // reset PDP Context and bring down interfaces except for EIMS OnSimStatusChanged
            if((pPdpContext->GetApnSetting() != NULL) &&
               (TextUtils::Equals(pPdpContext->GetApnSetting()->GetType(), APN_TYPE_EMERGENCY)) &&
               (bKeepAttachApn == false) && (m_bIsAttachDone == false)) {
                RilLogV("Skip ResetPdpContext EIMS");
            } else {
                pPdpContext->OnDeactivated();
            }
            // delete old APN setting information
            if ( bKeepAttachApn == false || pPdpContext->GetCID() != GetAttachCid() )
            {
                pPdpContext->SetApnSetting(NULL);
            }
        }
    }
    return TRUE;
}

PdpContext *PsService::GetPdpContext(int cid)
{
    for (int i = 0; i < m_nPdpContextSize; i++) {
        PdpContext *pPdpContext = m_PdpContext[i];
        if (pPdpContext != NULL && pPdpContext->GetCID() == cid) {
            return pPdpContext;
        }
    } // end for i ~
    RilLogW("[%s] %s() Not found PDPContext for cid = %d", m_szSvcName, __FUNCTION__, cid);

    return NULL;
}

PdpContext *PsService::GetPdpContextByIndex(int index)
{
    if (index >= 0 && index < m_nPdpContextSize) {
        return m_PdpContext[index];
    }

    RilLogW("[%s] %s() Out of range : index=%d", m_szSvcName, __FUNCTION__, index);
    return NULL;
}

PdpContext *PsService::GetAvailablePdpContext(ApnSetting *pApnSetting, int dataProfileId)
{
    if (pApnSetting == NULL) {
        return NULL;
    }

    // at first, try to find same data profile and same apn including attach PDP context
    RilLogV("[%s] Try to find reusable PDPContext", m_szSvcName);
    RilLogV("[-]Requested: Data ProfileId=%d, ApnSetting{apn=%s,apntype=%s}", dataProfileId, pApnSetting->GetApn(), pApnSetting->GetType());
    for (int i = 0; i < m_nPdpContextSize; i++) {
        PdpContext *pPdpContext = GetPdpContextByIndex(i);
        if (pPdpContext != NULL) {
            const ApnSetting *pExistsApnSetting = pPdpContext->GetApnSetting();

            if (pExistsApnSetting != NULL) {
                RilLogV("[%d]PDPContext{cid=%d,Data Profile=%d,state=%d},ApnSetting{apn=%s,apntype=%s}",
                        i, pPdpContext->GetCID(), pPdpContext->GetDataProfileId(), pPdpContext->GetState(),
                        pExistsApnSetting->GetApn(), pExistsApnSetting->GetType());
                if (pPdpContext->GetDataProfileId() == dataProfileId && TextUtils::Equals(pExistsApnSetting->GetApn(), pApnSetting->GetApn())) {
                    // do not change if selected PDP Context is attach PDP context
                    bool usedForAttach = true;
                    if (pPdpContext != GetAttachPdpContext()) {
                        pPdpContext->SetApnSetting(pApnSetting);
                        usedForAttach = false;
                    }
                    RilLogV("Available PDPContext(cid=%d), UsedForAttach(%d)", pPdpContext->GetCID(), usedForAttach);
                    return pPdpContext;
                }
            }
        }
    } // end for i ~

    // 2nd, find currently available PDP context
    RilLogV("[%s] Try to find Available PDPContext", m_szSvcName);
    for (int i = 0; i < m_nPdpContextSize; i++) {
        PdpContext *pPdpContext = GetPdpContextByIndex(i);
        if (pPdpContext != NULL) {
            RilLogV("[%d]PDPContext{cid=%d,state=%d}", i, pPdpContext->GetCID(), pPdpContext->GetState());
            if (pPdpContext->GetCID() == GetAttachCid()) {
                continue;
            }

            if (pPdpContext->IsAvailable()) {
                pPdpContext->SetApnSetting(pApnSetting);
                pPdpContext->SetDataProfileId(dataProfileId);
                RilLogV("Available PDPContext(cid=%d)", pPdpContext->GetCID());
                return pPdpContext;
            }
        }
    } // end for i ~
    RilLogW("[%s] Not found available PDPContext", m_szSvcName);

    return NULL;
}

PdpContext *PsService::GetAttachPdpContext()
{
    return GetPdpContext(GetAttachCid());
}

PdpContext *PsService::GetAttachPdpContext(ApnSetting *pApnSetting)
{
    if (pApnSetting == NULL) {
        return NULL;
    }

    // Recover ProfileId from APN_TYPE
    RilLogV("Attach APN Type = %s", pApnSetting->GetType());
    int dataProfileId = DATA_PROFILE_DEFAULT;
    if ((!TextUtils::Equals(pApnSetting->GetType(), APN_TYPE_ALL) && pApnSetting->CanHandleType(APN_TYPE_IMS)) ||
        (TextUtils::Equals(GetAttachApnType(), APN_TYPE_IMS) && pApnSetting->CanHandleType(APN_TYPE_IMS)) ) {
        RilLogV("Attach data profile to IMS");
        dataProfileId = DATA_PROFILE_IMS;
    }

    int attachCid = GetAttachCid();
    PdpContext *pAttachPdpContext = GetPdpContext(attachCid);
    if (pAttachPdpContext == NULL) {
        return NULL;
    }

    // TODO need a discussion if old APN is existed and different with new APN
    ApnSetting *pOldApnSetting = pAttachPdpContext->GetApnSetting();
    if (pOldApnSetting != NULL) {

    }

    // set new APN
    pAttachPdpContext->SetApnSetting(pApnSetting);
    pAttachPdpContext->SetDataProfileId(dataProfileId);

    return pAttachPdpContext;
}

int PsService::DoSetupDataCall(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    SetupDataCallRequestData *rildata = (SetupDataCallRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    string strSimPlmn = GetSimOperatorNumeric();
    const char *carrier = strSimPlmn.c_str();
    int rat = rildata->GetRadioTech();
    const char *apn = rildata->GetApn();
    const char *username = rildata->GetUsername();
    const char *password = rildata->GetPassword();
    int authType = rildata->GetAuthType();
    const char *protocol = rildata->GetProtocol();
    const char *roaming_protocol = rildata->GetRoamingProtocol();
    int dataProfileId = rildata->GetDataProfileId();
    int supportedApnTypesBitmap = rildata->GetSupportedApnTypesBitmap();
    bool roamingAllowed = rildata->IsRoamingAllowed();

    // Hook and Deliver Internet SetupDataCall Information as DataProfile
    if (supportedApnTypesBitmap & RIL_APN_TYPE_DEFAULT)
        DoSetDataProfileDirect(rildata);

    ApnSetting *pApnSetting = ApnSetting::NewInstance(carrier, apn, supportedApnTypesBitmap, username, password, protocol, roaming_protocol, authType);
    if (pApnSetting == NULL) {
        RilLogW("Cannot create ApnSetting instance");
        OnSetupDataCallComplete(RIL_E_GENERIC_FAILURE, NULL);
        return 0;
    }

    PdpContext *pPdpContext = GetAvailablePdpContext(pApnSetting, dataProfileId);
    if (pPdpContext == NULL) {
        RilLogE("No available PDP Context");
        OnSetupDataCallComplete(RIL_E_GENERIC_FAILURE, NULL);

        if (pApnSetting != NULL) {
            delete pApnSetting;
        }
        return 0;
    }

    RilLog("[%d] %s", GetRilSocketId(), pApnSetting->ToString().c_str());
    if (!pPdpContext->IsAvailable()) {
        if (pPdpContext->GetState() == PDP_CONTEXT_CONNECTED) {
            RilLogV("Already connected CID=%d, APN=%s", pPdpContext->GetCID(), pApnSetting->GetApn());
            PsDataBuilder builder;
            const RilData *rildata = builder.BuildSetupDataCallResponse(pPdpContext);
            if (rildata != NULL) {
                OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
                delete rildata;
                return 0;
            }
        }
        RilLogE("No available PDP Context. PDP Context State=%d", pPdpContext->GetState());
        // We can examine 'Not a DISCONNECTED' and 'Not a CONNECTED' Cases
        OnSetupDataCallComplete(RIL_E_GENERIC_FAILURE, pPdpContext);
        return 0;
    }

    // Check Radio Power Off/Unavailable Case earlier
    // If CP has no limitation to get SETUP_DATA_CALL in Radio Power off state,
    // We can skip this
    if (m_pRilContext->GetCurrentRadioState() == RADIO_STATE_UNAVAILABLE) {
        RilLogV("[%s] %s() Current Radio State is RADIO_STATE_UNAVAILABLE", m_szSvcName,  __FUNCTION__);
        OnSetupDataCallCompletePdpFail(PDP_FAIL_SIGNAL_LOST);
        return 0;
    }
    if (m_pRilContext->GetCurrentRadioState() == RADIO_STATE_OFF) {
        RilLogV("[%s] %s() Current Radio State is RADIO_STATE_ROFF", m_szSvcName,  __FUNCTION__);
        OnSetupDataCallCompletePdpFail(PDP_FAIL_RADIO_POWER_OFF);
        return 0;
    }

    if(isRoamState()) {
        protocol = roaming_protocol;
        RilLogV("isRoamState(%d) roamingAllowed(%d), protocol(%s)", isRoamState(), roamingAllowed, protocol);
    }
    //Now update Protocol type by RoamState
    RilLogV("RoamState:%d, roamingAllowed:%d, updated protocol:%s, original protocol:%s, roaming_protocol:%s",
            isRoamState(), roamingAllowed, protocol, rildata->GetProtocol(), roaming_protocol);
    ApnSetting *apnSetting = pPdpContext->GetApnSetting();
    if (!apnSetting) {
        RilLogE("apnSetting is null, SetupDataCall will fail. pApnSetting:%p, pPdpContext:%p", pApnSetting, pPdpContext);
        OnSetupDataCallComplete(RIL_E_GENERIC_FAILURE, NULL);
        return 0;
    }
    apnSetting->UpdateProtocol(protocol);

    bool isImsType = TextUtils::Equals(apnSetting->GetType(), APN_TYPE_IMS);
    if (isImsType && isRejectRatForIMS(rat))
    {
        RilLogV("[%s] %s() Not available for RAT = %d ", m_szSvcName,  __FUNCTION__, rat);
        OnSetupDataCallCompletePdpFail(PDP_FAIL_ERROR_UNSPECIFIED);
        return 0;
    }

    ProtocolPsBuilder builder;
    ModemData *pModemData = builder.BuildSetupDataCall(rat, pPdpContext);
    if (SendRequest(pModemData, TIMEOUT_SETUP_DATA_CALL, MSG_PS_SETUP_DATA_CALL_DONE) < 0) {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        return 0;
    }

    pPdpContext->SetState(PDP_CONTEXT_CONNECTING);
    m_pActivatingPdpContext = pPdpContext;

#define TEST_FORCE_FAIL_SETUP_DATA (0)
#if (TEST_FORCE_FAIL_SETUP_DATA == 1)
    if (TextUtils::Equals(pApnSetting->GetType(), APN_TYPE_EMERGENCY))
        m_bForceFailAfterSuccess = true;
#endif

    return 0;
}

int PsService::OnSetupDataCallDone(Message *pMsg)
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

#define TEST_NEW_IPC_FORMAT (0)
#if (TEST_NEW_IPC_FORMAT == 1)
    ModemData *newModemData = NULL;
    newModemData = replaceModemDataForTest(pModemData, 0);

    if(newModemData == NULL)
        newModemData = pModemData;
    else
        delete pModemData;
    ProtocolPsSetupDataCallAdapter adapter(newModemData);
#else
    ProtocolPsSetupDataCallAdapter adapter(pModemData);
#endif

    int errorCode = adapter.GetErrorCode();

#if (TEST_FORCE_FAIL_SETUP_DATA == 1)
    if (m_bForceFailAfterSuccess) {
        errorCode = RIL_E_GENERIC_FAILURE;
        m_bForceFailAfterSuccess = false;
    }
#endif

    if (errorCode == RIL_E_SUCCESS) {
        int cid = adapter.GetCid();
        int status = adapter.GetStatus();
        int active = adapter.GetActiveStatus();
        int mtu = adapter.GetMTU();
        int pco = adapter.GetPCO();
        int suggestedRetryTime = adapter.GetSuggestedRetryTime();
        const DataCall *pDc = adapter.GetDataCall();
        PdpContext *pPdpContext = GetPdpContext(cid);

        RilLogV("===================================================");
        RilLogV("  SetupDataCall Result");
        RilLogV("  CID=%d", cid);
        RilLogV("  Status=0x%X", status);
        RilLogV("  Active=%s(%d)", active ? "ACTIVE" : "INACTIVE", active);
        RilLogV("  MTU=%d, PCO=%d", mtu, pco);
        RilLogV("  suggestedRetryTime=%d", suggestedRetryTime);
        RilLogV("===================================================");

        writeRilEvent(m_szSvcName, __FUNCTION__, "===================================================");
        writeRilEvent(m_szSvcName, __FUNCTION__, "  SetupDataCall Result");
        writeRilEvent(m_szSvcName, __FUNCTION__, "  CID=%d", cid);
        writeRilEvent(m_szSvcName, __FUNCTION__, "  Status=0x%X", status);
        writeRilEvent(m_szSvcName, __FUNCTION__, "  Active=%s(%d)", active ? "ACTIVE" : "INACTIVE", active);
        writeRilEvent(m_szSvcName, __FUNCTION__, "  MTU=%d, PCO=%d", mtu, pco);
        writeRilEvent(m_szSvcName, __FUNCTION__, "  suggestedRetryTime=%d", suggestedRetryTime);
        writeRilEvent(m_szSvcName, __FUNCTION__, "===================================================");


        if (active == ACTIVE) {
            // print IP address information
            PrintAddressInfo(pDc);

            // OnActivated PDP Context
            if (pPdpContext != NULL && pPdpContext->OnActivated(pDc) == 0) {
                // Update P-CSCF address, Use Embedded values in RIL Response
                // TBD: Update MTU, PCO
                // TBD: Check IP Changing
            }
            else {
                RilLogE("PDP Context : OnActivated error");
            }
        }
        else{
            pPdpContext->UpdateDataCallInfo(pDc);
            RilLogE("PDP is inactive state");
        }

        OnSetupDataCallComplete(RIL_E_SUCCESS, pPdpContext);
    } else {
        RilLogW("========== Warning, W/A OnSetupDataCallDone error ==========)");
        RilLogW("ErrorCode=0x%0X", errorCode);
        writeRilEvent(m_szSvcName, __FUNCTION__, "========== Warning, W/A OnSetupDataCallDone error ==========)");
        writeRilEvent(m_szSvcName, __FUNCTION__, "ErrorCode=0x%0X", errorCode);
        if (m_pActivatingPdpContext != NULL) {
            RilLogW("Reset PDP Context CID=%d", m_pActivatingPdpContext->GetCID());
        }

        OnSetupDataCallComplete(errorCode, m_pActivatingPdpContext);
    }

    // Set Fast Dormancy timer info after PDP activation done
    DoSetFastDormancy();

    return 0;
}

void PsService::OnSetupDataCallComplete(int errorCode, PdpContext *pPdpContext)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (errorCode == RIL_E_SUCCESS) {
        if (pPdpContext != NULL) {
            int cid = pPdpContext->GetCID();
            int state = pPdpContext->GetState();
            int active = pPdpContext->GetActive();

            if (state == PDP_CONTEXT_IPV6_CONFIGURING) {
                // Launch Timer with Timeout of RTR_MAX*RTR_INTERVAL
                SetRSRATimeoutTimer(cid, pPdpContext->GetInterfaceName());

//#define DEFER_IPV6_GLOBAL_ADDRESS_GEN
#ifdef DEFER_IPV6_GLOBAL_ADDRESS_GEN
                DataCall *dc = pPdpContext->GetDataCallInfo();
                ApnSetting* pApnSetting = pPdpContext->GetApnSetting();
                if( NULL != pApnSetting && pApnSetting->CanHandleType(APN_TYPE_DEFAULT) &&
                    dc->ipv4.valid && dc->ipv6.valid )
                {
                    // No need to wait, New Global IPv6 will be reported with DataCallList Changed.
                    RilLogV("[%s] %s() Return Immediately with IPv6 Link-local address", m_szSvcName, __FUNCTION__);
                    // We are in already ACTIVE_AND_LINKUP
                    //pPdpContext->OnActivated(dc);
                    // Force CONNECTED STATE
                    pPdpContext->SetState(PDP_CONTEXT_CONNECTED);
                    state = PDP_CONTEXT_CONNECTED;
                }
                else
#endif
                {
                    // waiting IPv6 RS/RA OnSetupDataCallComplete
                    RilLogV("[%s] %s() Waiting until IPv6 RS / RA is completed", m_szSvcName, __FUNCTION__);
                    return;
                }
            }
            RilLogV("[%s] state=%d, active=%d", __FUNCTION__, state, active);

            if (state == PDP_CONTEXT_CONNECTED && active == ACTIVE_AND_LINKUP)
            {
                PsDataBuilder builder;
                const RilData *rildata = builder.BuildSetupDataCallResponse(errorCode, pPdpContext);
                DataCall *dc = pPdpContext->GetDataCallInfo();
                if (rildata != NULL) {
                    RilLogW("[%s] %s() Verifying RilData", m_szSvcName, __FUNCTION__);

                    int cid = ((RIL_Data_Call_Response_v11*)rildata->GetData())[0].cid;
                    int status = ((RIL_Data_Call_Response_v11*)rildata->GetData())[0].status;
                    int active = ((RIL_Data_Call_Response_v11*)rildata->GetData())[0].active;
                    int mtu = ((RIL_Data_Call_Response_v11*)rildata->GetData())[0].mtu;
                    // pco field is not a member of RIL DataCall Response
                    int pco = dc->pco;
                    int suggestedRetryTime = ((RIL_Data_Call_Response_v11*)rildata->GetData())[0].suggestedRetryTime;

                    RilLogV("===================================================");
                    RilLogV("  OnSetupDataCallComplete");
                    RilLogV("  CID=%d", cid);
                    RilLogV("  Status=0x%X", status);
                    RilLogV("  Active=%s(%d)", active ? "ACTIVE" : "INACTIVE", active);
                    RilLogV("  MTU=%d", mtu);
                    RilLogV("  PCO=%x", pco);
                    RilLogV("  suggestedRetryTime=%d", suggestedRetryTime);
                    RilLogV("  Length=%d, sizeof(v11)=(%d)", rildata->GetDataLength(), sizeof(RIL_Data_Call_Response_v11));
                    RilLogV("===================================================");

                    // Check AMBR
                    decodeAMBR(dc);
                    // Notify AMBR on Internet success case
                    ApnSetting* pApnSetting = pPdpContext->GetApnSetting();
                    if( NULL != pApnSetting && pApnSetting->CanHandleType(APN_TYPE_DEFAULT)) {
                        notifyAMBR(dc);
                    }

                    OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
                    delete rildata;

                    // notify data call list
                    OnNotifyDataCallList();
                    m_pActivatingPdpContext = NULL;
                    return;
                }
            }
        }
    }

    RilLogV("[%s] %s errorCode is %d, pPdpContext=%p", m_szSvcName, __FUNCTION__, errorCode, pPdpContext);
    // send a result as SUCCESS even if status is not PDP_FAIL_NONE.
    // status, suggestedRetryTime is determined in BuildSetupDataCallResponse.
    PsDataBuilder builder;
    const RilData *rildata = builder.BuildSetupDataCallResponse(errorCode, pPdpContext);
    if (rildata != NULL) {
        OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }

    // In case ERROR or fail to request complete even if errorCode is SUCCESS.
    if (pPdpContext != NULL) {
        RilLogV("PDP Context{cid=%d,state=%d,active=%d}", pPdpContext->GetCID(), pPdpContext->GetState(), pPdpContext->GetActive());
        int state = pPdpContext->GetState();
        int active = pPdpContext->GetActive();
        bool deact = (state == PDP_CONTEXT_CONNECTING && (active >= ACTIVE_AND_LINKDOWN || errorCode != RIL_E_SUCCESS)) ||
                     (state == PDP_CONTEXT_CONNECTED || state == PDP_CONTEXT_IPV6_CONFIGURING);
        // VZ_REQ_LTEDATARETRY_7783
        int reason = DEACT_REASON_NORMAL;
        if(state == PDP_CONTEXT_IPV6_CONFIGURING)
            reason = DEACT_REASON_RSRA_FAIL;
        if (deact) {
            RilLogW("Deactivate selected PDP Context");

            DataCall *dc = pPdpContext->GetDataCallInfo();
            static unsigned char nullIpv4[MAX_IPV4_ADDR_LEN] = { 0, };
            //recovery for IPv4 0000, set deact reason as PDP reset
            if ((dc->pdpType != PDP_TYPE_IPV6) && (memcmp(nullIpv4, dc->ipv4.addr, sizeof(nullIpv4)) == 0)) {
                RilLogW("IPv4 is 0000, DEACT_REASON_PDP_RESET");
                reason = DEACT_REASON_PDP_RESET;
            }
            OnRequestDeactDataCall(pPdpContext, reason);
        }
        pPdpContext->OnDeactivated();
    }

    // clear previous activated PDP Context
    m_pActivatingPdpContext = NULL;
}

void PsService::OnSetupDataCallCompletePdpFail(int pdp_fail_status)
{
    RilLogV("[%s] %s PDP_FAIL status is %d", m_szSvcName, __FUNCTION__, pdp_fail_status);
    // send a result as SUCCESS even if status is not PDP_FAIL_NONE.
    // status, suggestedRetryTime is determined in BuildSetupDataCallResponse.
    PsDataBuilder builder;
    const RilData *rildata = builder.BuildSetupDataCallResponse(RIL_E_GENERIC_FAILURE, pdp_fail_status);
    if (rildata != NULL) {
        OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }
}

void PsService::OnNotifyDataCallList()
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    RilLogV("********** Prepare DataCall *************");
    PsDataCallListBuilder builder;

#if USE_RESTRICTED_STATE_PS_ALL_FOR_PCO_3
    int restricted=RIL_RESTRICTED_STATE_NONE;
    int *pco_pdp= new int[m_nPdpContextSize];
#endif

    for (int i = 0; i < m_nPdpContextSize; i++) {
        PdpContext *pPdpContext = GetPdpContextByIndex(i);
        if (pPdpContext != NULL) {
            RilLogV("[%d] PDP Context{cid=%d,state=%d,active=%d}",
                    i, pPdpContext->GetCID(), pPdpContext->GetState(), pPdpContext->GetActive());
            if (pPdpContext->GetState() == PDP_CONTEXT_CONNECTED) {
                builder.AddDataCall(pPdpContext);
            }
            // Check AMBR
            DataCall *dc = pPdpContext->GetDataCallInfo();
            decodeAMBR(dc);
            // Notify AMBR
            ApnSetting* pApnSetting = pPdpContext->GetApnSetting();
            if( NULL != pApnSetting && pApnSetting->CanHandleType(APN_TYPE_DEFAULT)) {
                notifyAMBR(dc);
            }
#if USE_RESTRICTED_STATE_PS_ALL_FOR_PCO_3
            DataCall *dc = pPdpContext->GetDataCallInfo();
            pco_pdp[i] = dc->pco;
            restricted |= RIL_RESTRICTED_STATE_PS_ALL;//(dc->pco==3) ? RIL_RESTRICTED_STATE_PS_ALL: RIL_RESTRICTED_STATE_NONE;
        }
        else { pco_pdp[i]=0; }
#else
        }
#endif

    } // end for i ~
    RilLogV("****************************************");


    const RilData *rildata = builder.Build();
    if (rildata != NULL) {
        OnUnsolicitedResponse(RIL_UNSOL_DATA_CALL_LIST_CHANGED, rildata->GetData(), rildata->GetDataLength());
#if USE_RESTRICTED_STATE_PS_ALL_FOR_PCO_3
        for(int i=0; i< m_nPdpContextSize; i++){
            RilLogV("PCO[%d]=%d", i, pco_pdp[i]);
        }
        RilLogV("Restricted=%d", restricted);
        // TBD:How to deliver each PCO of all PDN
        OnUnsolicitedResponse(RIL_UNSOL_RESTRICTED_STATE_CHANGED, &restricted, sizeof(restricted));
        delete pco_pdp;
#endif

        delete rildata;
    }
}

int PsService::OnSetupDataCallTimeout(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    // complete connecting PDP context
    if (m_pActivatingPdpContext != NULL) {
        if (m_pActivatingPdpContext->GetState() == PDP_CONTEXT_IPV6_CONFIGURING) {
            DataCall *dc = m_pActivatingPdpContext->GetDataCallInfo();
            if (dc->ipv4.valid) {
                // clear IPv6 info and keep IPv4 only.
                memset(&dc->ipv6, 0, sizeof(dc->ipv6));

                // update final state
                m_pActivatingPdpContext->SetState(PDP_CONTEXT_CONNECTED);

                int cid = dc->cid;
                int status = dc->status;
                int active = dc->active;
                int mtu = dc->mtu_size;
                int pco = dc->pco;

                RilLogV("===================================================");
                RilLogV("  OnSetupDataCallTimeout");
                RilLogV("  CID=%d", cid);
                RilLogV("  Status=0x%X", status);
                RilLogV("  Active=%s(%d)", active ? "ACTIVE" : "INACTIVE", active);
                RilLogV("  MTU=%d, PCO=%d", mtu, pco);
                RilLogV("===================================================");

                // print IP information again
                PrintAddressInfo(dc);
                OnSetupDataCallComplete(RIL_E_SUCCESS, m_pActivatingPdpContext);
                return 0;
            }
        }
        RilLogW("Reset Activating PDP Context CID=%d", m_pActivatingPdpContext->GetCID());
    }
    else {
        RilLogW("Timeout DoSetupDataCall but no Activating PDP Context");
    }
    OnSetupDataCallComplete(RIL_E_GENERIC_FAILURE, m_pActivatingPdpContext);

    return 0;
}

int PsService::OnSetupDataCallIPv6Configured(Message *pMsg)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RilDataRaw *rildata = static_cast<RilDataRaw *>(pMsg->GetUserData());
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    Ipv6Prefix *ipv6prefix = (Ipv6Prefix *)rildata->GetData();
    if (ipv6prefix == NULL) {
        RilLogE("ipv6prefix is NULL");
        return -1;
    }

     RilLogI("%s(): index=%d from NETLINK", __FUNCTION__, ipv6prefix->index);
     int skfd = NetIfController::GetIfSockV6();
    if ( skfd < 0 )
    {
        RilLogE("socket is minus");
        return -1;
    }

    for (int i = 0; i < m_nPdpContextSize; i++) {
        PdpContext *pPdpContext = GetPdpContextByIndex(i);
        if ( pPdpContext == NULL )
        {
            continue;
        }
        int index = NetIfController::GetIfIndex(skfd, pPdpContext->GetInterfaceName());
        RilLogV("%s(): ifname=%s index=%d", __FUNCTION__, pPdpContext->GetInterfaceName(), index);
        if (ipv6prefix->index == index) {
            if (pPdpContext->GetState() == PDP_CONTEXT_IPV6_CONFIGURING
#ifdef DEFER_IPV6_GLOBAL_ADDRESS_GEN
                || pPdpContext->GetState() == PDP_CONTEXT_CONNECTED
#endif
                )
            {
                bool bNeedtoCompleteSetupDataCall = (pPdpContext->GetState() == PDP_CONTEXT_IPV6_CONFIGURING);
                RilLogV("PDP Context{cid=%d,state=%d}, needComplete:%d", pPdpContext->GetCID(), pPdpContext->GetState(), bNeedtoCompleteSetupDataCall);
                DataCall *dc = pPdpContext->GetDataCallInfo();
                if (dc != NULL && dc->ipv6.valid) {
                    memcpy(dc->ipv6.addr, ipv6prefix->prefix.s6_addr, 8);
                    memcpy(dc->ipv6.gw, ipv6prefix->gateway_addr.s6_addr, 16);
                    // Make Gateway address with route source address, needed?
                    //memcpy(dc->ipv6.gw, ipv6prefix->prefix.s6_addr, 8);
                    pPdpContext->OnActivated(dc);

                    int cid = dc->cid;
                    // Here we will change status with 0x1
                    if(!bNeedtoCompleteSetupDataCall && dc->status==PDP_FAIL_NONE)
                        dc->status = PDP_SUCCESS_IPV6_RENEW_PREFIX;
                    int status = dc->status;
                    int active = dc->active;
                    int mtu = dc->mtu_size;
                    int pco = dc->pco;

                    RilLogV("===================================================");
                    RilLogV("  OnSetupDataCallIPv6Configured");
                    RilLogV("  CID=%d", cid);
                    RilLogV("  Status=0x%X", status);
                    RilLogV("  Active=%s(%d)", active ? "ACTIVE" : "INACTIVE", active);
                    RilLogV("  MTU=%d, PCO=%d", mtu, pco);
                    RilLogV("===================================================");
                    // print IP information again
                    PrintAddressInfo(dc);

                    if(bNeedtoCompleteSetupDataCall)
                    {
                        // response the result
                        OnSetupDataCallComplete(RIL_E_SUCCESS, pPdpContext);
                    }
                    else // PDP_CONTEXT_CONNECTED
                    {
                        // Prefix Reconfiguration Case
                        // notify data call list only
                        OnNotifyDataCallList();
                        // Recover original status
                        dc->status=PDP_FAIL_NONE;
                    }
                }
            }
            else
            {
                RilLogW("Unexpected PDP Context state");
                RilLogW("PDP Context{cid=%d,state=%d,active=%d}", pPdpContext->GetCID(), pPdpContext->GetState(), pPdpContext->GetActive());
            }
            break;
        }
    } // end for i ~
    close(skfd);

    return 0;
}

/*
   Allowed RadioError even on CardState::ABSENT are
    RadioError::NONE
    RadioError::INVALID_CALL_ID
    RadioError::SIM_ABSENT
    RadioError::RADIO_NOT_AVAILABLE
    RadioError:OEM_ERROR_1 ~ OEM_ERROR_25
*/
int PsService::DoDeactDataCall(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    DeactivateDataCallRequestData *rildata = (DeactivateDataCallRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int cid = rildata->GetCid();
    int reason = rildata->GetDisconnectReason();

    RilLogI("Try Deact data call : CID=%d, Reason=%d", cid, reason);
    PdpContext *pdpContext = GetPdpContext(cid);
    if (pdpContext == NULL) {
        RilLogE("CID=%d is not valid", cid);
        OnRequestComplete(RIL_E_INVALID_CALL_ID);
        return 0;
    }

    if (pdpContext->GetState() != PDP_CONTEXT_CONNECTED) {
        RilLogE("PDP Context is not CONNTECTED(state=%d)", pdpContext->GetState());
        OnRequestComplete(RIL_E_INVALID_CALL_ID);
        return 0;
    }

    ApnSetting *apnSetting = pdpContext->GetApnSetting();
    bool isImsType = false;
    if (apnSetting) isImsType = TextUtils::Equals(apnSetting->GetType(), APN_TYPE_IMS);
    int imsReg = RIL_IMS_NOT_REGISTERED;

    RilProperty* propertyImsReg = GetRilContextProperty();
    int WaitForImsDereg = 20;
    if (isImsType) {
        RilLogI("[%s] %s(), Check IMS Reg state", m_szSvcName, __FUNCTION__);
        do {
            if (propertyImsReg != NULL) {
                imsReg = propertyImsReg->GetInt(RIL_CONTEXT_IMS_REGISTRATION);
                RilLogI("[%s] %s(), imsReg=%d", m_szSvcName, __FUNCTION__, imsReg);
                if (imsReg == RIL_IMS_NOT_REGISTERED) break;
            }
            usleep(500000); // 500ms interval checks upto 10 seconds
            WaitForImsDereg--;
        } while(WaitForImsDereg > 0);
    }

    ProtocolPsBuilder builder;
    ModemData *pModemData = builder.BuildDeactDataCall(cid, reason);
    if (SendRequest(pModemData, TIMEOUT_DEACT_DATA_CALL, MSG_PS_DEACT_DATA_CALL_DONE) < 0) {
        return -1;
    }
    pdpContext->SetState(PDP_CONTEXT_DISCONNECTING);
    m_pDeactivatingPdpContext = pdpContext;

    return 0;
}

int PsService::OnDeactDataCallDone(Message *pMsg)
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

    if (m_pDeactivatingPdpContext == NULL) {
        RilLogW("PDP Deactivation has been done. But cannot find PDP context!");
        RilLogW("PDP Context may be release by OnDataCallListChanged");
    }

    OnDeactDataCallComplete(m_pDeactivatingPdpContext);

    return 0;
}

int PsService::OnDeactDataCallTimeout(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    if (m_pDeactivatingPdpContext == NULL) {
        RilLogW("PDP Deactivation has been timeout. But cannot find PDP context!");
        RilLogW("PDP Context may be release by OnDataCallListChanged");
    }
    OnDeactDataCallComplete(m_pDeactivatingPdpContext);

    return 0;
}

void PsService::OnDeactDataCallComplete(PdpContext *pPdpContext)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pPdpContext != NULL) {
        // clear P-CSCF address information

        // disconnect
        RilLogV("Last Deact PDP Context : CID=%d", m_pDeactivatingPdpContext->GetCID());
        pPdpContext->OnDeactivated();
    }
    else {
        RilLogW("PDP Context is null");
    }
    OnRequestComplete(RIL_E_SUCCESS);
    OnNotifyDataCallList();
    m_pDeactivatingPdpContext = NULL;
}

int PsService::OnRequestDeactDataCall(PdpContext *pPdpContext, int reason/* = DEACT_REASON_NORMAL*/)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pPdpContext == NULL) {
        RilLogE("pPdpContext is NULL");
        return -1;
    }

    int state = pPdpContext->GetState();
    RilLogV("PDP Context{cid=%d,state=%d,active=%d}", pPdpContext->GetCID(), pPdpContext->GetState(), pPdpContext->GetActive());
    switch (state) {
    case PDP_CONTEXT_CONNECTING:
    case PDP_CONTEXT_CONNECTED:
    case PDP_CONTEXT_IPV6_CONFIGURING:
        break;
    default:
        RilLogW("Unexpected PDP Context state");
        return -1;
    }

    int cid = pPdpContext->GetCID();
    ProtocolPsBuilder builder;
    ModemData *pModemData = builder.BuildDeactDataCall(cid, reason);
    if (pModemData != NULL) {
        if (SendRequest(pModemData) < 0) {
            RilLogW("Fail to send PDP Deact request ");
        }
        delete pModemData;
    }

    return 0;
}

int PsService::OnRequestDeactDataCall(int cid, int reason/* = DEACT_REASON_NORMAL*/)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    ProtocolPsBuilder builder;
    ModemData *pModemData = builder.BuildDeactDataCall(cid, reason);
    if (pModemData != NULL) {
        if (SendRequest(pModemData) < 0) {
            RilLogW("Fail to send PDP Deact request ");
        }
        delete pModemData;
    }
    return 0;
}

int PsService::DoGetDataCallList(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolPsBuilder builder;
    ModemData *pModemData = builder.BuildGetDataCallList();
    if (SendRequest(pModemData, TIMEOUT_DEACT_DATA_CALL, MSG_PS_GET_DATA_CALL_LIST_DONE) < 0) {
        return -1;
    }

    return 0;
}

int PsService::OnGetDataCallListDone(Message *pMsg)
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
#if (TEST_NEW_IPC_FORMAT == 1)
    ModemData *newModemData = NULL;
    newModemData = replaceModemDataForTest(pModemData, 1);

    if(newModemData == NULL)
        newModemData = pModemData;
    else
        delete pModemData;
    ProtocolPsDataCallListAdapter adapter(newModemData);
#else
    ProtocolPsDataCallListAdapter adapter(pModemData);
#endif

    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        PsDataCallListBuilder builder;
        for (int i = 0; i < m_nPdpContextSize; i++) {
            PdpContext *pPdpContext = GetPdpContextByIndex(i);
            if (pPdpContext != NULL) {
                int cid = pPdpContext->GetCID();
                const DataCall *dc = adapter.GetDataCallByCid(cid);
                if (dc != NULL) {
                    bool deact = pPdpContext->GetActive() == INACTIVE && dc->active == ACTIVE;
                    bool reset = pPdpContext->GetActive() >= ACTIVE_AND_LINKDOWN && dc->active == INACTIVE;

                    // RIL inactive / CP active -> request deact and update state
                    if (deact) {
                        RilLogV("Deactivate PDP Context{cid=%d,state=%d,active=%d}", pPdpContext->GetCID(), pPdpContext->GetState(), pPdpContext->GetActive());
                        OnRequestDeactDataCall(cid);
                    }

                    // RIL active / CP inactive -> update state
                    if (deact || reset) {
                        RilLogV("Reset PDP Context{cid=%d,state=%d,active=%d}", pPdpContext->GetCID(), pPdpContext->GetState(), pPdpContext->GetActive());
                        pPdpContext->OnDeactivated();
                    }
                }

                if (pPdpContext->GetState() == PDP_CONTEXT_CONNECTED) {
                    builder.AddDataCall(pPdpContext);
                }
            }
        } // end for i ~

        const RilData *rildata = builder.Build();
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    } else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

int PsService::OnDataCallListChanged(Message *pMsg)
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
#if (TEST_NEW_IPC_FORMAT == 1)
    ModemData *newModemData = NULL;
    newModemData = replaceModemDataForTest(pModemData, 2);

    if(newModemData == NULL)
        newModemData = pModemData;
    else
        delete pModemData;
    ProtocolPsDataCallListChangedAdapter adapter(newModemData);
#else
    ProtocolPsDataCallListChangedAdapter adapter(pModemData);
#endif

    const DataCall *pDataCallList = adapter.GetDataCallList();
    int num = adapter.GetDataCallNum();
    bool bPdpActive=false;
    bool DataCallInformedArray[MAX_DATA_CALL_SIZE*2+1] = {false,};

    bool notifyResult = true;
    RilLog("DataCall Number=%d", num);
    writeRilEvent(m_szSvcName, __FUNCTION__, "DataCall Number=%d", num);
    for (int i = 0; i < num; i++) {
        const DataCall *dc = (pDataCallList + i);
        RilLog("DataCall : CID=%d Active=%d Status=0x%x", dc->cid, dc->active, dc->status);
        writeRilEvent(m_szSvcName, __FUNCTION__, "DataCall : CID=%d Active=%d Status=0x%x", dc->cid, dc->active, dc->status);
        DataCallInformedArray[dc->cid] = true;

        // sync current data call list and PDP context
        // in case RIL PDP Context is active but notified data call is inactive.
        if (dc->active == INACTIVE) {
            PdpContext *pPdpContext =GetPdpContext(dc->cid);
            if (pPdpContext != NULL) {
                RilLogV("PDP Context{cid=%d,state=%d,active=%d}", pPdpContext->GetCID(), pPdpContext->GetState(), pPdpContext->GetActive());
                writeRilEvent(m_szSvcName, __FUNCTION__, "PDP Context{cid=%d,state=%d,active=%d}", pPdpContext->GetCID(), pPdpContext->GetState(), pPdpContext->GetActive());
                if (pPdpContext->GetState() == PDP_CONTEXT_CONNECTED || pPdpContext->GetActive() >= 1) {
                    if (m_pActivatingPdpContext != NULL && m_pActivatingPdpContext->GetState() == PDP_CONTEXT_IPV6_CONFIGURING) {
                        OnSetupDataCallComplete(RIL_E_GENERIC_FAILURE, m_pActivatingPdpContext);
                    }
                    else {
                        pPdpContext->OnDeactivated();
                    }
                    notifyResult = true;
                }
            }
        } else {
            RilLogI("DataCall(cid=%d) from CP is Active", dc->cid);
            bPdpActive = true;
            PdpContext *pPdpContext = GetPdpContext(dc->cid);
            // For invalid cid, we just skip it
            if(pPdpContext == NULL)
                continue;
            if(dc->active == CP_DATA_CONNECTION_DORMANT) {
                RilLogV("Dormant");
                pPdpContext->SetActive(DATA_CONNECTION_ACTIVE_PH_LINK_DORMANT);
            } else {
                RilLogV("Active");
                pPdpContext->SetActive(DATA_CONNECTION_ACTIVE_PH_LINK_UP);
            }
            pPdpContext->UpdateDataCallInfo(dc); // This will update only Matched PdpContext
            //pPdpContext->OnActivated(dc); // This will update pdpContext and try to BringUp,
                                            // But useless currently for AOSP's default behavior because Framework will trigger deact datacall
        }
    } // end for i ~
    for (int i = 0; i < m_nPdpContextSize; i++) {
        PdpContext *pPdpContext = GetPdpContextByIndex(i);
        if (pPdpContext != NULL) {
            int cid = pPdpContext->GetCID();
            if(DataCallInformedArray[cid] == true) {
                ApnSetting *apnSetting = pPdpContext->GetApnSetting();
                if(apnSetting == NULL) {
                    RilLogV("[%s] %s() ApnSetting is null of cid=%d", m_szSvcName,  __FUNCTION__, cid);
                    continue;
                }

                continue;
            }

            bool cleanup = (pPdpContext->GetActive() >= ACTIVE);

            // RIL active / CP inactive -> update state
            if (cleanup) {
                RilLogV("DataCall Mismatch is detected. Cleanup PDP Context{cid=%d,state=%d,active=%d}", pPdpContext->GetCID(), pPdpContext->GetState(), pPdpContext->GetActive());
               writeRilEvent(m_szSvcName, __FUNCTION__, "DataCall Mismatch is detected. Cleanup PDP Context{cid=%d,state=%d, active=%d}", pPdpContext->GetCID(), pPdpContext->GetState(), pPdpContext->GetActive());
                pPdpContext->OnDeactivated();
                notifyResult = true;
            }
        }
    }
    // All DataCall is lost
    if (num == 0) ResetPdpContext(true);
    RilProperty *property = GetRilContextProperty();
    property->Put(RIL_CONTEXT_PDP_ACTIVE, bPdpActive);

    if (notifyResult) {
        OnNotifyDataCallList();
    }

    return 0;
}

int PsService::DoSetInitialAttachApn(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    SetInitialAttachApnRequestData *rildata = (SetInitialAttachApnRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    string strSimPlmn = GetSimOperatorNumeric();
    const char *carrier = strSimPlmn.c_str();
    const char *apn = rildata->GetApn();
    const char *protocol = rildata->GetProtocol();
    const char *roaming_protocol = rildata->GetRoamingProtocol();
    const char *username = rildata->GetUsername();
    const char *password = rildata->GetPassword();
    int authType = rildata->GetAuthType();
    int supportedApnTypesBitmap = rildata->GetSupportedApnTypesBitmap();

    // Check Emergency only InitialAttach Apn Request, if there's no explicit ia type, Do not allow Emergency InitalAttach
    if((supportedApnTypesBitmap & APN_TYPE_BIT_EMERGENCY) && !(supportedApnTypesBitmap & APN_TYPE_BIT_IA)) {
        RilLogW("Emergency InitialAttach is not allowed");
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
        return 0;
    }

    ApnSetting *pNewApnSetting = ApnSetting::NewInstance(carrier, apn, supportedApnTypesBitmap, username, password, protocol, roaming_protocol, authType);
    if (pNewApnSetting == NULL) {
        RilLogW("Cannot create ApnSetting instance");
        OnRequestComplete(RIL_E_INTERNAL_ERR);
        return 0;
    }

    int ret = RequestSetInitialAttachApn(pNewApnSetting);
    if (ret < 0) {
        RilLogV("[%d] Failed to set attach APN info.", GetRilSocketId());
        OnRequestComplete(RIL_E_INTERNAL_ERR);
    }

    if (pNewApnSetting != NULL) {
        delete pNewApnSetting;
        pNewApnSetting = NULL;
    }

    return 0;
}

int PsService::OnSetInitialAttachApnDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        m_bIsAttachDone = true;
    }

    // Android Kikat,
    // call OnRequestComplete
    OnRequestComplete(errorCode);

    return 0;
}

int PsService::OnSetDataProfileDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        m_bIsAttachDone = true;
    }

    OnRequestComplete(errorCode);

    return 0;
}

int PsService::DoStartKeepAlive(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    KeepaliveRequestData *rildata = (KeepaliveRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolPsBuilder builder;
    ModemData *pModemData = builder.BuildStartKeepAlive(rildata->m_keepAliveReq);
    if (SendRequest(pModemData, TIMEOUT_DEFAULT_PS, MSG_PS_START_KEEPALIVE_DONE) < 0) {
        return -1;
    }

    return 0;
}

int PsService::OnStartKeepAliveDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolPsStartKeepAliveAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        RIL_KeepaliveStatus result;
        result.sessionHandle = adapter.getSessionHandle();
        result.code = (RIL_KeepaliveStatusCode)adapter.getCode();
        OnRequestComplete(RIL_E_SUCCESS, &result, sizeof(result));
    } else {
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
    }

    return 0;
}

int PsService::DoStopKeepAlive(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolPsBuilder builder;
    ModemData *pModemData = builder.BuildStopKeepAlive(rildata->GetInt());
    if (SendRequest(pModemData, TIMEOUT_DEFAULT_PS, MSG_PS_STOP_KEEPALIVE_DONE) < 0) {
        return -1;
    }

    return 0;
}


int PsService::OnStopKeepAliveDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    //Modification for VTS
    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    }else{
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
    }


    return 0;
}

int PsService::OnUnsolKeepaliveStatus(Message *pMsg)
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

    ProtocolPsKeepAliveStatusAdapter adapter(pModemData);
    RIL_KeepaliveStatus result;
    result.sessionHandle = adapter.getSessionHandle();
    result.code = (RIL_KeepaliveStatusCode)adapter.getCode();
    OnUnsolicitedResponse(RIL_UNSOL_KEEPALIVE_STATUS, &result, sizeof(result));

    return 0;
}

int PsService::OnUnsolPcoData(Message *pMsg)
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

    ProtocolPsPcoDataAdapter adapter(pModemData);
    PsDataBuilder builder;
    int cid = adapter.GetCid();
    int nPdpType = adapter.GetPdpType();
    int pcoNum = adapter.GetPcoNum();

    RilLogI("[%s] %s() : cid:%d, nPdpType:%d, pcoNum:%d",
            m_szSvcName, __FUNCTION__, cid, nPdpType, pcoNum);
    sit_pdp_pco_data_entry e[256]; // Available MAX Carrier specific PCO Block is 0xFF00~0xFFFF
    for(int i = pcoNum ; i > 0 ; i--) {
        int remainPcoBlocks = adapter.GetPcoData(e[i]);
        if(remainPcoBlocks < 0) {
            RilLogI("[%s] %s() : pconum:%d, cur:%d, remain:%d",
                    m_szSvcName, __FUNCTION__,
                    pcoNum, pcoNum - i, remainPcoBlocks);
            return 0;
        }

        const RilData *pRilData = builder.BuildPcoData(cid, nPdpType, e[i].pco_id, e[i].contents_len, e[i].contents);
        if (pRilData != NULL) {
            RilLogI("[%s] %s() : sending: pco_id:%x(%d), contents_len:%d, contents:%p",
                    m_szSvcName, __FUNCTION__,
                    e[i].pco_id, e[i].pco_id, e[i].contents_len, &e[i].contents);
            OnUnsolicitedResponse(RIL_UNSOL_PCO_DATA, pRilData->GetData(), pRilData->GetDataLength());
            delete pRilData;
        }
    }

    return 0;
}

int PsService::DoRefreshInitialAttachApn(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    SetInitialAttachApnRequestData *rildata = (SetInitialAttachApnRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    // modemCognitive is no more related with initialAttach
    /*
    int modemCognitive = rildata->GetModemCognitive();
    if(modemCognitive == TRUE) {
        RilLogV("modem cognitive is true, skip initial attach");
        OnRequestComplete(RIL_E_SUCCESS);
        return 0;
    } else {
        RilLogV("modem cognitive is false");
    }
    */

    string strSimPlmn = GetSimOperatorNumeric();
    const char *carrier = strSimPlmn.c_str();
    const char *apn = rildata->GetApn();
    const char *username = rildata->GetUsername();
    const char *password = rildata->GetPassword();
    const char *protocol = rildata->GetProtocol();
    const char *roaming_protocol = rildata->GetRoamingProtocol();
    int authType = rildata->GetAuthType();
    int supportedApnTypesBitmap = rildata->GetSupportedApnTypesBitmap();

    // Check Emergency only InitialAttach Apn Request, if there's no explicit ia type, Do not allow Emergency InitalAttach
    if((supportedApnTypesBitmap & APN_TYPE_BIT_EMERGENCY) && !(supportedApnTypesBitmap & APN_TYPE_BIT_IA)) {
        RilLogW("Emergency InitialAttach is not allowed");
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
        return 0;
    }

    ApnSetting *pNewApnSetting = ApnSetting::NewInstance(carrier, apn, supportedApnTypesBitmap, username, password, protocol, roaming_protocol, authType);
    if (pNewApnSetting == NULL) {
        RilLogV("[%d] Failed to set attach APN info. No APN available", GetRilSocketId());
        writeRilEvent(m_szSvcName, __FUNCTION__, "[%d] Failed to set attach APN info. No APN available", GetRilSocketId());
        OnRequestComplete(RIL_E_INTERNAL_ERR);
        return 0;
    }
    writeRilEvent(m_szSvcName, __FUNCTION__, "[%d] Create new APN %s", GetRilSocketId(), pNewApnSetting->ToString().c_str());
    RilLogW("[%d] Create new APN %s", GetRilSocketId(), pNewApnSetting->ToString().c_str());

    // 1. There is the case which RIL does not detect SIM absent during fast hotswap. So RIL does not send initial attach APN to CP in the below code.
    // 2. CP confirmed that there is no need to check  initial attach APN info in RIL side.
    int ret = RequestSetInitialAttachApn(pNewApnSetting);
    if (ret < 0) {
        RilLogV("[%d] Failed to set attach APN info.", GetRilSocketId());
        writeRilEvent(m_szSvcName, __FUNCTION__, "[%d] Failed to set attach APN info.", GetRilSocketId());
        OnRequestComplete(RIL_E_INTERNAL_ERR);
    }

    if (pNewApnSetting != NULL) {
        delete pNewApnSetting;
        pNewApnSetting = NULL;
    }

    return 0;
}

int PsService::DoSetDataProfile(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    SetDataProfileRequestData *rildata = (SetDataProfileRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    bool isVzw = isSameTargetOperator(TARGET_OPER_VZW);
    int size = rildata->GetSize();
    for (int i = 0; i < size; i++) {
        const RIL_DataProfileInfo_V1_4 *dpi = rildata->GetDataProfileInfo(i);
        // send APN if persistent true only
        if (dpi != NULL && dpi->persistent) {
            ProtocolPsBuilder builder;
            ModemData *pModemData = builder.BuildSetDataProfile(dpi, isVzw);
            if (SendRequest(pModemData, TIMEOUT_DEFAULT_PS, MSG_PS_SET_DATA_PROFILE_DONE) < 0) {
                return -1;
            }
        }
    } // end for i ~
    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

/* Only used for Internet Notification, not for VZW */
int PsService::DoSetDataProfileInternal(SetupDataCallRequestData *rildata)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    RilLogV("[%d] DataProfile APN = %s", GetRilSocketId(), rildata->GetApn());

    RIL_DataProfileInfo_V1_4 dpi = rildata->mDataProfileInfo;
    dpi.profileId = 0xFF;

    RIL_DataProfileInfo_V1_4 *dpi_ptr[1];
    dpi_ptr[0] = &rildata->mDataProfileInfo;


    RequestData *pData = RilParser::CreateSetDataProfile(RIL_REQUEST_SET_DATA_PROFILE, 0,
                                                                (char *)&dpi_ptr, sizeof(RIL_DataProfileInfo_V1_4 *));
    if (pData != NULL) {
        Message *msg = Message::ObtainMessage(pData, RIL_SERVICE_PS, MSG_PS_SET_DATA_PROFILE);
        if (GetRilContext()->GetServiceManager()->SendMessage(msg) < 0) {
            if (msg) {
                delete msg;
            }
        }
    }
    return 0;
};

int PsService::DoSetDataProfileDirect(SetupDataCallRequestData *rildata)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    RilLogV("[%d] DataProfile APN = %s", GetRilSocketId(), rildata->GetApn());

    RIL_DataProfileInfo_V1_4 dpi = rildata->mDataProfileInfo;
    dpi.profileId = 0xFF; // CP Expects 0xFF

    bool isVzw = isSameTargetOperator(TARGET_OPER_VZW);
    ProtocolPsBuilder builder;
    ModemData *pModemData = builder.BuildSetDataProfile(&dpi, isVzw);
    // There can be response routing fail, It's intented behavior
    if (SendRequest(pModemData) < 0) {
        RilLogE("SendRequest fail");
    }
    if(pModemData)
        delete pModemData;

    return 0;
};

int PsService::RequestSetInitialAttachApn(const ApnSetting *pApnSetting)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pApnSetting == NULL) {
        RilLogE("[%d] Invalid parameter: ApnSetting is NULL.", GetRilSocketId());
        return -1;
    }

    ApnSetting *pNewApnSetting = pApnSetting->Clone();
    PdpContext *pPdpContext = GetAttachPdpContext(pNewApnSetting);
    if (pPdpContext == NULL) {
        RilLogE("[%d] No available PDP Context for Attach PDN(PDP)", GetRilSocketId());
        if (pNewApnSetting != NULL) {
            delete pNewApnSetting;
            pNewApnSetting = NULL;
        }
        return -1;
    }

    string strSimPlmn = GetSimOperatorNumeric();
    const char *carrier = strSimPlmn.c_str();
    bool isEsmFlagZero = MccTable::IsEsmFlagZeroOperator(carrier);
    RilLogV("[%d] PDP Context{cid=%d,data profile=%d} isEsmFlagZero=%d", GetRilSocketId(),
            pPdpContext->GetCID(), pPdpContext->GetDataProfileId(), isEsmFlagZero);

    ProtocolPsBuilder builder;
    ModemData *pModemData = builder.BuildSetInitialAttachApn(pPdpContext, isEsmFlagZero);
    if (SendRequest(pModemData, TIMEOUT_DEFAULT_PS, MSG_PS_SET_INITIAL_ATTACH_APN_DONE) < 0) {
        pPdpContext->SetApnSetting(NULL);
        return -1;
    }
    return 0;
}

int PsService::OnDedicatedBearerInfoUpdated(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolPsDedicatedBearInfoAdapter adapter(pModemData);
    const DedicatedBearerInfo *pDedicatedBearerInfo = adapter.GetDedicatedBearerInfo();
    if (pDedicatedBearerInfo != NULL) {
        RilLogV("DedicatedBearerInfo{status=0x%02X type=0x%02X QCI=0x%02X DL GBR=%d UL GBR=%d DL MAX GBR=0x%d UL MAX GBR=0x%d}",
                pDedicatedBearerInfo->status, pDedicatedBearerInfo->type, pDedicatedBearerInfo->qci,
                pDedicatedBearerInfo->dl_gbr, pDedicatedBearerInfo->ul_gbr,
                pDedicatedBearerInfo->dl_max_gbr, pDedicatedBearerInfo->ul_max_gbr);
        OnUnsolicitedResponse(RIL_UNSOL_OEM_IMS_DEDICATED_PDN_INFO, pDedicatedBearerInfo, sizeof(DedicatedBearerInfo));
    }
    else {
        RilLogE("Invalid Dedicated Bearer information");
        OnUnsolicitedResponse(RIL_UNSOL_OEM_IMS_DEDICATED_PDN_INFO);
    }

    return 0;
}

/**
 * when IMSI and MCC/MNC code are fetched successfully,
 * internal request is triggered by SIM Service
 *
 * rildata - RilDataStrings
 *           index 0 : IMSI
 *           index 1 : MCC/MNC code
 */
int PsService::OnSimOperatorInfoUpdated(Message *pMsg)
{
    RilLogI("[%s] %s", GetServiceName(), __FUNCTION__);
    return 0;
}

int PsService::OnNasTimerStatusChanged(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolPsNasTimerStatusAdapter adapter(pModemData);
    const SitNasTimerStatus *pNasTimerStatus = adapter.GetNasTimerStatus();

    if (pNasTimerStatus != NULL) {
        RilLogV("NasTimerStatus{type=0x%02X status=0x%02X value=%d apn=%s},size:%d",
                pNasTimerStatus->type, pNasTimerStatus->status, pNasTimerStatus->value,
                pNasTimerStatus->apn, sizeof(SitNasTimerStatus));

        PsDataNasTimerStatusBuilder builder;
        const RilData *rildata = builder.BuildNasTimerStatus(pNasTimerStatus);
        if(rildata != NULL) {
            OnUnsolicitedResponse(RIL_UNSOL_NAS_TIMER_STATUS_IND, rildata->GetData(), rildata->GetDataLength());
            // Framework will reprogress Pending Process, Setup_Data_Call for Attach, Unblock DataFlow
            // Or Block
            delete rildata;
        }
    }
    else {
        RilLogE("Invalid Nas Timer information");
    }

    return 0;
}

void PsService::PrintAddressInfo(const DataCall *pDc)
{
    if (pDc != NULL) {
        char szAddr[100] = { 0, };
        char szDns1[100] = { 0, };
        char szDns2[100] = { 0, };
        char szPcscf[100] = { 0, };
        char szGWAddr[100] = { 0, };

        RilLogV("***** IPv4 *****");
        if (pDc->ipv4.valid) {
            inet_ntop(AF_INET, pDc->ipv4.addr, szAddr, (socklen_t)sizeof(szAddr));
            inet_ntop(AF_INET, pDc->ipv4.dns1, szDns1, (socklen_t)sizeof(szDns1));
            inet_ntop(AF_INET, pDc->ipv4.dns2, szDns2, (socklen_t)sizeof(szDns2));
        }
        RilLogV("IP      : %s", szAddr);
        RilLogV("DNS1    : %s", szDns1);
        RilLogV("DNS2    : %s", szDns2);

        RilLogV("DataCall IPC Version is %d", pDc->IPC_version);
        RilLogV("PCSCF_EXT count is %d", pDc->pcscf_ext_count);

        int pcscf_count = MAX_PCSCF_NUM + pDc->pcscf_ext_count;

        for(int i = 0; i < pcscf_count; i++) {
            *szPcscf = 0;
            if(pDc->ipv4.valid) {
                inet_ntop(AF_INET, &pDc->ipv4.pcscf[i * MAX_IPV4_ADDR_LEN], szPcscf, (socklen_t)sizeof(szPcscf));
            }
            RilLogV("P-CSCF%1d  : %s", i + 1, szPcscf);
        }

        RilLogV("****************");

        *szAddr = 0;
        *szDns1 = 0;
        *szDns2 = 0;
        *szGWAddr = 0;
        if (pDc->ipv6.valid) {
            inet_ntop(AF_INET6, pDc->ipv6.addr, szAddr, (socklen_t)sizeof(szAddr));
            inet_ntop(AF_INET6, pDc->ipv6.dns1, szDns1, (socklen_t)sizeof(szDns1));
            inet_ntop(AF_INET6, pDc->ipv6.dns2, szDns2, (socklen_t)sizeof(szDns2));
            inet_ntop(AF_INET6, pDc->ipv6.gw, szGWAddr, (socklen_t)sizeof(szGWAddr));
        }
        RilLogV("***** IPv6 *****");
        RilLogV("IP      : %s", szAddr);
        RilLogV("DNS1    : %s", szDns1);
        RilLogV("DNS2    : %s", szDns2);

        for(int i = 0; i < pcscf_count; i++) {
            *szPcscf = 0;
            if(pDc->ipv6.valid) {
                inet_ntop(AF_INET6, &pDc->ipv6.pcscf[i * MAX_IPV6_ADDR_LEN], szPcscf, (socklen_t)sizeof(szPcscf));
            }
            RilLogV("P-CSCF%1d  : %s", i + 1, szPcscf);
        }

        RilLogV("****************");
    }
}

bool PsService::IsInternalAttachEnabled()
{
    char buf[100] = { 0 };
    const char *perperty_name = RIL_INTERNAL_ATTACH;
    property_get(perperty_name, buf, "");
    if (buf[0] == '1') {
        return true;
    }

    return false;
}

int PsService::RequestAttachApnInternal(const char *carrier)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    // RIL internal attach if there is no available APN information for initial attach
    // Deprecated

    return 0;
}

int PsService::DoSetFastDormancy(void)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    UINT nTargetOperator = 999;
    BOOL bFastDormancy = FALSE;
    BYTE bLcdOn=0, bLcdOff=0, bRel8LcdOn=0, bRel8LcdOff=0;

    RilProperty* appProperty = GetRilApplicationProperty();
    if ( appProperty != NULL ) {
        nTargetOperator = appProperty->GetInt(RIL_APP_TARGET_OPER);
    }

    if ( (TARGET_OPER_ATT == nTargetOperator) ||
         (TARGET_OPER_TMO == nTargetOperator) ){
        bFastDormancy = TRUE;
        bLcdOff = 10;    //default 5sec
        bRel8LcdOff = 10; //5sec
    } else {
        bFastDormancy = TRUE;
        bLcdOff = 10;
        bRel8LcdOff = 10;
        bLcdOn = 10;
        bRel8LcdOn = 10;
    }

    if ( bFastDormancy == TRUE )
    {
        RilLogI("[%s] Target Carrier(%d) support Fast Dormancy with Timer by 5ms*(%d,%d,%d,%d)",
            m_szSvcName, nTargetOperator, bLcdOn, bLcdOff, bRel8LcdOn, bRel8LcdOff);

        ProtocolPsBuilder builder;
        ModemData *pModemData = builder.BuildSetFastDormancyInfo(bLcdOn, bLcdOff, bRel8LcdOn, bRel8LcdOff);
        if (SendRequest(pModemData) < 0) {
            delete pModemData;
            return -1;
        } else {
            delete pModemData;
        }
    }
    else {
        RilLogI("[%s] Target Carrier(%d) don't need to support Fast Dormancy",m_szSvcName, nTargetOperator);
    }

    return 0;
}

bool PsService::IsPossibleToPassInRadioOffState(int request_id)
{
    switch (request_id) {
        case RIL_REQUEST_SET_INITIAL_ATTACH_APN:
        case RIL_REQUEST_SET_DATA_PROFILE:
        case RIL_REQUEST_SET_PREFERRED_DATA_MODEM:
            break;
        default:
            return false;
    }
    return true;
}

ModemData *PsService::replaceModemDataForTest(ModemData *pModemData, int type)
{
    char ext_payload[][102] = {
      { 0x02, 0x00,
        198, 162, 5, 1,
        198, 162, 15, 12,
        198, 162, 25, 23,
        198, 162, 35, 34,
        198, 162, 45, 45,
        0x20, 0x02, 0x02, 0xd8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0,

        0x20, 0x02, 0x02, 0xd8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xe0,

        0x20, 0x02, 0x02, 0xd8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xd0,

        0x20, 0x02, 0x02, 0xd8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0xc0,

        0x20, 0x02, 0x02, 0xd8, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0xb0},
      { 0x02, 0x00,
        190, 112, 105, 101,
        190, 112, 115, 112,
        190, 112, 125, 123,
        190, 112, 135, 134,
        190, 112, 145, 145,
        0x20, 0x02, 0x02, 0xd8, 0xf0, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x01, 0xf0,

        0x20, 0x02, 0x02, 0xd8, 0xf0, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x02, 0xe0,

        0x20, 0x02, 0x02, 0xd8, 0xf0, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x03, 0xd0,

        0x20, 0x02, 0x02, 0xd8, 0xf0, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x04, 0xc0,

        0x20, 0x02, 0x02, 0xd8, 0xf0, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xf0, 0xf0, 0x00, 0x05, 0xb0},
    };

    bool isTx = pModemData->IsTx();
    const size_t old_payload_length = pModemData->GetLength();
    const size_t DATACALL_EXT_SIZE = sizeof(sit_pdp_data_call_item_pcscf_ext);
    //const size_t NEW_DATACALL_SIZE = DATACALL_EXT_SIZE + sizeof(sit_pdp_data_call_item);
    size_t new_payload_length = 0;
    char *new_payload = NULL;

    RilLogV("=-=- Dump Original ModemData @ %p", pModemData->GetRawData());
    pModemData->Dump();

    switch(type) {
      case 0: // SETUP_DATA_CALL_RSP
          {
              const sit_pdp_setup_data_call_rsp *data = reinterpret_cast<const sit_pdp_setup_data_call_rsp *>(pModemData->GetRawData());
              new_payload_length = old_payload_length + DATACALL_EXT_SIZE;
              new_payload = new char[new_payload_length];

              memcpy(new_payload, data, old_payload_length);
              memcpy(new_payload + old_payload_length, ext_payload[0], DATACALL_EXT_SIZE);

              RilLogV("=-=- Original Data Copy from %p to %p with %d bytes", data, new_payload, old_payload_length);
              RilLogV("=-=- Test Vector Copy from %p to %p with %d bytes", ext_payload[0], new_payload + old_payload_length, DATACALL_EXT_SIZE);
              RilLogV("=-=- ext_payload[0]:%p, ext_payload[1]:%p", ext_payload[0], ext_payload[1]);
              //RilLogV("Dump PCSCF Extension Payload - Test Vector");
              //DumpPcscfExtPayload(ext_payload[0]);
          }
          break;
      case 1: // GET_DATA_CALL_LIST_RSP
          {
              const sit_pdp_get_data_call_list_rsp *data = reinterpret_cast<const sit_pdp_get_data_call_list_rsp *>(pModemData->GetRawData());
              const size_t datacall_num = data->datacall_info_num;
              const size_t hLen = sizeof(RCM_HEADER) + 1;
              const size_t old_datacall_size = (old_payload_length - hLen) / datacall_num;
              new_payload_length = old_payload_length + DATACALL_EXT_SIZE * datacall_num;
              new_payload = new char[new_payload_length];

              memcpy(new_payload, data, hLen);
              for(size_t i=0; i < datacall_num; i++) {
                  memcpy(new_payload + (old_datacall_size + DATACALL_EXT_SIZE) * i, (char *)data + hLen + old_datacall_size * i, old_datacall_size);
                  memcpy(new_payload + (old_datacall_size + DATACALL_EXT_SIZE) * i + old_datacall_size, ext_payload[i%2], DATACALL_EXT_SIZE);
              }
          }
          break;
      case 2: // DATA_CALL_LIST_CHANGED_IND
          {
              const sit_pdp_data_call_list_changed_ind *data = reinterpret_cast<const sit_pdp_data_call_list_changed_ind *>(pModemData->GetRawData());
              const size_t datacall_num = data->datacall_info_num;
              const size_t hLen = sizeof(RCM_IND_HEADER) + 1;
              const size_t old_datacall_size = (old_payload_length - hLen) / datacall_num;
              new_payload_length = old_payload_length + DATACALL_EXT_SIZE * datacall_num;
              new_payload = new char[new_payload_length];

              //DumpBuf((char *)data, old_payload_length);
              memcpy(new_payload, data, hLen);
              for(size_t i=0; i < datacall_num ; i++) {
                  memcpy(new_payload + hLen + (old_datacall_size + DATACALL_EXT_SIZE) * i,
                         (char *)data + hLen + old_datacall_size * i, old_datacall_size);
                  memcpy(new_payload + hLen + (old_datacall_size + DATACALL_EXT_SIZE) * i + old_datacall_size,
                         ext_payload[i%2], DATACALL_EXT_SIZE);
                  RilLogV("=-=- DataCallNum:%d, hLen:%d", i, hLen);
                  RilLogV("=-=- Original Data Copy from %p to %p with %d bytes", data + hLen + old_datacall_size * i,
                          new_payload + hLen + (old_datacall_size + DATACALL_EXT_SIZE) * i, old_datacall_size);
                  RilLogV("=-=- Test Vector Copy from %p to %p with %d bytes", ext_payload[i%2],
                          new_payload + hLen + (old_datacall_size + DATACALL_EXT_SIZE) * i + old_datacall_size, DATACALL_EXT_SIZE);
                  RilLogV("=-=- ext_payload[0]:%p, ext_payload[1]:%p", ext_payload[0], ext_payload[1]);
              }
          }
          break;
    }

    ModemData *newModemData = NULL;
    if(new_payload != NULL) {
        if(new_payload_length > 0) {
            newModemData = new ModemData(new_payload, new_payload_length, isTx);
            delete[] new_payload;
        } else {
            RilLogV("=-=- Invalid payload length");
            delete[] new_payload;
            return NULL;
        }
    } else {
        RilLogV("=-=- Invalid payload");
        return NULL;
    }

    RilLogV("=-=- Original ModemData Length:%d ", old_payload_length);
    RilLogV("=-=- AddedLength:%d, isTx:%u", sizeof(sit_pdp_data_call_item_pcscf_ext), isTx);
    RilLogV("=-=- New ModemData @ %p, Length:%d", newModemData->GetRawData(), newModemData->GetLength());

    RilLogV("=-=- Dump New ModemData - Test Vector is added");
    newModemData->Dump();
    RilLogV("=-=- exit %s",  __func__);
    return newModemData;
    //delete newModemData; // When delete? NO. This is Test code.
}

void PsService::DumpBuf(char *data, size_t tlen)
{
    RilLogV("=-=- Dump Buf @ %p", data);
    char buf[100];
    size_t len = 0;
    for(size_t i=0,j=0; i<tlen; i+=16) {
        len = 0;
        for(j=0; j<16; j++) {
            if(i+j == tlen) break;
            len += snprintf(&buf[len], 100-len, "%02x", data[i+j]);
        }
        buf[len]='\0';
        RilLogV("Dump 16bytes(idx=%d~%d): %s", i, i+j, buf);
    }
}

void PsService::DumpPcscfExtPayload(char *data)
{
    char buf[100];
    size_t len = 0;
    DumpBuf(data, sizeof(sit_pdp_data_call_item_pcscf_ext));
    for(size_t i=0, j=0; i<5; i++) {
        len = 0;
        for(j=0; j<4; j++) {
            len += snprintf(&buf[len], 100-len, "%02x", data[2 + j + 4*i]);
        }
        buf[len]='\0';
        RilLogV("IPv4[%d,%d]: %s", i, 2 + j + 4*i, buf);
        len = 0;
        for(j=0; j<16; j++) {
            len += snprintf(&buf[len], 100-len, "%02x", data[2 + 4*5 + j + 16*i]);
        }
        buf[len]='\0';
        RilLogV("IPv6[%d,%d]: %s", i, 2 + 4*5 + j + 16*i, buf);
    }
}

int PsService::isSameTargetOperator(int targetOperator)
{
    int nTargetOperator = 999;
    RilProperty* appProperty = GetRilApplicationProperty();
    if ( appProperty != NULL ) {
        nTargetOperator = appProperty->GetInt(RIL_APP_TARGET_OPER);
    }
    if ( targetOperator == nTargetOperator ) return true;
    return false;
}

bool PsService::isRoamState()
{
    int dataNetworkState = GetDataNetworkState();

    if(dataNetworkState == SIT_NET_REG_STATE_ROAMING ||
       dataNetworkState == SIT_NET_REG_STATE_DENIED_ROAMING)
    {
        return true;
    }
    return false;
}

int PsService::DoSetPreferredDataModem(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (IsNullRequest(pMsg)) return -1;

    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    int dataModemId = rildata->GetInt();

    if( dataModemId < 0 || dataModemId > 2) {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        return 0;
    }

    ProtocolPsBuilder builder;
    ModemData *pModemData = builder.BuildSetPreferredDataModem(dataModemId);
    if (SendRequest(pModemData, TIMEOUT_DEFAULT_PS, MSG_PS_SET_PREFERRED_DATA_MODEM_DONE) < 0) {
        return -1;
    }
    return 0;
}

int PsService::OnSetPreferredDataModemDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (IsNullResponse(pMsg)) return -1;

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    } else {
        OnRequestComplete(errorCode, NULL, 0);
    }

    return 0;
}

bool PsService::IsNullRequest(Message *pMsg)
{
    if(NULL == pMsg || NULL == pMsg->GetRequestData())
    {
        RilLogE("[PsService] pMsg is NULL or reqeust data is NULL");
        return 1;
    }
    return 0;
}

bool PsService::IsNullResponse(Message *pMsg)
{
    if(NULL == pMsg || NULL == pMsg->GetModemData())
    {
        RilLogE("[PsService] pMsg is NULL or modemdata data is NULL");
        return true;
    }
    return false;
}

void PsService::notifyAMBR(const DataCall *out)
{
    // Send AMBR through OEM RIL INDICATION
    // struct AMBR == sit_pdp_data_call_item_ext_ambr
    RilLogV("notify AMBR to framework : CID:%d, len:%d", out->cid, out->ambr.octet2);
    if(out->ambr.octet2 != 0)
        OnUnsolicitedResponse(RIL_UNSOL_OEM_AMBR, (void *)&out->ambr, sizeof(AMBR));
}

void PsService::decodeAMBR(const DataCall *out)
{
    int up = 0, down = 0;
    // Down, Evaluate first octet 3, 5
    switch(out->ambr.octet2)
    {
      case 6:
        // Use Extended value, ignore octet3,4
        if(out->ambr.octet5 != 0x00) {
            if( out->ambr.octet5 <= 0x4a) {
                down = 8600 + out->ambr.octet5 * 100;
            } else if( out->ambr.octet5 <= 0xba) {
                down = 16000 + (out->ambr.octet5 - 0x4a) * 1000;
            } else if( out->ambr.octet5 <= 0xfa) {
                down = 128000 + (out->ambr.octet5 - 0xba) * 2000;
            } else {
                down = 128000 + 0xfa * 2000;
            }
            break;
        }
        [[fallthrough]];
      case 4:
        if ( out->ambr.octet3 <= 0x3f ) {
            down = out->ambr.octet3;
        } else if( out->ambr.octet3 <= 0x7f ) {
            down = 64 + (out->ambr.octet3 - 0x40) * 8;
        } else if( out->ambr.octet3 <= 0xfe ) {
            down = 576 + (out->ambr.octet3 - 0x80) * 64;
        } else { // 0xFF
            down = 0;
        }
        break;
    }
    // Up
    switch(out->ambr.octet2) {
      case 6:
        if ( out->ambr.octet6 != 0x00 ) {
            if( out->ambr.octet6 <= 0x4a ) {
                up = 8600 + out->ambr.octet6 * 100;
            } else if( out->ambr.octet6 <= 0xba ) {
                up = 16000 + (out->ambr.octet6 - 0x4a) * 1000;
            } else if( out->ambr.octet6 <= 0xfa ) {
                up = 128000 + (out->ambr.octet6 - 0xba) * 2000;
            } else {
                up = 128000 + 0xfa * 2000;
            }
            break;
        }
        [[fallthrough]];
      case 4:
        if ( out->ambr.octet4 <= 0x3f ) {
            up = out->ambr.octet4;
        } else if( out->ambr.octet4 <= 0x7f ) {
            up = 64 + (out->ambr.octet4 - 0x40) * 8;
        } else if( out->ambr.octet4 <= 0xfe ) {
            up = 576 + (out->ambr.octet4 - 0x80) * 64;
        } else { // 0xFF
            up = 0;
        }
        break;
    }
    // Down octet 7, Up octet 8
    if (out->ambr.octet2 == 8){
        if ( out->ambr.octet7 > 0x00 && out->ambr.octet7 < 0xff ) {
            down += out->ambr.octet7 * 256000;
        }
        if ( out->ambr.octet8 > 0x00 && out->ambr.octet8 < 0xff ) {
            up += out->ambr.octet8 * 256000;
        }
    }

    RilLogV("CID:%d, Downlink %d kbps, Uplink %d kbps", out->cid, down, up);
}

bool PsService::isRejectRatForIMS(int rat)
{
    switch(rat) {
    case RADIO_TECH_GPRS:
        return true;
    default:
        return false;
    }
    return false;
}
