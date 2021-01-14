
#ifneq (,$(filter pcba modem wcn console fm, $(BOARD_USE_SSCR_TOOLS)))
####SELinux policies for mptool
MPTOOL_SEPOLICY := vendor/samsung_slsi/sscr_tools/mptool/sepolicy

ifeq (, $(findstring $(MPTOOL_SEPOLICY), $(BOARD_SEPOLICY_DIRS)))
$(warning  sscr mp tool dirs->$(MPTOOL_SEPOLICY))
BOARD_SEPOLICY_DIRS += $(MPTOOL_SEPOLICY)
endif
#endif
