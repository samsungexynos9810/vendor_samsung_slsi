/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <arpa/inet.h>

#include "embmsservice.h"
#include "servicemonitorrunnable.h"
#include "rillog.h"
#include "protocolembmsadapter.h"
#include "protocolembmsbuilder.h"
#include "embmsdata.h"
#include "embmsdatabuilder.h"
#include "miscdatabuilder.h"
#include "protocolnetbuilder.h"
#include "protocolnetadapter.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define NULL_REQ(msg)        { if(msg==NULL || msg->GetRequestData()==NULL) { RilLogE("%s::%s() RequestData = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define NULL_RSP(msg)        { if(msg==NULL || msg->GetModemData()==NULL) { RilLogE("%s::%s() ModemData = NULL", m_szSvcName, __FUNCTION__); return -1; } }

#define ENTER_FUNC()        { RilLogI("%s::%s() [<-- ", m_szSvcName, __FUNCTION__); }
#define LEAVE_FUNC()        { RilLogI("%s::%s() [--> ", m_szSvcName, __FUNCTION__); }

#define LOGI(format, ...)        RilLogI("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)
#define LOGV(format, ...)        RilLogV("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)
#undef LOGD
#define LOGD(format, ...)        RilLog("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)
#define LOGE(format, ...)        RilLogE("%s::%s() " format, m_szSvcName, __FUNCTION__, ##__VA_ARGS__)

#define EMBMS_CID_BASE      (30)
#define EMBMS_IPADDR_BASE   (5)

EmbmsService::EmbmsService(RilContext* pRilContext)
: Service(pRilContext, RIL_SERVICE_EMBMS)
{
    ENTER_FUNC();
    strcpy(m_szSvcName, "EmbmsService");
    m_PdpContext = NULL;
    mCardState = RIL_CARDSTATE_ABSENT;
    LEAVE_FUNC();
}

EmbmsService::~EmbmsService()
{
    ENTER_FUNC();
    LEAVE_FUNC();
}

int EmbmsService::OnCreate(RilContext *pRilContext)
{
    ENTER_FUNC();
    LEAVE_FUNC();
    return 0;
}

BOOL EmbmsService::OnHandleRequest(Message* pMsg)
{
    if(NULL == pMsg)
        return FALSE;

    int ret = -1;
    switch (pMsg->GetMsgId())
    {
        case MSG_EMBMS_ENABLE_SERVICE:
            ret = DoEnableService(pMsg);
            break;
        case MSG_EMBMS_DISABLE_SERVICE:
            ret = DoDisableService(pMsg);
            break;
        case MSG_EMBMS_SET_SESSION:
            ret = DoSetSession(pMsg);
            break;
        case MSG_EMBMS_GET_SESSION_LIST:
            ret = DoGetSessionList(pMsg);
            break;
        case MSG_EMBMS_GET_SIGNAL_STRENGTH:
            ret = DoGetSignalStrength(pMsg);
            break;
        case MSG_EMBMS_GET_NETWORK_TIME:
            ret = DoGetNetworkTime(pMsg);
            break;
        case MSG_EMBMS_CHECK_AVAIABLE_EMBMS:
            ret = DoCheckAvaiableEmbms(pMsg);
            break;
        default:
            LOGV("Unknown nMsgId = %d", pMsg->GetMsgId());
            ret = -3;
            break;
    }

    return (ret < 0 ? FALSE : TRUE);
}

BOOL EmbmsService::OnHandleSolicitedResponse(Message* pMsg)
{
    if(NULL == pMsg)
        return FALSE;

    int ret = -1;
    switch(pMsg->GetMsgId())
    {
        case MSG_EMBMS_ENABLE_SERVICE_DONE:
            ret = OnEnableServiceDone(pMsg);
            break;
        case MSG_EMBMS_DISABLE_SERVICE_DONE:
            ret = OnDisableServiceDone(pMsg);
            break;
        case MSG_EMBMS_SET_SESSION_DONE:
            ret = OnSetSessionDone(pMsg);
            break;
        case MSG_EMBMS_GET_SESSION_LIST_DONE:
            ret = OnGetSessionListDone(pMsg);
            break;
        case MSG_EMBMS_GET_SIGNAL_STRENGTH_DONE:
            ret = OnGetSignalStrengthDone(pMsg);
            break;
        case MSG_EMBMS_GET_NETWORK_TIME_DONE:
            ret = OnGetNetworkTimeDone(pMsg);
            break;
        case MSG_EMBMS_CHECK_AVAIABLE_EMBMS_DONE:
            ret = OnCheckAvaiableEmbmsDone(pMsg);
            break;
        default:
            LOGV("Unknown nMsgId = %d", pMsg->GetMsgId());
            ret = -3;
            break;
    }

    return (ret < 0 ? FALSE : TRUE);
}

