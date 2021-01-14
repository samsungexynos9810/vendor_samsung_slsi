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
include $(CLEAR_VARS)

LOCAL_VENDOR_MODULE := true

LOCAL_MODULE:= libsitril
LOCAL_CLANG := true

# four options: user eng tests optional
LOCAL_MODULE_TAGS := optional

# add product build variables for makefile
PRODUCT_FEATURE_PATH := ../productfeature
include $(LOCAL_PATH)/$(PRODUCT_FEATURE_PATH)/productfeature_build_var.mk

LOCAL_C_INCLUDES := external/sqlite/dist \
            bionic \
            external/openssl/include \
            external/stlport/stlport \
            external/libpcap \
            system/core/libutils/include \
    $(LOCAL_PATH)/../include \
    $(LOCAL_PATH)/../external/libsitril-client \
    $(LOCAL_PATH)/../external/libsitril-if

LOCAL_C_INCLUDES += $(LOCAL_PATH)/ \
                    $(LOCAL_PATH)/base/ \
                    $(LOCAL_PATH)/core/ \
                    $(LOCAL_PATH)/core/service/ \
                    $(LOCAL_PATH)/fw/ \
                    $(LOCAL_PATH)/protocol/io \
                    $(LOCAL_PATH)/protocol/test/ \
                    $(LOCAL_PATH)/oem/ \
                    $(LOCAL_PATH)/simRecord/ \
                    $(LOCAL_PATH)/stk/ \
                    $(LOCAL_PATH)/util/

ifeq ($(USE_SIPC_PROTOCOL), true)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/protocol/sipc/
else
LOCAL_C_INCLUDES += $(LOCAL_PATH)/protocol/sit/
endif

LOCAL_SHARED_LIBRARIES := \
            liblog \
            libcutils \
            libsqlite \
            libcrypto \
            libhardware_legacy \
            libnetutils \
            librilutils \
            libxml2

LOCAL_CPPFLAGS += -DRIL_SHLIB

ifeq ($(SIM_COUNT), 2)
    LOCAL_CPPFLAGS += -DANDROID_MULTI_SIM
    LOCAL_CPPFLAGS += -DANDROID_SIM_COUNT_2
endif

ifeq ($(USE_SIPC_PROTOCOL), true)
	LOCAL_CPPFLAGS += -DSIPC_PROTOCOL
endif

ifeq ($(SUPPORT_NR), true)
    LOCAL_CPPFLAGS += -DSUPPORT_NR
endif

LOCAL_CPPFLAGS += -DRIL_FEATURE_FUNCTION_CHECK_CURRENT_STACK_BUSY
LOCAL_CPPFLAGS += -DRIL_FEATURE_NO_EXTENTION_USSD_DCS

ifeq ($(SUPPORT_ONLY_BASIC_PREFERRED_NETWORK_TYPE), true)
    LOCAL_CPPFLAGS += -DSUPPORT_ONLY_BASIC_PREFERRED_NETWORK_TYPE
endif

################################################
#    WORKAROUND LIST
################################################
#LOCAL_CPPFLAGS += -D_WR_DELAY_5_SEC_AFTER_BOOTING_
LOCAL_CPPFLAGS += -Wno-unused-parameter -Wunused-variable -Werror
LOCAL_CPPFLAGS += -Wno-date-time
LOCAL_CPPFLAGS += -Wno-tautological-pointer-compare
LOCAL_CPPFLAGS += -Wno-shift-count-overflow
ifeq ($(USE_SIPC_PROTOCOL), true)
LOCAL_CPPFLAGS += -DCONF_IPC4VOLTE_INTERIM_UPDATE
endif

LOCAL_SRC_FILES := base/thread.cpp

LOCAL_SRC_FILES += core/message.cpp \
        core/modemdata.cpp \
        core/productfactory.cpp \
        core/rildata.cpp \
        core/rildatabuilder.cpp \
        core/rilparser.cpp \
        core/servicefactory.cpp \
        core/service.cpp \
        core/servicestate.cpp \
        core/servicemgr.cpp \
        core/servicemonitorrunnable.cpp \
        core/tokengen.cpp \
        core/waitlist.cpp \
        core/timerlist.cpp \
        core/build.cpp

