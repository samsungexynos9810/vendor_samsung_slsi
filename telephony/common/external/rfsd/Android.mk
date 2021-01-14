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

LOCAL_MODULE := rfsd
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := rfsd.rc
LOCAL_SRC_FILES := \
    rfsservice.cpp \
    rfsfile.cpp \
    rfschannel.cpp \
    modemstatemonitor.cpp \
    rfslog.cpp \
    main.cpp
LOCAL_CPPFLAGS += -fexceptions -Wno-unused-parameter
LOCAL_SRC_FILES += base/thread.cpp
#LOCAL_SRC_FILES += util/rillog.cpp

LOCAL_C_INCLUDES := \
#	bionic \
#	external/openssl/include \

LOCAL_C_INCLUDES += $(LOCAL_PATH)/ \
					$(LOCAL_PATH)/base/ \
                    system/core/libutils/include \
                    system/core/libcutils/include

LOCAL_SHARED_LIBRARIES := libcrypto \
	libcutils \
	liblog \

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_EXECUTABLE)
