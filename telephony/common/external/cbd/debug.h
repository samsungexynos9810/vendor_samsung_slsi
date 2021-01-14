/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __CBD_DEBUG_H__
#define __CBD_DEBUG_H__

#include <log/log.h>
#include "exynos_protocol.h"

#define ERR2STR     strerror(errno)
#define FMT_ERR     "CBD ERR!!! %s: "
#define FMT_CBD     "CBD:"

/******************************************************************************
 *  Extern functions
 ******************************************************************************/

int dprintf(int fd, const char *format, ...);
int get_kmsg_fd(void);

/* #define DEBUG_KERNEL_MSG */

#ifdef DEBUG_KERNEL_MSG
#define cbd_info(s, args...)    dprintf(get_kmsg_fd(), FMT_CBD s, ##args)
#define cbd_err(s, args...)    dprintf(get_kmsg_fd(), FMT_ERR s, __func__, ##args)
#define cbd_logcat_debug(...) __android_log_print(ANDROID_LOG_DEBUG, "CBD", __VA_ARGS__)
#define cbd_logcat_err(...)   __android_log_print(ANDROID_LOG_ERROR, "CBD", __VA_ARGS__)
#else
#define cbd_err(s, args...)    ALOGE(FMT_ERR s, __func__, ##args)
#define cbd_info(s, args...)    ALOGI(FMT_CBD s, ##args)
#define cbd_logcat_debug(...) __android_log_print(ANDROID_LOG_DEBUG, "CBD", __VA_ARGS__)
#define cbd_logcat_err(...)   __android_log_print(ANDROID_LOG_ERROR, "CBD", __VA_ARGS__)
#endif

#endif //__CBD_DEBUG_H__
