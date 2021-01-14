/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#define LOG_NDEBUG 0        // To enable VERBOSE LOG    //jhdaniel.kim

#include <utils/Log.h>
#include <cutils/properties.h>
#include <time.h>
#include "rillog.h"
#include "rilversioninfo.h"
#include "rilproperty.h"
#include "rillogcapture.h"

int CRilLog::m_PrintCategory = 0xffffff;
int CRilLog::m_logLevel = 0;
BOOL CRilLog::m_bInitialized = FALSE;
RilLogCapture *CRilLog::m_pRilLogCapture = NULL;

#ifndef PROP_VALUE_MAX
#define PROP_VALUE_MAX 92
#endif

CMutex gRilLogMutex;

void CRilLog::InitRilLog()
{
#if 0
    char szLogLevelBuf[PROP_VALUE_MAX] = {0};
    memset(szLogLevelBuf, 0, sizeof(szLogLevelBuf));

    property_get((char *)LOG_LEVEL_PROP, szLogLevelBuf, "");
    if (0 == strcmp(szLogLevelBuf, "")) {
        property_set((char *)LOG_LEVEL_PROP, LOG_LEVEL_PROP_HIGH);
        property_get((char *)LOG_LEVEL_PROP, szLogLevelBuf, LOG_LEVEL_PROP_HIGH);
    }

    char build_type[PROP_VALUE_MAX] = {0};
    memset(build_type, 0, sizeof(build_type));
    property_get(RO_BUILD_TYPE, build_type, "");

    if(0 == strcmp(build_type, "user")) {
        m_logLevel = E_RIL_WARNING_LOG;
    } else {
        if(0 == strcmp(szLogLevelBuf, LOG_LEVEL_PROP_LOW))
            m_logLevel = E_RIL_CRITICAL_LOG;
        else if (0 == strcmp(szLogLevelBuf, LOG_LEVEL_PROP_MID))
            m_logLevel = E_RIL_INFO_LOG;
        else if (0 == strcmp(szLogLevelBuf, LOG_LEVEL_PROP_HIGH))
            m_logLevel = E_RIL_VERBOSE_LOG;
        else
            m_logLevel = E_RIL_WARNING_LOG;
    }

    property_get((char *)LOG_CATEGORY_PROP, szLogLevelBuf, "");
    if (0 == strcmp(szLogLevelBuf, "")) {
        property_set((char *)LOG_CATEGORY_PROP, "16777215");
        property_get((char *)LOG_CATEGORY_PROP, szLogLevelBuf, "16777215");
    }
    m_PrintCategory = atoi(szLogLevelBuf);    //0xffffff

    m_bInitialized = TRUE;

    if (0 == strcmp(build_type, "eng")) {
        m_pRilLogCapture = RilLogCapture::MakeInstance();
        if (m_pRilLogCapture == NULL || m_pRilLogCapture->Start() < 0) {
            RilLog("%s: fail to make the thread for rilLogCapture", __FUNCTION__);
        }
    }
#else
    RilLog("%s", __FUNCTION__);
    char build_type[PROP_VALUE_MAX] = {0};
    memset(build_type, 0, sizeof(build_type));
    property_get(RO_BUILD_TYPE, build_type, "");

    if(0 == strcmp(build_type, "user")) {
        m_logLevel = E_RIL_WARNING_LOG;
    } else {
        m_logLevel = E_RIL_VERBOSE_LOG;
    }
    m_bInitialized = TRUE;
#endif

    RilLog("%s: Log level is %d", __FUNCTION__, m_logLevel);
    RilLog("%s: Log Category is %d", __FUNCTION__, m_PrintCategory);
    RilLog("%s: RIL library build time : %s", __FUNCTION__, RILVersionInfo::getBuildTime());
}

void CRilLog::Log(int level, const char* const szFormatString, ...)
{
    if ( m_bInitialized == false
        || m_logLevel > level )
    {
        return;
    }

    va_list argList;
    char szLogText[m_uiMaxLogBufferSize];

    va_start(argList, szFormatString);
    vsnprintf(szLogText, m_uiMaxLogBufferSize, szFormatString, argList);
    va_end(argList);

    if (m_pRilLogCapture != NULL) {
        m_pRilLogCapture->notifyNewRilLog(szLogText);
    }

    gRilLogMutex.lock();
    switch(level)
    {
    case E_RIL_CRITICAL_LOG:
        ALOGE("%s", szLogText); break;
    case E_RIL_WARNING_LOG:
        ALOGW("%s", szLogText); break;
    case E_RIL_VERBOSE_LOG:
        ALOGV("%s", szLogText); break;
    case E_RIL_INFO_LOG:
    default:
        ALOGI("%s", szLogText); break;
    }
    gRilLogMutex.unlock();
}