BOOL EmbmsService::OnHandleUnsolicitedResponse(Message* pMsg)
{
    if(NULL == pMsg)
        return FALSE;

    int ret = -1;
    switch (pMsg->GetMsgId())
    {
        case MSG_EMBMS_UNSOL_SESSION_LIST_UPDATE:
            ret = OnSessionListUpdate(pMsg);
            break;
        case MSG_EMBMS_UNSOL_COVERAGE:
            ret = OnCoverage(pMsg);
            break;
        case MSG_EMBMS_UNSOL_SIGNAL_STRENGTH:
            ret = OnSignalStrength(pMsg);
            break;
        case MSG_EMBMS_UNSOL_NETWORK_TIME:
            ret = OnNetworkTime(pMsg);
            break;
        case MSG_EMBMS_UNSOL_SAI_LIST:
            ret = OnSaiList(pMsg);
            break;
        case MSG_EMBMS_UNSOL_GLOBAL_CELL_ID:
            ret = OnGlobalCellId(pMsg);
            break;
        default:
            LOGV("Unknown nMsgId = %d", pMsg->GetMsgId());
            ret = -3;
            break;
    }

    return (ret < 0 ? FALSE : TRUE);
}

int EmbmsService::DoCheckAvaiableEmbms(Message *pMsg)
{
    ENTER_FUNC();

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetPreferredNetworkType();
    if (SendRequest(pModemData, TIMEOUT_EMBMS_DEFAULT, MSG_EMBMS_CHECK_AVAIABLE_EMBMS_DONE) < 0) {
        return -1;
    }

    LEAVE_FUNC();
    return 0;
}

int EmbmsService::OnCheckAvaiableEmbmsDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolNetPrefNetTypeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int netType = adapter.GetPreferredNetworkType();
        if (netType == PREF_NET_TYPE_LTE_CDMA_EVDO || netType == PREF_NET_TYPE_LTE_GSM_WCDMA
                || netType == PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA || netType == PREF_NET_TYPE_LTE_ONLY
                || netType == PREF_NET_TYPE_LTE_WCDMA) {
            if (mCardState == RIL_CARDSTATE_ABSENT) {
                OnRequestComplete(RIL_E_GENERIC_FAILURE);
            } else {
                OnRequestComplete(RIL_E_SUCCESS);
            }
        } else {
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }
    } else {
        OnRequestComplete(errorCode);
    }

    LEAVE_FUNC();
    return 0;
}

