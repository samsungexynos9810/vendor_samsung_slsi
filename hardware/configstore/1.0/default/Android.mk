LOCAL_PATH := $(call my-dir)

################################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.samsung_slsi.hardware.configstore@1.0-service
# seccomp is not required for coverage build.
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_INIT_RC := vendor.samsung_slsi.hardware.configstore@1.0-service.rc
LOCAL_SRC_FILES:= service.cpp

include $(LOCAL_PATH)/exynos_hwc_configs.mk

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    libbase \
    libhwminijail \
    liblog \
    libutils \
    vendor.samsung_slsi.hardware.configstore@1.0

include $(BUILD_EXECUTABLE)