void CRilLog::Log(int category, int level, const char* const szFormatString, ...)
{
    if ( m_bInitialized == false
        || m_logLevel > level
        || (m_PrintCategory != 0xffffff && ((m_PrintCategory&category) == 0)) )
    {
        return;
    }

    va_list argList;
    char szLogText[m_uiMaxLogBufferSize];

    va_start(argList, szFormatString);
    vsnprintf(szLogText, m_uiMaxLogBufferSize, szFormatString, argList);
    va_end(argList);

    if (m_pRilLogCapture != NULL) {
        m_pRilLogCapture->notifyNewRilLog(szLogText);
    }

    gRilLogMutex.lock();
    switch(level)
    {
    case E_RIL_CRITICAL_LOG:
        ALOGE("%s", szLogText); break;
    case E_RIL_WARNING_LOG:
        ALOGW("%s", szLogText); break;
    case E_RIL_VERBOSE_LOG:
        ALOGV("%s", szLogText); break;
    case E_RIL_INFO_LOG:
    default:
        ALOGI("%s", szLogText); break;
    }
    gRilLogMutex.unlock();
}

char** CRilLog::BufferedLog(char** ppbuffer, const char* const szFormatString, ...)
{
    if ( ppbuffer == NULL )
    {
        ppbuffer = new char*[m_uiMaxBufferedCount];
        if ( ppbuffer == NULL )
        {
            return NULL;
        }
        for ( unsigned int i = 0; i < m_uiMaxBufferedCount; i++ )
        {
            ppbuffer[i] = NULL;
        }
    }
    va_list argList;
    char szLogText[m_uiMaxLogBufferSize];

    va_start(argList, szFormatString);
    vsnprintf(szLogText, m_uiMaxLogBufferSize, szFormatString, argList);
    va_end(argList);

    if (m_pRilLogCapture != NULL) {
        m_pRilLogCapture->notifyNewRilLog(szLogText);
    }

    unsigned int new_len = strlen(szLogText);
    if ( new_len > m_uiMaxLogBufferSize )
    {
        ALOGE("RIL log size is over buffer size, Truncate overflowed logs");
        return ppbuffer;
    }

    unsigned int i = 0;
    for ( i = 0; i < m_uiMaxBufferedCount; i++ )
    {
        if ( ppbuffer[i] == NULL )
        {
            break;
        }
    }

    // if log is over 8 * buffer size, just return current saved buffer
    if ( i == m_uiMaxBufferedCount )
    {
        ALOGE("RIL log size is over buffer count, Truncate overflowed logs");
        return ppbuffer;
    }


    ppbuffer[i] = new char[m_uiMaxLogBufferSize];
    if ( ppbuffer[i] == NULL )
    {
        ALOGE("log buffer allocation fail, return current buffer pointer : there can be missing logs");
        return ppbuffer;
    }

    SECURELIB::strncpy(ppbuffer[i], m_uiMaxLogBufferSize, szLogText, SECURELIB::strlen(szLogText));

    return ppbuffer;
}


void CRilLog::BufferedLogFlash(int category, int level, char** ppbuffer)
{
    if ( ppbuffer == NULL )
    {
        return;
    }

    if ( m_bInitialized == false
        || m_logLevel > level
        || (m_PrintCategory != 0xffffff && ((m_PrintCategory&category) == 0)) )
    {
        for ( unsigned int i = 0; i < m_uiMaxBufferedCount; i++ )
        {
            if ( ppbuffer[i] != NULL )
            {
                delete[] ppbuffer[i];
            }
        }
        delete [] ppbuffer;
        return;
    }

    gRilLogMutex.lock();
    for ( unsigned int i = 0; i < m_uiMaxBufferedCount; i++ )
    {
        if ( ppbuffer[i] == NULL )
        {
            break;
        }
        switch(level)
        {
        case E_RIL_CRITICAL_LOG:
            ALOGE("%s", ppbuffer[i]); break;
        case E_RIL_WARNING_LOG:
            ALOGW("%s", ppbuffer[i]); break;
        case E_RIL_VERBOSE_LOG:
            ALOGV("%s", ppbuffer[i]); break;
        case E_RIL_INFO_LOG:
        default:
            ALOGI("%s", ppbuffer[i]); break;
        }
    }
    gRilLogMutex.unlock();

    for ( unsigned int i = 0; i < m_uiMaxBufferedCount; i++ )
    {
        if ( ppbuffer[i] != NULL )
        {
            delete[] ppbuffer[i];
        }
    }
    delete [] ppbuffer;
}

