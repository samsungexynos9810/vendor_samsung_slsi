LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_PLATFORM_32BIT),true)
use_arch :=
else
use_arch := 32
endif

$(info sctd arch is $(use_arch))

##sctd
include $(CLEAR_VARS)
LOCAL_SRC_FILES_32 := sctd32
LOCAL_SRC_FILES_64 := sctd
LOCAL_MODULE := sctd
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := sctd.rc
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)


