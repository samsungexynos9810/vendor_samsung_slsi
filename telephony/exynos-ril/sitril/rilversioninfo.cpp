/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version_info.h"
#include "build_info_defs.h"
#include "rilversioninfo.h"

static const char* VersionString = VERSION_STRING;

static const char VersionInfo[] =
{
    VERSION_MAJOR_INIT,
    '.',
    VERSION_MINOR_INIT,
    '\0'
};

static const char BuildInfo[] =
{
    BUILD_YEAR_CH0, BUILD_YEAR_CH1, BUILD_YEAR_CH2, BUILD_YEAR_CH3,
    '-',
    BUILD_MONTH_CH0, BUILD_MONTH_CH1,
    '-',
    BUILD_DAY_CH0, BUILD_DAY_CH1,
    ' ',
    BUILD_HOUR_CH0, BUILD_HOUR_CH1,
    ':',
    BUILD_MIN_CH0, BUILD_MIN_CH1,
    ':',
    BUILD_SEC_CH0, BUILD_SEC_CH1,
    '\0'
};

char RILVersionInfo::mVersion[128];

RILVersionInfo::RILVersionInfo()
{
}

RILVersionInfo::~RILVersionInfo()
{
}

const char* RILVersionInfo::getString(bool includeBuildInfo)
{
    memset(mVersion, 0, sizeof(mVersion));
    if ( includeBuildInfo == true )
    {
        snprintf(mVersion, sizeof(mVersion) - 1, "%s V%s Build %s", VersionString, VersionInfo, BuildInfo);
    }
    else
    {
        snprintf(mVersion, sizeof(mVersion) - 1, "%s V%s", VersionString, VersionInfo);
    }
    return (const char*)mVersion;
}

const char* RILVersionInfo::getBuildTime()
{
    return BuildInfo;
}
