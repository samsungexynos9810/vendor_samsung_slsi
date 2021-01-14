# Copyright (C) 2014 Samsung S.LSI

ifeq ($(MODEM_USE_EXYNOS), true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#modem type define (ONECHIP/TWOCHIP) in BoardConfig.mk

LOCAL_CFLAGS += -Wall -Wno-unused-parameter

ifeq ($(MODEM_USE_ONECHIP), true)
LOCAL_CFLAGS += -DSS310 -DLINK_BOOT_SHMEM -DLINK_MAIN_SHMEM
else

ifeq ($(MODEM_LINK_PCIE), true)
LOCAL_CFLAGS += -DSS300 -DLINK_BOOT_SPI -DLINK_MAIN_PCIE -DHAS_VSS
else
LOCAL_CFLAGS += -DSS300 -DLINK_BOOT_SPI -DLINK_MAIN_LLI
endif

endif

ifeq ($(MODEM_USE_GPT), true)
LOCAL_CFLAGS += -DUSE_GPT
endif

ifeq ($(MODEM_USE_UFS), true)
LOCAL_CFLAGS += -DUSE_UFS
endif

ifeq ($(MODEM_NOT_USE_CP_PARTITION), true)
LOCAL_CFLAGS += -DNOT_USE_CP_PARTITION
endif

ifeq ($(MODEM_USE_SPI_BOOT_LINK), true)
LOCAL_CFLAGS += -DUSE_SPI_BOOT_LINK
endif

ifeq ($(MODEM_NOT_USE_FINAL_CMD), true)
LOCAL_CFLAGS += -DDISABLE_FINAL_CMD
endif

LOCAL_SRC_FILES := main.c modem_common.c modem_ioctl.c

ifeq ($(MODEM_USE_ONECHIP), true)
LOCAL_SRC_FILES += modem_ss310.c
else
LOCAL_SRC_FILES += modem_ss300.c
endif
LOCAL_SRC_FILES += crash_dump.c

LOCAL_SRC_FILES += util_srinfo.c

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= cbd

LOCAL_PROPRIETARY_MODULE := true

LOCAL_SHARED_LIBRARIES := liblog libcutils

LOCAL_C_INCLUDES += system/core/include

include $(BUILD_EXECUTABLE)
endif
