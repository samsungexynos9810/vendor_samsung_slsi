#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := sced
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := sced.rc
LOCAL_SRC_FILES := \
    sced.cpp \
    OemServiceManager.cpp

LOCAL_SHARED_LIBRARIES := liblog libcutils

LOCAL_C_INCLUDES := \
        system/core/include \
        system/core/libcutils/include \
        system/core/libutils/include

LOCAL_CPPFLAGS += -Wno-unused-parameter

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)
