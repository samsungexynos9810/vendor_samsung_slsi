# Copyright (C) 2014 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Including order
# full_[product].mk -> device.mk (include BoardConfig.mk at the first line) -> telephony/device_vendor.mk

# Origianlly placed at last lines in device.mk
# Some device specific line should be reviewed
# It's better if we can use some variable holding device path or name

# Informative code For Debugging
# we can use these variables that's tested

$(call inherit-product-if-exists, vendor/samsung_slsi/telephony/common/device/samsung/conf_copy.mk)

######
# Expand AOSP definition from
#$(call inherit-product, $(SRC_TARGET_DIR)/product/aosp_base_telephony.mk)
##$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)
# aosp_base.mk live in device.mk
#$(call inherit-product, $(SRC_TARGET_DIR)/product/aosp_base.mk)
### Excluding AOSP apns copy

PRODUCT_PROPERTY_OVERRIDES += \
							  keyguard.no_require_sim=true

$(call inherit-product, $(SRC_TARGET_DIR)/product/telephony.mk)

PRODUCT_PACKAGES += \
	messaging
#
######

# Modify RIL daemon (rild-exynos)
# We no more need to copy and override init.rc
# rild.rc will takes this role

# Need to move common area, then split common/variable parts
# For SPN display
PRODUCT_COPY_FILES += \
    vendor/samsung_slsi/telephony/common/device/samsung/spn-conf.xml:system/etc/spn-conf.xml

# default network
ifeq ($(SUPPORT_NR), true)
PRODUCT_PROPERTY_OVERRIDES += \
        ro.telephony.default_network=26
else
PRODUCT_PROPERTY_OVERRIDES += \
        ro.telephony.default_network=10
endif

# multiple vendor
ifeq ($(TARGET_BUILD_CARRIER),)
$(warning No definition of TARGET_BUILD_CARRIER, set europen at default)
CARRIER := europen
else
$(warning Found TARGET_BUILD_CARRIER=$(TARGET_BUILD_CARRIER))
CARRIER := $(TARGET_BUILD_CARRIER)
endif

PRODUCT_PROPERTY_OVERRIDES += \
    ro.carrier=$(CARRIER) \
    ro.vendor.config.build_carrier=$(CARRIER)

ifeq ($(MODEM_NOT_USE_CP_PARTION), true)
PRODUCT_COPY_FILES += \
	vendor/samsung_slsi/telephony/common/radio/$(DEVICE_PRODUCT)/modem.bin:$(TARGET_COPY_OUT_VENDOR)/firmware/modem.bin
endif

DEVICE_PACKAGE_OVERLAYS += \
    vendor/samsung_slsi/telephony/common/device/samsung/carriers/overlay

# Enable 4GDS at default from Q
PRODUCT_PROPERTY_OVERRIDES += \
        persist.vendor.radio.dual.volte=1

PRODUCT_PROPERTY_OVERRIDES += \
        ro.ril.ecclist=911,112

PRODUCT_PROPERTY_OVERRIDES += \
	    ro.telephony.iwlan_operation_mode=legacy

#SMS Domain: CS pref(0), PS pref(1), CS only(2), PS only(3)
ifeq ($(TARGET_BUILD_CARRIER), $(filter $(TARGET_BUILD_CARRIER), cmcc ctc chnopen))
PRODUCT_PROPERTY_OVERRIDES += \
        vendor.radio.smsdomain=2
endif

PRODUCT_PROPERTY_OVERRIDES += \
        vendor.rild.libpath=libsitril.so

# Originally placed at last lines in full_[product].mk
# Telephony feature

# eng only
PRODUCT_PACKAGES_ENG += \
        KeyString

# debug only
PRODUCT_PACKAGES_DEBUG += \
        EngineerMode \
        NetworkTestMode \
        UARTSwitch \
        SysDebugMode \
        USBModeSwitch \
        DataTestMode \
        TestMode \
        AutoAnswer \
        SilentLogging

# optional
PRODUCT_PACKAGES += \
        dmd \
        vcd \
        sced \
        rfsd

ifneq ($(CBD_USE_V2), true)
PRODUCT_PACKAGES += \
        cbd
