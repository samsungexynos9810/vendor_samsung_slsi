#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#

CUSTOMPRODUCTFEATURE_LOCAL_PATH:= $(call my-dir)

LOCAL_C_INCLUDES += \
    $(CUSTOMPRODUCTFEATURE_LOCAL_PATH)

LOCAL_SRC_FILES += $(RELATIVE_CUR_PATH)/customproductfeature.cpp

LOCAL_CFLAGS += -DPRODUCT_NAME="\"$(MODEL_NAME)\""

#$(warning adding defines for $(MODEL_NAME) in $(LOCAL_MODULE))
include $(CUSTOMPRODUCTFEATURE_LOCAL_PATH)/feature.mk

