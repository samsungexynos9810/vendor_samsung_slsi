LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_PLATFORM_32BIT),true)
use_arch :=
else
use_arch := 32
endif

$(info sfmd arch is $(use_arch))

##sfmd
include $(CLEAR_VARS)
LOCAL_SRC_FILES_32 := sfmd32
LOCAL_SRC_FILES_64 := sfmd
LOCAL_MODULE := sfmd
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := sfmd.rc
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)


