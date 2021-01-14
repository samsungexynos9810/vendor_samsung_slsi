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
#include "util.h"
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
/**
 * SIM Authentication
 */

OemSimAuthRequest::OemSimAuthRequest(const int nReq, const Token tok, const ReqType type) : RequestData(nReq, tok,type)
{
    m_nAuthType = 0;
    m_nAuthLen = 0;
    memset(m_pAuth, 0, MAX_SIM_AUTH_REQ_LEN);
}

OemSimAuthRequest::~OemSimAuthRequest()
{
}

INT32 OemSimAuthRequest::encode(char *data, unsigned int length)
{
    if((0 == length) || (NULL == data)) return -1;

    auth_request_type *pOemSimAuthRequest = (auth_request_type *)data;
    memset(m_pAuth, 0, MAX_SIM_AUTH_REQ_LEN);

    m_nAuthType = pOemSimAuthRequest->auth_type;

    m_nAuthLen = pOemSimAuthRequest->auth_len;
    memcpy(m_pAuth, (const char*)pOemSimAuthRequest->auth, m_nAuthLen);

    return 0;
}
