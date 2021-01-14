/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __RIL_DEF_H__
#define __RIL_DEF_H__

#include <telephony/ril.h>
#include <telephony/ril_mnc.h>
#include <slsi/radio_v1_2.h>
#include <slsi/radio_v1_3.h>
#include <slsi/radio_v1_4.h>
#include "types.h"
#include "constdef.h"
#include "oemril.h"
#include "build.h"

/* Radio Config */
#include <slsi/radioconfig_v1_0.h>
#include <slsi/radioconfig_v1_1.h>
#include <slsi/radioconfig_v1_2.h>

/* RIL Extension */
#include <telephony/ril_ext.h>

/* RIL vendor external */
#include <slsi/ril_external.h>

#include <slsi/vendor.h>

/*
 * Facility Lock State
 */
enum {
    SIM_FAC_LOCK_STATE_UNLOCK,
    SIM_FAC_LOCK_STATE_LOCK
};

/*
 * Facility Service Class Bit
 */
enum {
    SIM_FAC_SERVICE_CLASS_BIT_NONE = 0,
    SIM_FAC_SERVICE_CLASS_BIT_DEFAULT = 7,
    SIM_FAC_SERVICE_CLASS_BIT_VOICE = 1,
    SIM_FAC_SERVICE_CLASS_BIT_DATA = 2,
    SIM_FAC_SERVICE_CLASS_BIT_FAX = 4,
    SIM_FAC_SERVICE_CLASS_BIT_SMS = 8,
    SIM_FAC_SERVICE_CLASS_BIT_DATA_SYNC = 16,
    SIM_FAC_SERVICE_CLASS_BIT_DATA_ASYNC = 32,
    SIM_FAC_SERVICE_CLASS_BIT_PACKET_ACCESS = 64,
    SIM_FAC_SERVICE_CLASS_BIT_PAD_ACCESS = 128
};

// Call & SS
enum CallType {
    CALL_TYPE_VOICE          = 0,
    CALL_TYPE_VIDEO          = 1,
    CALL_TYPE_EMERGENCY      = 2,
#ifdef SUPPORT_CDMA
    CALL_TYPE_CDMA_VOICE     = 3,
    CALL_TYPE_CDMA_EMERGENCY = 4
#endif
};

/*
 * Voice/video call status for call status notification
 */
enum CallStatusType {
    CALL_STATUS_NONE = 0,
    CALL_STATUS_OUTGOING = 1,
    CALL_STATUS_INCOMING = 2,
    CALL_STATUS_CONNECTED = 3,
    CALL_STATUS_RELEASED = 4,
    CALL_STATUS_CONNECTING = 5,
    CALL_STATUS_HOLDING = 6
};

/*
  namePresentation @ Ril_Call
    int             namePresentation;  0=Allowed, 1=Restricted, 2=Not Specified/Unknown 3=Payphone
 */
enum CallPresentation {
    RIL_CALL_NAME_PRESENTATION_ALLOW =      0,
    RIL_CALL_NAME_PRESENTATION_RESTRICT =   1,
    RIL_CALL_NAME_PRESENTATION_UNKNOWN =    2,
    RIL_CALL_NAME_PRESENTATION_PAYPHONE =   3
};

/*
 * CDMA Voice Privacy Mode
 */
 /* nonzero if CDMA voice privacy mode is active */
enum VoicePrivacyMode {
    RIL_CALL_CDMA_VOICEPRIVACY_INACTIVE =   0,
    RIL_CALL_CDMA_VOICEPRIVACY_ACTIVE =     1
};

/**
 * Emergency callback mode type
 */
enum CallEcbModeType {
    RIL_CALL_ECB_MODE_EXIT = 0,    // E911 Callback Mode Exit
    RIL_CALL_ECB_MODE_ENTRY = 1,   // E911 Callback Mode Entry
};

typedef enum {
    SUPPORT_RAT_MODE_3GPP  = 0,
    SUPPORT_RAT_MODE_3GPP2 = 1,
    SUPPORT_RAT_MODE_ALL   = 2,
} RIL_SupportRatMode;


/*
 * CLIR
 */
/* (same as 'n' paremeter in TS 27.007 7.7 "+CLIR"
 * clir == 0 on "use subscription default value"
 * clir == 1 on "CLIR invocation" (restrict CLI presentation)
 * clir == 2 on "CLIR suppression" (allow CLI presentation)
 */
enum ClirType {
    CLIR_DEFAULT =      0,  // (use subscription default value)
    CLIR_INVOCATION =   1,  // (restrict CLI presentation)
    CLIR_SUPPRESSION =  2   // (allow CLI presentation)
};

/* (same as 'm' parameter in TS 27.007 7.7 "+CLIR"
* 0 CLIR not provisioned
* 1 CLIR provisioned in permanent mode
* 2 unknown (e.g. no network, etc.)
* 3 CLIR temporary mode presentation restricted
* 4 CLIR temporary mode presentation allowed
*/
enum ClirStatusType{
    CLIR_NOT_PROVISIONED =  0,
    CLIR_PROVISIONED =      1,
    CLIR_UNKNOWN =          2,
    CLIR_TEMP_RESTRICTED =  3,
    CLIR_TEMP_ALLOWED =     4
};

/*
 * CLIP
 */
/*  * (int *)response)[0] is 1 for "CLIP provisioned"
 *                           and 0 for "CLIP not provisioned"
 *                           and 2 for "unknown, e.g. no network etc"
 */
enum ClipStatusType {
    CLIP_NOT_PROVISIONED =  0,
    CLIP_PROVISIONED =      1,
    CLIP_UNKNOWN =          2,
};

