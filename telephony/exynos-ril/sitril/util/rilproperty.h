/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/*
 * rilproperty.h
 *
 *  Created on: 2015. 1. 9.
 */

#ifndef __RIL_PROPERTY_H__
#define __RIL_PROPERTY_H__

#include "types.h"
#include "mutex.h"
#include <map>
#include <string>
#include <telephony/ril.h>

using namespace std;

#ifndef PROP_VALUE_MAX
#define PROP_VALUE_MAX 256
#endif

// RIL Application property
// - global unique value in RIL application
#define RIL_APP_NET_PS_SERVICE_ALLOW    "vendor.ril.app.net.ps_service_allow"
#define RIL_APP_PS_ACTIVE_SIM           "vendor.ril.app.ps.active_sim"
#define RIL_SIM_OVERALL_CARDSTATE       "vendor.ril.app.sim.overall_cardstate"
#define RIL_APP_MAIN_SIM                "vendor.ril.app.mainsim.phoneid"
#define RIL_APP_PS_ALLOW_SYNCDONE       "vendor.ril.app.ps.allow_syncdone"

#define RIL_APP_USERMODE                "vendor.ril.app.usermode"
#define RIL_APP_TARGET_OPER             "vendor.ril.app.target_carrier"
#define RIL_APP_NET_LAST_VALID_PLMN     "vendor.ril.app.net.last_valid_plmn"

// RIL Context property
//  - can be different for each SIM Context
#define RIL_CONTEXT_CS_ACTIVE_CID        "vendor.ril.context.cs.active_cid"
#define RIL_CONTEXT_SIM1_SMS_SMSC        "vendor.ril.context.sim1.sms.smsc"
#define RIL_CONTEXT_SIM2_SMS_SMSC        "vendor.ril.context.sim2.sms.smsc"
#define RIL_CONTEXT_NET_CURRENT_PLMN     "vendor.ril.context.net.current_plmn"
#define RIL_CONTEXT_PDP_ACTIVE           "vendor.ril.context.pdp.active"
#define RIL_CONTEXT_SIM_OPERATOR         "vendor.ril.context.sim.carrier"
#define RIL_CONTEXT_SIM_IMSI             "vendor.ril.context.sim.imsi"
#define RIL_CONTEXT_UICC_STATUS          "vendor.ril.context.sim.uicc_status"
#define RIL_CONTEXT_SIM_ICC_ID           "vendor.ril.context.sim.icc.id"

#if 0   // instead of sim hot swap detecting codes add OnSimStatusChanged handler in PSService
#define RIL_CONTEXT_SIM_HOT_SWAP         "vendor.ril.context.sim.hotswap"
#endif
#define RIL_CONTEXT_NET_VOICE_RADIO_TECH "vendor.ril.context.net.voice_rat"
#define RIL_CONTEXT_NET_DATA_RADIO_TECH  "vendor.ril.context.net.data_rat"
#define RIL_CONTEXT_NET_VOICE_REGISTRATION_STATE  "vendor.ril.context.net.voice_state"
#define RIL_CONTEXT_NET_DATA_REGISTRATION_STATE   "vendor.ril.context.net.data_state"
#define RIL_CONTEXT_IMS_APP_TYPE         "vendor.ril.context.sim.ims_app_type"
#define RIL_CONTEXT_MODEM1_STACK_STATUS   "vendor.ril.context.modem1.stack_status"
#define RIL_CONTEXT_MODEM2_STACK_STATUS   "vendor.ril.context.modem2.stack_status"

#define RIL_CONTEXT_GSM_SIM_IMSI         "gsm.sim.imsi"
#define RIL_CONTEXT_GSM_SIM_AID          "gsm.sim.aid"
#define RIL_CONTEXT_CDMA_SIM_IMSI        "cdma.sim.imsi"
#define RIL_CONTEXT_CDMA_SIM_AID         "cdma.sim.aid"
#define RIL_CONTEXT_IMS_REGISTRATION     "vendor.ril.context.ims.registration"

#define RIL_APP_PHONE_COUNT              "radio.config.phone_count"
#define RIL_APP_MULTISIM                 "radio.config.multisim"

// System property
// - system-wide unique value
#define RO_CONFIG_TARGET_CARRIER         "ro.vendor.config.build_carrier"
#define RO_CONFIG_IS_CTC                 "ro.vendor.config.ctc"

