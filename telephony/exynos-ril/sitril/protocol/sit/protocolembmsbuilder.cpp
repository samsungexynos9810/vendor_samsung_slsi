/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "protocolembmsbuilder.h"
#include "util.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_EMBMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

ModemData *ProtocolEmbmsBuilder::BuildSetService(int state)
{
    sit_embms_set_service_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_EMBMS_SERVICE, length);

    req.service_state = state;
    return new ModemData((char *)&req, length);
}

char *ProtocolEmbmsBuilder::getStringTypeTmgi(uint64_t tmgi) {

    memset(m_szTmgi, 0x00, sizeof(m_szTmgi));


    m_szTmgi[0] = ((tmgi>>40)&0xFF);
    m_szTmgi[1] = ((tmgi>>32)&0xFF);
    m_szTmgi[2] = ((tmgi>>24)&0xFF);
    m_szTmgi[3] = ((tmgi>>16)&0xFF);
    m_szTmgi[4] = ((tmgi>>8)&0xFF);
    m_szTmgi[5] = ((tmgi)&0xFF);

    return m_szTmgi;
}

ModemData *ProtocolEmbmsBuilder::BuildSetSession(int state, uint64_t tmgi, int saiListLen, const uint32_t *pSaiList, int freqListLen, const uint32_t *pFreqList)
{
    sit_embms_set_session_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_EMBMS_SESSION, length);

    req.session_state = state;
    req.sai_list_len = saiListLen;
    req.freq_list_len = freqListLen;
    memset(&req.TMGI, 0x00, sizeof(req.TMGI));
    memset(&req.deActTMGI, 0x00, sizeof(req.deActTMGI));
    memset(&req.saiList, 0x00, sizeof(req.saiList));
    memset(&req.freqList, 0x00, sizeof(req.freqList));

    if (state == 0x02) { // Deactive embms session
        memcpy(&req.deActTMGI, getStringTypeTmgi(tmgi), sizeof(req.deActTMGI));
    }

    memcpy(&req.TMGI, getStringTypeTmgi(tmgi), sizeof(req.TMGI));

    if (pSaiList != NULL) {
        for (int i = 0; i < saiListLen; i++) {
            req.saiList[i] = (uint16_t)pSaiList[i];
        }
    }
    if (pFreqList != NULL) {
        for (int i = 0; i < freqListLen; i++) {
            req.freqList[i] = pFreqList[i];
        }
    }

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolEmbmsBuilder::BuildGetSessionList(int state)
{
    sit_embms_ssesion_list_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_EMBMS_SESSION_LIST, length);

    req.session_state = state;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolEmbmsBuilder::BuildSignalStrength()
{
    null_data_format req;
    int length = sizeof(null_data_format);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_EMBMS_SIGNAL_STRENGTH, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolEmbmsBuilder::BuildNetworkTime()
{
    null_data_format req;
    int length = sizeof(null_data_format);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_GET_EMBMS_NETWORK_TIME, length);
    return new ModemData((char *)&req, length);
}