/**
 * Ss class type
 */
/*
 * ((const int *)data)[1] is the TS 27.007 service class bit vector of
 *                           services to modify
 */
 /*
 <classx>: is a sum of integers each representing a class of information (default 7 - voice, data and fax)
1 voice (telephony)
2 data (refers to all bearer services; with <mode>=2 this may refer only to some bearer service if TA does
not support values 16, 32, 64 and 128)
4 fax (facsimile services)
8 short message service
16 data circuit sync
32 data circuit async
64 dedicated packet access
128 dedicated PAD access
*/
enum SsClassX {
    RIL_SS_CLASS_UNKNOWN =                        0,
    RIL_SS_CLASS_VOICE =                          1,
    RIL_SS_CLASS_DATA =                           2,
    RIL_SS_CLASS_FAX =                            4,
    RIL_SS_CLASS_ALLTELE_EXCSMS =                 (RIL_SS_CLASS_VOICE + RIL_SS_CLASS_FAX), // 5
    RIL_SS_CLASS_DEFAULT =                        (RIL_SS_CLASS_VOICE+RIL_SS_CLASS_DATA+RIL_SS_CLASS_FAX),     //7    TS27.007, default 7 - voice, data and fax <classx>
    RIL_SS_CLASS_SMS =                            8,
    RIL_SS_CLASS_ALLTELE =                        (RIL_SS_CLASS_VOICE + RIL_SS_CLASS_FAX + RIL_SS_CLASS_SMS), // 13
    RIL_SS_CLASS_DATA_CIRCUIT_SYNC =              16,
    RIL_SS_CLASS_AllGPRSBEARER =                  (RIL_SS_CLASS_DATA_CIRCUIT_SYNC + RIL_SS_CLASS_VOICE), // 17
    RIL_SS_CLASS_DATA_CIRCUIT_ASYNC =             32,
    RIL_SS_CLASS_ALLBEARER =                      (RIL_SS_CLASS_DATA_CIRCUIT_ASYNC + RIL_SS_CLASS_DATA_CIRCUIT_SYNC), // 48
    RIL_SS_CLASS_ALL_DEDICATED_PACKET_ACCESS =    64,
    RIL_SS_CLASS_ALL_DEDICATED_PAD_ACCESS =       128,
    RIL_SS_CLASS_ALL =                            0xFF
};

/**
 * Ss mode type
 */
  /*
     * For RIL_REQUEST_QUERY_CALL_FORWARD_STATUS
     * status 1 = active, 0 = not active
     *
     * For RIL_REQUEST_SET_CALL_FORWARD:
     * status is:
     * 0 = disable    "not active"
     * 1 = enable    "active"
     * 2 = interrogate    "query status"
     * 3 = registeration
     * 4 = erasure
     */
enum SsModeType {
    RIL_SS_MODE_DISABLE =          0,
    RIL_SS_MODE_ENABLE =           1,
    RIL_SS_MODE_INTERROGATE =      2,
    RIL_SS_MODE_REGISTRATION =     3,
    RIL_SS_MODE_ERASURE =          4
};

enum SsStatusType {
    RIL_SS_STATUS_NOT_ACTIVE =     0,
    RIL_SS_STATUS_ACTIVE =         1,
};


/**
 * Ss code type
 */
enum SsCodeType {
    RIL_SS_CODE_CONNECTED_PARTY_NUMBER = 0x00,
    RIL_SS_CODE_CFU_ACTIVE = 0x00,
    RIL_SS_CODE_FORWARDED_MT = 0x00,
    RIL_SS_CODE_CFC_ACTIVE = 0x01,
    RIL_SS_CODE_CUG_CALL_MT = 0x01,
    RIL_SS_CODE_FORWARDED_MO = 0x02,
    RIL_SS_CODE_CALL_ON_HOLD = 0x02,
    RIL_SS_CODE_CALL_WAITING = 0x03,
    RIL_SS_CODE_CALL_RETRIEVED = 0x03,
    RIL_SS_CODE_CUG_CALL_MO = 0x04,
    RIL_SS_CODE_MPTY_ENTERED = 0x04,
    RIL_SS_CODE_OUTGOING_CALLS_BARRED = 0x05,
    RIL_SS_CODE_HELD_CALL_RELEASED = 0x05,
    RIL_SS_CODE_INCOMING_CALLS_BARRED = 0x06,
    RIL_SS_CODE_FORWARD_CHECK_SS = 0x06,
    RIL_SS_CODE_CLIR_SUPPRESSION_REJ = 0x07,
    RIL_SS_CODE_ECT_REMOTE_ALERT = 0x07,
    RIL_SS_CODE_DEFLECTED_MO = 0x08,
    RIL_SS_CODE_ECT_REMOTE_ACTIVE = 0x08,
    RIL_SS_CODE_DEFLECTED_MT = 0x09,
    RIL_SS_CODE_ADDITIONAL_INCOMING_FORWARDED = 0x0A,
    RIL_SS_CODE_ALL_OUTGOING_CALLS_BARRED = 0x0B,
    RIL_SS_CODE_CALL_ON_HOLD_WITHOUT_TONE = 0x20,
    RIL_SS_CODE_MAX = 0xFF
};

/**
 * TOA = TON + NPI
 * See TS 24.008 section 10.5.4.7 for details.
 * These are the only really useful TOA values
 */
 /* in PhoneNumberUtils.java, only 2 values are defined, and checking toa is done only with TOA_International
     public static final int TOA_International = 0x91;
    public static final int TOA_Unknown = 0x81;
 */
 /* type of address, eg 145 = intl */
