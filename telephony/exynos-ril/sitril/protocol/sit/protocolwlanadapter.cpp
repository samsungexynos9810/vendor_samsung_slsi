/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "wlandata.h"
#include "protocolwlanadapter.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

BYTE ProtocolWLanSimAuthenticate::GetAuthType() const
{
    BYTE authType = 0;
    if (m_pModemData != NULL) {
        sit_sim_get_sim_auth_rsp *pData = (sit_sim_get_sim_auth_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id==SIT_GET_SIM_AUTH) {
            authType = pData->auth_type;
        }
    }

    return authType;
}

BYTE ProtocolWLanSimAuthenticate::GetAuthLen() const
{
    BYTE authLen = 0;
    if (m_pModemData != NULL) {
        sit_sim_get_sim_auth_rsp *pData = (sit_sim_get_sim_auth_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id==SIT_GET_SIM_AUTH) {
            authLen = pData->auth_len;
        }
    }

    return authLen;
}

BYTE ProtocolWLanSimAuthenticate::GetAuth(BYTE *pBuffer) const
{
    BYTE authLen = 0;
    if (m_pModemData != NULL) {
        sit_sim_get_sim_auth_rsp *pData = (sit_sim_get_sim_auth_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id==SIT_GET_SIM_AUTH && pBuffer) {
            authLen = pData->auth_len;
            memcpy(pBuffer, pData->auth, pData->auth_len);
        }
    }

    return authLen;
}

