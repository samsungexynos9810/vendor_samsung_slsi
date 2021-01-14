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
include $(PRODUCTFEATURE_LOCAL_PATH)/productfeature_modelname.mk

ifneq (,$(wildcard $(CUSTOM_PRODUCT_FEATURE_PATH)/build_var.mk))
#$(warning including product build variables for $(MODEL_NAME) in $(LOCAL_MODULE))
include $(CUSTOM_PRODUCT_FEATURE_PATH)/build_var.mk
endif

