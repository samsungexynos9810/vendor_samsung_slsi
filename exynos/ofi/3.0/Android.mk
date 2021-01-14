LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libofi_kernels_cpu
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
ifeq ($(TARGET_ARCH),arm64)
LOCAL_SRC_FILES_64 := lib64/libofi_kernels_cpu.so
LOCAL_SRC_FILES_32 := lib/libofi_kernels_cpu.so
LOCAL_MULTILIB := both
else
LOCAL_SRC_FILES := lib/libofi_kernels_cpu.so
LOCAL_MULTILIB := 32
endif
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include/
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libofi_rt_framework
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
ifeq ($(TARGET_ARCH),arm64)
LOCAL_SRC_FILES_64 := lib64/libofi_rt_framework.so
LOCAL_SRC_FILES_32 := lib/libofi_rt_framework.so
LOCAL_MULTILIB := both
else
LOCAL_SRC_FILES := lib/libofi_rt_framework.so
LOCAL_MULTILIB := 32
endif
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include/
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libofi_rt_framework_user_vendor
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
ifeq ($(TARGET_ARCH),arm64)
LOCAL_SRC_FILES_64 := lib64/libofi_rt_framework_user_vendor.so
LOCAL_SRC_FILES_32 := lib/libofi_rt_framework_user_vendor.so
LOCAL_MULTILIB := both
else
LOCAL_SRC_FILES := lib/libofi_rt_framework_user_vendor.so
LOCAL_MULTILIB := 32
endif
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include/
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libofi_service_interface_vendor
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES := vendor.samsung_slsi.hardware.ofi@3.0
ifeq ($(TARGET_ARCH),arm64)
LOCAL_SRC_FILES_64 := lib64/libofi_service_interface_vendor.so
LOCAL_SRC_FILES_32 := lib/libofi_service_interface_vendor.so
LOCAL_MULTILIB := both
else
LOCAL_SRC_FILES := lib/libofi_service_interface_vendor.so
LOCAL_MULTILIB := 32
endif
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include/
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libofi_seva_vendor
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
ifeq ($(TARGET_ARCH),arm64)
LOCAL_SRC_FILES_64 := lib64/libofi_seva_vendor.so
LOCAL_SRC_FILES_32 := lib/libofi_seva_vendor.so
LOCAL_MULTILIB := both
else
LOCAL_SRC_FILES := lib/libofi_seva_vendor.so
LOCAL_MULTILIB := 32
endif
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include/
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libofi_dal
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
ifeq ($(TARGET_ARCH),arm64)
LOCAL_SRC_FILES_64 := lib64/libofi_dal.so
LOCAL_SRC_FILES_32 := lib/libofi_dal.so
LOCAL_MULTILIB := both
else
LOCAL_SRC_FILES := lib/libofi_dal.so
LOCAL_MULTILIB := 32
endif
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include/
include $(BUILD_PREBUILT)

include $(call all-makefiles-under, $(LOCAL_PATH))