enum ToaType {
    RIL_TOA_UNKNOWN =              0x81,
    RIL_TOA_INTERNATIONAL =        0x91
};

/*
<reason>: integer type
0 unconditional
1 mobile busy
2 no reply
3 not reachable
4 all call forwarding (refer 3GPP TS 22.030 [19])
5 all conditional call forwarding (refer 3GPP TS 22.030 [19])
 */
enum SsCfReason {
    RIL_SS_CF_REASON_UNCONDITIONAL =    0,    // unconditional
    RIL_SS_CF_REASON_MOBILE_BUSY =      1,    // mobile busy
    RIL_SS_CF_REASON_NO_REPLY =         2,    // no reply
    RIL_SS_CF_REASON_NOT_REACHABLE =    3,    // not reachable
    RIL_SS_CF_REASON_ALL =              4,    // all call forwarding
    RIL_SS_CF_REASON_ALL_CONDITIONAL =  5     // all conditional call forwarding
};

/**
 * ((const char **)data)[0] points to a type code, which is
 *  one of these string values:
 *      "0"   USSD-Notify -- text in ((const char **)data)[1]
 *      "1"   USSD-Request -- text in ((const char **)data)[1]
 *      "2"   Session terminated by network
 *      "3"   other local client (eg, SIM Toolkit) has responded
 *      "4"   Operation not supported
 *      "5"   Network timeout
 */
enum UssdTypeCode {
    RIL_USSD_NOTIFY =                           0,
    RIL_USSD_REQUEST =                          1,
    RIL_USSD_SESSION_TERMINATED_BY_NET =        2,
    RIL_USSD_OTHER_LOCAL_CLIENT_HAS_RESPONDED = 3,
    RIL_USSD_OPERATION_NOT_SUPPORT =            4,
    RIL_USSD_NETWORK_TIMEOUT =                  5
};

/**
 * Ussd dsc type
 */
enum DcsType{
    RIL_DCS_GSM7BIT_DA = 0,
    RIL_DCS_8BIT_DATA,
    RIL_DCS_UCS2,
    RIL_DCS_RESERVED,
    RIL_DCS_EUC_KR,
    /*RIL_DCS_I1_PROTO,*/
    /*RIL_DCS_WAP_FORUM,*/
    RIL_DCS_GSM7BIT_DATA,
    RIL_DCS_UNSPECIFIED
};

/*
 * RIL_UNSOL_SUPP_SVC_NOTIFICATION
 *
 * 0 = MO intermediate result code
 * 1 = MT unsolicited result code
 */
enum SuppSvcNotiType
{
    RIL_SSNOTI_TYPE_MO = 0,
    RIL_SSNOTI_TYPE_MT = 1
};

/*
 * ISIM_AUTH_TYPE
 */
enum
{
    ISIM_AUTH_IMS = 0x00,
    ISIM_AUTH_GSM,
    ISIM_AUTH_3G
};

// SMS
typedef struct {
    int fromServiceId;
    int toServiceId;
    int fromCodeScheme;
    int toCodeScheme;
    unsigned char selected;
} BcstSmsConfInfo;

//SIM & SAT

// DEVICEID & MISC
/**
 * ((int *)response)[0] is == 0 for TTY off
 * ((int *)response)[0] is == 1 for TTY Full
 * ((int *)response)[0] is == 2 for TTY HCO (hearing carryover)
 * ((int *)response)[0] is == 3 for TTY VCO (voice carryover)
 */
enum TtyModeType {
    RIL_TTY_MODE_OFF = 0,
    RIL_TTY_MODE_FULL = 1,
    RIL_TTY_MODE_HCO = 2,
    RIL_TTY_MODE_VCO = 3
};

// PDP & NET
typedef struct {
    char longPlmn[MAX_FULL_NAME_LEN + 1];
    char shortPlmn[MAX_SHORT_NAME_LEN + 1];
    char plmn[MAX_PLMN_LEN + 1];
    const char *status;
    int    rat;
} NetworkInfo;

#define MAX_PCSCF_NUM       (3)
#define MAX_PCSCF_EXT_NUM   (5)
typedef struct {
    BOOL valid;
    BYTE addr[MAX_IPV4_ADDR_LEN];
    BYTE dns1[MAX_IPV4_ADDR_LEN];
    BYTE dns2[MAX_IPV4_ADDR_LEN];
    BYTE gw[MAX_IPV4_ADDR_LEN];
    BYTE pcscf[MAX_IPV4_ADDR_LEN * (MAX_PCSCF_NUM + MAX_PCSCF_EXT_NUM)];
} PDP_ADDR_V4;

typedef struct {
    BOOL valid;
    BYTE addr[MAX_IPV6_ADDR_LEN];
    BYTE dns1[MAX_IPV6_ADDR_LEN];
    BYTE dns2[MAX_IPV6_ADDR_LEN];
    BYTE gw[MAX_IPV6_ADDR_LEN];
    BYTE pcscf[MAX_IPV6_ADDR_LEN * (MAX_PCSCF_NUM + MAX_PCSCF_EXT_NUM)];
} PDP_ADDR_V6;

typedef union {
    PDP_ADDR_V4 ipv4;
    PDP_ADDR_V6 ipv6;
} PDP_ADDR;

typedef struct {
    BYTE octet2;
    BYTE octet3;
    BYTE octet4;
    BYTE octet5;
    BYTE octet6;
    BYTE octet7;
    BYTE octet8;
} AMBR;

typedef struct {
    int status;
    int suggestedRetryTime;
    int cid;
    int active;
    int pdpType;
    PDP_ADDR_V4 ipv4;
    PDP_ADDR_V6 ipv6;
    int mtu_size;
    int pco;
    int IPC_version;
    int pcscf_ext_count;
    AMBR ambr;
} DataCall;

