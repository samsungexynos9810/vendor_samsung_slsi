/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "protocolwlanbuilder.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_WLAN, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

ModemData *ProtocolWLanBuilder::GetSimAuthenticate(int nAuthType, BYTE *pAuth, int nAuthLengh)
{
    sit_sim_get_sim_auth_req req;
    int length = sizeof(req);

    memset(&req, 0, sizeof(sit_sim_get_sim_auth_req));
    InitRequestHeader(&req.hdr, SIT_GET_SIM_AUTH, length);

    req.auth_type = nAuthType;
    memcpy(req.auth, pAuth, nAuthLengh);

    req.auth_len = nAuthLengh;
    return new ModemData((char *)&req, length);
}
