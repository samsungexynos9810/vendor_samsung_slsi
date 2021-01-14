ifeq ($(CONFIG_SAMSUNG_SCSC_WIFIBT),true)

PRODUCT_PACKAGES_ENG += \
	wlbtd \
	slsi_lerna \

PRODUCT_PACKAGES_DEBUG += \
	wlbtd \
	slsi_lerna \

# copy all files in dev_tools dir to /vendor/bin/
PRODUCT_COPY_FILES += \
	vendor/samsung_slsi/scsc_tools/dev_tools/btcli:$(TARGET_COPY_OUT_VENDOR)/bin/btcli \
	vendor/samsung_slsi/scsc_tools/dev_tools/cpu_stats.sh:$(TARGET_COPY_OUT_VENDOR)/bin/cpu_stats.sh \
	vendor/samsung_slsi/scsc_tools/dev_tools/devmem2:$(TARGET_COPY_OUT_VENDOR)/bin/devmem2 \
	vendor/samsung_slsi/scsc_tools/dev_tools/disable_auto_coredump:$(TARGET_COPY_OUT_VENDOR)/bin/disable_auto_coredump \
	vendor/samsung_slsi/scsc_tools/dev_tools/disable_logd_kmsg_collection:$(TARGET_COPY_OUT_VENDOR)/bin/disable_logd_kmsg_collection \
	vendor/samsung_slsi/scsc_tools/dev_tools/disable_wlbt_log:$(TARGET_COPY_OUT_VENDOR)/bin/disable_wlbt_log \
	vendor/samsung_slsi/scsc_tools/dev_tools/enable_wlbt_log:$(TARGET_COPY_OUT_VENDOR)/bin/enable_wlbt_log \
	vendor/samsung_slsi/scsc_tools/dev_tools/if_unifi.so:$(TARGET_COPY_OUT_VENDOR)/bin/if_unifi.so \
	vendor/samsung_slsi/scsc_tools/dev_tools/moredump:$(TARGET_COPY_OUT_VENDOR)/bin/moredump \
	vendor/samsung_slsi/scsc_tools/dev_tools/mxdecoder:$(TARGET_COPY_OUT_VENDOR)/bin/mxdecoder \
	vendor/samsung_slsi/scsc_tools/dev_tools/omnicli:$(TARGET_COPY_OUT_VENDOR)/bin/omnicli \
	vendor/samsung_slsi/scsc_tools/dev_tools/scsc_enable_flight_mode.sh:$(TARGET_COPY_OUT_VENDOR)/bin/scsc_enable_flight_mode.sh \
	vendor/samsung_slsi/scsc_tools/dev_tools/scsc_get_platform_info.sh:$(TARGET_COPY_OUT_VENDOR)/bin/scsc_get_platform_info.sh \
	vendor/samsung_slsi/scsc_tools/dev_tools/slsi_wlan_loopback_config:$(TARGET_COPY_OUT_VENDOR)/bin/slsi_wlan_loopback_config \
	vendor/samsung_slsi/scsc_tools/dev_tools/slsi_wlan_mib:$(TARGET_COPY_OUT_VENDOR)/bin/slsi_wlan_mib \
	vendor/samsung_slsi/scsc_tools/dev_tools/slsi_wlan_src_sink:$(TARGET_COPY_OUT_VENDOR)/bin/slsi_wlan_src_sink \
	vendor/samsung_slsi/scsc_tools/dev_tools/slsi_wlan_udi_log:$(TARGET_COPY_OUT_VENDOR)/bin/slsi_wlan_udi_log \
	vendor/samsung_slsi/scsc_tools/dev_tools/slsi_wlan_udi_log_decode:$(TARGET_COPY_OUT_VENDOR)/bin/slsi_wlan_udi_log_decode \
	vendor/samsung_slsi/scsc_tools/dev_tools/tgen:$(TARGET_COPY_OUT_VENDOR)/bin/tgen \
	vendor/samsung_slsi/scsc_tools/dev_tools/trigger_moredump:$(TARGET_COPY_OUT_VENDOR)/bin/trigger_moredump \
	vendor/samsung_slsi/scsc_tools/dev_tools/vectordriverbroker:$(TARGET_COPY_OUT_VENDOR)/bin/vectordriverbroker \
	vendor/samsung_slsi/scsc_tools/dev_tools/vectordriverbroker.bin:$(TARGET_COPY_OUT_VENDOR)/bin/vectordriverbroker.bin \
	vendor/samsung_slsi/scsc_tools/dev_tools/wlan_debug_level.sh:$(TARGET_COPY_OUT_VENDOR)/bin/wlan_debug_level.sh \
	vendor/samsung_slsi/scsc_tools/dev_tools/wlbt_onoff.sh:$(TARGET_COPY_OUT_VENDOR)/bin/wlbt_onoff.sh \
	vendor/samsung_slsi/scsc_tools/dev_tools/wlbtd/mx_logger.sh:$(TARGET_COPY_OUT_VENDOR)/bin/mx_logger.sh \
	vendor/samsung_slsi/scsc_tools/dev_tools/wlbtd/mx_logger_dump.sh:$(TARGET_COPY_OUT_VENDOR)/bin/mx_logger_dump.sh \
	vendor/samsung_slsi/scsc_tools/dev_tools/wlbtd/mx_log_collection.sh:$(TARGET_COPY_OUT_VENDOR)/bin/mx_log_collection.sh \

endif