typedef struct {
    BYTE status;
    BYTE type;
    BYTE qci;
    int ul_gbr;
    int dl_gbr;
    int ul_max_gbr;
    int dl_max_gbr;
} DedicatedBearerInfo;

#pragma pack(push)
#pragma pack(1)
typedef struct {
    BYTE type;
    BYTE status;
    int value;
    char apn[MAX_PDP_APN_LEN];
} SitNasTimerStatus;
#pragma pack(pop)

typedef struct {
    BYTE status;
} DataPossibleStatus;

enum {
    DATA_IS_POSSIBLE = 0,
    DATA_IS_NOT_POSSIBLE = 1,
};


// Vendor added RIL_DataCallFailCause
typedef enum {
    //modified for Vendor RIL
    PDP_SUCCESS_IPV6_RENEW_PREFIX = -100,    /* This is not a fail cause, to renew IPv6 from Link-Local
                                                                                    to Global Address */
} RIL_DataCallFailCause_Ext;;


// Vendor added RIL_PreferredNetworkType
typedef enum {
    PREF_NET_TYPE_NR_ONLY                            = 23,
    PREF_NET_TYPE_NR_LTE                             = 24,
    PREF_NET_TYPE_NR_LTE_CDMA_EVDO                   = 25,
    PREF_NET_TYPE_NR_LTE_GSM_WCDMA                   = 26,
    PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA         = 27,
    PREF_NET_TYPE_NR_LTE_WCDMA                       = 28,
    PREF_NET_TYPE_NR_LTE_TDSCDMA                     = 29,
    PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM                 = 30,
    PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA               = 31,
    PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA           = 32,
    PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA = 33,
    PREF_NET_TYPE_TD_SCDMA_CDMA = 50, /* TD-SCDMA , CDMA and EvDo */
    PREF_NET_TYPE_TD_SCDMA_CDMA_NO_EVDO = 51, /* TD-SCDMA , CDMA */
    PREF_NET_TYPE_TD_SCDMA_CDMA_EVDO_LTE = 52,             /* TD-SCDMA , LTE, CDMA and EvDo */
    PREF_NET_TYPE_TD_SCDMA_EVDO_NO_CDMA = 53 /* TD-SCDMA , EVDO */
} RIL_PreferredNetworkType_Ext;

enum {
    NOT_REGISTERED =                   0,
    REGISTERED_HOME =                  1,
    SEARCHING =                        2,
    DENIED =                          3,
    UNKNOWN =                          4,
    REGISTERED_ROAMING =               5,
    NOT_REGISTERED_EMERGENCY_ONLY =    10,
    SEARCHING_EMERGENCY_ONLY =         12,
    DENIED_EMERGENCY_ONLY =           13,
    UNKNOWN_EMERGENCY_ONLY =           14,

    DENIED_ROAMING =        50
};

enum {
    NET_REJ_CAUSE_GENERAL = 0,
    NET_REJ_CAUSE_AUTH_FAIL,
    NET_REJ_CAUSE_IMSI_UNKNOWN_IN_HLR,
    NET_REJ_CAUSE_ILLEGAL_MS,
    NET_REJ_CAUSE_ILLEGAL_ME,
    NET_REJ_CAUSE_PLMN_NOT_ALLOWED,
    NET_REJ_CAUSE_LOCATION_AREA_NOT_ALLOWED,
    NET_REJ_CAUSE_ROAMING_NOT_ALLOWED,
    NET_REJ_CAUSE_NO_SUITABLE_CELLS_IN_THIS_LOCATION_AREA,
    NET_REJ_CAUSE_NETWORK_FAIL,
};

enum {
    NETWORK_DOMAIN_UNKNOWN = 0x00,
    NETWORK_DOMAIN_CS =      0x01,
    NETWORK_DOMAIN_PS =      0x02,
};

enum {
    DATA_PROFILE_DEFAULT =   0,
    DATA_PROFILE_TETHERED =  1,
    DATA_PROFILE_IMS =       2,
    DATA_PROFILE_FOTA =      3,
    DATA_PROFILE_CBS =       4,
    DATA_PROFILE_NULL_APN_IA = 5,
    DATA_PROFILE_OEM_BASE =  1000,
};

enum {
    PDP_TYPE_UNKNOWN,
    PDP_TYPE_IPV4,
    PDP_TYPE_IPV6,
    PDP_TYPE_IPV4V6,
    PDP_TYPE_PPP
};

#define DATA_PROTOCOL_IP        "IP"
#define DATA_PROTOCOL_IPV6      "IPV6"
#define DATA_PROTOCOL_IPV4V6    "IPV4V6"
#define DATA_PROTOCOL_PPP       "PPP"

// constants of suggested retry time
#define RETRY_NO_SUGGESTED      (-1)
#define RETRY_ASAP              (0)
#define NO_RETRY                (INT32_MAX)

enum {
    SETUP_DATA_AUTH_NONE,
    SETUP_DATA_AUTH_PAP ,
    SETUP_DATA_AUTH_CHAP,
    SETUP_DATA_AUTH_PAP_CHAP,
};

enum {
    DEACT_REASON_NORMAL,
    DEACT_REASON_RADIO_SHUTDOWN,
    DEACT_REASON_PDP_RESET,
    DEACT_REASON_RSRA_FAIL,
    DEACT_REASON_PCSCF_ADDRESS_FAIL,
    DEACT_REASON_IPV6_REFRESH_FAIL,
};

