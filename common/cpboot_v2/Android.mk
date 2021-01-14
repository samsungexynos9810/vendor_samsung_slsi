# Copyright 2005 The Android Open Source Project

ifneq ($(MODEM_USE_EXYNOS), true)
LOCAL_PATH:= $(call my-dir)
# build for power consumption test
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := bionic/libc/bionic
LOCAL_CFLAGS += -Wall -Wno-unused-parameter \
                -D_GNU_SOURCE

# Modem type definitions

LOCAL_SRC_FILES := main.c util.c std_boot.c
LOCAL_SRC_FILES += util_srinfo.c
LOCAL_SRC_FILES += boot_shannon310.c boot_shannon5100.c

ifeq ($(SEC_CP_SECURE_BOOT),true)
LOCAL_CFLAGS += -DCONFIG_SEC_CP_SECURE_BOOT
LOCAL_C_INCLUDES += $(TOP)/hardware/samsung_slsi/exynos3/include
endif

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= cbd_v2
LOCAL_POST_INSTALL_CMD := $(hide) mv $(TARGET_OUT_VENDOR)/bin/cbd_v2 $(TARGET_OUT_VENDOR)/bin/cbd;

ifeq ($(SEC_CP_SECURE_BOOT),true)
LOCAL_FORCE_STATIC_EXECUTABLE := false
else
#LOCAL_FORCE_STATIC_EXECUTABLE := true
endif

LOCAL_PROPRIETARY_MODULE := true

LOCAL_SHARED_LIBRARIES := liblog libcutils

ifeq ($(SEC_CP_SECURE_BOOT),true)
LOCAL_SHARED_LIBRARIES += libsecurepath
LOCAL_SHARED_LIBRARIES += libMcClient liblog libc
else
LOCAL_SHARED_LIBRARIES += libc
endif

ifneq ($(filter full_universal9820 full_universal9820_s5100, $(TARGET_PRODUCT)), )
LOCAL_CFLAGS += -DCONFIG_SEC_CP_VERIFYING_ALL -DCONFIG_DUMP_DATABUF
endif

ifeq ($(CBD_PROTOCOL_SIT), true)
LOCAL_CFLAGS += -DCONFIG_PROTOCOL_SIT
endif

ifeq ($(CBD_DUMP_LIMIT), true)
LOCAL_CFLAGS += -DCONFIG_DUMP_LIMIT
endif

include $(BUILD_EXECUTABLE)

# CP LOOPBACK DAEMON
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := bionic/libc/bionic

LOCAL_SRC_FILES:= lb_main.c

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= cld_v2
LOCAL_POST_INSTALL_CMD := $(hide) mv $(TARGET_OUT_VENDOR)/bin/cld_v2 $(TARGET_OUT_VENDOR)/bin/cld;

#LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_PROPRIETARY_MODULE := true

LOCAL_SHARED_LIBRARIES := liblog libcutils libc

include $(BUILD_EXECUTABLE)
endif
