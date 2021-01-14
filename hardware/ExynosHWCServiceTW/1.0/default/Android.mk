#
# Copyright (C) 2016 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


ifneq ($(filter exynos,$(TARGET_SOC_NAME)),)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifndef TARGET_SOC_BASE
	TARGET_SOC_BASE := $(TARGET_SOC)
endif

LOCAL_MODULE := vendor.samsung_slsi.hardware.ExynosHWCServiceTW@1.0-service
LOCAL_INIT_RC := vendor.samsung_slsi.hardware.ExynosHWCServiceTW@1.0-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := \
  ExynosHWCServiceTW.cpp \
  service.cpp

LOCAL_CFLAGS := -fno-exceptions

ifdef BOARD_HWC_VERSION
LOCAL_CFLAGS += -DUSE_LIBHWC21
LOCAL_CFLAGS += -DENABLE_DDI_SCALER
LOCAL_CFLAGS += -DSUPPORT_WFD_COMMAND
ifeq ($(BOARD_USES_HWC_CPU_PERF_MODE),true)
LOCAL_CFLAGS += -DUSE_CPU_PERF_MODE
endif
LOCAL_C_INCLUDES += \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libdevice \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libdisplay\
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libmaindisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libexternaldisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libhwchelper \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION) \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION)/libmaindisplay \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION)/libexternaldisplay \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION)/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION)/libdevice \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION)/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libhwcService \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)
else
ifeq ($(BOARD_USES_HWC2), true)
LOCAL_C_INCLUDES += \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libdevice \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libdisplay\
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libmaindisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libexternaldisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libhwchelper \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2 \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2/libmaindisplay \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2/libexternaldisplay \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2/libdevice \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libhwcService \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2
else
LOCAL_C_INCLUDES += \
	$(TOP)/hardware/samsung_slsi/exynos/include \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1 \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1/libvppdisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1/libvpphdmi \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1/libvppvirtualdisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1/libhwcutils \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwcmodule \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwcutilsmodule \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libdisplaymodule \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhdmimodule \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libvirtualdisplaymodule \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1/libhwcService

LOCAL_CFLAGS += -DUSE_LIBHWC
endif
endif

LOCAL_SHARED_LIBRARIES := \
  libutils \
  libcutils \
  libhidlbase \
  libhidltransport \
  liblog \
  libbinder \
  libsync \
  libhardware \
  android.hardware.graphics.composer@2.1 \
  android.hardware.graphics.allocator@2.0 \
  vendor.samsung_slsi.hardware.ExynosHWCServiceTW@1.0 \
  libExynosHWCService \
  libion \
  libmpp

LOCAL_HEADER_LIBRARIES := libhardware_legacy_headers libbinder_headers libutils_headers libexynos_headers
include $(TOP)/hardware/samsung_slsi/graphics/base/BoardConfigCFlags.mk
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.samsung_slsi.hardware.ExynosHWCServiceTW@1.0-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SRC_FILES := \
  ExynosHWCServiceTW.cpp

LOCAL_CFLAGS := -fno-exceptions

ifdef BOARD_HWC_VERSION
LOCAL_CFLAGS += -DENABLE_DDI_SCALER
LOCAL_CFLAGS += -DSUPPORT_WFD_COMMAND
LOCAL_C_INCLUDES += \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libdevice \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libdisplay\
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libmaindisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libexternaldisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libhwchelper \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION) \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION)/libmaindisplay \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION)/libexternaldisplay \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION)/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION)/libdevice \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/$(BOARD_HWC_VERSION)/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)/libhwcService \
	$(TOP)/hardware/samsung_slsi/graphics/base/$(BOARD_HWC_VERSION)
else
ifeq ($(BOARD_USES_HWC2), true)
LOCAL_C_INCLUDES += \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libdevice \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libdisplay\
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libmaindisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libexternaldisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libhwchelper \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2 \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2/libmaindisplay \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2/libexternaldisplay \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2/libdevice \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwc2/libresource \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2/libhwcService \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc2
else
LOCAL_C_INCLUDES += \
	$(TOP)/hardware/samsung_slsi/exynos/include \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1 \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1/libvppdisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1/libvpphdmi \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1/libvppvirtualdisplay \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1/libhwcutils \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwcmodule \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhwcutilsmodule \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libdisplaymodule \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libhdmimodule \
	$(TOP)/hardware/samsung_slsi/graphics/$(TARGET_SOC_BASE)/libvirtualdisplaymodule \
	$(TOP)/hardware/samsung_slsi/graphics/base/libhwc1/libhwcService

LOCAL_CFLAGS += -DUSE_LIBHWC
endif
endif

LOCAL_SHARED_LIBRARIES := \
  libutils \
  libcutils \
  libhidlbase \
  libhidltransport \
  liblog \
  libbinder \
  libsync \
  libhardware \
  android.hardware.graphics.composer@2.1 \
  android.hardware.graphics.allocator@2.0 \
  vendor.samsung_slsi.hardware.ExynosHWCServiceTW@1.0 \
  libion \
  libExynosHWCService

LOCAL_HEADER_LIBRARIES := libhardware_legacy_headers libbinder_headers libutils_headers libexynos_headers
include $(TOP)/hardware/samsung_slsi/graphics/base/BoardConfigCFlags.mk
include $(BUILD_SHARED_LIBRARY)
endif
