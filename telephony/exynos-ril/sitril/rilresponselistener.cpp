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
 * rilresponselistener.cpp
 *
 *  Created on: 2014. 11. 20.
 *      Author: sungwoo48.choi
 */

#include "rilresponselistener.h"
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

RilResponseListener::RilResponseListener(const struct RIL_Env *pRilEnv)
    : m_pRilEnv(pRilEnv)
{
    m_pRilApp = NULL;
}

RilResponseListener::~RilResponseListener()
{

}

IMPLEMENT_MODULE_TAG(RilResponseListener, RilResponseListener)

// response
void RilResponseListener::OnRequestComplete(RIL_Token t, RIL_Errno e, void *response, unsigned int responselen)
{
    //RilLogI("[%s] %s ", TAG, __FUNCTION__);

    // In case of RIL request
    if (m_pRilEnv != NULL && m_pRilEnv->OnRequestComplete != NULL) {
        m_pRilEnv->OnRequestComplete(t, e, response, responselen);
    }
    else {
        RilLogW("[%s] %s cannot invoke m_pRilEnv->OnRequestComplete()", TAG, __FUNCTION__);
    }
}

void RilResponseListener::OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen)
{
    OnUnsolicitedResponse(unsolResponse, data, datalen, RIL_SOCKET_1);
}

void RilResponseListener::OnUnsolicitedResponse(int unsolResponse, const void *data, unsigned int datalen, RIL_SOCKET_ID socket_id)
{
    //RilLogI("[%s] %s ", TAG, __FUNCTION__);

    if (FilterOut(unsolResponse, data, datalen)) {
        RilLogV("[%s] %s Filter out Unsolicited response=%d", TAG, __FUNCTION__, unsolResponse);
        return;
    }

    // To the RIL daemon
    if (m_pRilEnv != NULL && m_pRilEnv->OnUnsolicitedResponse != NULL) {
#ifdef ANDROID_MULTI_SIM
        m_pRilEnv->OnUnsolicitedResponse(unsolResponse, data, datalen, socket_id);
#else
        m_pRilEnv->OnUnsolicitedResponse(unsolResponse, data, datalen);
#endif // ANDROID_MULTI_SIM
    }
    else {
        RilLogW("[%s] %s cannot invoke m_pRilEnv->OnUnsolicitedResponse()", TAG, __FUNCTION__);
    }
}

void RilResponseListener::OnRequestAck(RIL_Token t)
{
    RilLogI("[%s] %s ", TAG, __FUNCTION__);

    // In case of RIL request
    if (m_pRilEnv != NULL && m_pRilEnv->OnRequestAck != NULL) {
        m_pRilEnv->OnRequestAck(t);
    }
    else {
        RilLogW("[%s] %s cannot invoke m_pRilEnv->OnRequestAck()", TAG, __FUNCTION__);
    }
}

// Filter out unsolicited response
bool RilResponseListener::FilterOut(int unsolResponse, const void *data, unsigned int datalen)
{
    unsolResponse = DECODE_REQUEST(unsolResponse);

    // OEM unsolicited response
    if (unsolResponse > RIL_OEM_UNSOL_RESPONSE_BASE) {
        // default policy : not allow at all
        switch (unsolResponse) {
        default:
            return true;
        } // end switch ~
    }

    // RIL unsolicited response
    // default policy : allow all

    return false;
}
