ifeq ($(TARGET_SOC),exynos850)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#ifeq ($(TARGET_ARCH), arm)
#ifneq (,$(filter exynos5, $(TARGET_DEVICE)))

# if we have the oemcrypto sources mapped in, then use those unless an
# override has been provided. To force using the prebuilt while having
# the source, set
# HAVE_SLSI_EXYNOS5_OEMCRYPTO_SRC=false
#

BOARD_WIDEVINE_OEMCRYPTO_LEVEL := 1

_should_use_oemcrypto_prebuilt := true
ifeq ($(wildcard vendor/widevine),vendor/widevine)
ifeq ($(wildcard vendor/samsung_slsi/proprietary),vendor/samsung_slsi/proprietary)
ifneq ($(HAVE_SLSI_EXYNOS5_OEMCRYPTO_SRC),false)
_should_use_oemcrypto_prebuilt := false
endif
endif

ifeq ($(_should_use_oemcrypto_prebuilt),true)

###############################################################################
# liboemcrypto.so for Modular DRM

include $(CLEAR_VARS)

LOCAL_MODULE := liboemcrypto_modular
LOCAL_MODULE_STEM := liboemcrypto
LOCAL_MODULE_SUFFIX := .so
LOCAL_SRC_FILES := $(LOCAL_MODULE_STEM)$(LOCAL_MODULE_SUFFIX)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib
LOCAL_MODULE_OWNER := samsung
# Proprietary modules are put in vendor/lib instead of /system/lib.
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

endif # _should_use_oemcrypto_prebuilt == true
endif # $(wildcard vendor/widevine) == vendor/widevine
endif

#endif
#endif
