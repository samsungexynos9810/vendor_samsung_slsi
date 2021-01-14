/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file log.h
    @brief NPU log header
*/
#ifndef COMMON_LOG_H_
#define COMMON_LOG_H_

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EDEN"
#define EDEN_VERSION "v1.4.0"
#define VERSION_MAJOR 1
#define VERSION_MINOR 4
#define VERSION_BUILD 0

enum _eden_log_flags {
    EDEN_FORCE = 0,
    EDEN_NN = 1,
    EDEN_RT = 2,
    EDEN_UD = 3,
    EDEN_LINK = 4,
    EDEN_EMA = 5,
    EDEN_GTEST = 6,
    EDEN_HIDL = 7,
    EDEN_RT_STUB = 8,
    EDEN_DRIVER = 9,
    EDEN_CL = 10,
    EDEN_MAX_FLAG = 11,
};

#define EDEN_LOG_FORCE       (1 << EDEN_FORCE)
#define EDEN_LOG_NN          (1 << EDEN_NN)
#define EDEN_LOG_RT          (1 << EDEN_RT)
#define EDEN_LOG_UD          (1 << EDEN_UD)
#define EDEN_LOG_LINK        (1 << EDEN_LINK)
#define EDEN_LOG_EMA         (1 << EDEN_EMA)
#define EDEN_LOG_GTEST       (1 << EDEN_GTEST)
#define EDEN_LOG_HIDL        (1 << EDEN_HIDL)
#define EDEN_LOG_RT_STUB     (1 << EDEN_RT_STUB)
#define EDEN_LOG_DRIVER      (1 << EDEN_DRIVER)

/*
 * comment out to mask
 */
static int glive_log = \
    EDEN_LOG_FORCE | \
    EDEN_LOG_NN | \
    EDEN_LOG_RT | \
    EDEN_LOG_UD | \
    EDEN_LOG_LINK | \
    EDEN_LOG_EMA | \
    EDEN_LOG_GTEST | \
    EDEN_LOG_HIDL | \
    EDEN_LOG_RT_STUB | \
    EDEN_LOG_DRIVER;

#define IS_LOG_ON(FLAG) \
    ((glive_log & (FLAG)) != 0)

#ifdef REMOVE_LOG
#define LOGD(FLAG, ...) \
    {                   \
        ;               \
    }
#define LOGI(FLAG, ...) \
    {                   \
        ;               \
    }
#define LOGE(FLAG, ...) \
    {                   \
        ;               \
    }
#define LOGW(FLAG, ...) \
    {                   \
        ;               \
    }
#else  // REMOVE_LOG

#ifdef EDEN_DEBUG
#if defined(LINUX_LOG)
#include <stdio.h>
#define LOGD(FLAG, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
        fprintf(stdout, "[Exynos][EDEN][%s][%s][D] %s:%d: ", EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__); fprintf(stdout, __VA_ARGS__); \
    } \
    }
#define LOGI(FLAG, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
        fprintf(stdout, "[Exynos][EDEN][%s][%s][I] %s:%d: ", EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__); fprintf(stdout, __VA_ARGS__); \
    } \
    }
#define LOGE(FLAG, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
      fprintf(stdout, "\n[Exynos][EDEN][%s][%s][ERR] %s:%d: ", EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); \
    } \
    }
#define LOGW(FLAG, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
      fprintf(stdout, "\n[Exynos][EDEN][%s][%s][WARN] %s:%d: ", EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); \
    } \
    }
/** @todo add android log lib */
#elif defined(ANDROID_LOG)
#include <android/log.h>
#define LOGD(FLAG, fmt, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
        __android_log_print(ANDROID_LOG_DEBUG, "EDEN", "[Exynos][EDEN][%s][%s] %s:%d: " fmt, EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__, \
                ##__VA_ARGS__); \
    } \
    }
#define LOGI(FLAG, fmt, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
        __android_log_print(ANDROID_LOG_INFO, "EDEN", "[Exynos][EDEN][%s][%s] %s:%d: " fmt, EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__, \
                ##__VA_ARGS__); \
    } \
    }
#define LOGE(FLAG, fmt, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
        __android_log_print(ANDROID_LOG_ERROR, "EDEN", "[Exynos][EDEN][%s][%s] %s:%d: " fmt, EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__, \
                ##__VA_ARGS__); \
    } \
    }
#define LOGW(FLAG, fmt, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
        __android_log_print(ANDROID_LOG_WARN, "EDEN", "[Exynos][EDEN][%s][%s] %s:%d: " fmt, EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__, \
                ##__VA_ARGS__); \
    } \
    }
#else  // defined(ANDROID_LOG)
/** @todo notify proper error */
#define LOGD(...) (void)0
#define LOGI(...) (void)0
#define LOGE(...) (void)0
#endif  // __linux__
#else  //! EDEN_DEBUG

#if defined(LINUX_LOG)
#include <stdio.h>
#define LOGI(FLAG, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
        fprintf(stdout, "[Exynos][EDEN][%s][%s][I] %s:%d: ", EDEN_VERSION, LOG_TAG,  __FUNCTION__, __LINE__); fprintf(stdout, __VA_ARGS__); \
    } \
    }
#define LOGE(FLAG, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
      fprintf(stdout, "\n[Exynos][EDEN][%s][%s][ERR] %s:%d: ", EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); \
    } \
    }
#define LOGW(FLAG, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
      fprintf(stdout, "\n[Exynos][EDEN][%s][%s][WARN] %s:%d: ", EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__); fprintf(stdout, __VA_ARGS__); fprintf(stdout, "\n"); \
    } \
    }
/** @todo add android log lib */
#elif defined(ANDROID_LOG)
#include <android/log.h>
#define LOGI(FLAG, fmt, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
        __android_log_print(ANDROID_LOG_INFO, "EDEN", "[Exynos][EDEN][%s][%s] %s:%d: " fmt, EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__, \
                ##__VA_ARGS__); \
    } \
    }
#define LOGE(FLAG, fmt, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
        __android_log_print(ANDROID_LOG_ERROR, "EDEN", "[Exynos][EDEN][%s][%s] %s:%d: " fmt, EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__, \
                ##__VA_ARGS__); \
    } \
    }
#define LOGW(FLAG, fmt, ...) \
    { \
    if (!IS_LOG_ON(FLAG)) { \
    } else { \
        __android_log_print(ANDROID_LOG_WARN, "EDEN", "[Exynos][EDEN][%s][%s] %s:%d: " fmt, EDEN_VERSION, LOG_TAG, __FUNCTION__, __LINE__, \
                ##__VA_ARGS__); \
    } \
    }
#else  // defined(ANDROID_LOG)
#define LOGI(...) (void)0
#define LOGE(...) (void)0
#define LOGW(...) (void)0
#endif  // Android

#define LOGD(...) (void)0
#endif  // EDEN_DEBUG

#endif  // REMOVE_LOG

#endif  // COMMON_LOG_H_
