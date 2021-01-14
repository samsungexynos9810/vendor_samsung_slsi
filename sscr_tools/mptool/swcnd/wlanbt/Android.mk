#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#
#SSCR Factory Momdem Daemon/SSCR Factory Modem library

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libwlbt
LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES_32 := 32bit/libwlbt.so
LOCAL_SRC_FILES_64 := 64bit/libwlbt.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX = .so
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := atsmdk
LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES_32 := 32bit/atsmdk
LOCAL_SRC_FILES_64 := 64bit/atsmdk
LOCAL_MODULE_CLASS := EXECUTABLES
include $(BUILD_PREBUILT)

