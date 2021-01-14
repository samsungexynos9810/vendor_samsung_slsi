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
LOCAL_MODULE := wland
LOCAL_INIT_RC := wland.rc
LOCAL_SRC_FILES := wland.cpp FaultIds.cpp RouteMonitor.cpp wlbtlog.cpp filedir.cpp common.cpp mxlog.cpp udilog.cpp livemxlog.cpp

LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_C_INCLUDES := \
        system/core/libcutils/include \
        system/core/libutils/include

LOCAL_PROPRIETARY_MODULE := true
LOCAL_CPPFLAGS += -Wno-error=date-time
LOCAL_CPPFLAGS += -fexceptions
LOCAL_CPPFLAGS += -O2
include $(BUILD_EXECUTABLE)
