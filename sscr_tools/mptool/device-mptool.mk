#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#
ifneq (,$(filter wcn modem console pcba fm,$(BOARD_USE_SSCR_TOOLS)))
PRODUCT_PACKAGES += \
	libmptool_utils \
	libmptool_json \
	libmptool_log
endif

ifneq (,$(filter wcn,$(BOARD_USE_SSCR_TOOLS)))
PRODUCT_PACKAGES += \
	swcnd \
	atsmdk \
	libwlbt
endif

ifneq (,$(filter modem,$(BOARD_USE_SSCR_TOOLS)))
PRODUCT_PACKAGES += \
	smcd
endif

ifneq (,$(filter pcba,$(BOARD_USE_SSCR_TOOLS)))
PRODUCT_PACKAGES += \
	libspad_core \
	spad
endif

ifneq (,$(filter fm,$(BOARD_USE_SSCR_TOOLS)))
PRODUCT_PACKAGES += \
	sfmd
endif

ifneq (,$(filter console,$(BOARD_USE_SSCR_TOOLS)))
PRODUCT_PACKAGES += \
	sctd
endif

PRODUCT_COPY_FILES += vendor/samsung_slsi/sscr_tools/mptool/conf/mplog.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/mplog.rc