#define RIL_CALL_WAITING_PREFIX          "persist.vendor.radio.call_waiting_"
#define RIL_SIM1_OPEN_CARRIER_ID         "persist.vendor.ril.sim1.oc_name_id"
#define RIL_SIM2_OPEN_CARRIER_ID         "persist.vendor.ril.sim2.oc_name_id"
#define RIL_INTERNAL_ATTACH              "persist.vendor.ril.internal_attach"
#define VENDOR_CP_DEBUG_OFF_ON_BOOT      "persist.vendor.ril.cpdebugoff.onboot"
#define RIL_SGC_TEST_CONFIG              "persist.vendor.radio.sgc"
#define VENDOR_RIL_SUPPORT_NR            "persist.vendor.ril.support_nr"
#define VENDOR_RIL_EMERGENCY_NUMBER_SIM  "persist.vendor.ril.emergencynumber.sim"

#define RIL_NET_REJECT_CAUSE             "vendor.ril.net.rejectcause"
#define RIL_NET_DINIED_ROAM              "vendor.ril.net.denied_roam_"       // 0:N/A 1: CS denied roam, 2: PS denied roam

#define RIL_PS_DEFAULT_IPV6_DNS          "vendor.ril.ps.default_ipv6dns"

#define VENDOR_RIL_RESET_TYPE            "vendor.ril.reset_type"
#define VENDOR_RIL_DATA_STATE            "vendor.ril.data_state"

#define VENDOR_GSM_SIM_IMEI1             "vendor.gsm.sim.imei1"
#define VENDOR_GSM_SIM_IMEI2             "vendor.gsm.sim.imei2"

#define RIL_UIM_REMOTE_MCC               "persist.radio.uim.remote.mcc"
#define RIL_UIM_REMOTE_SLOT              "persist.radio.uim.remote.slot"

// AOSP origin property
#define RO_BUILD_TYPE                    "ro.build.type"
#define RO_BUILD_DISPLAY_ID              "ro.build.display.id"
#define RO_BUILD_ID                      "ro.build.id"
#define RO_PRODUCT_MODEL                 "ro.product.model"
#define RO_PRODUCT_MODEL_DM              "ro.product.model.dm"
#define RO_PRODUCT_NAME                  "ro.product.name"
#define PROPERTY_ICC_OPERATOR_NUMERIC    "gsm.sim.operator.numeric"
#define PROPERTY_ICC_OPERATOR_ALPHA      "gsm.sim.operator.alpha"
#define RO_IMEI_SVN                      "ro.radio.imei.sv"
#define SYS_BOOT_REASON                  "sys.boot.reason"

#define RIL_VENDOR_RADIO_DUAL_VOLTE       "persist.vendor.radio.dual.volte"
#define RIL_VENDOR_BASEBAND_HW_VERSION    "persist.vendor.radio.cp.hw.version"

#define RIL_VENDOR_RF_CONNECTION        "vendor.ril.rf.connection"

class RilProperty {
    DECLARE_MODULE_TAG()

private:
    map<string, string> mData;
    CMutex mLock;
    int m_sockId;
    char m_modName[32];

public:
    RilProperty();
    virtual ~RilProperty();

public:
    void SetSocketId(int sockId);
    int GetInt(string &key);
    int GetInt(string &key, int defValue);
    int GetInt(const char *key);
    int GetInt(const char *key, int defValue);
    string GetString(string &key);
    string GetString(string &key, string &defValue);
    string GetString(const char *key);
    string GetString(const char *key, const char *defValue);
    bool GetBool(string &key);
    bool GetBool(string &key, bool defValue);
    bool GetBool(const char *key);
    bool GetBool(const char *key, bool defValue);

    void Put(string &key, int value);
    void Put(const char *key, int value);
    void Put(string &key, string &value);
    void Put(const char *key, const char *value);
    void Put(string &key, bool value);
    void Put(const char *key, bool value);
    void Dump() const;

    static bool IsUserMode(void);
};

void SetCallWaiting(bool bEnable, RIL_SOCKET_ID socketid);
bool GetCallWaiting(RIL_SOCKET_ID socketid);

#endif /* __RIL_PROPERTY_H__ */