int EmbmsService::DoEnableService(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);
    int nResult = -1;

    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int state = rildata->GetInt();

    ProtocolEmbmsBuilder builder;
    ModemData *pModemData = builder.BuildSetService(state);
    nResult = SendRequest(pModemData, TIMEOUT_EMBMS_DEFAULT, MSG_EMBMS_ENABLE_SERVICE_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int EmbmsService::OnEnableServiceDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);
    BOOL bSendUnsol = false;

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        m_PdpContext = new PdpContext(EMBMS_CID_BASE+GetRilSocketId(), "embms", GetRilSocketId());
        if (m_PdpContext != NULL) {
            m_PdpContext->Init();
        }

        DataCall *dc = m_PdpContext->GetDataCallInfo();
        dc->cid = EMBMS_CID_BASE+GetRilSocketId();
        dc->active = ACTIVE_AND_LINKUP;

        char ip_addr[40] = {0,};
        memset(ip_addr, 0x00, sizeof(ip_addr));

        snprintf(ip_addr, sizeof(ip_addr)-1, "169.254.0.%d", EMBMS_IPADDR_BASE + GetRilSocketId());
        int err = inet_pton(AF_INET, ip_addr, (void *)&dc->ipv4.addr);
        if (err != 1) LOGE("inet_pton for EMBMS AF_INET failed: err:%d", err);
        dc->ipv4.valid = true;

        snprintf(ip_addr, sizeof(ip_addr)-1, "fe80:4:6c:8c74:0:5efe:6dcd:8c7%d", EMBMS_IPADDR_BASE + GetRilSocketId());
        err = inet_pton(AF_INET6, ip_addr, (void *)&dc->ipv6.addr);
        if (err != 1) LOGE("inet_pton for EMBMS AF_INET6 failed: err:%d", err);
        dc->ipv6.valid = true;
        if (m_PdpContext->OnActivated(dc) != 0) {
            LOGI("OnActivated FAIL");
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        } else {
            LOGI("OnActivated SUCCESS");
            OnRequestComplete(RIL_E_SUCCESS);
            bSendUnsol = true;
        }
    } else {
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }

    if (bSendUnsol) {
        int modemStatus = MODEM_STATUS_EMBMS_ENABLE;
        OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_MODEM_STATUS, &modemStatus, sizeof(int));
    }

    LEAVE_FUNC();
    return 0;
}

int EmbmsService::DoDisableService(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);
    int nResult = -1;

    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        LOGE("rildata is NULL");
        return -1;
    }

    int state = rildata->GetInt();

    ProtocolEmbmsBuilder builder;
    ModemData *pModemData = builder.BuildSetService(state);
    nResult = SendRequest(pModemData, TIMEOUT_EMBMS_DEFAULT, MSG_EMBMS_DISABLE_SERVICE_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int EmbmsService::OnDisableServiceDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if ( errorCode == RIL_E_SUCCESS ) {
        if (m_PdpContext != NULL) {
            if (m_PdpContext->OnDeactivated() != 0) {
                LOGI("OnDeactivated FAIL");
                OnRequestComplete(RIL_E_GENERIC_FAILURE);
            } else {
                LOGI("OnDeactivated SUCCESS");
                OnRequestComplete(RIL_E_SUCCESS);
            }

            m_PdpContext->Init();
            delete m_PdpContext;
            m_PdpContext = NULL;
        } else {
            LOGI("m_PdpContext is NULL - SUCCESS");
            OnRequestComplete(RIL_E_SUCCESS);
        }
    } else {
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }

    int modemStatus = MODEM_STATUS_EMBMS_DISABLE;
    OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_MODEM_STATUS, &modemStatus, sizeof(int));
    LEAVE_FUNC();
    return 0;
}

int EmbmsService::DoSetSession(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);
    int nResult = -1;

    EmbmsSessionData *rildata = (EmbmsSessionData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        LOGE("rildata is NULL");
        return -1;
    }

    int state = rildata->GetState();
    uint64_t tmgi = rildata->GetTmgi();
    RIL_InfoBinding InfoBind = rildata->GetInfoBind();

    ProtocolEmbmsBuilder builder;
    ModemData *pModemData = builder.BuildSetSession(state, tmgi, InfoBind.uSAICount, InfoBind.nSAIList, InfoBind.uFreqCount, InfoBind.nFreqList);
    nResult = SendRequest(pModemData, TIMEOUT_EMBMS_DEFAULT, MSG_EMBMS_SET_SESSION_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int EmbmsService::OnSetSessionDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    int result = -1;/* ERROR */
    ModemData *pModemData = pMsg->GetModemData();
    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    Message *pCurrMsg = GetCurrentMsg();
    EmbmsSessionData *rildata = (EmbmsSessionData *)pCurrMsg->GetRequestData();
    if (rildata == NULL) {
        LOGE("rildata is NULL");
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
        return -1;
    }

    if ( errorCode == RIL_E_SUCCESS ) {
        result = 0; /* SUCCESS */
    }

    EmbmsDataBuilder builder;
    uint32_t state = rildata->GetState();
    const RilData *pRilData = builder.BuildEmbmsSessionControlIndicate(!state, result, rildata->GetTmgi());
    if (pRilData == NULL) {
        LOGE("pRilData is NULL");
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
        return -1;
    }

    if ( errorCode == RIL_E_SUCCESS ) {
        OnRequestComplete(RIL_E_SUCCESS);
    } else {
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }

    OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_SESSION_CONTROL, pRilData->GetData(), pRilData->GetDataLength());
    LEAVE_FUNC();
    return 0;

}

