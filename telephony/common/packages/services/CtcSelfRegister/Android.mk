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

LOCAL_PACKAGE_NAME := CtcSelfRegister
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_DEX_PREOPT := false

LOCAL_PROGUARD_ENABLED := disabled

# only build for CTC binary // remove comment when release
ifneq (, $(filter $(TARGET_BUILD_CARRIER), ctc chnopen))
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_CERTIFICATE := platform
LOCAL_JAVA_LIBRARIES += telephony-common

include $(BUILD_PACKAGE)
endif

