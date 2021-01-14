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
 * RilExternalResponseListener.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: sungwoo48.choi
 */

#include "rilexternalresponselistener.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

RilExternalResponseListener::RilExternalResponseListener(const struct RIL_External_Env *pRilExternalEnv)
    : m_pRilExternalEnv(pRilExternalEnv)
{
    m_pRilApp = NULL;
}

RilExternalResponseListener::~RilExternalResponseListener()
{

}

IMPLEMENT_MODULE_TAG(RilExternalResponseListener, RilExternalResponseListener)

// response
void RilExternalResponseListener::OnRequestComplete(RIL_External_Token t, RIL_Errno e, void *response, unsigned int responselen)
{
    //RilLogI("[%s] %s ", TAG, __FUNCTION__)

    // In case of RIL External request
    if (m_pRilExternalEnv != NULL && m_pRilExternalEnv->ExternalOnRequestComplete != NULL) {
        m_pRilExternalEnv->ExternalOnRequestComplete(t, e, response, responselen);
    }
    else {
        RilLogW("[%s] %s cannot invoke m_pRilExternalEnv->ExternalOnRequestComplete()", TAG, __FUNCTION__);
    }
}

void RilExternalResponseListener::OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen)
{
    OnUnsolicitedResponse(unsolResponse, data, datalen, RIL_SOCKET_1);
}

void RilExternalResponseListener::OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen, RIL_SOCKET_ID socket_id)
{
    //RilLogI("[%s] %s ", TAG, __FUNCTION__)

    if (FilterOut(unsolResponse, data, datalen)) {
        RilLogV("[%s] %s Filter out Unsolicited response=%d, channel=%d", TAG, __FUNCTION__, unsolResponse, (int)socket_id);
        return;
    }

    // To the RIL daemon
    if (m_pRilExternalEnv != NULL && m_pRilExternalEnv->ExternalOnUnsolicitedResponse != NULL) {
        m_pRilExternalEnv->ExternalOnUnsolicitedResponse(unsolResponse, data, datalen, socket_id);
    }
    else {
        RilLogW("[%s] %s cannot invoke m_pRilExternalEnv->ExternalOnUnsolicitedResponse()", TAG, __FUNCTION__);
    }
}

void RilExternalResponseListener::OnRequestAck(RIL_External_Token t)
{
    //RilLogI("[%s] %s ", TAG, __FUNCTION__);
}

// Filter out unsolicited response
bool RilExternalResponseListener::FilterOut(int unsolResponse, const void *data, unsigned int datalen)
{
    unsolResponse = DECODE_REQUEST(unsolResponse);
    // RIL unsolicited response
    if (unsolResponse < RIL_OEM_UNSOL_RESPONSE_BASE) {
        // default policy : not allow at all
        switch (unsolResponse) {
        case RIL_UNSOL_WB_AMR_REPORT_IND:
            break;
        default:
            return true;
        } // end switch ~
    }

    // RIL OEM unsolicited response
    // default policy : allow all
    return false;
}
