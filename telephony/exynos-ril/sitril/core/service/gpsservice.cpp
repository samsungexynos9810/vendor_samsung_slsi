/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "gpsservice.h"
#include "servicemonitorrunnable.h"
#include "rillog.h"
#include "protocolgpsadapter.h"
#include "protocolgpsbuilder.h"
#include "gpsdatabuilder.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define ENTER_FUNC()        { RilLogI("%s::%s() [<-- ", m_szSvcName, __FUNCTION__); }
#define LEAVE_FUNC()        { RilLogI("%s::%s() [--> ", m_szSvcName, __FUNCTION__); }
#define NULL_REQ(msg)        { if(msg==NULL || msg->GetRequestData()==NULL) { RilLogE("%s::%s() RequestData = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define PARAM_NULL(msg)        { if(msg==NULL) { RilLogE("%s::%s() Parameter = NULL", m_szSvcName, __FUNCTION__); return -1; } }

/* #### Definition for Debugging Logs ####
#define ENABLE_ANDROID_LOG

#define LogE    RilLogE
#define LogW    RilLogW
#define LogN    RilLogI
#ifdef ENABLE_ANDROID_LOG
#define LogI    RilLogI
#define LogV    RilLogV
#endif // end  of ENABLE_ANDROID_LOG
*/

GpsService::GpsService(RilContext* pRilContext)
: Service(pRilContext, RIL_SERVICE_GPS)
{
    ENTER_FUNC();
    strcpy(m_szSvcName, "GpsService");
    LEAVE_FUNC();
}

GpsService::~GpsService()
{
    ENTER_FUNC();
    LEAVE_FUNC();
}

int GpsService::OnCreate(RilContext *pRilContext)
{
    ENTER_FUNC();
    LEAVE_FUNC();
    return 0;
}

BOOL GpsService::OnHandleRequest(Message* pMsg)
{
    ENTER_FUNC();

    INT32 nRet = -1;
    if(NULL == pMsg)
        return FALSE;

    switch (pMsg->GetMsgId())
    {
        case MSG_GPS_SET_FREQUENCY_AIDING:
        case MSG_GPS_GET_LPP_SUPL_REQ_ECID_INFO:
        case MSG_GPS_GET_RRLP_SUPL_REQ_ECID_INFO:
        case MSG_GPS_MO_LOCATION_REQUEST:
        case MSG_GPS_GET_LPP_REQ_SERVING_CELL_INFO:
        case MSG_GPS_SET_SUPL_NI_READY:
        case MSG_GPS_GET_GSM_EXT_INFO_MSG:
        case MSG_GPS_CONTROL_PLANE_ENABLE:
        case MSG_GPS_GNSS_LPP_PROFILE_SET:
        // CDMA & HEDGE GANSS
        case MSG_GPS_SET_GANSS_MEAS_POS_RSP:
        case MSG_GPS_SET_GPS_LOCK_MODE:
        case MSG_GPS_GET_REFERENCE_LOCATION:
        case MSG_GPS_SET_PSEUDO_RANGE_MEASUREMENTS:
        case MSG_GPS_GET_CDMA_PRECISE_TIME_AIDING_INFO:
        case MSG_GPS_CDMA_FREQ_AIDING:
            nRet = DoAgpsDefaultRequestHandler(pMsg);
            break;

        /* Indication from AP, No resp */
        case MSG_GPS_IND_MEASURE_POS_RSP:
        case MSG_GPS_IND_RELEASE_GPS:
        case MSG_GPS_IND_MT_LOCATION_REQUEST:
        case MSG_GPS_LPP_PROVIDE_CAPABILITIES_IND:
        case MSG_GPS_IND_LPP_REQUEST_ASSIST_DATA:
        case MSG_GPS_LPP_PROVIDE_LOCATION_INFO_IND:
        case MSG_GPS_LPP_GPS_ERROR_IND:
        case MSG_GPS_IND_SUPL_LPP_DATA_INFO:
        case MSG_GPS_IND_SUPL_NI_MESSAGE:
        case MSG_GPS_RETRIEVE_LOC_INFO:
        // CDMA & HEDGE GANSS
        case MSG_GPS_IND_GANSS_AP_POS_CAP_RSP:
            nRet = DoAgpsDefaultRequestIndHandler(pMsg);
            break;

        default:
            RilLogV("%s::%s() Unknown nMsgId = %d", m_szSvcName, __FUNCTION__, pMsg->GetMsgId());
            nRet = -3;
            break;
    }

    LEAVE_FUNC();

    if(0 == nRet)
        return TRUE;
    else
        return FALSE;
}
BOOL GpsService::OnHandleSolicitedResponse(Message* pMsg)
{
    ENTER_FUNC();
    INT32 nRet = -1;

    if(NULL == pMsg)
        return FALSE;

    switch(pMsg->GetMsgId())
    {
        case MSG_GPS_SET_FREQUENCY_AIDING_DONE:
        case MSG_GPS_GET_LPP_SUPL_REQ_ECID_INFO_DONE:
        case MSG_GPS_GET_RRLP_SUPL_REQ_ECID_INFO_DONE:
        case MSG_GPS_MO_LOCATION_REQUEST_DONE:
        case MSG_GPS_GET_LPP_REQ_SERVING_CELL_INFO_DONE:
        case MSG_GPS_SET_SUPL_NI_READY_DONE:
        case MSG_GPS_GET_GSM_EXT_INFO_MSG_DONE:
        case MSG_GPS_CONTROL_PLANE_ENABLE_DONE:
        case MSG_GPS_GNSS_LPP_PROFILE_SET_DONE:
        // CDMA & HEDGE GANSS
        case MSG_GPS_SET_GANSS_MEAS_POS_RSP_DONE:
        case MSG_GPS_SET_GPS_LOCK_MODE_DONE:
        case MSG_GPS_GET_REFERENCE_LOCATION_DONE:
        case MSG_GPS_SET_PSEUDO_RANGE_MEASUREMENTS_DONE:
        case MSG_GPS_GET_CDMA_PRECISE_TIME_AIDING_INFO_DONE:
        case MSG_GPS_CDMA_FREQ_AIDING_DONE:
            nRet = OnAgpsDefaultResponseHandler(pMsg);
            break;

        default:
            RilLogV("%s::%s() Unknown nMsgId = %d", m_szSvcName, __FUNCTION__, pMsg->GetMsgId());
            nRet = -3;
            break;
    }

    LEAVE_FUNC();

    if(0 == nRet)
        return TRUE;
    else
        return FALSE;

}

