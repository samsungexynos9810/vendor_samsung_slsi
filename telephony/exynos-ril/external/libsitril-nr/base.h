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
 * base.h
 *
 */

#ifndef __BASE_H__
#define __BASE_H__

#include <sys/types.h>
#include <stdio.h>
#include <map>
#include <string>
#include <list>

// to be enabled verbose log
// should be defined before <utils/Log.h>
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "libsitril-nr"
#endif

using namespace std;

#ifdef RIL_API_EXPORT
#undef RIL_API_EXPORT
#endif

#ifdef __cplusplus
#define RIL_API_EXPORT extern "C"
#else
#define RIL_API_EXPORT
#endif

typedef void * HANDLE;

#define    DECLARE_MODULE_TAG()    static const char *TAG;
#define    IMPLEMENT_MODULE_TAG(theClassName, theTag)    const char *theClassName::TAG = #theTag;


#endif /* __BASE_H__ */