#define    STR_NETWORK_STATUS_UNKNOWN      "unknown"
#define    STR_NETWORK_STATUS_AVAILABLE    "available"
#define    STR_NETWORK_STATUS_CURRENT      "current"
#define    STR_NETWORK_STATUS_FORBIDDEN    "forbidden"

#define    STR_PDP_TYPE_IPV4    "IP"
#define    STR_PDP_TYPE_IPV6    "IPV6"
#define    STR_PDP_TYPE_IPV4V6  "IPV4V6"
#define    STR_PDP_TYPE_PPP     "PPP"

#define    APN_TYPE_ALL         "*"
#define    APN_TYPE_DEFAULT     "default"
#define    APN_TYPE_MMS         "mms"
#define    APN_TYPE_SUPL        "supl"
#define    APN_TYPE_DUN         "dun"
#define    APN_TYPE_HIPRI       "hipri"
#define    APN_TYPE_FOTA        "fota"
#define    APN_TYPE_IMS         "ims"
#define    APN_TYPE_CBS         "cbs"
#define    APN_TYPE_IA          "ia"
#define    APN_TYPE_EMERGENCY   "emergency"

// defined in types.hal in radio@1.0
enum ApnTypes {
    APN_TYPE_BIT_NONE = 0,                             // None
    APN_TYPE_BIT_DEFAULT = 1 << 0,                     // APN type for default data traffic
    APN_TYPE_BIT_MMS = 1 << 1,                         // APN type for MMS traffic
    APN_TYPE_BIT_SUPL = 1 << 2,                        // APN type for SUPL assisted GPS
    APN_TYPE_BIT_DUN = 1 << 3,                         // APN type for DUN traffic
    APN_TYPE_BIT_HIPRI = 1 << 4,                       // APN type for HiPri traffic
    APN_TYPE_BIT_FOTA = 1 << 5,                        // APN type for FOTA
    APN_TYPE_BIT_IMS = 1 << 6,                         // APN type for IMS
    APN_TYPE_BIT_CBS = 1 << 7,                         // APN type for CBS
    APN_TYPE_BIT_IA = 1 << 8,                          // APN type for IA Initial Attach APN
    APN_TYPE_BIT_EMERGENCY = 1 << 9,                   // APN type for Emergency PDN. This is not an IA apn,
                                                       // but is used for access to carrier services in an
                                                       // emergency call situation.
    APN_TYPE_BIT_ALL = APN_TYPE_BIT_DEFAULT | APN_TYPE_BIT_MMS | APN_TYPE_BIT_SUPL | \
    APN_TYPE_BIT_DUN | APN_TYPE_BIT_HIPRI | APN_TYPE_BIT_FOTA | APN_TYPE_BIT_IMS | \
    APN_TYPE_BIT_CBS | APN_TYPE_BIT_IA | APN_TYPE_BIT_EMERGENCY,
};

#define UPLMN_INDEX_INVALID     0x7FFFFFFF
#define UPLMN_INDEX_MAX         149

enum {
    UPLMN_MODE_INVALID = 0x00,
    UPLMN_MODE_ADD = 0x01,
    UPLMN_MODE_EDIT = 0x02,
    UPLMN_MODE_DELETE = 0x03,
};

enum {
    UPLMN_ACT_BIT_UNKNOWN = 0x00,
    UPLMN_ACT_BIT_GSM = 0x01,
    UPLMN_ACT_BIT_GSM_COMPACT = 0x02,
    UPLMN_ACT_BIT_UTRAN = 0x04,
    UPLMN_ACT_BIT_EUTRAN = 0x08,
    UPLMN_ACT_BIT_ALL = (UPLMN_ACT_BIT_GSM | UPLMN_ACT_BIT_GSM_COMPACT | UPLMN_ACT_BIT_UTRAN | UPLMN_ACT_BIT_EUTRAN),
};

typedef struct {
    int index;
    char plmn[MAX_PLMN_LEN + 1];
    int act;
} PreferredPlmn;

enum {
    RF_CONFIG_UNSPECIFIED =     0,          // selected by baseband automatically
    RF_CONFIG_EURO_BAND =       1,          // GSM-900 / DCS-1800 / WCDMA-IMT-2000
    RF_CONFIG_US_BAND =         2,          // GSM-850 / PCS-1900 / WCDMA-850 / WCDMA-PCS-1900
    RF_CONFIG_JPN_BAND =        3,          // WCDMA-800 / WCDMA-IMT-2000
    RF_CONFIG_AUS_BAND =        4,          // GSM-900 / DCS-1800 / WCDMA-850 / WCDMA-IMT-2000
    RF_CONFIG_AUS2_BAND =       5,          // GSM-900 / DCS-1800 / WCDMA-850
    RF_CONFIG_CELLULAR_BAND =   6,          // 800-MHz Band
    RF_CONFIG_PCS_BAND =        7,          // 1900-MHz Band
    RF_CONFIG_CLASS3_BAND =     8,          // JTACS Band
    RF_CONFIG_CLASS4_BAND =     9,          // Korean PCS Band
    RF_CONFIG_CLASS5_BAND =     10,         // 450-MHz Band
    RF_CONFIG_CLASS6_BAND =     11,         // 2-GMHz IMT2000 Band
    RF_CONFIG_CLASS7_BAND =     12,         // Upper 700-MHz Band
    RF_CONFIG_CLASS8_BAND =     13,         // 1800-MHz Band
    RF_CONFIG_CLASS9_BAND =     14,         // 900-MHz Band
    RF_CONFIG_CLASS10_BAND =    15,         // Secondary 800-MHz Band
    RF_CONFIG_CLASS11_BAND =    16,         // 400-MHz European PAMR Band
    RF_CONFIG_CLASS15_BAND =    17,         // AWS Band
    RF_CONFIG_CLASS16_BAND =    18,         // US 2.5-GHz Band
};

