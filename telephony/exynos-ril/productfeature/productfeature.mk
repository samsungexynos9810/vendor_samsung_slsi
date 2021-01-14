#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#

PRODUCTFEATURE_LOCAL_PATH:= $(call my-dir)

LOCAL_C_INCLUDES += \
    $(PRODUCTFEATURE_LOCAL_PATH)

LOCAL_SRC_FILES += $(RELATIVE_CUR_PATH)/productfeature.cpp

# 1) add common feature
# Multi-SIM define
ifeq ($(SUPPORT_MULTI_SIM),true)
    LOCAL_CFLAGS += -DANDROID_MULTI_SIM
endif

ifeq ($(SIM_COUNT), 2)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_2
endif

ifeq ($(DEVICE_SUPPORT_CDMA), true)
# set CDMA enable feature
LOCAL_CFLAGS += -DSUPPORT_CDMA
endif

# 2) add model specific feature
# Set $(MODEL_NAME) variable
include $(PRODUCTFEATURE_LOCAL_PATH)/productfeature_modelname.mk

# add customized product feature for model
#$(warning including product feature for $(MODEL_NAME) in $(LOCAL_MODULE))
include $(CUSTOM_PRODUCT_FEATURE_PATH)/customproductfeature.mk