void CRilLog::DumpResetLog(const char* reset_reason)
{
    char szResetCnt[PROP_VALUE_MAX] = {0};
    memset(szResetCnt, 0, sizeof(szResetCnt));

    property_get((char*)RIL_RESET_PROP, szResetCnt, "0");
    int nResetCnt = strtol(szResetCnt, NULL, 10);
    RilLog("%s: RIL reset count : %d", __FUNCTION__, nResetCnt);
    snprintf(szResetCnt, sizeof(szResetCnt)-1, "%d", nResetCnt+1);
    property_set((char*)RIL_RESET_PROP, szResetCnt);
}

const char* CRilLog::GetParentLogPath(void)
{
    //return "/data/local/tmp";
    return "/data/vendor/dump";
}

char CRilEventLog::filestr[100] = {0,};
struct tm *t;
time_t timer = time(NULL);
FILE *ril_event_file = NULL;
int CRilEventLog::m_PrintEventCate = 0xffff;
int preDate = 0;

void CRilEventLog::InitRilEventLog()
{
#if 0
    char szEventLogBuf[PROP_VALUE_MAX] = {0,};
    memset(szEventLogBuf, 0, sizeof(szEventLogBuf));

    property_get((char *)EVENT_LOG_CATEGORY_PROP, szEventLogBuf, "");

    if (0 == strcmp(szEventLogBuf, "")) {
        property_set((char *)EVENT_LOG_CATEGORY_PROP, "32767");
        property_get((char *)EVENT_LOG_CATEGORY_PROP, szEventLogBuf, "32767");
    }

    m_PrintEventCate = atoi(szEventLogBuf);

    char build_type[PROP_VALUE_MAX] = {0};
    memset(build_type, 0, sizeof(build_type));
    property_get(RO_BUILD_TYPE, build_type, "");

    if (0 == strcmp(build_type, "user")) {
        m_PrintEventCate = 0;
    }

    t = localtime(&timer);
    preDate = t->tm_mday;
    sprintf(filestr, "/data/vendor/dump/ril_event_%04d%02d%02d_%02d%02d%02d.log", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
#endif
}

void CRilEventLog::writeRilEvent(int category, const char* const service, const char* const func)
{
#if 0
    if ((m_PrintEventCate&category) == 0)
        return;

    timer = time(NULL);
    t = localtime(&timer);

    if (preDate != t->tm_mday) {
        preDate = t->tm_mday;
        sprintf(filestr, "/data/vendor/dump/ril_event_%04d%02d%02d_%02d%02d%02d.log", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    }

    ril_event_file = fopen(filestr, "a+");

    if(NULL != ril_event_file) {
        fprintf(ril_event_file, "%04d%02d%02d-%02d:%02d:%02d\t%-30s\t%-35s\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, service, func);
        fclose(ril_event_file);
    }
#endif
}

void CRilEventLog::writeRilEvent(int category, const char* const service, const char* const func, const char* const param, ...)
{
#if 0
    if ((m_PrintEventCate&category) == 0)
        return;

    timer = time(NULL);
    t = localtime(&timer);

    if (preDate != t->tm_mday) {
        preDate = t->tm_mday;
        sprintf(filestr, "/data/vendor/dump/ril_event_%04d%02d%02d_%02d%02d%02d.log", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    }

    ril_event_file = fopen(filestr, "a+");

    if(NULL != ril_event_file) {
        char buf[512] = {0, };
        va_list ap;

        va_start(ap, param);
        vsprintf(buf + strlen(buf), param, ap);
        va_end(ap);
        fprintf(ril_event_file, "%04d%02d%02d-%02d:%02d:%02d\t%-30s\t%-35s\t%s\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, service, func, buf);
        fclose(ril_event_file);
    }
#endif
}
