#
# Copyright (C) 2012 The Android Open Source Project
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
#

ifeq ($(TARGET_SOC),exynos850)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
ifeq ($(BOARD_USES_EXYNOS_GRALLOC_VERSION), 0)
ERROR! it does not support anymore for gralloc 0
endif
ifneq (,$(filter $(BOARD_USES_EXYNOS_GRALLOC_VERSION),1 3))
LOCAL_SRC_FILES := libGLES_mali.so
endif
LOCAL_MODULE := libGLES_mali
LOCAL_MODULE_OWNER := samsung_arm
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/egl
LOCAL_STRIP_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_POST_INSTALL_CMD := $(hide) cd $(TARGET_OUT_VENDOR_SHARED_LIBRARIES); \
    mkdir -p hw; \
    ln -sf /vendor/lib64/egl/libGLES_mali.so libOpenCL.so.1.1; \
    ln -sf libOpenCL.so.1.1 libOpenCL.so.1; \
    ln -sf libOpenCL.so.1 libOpenCL.so; \
    ln -sf /vendor/lib64/egl/libGLES_mali.so hw/vulkan.$(TARGET_BOARD_PLATFORM).so;

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
ifeq ($(BOARD_USES_EXYNOS_GRALLOC_VERSION), 0)
ERROR! it does not support anymore for gralloc 0
endif
ifneq (,$(filter $(BOARD_USES_EXYNOS_GRALLOC_VERSION),1 3))
LOCAL_SRC_FILES := libGLES_mali32.so
endif
LOCAL_MODULE := libGLES_mali32
LOCAL_MODULE_OWNER := samsung_arm
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib/egl
LOCAL_STRIP_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_POST_INSTALL_CMD := $(hide) cd $(TARGET_OUT_VENDOR)/lib; \
    mv egl/libGLES_mali32.so egl/libGLES_mali.so; \
    mkdir -p hw; \
    ln -sf /vendor/lib/egl/libGLES_mali.so libOpenCL.so.1.1; \
    ln -sf libOpenCL.so.1.1 libOpenCL.so.1; \
    ln -sf libOpenCL.so.1 libOpenCL.so; \
    ln -sf /vendor/lib/egl/libGLES_mali.so hw/vulkan.$(TARGET_BOARD_PLATFORM).so;
include $(BUILD_PREBUILT)
endif
