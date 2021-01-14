/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "imsservice.h"
#include "servicemonitorrunnable.h"
#include "rillog.h"
#include "protocolimsbuilder.h"
#include "protocolimsadapter.h"
#include "protocolbuilder.h"
#include "rildatabuilder.h"
#include "imsdatabuilder.h"
#include "customproductfeature.h"

#include "simdata.h"
#include "simdatabuilder.h"
#include "protocolsimbuilder.h"
#include "protocolsimadapter.h"
#include "protocolcallbuilder.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define PARAM_NULL(msg)        { if(msg==NULL) { RilLogE("%s::%s() Parameter = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define NULL_REQ(msg)        { if(msg==NULL || msg->GetRequestData()==NULL) { RilLogE("%s::%s() RequestData = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define NULL_RSP(msg)        { if(msg==NULL || msg->GetModemData()==NULL) { RilLogE("%s::%s() ModemData = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define ENTER_FUNC()        { RilLogI("%s::%s() [<-- ", m_szSvcName, __FUNCTION__); }
#define LEAVE_FUNC()        { RilLogI("%s::%s() [--> ", m_szSvcName, __FUNCTION__); }
#define NOT_IMPLEMENT()        { RilLogE("%s::%s() Not Implemented", m_szSvcName, __FUNCTION__); }

// #### Definition for Debugging Logs ####
//#define ENABLE_LOGS_FUNC_ENTER_EXIT
#define ENABLE_ANDROID_LOG

#define LogE    RilLogE
#define LogW    RilLogW
#define LogN    RilLogI
#ifdef ENABLE_ANDROID_LOG
#define LogI    RilLogI
#define LogV    RilLogV
#endif // end of ENABLE_ANDROID_LOG

#define PROPERTY_IMS_APP_TYPE "vendor.ril.context.sim.ims_app_type"
#define IMS_AIMS_REQUEST_TIMEOUT   180000

// #### Internal Done Functions ####
INT32 ImsService::DoXXXDone(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    do
    {
        // Parsing Params from Modem
        ModemData *pModemData = pMsg->GetModemData();
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail GetModemData()",m_szSvcName, __FUNCTION__);
            nResult = -2;
            break;
        }

        ProtocolRespAdapter adapter(pModemData);
        INT32 errorCode = adapter.GetErrorCode();
        if(errorCode != RIL_E_SUCCESS)
        {
            RilLogE("%s::%s() !! ERROR !!, errorCode(0x%x) in GetErrorCode()",m_szSvcName, __FUNCTION__,errorCode);

            // Complete Request for Error
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
            nResult = -3;
            break;
        }

        // Complete Request for Success
        OnRequestComplete( RIL_E_SUCCESS);

        RilLogV("%s::%s() Done ...Success",m_szSvcName, __FUNCTION__);
        nResult= 0;
    }while(0);

    LEAVE_FUNC();
    return nResult;
}



// #### Internal Set Functions ####
INT32 ImsService::DoSetConfig(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    const char *ImsData = (char *)rildata->GetRawData();
    if ( ImsData != NULL )
    {
        BYTE ConfigSelection = ImsData[0];
        RilLogV("[CImsService::%s] Set Configuration : %d", __FUNCTION__, ConfigSelection);

        ProtocolImsBuilder builder;
        ModemData *pModemData = builder.BuildSetConfig(ImsData);
        nResult = SendRequest(pModemData, IMS_DEFAULT_TIMEOUT, MSG_IMS_SET_CONF_DONE);
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

INT32 ImsService::OnSetConfigDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolImsRespAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    if(uErrCode==RIL_E_SUCCESS)
    {
        mResp2Byte[0] = adapter.GetResult();
        RilLogV("[CImsService::%s] Result : %d", __FUNCTION__, adapter.GetResult());
        OnRequestComplete( RIL_E_SUCCESS, mResp2Byte, sizeof(BYTE));
    }
    else
    {
        OnRequestComplete(uErrCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }

    LEAVE_FUNC();
    return 0;
}

INT32 ImsService::DoSetEmergencyCallStatus(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    int nResult = -1;

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    const char *ImsData = (char *)rildata->GetRawData();

    if ( ImsData != NULL )
    {
        ProtocolImsBuilder builder;
        ModemData *pModemData = builder.BuildEmergencyCallStatus(ImsData[0], ImsData[1]);
        nResult = SendRequest(pModemData, IMS_DEFAULT_TIMEOUT, MSG_IMS_SET_ERERGENCY_CALL_STATUS_DONE);
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

INT32 ImsService::OnSetEmergencyCallStatusDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolRespAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();

    OnRequestComplete(uErrCode == RIL_E_SUCCESS ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE);

    LEAVE_FUNC();
    return 0;
}

INT32 ImsService::DoSetSrvccCallList(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);
    int nResult = -1;
    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }
    const char *SrvccCallData = (char *)rildata->GetRawData();
    RilLogV("[CImsService::%s] Set Srvcc Call List ", __FUNCTION__);
    if ( SrvccCallData != NULL )
    {
        ProtocolImsBuilder builder;
        ModemData *pModemData = builder.BuildSetSrvccCallList(SrvccCallData);
        nResult = SendRequest(pModemData, IMS_DEFAULT_TIMEOUT, MSG_IMS_SET_SRVCC_CALL_LIST_DONE);
    }
    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}
INT32 ImsService::OnSetSrvccCallListDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);
    ProtocolRespAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    OnRequestComplete(uErrCode == RIL_E_SUCCESS ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE);
    LEAVE_FUNC();
    return 0;
}