int EmbmsService::DoGetSessionList(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;
    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        LOGE("rildata is NULL");
        return -1;
    }

    int state = rildata->GetInt();

    ProtocolEmbmsBuilder builder;
    ModemData *pModemData = builder.BuildGetSessionList(state);
    nResult = SendRequest(pModemData, TIMEOUT_EMBMS_DEFAULT, MSG_EMBMS_GET_SESSION_LIST_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int EmbmsService::OnGetSessionListDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolEmbmsSessionListAdapter adapter(pMsg->GetModemData());
    UINT errorCode = adapter.GetErrorCode();
    if ( errorCode == RIL_E_SUCCESS ) {
        EmbmsDataBuilder builder;
        const RilData *pRilData = builder.BuildEmbmsSessionListResponse(adapter.GetState(), adapter.GetOosReason(), adapter.GetRecordNum(), adapter.GetTMGI());
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    } else {
        OnRequestComplete(errorCode == RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }

    LEAVE_FUNC();
    return 0;
}

int EmbmsService::OnSessionListUpdate(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolEmbmsSessionListIndAdapter adapter(pMsg->GetModemData());

    EmbmsDataBuilder builder;
    const RilData *pRilData = builder.BuildEmbmsSessionListResponse(adapter.GetState(), adapter.GetOosReason(), adapter.GetRecordNum(), adapter.GetTMGI());

    OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_SESSION_LIST, pRilData->GetData(), pRilData->GetDataLength());
    delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int EmbmsService::DoGetSignalStrength(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);
    int nResult = -1;

    ProtocolEmbmsBuilder builder;
    ModemData *pModemData = builder.BuildSignalStrength();
    nResult = SendRequest(pModemData, TIMEOUT_EMBMS_DEFAULT, MSG_EMBMS_GET_SIGNAL_STRENGTH_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int EmbmsService::OnGetSignalStrengthDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolEmbmsSignalStrengthAdapter adapter(pMsg->GetModemData());
    EmbmsDataBuilder builder;
    const RilData *pRilData = builder.BuildEmbmsSignalStrengthResponse(adapter.GetCount(), adapter.GetSnrList());

    UINT errorCode = adapter.GetErrorCode();
    if ( errorCode == RIL_E_SUCCESS )
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
    else
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);

    delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int EmbmsService::OnSignalStrength(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolEmbmsSignalStrengthAdapter adapter(pMsg->GetModemData());
    EmbmsDataBuilder builder;
    const RilData *pRilData = builder.BuildEmbmsSignalStrengthResponse(adapter.GetCount(), adapter.GetSnrList());

    OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_SIGNAL_STRENGTH, pRilData->GetData(), pRilData->GetDataLength());
    delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int EmbmsService::DoGetNetworkTime(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);
    int nResult = -1;

    ProtocolEmbmsBuilder builder;
    ModemData *pModemData = builder.BuildNetworkTime();
    nResult = SendRequest(pModemData, TIMEOUT_EMBMS_DEFAULT, MSG_EMBMS_GET_NETWORK_TIME_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

int EmbmsService::OnGetNetworkTimeDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolEmbmsNetworkTimeAdapter adapter(pMsg->GetModemData());

    int daylightvalid = adapter.DayLightValid();
    int year = adapter.Year();
    int month = adapter.Month();
    int day = adapter.Day();
    int hour = adapter.Hour();
    int minute = adapter.Minute();
    int second = adapter.Second();
    int timezone = adapter.TimeZone();
    int daylightadjust = adapter.DayLightAdjust();
    int dayofweek = adapter.DayofWeek();

    EmbmsDataBuilder builder;
    const RilData *pRilData = builder.BuildEmbmsNetworkTimeResponse(adapter.GetNetworkTime());
    if (pRilData == NULL) {
        LOGE("pRilData is NULL");
        LEAVE_FUNC();
        return -1;
    }

    string prop = SystemProperty::Get("persist.vendor.radio.embmstest_enabled", "0");
    LOGI("prop = %s", prop.c_str());
    if (TextUtils::Equals(prop, "1")) {
        LOGI("NITZ : %4d-%02d-%02d, %02d:%02d:%02d", year, month, day, hour, minute, second);
        MiscDataBuilder nitz_builder;
        const RilData *nitz_rildata = nitz_builder.BuildNitzTimeIndication(daylightvalid, year, month,
                day, hour, minute, second, timezone, daylightadjust, dayofweek );
        if (nitz_rildata != NULL){
            OnUnsolicitedResponse(RIL_UNSOL_NITZ_TIME_RECEIVED, nitz_rildata->GetData(), nitz_rildata->GetDataLength());
            delete nitz_rildata;
        }

        usleep(500000);
    }

    UINT errorCode = adapter.GetErrorCode();
    if ( errorCode == RIL_E_SUCCESS ) {
        OnRequestComplete(RIL_E_SUCCESS);
        OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_NETWORK_TIME, pRilData->GetData(), pRilData->GetDataLength());
    } else {
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
        uint64_t NoNetworktime = NO_NETWORK_TIME_AVAILABLE;
        OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_NETWORK_TIME, &NoNetworktime, sizeof(uint64_t));
    }

    delete pRilData;
    LEAVE_FUNC();
    return 0;
}

