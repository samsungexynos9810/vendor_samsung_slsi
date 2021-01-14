#
# Copyright Samsung Electronics Co., LTD.
#
# This software is proprietary of Samsung Electronics.
# No part of this software, either material or conceptual may be copied or distributed, transmitted,
# transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
# electronic, mechanical, manual or otherwise, or disclosed
# to third parties without the express written permission of Samsung Electronics.
#
LOCAL_PATH := $(call my-dir)
tools_dir := external

ifneq (,$(filter wcn,$(BOARD_USE_SSCR_TOOLS)))
tools_dir += swcnd
endif

ifneq (,$(filter modem,$(BOARD_USE_SSCR_TOOLS)))
tools_dir += smcd
endif

ifneq (,$(filter pcba,$(BOARD_USE_SSCR_TOOLS)))
tools_dir += spad
endif

ifneq (,$(filter fm,$(BOARD_USE_SSCR_TOOLS)))
tools_dir += sfmd
endif

ifneq (,$(filter console,$(BOARD_USE_SSCR_TOOLS)))
tools_dir += sctd
endif

-include $(call all-named-subdir-makefiles,$(tools_dir))