// #### Internal Get Functions ####
INT32 ImsService::DoGetConfig(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    int nResult = -1;

    RilLogI("[CImsService::%s] IMS Get Configuration ", __FUNCTION__);

    ProtocolImsBuilder builder;
    ModemData *pModemData = builder.BuildGetConfig();
    nResult = SendRequest(pModemData, IMS_DEFAULT_TIMEOUT, MSG_IMS_GET_CONF_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}
INT32 ImsService::OnGetConfigDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolImsGetConfRespAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    if(uErrCode==RIL_E_SUCCESS)
    {
        ImsDataBuilder builder;
        const RilData *pRilData = builder.BuildImsGetConfigResponse(adapter.GetParameter(), adapter.GetParameterLength());
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    }
    else
    {
        OnRequestComplete(uErrCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }

    LEAVE_FUNC();
    return 0;
}

INT32 ImsService::DoSimAuth(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    int nResult = -1;

    IsimAuth rildata;
    if(rildata.ParseWithAuthType((StringRequestData *) pMsg->GetRequestData())==FALSE
            || rildata.GetLength()==0 || rildata.GetAuth()==NULL) {
        LEAVE_FUNC();
        return -1;
    }

    ProtocolSimBuilder builder;
    ModemData *pModemData = builder.BuildSimGetIsimAuth(rildata.GetAuthType(), rildata.GetAuth(), rildata.GetLength());

    nResult = SendRequest(pModemData, IMS_DEFAULT_TIMEOUT, MSG_IMS_SIM_AUTH_DONE);

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

INT32 ImsService::OnSimAuthDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolRespAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    if(uErrCode==RIL_E_SUCCESS)
    {
        ProtocolSimGetSimAuthAdapter adapter(pMsg->GetModemData());
        SimDataBuilder builder;
        const RilData *pRilData = builder.BuildSimGetIsimAuthResponse(adapter.GetAuthLength(), adapter.GetAuth());
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    }
    else
    {
        OnRequestComplete(uErrCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }

    LEAVE_FUNC();
    return 0;
}

INT32 ImsService::DoGetGbaAuth(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    int nResult = -1;

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    const char *GbaAuthData = (char *)rildata->GetRawData();
    if (GbaAuthData != NULL)
    {
        ProtocolSimBuilder builder;
        ModemData *pModemData = builder.BuildSimGetGbaAuth(GbaAuthData);
        nResult = SendRequest(pModemData, IMS_DEFAULT_TIMEOUT, MSG_IMS_GET_GBA_AUTH_DONE);
    }

    LEAVE_FUNC();
    return (nResult<0)? -1: 0;
}

INT32 ImsService::OnGetGbaAuthDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolRespAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();

    if(uErrCode==RIL_E_SUCCESS)
    {
        ProtocolSimGetGbaAuthAdapter adapter(pMsg->GetModemData());
        SimDataBuilder builder;
        const RilData *pRilData = builder.BuildSimGetGbaAuthResponse(adapter.GetGbaAuthLength(), adapter.GetGbaAuth());
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    }
    else
    {
        OnRequestComplete(uErrCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }

    LEAVE_FUNC();
    return 0;
}

// #### Internal Indication Functions ####
INT32 ImsService::OnImsConfiguration(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    BYTE ImsConf[2];
    ProtocolIndAdapter adapter(pMsg->GetModemData());
    memcpy(ImsConf,adapter.GetParameter(), sizeof(ImsConf));

    OnUnsolicitedResponse(RIL_UNSOL_OEM_IMS_CONFIGURATION, (void *)ImsConf, sizeof(ImsConf));

    LEAVE_FUNC();
    return 0;
}

INT32 ImsService::OnImsDedicatedPdnInfo(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    BYTE ImsConf[7];
    ProtocolIndAdapter adapter(pMsg->GetModemData());
    memcpy(ImsConf,adapter.GetParameter(), sizeof(ImsConf));
    OnUnsolicitedResponse(RIL_UNSOL_OEM_IMS_DEDICATED_PDN_INFO, (void *)ImsConf, sizeof(ImsConf));

    LEAVE_FUNC();

    return 0;
}

INT32 ImsService::OnImsEmergencyActInfo(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    BYTE EmgInfo[2];
    ProtocolIndAdapter adapter(pMsg->GetModemData());
    OnUnsolicitedResponse(RIL_UNSOL_OEM_IMS_EMERGENCY_ACT_INFO, (void *)EmgInfo, sizeof(EmgInfo));

    LEAVE_FUNC();

    return 0;
}

INT32 ImsService::OnImsSetSrvccInfo(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    BYTE SrvccInfo[3];
    ProtocolIndAdapter adapter(pMsg->GetModemData());
    memcpy(SrvccInfo, adapter.GetParameter(), sizeof(SrvccInfo));
    OnUnsolicitedResponse(RIL_UNSOL_OEM_IMS_SET_SRVCC_INFO, (void *)SrvccInfo, sizeof(SrvccInfo));

    //Send to Framework
    {
        INT32 SrvccResult =  SrvccInfo[1] < 2 ? 0x00000011 & SrvccInfo[1] : 2;
        OnUnsolicitedResponse(RIL_UNSOL_SRVCC_STATE_NOTIFY, (int *)&SrvccResult, sizeof(INT32));
        RilLogV("%s() RIL_UNSOL_SRVCC_STATE_NOTIFY: %d ##", __FUNCTION__, SrvccResult);

        m_srvccState = (RIL_SrvccState)SrvccResult;
    }

    switch(m_srvccState)
    {
    case HANDOVER_STARTED:
        RilLogI("[%s] SRVCC : HO Started", GetServiceName());
        break;
    case HANDOVER_COMPLETED:
        {
            RilLogI("[%s] SRVCC : HO Completed", GetServiceName());
            OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED);

            // send non-VoLTE command instead of VoLTE request
            if ( m_pCurReqMsg != NULL ) {
                if ( m_pCurReqMsg->GetMsgId() == MSG_AIMS_ANSWER ) {
                    RilLogI("[%s] Change AIMS answer to CS answer", GetServiceName());
                    ProtocolCallBuilder builder;
                    ModemData *pModemData = builder.BuildAnswer();
                    if ( SendRequest(pModemData, IMS_AIMS_REQUEST_TIMEOUT, MSG_AIMS_ANSWER_DONE) < 0 ) {
                        OnRequestComplete(RIL_E_GENERIC_FAILURE);
                    }
                }
            }
        }
        break;
    default://failed
        {
            RilLogI("[%s] SRVCC : HO Failed or cancelled", GetServiceName());

            if ( m_pCurReqMsg != NULL ) {
                if ( m_pCurReqMsg->GetMsgId() == MSG_AIMS_ANSWER ) {
                    RilLogI("[%s] Send original AIMS command", GetServiceName());
                    if ( DoAimsDefaultRequestHandler(m_pCurReqMsg) < 0 ) {
                        OnRequestComplete(RIL_E_GENERIC_FAILURE);
                    }
                }
            }
        }
        break;
    }

    LEAVE_FUNC();

    return 0;
}

INT32 ImsService::OnImsEmergencyCallList(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolIndAdapter adapter(pMsg->GetModemData());
    OnUnsolicitedResponse(RIL_UNSOL_OEM_IMS_EMERGENCY_ACT_INFO, adapter.GetParameter(), adapter.GetParameterLength());

    LEAVE_FUNC();

    return 0;
}

//AIMS support start ---------------------
INT32 ImsService::Do_AIMS_SET_CALL_WAITING(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    //int nResult = -1;

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    const char *ImsData = (char *)rildata->GetRawData();

    RilLogI("[%s] %s ", m_szSvcName, __FUNCTION__ );

    if ( ImsData != NULL )
    {
        ProtocolImsAimCallWaitingAdapter adapter(ImsData, rildata->GetSize());
        BOOL bEnable = adapter.IsEnable();
        int nServiceClass = adapter.GetServiceClass();
        RilLogV("[%s] set cw --> enable : %d, SS Class:%d, socketid:%d", __FUNCTION__, bEnable, nServiceClass, GetRilSocketId());
        SetCallWaiting(bEnable, GetRilSocketId());

        return DoAimsDefaultRequestHandler(pMsg);
    }

    LEAVE_FUNC();
    return -1;

}

INT32 ImsService::On_AIMS_SET_CALL_WAITINGDone(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolRespAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();

    OnRequestComplete(uErrCode == RIL_E_SUCCESS ? RIL_E_SUCCESS : RIL_E_GENERIC_FAILURE);

    LEAVE_FUNC();
    return 0;
}

INT32 ImsService::Do_AIMS_GET_REGISTRATION(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    INT32 RespInts[2];
    RespInts[0] = mImsRegState;
    RespInts[1] = RADIO_TECH_3GPP;

    OnRequestComplete(RIL_E_SUCCESS, RespInts, 2*sizeof(INT32));

    LEAVE_FUNC();
    return 0;
}

INT32 ImsService::OnUNSOL_AIMS_REGISTRATION(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    ProtocolImsRegIndAdapter adapter(pMsg->GetModemData());
    OnUnsolicitedResponse(RIL_UNSOL_OEM_AIMS_REGISTRATION, adapter.GetParameter(), adapter.GetParameterLength());

    int imsReg = adapter.GetRegState();
    if (imsReg != -1)
        mImsRegState = imsReg;

    RilProperty *property = GetRilContextProperty();
    if (property != NULL) {
        property->Put(RIL_CONTEXT_IMS_REGISTRATION, mImsRegState);
    }

    RilLogV("[%s::%s] Ims Registration : %s", m_szSvcName,__FUNCTION__, mImsRegState==RIL_IMS_REGISTERED?"TRUE":"FALSE");
    OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED);

    LEAVE_FUNC();

    return 0;
}

