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

LOCAL_MODULE := dmd
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := dmd.rc
LOCAL_SRC_FILES := \
    dmd_main.cpp \
    dmd_socket.cpp \
    OemServiceManager.cpp \
    DMAgent.cpp \
    DMFileManager.cpp

LOCAL_CPPFLAGS += -Wno-unused-parameter

LOCAL_SHARED_LIBRARIES := libcutils liblog
LOCAL_SHARED_LIBRARIES += libziparchive

LOCAL_C_INCLUDES += system/core/include
LOCAL_C_INCLUDES += system/core/base/include

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)
