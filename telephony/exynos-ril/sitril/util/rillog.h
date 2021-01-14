/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _RIL_LOG_H_
#define _RIL_LOG_H_

#include <stdio.h>
#include "types.h"
#include "util.h"

#define LOG_LEVEL_PROP          "persist.vendor.ril.debug_level"
#define LOG_LEVEL_PROP_LOW      "0x6c65"
#define LOG_LEVEL_PROP_MID      "0x6d69"
#define LOG_LEVEL_PROP_HIGH     "0x6876"

#define EVENT_LOG_CATEGORY_PROP    "persist.vendor.radio.log.event"

#define LOG_CATEGORY_PROP    "persist.vendor.radio.log.categorymask"
#define RIL_RESET_PROP      "vendor.radio.ril.reset_count"

enum {
    RIL_LOG_CAT_CORE = 1,
    RIL_LOG_CAT_CALL = 1<<1,
    RIL_LOG_CAT_SMS = 1<<2,
    RIL_LOG_CAT_SIM = 1<<3,
    RIL_LOG_CAT_NET = 1<<4,    //16
    RIL_LOG_CAT_PDP = 1<<5,
    RIL_LOG_CAT_MISC = 1<<6,
    RIL_LOG_CAT_SOUND = 1<<7,    //128
    RIL_LOG_CAT_OEM = 1<<8,
    RIL_LOG_CAT_RFS = 1<<9,
    RIL_LOG_CAT_IMS = 1<<10,
    RIL_LOG_CAT_GPS = 1<<11,
    RIL_LOG_CAT_WLAN = 1<<12,
    RIL_LOG_CAT_AIMS = 1<<13,
    RIL_LOG_CAT_VSIM = 1<<14,
    RIL_LOG_CAT_NETIF = 1<<15,
    RIL_LOG_CAT_NETL = 1<<16,
    RIL_LOG_CAT_TELPRO = 1<<17,
    RIL_LOG_CAT_OPERTABLE = 1<<18,
    RIL_LOG_CAT_DATA = 1<<19,
    RIL_LOG_CAT_ETC = 1<<20,
    RIL_LOG_CAT_STK = 1<<21,
    RIL_LOG_CAT_EMBMS = 1<<22,
};

#define RilLogE(format, ...) CRilLog::Log(CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)
#define RilLogW(format, ...) CRilLog::Log(CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)

#define RilLog  RilLogI

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG     "RIL"
#include <stdarg.h>

#ifdef HAVE_ANDROID_OS
    #include <utils/Log.h>
#else

#ifdef LOGD
#undef LOGD
#define LOGD(...) {}
#else
#include <stdio.h>
#define LOGD(...) { \
    printf("[%s] ", LOG_TAG); \
    printf(__VA_ARGS__); \
    printf("\n"); \
}
#endif
#endif

//#define LOGV(format, ...)        RilLogV("%s::%s() " format, m_szSvcName, __FUNCTION__, __VA_ARGS__)
//#define LOGD(format, ...)        RilLogV("%s::%s() " format, m_szSvcName, __FUNCTION__, __VA_ARGS__)
//#define LOGE(format, ...)        RilLogE("%s::%s() " format, m_szSvcName, __FUNCTION__, __VA_ARGS__)


#include "mutex.h"

class RilLogCapture;

class CRilLog
{
public:
    CRilLog();

    static void InitRilLog();
    static void Log(int level, const char* const szFormatString, ...);
    static void Log(int category, int level, const char* const szFormatString, ...);
    static char** BufferedLog(char** buffer, const char* const szFormatString, ...);
    static void BufferedLogFlash(int category, int level, char** buffer);
    enum
    {
        E_RIL_VERBOSE_LOG  = 0x01,
        E_RIL_INFO_LOG     = 0x02,
        E_RIL_WARNING_LOG  = 0x04,
        E_RIL_CRITICAL_LOG = 0x08
    };

    static void DumpResetLog(const char* reset_reason);
    static const char* GetParentLogPath(void);
public:
    static const UINT32 m_uiMaxLogBufferSize = 1024;
    static const UINT32 m_uiMaxBufferedCount = 30;
private:
    static int m_PrintCategory;
    static int m_logLevel;
    static BOOL  m_bInitialized;
protected:
    static RilLogCapture *m_pRilLogCapture;
};

class CRilEventLog
{
public:
    CRilEventLog();

    static void InitRilEventLog();
    static void writeRilEvent(int category, const char* const service, const char* const func);
    static void writeRilEvent(int category, const char* const service, const char* const func, const char* const param, ...);

    static char filestr[100];
    static int m_PrintEventCate;
};

#endif /*_RIL_LOG_H_*/
