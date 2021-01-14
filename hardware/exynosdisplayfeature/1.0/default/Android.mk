LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.samsung_slsi.hardware.exynosdisplayfeature@1.0-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := \
    ExynosDisplayFeature.cpp \
    ExynosDisplayUtils.cpp \
    ExynosDisplayColor.cpp \
    ExynosDisplayHDR.cpp \
    ExynosDisplayPanel.cpp \

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    libutils \
    liblog \
    libcutils \
    libhardware \
    libbase \
    libbinder \

LOCAL_SHARED_LIBRARIES += vendor.samsung_slsi.hardware.exynosdisplayfeature@1.0

LOCAL_SHARED_LIBRARIES += libxml2

LOCAL_C_INCLUDES := \
    external/libxml2/include \

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.samsung_slsi.hardware.exynosdisplayfeature@1.0-service
LOCAL_INIT_RC := vendor.samsung_slsi.hardware.exynosdisplayfeature@1.0-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := \
    service.cpp \

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libbinder \
    libhardware \
    libhidlbase \
    libhidltransport \

LOCAL_SHARED_LIBRARIES += vendor.samsung_slsi.hardware.exynosdisplayfeature@1.0

include $(BUILD_EXECUTABLE)
