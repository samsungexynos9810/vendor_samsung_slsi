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
#$(warning LOCAL_TARGET_DEVICE_DIR =$(LOCAL_TARGET_DEVICE_DIR))
TARGET_COMPANY :=$(strip $(subst /,, $(subst $(TARGET_DEVICE),,$(subst device,,$(LOCAL_TARGET_DEVICE_DIR)))))
#$(warning TARGET_COMPANY =$(TARGET_COMPANY))

# make MODEL_NAME variable
CUSTOM_MODEL_PATH := $(MODEL_LOCAL_PATH)/../../../product/$(TARGET_COMPANY)

# SET Target Model
# 1) first, check carrier for build : e.g. att, tmo
ifneq ($(strip $(TARGET_BUILD_CARRIER)),)
ifneq ($(filter chnopen wwopen, $(TARGET_BUILD_CARRIER)),)
    CARRIER_NAME := -open
else
    CARRIER_NAME := -$(TARGET_BUILD_CARRIER)
endif
endif
TARGET_SUFFIX := $(CARRIER_NAME)

# 2) check, Protocol to use [sipc / sit] : e.g. att-sipc, att(-sit)
ifeq ($(USE_SIPC_PROTOCOL), true)
    ifneq (,$(wildcard $(CUSTOM_MODEL_PATH)/$(TARGET_DEVICE)$(TARGET_SUFFIX)-sipc/model/custom.mk))
        TARGET_SUFFIX := $(TARGET_SUFFIX)-sipc
    endif
endif

ifneq ($(filter m96 m97 m99, $(TARGET_DEVICE)),)
    MODEL_NAME := m9x
else ifneq (,$(wildcard $(CUSTOM_MODEL_PATH)/$(TARGET_DEVICE)$(TARGET_SUFFIX)/model/custom.mk))
    MODEL_NAME := $(TARGET_DEVICE)$(TARGET_SUFFIX)
else ifneq (,$(wildcard $(CUSTOM_MODEL_PATH)/$(TARGET_DEVICE)/model/custom.mk))
    MODEL_NAME := $(TARGET_DEVICE)
else
    MODEL_NAME := default
endif

ifeq ($(MODEL_NAME),default)
    CUSTOM_MODEL_PATH := $(MODEL_LOCAL_PATH)/default
    RELATIVE_CUR_PATH := model/default
else
    CUSTOM_MODEL_PATH := $(CUSTOM_MODEL_PATH)/$(MODEL_NAME)/model
    RELATIVE_CUR_PATH := ../../product/$(TARGET_COMPANY)/$(MODEL_NAME)/model
endif