// ril.h defines 0 to allow and 1 to disallow but
// ril.java sends 1 to allow and 0 to disallow
// MUST be checked
enum {
    DISALLOW_DATA_CALL = 0,
    ALLOW_DATA_CALL =    1,
};

enum {
    EMERGENCY_CALL_STATUS_START     = 1,
    EMERGENCY_CALL_STATUS_END       = 2,
    EMERGENCY_CALL_STATUS_CANCELED  = 3,
    EMERGENCY_CALL_STATUS_FAIL      = 4,
};

// extended RIL_RadioTechnology (undefined value in ril.h)
enum {
    //RADIO_TECH_HSPADCPLUS = 20,     // deprecated
    RADIO_TECH_UNSPECIFIED      = 255,
};

typedef enum {
    RAF_CP_UNKNOWN = (1 << 0),
    RAF_CP_GPRS = (1 << 1),
    RAF_CP_EDGE = (1 << 2),
    RAF_CP_UMTS = (1 << 3),
    RAF_CP_HSDPA = (1 << 4),
    RAF_CP_HSUPA = (1 << 5),
    RAF_CP_HSPA = (1 << 6),
    RAF_CP_LTE = (1 << 7),
    RAF_CP_HSPAP = (1 << 8),
    RAF_CP_GSM = (1 << 9),
    RAF_CP_TD_SCDMA = (1 << 10),
    RAF_CP_IS95A = (1 << 11),
    RAF_CP_IS95B = (1 << 12),
    RAF_CP_1xRTT = (1 << 13),
    RAF_CP_EVDO_0 = (1 << 14),
    RAF_CP_EVDO_A = (1 << 15),
    RAF_CP_EVDO_B = (1 << 16),
    RAF_CP_EHRPD = (1 << 17),
    RAF_CP_5G = (1 << 18)
} RIL_RadioAccessFamilyForCp;

enum {
    EMERGENCY_CALL_NOT_AVAILABLE,
    EMERGENCY_CALL_AVAILABLE,
    EMERGENCY_CALL_RETRY,
};

enum {
    NR_STATUS_NONE,
    NR_STATUS_RESTRICTED,
    NR_STATUS_NOT_RESTRICTED,
    NR_STATUS_CONNECTED,
};

/*******************************************/
// MISC
/*******************************************/
enum DeviceIdentityType {
    DEV_IDENTITY_NONE =        0x00,
    DEV_IDENTITY_IMEI =        0x01,
    DEV_IDENTITY_IMEISV =      0x02,
    DEV_IDENTITY_ESN =         0x04,
    DEV_IDENTITY_MEID =        0x08,
    DEV_IDENTITY_SERIAL =      0x10,
    DEV_IDENTITY_MANUFACTURE = 0x20,
    DEV_IDENTITY_BARCODE =     0x40,
    DEV_IDENTITY_ALL =         (DEV_IDENTITY_IMEI | DEV_IDENTITY_IMEISV | DEV_IDENTITY_ESN | DEV_IDENTITY_MEID |
                               DEV_IDENTITY_SERIAL | DEV_IDENTITY_MANUFACTURE | DEV_IDENTITY_BARCODE)
};

enum BasebandVerInfoType {
    BB_VERINFO_NONE =       0x00,
    BB_VERINFO_SW =         0x01,
    BB_VERINFO_HW =         0x02,
    BB_VERINFO_CAL_DATE =   0x04,
    BB_VERINFO_PROD_CODE =  0x08,
    BB_VERINFO_MODEL_ID =   0x10,
    BB_VERINFO_PRL_NAM =    0x20,
    BB_VERINFO_ERI_NAM =    0x40,
    BB_VERINFO_CP_CHIPSET = 0x80
};

enum LogDumpCase {
    LOG_DUMP_CASE_UNKNOWN = 0,
    LOG_DUMP_CASE_DUMPSTATE,
    LOG_DUMP_CASE_RADIO,
    LOG_DUMP_CASE_MAIN
};

enum LogDumpCause {
    LOG_DUMP_CAUSE_CP_CRASH_APP = 1,
    LOG_DUMP_CAUSE_CP_CRASH_MNR,
    LOG_DUMP_CAUSE_UNKNOWN
};

#define MAX_NV_ITEM_SIZE        128

enum TargetOperatorValue {
    TARGET_OPER_CHNOPEN = 0,
    TARGET_OPER_CHNOPEN_GCF = 1,

    TARGET_OPER_CMCC = 100,
    TARGET_OPER_CTC = 101,
    TARGET_OPER_CU = 102,

    TARGET_OPER_ATT = 200,
    TARGET_OPER_TMO = 201,
    TARGET_OPER_VZW = 202,
    TARGET_OPER_SPR = 203,

    TARGET_OPER_LATIN = 300,
    TARGET_OPER_LATIN_GCF = 301,

    TARGET_OPER_EUROPEN = 400,
    TARGET_OPER_EUROPEN_GCF = 401,

    TARGET_OPER_NTT = 500,
    TARGET_OPER_KDDI = 501,