int EmbmsService::OnNetworkTime(Message *pMsg)
{
/*
    ENTER_FUNC();
    NULL_RSP(pMsg);


    ProtocolEmbmsNetworkTimeAdapter adapter(pMsg->GetModemData());
    EmbmsDataBuilder builder;
    const RilData *pRilData = builder.BuildEmbmsNetworkTimeResponse(adapter.GetNetworkTime());

    OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_NETWORK_TIME, pRilData->GetData(), pRilData->GetDataLength());
    delete pRilData;

    LEAVE_FUNC();
*/
    return 0;
}

int EmbmsService::OnCoverage(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolEmbmsCoverageAdapter adapter(pMsg->GetModemData());
    EmbmsDataBuilder builder;
    const RilData *pRilData = builder.BuildEmbmsCoverageIndicate(adapter.GetCoverage());

    OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_COVERAGE, pRilData->GetData(), pRilData->GetDataLength());
    delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int EmbmsService::OnSaiList(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolEmbmsSaiListAdapter adapter(pMsg->GetModemData());
    EmbmsDataBuilder builder;
    const RilData *pRilData = builder.BuildEmbmsSaiIndicate(adapter.GetIntraSaiListLen(), adapter.GetInterSaiListLen(), adapter.GetIntraSaiList(), adapter.GetInterSaiList());

    OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_SAI_LIST, pRilData->GetData(), pRilData->GetDataLength());
    delete pRilData;

    LEAVE_FUNC();
    return 0;
}

int EmbmsService::OnGlobalCellId(Message *pMsg)
{
    ENTER_FUNC();
    NULL_RSP(pMsg);

    ProtocolEmbmsGlobalCellIdAdapter adapter(pMsg->GetModemData());
    EmbmsDataBuilder builder;
    const RilData *pRilData = builder.BuildEmbmsGlobalCellIdIndicate(adapter.GetMcc(), adapter.GetMnc(), adapter.GetCellId());

    OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_GLOBAL_CELL_ID, pRilData->GetData(), pRilData->GetDataLength());
    delete pRilData;

    LEAVE_FUNC();
    return 0;
}

void EmbmsService::OnRadioStateChanged(int radioState)
{
    ENTER_FUNC();
    OnUnsolicitedResponse(RIL_UNSOL_OEM_EMBMS_RADIO_STATE_CHANGED, &radioState, sizeof(int));
    LEAVE_FUNC();
    return;
}

void EmbmsService::OnSimStatusChanged(int cardState, int appState)
{
    mCardState = cardState;
    return;
}
