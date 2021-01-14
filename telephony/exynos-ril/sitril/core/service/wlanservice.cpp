/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "wlanservice.h"
#include "rillog.h"
#include "protocolwlanadapter.h"
#include "protocolwlanbuilder.h"
#include "wlandatabuilder.h"
#include "wlandata.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

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

WLanService::WLanService(RilContext* pRilContext)
: Service(pRilContext, RIL_SERVICE_WLAN)
{
    strcpy(m_szSvcName, "WLanService");
}

WLanService::~WLanService()
{
}

int WLanService::OnCreate(RilContext *pRilContext)
{
    RilLogI("%s::%s()", m_szSvcName, __FUNCTION__);
    return 0;
}

BOOL WLanService::OnHandleRequest(Message* pMsg)
{
    RilLogI("%s::%s()", m_szSvcName, __FUNCTION__);

    INT32 nRet = -1;
    if(NULL == pMsg)
        return FALSE;

    switch (pMsg->GetMsgId())
    {
        case MSG_WLAN_SIM_AUTHENTICATE:
        {
            nRet = DoSimAuthenticate(pMsg);
            break;
        }
        default:
            break;
    }

    if(0 == nRet)
        return TRUE;
    else
        return FALSE;
}
BOOL WLanService::OnHandleSolicitedResponse(Message* pMsg)
{
    RilLogI("%s::%s()", m_szSvcName, __FUNCTION__);
    INT32 nRet = -1;

    if(NULL == pMsg)
        return FALSE;

    switch(pMsg->GetMsgId())
    {
        case MSG_WLAN_SIM_AUTHENTICATE_DONE:
        {
            nRet = OnSimAuthenticateDone(pMsg);
            break;
        }
        default:
            break;
    }

    if(0 == nRet)
        return TRUE;
    else
        return FALSE;

}

BOOL WLanService::OnHandleUnsolicitedResponse(Message* pMsg)
{
    RilLogI("%s::%s()", m_szSvcName, __FUNCTION__);
    if(NULL == pMsg)
        return FALSE;

    switch (pMsg->GetMsgId())
    {
        default:
            break;
    }

    return TRUE;
}

//  handle request
int WLanService::DoSimAuthenticate(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    OemSimAuthRequest *rildata = (OemSimAuthRequest *)pMsg->GetRequestData();
    if (NULL == rildata) {
        RilLogE("rildata is NULL");
        return -1;
    }
    //int i = 0;
    //BYTE *auth = rildata->GetAuth();

    ProtocolWLanBuilder builder;
    ModemData *pModemData = builder.GetSimAuthenticate(rildata->GetAuthType(), rildata->GetAuth(), rildata->GetLength());
    if (SendRequest(pModemData, TIMEOUT_WLAN_DEFAULT, MSG_WLAN_SIM_AUTHENTICATE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int WLanService::OnSimAuthenticateDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolWLanSimAuthenticate adapter(pMsg->GetModemData());

    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        WLanDataBuilder builder;
        BYTE Auth[69];
        memset(Auth, 0, 69);

        adapter.GetAuth(Auth);
        const RilData *rildata = builder.BuildSimAuthenticate(adapter.GetAuthType(), adapter.GetAuthLen(), Auth);
        if (rildata != NULL){
            RilLogV("[%s] %s (%d)", __FUNCTION__, rildata->GetData(), rildata->GetDataLength());
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

bool WLanService::IsPossibleToPassInRadioOffState(int request_id)
{
    switch (request_id) {
        case RIL_REQUEST_OEM_SIM_AUTHENTICATION:
            break;
        default:
            return false;
    }
    return true;
}