/**
 * DoAimsDefaultRequestHandler
 * @desc default AIMS Request handler
 */
int ImsService::DoAimsDefaultRequestHandler(Message *pMsg)
{
    UINT timeout = IMS_AIMS_REQUEST_TIMEOUT;

    RilLogI("[%s] Process AIMS Request", GetServiceName());
    NULL_REQ(pMsg);

    RawRequestData *rildata =(RawRequestData *)(pMsg->GetRequestData());
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    RilLogV("[%s] AIMS Request: msgId=%d requestId=%d parameter=0x%p parameter length=%d",
            GetServiceName(), pMsg->GetMsgId(), rildata->GetReqId(), rildata->GetRawData(), rildata->GetSize());

    ProtocolImsBuilder builder;
    ModemData *pModemData = builder.BuildAimsPDU(rildata->GetReqId(), rildata->GetRawData(), rildata->GetSize());

    // if timeout value need to be changed, set it here
    switch( pMsg->GetMsgId() ) {
        case MSG_AIMS_HANGUP:
            timeout = 3000;
            break;
    }

    if (SendRequest(pModemData, timeout, pMsg->GetMsgId() + 1) < 0) {
        RilLogW("[%s] Failed to send AIMS PDU", GetServiceName());
        return -1;
    }

    return 0;
}

