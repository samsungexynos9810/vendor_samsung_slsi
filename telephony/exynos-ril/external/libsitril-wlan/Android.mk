#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

USE_RILCLIENT_DYNAMIC_LOAD := false

LOCAL_MODULE_TAGS := optional

LOCAL_CPPFLAGS += -Wno-unused-parameter

LOCAL_C_INCLUDES := bionic \
                    external/stlport/stlport
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libsitril-client
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../sitril/protocol/sit/
#LOCAL_C_INCLUDES += hardware/ril/include

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libcutils \
    libbinder \
    libhardware_legacy \
    libdl

LOCAL_SRC_FILES :=
LOCAL_SRC_FILES += SITRilWLan.cpp

ifneq ($(USE_RILCLIENT_DYNAMIC_LOAD),true)
    LOCAL_SHARED_LIBRARIES += libsitril-client
else
    LOCAL_CPPFLAGS += -D_USE_RILCLIENT_DL_
endif

ifeq ($(SIM_COUNT), 2)
    LOCAL_CFLAGS += -DANDROID_MULTI_SIM
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_2
endif

LOCAL_MODULE:= libsitril-wlan
LOCAL_PRELINK_MODULE := false
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
