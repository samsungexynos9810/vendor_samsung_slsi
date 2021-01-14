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

LOCAL_BUILD_EXECUTABLE := false

LOCAL_SRC_FILES:= fdm.cpp resource.cpp

LOCAL_SHARED_LIBRARIES := \
    libutils \
    liblog

LOCAL_SHARED_LIBRARIES += \
    vendor.samsung_slsi.telephony.hardware.oemservice@1.0 \
    libhidlbase  \
    libhidltransport \
    libhwbinder

LOCAL_CPPFLAGS := -Wno-unused-parameter
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(LOCAL_BUILD_EXECUTABLE),true)
LOCAL_MODULE:= fdm
include $(BUILD_EXECUTABLE)
else
LOCAL_CPPFLAGS += -D__FACTORY_DM_SLIB__
LOCAL_MODULE:= libfdm
include $(BUILD_SHARED_LIBRARY)
endif
