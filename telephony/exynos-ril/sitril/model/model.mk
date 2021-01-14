#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#

MODEL_LOCAL_PATH:= $(call my-dir)

# Set $(MODEL_NAME) variable
include $(MODEL_LOCAL_PATH)/model_modelname.mk

# add customized product feature for model
#$(warning including custom model codes for $(MODEL_NAME))
include $(CUSTOM_MODEL_PATH)/custom.mk

