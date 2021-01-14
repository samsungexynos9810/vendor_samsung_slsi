LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_PLATFORM_32BIT),true)
use_arch :=
dirs := 64
else
use_arch := 32
dirs :=
endif

$(info spad arch is $(use_arch))

##spad
include $(CLEAR_VARS)
LOCAL_SRC_FILES_32 := spad32
LOCAL_SRC_FILES_64 := spad
LOCAL_MODULE := spad
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := spad.rc
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_SRC_FILES_32 := lib32/libspad_core.so
LOCAL_SRC_FILES_64 := lib64/libspad_core.so
LOCAL_MODULE := libspad_core
LOCAL_MODULE_OWNER := samsung
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := $(dirs)
LOCAL_VENDOR_MODULE := true
include $(BUILD_PREBUILT)

#include $(CLEAR_VARS)
#LOCAL_SRC_FILES_32 := lib32/libspad_services.so
#LOCAL_SRC_FILES_64 := lib64/libspad_services.so
#LOCAL_MODULE := libspad_services
#LOCAL_MODULE_OWNER := samsung
#LOCAL_MODULE_SUFFIX := .so
#LOCAL_MODULE_CLASS := SHARED_LIBRARIES
#LOCAL_MODULE_TAGS := optional
#LOCAL_MULTILIB := $(dirs)
#LOCAL_VENDOR_MODULE := true
#include $(BUILD_PREBUILT)
#

