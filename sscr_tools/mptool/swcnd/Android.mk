LOCAL_PATH := $(call my-dir)

ifneq ($(TARGET_PLATFORM_32BIT),true)
use_arch :=
else
use_arch := 32
endif

$(info swcnd arch is $(use_arch))

##swcnd
include $(CLEAR_VARS)
LOCAL_SRC_FILES_32 := swcnd32
LOCAL_SRC_FILES_64 := swcnd
LOCAL_MODULE := swcnd
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := swcnd.rc
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(call all-makefiles-under,$(LOCAL_PATH))

