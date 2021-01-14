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

#ifndef __RFS_LOG_H__
#define __RFS_LOG_H__

#define LOG_TAG "RFSD"
#define LOG_NDEBUG 1

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <utils/Log.h>
#include <unistd.h>
#include <time.h>

#define RFS_LOG_PATH        "/mnt/vendor/efs/err/"
#define MAX_FILE_NAME       128
#define DEFAULT_FILE_NAME   "rfslog"

class CRfsLog
{
    public:
        static void WriteRfsLog(const char* const func);
        static void WriteRfsLog(const char* const func, const char* const param, ...);
};

#endif // __RFS_LOG_H__
