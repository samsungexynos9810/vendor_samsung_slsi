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

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include
#LOCAL_C_INCLUDES += hardware/ril/include

LOCAL_SRC_FILES:= sitril-client.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libbinder \
    libcutils \
    libhardware_legacy

LOCAL_SHARED_LIBRARIES += vendor.samsung_slsi.telephony.hardware.radioExternal@1.0 \
    libhidlbase  \
    libhidltransport \
    libhwbinder

ifeq ($(SIM_COUNT), 2)
    LOCAL_CFLAGS += -DANDROID_MULTI_SIM
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_2
endif

LOCAL_MODULE:= libsitril-client
LOCAL_PRELINK_MODULE := false
LOCAL_PROPRIETARY_MODULE := true

# add product feature
RELATIVE_CUR_PATH := ../../productfeature
include $(LOCAL_PATH)/$(RELATIVE_CUR_PATH)/productfeature.mk
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include
#LOCAL_C_INCLUDES += hardware/ril/include

LOCAL_SRC_FILES:= sitril-client.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libbinder \
    libcutils \
    libhardware_legacy

LOCAL_SHARED_LIBRARIES += vendor.samsung_slsi.telephony.hardware.radioExternal@1.0 \
    libhidlbase  \
    libhidltransport \
    libhwbinder

LOCAL_MODULE:= libsitril-client-system
LOCAL_PRELINK_MODULE := false

# add product feature
RELATIVE_CUR_PATH := ../../productfeature
include $(LOCAL_PATH)/$(RELATIVE_CUR_PATH)/productfeature.mk

include $(BUILD_SHARED_LIBRARY)
