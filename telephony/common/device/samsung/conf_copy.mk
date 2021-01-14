APN_ROOT := vendor/samsung_slsi/telephony/common/device/samsung
OPEN_CARRIER_ROOT := vendor/samsung_slsi/telephony/common/device/samsung

PRODUCT_COPY_FILES += \
    $(APN_ROOT)/apns-full-conf.xml:system/etc/apns-conf.xml

ifneq (,$(wildcard $(APN_ROOT)/apns-$(CARRIER)-conf.xml))
#    $(warning include apns-$(CARRIER)-conf.xml)
PRODUCT_COPY_FILES += \
    $(APN_ROOT)/apns-$(CARRIER)-conf.xml:system/etc/apns-carrier-conf.xml
else
#    $(warning Do not include the carrier_conf)
PRODUCT_COPY_FILES += \
    $(APN_ROOT)/apns-nocarrier-conf.xml:system/etc/apns-carrier-conf.xml
endif

#copy open carrier info file to system folder
PRODUCT_COPY_FILES += \
    $(OPEN_CARRIER_ROOT)/open_carrier_info.dat:system/etc/open_carrier_info.dat
