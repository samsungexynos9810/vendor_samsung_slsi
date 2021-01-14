ifeq ($(CONFIG_SAMSUNG_SCSC_WIFIBT),true)

FW_PATH_HOST := vendor/samsung_slsi/mx140/firmware/$(SCSC_WLAN_DEVICE_NAME)
FW_PATH_DEVICE := vendor/etc/wifi

PRODUCT_PACKAGES_ENG += enable_monitor_mode.sh \
			enable_test_mode.sh
PRODUCT_PACKAGES_DEBUG += enable_monitor_mode.sh \
			enable_test_mode.sh

# fw dirs: mx140.bin mx140_t.bin mx140/ mx140_t/
ifneq (,$(filter userdebug eng, $(TARGET_BUILD_VARIANT)))
PRODUCT_COPY_FILES += $(foreach image,\
	$(shell find $(FW_PATH_HOST) -type f),\
	$(image):$(subst $(FW_PATH_HOST),$(FW_PATH_DEVICE),$(image)))
else # don't copy *.sym files in user build
PRODUCT_COPY_FILES += $(foreach image,\
	$(shell find $(FW_PATH_HOST) -type f -not -name '*.sym'),\
	$(image):$(subst $(FW_PATH_HOST),$(FW_PATH_DEVICE),$(image)))
endif

# Moredump binary for this FW
PRODUCT_COPY_FILES += $(FW_PATH_HOST)/mx140/debug/hardware/moredump/moredump.bin:$(TARGET_COPY_OUT_VENDOR)/bin/moredump.bin

# Regulatory database
PRODUCT_COPY_FILES += vendor/samsung_slsi/mx140/firmware/slsi_reg_database.bin:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/slsi_reg_database.bin
 
endif
