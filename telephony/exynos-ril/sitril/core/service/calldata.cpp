/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include "rillog.h"
#include "cscservice.h"
#include "rildatabuilder.h"
#include "calldata.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define writeRilEvent(format1, format2, ...) CRilEventLog::writeRilEvent(RIL_LOG_CAT_CALL, format1, format2, ##__VA_ARGS__)

ClirInfo::ClirInfo()
{
    m_aoc = CLIR_DEFAULT;
}

void ClirInfo::SetClirAoc(int aoc, RIL_SOCKET_ID socketid)
{
    if (CLIR_DEFAULT <= aoc && aoc <= CLIR_SUPPRESSION)
    {
        char szProp[64] = { 0, };
        sprintf(szProp, "persist.radio.csc.clir_aoc_%d", (int)socketid);
        m_aoc = aoc;
        char szBuff[PROP_VALUE_MAX];
        memset(szBuff, 0x00, sizeof(szBuff));
        snprintf(szBuff, sizeof(szBuff)-1, "%d", m_aoc);

        RilLogV("set property : %s : %s", szProp, szBuff);
        property_set(szProp, szBuff);
    }
}

int ClirInfo::GetClirAoc(RIL_SOCKET_ID socketid)
{
    char szProp[64] = { 0, };
    sprintf(szProp, "persist.radio.csc.clir_aoc_%d", (int)socketid);
    char szBuff[PROP_VALUE_MAX];
    memset(szBuff, 0x00, sizeof(szBuff));
    property_get(szProp, szBuff, "0");
    m_aoc = atoi(szBuff);
    return m_aoc;
}
