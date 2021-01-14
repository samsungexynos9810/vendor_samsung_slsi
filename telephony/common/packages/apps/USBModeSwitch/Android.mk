#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PACKAGE_NAME := USBModeSwitch
LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_SRC_FILES := $(call all-java-files-under, src)
#LOCAL_SRC_FILES += usb/default/UsbManagerInterfaceImpl.java

#For Android6(M/N version)
#LOCAL_SRC_FILES += usb/v6/UsbManagerInterfaceImpl.java

#For Android8(O version)
#LOCAL_SRC_FILES += usb/v8/UsbManagerInterfaceImpl.java

#For Android8(P version or later)
LOCAL_SRC_FILES += usb/v9/UsbManagerInterfaceImpl.java

LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