    TARGET_OPER_CLARO_AR = 601,
    TARGET_OPER_MOV_AR = 602,
    TARGET_OPER_TUENTI_AR = 603,
    TARGET_OPER_NII_AR = 604,
    TARGET_OPER_NUESTRO_AR = 605,
    TARGET_OPER_PERSONAL_AR = 606,
    TARGET_OPER_TIGO_BO = 607,
    TARGET_OPER_VIVA_BO = 608,
    TARGET_OPER_CLARO_BR = 609,
    TARGET_OPER_VIVO_BR = 610,
    TARGET_OPER_NII_BR = 611,
    TARGET_OPER_OI_BR = 612,
    TARGET_OPER_PORTO_CONECTA_BR = 613,
    TARGET_OPER_SURF_BR = 614,
    TARGET_OPER_TIM_BR = 615,
    TARGET_OPER_CLARO_CL = 616,
    TARGET_OPER_MOV_CL = 617,
    TARGET_OPER_ENTEL_CL = 618,
    TARGET_OPER_WOM_CL = 619,
    TARGET_OPER_CLARO_CO = 620,
    TARGET_OPER_MOV_CO = 621,
    TARGET_OPER_AVANTEL_CO = 622,
    TARGET_OPER_ETB_CO = 623,
    TARGET_OPER_TIGO_CO = 624,
    TARGET_OPER_CLARO_CR = 625,
    TARGET_OPER_CLARO_DO = 626,
    TARGET_OPER_CLARO_EC = 627,
    TARGET_OPER_MOV_EC = 628,
    TARGET_OPER_CNT_EC = 629,
    TARGET_OPER_CLARO_SV = 630,
    TARGET_OPER_TIGO_SV = 631,
    TARGET_OPER_CLARO_GT = 632,
    TARGET_OPER_TIGO_GT = 633,
    TARGET_OPER_CLARO_HN = 634,
    TARGET_OPER_TIGO_HO = 635,
    TARGET_OPER_TELCEL_MX = 636,
    TARGET_OPER_MOV_MX = 637,
    TARGET_OPER_ALTAN_MX = 638,
    TARGET_OPER_ATT_MX = 639,
    TARGET_OPER_CLARO_NI = 640,
    TARGET_OPER_CLARO_PA = 641,
    TARGET_OPER_CLARO_PY = 642,
    TARGET_OPER_PERSONAL_PY = 643,
    TARGET_OPER_TIGO_PY = 644,
    TARGET_OPER_CLARO_PE = 645,
    TARGET_OPER_MOV_PE = 646,
    TARGET_OPER_ENTEL_PE = 647,
    TARGET_OPER_CLARO_PR = 648,
    TARGET_OPER_OPEN_MOBILE_PR = 649,
    TARGET_OPER_CLARO_UY = 650,
    TARGET_OPER_MOV_UY = 651,
    TARGET_OPER_ANTEL_UY = 652,
    TARGET_OPER_MOV_UZ = 653,

    TARGET_OPER_BELL = 801,
    TARGET_OPER_TELUS = 802,
    TARGET_OPER_ROGERS = 803,
    TARGET_OPER_FREEDOM = 804,
};

enum LceServiceMode {
    LCE_SERVICE_MODE_PUSH = 0,
    LCE_SERVICE_MODE_PULL = 1,
};

typedef struct {
    bool valid;
    int lte_rsrp;
    int lte_rssnr;
    int umts_sig_str;
    int umts_ecno;
} RIL_SignalStrengthForVoWifi;

/*******************************************
// SND
*******************************************/

/**
 * Sound mute status
 */
enum MuteStatus {
    RIL_MUTE_STATUS_UNMUTE =    0,
    RIL_MUTE_STATUS_MUTE =      1
};

/**
 * Ringback tone type
 */
enum RingbackToneType {
    RIL_SND_RINGBACK_TONE_END =     0,
    RIL_SND_RINGBACK_TONE_START =   1
};

/**
 * Ringback tone flags
 */
enum {
    RINGBACK_FLAG_PLAY_OUTBAND_BY_UE = 0,
    RINGBACK_FLAG_PLAY_INBAND_BY_NW = 1,
};

/**
 * Call Confirm Stats
 */
enum CallConfrmStatus {
    RIL_CALL_CONFIRM_DISABLE = 0,
    RIL_CALL_CONFIRM_ENABLE  = 1
};

/**
 * Audio quality (WB-AMR Report)
 */
enum {
    AUDIO_QUALITY_NB = 0,    // Narrow Band
    AUDIO_QUALITY_WB = 1,    // Wide Band
    AUDIO_QUALITY_SWB = 8,   // Super WB
    AUDIO_QUALITY_FB = 9,    // Full Band
};

enum {
    AUDIO_CALL_TYPE_UNKNOWN = 0,
    AUDIO_CALL_TYPE_GSM     = 1,  // GSM voice call
    AUDIO_CALL_TYPE_CDMA    = 2,  // CDMA voice call
    AUDIO_CALL_TYPE_IMS     = 4,  // IMS voice call
    AUDIO_CALL_TYPE_OTHERS  = 8,  // Etc
};

/*******************************************
// IMS
*******************************************/
/**
 * PDN IP Type
 */
enum PdnIpType {
    RIL_IMS_PDN_IPv4 = 1,
    RIL_IMS_PDN_IPv6
};

/*
 * "response" is int *
 * ((int *)response)[0] is registration state:
 *              0 - Not registered
 *              1 - Registered
 *
 * If ((int*)response)[0] is = 1, then ((int *) response)[1]
 * must follow with IMS SMS format:
 *
 * ((int *) response)[1] is of type RIL_RadioTechnologyFamily
 typedef enum {
    RADIO_TECH_3GPP = 1, // 3GPP Technologies - GSM, WCDMA
    RADIO_TECH_3GPP2 = 2 // 3GPP2 Technologies - CDMA
} RIL_RadioTechnologyFamily;
*/
enum ImsRegistrationState {
    RIL_IMS_NOT_REGISTERED = 0,
    RIL_IMS_REGISTERED
};

