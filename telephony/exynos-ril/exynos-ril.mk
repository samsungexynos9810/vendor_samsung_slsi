#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#

ifeq (true, $(TARGET_USES_EXYNOS_RIL))
  TARGET_EXYNOS_RIL_SOURCE := true

  LOCAL_LIB_PACKAGES := \
    libril_sitril \
    librilutils_sitril \
    libsitril \
    liboemreqapp_jni \
    libsitril-client \
    libsitril-audio \
    libsitril-ims \
    libsitril-wlan \
    libsitril-gps

  LOCAL_RILBIN_PACKAGES := \
    radiooptions_exynos \
    rild_exynos

  PRODUCT_PACKAGES += $(LOCAL_LIB_PACKAGES) $(LOCAL_RILBIN_PACKAGES)

endif