BOOL GpsService::OnHandleUnsolicitedResponse(Message* pMsg)
{
    ENTER_FUNC();
    INT32 nRet = -1;

    if(NULL == pMsg)
        return FALSE;

    switch (pMsg->GetMsgId())
    {
        case MSG_GPS_IND_MEASURE_POS_REQ:
        case MSG_GPS_IND_ASSIST_DATA:
        case MSG_GPS_IND_RELEASE_GPS:
        case MSG_GPS_IND_MT_LOCATION_REQUEST:
        case MSG_GPS_IND_RESET_GPS_ASSIST_DATA:
        case MSG_GPS_IND_LPP_REQUEST_CAPABILITIES:
        case MSG_GPS_IND_LPP_PROVIDE_ASSIST_DATA:
        case MSG_GPS_IND_LPP_REQUEST_LOCATION_INFO:
        case MSG_GPS_LPP_GPS_ERROR_IND:
        case MSG_GPS_IND_SUPL_LPP_DATA_INFO:
        case MSG_GPS_IND_SUPL_NI_MESSAGE:
        case MSG_GPS_SET_SUPL_NI_READY:
        case MSG_GPS_START_MDT_LOC:
        case MSG_GPS_IND_LPP_UPDATE_UE_LOC_INFO:
        case MSG_GPS_IND_GPS_LOCK_MODE:

        // CDMA & HEDGE GANSS
        case MSG_GPS_IND_3GPP_SEND_GANSS_ASSIT_DATA:
        case MSG_GPS_IND_GANSS_MEAS_POS_MSG:
        case MSG_GPS_IND_CDMA_GPS_POWER_ON:
        case MSG_GPS_IND_CDMA_SEND_ACQUSITION_ASSIT_DATA:
        case MSG_GPS_IND_CDMA_SESSION_CANCELLATION:
        case MSG_GPS_IND_GANSS_AP_POS_CAP_REQ:
            nRet = OnAgpsDefaultUnsolRespHandler(pMsg);
            break;

        default:
            RilLogV("%s::%s() Unknown nMsgId = %d", m_szSvcName, __FUNCTION__, pMsg->GetMsgId());
            nRet = -3;
            break;
    }

    LEAVE_FUNC();

    if(0 == nRet)
        return TRUE;
    else
        return FALSE;
}

