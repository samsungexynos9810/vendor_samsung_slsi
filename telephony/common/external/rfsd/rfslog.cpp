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
    RFSD Log
*/

#include "rfslog.h"

void CRfsLog::WriteRfsLog(const char* const func)
{
    char filestr[MAX_FILE_NAME];
    struct tm *t;
    time_t timer = time(NULL);

    if(chdir(RFS_LOG_PATH) < 0) {
        if(mkdir(RFS_LOG_PATH, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
            ALOGE("%s : mkdir %s create fail", __FUNCTION__, RFS_LOG_PATH);
            return;
        }
        else
            ALOGD("%s : %s", __FUNCTION__, RFS_LOG_PATH);
    }

    t = localtime(&timer);
    sprintf(filestr, "%s%s_%04d%02d%02d", RFS_LOG_PATH, DEFAULT_FILE_NAME, t->tm_year+1900, t->tm_mon+1, t->tm_mday);
    FILE *rfsLogFile = fopen(filestr, "a+");

    if(rfsLogFile != NULL) {
        fprintf(rfsLogFile, "%04d%02d%02d-%02d:%02d:%02d\t%-35s\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, func);
        fclose(rfsLogFile);
    }
}

void CRfsLog::WriteRfsLog(const char* const func, const char* const param, ...)
{
    char filestr[MAX_FILE_NAME];
    struct tm *t;
    time_t timer = time(NULL);

    if(chdir(RFS_LOG_PATH) < 0) {
        if(mkdir(RFS_LOG_PATH, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
            ALOGE("%s : mkdir %s create fail", __FUNCTION__, RFS_LOG_PATH);
            return;
        }
        else
            ALOGD("%s : %s", __FUNCTION__, RFS_LOG_PATH);
    }

    t = localtime(&timer);
    sprintf(filestr, "%s_%04d%02d%02d", DEFAULT_FILE_NAME, t->tm_year+1900, t->tm_mon+1, t->tm_mday);
    FILE *rfsLogFile = fopen(filestr, "a+");

    if(rfsLogFile != NULL) {
        char buf[512] = {0, };
        va_list ap;
        va_start(ap, param);
        vsprintf(buf + strlen(buf), param, ap);
        va_end(ap);

        fprintf(rfsLogFile, "%04d%02d%02d-%02d:%02d:%02d\t%-35s\t%s\n", t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, func, buf);
        fclose(rfsLogFile);
    }
}
