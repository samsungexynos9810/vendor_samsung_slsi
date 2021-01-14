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
#include "wlandatabuilder.h"
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

const RilData *WLanDataBuilder::BuildSimAuthenticate(BYTE authType, BYTE authLen, BYTE *pBuffer)
{
    auth_response_type resp;

    if (authLen == 0 || pBuffer == NULL) {
        return NULL;
    }

    RilDataString *rildata = new RilDataString();
    if (rildata != NULL) {

        memset(&resp, 0, sizeof(auth_response_type));

        resp.auth_type = authType;
        resp.auth_rsp_len = authLen;
        memcpy(resp.auth_rsp, pBuffer, authLen);

        char *pszAuthString = new char[sizeof(auth_response_type)+1];
        memset(pszAuthString, 0, sizeof(auth_response_type)+1);
        int nAuthLength = Value2HexString(pszAuthString, (const BYTE*)&resp, sizeof(auth_response_type));
        pszAuthString[nAuthLength] = '\0';
        rildata->SetString(pszAuthString);
        delete [] pszAuthString;
    }
    return rildata;
}