/**
 * DoAgpsDefaultRequestHandler
 * @desc default AGPS Request handler
 */
int GpsService::DoAgpsDefaultRequestHandler(Message *pMsg)
{

    RilLogI("[%s] Process AGPS Request", GetServiceName());
    NULL_REQ(pMsg);

    RawRequestData *rildata =(RawRequestData *)(pMsg->GetRequestData());
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    RilLogV("[%s] AGPS Request: msgId=%d requestId=%d parameter=0x%p parameter length=%d",
            GetServiceName(), pMsg->GetMsgId(), rildata->GetReqId(), rildata->GetRawData(), rildata->GetSize());

    ProtocolGpsBuilder builder;
    ModemData *pModemData = builder.BuildAgpsPDU(rildata->GetReqId(), rildata->GetRawData(), rildata->GetSize());
    if (SendRequest(pModemData, TIMEOUT_GPS_DEFAULT, pMsg->GetMsgId() + 1) < 0) {
        RilLogW("[%s] Failed to send AGPS PDU", GetServiceName());
        return -1;
    }

    return 0;
}

/**
 * OnAgpsDefaultResponseHandler
 * @desc default AGPS Response handler
 */
int GpsService::OnAgpsDefaultResponseHandler(Message *pMsg)
{
    RilLogI("[%s] Process AGPS Response", GetServiceName());
    PARAM_NULL(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    RilLogV("[%s] AGPS Response: msgId=%d errorCode=%d parameter=0x%p parameter length=%d",
            GetServiceName(), pMsg->GetMsgId(), errorCode, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

/**
 * DoAgpsDefaultRequestIndHandler
 * @desc default AGPS Request(Ind) handler
 *           there's no response for this request. (indication from AP to CP)
 */
int GpsService::DoAgpsDefaultRequestIndHandler(Message *pMsg)
{
    RilLogV("[%s] Process AGPS Request(Ind)", GetServiceName());
    NULL_REQ(pMsg);

    RawRequestData *rildata =(RawRequestData *)(pMsg->GetRequestData());
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    RilLogV("[%s] AGPS Request(Ind): msgId=%d requestId=%d parameter=0x%p parameter length=%d",
            GetServiceName(), pMsg->GetMsgId(), rildata->GetReqId(), rildata->GetRawData(), rildata->GetSize());

    ProtocolGpsBuilder builder;
    ModemData *pModemData = builder.BuildAgpsIndPDU(rildata->GetReqId(), rildata->GetRawData(), rildata->GetSize());
    if (SendRequest(pModemData) < 0) {
        RilLogW("[%s] Failed to send AGPS Ind PDU", GetServiceName());
        delete pModemData;
        return -1;
    } else {
        delete pModemData;
    }

    OnRequestComplete(RIL_E_SUCCESS);

    return 0;
}

/**
 * OnAgpsDefaultUnsolRespHandler
 * @desc default AGPS Unsolicited Response handler
 */
int GpsService::OnAgpsDefaultUnsolRespHandler(Message *pMsg)
{
    RilLogV("[%s] Process AGPS Indication", GetServiceName());
    PARAM_NULL(pMsg);

    ProtocolAgpsIndAdapter adapter(pMsg->GetModemData());
    int resultId = adapter.GetResultId();
    RilLogV("[%s] AGPS Indication: msgId=%d resultId=%d parameter=0x%p parameter length=%d",
            GetServiceName(), pMsg->GetMsgId(), resultId, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());

    if (resultId != -1) {
        OnUnsolicitedResponse(resultId, adapter.GetParameter(), adapter.GetParameterLength());
    }
    else {
        RilLogW("[%s] Invalid or undefined protocolId=%d", adapter.GetId());
    }

    return 0;
}
