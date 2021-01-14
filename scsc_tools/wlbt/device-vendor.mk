ifeq ($(CONFIG_SAMSUNG_SCSC_WIFIBT),true)
PRODUCT_PACKAGES_DEBUG += \
        wland \
        CNNTLogger

DEVICE_PACKAGE_OVERLAYS += \
                vendor/samsung_slsi/scsc_tools/wlbt/device/samsung/overlay
endif
