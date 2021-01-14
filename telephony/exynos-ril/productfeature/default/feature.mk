#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#

#-------------------------------------------------------------------
# add product feature define
#-------------------------------------------------------------------

# description
#LOCAL_CFLAGS += -DRIL_XXX

# when manual plmn search, rat information will be included
LOCAL_CFLAGS += -DRIL_FEATURE_EXTENTION_BPLMN

ifeq ($(TARGET_BUILD_CARRIER),cmcc)
LOCAL_CFLAGS += -DRIL_IPV4_DNS_QUERY_FIRST
else
ifeq ($(TARGET_BUILD_CARRIER),chnopen)
LOCAL_CFLAGS += -DRIL_IPV4_DNS_QUERY_FIRST
else
endif
endif
