#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#
# This code is provided as it is, For sample and test purpose.

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PACKAGE_NAME := TetherProvisioning
LOCAL_PRIVATE_PLATFORM_APIS := true

ifneq (, $(filter $(TARGET_BUILD_CARRIER), spr tmo))
#LOCAL_SDK_VERSION := current // To use android.os.SystemProperties

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_RESOURCE_DIR := \
        frameworks/support/v7/appcompat/res \
        $(LOCAL_PATH)/res

LOCAL_AAPT_FLAGS += --auto-add-overlay

LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

include $(BUILD_PACKAGE)
endif