endif

PRODUCT_PACKAGES += \
        OemRilService

PRODUCT_PACKAGES += \
        rild_exynos \
        libsitril \
        libril_sitril \
        librilutils_sitril \
        libsitril-client \
        libsitril-audio \
        libsitril-ims \
        libsitril-gps \
        libsitril-wlan \
        libsitril-se \
        libsitril-psensor \
        libsitril-sar \
        libsitril-nr \
        libsmiril-ims

PRODUCT_PACKAGES += \
        vendor.samsung_slsi.telephony.hardware.radio@1.0 \
        vendor.samsung_slsi.telephony.hardware.radioExternal@1.0

PRODUCT_PACKAGES += \
        vendor.samsung_slsi.telephony.hardware.radio@1.1

PRODUCT_PACKAGES += \
        liboemservice \
        vendor.samsung_slsi.telephony.hardware.oemservice@1.0

PRODUCT_PACKAGES += \
        android.hardware.secure_element@1.1-service-uicc

PRODUCT_PACKAGES += \
        android.hardware.radio.config@1.2-service

# Vendor ext (custom platform dependency)
ifneq ($(PRODUCT_VENDOR_TELEPHONY_EXT), )
#PRODUCT_PACKAGES += \
        UplmnSetting

# For slsi systemUI.
# System_slsi is implemented based on frameworks/base/packages/SystemUI
PRODUCT_PACKAGES += SystemUI_slsi
endif

# STK
PRODUCT_PACKAGES += \
        Stk

ifeq ($(TARGET_BUILD_CARRIER), chnopen)
PRODUCT_PACKAGES += \
    CtcSelfRegister
endif

BOARD_SEPOLICY_DIRS += \
	vendor/samsung_slsi/telephony/common/device/samsung/sepolicy

# For security reason, only applying to eng build
ifneq (, $(filter $(TARGET_BUILD_VARIANT), eng userdebug))
ifeq ($(BOARD_USES_NSIOT_TESTSCRIPT), true)
# to support TestApp triggers init services in init.testext.rc
# which uses ndc sends commands to Netd
# This will provide interface between TestMode app
# When rc script is used in Userdebug mode, SELINUX PERMISSIVE MODE IS "REQUIRED".
# ex) adb_root_shell # setenforce 0
PRODUCT_COPY_FILES += \
	vendor/samsung_slsi/telephony/common/device/samsung/conf/init.testext.rc:vendor/etc/init/init.testext.rc

# Now Netd code supports Port forwarding
# no need to run script, and set addtional sepolicy
#PRODUCT_COPY_FILES += \
	vendor/samsung_slsi/telephony/common/device/samsung/portfwd.sh:vendor/bin/portfwd.sh
endif
endif
#ifeq ($(TARGET_BUILD_VARIANT), eng)
# For Testing purpose as example
# If configuration is set and app is not installed
# TetherProvisioing will fail.
# build/envsetup.sh
#  -CARRIER_CHOICES=(att vzw tmo cmcc chnopen)
#  +CARRIER_CHOICES=(att vzw tmo cmcc chnopen spr)
#
# device/samsung/*/vendorsetup.sh  (for espresso8890 example)
#  +add_lunch_combo full_espresso8890-eng-spr
#
# packages/apps/IMSClient
#  adding spr  symbolic link to others(ex cmcc)
#  lrwxrwxrwx  1 * *    4  Apr 10 12:35 spr -> cmcc

#ifneq (, $(filter $(TARGET_BUILD_CARRIER), spr tmo))
#PRODUCT_PACKAGES += \
#        TetherProvisioning
#endif
#endif

# Device Manifest, Device Compatibility Matrix for Treble
DEVICE_MANIFEST_FILE += \
        vendor/samsung_slsi/telephony/common/device/samsung/manifest.xml


# telephony features
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.telephony.cdma.xml

PRODUCT_COPY_FILES += $(call find-copy-subdir-files,*,vendor/samsung_slsi/telephony/exynos-ril/sitril/database,$(TARGET_COPY_OUT_VENDOR)/etc/database)
