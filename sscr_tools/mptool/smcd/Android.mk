LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_PLATFORM_32BIT),true)
use_arch :=
else
use_arch := 32
endif

$(info smcd arch is $(use_arch))

##smcd
include $(CLEAR_VARS)
LOCAL_SRC_FILES_32 := smcd32
LOCAL_SRC_FILES_64 := smcd
LOCAL_MODULE := smcd
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := smcd.rc
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)