/**
 * OnAimsDefaultResponseHandler
 * @desc default AIMS Response handler
 */
int ImsService::OnAimsDefaultResponseHandler(Message *pMsg)
{
    RilLogI("[%s] Process AIMS Response", GetServiceName());
    PARAM_NULL(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    RilLogV("[%s] AIMS Response: msgId=%d errorCode=%d parameter=0x%p parameter length=%d",
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
 * OnAimsResponseHandlerWithErrorCode
 * @desc default AIMS Response handler
 */
int ImsService::OnAimsResponseHandlerWithErrorCode(Message *pMsg)
{
    RilLogI("[%s] Process AIMS Response", GetServiceName());
    PARAM_NULL(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    RilLogV("[%s] AIMS Response: msgId=%d errorCode=%d parameter=0x%p parameter length=%d",
            GetServiceName(), pMsg->GetMsgId(), errorCode, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());
    }
    else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

/**
 * DoAimsDefaultRequestIndHandler
 * @desc default AIMS Request(Ind) handler
 *           there's no response for this request. (indication from AP to CP)
 */
int ImsService::DoAimsDefaultRequestIndHandler(Message *pMsg)
{
    RilLogV("[%s] Process AIMS Request(Ind)", GetServiceName());
    NULL_REQ(pMsg);

    RawRequestData *rildata =(RawRequestData *)(pMsg->GetRequestData());
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    RilLogV("[%s] AIMS Request(Ind): msgId=%d requestId=%d parameter=0x%p parameter length=%d",
            GetServiceName(), pMsg->GetMsgId(), rildata->GetReqId(), rildata->GetRawData(), rildata->GetSize());

    ProtocolImsBuilder builder;
    ModemData *pModemData = builder.BuildAimsIndPDU(rildata->GetReqId(), rildata->GetRawData(), rildata->GetSize());

    if (pModemData != NULL) {
        if (SendRequest(pModemData) < 0) {
            RilLogW("[%s] Failed to send AIMS Ind PDU", GetServiceName());
        }
        delete pModemData;
    }
    OnRequestComplete(RIL_E_SUCCESS);

    return 0;
}

/**
 * OnAimsDefaultUnsolRespHandler
 * @desc default AIMS Unsolicited Response handler
 */
int ImsService::OnAimsDefaultUnsolRespHandler(Message *pMsg)
{
    RilLogV("[%s] Process AIMS Indication", GetServiceName());
    PARAM_NULL(pMsg);

    ProtocolAimsIndAdapter adapter(pMsg->GetModemData());
    int resultId = adapter.GetResultId();
    RilLogV("[%s] AIMS Indication: msgId=%d resultId=%d parameter=0x%p parameter length=%d",
            GetServiceName(), pMsg->GetMsgId(), resultId, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());

    if (resultId != -1) {
        OnUnsolicitedResponse(resultId, adapter.GetParameter(), adapter.GetParameterLength());
    }
    else {
        RilLogW("[%s] Invalid or undefined protocolId=%d", adapter.GetId());
    }

    return 0;
}

//AIMS support end ----------------------



// #### Mandantory Functions ####

ImsService::ImsService(RilContext* pRilContext)
: Service(pRilContext, RIL_SERVICE_IMS)
{
    strcpy(m_szSvcName, "ImsService");
    mImsRegState = RIL_IMS_NOT_REGISTERED;
    RilProperty *property = GetRilContextProperty();
    if (property != NULL) {
        property->Put(RIL_CONTEXT_IMS_REGISTRATION, mImsRegState);
    }

    m_srvccState = HANDOVER_COMPLETED;

}

ImsService::~ImsService()
{
}

int ImsService::OnCreate(RilContext *pRilContext)
{
    ENTER_FUNC();

    //INT32 nRet = -1;

    LEAVE_FUNC();
    return 0;
}

void ImsService::OnDestroy()
{
    INT32 nRet = -1;

    ENTER_FUNC();

    do
    {
        nRet = 0;
    }while(FALSE);


    LEAVE_FUNC();
}

BOOL ImsService::OnHandleRequest(Message *pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    INT32 nRet = -1;
    INT32 nMsgId = 0;


    do
    {
        if(NULL == pMsg)
        {
            RilLogE("%s::%s() !! ERROR !!, pMsg = NULL",m_szSvcName,__FUNCTION__);
            nRet = -2;
            break;
        }

        nMsgId = pMsg->GetMsgId();

        // Processing Received Message(pMsg)
        RilLogV("%s::%s() nMsgId=%d",m_szSvcName,__FUNCTION__,nMsgId);
        nRet = 0;
        switch (nMsgId)
        {
            case MSG_IMS_SET_CONF:
                nRet = DoSetConfig(pMsg);
                break;

            case MSG_IMS_GET_CONF:
                nRet = DoGetConfig(pMsg);
                break;

            case MSG_IMS_SIM_AUTH:
                nRet = DoSimAuth(pMsg);
                break;

            case MSG_IMS_SET_SRVCC_CALL_LIST:
                nRet = DoSetSrvccCallList(pMsg);
                break;

            case MSG_IMS_SET_ERERGENCY_CALL_STATUS:
                nRet = DoSetEmergencyCallStatus(pMsg);
                break;

            case MSG_IMS_GET_GBA_AUTH:
                nRet = DoGetGbaAuth(pMsg);
                break;

            case MSG_AIMS_GET_REGISTRATION:
                nRet = Do_AIMS_GET_REGISTRATION(pMsg);
                break;

            //AIMS support start ---------------------
            // AIMS common handler
            case MSG_AIMS_DIAL:
            case MSG_AIMS_DEREGISTRATION:
            case MSG_AIMS_CALL_MANAGE:
            case MSG_AIMS_CALL_MODIFY:
            case MSG_AIMS_RESPONSE_CALL_MODIFY:
            case MSG_AIMS_CONF_CALL_ADD_REMOVE_USER:
            case MSG_AIMS_ENHANCED_CONF_CALL:
            case MSG_AIMS_GET_CALL_FORWARD_STATUS:
            case MSG_AIMS_SET_CALL_FORWARD_STATUS:
            case MSG_AIMS_GET_CALL_WAITING:
            //case MSG_AIMS_SET_CALL_WAITING:
            case MSG_AIMS_GET_CALL_BARRING:
            case MSG_AIMS_SET_CALL_BARRING:
            case MSG_AIMS_CHG_BARRING_PWD:
            case MSG_AIMS_SEND_USSD_INFO:
            case MSG_AIMS_HANGUP:
            case MSG_AIMS_HIDDEN_MENU:
            case MSG_AIMS_SEND_DTMF:
            case MSG_AIMS_SET_FRAME_TIME:
            case MSG_AIMS_GET_FRAME_TIME:
            case MSG_AIMS_TIME_INFO:
            case MSG_AIMS_GET_PRESENTATION_SETTINGS:
            case MSG_AIMS_SET_PRESENTATION_SETTINGS:
            case MSG_AIMS_SET_SELF_CAPABILITY:
            case MSG_AIMS_XCAPM_START_REQ:
            case MSG_AIMS_XCAPM_STOP_REQ:
            case MSG_AIMS_RTT_SEND_TEXT:
            case MSG_AIMS_EXIT_EMERGENCY_CB_MODE:
            case MSG_AIMS_SET_GEO_LOCATION_INFO:
            case MSG_AIMS_CDMA_SEND_SMS:
            case MSG_AIMS_RCS_MULTI_FRAME_REQ:
            case MSG_AIMS_RCS_CHAT_REQ:
            case MSG_AIMS_RCS_GROUP_CHAT_REQ:
            case MSG_AIMS_RCS_OFFLINE_MODE_REQ:
            case MSG_AIMS_RCS_FILE_TRANSFER_REQ:
            case MSG_AIMS_RCS_COMMON_MESSAGE_REQ:
            case MSG_AIMS_RCS_CONTENT_SHARE_REQ:
            case MSG_AIMS_RCS_PRESENCE_REQ:
            case MSG_AIMS_XCAP_MANAGE_REQ:
            case MSG_AIMS_RCS_CONFIG_MANAGE_REQ:
            case MSG_AIMS_RCS_TLS_MANAGE_REQ:
            case MSG_AIMS_SET_PDN_EST_STATUS:
            case MSG_AIMS_SET_RTP_RX_STATISTICS:
            case MSG_WFC_MEDIA_CHANNEL_CONFIG:
            case MSG_WFC_DTMF_START:
            case MSG_WFC_SET_VOWIFI_HO_THRESHOLD:
                nRet = DoAimsDefaultRequestHandler(pMsg);
                break;
            case MSG_AIMS_HO_TO_WIFI_CANCEL_IND:
            case MSG_AIMS_PAYLOAD_INFO_IND:
            case MSG_AIMS_MEDIA_STATE_IND:
                nRet = DoAimsDefaultRequestIndHandler(pMsg);
                break;

            // AIMS custom handler
#if 0
            case MSG_AIMS_GET_CALL_WAITING:
                nRet = Do_AIMS_GET_CALL_WAITING(pMsg);
                break;
#endif
            case MSG_AIMS_SET_CALL_WAITING:
                nRet = Do_AIMS_SET_CALL_WAITING(pMsg);
                break;

            // handle aims command during srvcc
            case MSG_AIMS_ANSWER:
                if ( m_srvccState == HANDOVER_STARTED ) {
                    // command will be decided after SRVCC HO complete
                    RilLogI("[%s::%s] Command is pending during SRVCC is processing", GetServiceName(), __FUNCTION__ );
                    nRet = 0;
                }
                else {
                    nRet = DoAimsDefaultRequestHandler(pMsg);
                }
                break;

            //AIMS support end ---------------------

            default:
            {
                if (nMsgId - MSG_IMS_BASE < 1000) {
                    RilLogE("%s::%s() MSG_IMS_BASE msgId= %d", m_szSvcName, __FUNCTION__, nMsgId);
                    nRet = DoAimsDefaultRequestHandler(pMsg);
                }
                else {
                    RilLogE("%s::%s() !! ERROR !!, Unknown nMsgId = %d",m_szSvcName,__FUNCTION__,nMsgId);
                    nRet = -3;
                }
                break;
            }
        }
    }while(FALSE);


    LEAVE_FUNC();

    if(0 == nRet)
        return TRUE;
    else
        return FALSE;

}

BOOL ImsService::OnHandleSolicitedResponse(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    INT32 nRet = -1;
    INT32 nMsgId = 0;

    do
    {
        if(NULL == pMsg)
        {
            RilLogE("%s::%s() !! ERROR !!, pMsg = NULL",m_szSvcName,__FUNCTION__);
            nRet = -2;
            break;
        }
        nMsgId = pMsg->GetMsgId();


        // Processing Received Message(pMsg)
        RilLogV("%s::%s() nMsgId=%d",m_szSvcName,__FUNCTION__,nMsgId);
        nRet = 0;
        switch (nMsgId)
        {
            case MSG_IMS_SET_CONF_DONE:
                nRet = OnSetConfigDone(pMsg);
                break;

            case MSG_IMS_GET_CONF_DONE:
                nRet = OnGetConfigDone(pMsg);
                break;

            case MSG_IMS_SIM_AUTH_DONE:
                nRet = OnSimAuthDone(pMsg);
                break;

            case MSG_IMS_SET_ERERGENCY_CALL_STATUS_DONE:
                nRet = OnSetEmergencyCallStatusDone(pMsg);
                break;

            case MSG_IMS_SET_SRVCC_CALL_LIST_DONE:
                nRet = OnSetSrvccCallListDone(pMsg);
                break;

            case MSG_IMS_GET_GBA_AUTH_DONE:
                nRet = OnGetGbaAuthDone(pMsg);
                break;

            //AIMS support start ---------------------
            // AIMS common handler
            case MSG_AIMS_DIAL_DONE:
            case MSG_AIMS_ANSWER_DONE:
            case MSG_AIMS_DEREGISTRATION_DONE:
            case MSG_AIMS_CALL_MANAGE_DONE:
            case MSG_AIMS_CALL_MODIFY_DONE:
            case MSG_AIMS_RESPONSE_CALL_MODIFY_DONE:
            case MSG_AIMS_CONF_CALL_ADD_REMOVE_USER_DONE:
            case MSG_AIMS_ENHANCED_CONF_CALL_DONE:
            case MSG_AIMS_CHG_BARRING_PWD_DONE:
            case MSG_AIMS_SEND_USSD_INFO_DONE:
            case MSG_AIMS_HANGUP_DONE:
            case MSG_AIMS_HIDDEN_MENU_DONE:
            case MSG_AIMS_SEND_DTMF_DONE:
            case MSG_AIMS_SET_FRAME_TIME_DONE:
            case MSG_AIMS_GET_FRAME_TIME_DONE:
            case MSG_AIMS_TIME_INFO_DONE:
            case MSG_AIMS_SET_SELF_CAPABILITY_DONE:
            case MSG_AIMS_XCAPM_START_REQ_DONE:
            case MSG_AIMS_XCAPM_STOP_REQ_DONE:
            case MSG_AIMS_RTT_SEND_TEXT_DONE:
            case MSG_AIMS_EXIT_EMERGENCY_CB_MODE_DONE:
            case MSG_AIMS_SET_GEO_LOCATION_INFO_DONE:
            case MSG_AIMS_CDMA_SEND_SMS_DONE:
            case MSG_AIMS_RCS_MULTI_FRAME_REQ_DONE:
            case MSG_AIMS_RCS_CHAT_REQ_DONE:
            case MSG_AIMS_RCS_GROUP_CHAT_REQ_DONE:
            case MSG_AIMS_RCS_OFFLINE_MODE_REQ_DONE:
            case MSG_AIMS_RCS_FILE_TRANSFER_REQ_DONE:
            case MSG_AIMS_RCS_COMMON_MESSAGE_REQ_DONE:
            case MSG_AIMS_RCS_CONTENT_SHARE_REQ_DONE:
            case MSG_AIMS_RCS_PRESENCE_REQ_DONE:
            case MSG_AIMS_XCAP_MANAGE_REQ_DONE:
            case MSG_AIMS_RCS_CONFIG_MANAGE_REQ_DONE:
            case MSG_AIMS_RCS_TLS_MANAGE_REQ_DONE:
            case MSG_AIMS_SET_PDN_EST_STATUS_DONE:
            case MSG_AIMS_SET_RTP_RX_STATISTICS_DONE:
            case MSG_WFC_MEDIA_CHANNEL_CONFIG_DONE:
            case MSG_WFC_DTMF_START_DONE:
            case MSG_WFC_SET_VOWIFI_HO_THRESHOLD_DONE:
                nRet = OnAimsDefaultResponseHandler(pMsg);
                break;
            case MSG_AIMS_GET_PRESENTATION_SETTINGS_DONE:
            case MSG_AIMS_SET_PRESENTATION_SETTINGS_DONE:
            case MSG_AIMS_GET_CALL_FORWARD_STATUS_DONE:
            case MSG_AIMS_SET_CALL_FORWARD_STATUS_DONE:
            case MSG_AIMS_GET_CALL_WAITING_DONE:
            case MSG_AIMS_SET_CALL_WAITING_DONE:
            case MSG_AIMS_GET_CALL_BARRING_DONE:
            case MSG_AIMS_SET_CALL_BARRING_DONE:
                nRet = OnAimsResponseHandlerWithErrorCode(pMsg);
                break;
            //AIMS support end ---------------------

            default:
            {
                if (nMsgId - MSG_IMS_BASE < 1000) {
                    RilLogV("%s::%s() MSG_IMS_BASE msgId=%d", m_szSvcName, __FUNCTION__, nMsgId);
                    nRet = OnAimsDefaultResponseHandler(pMsg);
                }
                else {
                    RilLogE("%s::%s() !! ERROR !!, Unknown nMsgId = %d",m_szSvcName,__FUNCTION__,nMsgId);
                    nRet = -3;
                }
                break;
            }
        }
    }while(FALSE);


    LEAVE_FUNC();

    if(0 == nRet)
        return TRUE;
    else
        return FALSE;

}

BOOL ImsService::OnHandleUnsolicitedResponse(Message *pMsg)
{
    ENTER_FUNC();
    PARAM_NULL(pMsg);

    INT32 nRet = -1;
    INT32 nMsgId = 0;

    do
    {
        if(NULL == pMsg)
        {
            RilLogE("%s::%s() !! ERROR !!, pMsg = NULL",m_szSvcName,__FUNCTION__);
            nRet = -2;
            break;
        }
        nMsgId = pMsg->GetMsgId();

        RilLogI("%s::%s() nMsgId=%d",m_szSvcName,__FUNCTION__,nMsgId);
        nRet = 0;
        switch (nMsgId)
        {
            case MSG_IMS_CONFIG:
                nRet = OnImsConfiguration(pMsg);
                break;

            case MSG_IMS_DEDICATED_PDN_INFO:
                nRet = OnImsDedicatedPdnInfo(pMsg);
                break;

            case MSG_IMS_EMERGENCY_ACT_INFO:
                nRet = OnImsEmergencyActInfo(pMsg);
                break;

            case MSG_IMS_SET_SRVCC_INFO:
                nRet = OnImsSetSrvccInfo(pMsg);
                break;

            case MSG_IMS_EMERGENCY_CALL_LIST:
                nRet = OnImsEmergencyCallList(pMsg);
                break;

            //AIMS support start ---------------------
            // AIMS common handler
            case MSG_IND_AIMS_CALL_RING:
            case MSG_IND_AIMS_ON_USSD:
            case MSG_IND_AIMS_CONFERENCE_CALL_EVENT:
            case MSG_IND_AIMS_CALL_MODIFY:
            case MSG_IND_AIMS_FRAME_TIME:
            case MSG_IND_AIMS_SUPP_SVC_NOTIFICATION:
            case MSG_IND_AIMS_NEW_SMS:
            case MSG_IND_AIMS_NEW_SMS_STATUS_REPORT:
            case MSG_IND_AIMS_PAYLOAD_INFO:
            case MSG_IND_AIMS_VOWIFI_HO_CALL_INFO:
            case MSG_IND_AIMS_NEW_CDMA_SMS:
            case MSG_IND_AIMS_RINGBACK_TONE:
            case MSG_IND_AIMS_CALL_MANAGE:
            case MSG_IND_AIMS_CONF_CALL_ADD_REMOVE_USER:
            case MSG_IND_AIMS_ENHANCED_CONF_CALL:
            case MSG_IND_AIMS_CALL_MODIFY_RSP:
            case MSG_IND_AIMS_CALL_STATUS:
            case MSG_IND_AIMS_RCS_MULTI_FRAME:
            case MSG_IND_AIMS_RCS_CHAT:
            case MSG_IND_AIMS_RCS_GROUP_CHAT:
            case MSG_IND_AIMS_RCS_OFFLINE_MODE:
            case MSG_IND_AIMS_RCS_FILE_TRANSFER:
            case MSG_IND_AIMS_RCS_COMMON_MESSAGE:
            case MSG_IND_AIMS_RCS_CONTENT_SHARE:
            case MSG_IND_AIMS_RCS_PRESENCE:
            case MSG_IND_AIMS_RCS_XCAP_MANAGE:
            case MSG_IND_AIMS_RCS_CONFIG_MANAGE:
            case MSG_IND_AIMS_RCS_TLS_MANAGE:
            case MSG_IND_WFC_RTP_RTCP_TIMEOUT:
            case MSG_IND_WFC_FIRST_RTP:
            case MSG_IND_WFC_RTCP_RX_SR:
            case MSG_IND_WFC_RCV_DTMF_NOTI:
            case MSG_IND_AIMS_DTMF_EVENT:
            case MSG_IND_AIMS_RTT_NEW_TEXT:
            case MSG_IND_AIMS_RTT_FAIL_SENDING_TEXT:
            case MSG_IND_AIMS_EXIT_EMERGENCY_CB_MODE:
            case MSG_IND_AIMS_DIALOG_INFO:
            case MSG_IND_AIMS_MEDIA_STATUS:
            case MSG_IND_AIMS_SIP_MSG_INFO:
            case MSG_IND_AIMS_VOICE_RTQ_QUALITY:
            case MSG_IND_AIMS_RTP_RX_STATISTICS:
                nRet = OnAimsDefaultUnsolRespHandler(pMsg);
                break;
            //AIMS custom handler
            case MSG_IND_AIMS_REGISTRATION:
                nRet = OnUNSOL_AIMS_REGISTRATION(pMsg);
                break;
            //AIMS support end ---------------------

            default:
            {
                if (nMsgId - MSG_IMS_BASE < 1000) {
                    RilLogV("%s::%s() MSG_IMS_BASE msgId=%d", m_szSvcName, __FUNCTION__, nMsgId);
                    nRet = OnAimsDefaultUnsolRespHandler(pMsg);
                }
                else {
                    RilLogE("%s::%s() !! ERROR !!, Unknown nMsgId = %d",m_szSvcName,__FUNCTION__,nMsgId);
                    nRet = -3;
                }
                break;
            }
        }
    }while(FALSE);


    LEAVE_FUNC();

    if(0 == nRet)
        return TRUE;
    else
        return FALSE;

}

BOOL ImsService::OnHandleInternalMessage(Message* pMsg)
{
    ENTER_FUNC();
    NULL_REQ(pMsg);

    INT32 nRet = -1;
    INT32 nMsgId = 0;
    //char *pData = NULL;

    do
    {
        if(NULL == pMsg)
        {
            RilLogE("%s::%s() !! ERROR !!, pMsg = NULL",m_szSvcName,__FUNCTION__);
            nRet = -2;
            break;
        }

        nMsgId = pMsg->GetMsgId();
        //pData = pMsg->GetInternalData();

        RilLogI("%s::%s() nMsgId=%d",m_szSvcName,__FUNCTION__,nMsgId);
        nRet = 0;
        switch(nMsgId)
        {
            default:
                RilLogE("%s::%s() !! ERROR !!, Unkonwn nMsgId = %d",m_szSvcName,__FUNCTION__,nMsgId);
                break;
        }

    }while(FALSE);

    LEAVE_FUNC();

    return TRUE;
}

bool ImsService::IsPossibleToPassInRadioOffState(int request_id)
{
    return true;
}

INT32 ImsService::GetImsRegState() {
    return mImsRegState;
}
