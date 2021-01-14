build_dirs :=

ifneq (,$(filter exynos9630, $(TARGET_SOC_BASE)))
build_dirs := $(BOARD_OFI_HAL_VERSION)
endif

ifneq ($(build_dirs)), )
include $(call all-named-subdir-makefiles,$(build_dirs))
endif