LOCAL_SRC_FILES += core/service/apnsetting.cpp \
        core/service/mcctable.cpp \
        core/service/netifcontroller.cpp \
        core/service/netlink.cpp \
        core/service/operatortable.cpp \
        core/service/ts25table.cpp \
        core/service/pdpcontext.cpp \
        core/service/telephonyprovider.cpp \
        core/service/timezoneid.cpp \
        core/service/cscservice.cpp \
        core/service/psservice.cpp \
        core/service/simservice.cpp \
        core/service/stkservice.cpp \
        core/service/miscservice.cpp \
        core/service/networkservice.cpp \
        core/service/smsservice.cpp \
        core/service/audioservice.cpp \
        core/service/imsservice.cpp \
        core/service/gpsservice.cpp \
        core/service/wlanservice.cpp \
        core/service/vsimservice.cpp \
        core/service/testservice.cpp \
        core/service/embmsservice.cpp \
        core/service/uplmnselector.cpp \
        core/service/supplementaryservice.cpp \
        core/service/calldata.cpp        

LOCAL_SRC_FILES += fw/requestdata.cpp \
        fw/callreqdata.cpp \
        fw/datacallreqdata.cpp \
        fw/miscdata.cpp \
        fw/miscdatabuilder.cpp \
        fw/netdata.cpp \
        fw/netdatabuilder.cpp \
        fw/oemreqdata.cpp \
        fw/psdatabuilder.cpp \
        fw/simdata.cpp \
        fw/simdatabuilder.cpp \
        fw/smsdata.cpp \
        fw/smsdatabuilder.cpp \
        fw/sounddatabuilder.cpp \
        fw/stkdata.cpp \
        fw/stkdatabuilder.cpp \
        fw/imsreqdata.cpp \
        fw/imsdatabuilder.cpp \
        fw/gpsdatabuilder.cpp \
        fw/wlandata.cpp \
        fw/wlandatabuilder.cpp \
        fw/vsimdatabuilder.cpp \
        fw/vsimdata.cpp \
        fw/nvitemdata.cpp \
        fw/embmsdata.cpp \
        fw/embmsdatabuilder.cpp \
        fw/radioconfigbuilder.cpp

LOCAL_SRC_FILES += protocol/test/testpacketbuilder.cpp

LOCAL_SRC_FILES += protocol/io/iochannel.cpp \
        protocol/io/modemcontrol.cpp

ifeq ($(USE_SIPC_PROTOCOL), true)
LOCAL_SRC_FILES += $(call all-cpp-files-under, protocol/sipc)
else
LOCAL_SRC_FILES += $(call all-cpp-files-under, protocol/sit)
endif

LOCAL_SRC_FILES += simRecord/pnnRecords.cpp \
        simRecord/oplRecords.cpp \
        simRecord/eonsResolver.cpp

LOCAL_SRC_FILES += stk/stkmodule.cpp \
        stk/tlv.cpp \
        stk/ber.cpp \
        stk/comprehension.cpp \
        stk/socket.cpp \
        stk/tlvparser.cpp \
        stk/tlvbuilder.cpp

LOCAL_SRC_FILES += util/rillog.cpp \
        util/iccUtil.cpp \
        util/util.cpp \
        util/textutils.cpp \
        util/rilproperty.cpp \
        util/reset_util.cpp \
        util/systemproperty.cpp \
        util/sms_util.cpp \
        util/carrierloader.cpp \
        util/open_carrier.cpp \
        util/rillogcapture.cpp \
        util/networkutils.cpp \
        util/EccListLoader.cpp \
        util/MccMncChanger.cpp

LOCAL_SRC_FILES += libril.cpp \
        rilapplication.cpp \
        rilcontextwrapper.cpp \
        modemstatemonitor.cpp \
        rcmmgr.cpp \
        rilexternalresponselistener.cpp \
        rilresponselistener.cpp \
        signal_handler.cpp

ifeq ($(DEVICE_SUPPORT_CDMA), true)
LOCAL_SRC_FILES += fw/cdmasmsdata.cpp \
       util/bitwiseiostream.cpp
endif

#add custom model codes
include $(LOCAL_PATH)/model/model.mk

#to add RIL library build date/time info
LOCAL_SRC_FILES += rilversioninfo.cpp

#secure c library
LOCAL_SRC_FILES += util/secure_c.cpp

# add product feature
RELATIVE_CUR_PATH := ../productfeature
include $(LOCAL_PATH)/$(RELATIVE_CUR_PATH)/productfeature.mk


include $(BUILD_SHARED_LIBRARY)
