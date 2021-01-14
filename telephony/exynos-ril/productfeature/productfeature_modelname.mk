#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#

LOCAL_TARGET_DEVICE_DIR :=$(dir $(TARGET_BOARD_INFO_FILE))
#$(warning TARGET_DEVICE_DIR =$(TARGET_DEVICE_DIR))
TARGET_COMPANY :=$(strip $(subst /,, $(subst $(TARGET_DEVICE),,$(subst device,,$(LOCAL_TARGET_DEVICE_DIR)))))
#$(warning TARGET_COMPANY =$(TARGET_COMPANY))

# make MODEL_NAME variable
CUSTOM_PRODUCT_PATH := $(PRODUCTFEATURE_LOCAL_PATH)/../../product/$(TARGET_COMPANY)

ifneq ($(filter m96 m97 m99, $(TARGET_DEVICE)),)
    MODEL_NAME := m9x
else ifneq (,$(wildcard $(CUSTOM_PRODUCT_PATH)/$(TARGET_DEVICE)-$(TARGET_BUILD_CARRIER)/productfeature/customproductfeature.mk))
    MODEL_NAME := $(TARGET_DEVICE)-$(TARGET_BUILD_CARRIER)
else ifneq (,$(wildcard $(CUSTOM_PRODUCT_PATH)/$(TARGET_DEVICE)/productfeature/customproductfeature.mk))
    MODEL_NAME := $(TARGET_DEVICE)
else
    MODEL_NAME := default
endif

ifeq ($(MODEL_NAME),default)
    CUSTOM_PRODUCT_FEATURE_PATH := $(PRODUCTFEATURE_LOCAL_PATH)/default
    RELATIVE_CUR_PATH := $(RELATIVE_CUR_PATH)/default
else
    CUSTOM_PRODUCT_FEATURE_PATH := $(CUSTOM_PRODUCT_PATH)/$(MODEL_NAME)/productfeature
    RELATIVE_CUR_PATH := $(RELATIVE_CUR_PATH)/../../product/$(TARGET_COMPANY)/$(MODEL_NAME)/productfeature
endif
