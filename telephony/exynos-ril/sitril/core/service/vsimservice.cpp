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
 * vsimservice.cpp
 *
 *  Created on: 2016. 02. 26.
 */

#include "vsimservice.h"
#include "rillog.h"
#include "protocolvsimbuilder.h"
#include "protocolvsimadapter.h"
#include "vsimdatabuilder.h"
#include "vsimdata.h"
#include "util.h"
#include "reset_util.h"

//MOX: Auto Verify PIN
#include "simservice.h"

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_VSIM, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_VSIM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_VSIM, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_VSIM, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

VSimService::VSimService(RilContext* pRilContext)
    : Service(pRilContext, RIL_SERVICE_VSIM)
{

}

VSimService::~VSimService()
{
}


int VSimService::OnCreate(RilContext *pRilContext)
{
    RilLog("[%s] %s()", GetServiceName(), __FUNCTION__);
    return 0;
}

void VSimService::OnStart()
{

}


BOOL VSimService::OnHandleRequest(Message* pMsg)
{
    int ret = -1;
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_VSIM_NOTIFICATION:
        ret = DoVsimNotification(pMsg);
        break;
    case MSG_VSIM_OPERATION:
        ret = DoVsimOperation(pMsg);
        break;
    default:
        break;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL VSimService::OnHandleSolicitedResponse(Message* pMsg)
{
    int ret = -1;
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_VSIM_NOTIFICATION_DONE:
        ret = OnVsimNotificationDone(pMsg);
        break;
    case MSG_VSIM_OPERATION_DONE:
        ret = OnVsimOperationDone(pMsg);
        break;
    default:
        break;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL VSimService::OnHandleUnsolicitedResponse(Message* pMsg)
{
    //int ret = -1;
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_VSIM_OPERATION_IND:
        OnVsimOperationInd(pMsg);
        break;
    default:
        break;
    } // end switch ~

    return TRUE;
}

int VSimService::DoVsimNotification(Message* pMsg)
{
    RilLog("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntsRequestData *rildata = (IntsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int tid = rildata->GetInt(0);
    int eventid = rildata->GetInt(1);
    int simtype = rildata->GetInt(2);

    RilLog("[%s] tid: %d , eventid: %d, simtype: %d", __FUNCTION__, tid, eventid, simtype);

    ProtocolVsimBuilder builder;
    ModemData *pModemData = builder.BuildVsimNotification(tid, eventid, simtype);
    if ( pModemData == NULL )
    {
        RilLogE("%s: Fail to make modem data to send", __FUNCTION__);
        return -1;
    }
    else if (SendRequest(pModemData, TIMEOUT_VSIM_DEFAULT_TIMEOUT, MSG_VSIM_NOTIFICATION_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int VSimService::OnVsimNotificationDone(Message* pMsg)
{
    RilLog("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);

// MOX: Automatic Verifying PIN
#ifdef _AUTO_VERIFY_PIN_
        SimService *pSimService = (SimService *) GetRilContext()->GetService(RIL_SERVICE_SIM);
        if(pSimService) pSimService->SaveAutoVerifyPin();
        else RilLogE("pSimService is NULL");

        pSimService = (SimService *) GetOppositeService(RIL_SERVICE_SIM);
        if(pSimService) pSimService->SaveAutoVerifyPin();
        else RilLogE("GetOppositeService()::pSimService is NULL");
#endif
        property_set(RIL_UIM_REMOTE_SLOT, GetRilSocketId() == 0 ? "0" : "1");

        //For enable/disable vsim without silent reset
        //usleep(500000);   // guarantee nv file sync : cp send response right after NV file update, but for sure, add 500 msec
        //RilReset("VSIM_NOTIFICATION_RESET");
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

int VSimService::DoVsimOperation(Message* pMsg)
{
    RilLog("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    VsimOperationData *rildata = (VsimOperationData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int tid = rildata->GetTid();
    int eventid = rildata->GetEventId();
    int result = rildata->GetResult();
    int datalen = rildata->GetDataLength();
    char* pData = rildata->GetData();
    RilLog("[%s] tid : %d , eventid :%d, result: %d, datalen: %d", __FUNCTION__, tid, eventid, result, datalen);

    ProtocolVsimBuilder builder;
    ModemData *pModemData = builder.BuildVsimOperation(tid, eventid, result, datalen, pData);
    if ( pModemData == NULL )
    {
        RilLogE("%s: Fail to make modem data to send", __FUNCTION__);
        return -1;
    }
    else if (SendRequest(pModemData, TIMEOUT_VSIM_DEFAULT_TIMEOUT, MSG_VSIM_OPERATION_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int VSimService::OnVsimOperationDone(Message* pMsg)
{
    RilLog("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

int VSimService::OnVsimOperationInd(Message* pMsg)
{
    RilLog("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    // notify to the upper layer with data
    ProtocolVsimOperationAdapter adapter(pModemData);

    int tid = adapter.GetTransactionId();
    int eventid = adapter.GetEventId();
    int result = adapter.GetResult();
    const char* pData = adapter.GetOperationData();  // null-terminated HEX string
    int datalen = adapter.GetOperationDataLength();  // a length of HEX string
    RilLog("[%s] tid : %d , eventid(%d), result(%d), datalen(%d)", __FUNCTION__, tid, eventid, result, datalen);

    if ( eventid == -1 )
    {
        RilLogE("%s : invalid eventid, ignore CP's indication", __FUNCTION__);
        return 0;
    }

    VsimDataBuilder builder;
/*
    Using OEM path only
    const RilData *rildata = builder.BuildVsimOperation(tid, eventid, result, datalen, pData);
    if (rildata != NULL) {
        OnUnsolicitedResponse(RIL_UNSOL_VSIM_OPERATION_INDICATION, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }
*/

    // External path
    const RilData *rildataExt = builder.BuildVsimOperationExt(tid, eventid, result, datalen, pData);
    if (rildataExt != NULL) {
        OnUnsolicitedResponse(RIL_UNSOL_OEM_VSIM_OPERATION, rildataExt->GetData(), rildataExt->GetDataLength());
        delete rildataExt;
    }

    return 0;
}

bool VSimService::IsPossibleToPassInRadioOffState(int request_id)
{
    switch (request_id) {
        case RIL_LOCAL_REQUEST_VSIM_NOTIFICATION:
        case RIL_LOCAL_REQUEST_VSIM_OPERATION:
        case RIL_REQUEST_OEM_VSIM_NOTIFICATION:
        case RIL_REQUEST_OEM_VSIM_OPERATION:
            break;
        default:
            return false;
    }
    return true;
}

bool VSimService::IsPossibleToPassInRadioUnavailableState(int request_id)
{
    switch(request_id) {
        //case RIL_LOCAL_REQUEST_VSIM_NOTIFICATION:
        case RIL_LOCAL_REQUEST_VSIM_OPERATION:
        //case RIL_REQUEST_OEM_VSIM_NOTIFICATION:
        case RIL_REQUEST_OEM_VSIM_OPERATION:
            break;
        default:
            return false;
    }
    return true;
}