/*******************************************
// NETWORK
*******************************************/
/**
 * Micor Cell Search Mode
 */
enum MicroCellSrchModeType
{
    RIL_MC_SRCH_MODE_MANUAL = 0,
    RIL_MC_SRCH_MODE_AUTO = 1,
    RIL_MC_SRCH_MODE_CANCEL = 2,
};

enum CurrentSimState
{
    RIL_CURRENT_SIM_DEACTIVATED = 0,
    RIL_CURRENT_SIM_ACTIVATED = 1,
};

enum NetworkModeSelectionType
{
    RIL_NET_NETWORK_MODE_AUTOMATIC = 0x00,
    RIL_NET_NETWORK_MODE_MANUAL,

    RIL_NET_NETWORK_MODE_MAX
};

enum HsdpaStatusType
{
    RIL_NET_DISABLED_HSDPA      = 0x00,         /* 0x00 : disable HSDPA option */
    RIL_NET_HSDPA,                              /* 0x01 : enable HSDPA option */
    RIL_NET_HSPA_PLUS,                          /* 0x02 : enable HSPA PLUS option */
    RIL_NET_HSUPA,                              /* 0x03 : enable HSUPA option */
    RIL_NET_HSPA_PLUS_DC        = 0x10,         /* 0x10 : enable HSPAP DC PLUS option (for specific operator)*/
    RIL_NET_LTE_CA_DISABLE      = 0x20,         /* 0x20 : disable Carrier Aggregation, aron.kwon 140526 Display 4G icon */
    RIL_NET_LTE_CA_ENABLE       = 0x21,         /* 0x21 : enable Carrier Aggregation, aron.kwon 140526 Display 4G icon */
    RIL_NET_SETTING_MAX
};

/* refer RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE in ril.h
 * ((int *)response)[0] is == 0 for Home Networks only, as defined in PRL
 * ((int *)response)[0] is == 1 for Roaming on Affiliated networks, as defined in PRL
 * ((int *)response)[0] is == 2 for Roaming on Any Network, as defined in the PRL
 */
enum CdmaRoamingType
{
    CDMA_ROAMING_HOME_ONLY = 0,
    CDMA_ROAMING_AFFILIATED_NETWORKS,
    CDMA_ROAMING_ANY_NETWORK,
};

enum {
    OPERATOR_REG_DEFAULT = 0,
    OPERATOR_REG_HOME = 1,
    OPERATOR_REG_ROAM = 2,
    OPERATOR_REG_UNKNOWN
};

enum {
    EONS_RESOLVER_REG_OPERATOR,
    EONS_RESOLVER_AVAILABLE_NETWORK,
};

/*******************************************
// SMS
*******************************************/

enum SmsRuimStatusType {
    RIL_RUIM_STATUS_RECEIVED_UNREAD = 0,
    RIL_RUIM_STATUS_RECEIVED_READ = 1,
    RIL_RUIM_STATUS_STORED_UNSENT = 2,
    RIL_RUIM_STATUS_STORED_SENT = 3,

    RIL_RUIM_STATUS_MAX
};

// refer RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION in ril.h
// 0 - Activate, 1 - Turn off
enum SmsCdmaBcstActType {
    RIL_SMS_CDMA_BCST_ACT_ACTIVATE = 0x00,
    RIL_SMS_CDMA_BCST_ACT_DEACTIVATE,

    RIL_SMS_CDMA_BCST_ACT_MAX
};

// eMBMS SAIL List

#define MAX_INTER_SAI_NUMBER        (64)
#define MAX_MULTI_BAND_NUMBER       (8)
#define MAX_INTRA_SAILIST_NUMBER    (64)
#define MAX_INTER_SAILIST_NUMBER    (8)

typedef struct
{
    UINT32 Frequency;
    BYTE InterSaiNumber; // max 64
    BYTE MultiBandInfoNumber; // max 8
    UINT16 InterSaiInfo[MAX_INTER_SAI_NUMBER];
    UINT8 MultiBandInfo[MAX_MULTI_BAND_NUMBER];
} __attribute__((packed)) EMBMS_InterSaiList;

typedef struct
{
    int IntraSaiListNum; // max 64
    int InterSaiListNum; // max 8
    UINT16 IntraSaiList[MAX_INTRA_SAILIST_NUMBER];
    EMBMS_InterSaiList InterSaiList[MAX_INTER_SAILIST_NUMBER];
} __attribute__((packed)) EMBMS_SaiList;

/*******************************************
// SIM
*******************************************/
typedef struct {
    char mcc[MAX_MCC_LEN + 1];
    char mnc[MAX_MNC_LEN + 1];
    RIL_CarrierMatchType match_type;
    char match_data[MAX_CR_MATCH_DATA_SIZE+1];
} CarrierInfo;

#define DEFAULT_UUID "com.slsi.exynos"

/**
 * OEM Modem Info
 */
enum {
    MODEM_INFO_SCREEN_STATE = 1,
    MODEM_INFO_CELL_INFO = 2,
    MODEM_INFO_CA_MIMO_HORXD = 3,
    MODEM_INFO_VOLTE = 4,
    MODEM_INFO_CDRX = 5,
    MODEM_INFO_BAND_CONFIG = 7,
    MODEM_INFO_HW_BAND = 8,
};

enum {
    MODEM_INFO_MIMO_INFO = 1,
    MODEM_INFO_SCG_FAIL_INFO = 2,
    MODEM_INFO_NR_CELL_INFO = 3,
};

#endif
