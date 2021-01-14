/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _SIT_DEF_H_
#define _SIT_DEF_H_
/******************************************************************************
 *
 * File: sitdef.h
 *
 * Description: Define SIT(Shannon Interface for Telephony) Command Structure
 *
  *****************************************************************************/

/**********************************************************************************

                       S I T   M E S S A G E   F O R M A T

***********************************************************************************/
/* SIT HEADER
-----------------------------------------------------------------------------------
  | LENGTH(4) | SEQ_NUM(4) | CONFIG(1) | CH_ID(2) | RESERVED(1) | PAYLOAD(X)
  ----------------------------------------------------------------------------------
*/

/* SIT PAYLOAD
-----------------------------------------------------------------------------------
  | CMD_ID(2) | TRANS_ID(4) | LENGTH(4) | PARAMETER(X)
  ----------------------------------------------------------------------------------
*/

/*********************************************************************************/

#include "types.h"
#include "constdef.h"

#pragma pack(1)


#ifndef packed__
#define packed__ __attribute__ ((packed))
#endif

//-------------------------------------------------------------------------------------------------
// Codes below must be consistently updated all together. Be careful!!
//    : Each 'enum', 'rcm_name', and 'rcm_err_name' must be consistent for a proper log printing.
//
// [2014-07-18] Written by arkade (arkade.kang@samsung.com)
//-------------------------------------------------------------------------------------------------
#define RCM_CATEGORY                       17
typedef enum
{
    SIT_BASE_ID_CALL = 0x0000,
    SIT_BASE_ID_SMS = 0x0100,
    SIT_BASE_ID_SIM = 0x0200,
    SIT_BASE_ID_SAT = 0x0300,
    SIT_BASE_ID_DEVICEID =  0x0400,
    SIT_BASE_ID_SS = 0x0500,
    SIT_BASE_ID_PDP = 0x0600,
    SIT_BASE_ID_NET = 0x0700,
    SIT_BASE_ID_PWR = 0x0800,
    SIT_BASE_ID_MISC = 0x0900,
    SIT_BASE_ID_SND = 0x0A00,
    SIT_BASE_ID_IMS = 0x0B00,
    SIT_BASE_ID_AGPS = 0x0C00,
    SIT_BASE_ID_AIMS = 0x0D00,
    SIT_BASE_ID_VSIM = 0x0E00,
    SIT_BASE_ID_WFC = 0x0F00,
    SIT_BASE_ID_EMBMS = 0x1000,
    SIT_BASE_ID_OEM = 0x4000,
    SIT_BASE_ID_OEM2 = 0x4100,
    SIT_BASE_ID_OEM3 = 0x4200,
    SIT_BASE_ID_OEM4 = 0x4300,
    SIT_BASE_ID_OEM5 = 0x4400,
}SIT_BASE_ID;

typedef enum
{
    SIT_GET_CURRENT_CALLS = 0x0000,
    SIT_DIAL = 0x0001,
    SIT_GET_LAST_CALL_FAIL_CAUSE = 0x0002,
    SIT_DTMF = 0x0003,
    SIT_ANSWER = 0x0004,
    SIT_DTMF_START = 0x0005,
    SIT_DTMF_STOP = 0x0006,
    SIT_SEND_EXPLICIT_CALL_TRANSFER = 0x0007,
    SIT_HANGUP = 0x0008,
    SIT_IND_CALL_STATE_CHANGED = 0x0009,
    SIT_IND_CALL_RING = 0x000A,
    SIT_SET_SRVCC_CALL_LIST = 0x000B,
    SIT_IND_SRVCC_HO = 0x000C,
    SIT_IND_EMERGENCY_CALL_LIST  = 0x000D,
    SIT_CALL_CONFIRM_FEATURE_SET_REQ = 0x000E,
    SIT_CALL_CONFIRM = 0x000F,
    SIT_IND_CALL_PRESENT_IND = 0x0010,
#ifdef SUPPORT_CDMA
    SIT_SET_CDMA_VOICE_PRIVACY_MODE = 0x0011,
    SIT_GET_CDMA_VOICE_PRIVACY_MODE = 0x0012,
    SIT_IND_CDMA_VOICE_PRIVACY_MODE = 0x0013,
#endif

    SIT_IND_ENTER_EMERGENCY_CB_MODE = 0x0014,

#ifdef SUPPORT_CDMA
    SIT_CDMA_BURST_DTMF = 0x0015,
    SIT_IND_OTA_PROVISION_STATUS = 0x0016,
    SIT_IND_CDMA_INFO_REC = 0x0017,
#endif

    SIT_IND_EMERGENCY_SUPPORT_RAT_MODE = 0x0018,
    SIT_IND_EXIT_EMERGENCY_CB_MODE = 0x0019,
    SIT_EXIT_EMERGENCY_CB_MODE = 0x001A,
}SIT_ID_CALL;

typedef enum
{
    SIT_SEND_SMS  = 0x0100,
    SIT_SEND_SMS_EXPECT_MORE = 0x0101,
    SIT_SEND_SMS_ACK = 0x0102,
    SIT_WRITE_SMS_TO_SIM = 0x0103,
    SIT_DELETE_SMS_ON_SIM = 0x0104,
    SIT_GET_BCST_SMS_CFG = 0x0105,
    SIT_SET_BCST_SMS_CFG = 0x0106,
    SIT_ACT_BCST_SMS = 0x0107,
    SIT_GET_SMSC_ADDR = 0x0108,
    SIT_SET_SMSC_ADDR = 0x0109,
    SIT_SEND_SMS_MEM_STATUS = 0x010A,
    SIT_SEND_ACK_INCOMING_SMS = 0x010B,
    SIT_IND_NEW_SMS = 0x010C,
    SIT_IND_NEW_SMS_STATUS_REPORT = 0x010D,
    SIT_IND_NEW_SMS_ON_SIM = 0x010E,
    SIT_IND_SIM_SMS_STORAGE_FULL = 0x010F,
    SIT_IND_NEW_BCST_SMS = 0x0110,

#ifdef SUPPORT_CDMA
    SIT_CDMA_SEND_SMS = 0x0111,
    SIT_CDMA_SEND_SMS_ACK = 0x0112,
    SIT_CDMA_GET_BCST_SMS_CFG = 0x0113,
    SIT_CDMA_SET_BCST_SMS_CFG = 0x0114,
    SIT_CDMA_ACT_BCST_SMS = 0x0115,
    SIT_CDMA_WRITE_SMS_TO_RUIM = 0x0116,
    SIT_CDMA_DELETE_SMS_ON_RUIM = 0x0117,
    SIT_IND_CDMA_NEW_SMS = 0x0118,
    SIT_IND_CDMA_RUIM_SMS_STORAGE_FULL = 0x0119,
    SIT_IND_CDMA_VOICE_MSG_WAITING_INFO = 0x0120,
#endif // SUPPORT_CDMA
    SIT_GET_STORED_SMS_COUNT =  0x0121,
}SIT_ID_SMS;

typedef enum
{
    SIT_GET_SIM_STATUS = 0x0200,
    SIT_VERIFY_SIM_PIN = 0x0201,
    SIT_VERIFY_SIM_PUK = 0x0202,
    SIT_VERIFY_SIM_PIN2 = 0x0203,
    SIT_VERIFY_SIM_PUK2 = 0x0204,
    SIT_CHG_SIM_PIN = 0x0205,
    SIT_CHG_SIM_PIN2 = 0x0206,
    SIT_VERIFY_NETWORK_LOCK = 0x0207,
    SIT_SIM_IO = 0x0208,
    SIT_GET_FACILITY_LOCK = 0x0209,
    SIT_SET_FACILITY_LOCK = 0x020A,
    SIT_GET_SIM_AUTH = 0x020B,
    SIT_TRANSMIT_SIM_APDU_BASIC = 0x020C,
    SIT_OPEN_SIM_CHANNEL = 0x020D,
    SIT_CLOSE_SIM_CHANNEL = 0x020E,
    SIT_TRANSMIT_SIM_APDU_CHANNEL    = 0x020F,
    SIT_IND_SIM_STATUS_CHANGED = 0x0210,
    SIT_GET_GBA_CONTEXT = 0x0211,
    SIT_GET_ATR = 0x0212,
    SIT_READ_PB_ENTRY = 0x0240,
    SIT_UPDATE_PB_ENTRY = 0x0241,
    SIT_GET_PB_STORAGE_INFO = 0x0242,
    SIT_GET_PB_STORAGE_LIST = 0x0243,
    SIT_GET_PB_ENTRY_INFO = 0x0244,
    SIT_GET_3G_PB_CAPA = 0x0245,
    SIT_IND_SIM_PB_READY = 0x0246,
    SIT_OPEN_SIM_CHANNEL_WITH_P2 = 0x0247,
    SIT_IND_ICCID_INFO = 0x0248,
    SIT_SET_SET_UICC_SUBSCRIPTION = 0x0249,
    SIT_IND_UICC_SUBSCRIPTION_STATE_CHANGED = 0x024A,
    //SIT_GET_SIM_AUTH = 0x024B,    // ISIM_AUTH is changed into SIM_AUTH
    SIT_SET_SIM_CARD_POWER = 0x024C,

    // Radio Config
    SIT_GET_SLOT_STATUS = 0x024D,
    SIT_IND_SIM_SLOT_STATUS_CHANGED = 0x024E,
    SIT_IND_SIM_DATA_INFO = 0x024F,
    // TODO: id needs to be fixed when CP prepares this command
    SIT_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING = 0x0250,
}SIT_ID_SIM;

typedef enum
{
    SIT_SEND_STK_ENVELOPE_CMD = 0x0300,
    SIT_SEND_STK_TERMINAL_RSP = 0x0301,
    SIT_SEND_STK_ENVELOPE_WITH_STATUS= 0x0302,
    SIT_IND_STK_PROACTIVE_COMMAND = 0x0303,
    SIT_IND_SIM_REFRESH = 0x0304,
    SIT_STK_CALL_SETUP = 0x0305,
    SIT_IND_STK_SESSION_END = 0x0306,
    SIT_IND_STK_CC_ALPHA_NOTIFY = 0x0307,
}SIT_ID_SAT;

typedef enum
{
    SIT_GET_IMSI =0x0400,
    SIT_GET_IMEI =0x0401,
    SIT_GET_IMEISV =0x0402,
    SIT_GET_DEVICE_ID =0x0403,
    SIT_SET_SGC = 0x0404,
    SIT_GET_SGC = 0x0405,
    SIT_SET_CARRIER_INFO_IMSI_ENCRYPTION =0x0406,
    SIT_IND_CARRIER_INFO_IMSI_ENCRYPTION = 0x0407,
}SIT_ID_DEVICEID;

typedef enum
{
    SIT_HANGUP_WAITING_OR_BACKGROUND = 0x0500,
    SIT_HANGUP_FORGROUND_RESUME_BACKGROUND = 0x0501,
    SIT_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE = 0x0502,
    SIT_CONFERENCE = 0x0503,
    SIT_UDUB = 0x0504,
    SIT_SEND_USSD = 0x0505,
    SIT_CANCEL_USSD = 0x0506,
    SIT_GET_CLIR = 0x0507,
    SIT_GET_COLP= 0x0508,
    SIT_GET_CALL_FORWARD_STATUS = 0x0509,
    SIT_SET_CALL_FORWARD = 0x050A,
    SIT_GET_CALL_WAITING = 0x050B,
    SIT_SET_CALL_WAITING = 0x050C,
    SIT_CHG_BARRING_PWD = 0x050D,
    SIT_SEPARATE_CONNECTION = 0x050E,
    SIT_GET_CLIP = 0x050F,
    SIT_IND_ON_USSD = 0x0510,
    SIT_IND_SUPP_SVC_NOTIFICATION = 0x0511,
    SIT_GET_COLR= 0x0512,
    SIT_IND_SS_RETURN_RESULT = 0x0513,
#ifdef SUPPORT_CDMA
    SIT_SET_CDMA_FLASH_INFO = 0x0514,
    SIT_IND_CDMA_CALL_WAITING = 0x0515,
#endif
    SIT_IND_ON_SS = 0x0516,
}SIT_ID_SS;

typedef enum
{
    SIT_SETUP_DATA_CALL = 0x0600,
    SIT_DEACT_DATA_CALL = 0x0601,
    SIT_GET_DATA_CALL_LIST = 0x0602,
    SIT_SET_INITIAL_ATTACH_APN = 0x0603,
    SIT_IND_DATA_CALL_LIST_CHANGED = 0x0604,
    SIT_SET_FD_INFO = 0x0605,
    SIT_IND_DEDICATED_BEARER_INFO = 0x0606,
    SIT_IND_NAS_TIMER_STATUS = 0x0607,
    SIT_DETACH = 0x0608,
    SIT_IND_DATA_STATE_CHANGE = 0x0609,
    SIT_START_KEEPALIVE = 0x060A,
    SIT_STOP_KEEPALIVE = 0x060B,
    SIT_IND_KEEPALIVE_STATUS = 0x060C,
    SIT_IND_PCO_DATA = 0x060D,
    SIT_START_LCE_INFO = 0x060E,
    SIT_STOP_LCE_INFO = 0x060F,
    SIT_GET_LCE_DATA = 0x0610,
    SIT_IND_LCE_DATA = 0x0611,
    SIT_SET_DATA_PROFILE = 0x0613,
    SIT_SET_IMS_TEST_MODE = 0x0614,
    SIT_GET_PHONE_CAPABILITY = 0x0615,
}SIT_ID_PDP;

typedef enum
{
    SIT_GET_CS_REG_STATE = 0x0700,
    SIT_GET_PS_REG_STATE = 0x0701,
    SIT_GET_OPERATOR = 0x0702,
    SIT_GET_NTW_MODE = 0x0703,
    SIT_SET_NTW_MODE_AUTO = 0x0704,
    SIT_SET_NTW_MODE_MANUAL = 0x0705,
    SIT_GET_AVAILABLE_NETWORKS = 0x0706,
    SIT_CANCEL_GET_AVAILABLE_NETWORKS = 0x0707,
    SIT_SET_BAND_MODE = 0x0708,
    SIT_GET_BAND_MODE = 0x0709,
    SIT_SET_PREFERRED_NTW_TYPE = 0x070A,
    SIT_GET_PREFERRED_NTW_TYPE = 0x070B,
    SIT_GET_CELL_INFO_LIST = 0x070C,
    SIT_SET_CELL_INFO_LIST_REPORT_RATE = 0x070D,
    SIT_IND_NTW_STATE_CHANGED = 0x070E,
    SIT_IND_CELL_INFO_LIST = 0x070F,
    SIT_SET_PS_SERVICE = 0x0710,
    SIT_GET_PS_SERVICE = 0x0711,
    SIT_SET_EMERGENCY_CALL_STATUS = 0x0712,
    SIT_IND_EMERGENCY_ACT_INFO = 0x0713,
    SIT_SET_UPLMN = 0x0714,
    SIT_GET_UPLMN = 0x0715,
    SIT_SET_DS_NTW_TYPE = 0x0716,
    SIT_IND_NET_CURRENT_LTE_MODE  = 0x0717,
    SIT_GET_RADIO_CAPABILITY = 0x0718,
    SIT_SET_RADIO_CAPABILITY = 0x0719,
    SIT_GET_DUPLEX_MODE  = 0x071A,
    SIT_SET_DUPLEX_MODE  = 0x071B,
    SIT_IND_LTE_CA_CHANGED  = 0x071C,
    SIT_GET_LTE_CA_INFO = 0x071D,
    SIT_GET_GSM_CELL_INFO = 0x071E,
    SIT_SET_MICRO_CELL_SEARCH = 0x071F,
    //SIT_IND_RC_INFO_RECV = 0x720,
    SIT_IND_AC_BARRING_INFO = 0x0720,
    SIT_SET_CDMA_ROAMING_PREFERENCE = 0x0721,
    SIT_GET_CDMA_ROAMING_PREFERENCE = 0x0722,
    SIT_SET_CDMA_HYBRID_MODE = 0x0723,
    SIT_GET_CDMA_HYBRID_MODE = 0x0724,
    SIT_SET_DUAL_NTW_AND_PS_TYPE = 0x072B,
    SIT_IND_TOTAL_OOS = 0x072C,
    SIT_IND_MCC = 0x072D,
    SIT_SET_CARRIER_RESTRICTIONS = 0x072E,
    SIT_GET_CARRIER_RESTRICTIONS = 0x072F,
    SIT_GET_MANUAL_BAND_MODE = 0x0730,
    SIT_SET_MANUAL_BAND_MODE = 0x0731,
    SIT_GET_RF_DESENSE_MODE = 0x0732,
    SIT_SET_RF_DESENSE_MODE = 0x0733,
    SIT_START_SCANNING_NETWORKS = 0x0734,
    SIT_STOP_SCANNING_NETWORKS = 0x0735,
    SIT_IND_SCANNING_NETWORKS = 0x0736,
    SIT_IND_RADIO_CAPABILITY = 0x0737,
    SIT_GET_MANUAL_RAT_MODE = 0x0738,
    SIT_SET_MANUAL_RAT_MODE = 0x0739,
    SIT_GET_FREQUENCY_LOCK = 0x073A,
    SIT_SET_FREQUENCY_LOCK = 0x073B,
    SIT_IND_B2_B1_CONFIG = 0x073C,
    SIT_SET_ENDC_MODE = 0x073D,
    SIT_GET_ENDC_MODE = 0x073E,
    SIT_IND_SCG_BEARER_ALLOCATION = 0x073F,
    // Implementation is in PSService
    SIT_SET_PREFERRED_DATA_MODEM = 0x0740,
    // SIT_SET_MODEMS_CONFIG = ?,
    // SIT_GET_MODEMS_CONFIG = ?,
    // SIT_GET_PHONE_CAPABILITY = ?,
    SIT_IND_FREQUENCY_INFO = 0x0741,
    SIT_IND_PHYSICAL_CHANNEL_CONFIG = 0x0742,
    SIT_SET_LOCATION_UPDATE_SETTING = 0x0744,
    SIT_GET_FREQUENCY_INFO = 0x0746,
}SIT_ID_NET;

typedef enum
{
    SIT_SET_RADIO_POWER = 0x0800,
    SIT_GET_RADIO_POWER = 0x0801,
    SIT_IND_RADIO_STATE_CHANGED = 0x0802,
    SIT_IND_RADIO_READY = 0x0803,
    SIT_IND_PHONE_RESET = 0x0804,
    SIT_IND_MODEM_RESTART = 0x0805,
    SIT_SET_STACK_STATUS = 0x080F,
    SIT_GET_STACK_STATUS = 0x0810,
}SIT_ID_PWR;

typedef enum
{
    SIT_GET_SIGNAL_STRENGTH = 0x0900,
    SIT_GET_BASEBAND_VERSION = 0x0901,
    SIT_SET_SCREEN_STATE = 0x0902,
    SIT_SET_TTY_MODE = 0x0903,
    SIT_GET_TTY_MODE = 0x0904,
    SIT_IND_NITZ_TIME_RECEIVED = 0x0905,
    SIT_IND_SIGNAL_STRENGTH = 0x0906,
    SIT_SET_CFG_DEFAULT = 0x0907,
    SIT_SET_ENG_MODE = 0x0908,
    SIT_SET_SCREEN_LINE = 0x0909,
    SIT_IND_DISPLAY_ENG = 0x090A,
    SIT_SET_DEBUG_TRACE = 0x090B,
    SIT_GET_ACTIVITY_INFO = 0x090C,
    SIT_SET_OPERATOR_INFO = 0x090D,
    SIT_SET_FEATURE_INFO = 0x090E,
    SIT_SET_ENG_STRING_INPUT = 0x0910,
    SIT_GET_MSL_CODE = 0x0911,
    SIT_SET_PIN_CONTROL = 0x0920,
    SIT_IND_PIN_CONTROL = 0x0921,

#ifdef SUPPORT_CDMA
    SIT_GET_CDMA_SUBSCRIPTION = 0x090F,
#endif // SUPPORT_CDMA

    SIT_SET_VOICE_OPERATION = 0x091A,
    SIT_GET_VOICE_OPERATION = 0x091B,

    SIT_SET_SW_VERSION = 0x0922,

    SIT_GET_HW_CONFIG = 0x0926,
    SIT_IND_HW_CONFIG_CHANGED = 0x0927,
    SIT_SET_IND_CMD_FILTER = 0x0928,
    SIT_IND_CDMA_PRL_CHANGED = 0x0929,
    SIT_SET_RSSI_SCAN = 0x092D,
    SIT_IND_RSSI_SCAN = 0x092E,
    SIT_SET_PREFERRED_CALL_CAPABILITY = 0x092F,
    SIT_GET_PREFERRED_CALL_CAPABILITY = 0x0930,
    SIT_SET_FORWARDING_AT_COMMAND = 0x0931,
    SIT_IND_FORWARDING_AT_COMMAND = 0x0932,
    SIT_SET_INTPS_SERVICE = 0x0933, /* DEPRECATED */
    SIT_SET_VOWIFI_HO_THRESHOLD = 0x0935,
    SIT_GET_RADIO_NODE = 0x0936,
    SIT_SET_RADIO_NODE = 0x0937,
    SIT_GET_VOLTE_PROVISION_UPDATE = 0x0938,
    SIT_SET_VOLTE_PROVISION_UPDATE = 0x0939,
    SIT_SET_SELFLOG = 0x093A,
    SIT_GET_SELFLOG_STATUS = 0x093B,
    SIT_IND_SELFLOG_STATUS = 0x093C,
    SIT_SET_ELEVATOR_SENSOR = 0x093D,
    SIT_SET_MODEM_CONFIG = 0x093F,
    SIT_SET_MODEM_LOG_DUMP = 0x0940,
    SIT_SET_SELFLOG_PROFILE = 0x0942,
    SIT_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA = 0x0943,
    SIT_SET_LINK_CAPACITY_REPORTING_CRITERIA = 0x0944,
    SIT_IND_CURRENT_LINK_CAPACITY_ESTIMATE = 0x0945,
}SIT_ID_MISC;

typedef enum
{
    SIT_IND_SAR_CONTROL_STATE = 0x092A,
    SIT_SET_SENSOR_STATUS = 0x092B,
}SIT_ID_PSENSOR;

typedef enum
{
    SIT_SET_MUTE =0x0A00,
    SIT_GET_MUTE = 0x0A01,
    SIT_IND_RINGBACK_TONE = 0x0A02,
    SIT_SET_VOLUME = 0x0A03,
    SIT_GET_VOLUME = 0x0A04,
    SIT_SET_AUDIOPATH = 0x0A05,
    SIT_GET_AUDIOPATH = 0x0A06,
    SIT_SET_MULTI_MIC = 0x0A07,
    SIT_GET_MULTI_MIC = 0x0A08,
    SIT_SWITCH_VOICE_CALL_AUDIO = 0x0A09,
    SIT_SET_AUDIO_CLK = 0x0A0A,
    SIT_SET_AUDIO_LOOPBACK = 0x0A0B,
    SIT_IND_WB_AMR_REPORT = 0x0A0C,
    SIT_SET_WB_CAPABILITY = 0x0A0D,
    SIT_GET_WB_CAPABILITY = 0x0A0E,
    SIT_IND_RESEND_IN_CALL_MUTE = 0x0A0F,
}SIT_ID_SND;

typedef enum
{
    SIT_SET_IMS_CONFIGURATION = 0x0B00,
    SIT_GET_IMS_CONFIGURATION = 0x0B01,
    SIT_IND_IMS_CONFIGURATION = 0x0B02,
    SIT_IND_IMS_DEDICATED_PDN = 0x0B03,
}SIT_ID_IMS;

typedef enum
{
    SIT_SET_GPS_FREQUENCY_AIDING            = 0x0C00,
    SIT_GET_LPP_SUPL_REQ_ECID_INFO          = 0x0C01,
    SIT_GET_RRLP_SUPL_REQ_ECID_INFO         = 0x0C02,
    SIT_IND_GPS_MEASURE_POS_REQ             = 0x0C03,
    SIT_IND_GPS_MEASURE_POSITION_RSP        = 0x0C04,
    SIT_IND_GPS_ASSIST_DATA                 = 0x0C05,
    SIT_IND_RELEASE_GPS                     = 0x0C06,
    SIT_GPS_MO_LOCATION_REQUEST             = 0x0C07,
    SIT_IND_GPS_MT_LOCATION_REQUEST         = 0x0C08,
    SIT_IND_RESET_GPS_ASSIST_DATA           = 0x0C09,
    SIT_GET_LPP_REQ_SERVING_CELL_INFO       = 0x0C0A,
    SIT_IND_LPP_REQUEST_CAPABILITIES        = 0x0C0B,
    SIT_LPP_PROVIDE_CAPABILITIES_IND        = 0x0C0C,
    SIT_IND_LPP_REQUEST_ASSIST_DATA         = 0x0C0D,
    SIT_IND_LPP_PROVIDE_ASSIST_DATA         = 0x0C0E,
    SIT_IND_LPP_REQUEST_LOCATION_INFO       = 0x0C0F,
    SIT_LPP_PROVIDE_LOCATION_INFO_IND       = 0x0C10,
    SIT_LPP_GPS_ERROR_IND                   = 0x0C11,
    SIT_IND_SUPL_LPP_DATA_INFO              = 0x0C12,
    SIT_IND_SUPL_NI_MESSAGE                 = 0x0C13,
    SIT_SET_GPS_SUPL_NI_READY               = 0x0C14,
    SIT_IND_3GPP_SEND_GANSS_ASSIT_DATA      = 0x0C15,
    SIT_IND_GANSS_MEAS_POS_MSG              = 0x0C16,
    SIT_SET_GANSS_MEAS_POS_RSP              = 0x0C17,
    SIT_IND_GANSS_AP_POS_CAP_REQ            = 0x0C18,
    SIT_IND_GANSS_AP_POS_CAP_RSP            = 0x0C19,
    SIT_GET_GSM_EXT_INFO_MSG                = 0x0C1A,
    SIT_GPS_CONTROL_PLANE_ENABLE            = 0x0C1B,
    SIT_GNSS_LPP_PROFILE_SET                = 0x0C1C,
    SIT_SET_GPS_LOCK_MODE                   = 0x0C20,
    SIT_GET_REFERENCE_LOCATION              = 0x0C21,
    SIT_IND_CDMA_GPS_POWER_ON               = 0x0C22,
    SIT_SET_PSEUDO_RANGE_MEASUREMENTS       = 0x0C23,
    SIT_IND_CDMA_SEND_ACQUSITION_ASSIT_DATA = 0x0C24,
    SIT_IND_CDMA_SESSION_CANCELLATION       = 0x0C25,
    SIT_GET_CDMA_PRECISE_TIME_AIDING_INFO   = 0x0C26,
    SIT_GET_GPS_CDMA_FREQ_AIDING            = 0x0C27,
    SIT_IND_GPS_LOCK_MODE                   = 0x0C28,
    SIT_IND_GPS_START_MDT_LOC               = 0x0C29,
    SIT_GPS_RETRIEVE_LOC_INFO               = 0x0C2A,
    SIT_IND_LPP_UPDATE_UE_LOC_INFO          = 0x0C2B,
}SIT_ID_GPS;

//AIMS support start ---------------------
typedef enum
{
    SIT_AIMS_DIAL                               = 0x0D00,
    SIT_IND_AIMS_CALL_RING                      = 0x0D01,
    SIT_AIMS_ANSWER                             = 0x0D02,
    SIT_AIMS_HANGUP                             = 0x0D03,
    SIT_IND_AIMS_CALL_STATUS                    = 0x0D04,
    SIT_IND_AIMS_REGISTRATION                   = 0x0D05,
    SIT_AIMS_DEREGISTRATION                     = 0x0D06,
    SIT_AIMS_HIDDEN_MENU                        = 0x0D07,
    SIT_AIMS_ADD_PDN_INFO                       = 0x0D08,
    SIT_AIMS_CALL_MANAGE                        = 0x0D09,
    SIT_AIMS_SEND_DTMF                          = 0x0D0A,
    SIT_AIMS_SET_FRAME_TIME                     = 0x0D0B,
    SIT_AIMS_GET_FRAME_TIME                     = 0x0D0C,
    SIT_AIMS_CALL_MODIFY                        = 0x0D0D,
    SIT_IND_AIMS_CALL_MODIFY                    = 0x0D0E,
    SIT_AIMS_RESPONSE_CALL_MODIFY               = 0x0D0F,
    SIT_AIMS_TIME_INFO                          = 0x0D10,
    SIT_IND_AIMS_FRAME_TIME                     = 0x0D11,
    SIT_AIMS_CONF_CALL_ADD_REMOVE_USER          = 0x0D12,
    SIT_AIMS_ENHANCED_CONF_CALL                 = 0x0D13,
    SIT_AIMS_GET_CALL_FORWARD_STATUS            = 0x0D14,
    SIT_AIMS_SET_CALL_FORWARD_STATUS            = 0x0D15,
    SIT_AIMS_GET_CALL_WAITING                   = 0x0D16,
    SIT_AIMS_SET_CALL_WAITING                   = 0x0D17,
    SIT_AIMS_GET_CALL_BARRING                   = 0x0D18,
    SIT_AIMS_SET_CALL_BARRING                   = 0x0D19,
    SIT_IND_AIMS_SUPP_SVC_NOTIFICATION          = 0x0D1A,
    SIT_AIMS_SEND_SMS                           = 0x0D1B,
    SIT_AIMS_SEND_EXPECT_MORE                   = 0x0D1C,
    SIT_AIMS_SEND_SMS_ACK                       = 0x0D1D,
    SIT_IND_AIMS_NEW_SMS                        = 0x0D1E,
    SIT_AIMS_SEND_ACK_INCOMING_SMS              = 0x0D1F,
    SIT_IND_AIMS_NEW_SMS_STATUS_REPORT          = 0x0D20,
    SIT_IND_AIMS_ON_USSD                        = 0x0D21,
    SIT_AIMS_CHG_BARRING_PWD                    = 0x0D22,
    SIT_IND_AIMS_CONFERENCE_CALL_EVENT          = 0x0D23,
    SIT_AIMS_SEND_USSD_INFO                     = 0x0D24,
    SIT_AIMS_GET_PRESENTATION_SETTINGS          = 0x0D25,
    SIT_AIMS_SET_PRESENTATION_SETTINGS          = 0x0D26,
    SIT_AIMS_SET_SELF_CAPABILITY                = 0x0D27,
    SIT_AIMS_HO_TO_WIFI_READY_REQ               = 0x0D28,
    SIT_AIMS_HO_TO_WIFI_CANCEL_IND              = 0x0D29,
    SIT_IND_AIMS_PAYLOAD_INFO_IND               = 0x0D2A,
    SIT_AIMS_HO_TO_3GPP_REQ                     = 0x0D2B,
    SIT_IND_AIMS_VOWIFI_HO_CALL_INFO            = 0x0D2C,
    SIT_IND_AIMS_NEW_CDMA_SMS                   = 0x0D2D,
    SIT_AIMS_SEND_ACK_INCOMING_CDMA_SMS         = 0x0D2E,
    SIT_IND_AIMS_RINGBACK_TONE                  = 0x0D2F,
    SIT_AIMS_MEDIA_STATE_IND                    = 0x0D30,
    SIT_IND_AIMS_CALL_MANAGE                    = 0x0D31,
    SIT_IND_AIMS_CONF_CALL_ADD_REMOVE_USER      = 0x0D32,
    SIT_IND_AIMS_ENHANCED_CONF_CALL             = 0x0D33,
    SIT_IND_AIMS_CALL_MODIFY_RSP                = 0x0D34,
    SIT_IND_AIMS_DTMF_EVENT                     = 0x0D35,
    SIT_AIMS_SET_HIDDEN_MENU_ITEM               = 0x0D36,
    SIT_AIMS_GET_HIDDEN_MENU_ITEM               = 0x0D37,
    SIT_AIMS_DEL_PDN_INFO                       = 0x0D38,
    SIT_AIMS_STACK_START_REQ                    = 0x0D39,
    SIT_AIMS_STACK_STOP_REQ                     = 0x0D3A,
    SIT_AIMS_XCAPM_START_REQ                    = 0x0D3B,
    SIT_AIMS_XCAPM_STOP_REQ                     = 0x0D3C,
    SIT_AIMS_RTT_SEND_TEXT                      = 0x0D3D,
    SIT_IND_AIMS_RTT_NEW_TEXT                   = 0x0D3E,
    SIT_IND_AIMS_RTT_FAIL_SENDING_TEXT          = 0x0D3F,
    SIT_AIMS_RCS_MULTI_FRAME                    = 0x0D40,
    SIT_IND_AIMS_RCS_MULTI_FRAME                = 0x0D41,
    SIT_AIMS_RCS_CHAT                           = 0x0D42,
    SIT_IND_AIMS_RCS_CHAT                       = 0x0D43,
    SIT_AIMS_RCS_GROUP_CHAT                     = 0x0D44,
    SIT_IND_AIMS_RCS_GROUP_CHAT                 = 0x0D45,
    SIT_AIMS_RCS_OFFLINE_MODE                   = 0x0D46,
    SIT_IND_AIMS_RCS_OFFLINE_MODE               = 0x0D47,
    SIT_AIMS_RCS_FILE_TRANSFER                  = 0x0D48,
    SIT_IND_AIMS_RCS_FILE_TRANSFER              = 0x0D49,
    SIT_AIMS_RCS_COMMON_MESSAGE                 = 0x0D5A,
    SIT_IND_AIMS_RCS_COMMON_MESSAGE             = 0x0D4B,
    SIT_AIMS_RCS_CONTENT_SHARE                  = 0x0D4C,
    SIT_IND_AIMS_RCS_CONTENT_SHARE              = 0x0D4D,
    SIT_AIMS_RCS_PRESENCE                       = 0x0D4E,
    SIT_IND_AIMS_RCS_PRESENCE                   = 0x0D4F,
    SIT_AIMS_RCS_XCAP_MANAGE                    = 0x0D50,
    SIT_IND_AIMS_RCS_XCAP_MANAGE                = 0x0D51,
    SIT_AIMS_RCS_CONFIG_MANAGE                  = 0x0D52,
    SIT_IND_AIMS_RCS_CONFIG_MANAGE              = 0x0D53,
    SIT_AIMS_RCS_TLS_MANAGE                     = 0x0D54,
    SIT_IND_AIMS_RCS_TLS_MANAGE                 = 0x0D55,
    SIT_AIMS_EXIT_EMERGENCY_CB_MODE             = 0x0D60,
    SIT_IND_AIMS_EXIT_EMERGENCY_CB_MODE         = 0x0D61,
    SIT_AIMS_SET_GEO_LOCATION_INFO              = 0x0D62,
    SIT_IND_AIMS_DIALOG_INFO                    = 0x0D63,
    SIT_AIMS_CDMA_SEND_SMS                      = 0x0D64,
    SIT_IND_AIMS_SIP_MSG_INFO                   = 0x0D65,
    SIT_IND_AIMS_MEDIA_STATUS                   = 0x0D70,
    SIT_AIMS_SET_PDN_EST_STATUS                 = 0x0D71,
    SIT_AIMS_SET_RTP_RX_STATISTICS              = 0x0D72,
    SIT_AIMS_IND_RTP_RX_STATISTICS              = 0x0D73,
    SIT_IND_AIMS_VOICE_RTP_QUALITY              = 0x0D74,
}SIT_ID_AIMS;
//AIMS support end ---------------------

typedef enum
{
    SIT_VSIM_NOTIFICATION = 0x0E00,
    SIT_IND_VSIM_OPERATION = 0x0E01,
    SIT_VSIM_OPERATION = 0x0E02,
}SIT_ID_VSIM;

typedef enum
{
    SIT_SET_WFC_MEDIA_CONFIGURATION = 0x0F00,
    SIT_IND_WFC_RTP_RTCP_TIMEOUT  = 0x0F01,
    SIT_WFC_DTMF_START   = 0x0F02,
    SIT_IND_WFC_FIRST_RTP   = 0x0F03,
    SIT_IND_WFC_RTCP_RX_SR   = 0x0F04,
    SIT_IND_WFC_RCV_DTMF_NOTI   = 0x0F05,
}SIT_ID_WFC;

typedef enum
{
    SIT_SET_EMBMS_SERVICE = 0x1000,
    SIT_SET_EMBMS_SESSION = 0x1001,
    SIT_IND_EMBMS_COVERAGE = 0x1002,
    SIT_GET_EMBMS_SESSION_LIST = 0x1003,
    SIT_IND_EMBMS_SESSION_LIST = 0x1004,
    SIT_GET_EMBMS_SIGNAL_STRENGTH = 0x1005,
    SIT_IND_EMBMS_SIGNAL_STRENGTH = 0x1006,
    SIT_GET_EMBMS_NETWORK_TIME = 0x1007,
    SIT_IND_EMBMS_NETWORK_TIME = 0x1008,
    SIT_IND_EMBMS_SAI_LIST = 0x1009,
    SIT_IND_EMBMS_GLOBAL_CELL_ID = 0x100A,
}SIT_ID_EMBMS;

typedef enum
{
    SIT_OEM_STORE_ADB_SERIAL_NUMBER_REQ = 0x4011,
    SIT_OEM_READ_ADB_SERIAL_NUMBER_REQ = 0x4012,
}SIT_ID_OEM;

typedef enum
{
    SIT_OEM_GET_SAR_STATE = 0x4100,
    SIT_OEM_SET_SAR_STATE = 0x4101,
    SIT_OEM_IND_RF_CONNECTION = 0x4102,
    SIT_OEM_SET_SVN = 0x4103,
    SIT_OEM_GET_SIM_LOCK_INFO = 0x4104,
    SIT_OEM_SET_CA_BW_FILTER = 0x4105,
    SIT_OEM_IND_CA_BW_FILTER = 0x4106,
}SIT_ID_OEM2;

typedef enum
{
    SIT_OEM_SET_PSEUDO_BS_CON = 0x4200,
    SIT_OEM_GET_PSEUDO_BS_CON = 0x4201,
    SIT_OEM_IND_PSEUDO_BS_INFO = 0x4202,
}SIT_ID_OEM3;

typedef enum
{
    SIT_OEM_GET_MODEM_INFO = 0x4300,
    SIT_OEM_SET_MODEM_INFO = 0x4301,
}SIT_ID_OEM4;

typedef enum
{
    SIT_OEM_NW_INFO = 0x4400,
    SIT_OEM_IND_NW_INFO = 0x4401,
    SIT_OEM_SET_ACTIVATE_VSIM = 0x4402,
    SIT_OEM_SET_FORBID_LTE_CELL = 0x4403,
    SIT_OEM_SET_CP_SLEEP_BLOCK = 0x4404,
    SIT_OEM_SET_FUNC_SWITCH_REQ = 0x4405,
    SIT_OEM_SET_RTP_PKTLOSS_THRESHOLD = 0x4406,
    SIT_OEM_IND_RTP_PKTLOSS_THRESHOLD = 0x4407,
    SIT_OEM_SET_PDCP_DISCARD_TIMER = 0x4408,
    SIT_OEM_GET_CQI_INFO = 0x4409,
    SIT_OEM_SET_SAR_SETTING = 0x440A,
    SIT_OEM_SET_GMO_SWITCH = 0x440B,
    SIT_OEM_SET_TCS_FCI_REQ = 0x440C,
    SIT_OEM_GET_TCS_FCI_INFO = 0x440D,
    SIT_OEM_IND_ENDC_CAPABILITY  = 0x440E,
}SIT_ID_OEM5;

typedef enum
{
    RCM_E_SUCCESS = 0,

    RCM_E_RADIO_NOT_AVAILABLE,
    RCM_E_GENERIC_FAILURE,
    RCM_E_PASSWORD_INCORRECT,
    RCM_E_SIM_PIN2,
    RCM_E_SIM_PUK2,
    RCM_E_REQUEST_NOT_SUPPORTED,
    RCM_E_CANCELLED,
    RCM_E_OP_NOT_ALLOWED_DURING_VOICE_CALL,
    RCM_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW,
    RCM_E_SMS_SEND_FAIL_RETRY = 10,

    RCM_E_SIM_ABSENT,
    RCM_E_SUBSCRIPTION_NOT_AVAILABLE,
    RCM_E_MODE_NOT_SUPPORTED,
    RCM_E_FDN_CHECK_FAILURE,
    RCM_E_ILLEGAL_SIM_OR_ME,
    RCM_E_MISSING_RESOURCE,
    RCM_E_NO_SUCH_ELEMENT,
    RCM_E_SIM_MEMORY_FULL,
    RCM_E_PB_TOO_LONG_DATA,

    RCM_E_SIM_PIN_REQ = 20,
    RCM_E_SIM_PUK_REQ,
    RCM_E_UNDEFINED_CMD,
    RCM_E_OP_NOT_ALLOWED_DURING_PLMN_SEARCH,

    RCM_E_MAX
}SIT_ERROR_ID;

enum {
    RCM_TYPE_REQUEST = 0x00,
    RCM_TYPE_RESPONSE = 0x01,
    RCM_TYPE_INDICATION = 0x02,
};

enum {
    SIT_RESULT_FAIL = 0x00,
    SIT_RESULT_SUCCESS = 0x01,
};

#define SIT_MAX_UUID_LENGTH (64)
#define SIT_CELL_INFO_V12 (12)
#define SIT_CELL_INFO_V14 (14)

// struct
#pragma pack(1)

typedef struct {
    UINT    token;
    BYTE    reserved[2];
} RCM_REQ_EXT;

typedef struct {
    UINT    token;
    BYTE    error;
    BYTE    reserved[1];
} RCM_RSP_EXT;

typedef struct {
    BYTE    reserved[2];
} RCM_IND_EXT;

typedef struct {
    BYTE    type;
    BYTE    reserved;
    UINT16    id;
    UINT16    length;
} RCM_GEN_HEADER;

typedef struct {
    BYTE    type;
    BYTE    reserved;
    UINT16    id;
    UINT16    length;
    union {
        RCM_REQ_EXT req;
        RCM_RSP_EXT rsp;
    } ext;
} RCM_HEADER;

typedef struct {
    BYTE    type;
    BYTE    reserved;
    UINT16    id;
    UINT16    length;
    RCM_IND_EXT ext;
} RCM_IND_HEADER;

typedef struct
{
    RCM_HEADER hdr;
}null_data_format;

typedef struct
{
    RCM_IND_HEADER hdr;
}null_ind_data_format;

/*
    SIT_GET_CURRENT_CALLS (RCM ID = 0x0000)
*/
typedef struct
{
    RCM_HEADER hdr;
}sit_call_get_current_calls_req ;

typedef struct
{
    BYTE state;
    INT32 index;
    INT32 type_of_address;
    BYTE is_mpty;
    BYTE is_mt;
    BYTE als;
    BYTE call_type;
    BYTE is_voice_privacy;
    BYTE num_len;
    BYTE num[ MAX_DIAL_NUM ];
    INT32 number_presentation;
    BYTE name_len;
    BYTE name[ MAX_DIAL_NAME ];
    INT32 name_presentation;
    BYTE name_dcs;
    INT32 uus_type;
    INT32 uus_dcs;
    BYTE uus_data_len;
    BYTE uus_data[ MAX_UUS_DATA_LEN ];
//#ifdef RIL_FEATURE_FUNCITON_SRVCC_SUPPORT_CALL_LIST   // make this parameter as commonly used regardless of target product
    BYTE SRVCCCall;
//#endif
}sit_call_info_type;

typedef struct
{
    RCM_HEADER hdr;
    INT32 number;
    sit_call_info_type record[MAX_CALL_LIST_NUM];
}sit_call_get_current_calls_rsp;

typedef struct
{
    INT32 number;
    sit_call_info_type record[MAX_CALL_LIST_NUM];
}sit_call_set_srvcc_call_list;
typedef struct
{
    RCM_HEADER hdr;
    sit_call_set_srvcc_call_list call_list;
}sit_call_set_srvcc_call_list_req;

typedef enum
{
    SIT_CALL_STATE_ACTIVE = 0x00,
    SIT_CALL_STATE_HOLDING,
    SIT_CALL_STATE_DIALING,
    SIT_CALL_STATE_ALERTING,
    SIT_CALL_STATE_INCOMING,
    SIT_CALL_STATE_WAITING,

    SIT_CALL_STATE_MAX
} sit_call_state_e_type;

typedef enum
{
    SIT_CALL_IS_MTPY_SINGLE = 0x00,
    SIT_CALL_IS_MTPY_MULTIPARTY,

    SIT_CALL_IS_MTPY_MAX
}sit_call_is_mtpy_e_type;

typedef enum
{
    SIT_CALL_IS_MT_MO = 0x00,
    SIT_CALL_IS_MT_MT,

    SIT_CALL_IS_MT_MAX
}sit_call_is_mt_e_type;

/*
typedef enum
{
    SIT_CALL_ALS_LINE1 = 0x00,

    SIT_CALL_ALS_MAX
}sit_call_als_e_type;
*/

typedef enum
{
    SIT_CALL_CALL_TYPE_NULL = 0x00,
    SIT_CALL_CALL_TYPE_VOICE,
    SIT_CALL_CALL_TYPE_DATA,
    SIT_CALL_CALL_TYPE_VIDEO,
    SIT_CALL_CALL_TYPE_EMERGENCY,
    SIT_CALL_CALL_TYPE_VOLTE,

#ifdef SUPPORT_CDMA
    SIT_CALL_CALL_TYPE_CDMA_VOICE = 0x0A,
    SIT_CALL_CALL_TYPE_CDMA_OTASP,
    SIT_CALL_CALL_TYPE_CDMA_EMERGENCY,
    SIT_CALL_CALL_TYPE_CDMA_MARKOV8K,
    SIT_CALL_CALL_TYPE_CDMA_MARKOV13K,
    SIT_CALL_CALL_TYPE_CDMA_LOOPBACK8K,
    SIT_CALL_CALL_TYPE_CDMA_LOOPBACK13K,
#endif

    SIT_CALL_CALL_TYPE_MAX
}sit_call_call_type_e_type;

typedef enum
{
    SIT_CALL_IS_VOICE_PRIVACY_INACTIVATED = 0x00,
    SIT_CALL_IS_VOICE_PRIVACY_ACTIVATED,

    SIT_CALL_IS_VOICE_PRIVACY_MAX
}sit_call_is_voice_privacy_e_type;

typedef enum
{
    SIT_CALL_PRESENTATION_ALLOWED = 0x00,
    SIT_CALL_PRESENTATION_RESTRICTED,
    SIT_CALL_PRESENTATION_NOT_SPECIFIED,
    SIT_CALL_PRESENTATION_PAYPHONE,

    SIT_CALL_PRESENTATION_MAX
}sit_call_presentation_e_type;

typedef enum
{
    SIT_CALL_UUS_TYPE_TYPE1_IMPLICIT = 0x00,
    SIT_CALL_UUS_TYPE_TYPE1_REQUIRED,
    SIT_CALL_UUS_TYPE_TYPE1_NOT_REQUIRED,
    SIT_CALL_UUS_TYPE_TYPE2_REQUIRED,
    SIT_CALL_UUS_TYPE_TYPE2_NOT_REQUIRED,
    SIT_CALL_UUS_TYPE_TYPE3_REQUIRED,
    SIT_CALL_UUS_TYPE_TYPE3_NOT_REQUIRED,

    SIT_CALL_UUS_TYPE_MAX
}sit_call_uus_type_e_type;

typedef enum
{
    SIT_CALL_UUS_DCS_USP = 0x00,
    SIT_CALL_UUS_DCS_OSIHLP,
    SIT_CALL_UUS_DCS_X244RMCF,
    SIT_CALL_UUS_DCS_RMCF,
    SIT_CALL_UUS_DCS_IA5C,

    SIT_CALL_UUS_DCS_MAX
}sit_call_uus_dcs_e_type;

/*
    SIT_DIAL (RCM ID = 0x0001)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE call_type;
    BYTE emergency_call_subtype;
    BYTE num_len;
    BYTE num[ MAX_DIAL_NUM ];
    BYTE num_type;
    BYTE num_plan;
    BYTE clir;
    BYTE cug_call;
    BYTE cug_info_index;
    BYTE cug_info;
}sit_call_dial_req;

typedef enum
{
    SIT_CALL_EMERGENCY_CALL_SUBTYPE_DEFAULT_EMERGENCY_CENTER = 0x00,
    SIT_CALL_EMERGENCY_CALL_SUBTYPE_POLICE = 0x01,
    SIT_CALL_EMERGENCY_CALL_SUBTYPE_AMBULANCE = 0x02,
    SIT_CALL_EMERGENCY_CALL_SUBTYPE_FIRE_BRIGADE = 0x04,
    SIT_CALL_EMERGENCY_CALL_SUBTYPE_MARINE_GUARD = 0x08,
    SIT_CALL_EMERGENCY_CALL_SUBTYPE_MOUNTAIN_RESCUE = 0x10,
    SIT_CALL_EMERGENCY_CALL_SUBTYPE_NONE = 0xff,

    SIT_CALL_EMERGENCY_CALL_SUBTYPE_MAX
}sit_call_emergency_call_subtype_e_type;


typedef enum
{
    SIT_CALL_NUMBER_TYPE_UNKNOWN = 0x00,
    SIT_CALL_NUMBER_TYPE_INTERNATIONAL = 0x10,
    SIT_CALL_NUMBER_TYPE_NATIONAL = 0x20,
    SIT_CALL_NUMBER_TYPE_NETWORK_SPECIFIC = 0x30,
    SIT_CALL_NUMBER_TYPE_DEDICATED_ACCESS_SHORT_CODE = 0x40,

    SIT_CALL_NUMBER_TYPE_MAX
}sit_call_number_type_e_type;

typedef enum
{
    SIT_CALL_NUMBER_PLAN_UNKNOWN = 0x00,
    SIT_CALL_NUMBER_PLAN_ISDN = 0x01,
    SIT_CALL_NUMBER_PLAN_DATA = 0x03,
    SIT_CALL_NUMBER_PLAN_TELEX = 0x04,
    SIT_CALL_NUMBER_PLAN_NATIONAL = 0x08,
    SIT_CALL_NUMBER_PLAN_PRIVATE = 0x09,

    SIT_CALL_NUMBER_PLAN_MAX
}sit_call_number_plan_e_type;

typedef enum
{
    SIT_CALL_CLIR_DEFAULT = 0x00,
    SIT_CALL_CLIR_INVOCATION,
    SIT_CALL_CLIR_SUPPRESSION,

    SIT_CALL_CLIR_MAX
}sit_call_clir_e_type;

typedef enum
{
    SIT_CALL_CUG_CALL_DISABLED = 0X00,
    SIT_CALL_CUG_CALL_ENABLED,

    SIT_CALL_CUG_CALL_MAX
}sit_call_cug_call_e_type;

typedef enum
{
    SIT_CALL_CUG_INFO_NONE = 0x00,
    SIT_CALL_CUG_INFO_SUPPRESS_OA,
    SIT_CALL_CUG_INFO_SUPPRESS_PREFERENTIAL,
    SIT_CALL_CUG_INFO_SUPPRESS_OA_AND_PREFERENTIAL,

    SIT_CALL_CUG_INFO_MAX
}sit_call_cug_info_e_type;

typedef null_data_format sit_call_dial_rsp;

/*
    SIT_GET_LAST_CALL_FAIL_CAUSE (RCM ID = 0x0002)
*/
typedef null_data_format sit_call_get_last_call_fail_cause_req ;

typedef struct
{
    RCM_HEADER hdr;
    INT32 last_call_fail_cause;
}sit_call_get_last_call_fail_cause_rsp;

typedef enum
{
    SIT_CALL_LAST_CALL_FAIL_UNOBTAINABLE_NUMBER = 0x0001,
    SIT_CALL_LAST_CALL_FAIL_NORMAL = 0x0010,
    SIT_CALL_LAST_CALL_FAIL_BUSY=  0x0011,
    SIT_CALL_LAST_CALL_FAIL_NORMAL_UNSPECIFIED = 0x001F,
    SIT_CALL_LAST_CALL_FAIL_CONGESTION = 0x0022,
    SIT_CALL_LAST_CALL_FAIL_ACM_LIMIT_EXCEEDED = 0x0044,
    SIT_CALL_LAST_CALL_FAIL_CALL_BARRED = 0x00F0,
    SIT_CALL_LAST_CALL_FAIL_FDN_BLOCKED = 0x00F1,
    SIT_CALL_LAST_CALL_FAIL_IMSI_UNKNOWN_IN_VLR = 0x00F2,
    SIT_CALL_LAST_CALL_FAIL_IMEI_NOT_ACCEPTED = 0x00F3,
    SIT_CALL_LAST_CALL_FAIL_CDMA_LOCKED_UNTIL_POWER_CYCLE = 0x03E8,
    SIT_CALL_LAST_CALL_FAIL_CDMA_DROP = 0x03E9,
    SIT_CALL_LAST_CALL_FAIL_CDMA_INTERCEPT = 0x03EA,
    SIT_CALL_LAST_CALL_FAIL_CDMA_REORDER = 0x03EB,
    SIT_CALL_LAST_CALL_FAIL_CDMA_SO_REJECT = 0x03EC,
    SIT_CALL_LAST_CALL_FAIL_CDMA_RETRY_ORDER = 0x03ED,
    SIT_CALL_LAST_CALL_FAIL_CDMA_ACCESS_FAILURE = 0x03EE,
    SIT_CALL_LAST_CALL_FAIL_CDMA_PREEMPTED = 0x03EF,
    SIT_CALL_LAST_CALL_FAIL_CDMA_NOT_EMERGENCY = 0x03F0,
    SIT_CALL_LAST_CALL_FAIL_CDMA_ACCESS_BLOCKED = 0x03F1,
    SIT_CALL_LAST_CALL_FAIL_ERROR_UNSPECIFIED = 0xFFFF,

    SIT_CALL_LAST_CALL_FAIL_MAX
}sit_call_last_call_fail_cause_e_type;

/*
    SIT_DTMF (RCM ID =0x0003)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE dtmf_len;
    BYTE dtmf_digit[MAX_DTMF_LEN];
}sit_call_dtmf_req ;

typedef null_data_format sit_call_dtmf_rsp;

/*
    SIT_ANSWER (RCM ID =0x0004)
*/
typedef null_data_format sit_call_answer_req ;
typedef null_data_format sit_call_answer_rsp ;


/*
    SIT_DTMF_START (RCM ID =0x0005)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE tone_type;
    BYTE tone_len;
    BYTE dtmf_digit;
}sit_call_dtmf_start_req ;

typedef enum
{
    SIT_CALL_LOCAL_DTMF_OFF = 0x00,
    SIT_CALL_LOCAL_DTMF_ON,

    SIT_CALL_LOCAL_DTMF_MAX
}sit_call_dtmf_local_tone_e_type;

typedef enum
{
    SIT_CALL_DTMF_TONE_LENGTH_SHORT = 0x01,
    SIT_CALL_DTMF_TONE_LENGTH_LONG,

    SIT_CALL_DTMF_TONE_LENGTH_MAX
}sit_call_dtmf_tone_length_e_type;

/*
    SIT_DTMF_STOP (RCM ID =0x0006)

*/
typedef null_data_format sit_call_dtmf_stop_req ;
typedef null_data_format sit_call_dtmf_stop_rsp ;

/*
    SIT_SEND_EXPLICIT_CALL_TRANSFER (RCM ID =0x0007)
*/
typedef null_data_format sit_call_explicit_call_transfer_req ;
typedef null_data_format sit_call_explicit_call_transfer_rsp ;


/*
    SIT_HANGUP (RCM ID =0x0008)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 index;
    INT32 call_type;
}sit_call_hangup_req ;

typedef enum
{
    SIT_CALL_HANGUP_CALL_TYPE_SINGLE = 0x01,
    SIT_CALL_HANGUP_CALL_TYPE_MULTI,

    SIT_CALL_HANGUP_CALL_TYPE_MAX
}sit_call_hangup_call_type_e_type;

typedef null_data_format sit_call_hangup_rsp;

/*
    SIT_IND_CALL_STATE_CHANGED (RCM ID =0x0009)
*/
typedef null_ind_data_format sit_call_call_state_changed_ind;


/*
    SIT_IND_CALL_RING (RCM ID =0x000A)
*/
typedef null_ind_data_format sit_call_call_ring_ind;

/*
    SIT_IND_EMERGENCY_CALL_LIST (RCM ID =0x000D)
*/

typedef struct
{
    BYTE category;
    BYTE emc_number_len;
    BYTE emc_number[MAX_EMERGENCY_NUMBER_LEN]; //32
    BYTE source;
}sit_call_emergency_call_number_info;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE mcc[MAX_MCC_LEN];
    BYTE mnc[MAX_MNC_LEN];
    BYTE num;
    sit_call_emergency_call_number_info number_info[MAX_EMERGENCY_CALL_NUM]; //20
}sit_call_emergency_call_list_ind;

typedef enum {
    SIT_NET_EMERGENCY_CALL_CATEGORY_POLICE       = 1, //0b00000001
    SIT_NET_EMERGENCY_CALL_CATEGORY_AMBULANCE    = 2,
    SIT_NET_EMERGENCY_CALL_CATEGORY_FIRE         = 4,
    SIT_NET_EMERGENCY_CALL_CATEGORY_MARINE       = 8,
    SIT_NET_EMERGENCY_CALL_CATEGORY_MOUNTAIN     = 16,
    SIT_NET_EMERGENCY_CALL_CATEGORY_MANUAL       = 32,
    SIT_NET_EMERGENCY_CALL_CATEGORY_AUTO         = 64,
    SIT_NET_EMERGENCY_CALL_CATEGORY_RESERVED     = 128,
}sit_call_emergency_call_category_e_type;

typedef enum {
    SIT_EMERGENCY_CONDITION_ALWAYS                    = 1 << 0,
    SIT_EMERGENCY_CONDITION_NO_SIM                    = 1 << 4,
    SIT_EMERGENCY_CONDITION_TESTCARD                  = 1 << 8,
    SIT_EMERGENCY_CONDITION_NOT_IMS_REGI              = 1 << 12,
    SIT_EMERGENCY_CONDITION_UI_ONLY                   = 1 << 16,
}sit_emergency_condition;

/*
    SIT_IND_ENTER_EMERGENCY_CB_MODE (RCM ID = 0x0014)
*/
typedef null_ind_data_format sit_call_enter_emergency_cb_ind;

/*
    SIT_IND_EXIT_EMERGENCY_CB_MODE (RCM ID = 0x0019)
*/
typedef null_ind_data_format sit_call_exit_emergency_cb_ind;

/*
    SIT_EXIT_EMERGENCY_CB_MODE (RCM ID = 0x001A)
*/
typedef null_data_format sit_call_exit_emergency_cb_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE result; // 0x00: Fail, 0x01: Success
}sit_call_exit_emergency_cb_mode_rsp;

/*
    SIT_SEND_SMS (RCM ID = 0x0100)
*/
typedef struct
{
    RCM_HEADER hdr;
#ifndef NO_USE_SMS_DOMAIN
    BYTE sms_domain;
#endif // NO_USE_SMS_DOMAIN
    BYTE smsc_len;
    BYTE smsc[MAX_GSM_SMS_SERVICE_CENTER_ADDR];
    BYTE sms_len;
    BYTE sms_data[MAX_GSM_SMS_TPDU_SIZE];
}sit_sms_send_sms_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 msg_ref;
    BYTE ack_pdu_len;
    BYTE ack_pdu[MAX_GSM_SMS_TPDU_SIZE];
    INT32 error_code;
}sit_sms_send_sms_rsp;

/*
    SIT_SEND_EXPECT_MORE (RCM ID = 0x0101)
*/
//same as SIT_SEND_SMS
typedef sit_sms_send_sms_req sit_sms_send_expect_more_req;

typedef sit_sms_send_sms_rsp sit_sms_send_expect_more_rsp;

/*
    SIT_SEND_SMS_ACK (RCM ID = 0x0102)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 result;
    BYTE msg_tpid;
    INT32 error_code;
}sit_sms_send_sms_ack_req;

typedef null_ind_data_format sit_sms_send_sms_ack_rsp;

/*
    SIT_WRITE_SMS_TO_SIM (RCM ID = 0x0103)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 status;
    INT32 index;
    BYTE pdu_len;
    BYTE pdu_data[MAX_GSM_SMS_TPDU_SIZE];
}sit_sms_write_sms_to_sim_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 index;
}sit_sms_write_sms_to_sim_rsp;

typedef enum
{
    SIT_SIM_STATUS_RECEIVED_UNREAD = 0x00,
    SIT_SIM_STATUS_RECEIVED_READ,
    SIT_SIM_STATUS_STORED_UNSENT,
    SIT_SIM_STATUS_STORED_SENT,

    SIT_SIM_STATUS_MAX
}sit_sms_status_e_type;

/*
    SIT_DELETE_SMS_ON_SIM (RCM ID = 0x0104)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 index;
}sit_sms_delete_sms_on_sim_req;

typedef null_data_format sit_sms_delete_sms_on_sim_rsp;

/*
    SIT_GET_BCST_SMS_CFG (RCM ID = 0x0105)
*/
typedef null_data_format sit_sms_get_bcst_sms_cfg_req;

typedef struct
{
    INT16 from_svc_id;
    INT16 to_svc_id;
    BYTE from_code_scheme;
    BYTE to_code_scheme;
    BYTE selected;
}sit_sms_bcst_sms_cfg_item;

typedef struct
{
    RCM_HEADER hdr;
    BYTE bcst_info_num;
    sit_sms_bcst_sms_cfg_item cfgitem[MAX_BCST_INFO_NUM];
}sit_sms_get_bcst_sms_cfg_rsp;

/*
    SIT_SET_BCST_SMS_CFG (RCM ID = 0x0106)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE bcst_info_num;
    sit_sms_bcst_sms_cfg_item cfgitem[MAX_BCST_INFO_NUM];
}sit_sms_set_bcst_sms_cfg_req;

typedef null_data_format sit_sms_set_bcst_sms_cfg_rsp;

/*
    SIT_ACT_BCST_SMS (RCM ID = 0x0107)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 bcst_act;
}sit_sms_act_bcst_sms_req;

typedef null_data_format sit_sms_act_bcst_sms_rsp;

typedef enum
{
    SIT_SMS_BCST_ACT_ACTIVATE = 0x00,
    SIT_SMS_BCST_ACT_DEACTIVATE,

    SIT_SMS_BCST_ACT_MAX
}sit_sms_bcst_act_e_type;

/*
    SIT_GET_SMSC_ADDR (RCM ID = 0x0108)
*/
typedef null_data_format sit_sms_get_smsc_addr_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE sca_len;
    BYTE sca[MAX_GSM_SMS_SERVICE_CENTER_ADDR];
}sit_sms_get_smsc_addr_rsp;

/*
    SIT_SET_SMSC_ADDR (RCM ID = 0x0109)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE sca_len;
    BYTE sca[MAX_GSM_SMS_SERVICE_CENTER_ADDR];
}sit_sms_set_smsc_addr_req;

typedef null_data_format sit_sms_set_smsc_addr_rsp;


/*
    SIT_SEND_SMS_MEM_STATUS (RCM ID = 0x010A)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 mem_status;
}sit_sms_send_sms_mem_status_req;

enum {
    MEMORY_CAPACITY_EXCEEDED = 0x00,
    MEMORY_AVAILABLE = 0x01,
};

typedef null_data_format sit_sms_send_sms_mem_status_rsp;

/*
    SIT_SEND_ACK_INCOMING_SMS (RCM ID = 0x010B)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 result;
    BYTE msg_tpid;
    BYTE tpdu_len;
    BYTE tpdu[MAX_GSM_SMS_TPDU_SIZE];
}sit_sms_send_ack_incoming_sms_req;

typedef null_data_format sit_sms_send_ack_incoming_sms_rsp;

/*
    SIT_IND_NEW_SMS (RCM ID = 0x010C)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE msg_tpid;
    BYTE tpdu_len;
    BYTE tpdu[MAX_GSM_SMS_TPDU_SIZE*2];
}sit_sms_new_sms_ind;

/*
    SIT_IND_NEW_SMS_STATUS_REPORT (RCM ID = 0x010D), This seems to be able to be replaced with the sit_sms_new_sms_ind above later.
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE msg_tpid;
    BYTE tpdu_len;
    BYTE tpdu[MAX_GSM_SMS_TPDU_SIZE*2];
}sit_sms_new_sms_status_report_ind;


/*
    SIT_IND_NEW_SMS_ON_SIM (RCM ID = 0x010E)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    INT32 index;
}sit_sms_new_sms_on_sim_ind;

/*
SIT_IND_SIM_SMS_STORAGE_FULL (RCM ID = 0x010F)
*/
typedef null_ind_data_format sit_sms_sim_sms_storage_full_ind;

/*
    SIT_IND_NEW_BCST_SMS (RCM ID = 0x0110)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE bcst_msg_len;
    BYTE bcst_msg[MAX_BCST_MSG_LEN];
}sit_sms_new_bcst_sms_ind;

/*
    SIT_GET_STORED_SMS_COUNT (RCM ID = 0x0121)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE sim_id;
}sit_sms_get_stored_sms_count_req;

enum {
    STORED_SIM = 0x00,
    STORED_RUIM = 0x01,
};

typedef struct
{
    RCM_HEADER hdr;
    BYTE sim_id;
    INT16 total_num;
    INT16 used_num;
}sit_sms_get_stored_sms_count_rsp;

/*
    SIT_GET_SIM_STATUS (RCM ID = 0x0200)
*/
typedef null_data_format sit_sim_get_sim_status_req;

typedef struct
{
    BYTE apps_type;
    BYTE apps_state;
    BYTE perso_substate;
    BYTE aid_len;
    BYTE AID[MAX_SIM_AID_LEN];
    BYTE app_label_len;
    BYTE app_label[MAX_SIM_APP_LABEL_LEN];
    INT32 pin1_replaced;
    BYTE pin1_state;
    BYTE pin2_state;
    BYTE pin1_remain_count;
    BYTE puk1_remain_count;
    BYTE pin2_remain_count;
    BYTE puk2_remain_count;
}sit_sim_apps_status_info;

typedef struct
{
    RCM_HEADER hdr;
    BYTE card_state;
    BYTE universal_pin_state;
    BYTE application_num;
    sit_sim_apps_status_info apps_status_info[MAX_SIM_APPS_INFO_COUNT];
}sit_sim_get_sim_status_rsp;

typedef struct
{
    BYTE esim_no_profile;
    BYTE physical_slot_id;
    BYTE atr_length;
    BYTE atr[MAX_ATR_LEN_GET_SIM];
    BYTE iccid_length;
    BYTE iccid[MAX_ICCID_LEN];
    BYTE eid_length;
    BYTE eid[MAX_EID_LEN];
}sit_sim_get_sim_status_rsp_ext;

typedef enum
{
    SIT_SIM_CARD_STATE_ABSENT = 0x00,
    SIT_SIM_CARD_STATE_PRESENT,
    SIT_SIM_CARD_STATE_ERROR,

    SIT_SIM_CARD_STATE_MAX
}sit_sim_card_state_e_type;

typedef enum
{
    SIT_SIM_PIN_STATE_UNKNOWN = 0x00,
    SIT_SIM_PIN_STATE_ENABLED_NOT_VERIFIED,
    SIT_SIM_PIN_STATE_ENABLED_VERIFIED,
    SIT_SIM_PIN_STATE_DISABLED,
    SIT_SIM_PIN_STATE_ENABLED_BLOCKED,
    SIT_SIM_PIN_STATE_ENABLED_PERM_BLOCKED,

    SIT_SIM_PIN_STATE_MAX
}sit_sim_pin_state_e_type;

typedef enum
{
    SIT_SIM_APPS_TYPE_UNKNOWN = 0x00,
    SIT_SIM_APPS_TYPE_SIM,
    SIT_SIM_APPS_TYPE_USIM,
    SIT_SIM_APPS_TYPE_RUIM,
    SIT_SIM_APPS_TYPE_CSIM,
    SIT_SIM_APPS_TYPE_ISIM,

    SIT_SIM_APPS_TYPE_MAX
}sit_sim_apps_type_e_type;

typedef enum
{
    SIT_SIM_APPS_STATE_UNKNOWN = 0x00,
    SIT_SIM_APPS_STATE_DETECTED,
    SIT_SIM_APPS_STATE_PIN_REQUIRED,
    SIT_SIM_APPS_STATE_PUK_REQUIRED,
    SIT_SIM_APPS_STATE_SUBSCRIPTION_PERSO,
    SIT_SIM_APPS_STATE_READY,

    SIT_SIM_APPS_STATE_MAX
}sit_sim_apps_state_e_type;

typedef enum
{
    SIT_SIM_PERSO_SUBSTATE_UNKNOWN = 0x00,
    SIT_SIM_PERSO_SUBSTATE_IN_PROGRESS,
    SIT_SIM_PERSO_SUBSTATE_READY,
    SIT_SIM_PERSO_SUBSTATE_SIM_NETWORK,
    SIT_SIM_PERSO_SUBSTATE_SIM_NETWORK_SUBSET,
    SIT_SIM_PERSO_SUBSTATE_SIM_CORPORATE,
    SIT_SIM_PERSO_SUBSTATE_SIM_SERVICE_PROVIDER,
    SIT_SIM_PERSO_SUBSTATE_SIM_SIM,
    SIT_SIM_PERSO_SUBSTATE_SIM_NETWORK_PUK,
    SIT_SIM_PERSO_SUBSTATE_SIM_NETWORK_SUBSET_PUK,
    SIT_SIM_PERSO_SUBSTATE_SIM_CORPORATE_PUK,
    SIT_SIM_PERSO_SUBSTATE_SIM_SERVICE_PROVIDER_PUK,
    SIT_SIM_PERSO_SUBSTATE_SIM_PUK,
    SIT_SIM_PERSO_SUBSTATE_RUIM_NETWORK1,
    SIT_SIM_PERSO_SUBSTATE_RUIM_NETWORK2,
    SIT_SIM_PERSO_SUBSTATE_RUIM_HRPD,
    SIT_SIM_PERSO_SUBSTATE_RUIM_CORPORATE,
    SIT_SIM_PERSO_SUBSTATE_RUIM_SERVICE_PROVIDER,
    SIT_SIM_PERSO_SUBSTATE_RUIM_RUIM,
    SIT_SIM_PERSO_SUBSTATE_RUIM_NETWORK1_PUK,
    SIT_SIM_PERSO_SUBSTATE_RUIM_NETWORK2_PUK,
    SIT_SIM_PERSO_SUBSTATE_RUIM_HRPD_PUK,
    SIT_SIM_PERSO_SUBSTATE_RUIM_CORPORATE_PUK,
    SIT_SIM_PERSO_SUBSTATE_RUIM_SERVICE_PROVIDER_PUK,
    SIT_SIM_PERSO_SUBSTATE_RUIM_RUIM_PUK,

    SIT_SIM_PERSO_SUBSTATE_MAX
}sit_sim_perso_substate_e_type;

/*
    SIT_VERIFY_SIM_PIN (RCM ID = 0x0201)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE pin_len;
    BYTE pin[MAX_SIM_PIN_LEN];
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_sim_verify_sim_pin_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 remain_count;
}sit_sim_verify_sim_pin_rsp;


/*
    SIT_VERIFY_SIM_PUK (RCM ID = 0x0202)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE puk_len;
    BYTE puk[MAX_SIM_PUK_LEN];
    BYTE new_pin_len;
    BYTE new_pin[MAX_SIM_PIN_LEN];
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_sim_verify_sim_puk_req;

/*
typedef struct
{
    RCM_HEADER hdr;
    INT32 remain_count;
}sit_sim_verify_sim_puk_rsp;
*/

// same as SIT_VERIFY_SIM_PIN
typedef sit_sim_verify_sim_pin_rsp sit_sim_verify_sim_puk_rsp;

/*
    SIT_VERIFY_SIM_PIN2 (RCM ID = 0x0203)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE pin2_len;
    BYTE pin2[MAX_SIM_PIN_LEN];
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_sim_verify_sim_pin2_req;

/*
typedef struct
{
    RCM_HEADER hdr;
    INT32 remain_count;
}sit_sim_verify_sim_pin2_rsp;
*/
// same as SIT_VERIFY_SIM_PIN
typedef sit_sim_verify_sim_pin_rsp sit_sim_verify_sim_pin2_rsp;

/*
    SIT_VERIFY_SIM_PUK2 (RCM ID = 0x0204)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE puk2_len;
    BYTE puk2[MAX_SIM_PUK_LEN];
    BYTE new_pin2_len;
    BYTE new_pin2[MAX_SIM_PIN_LEN];
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_sim_verify_sim_puk2_req;

/*
typedef struct
{
    RCM_HEADER hdr;
    INT32 remain_count;
}sit_sim_verify_sim_puk2_rsp;
*/
// same as SIT_VERIFY_SIM_PIN
typedef sit_sim_verify_sim_pin_rsp sit_sim_verify_sim_puk2_rsp;

/*
    SIT_CHG_SIM_PIN (RCM ID = 0x0205)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE fac;
    BYTE old_pin_len;
    BYTE old_pin[MAX_SIM_PIN_LEN];
    BYTE new_pin_len;
    BYTE new_pin[MAX_SIM_PIN_LEN];
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_sim_change_sim_pin_req;

/*
typedef struct
{
    RCM_HEADER hdr;
    INT32 remain_count;
}sit_sim_change_sim_pin_rsp;
*/
// same as SIT_VERIFY_SIM_PIN
typedef sit_sim_verify_sim_pin_rsp sit_sim_change_sim_pin_rsp;

/*
    SIT_CHG_SIM_PIN2 (RCM ID = 0x0206)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE fac;
    BYTE old_pin2_len;
    BYTE old_pin2[MAX_SIM_PIN_LEN];
    BYTE new_pin2_len;
    BYTE new_pin2[MAX_SIM_PIN_LEN];
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_sim_change_sim_pin2_req;

/*
typedef struct
{
    RCM_HEADER hdr;
    INT32 remain_count;
}sit_sim_change_sim_pin2_rsp;
*/
// same as SIT_VERIFY_SIM_PIN
typedef sit_sim_verify_sim_pin_rsp sit_sim_change_sim_pin2_rsp;

typedef enum
{
    SIT_SIM_FAC_CS = 0,
    SIT_SIM_FAC_PS,
    SIT_SIM_FAC_PF,
    SIT_SIM_FAC_SC,
    SIT_SIM_FAC_AO,
    SIT_SIM_FAC_OI,
    SIT_SIM_FAC_OX,
    SIT_SIM_FAC_AI,
    SIT_SIM_FAC_IR,
    SIT_SIM_FAC_NT,
    SIT_SIM_FAC_NM, /*10*/
    SIT_SIM_FAC_NS,
    SIT_SIM_FAC_NA,
    SIT_SIM_FAC_AB,
    SIT_SIM_FAC_AG,
    SIT_SIM_FAC_AC,
    SIT_SIM_FAC_FD,
    SIT_SIM_FAC_PN,
    SIT_SIM_FAC_PU,
    SIT_SIM_FAC_PP,
    SIT_SIM_FAC_PC, /*20*/
    SIT_SIM_FAC_SC2,

    SIT_SIM_FAC_MAX
}sit_sim_fac_lock_type_e_type;

/*
    SIT_VERIFY_NETWORK_LOCK  (RCM ID = 0x0207)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE fac;
    BYTE password_len;
    BYTE password[MAX_SIM_FACILITY_PASSWORD_LEN];
    BYTE service_class;
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_sim_verify_network_lock_req;

/*
typedef struct
{
    RCM_HEADER hdr;
    INT32 remain_count;
}sit_sim_verify_network_lock_rsp;
*/
// same as SIT_VERIFY_SIM_PIN
typedef sit_sim_verify_sim_pin_rsp sit_sim_verify_network_lock_rsp;

typedef enum
{
    SIT_SIM_FAC_LOCK_MODE_UNLOCK = 0x00,
    SIT_SIM_FAC_LOCK_MODE_LOCK,

    SIT_SIM_FAC_LOCK_MODE_MAX
}sit_sim_facility_lock_mode_e_type;

/*
    SIT_SIM_IO (RCM ID = 0x0208)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE command;
    BYTE app_type;
    INT16 file_id;
    BYTE path_len;
    BYTE path[MAX_SIM_IO_PATH_LEN];
    BYTE p1;
    BYTE p2;
    BYTE p3;
    WORD data_len;
    BYTE data[MAX_SIM_IO_DATA_LEN];
    BYTE pin2_len;
    BYTE pin2[MAX_SIM_PIN_LEN];
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_sim_sim_io_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE sw1;
    BYTE sw2;
    INT16 response_len;
    BYTE response[MAX_SIM_IO_DATA_LEN];
}sit_sim_sim_io_rsp;

typedef enum
{
    SIT_SIM_SIM_IO_COMMAND_READ_BINARY = 0xB0,
    SIT_SIM_SIM_IO_COMMAND_READ_RECORD = 0xB2,
    SIT_SIM_SIM_IO_COMMAND_GET_RESPONSE = 0xC0,
    SIT_SIM_SIM_IO_COMMAND_UPDATE_BINARY = 0xD6,
    SIT_SIM_SIM_IO_COMMAND_UPDATE_RECORD = 0xDC,
    SIT_SIM_SIM_IO_COMMAND_STATUS = 0xF2,

    SIT_SIM_SIM_IO_COMMAND_MAX
}sit_sim_sim_io_command_e_type;


/*
    SIT_GET_FACILITY_LOCK (RCM ID = 0x0209)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE code;
    BYTE password_len;
    BYTE password[MAX_SIM_FACILITY_PASSWORD_LEN];
    BYTE service_class;
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_sim_get_facility_lock_req;

typedef struct
{
    RCM_HEADER hdr;
    //INT32 service_class;
    BYTE lock_mode;
    BYTE service_class;
}sit_sim_get_facility_lock_rsp;

/*
    SIT_SET_FACILITY_LOCK  (RCM ID = 0x020A)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE code;
    BYTE lock_mode;
    BYTE password_len;
    BYTE password[MAX_SIM_FACILITY_PASSWORD_LEN];
    BYTE service_class;
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_sim_set_facility_lock_req;

/*
typedef struct
{
    RCM_HEADER hdr;
    INT32 remain_count;
}sit_sim_set_facility_lock_rsp;
*/
// same as SIT_VERIFY_SIM_PIN
typedef sit_sim_verify_sim_pin_rsp sit_sim_set_facility_lock_rsp;

/*
    SIT_GET_SIM_AUTH  (RCM ID = 0x020B)
*/
typedef enum
{
    SIT_SIM_AUTH_IMS = 0x00,
    SIT_SIM_AUTH_GSM,
    SIT_SIM_AUTH_3G
}sit_sim_get_sim_auth_type;

typedef struct
{
    RCM_HEADER hdr;
    BYTE auth_type;
    BYTE auth_len;
    BYTE auth[MAX_SIM_AUTH_REQ_LEN];
    /*
     * IMS Auth : RAND_LEN(1) + RAND(V) + AUTN_LEN(1) + AUTH(V)
     * GSM Auth : RAND_LEN(1) + RAND(V)
     * 3G Auth : RAND_LEN(1) + RAND(V) + AUTN_LEN(1) + AUTH(V)
     */
}sit_sim_get_sim_auth_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE auth_type;
    BYTE auth_len;
    BYTE auth[MAX_SIM_AUTH_RSP_LEN];
    /*
     * Result(1) + Auth Response
     * IMS Auth : RES_AUTS_len(1) + RES_AUTS(V) + CK_len(1) + CK(V) + IK_len(1) + IK(V) + KC_len(1) + KC(V)
     * GSM Auth : SRES_len(1) + SRES(V) + Kc_len(1) + Kc(V)
     * 3G Auth : RES_AUTS_len(1) + RES_AUTS(V) + CK_len(1) + CK(V) + IK_len(1) + IK(V) + KC_len(1) + KC(V)
     */
}sit_sim_get_sim_auth_rsp;

/*
    SIT_GET_GBA_CONTEXT  (RCM ID = 0x0211)
*/
typedef enum
{
    SIT_ISIM_AUTH_USIM = 0x00,
    SIT_ISIM_AUTH_ISIM
}sit_sim_get_sim_gba_auth_type;

typedef struct
{
    RCM_HEADER hdr;
    BYTE auth_type;
    BYTE gba_type;
    BYTE gba_tag;
    BYTE data1_len;
    BYTE data1[255];
    BYTE data2_len;
    BYTE data2[255];
}sit_sim_get_sim_gba_auth_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE auth[MAX_SIM_GBA_AUTH_RSP_LEN];
}sit_sim_get_sim_gba_auth_rsp;

typedef struct
{
    BYTE cla;
    BYTE instruction;
    BYTE p1;
    BYTE p2;
    BYTE p3;
    BYTE data[0];
}sit_sim_apdu;

/*
    SIT_TRANSMIT_SIM_APDU_BASIC (RCM ID = 0x020C)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 session_id;
    UINT16 apdu_len;
    union {
        sit_sim_apdu sim_apdu;
        BYTE apdu[0];
    } entry;
}sit_sim_transmit_sim_apdu_basic_req;

typedef struct
{
    RCM_HEADER hdr;
    WORD apdu_len;
    BYTE apdu[0];      // MAX_APDU_LEN is 16K
}sit_sim_transmit_sim_apdu_basic_rsp;

/*
    SIT_OPEN_SIM_CHANNEL (RCM ID = 0x020D)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_sim_open_sim_channel_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 session_id;
    BYTE sw1;
    BYTE sw2;
    INT16 response_len;
    BYTE response[MAX_OPEN_CHANNEL_RSP_LEN];
}sit_sim_open_channel_rsp;


/*
    SIT_CLOSE_SIM_CHANNEL (RCM ID = 0x020E)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 session_id;
}sit_sim_close_sim_channel_req;

typedef null_data_format sit_sim_close_sim_channel_rsp;


/*
    SIT_TRANSMIT_SIM_APDU_CHANNEL (RCM ID = 0x020F)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 session_id;
    INT32 cla;
    INT32 instruction;
    INT32 p1;
    INT32 p2;
    INT32 p3;
    UINT16 data_len;
    BYTE data[0];
}sit_sim_transmit_sim_apdu_channel_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE sw1;
    BYTE sw2;
    INT16 response_len;
    BYTE response[0];
}sit_sim_transmit_sim_apdu_channel_rsp;


/*
    SIT_IND_SIM_STATUS_CHANGED (RCM ID = 0x0210)
*/
typedef null_ind_data_format sit_sim_sim_status_changed_ind;


/* PhoneBook */

/*
    SIT_READ_PB_ENTRY (RCM_ID = 0x0240)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE pb_type;
    UINT16 index;
}sit_read_pb_entry_req;

typedef struct {
    RCM_HEADER hdr;
    BYTE pb_type;
    UINT16 index;
    UINT16 data_len;
    char entry_data[MAX_PB_ENTRY_LEN];
}sit_read_pb_resp;

/*
    SIT_UPDATE_PB_ENTRY (RCM_ID = 0x0241)
*/
typedef struct
{
    BYTE num_len;
    BYTE num_type;
    BYTE *number;
    BYTE text_len;
    BYTE text_type;
    BYTE *text;
}sit_pb_entry1;

typedef struct
{
    BYTE text_len;
    BYTE text_type;
    BYTE *text;
}sit_pb_entry2;

typedef struct
{
    BYTE type3g;
    UINT16 data_len;
    BYTE data_type;
    BYTE *data;
}sit_3g_pb;

typedef struct
{
    RCM_HEADER hdr;
    BYTE mode;
    BYTE pb_type;
    BYTE index;
    UINT16 entry_len;
    union {
        sit_pb_entry1 pb1;
        sit_pb_entry2 pb2;
        sit_3g_pb *pb3g;
    } entry;
}sit_update_pb_entry_req;

typedef struct{
    RCM_HEADER hdr;
    BYTE mode;
    BYTE pb_type;
    BYTE index;
}sit_update_pb_entry_resp;

/*
    SIT_GET_PB_STORAGE_INFO (RCM_ID = 0x0242)
*/
typedef struct {
    RCM_HEADER hdr;
    BYTE pb_type;
}sit_sim_pb_storage_info;

typedef struct {
    RCM_HEADER hdr;
    BYTE pb_type;
    UINT16 total_count;
    UINT16 used_count;
}sit_sim_pb_storage_info_rsp;

/*
    SIT_GET_PB_STORAGE_LIST (RCM_ID = 0x0243)
*/
typedef struct {
    RCM_HEADER hdr;
    int pb_list;
}sit_sim_pb_storage_list_rsp;

/*
    SIT_GET_PB_ENTRY_INFO (RCM_ID = 0x0244)
*/
typedef struct {
    RCM_HEADER hdr;
    BYTE pb_type;
}sit_sim_pb_entry_info;

typedef struct {
    RCM_HEADER hdr;
    BYTE pb_type;
    UINT16 index_min;
    UINT16 index_max;
    UINT16 num_max;
    UINT16 text_max;
}sit_sim_pb_entry_info_rsp;

/*
    SIT_GET_3G_PB_CAPA (RCM_ID = 0x0245)
*/
typedef struct {
    BYTE pb_type;
    UINT16 index_max;
    UINT16 entry_max;
    UINT16 used_count;
}sit_sim_pb_capa;

typedef struct
{
    RCM_HEADER hdr;
    BYTE entry_num;
    sit_sim_pb_capa pb_list[MAX_PB_ENTRY_NUM];
}sit_sim_pb_capa_rsp;

/*
    SIT_SIM_IND_PB_READY (RCM ID = 0x0246)
*/
typedef struct {
    RCM_IND_HEADER hdr;
    BYTE pb_ready;
}sit_sim_pb_ready_ind;

/*
    SIT_OPEN_SIM_CHANNEL_WITH_P2 (RCM ID = 0x0247)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
    BYTE p2;
}sit_sim_open_sim_channel_with_p2_req;

/*
    SIT_IND_ICCID_INFO (RCM ID = 0x0248)
*/

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE iccid_len;
    BYTE iccid[MAX_ICCID_LEN];
}sit_sim_iccid_info_ind;

/*
    SIT_SET_SET_UICC_SUBSCRIPTION (RCM ID = 0x0249)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE state;    // 0: deactivated, 1: activated which are same to RIL_UiccSubActStatus
}sit_sim_set_uicc_sub_req;

/*
    SIT_IND_UICC_SUBSCRIPTION_STATE_CHANGED (RCM ID = 0x024A)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    INT32 state;    // 0: deactivated, 1: activated which are same to RIL_UiccSubActStatus
}sit_sim_uicc_sub_state_changed_ind;

/*
    SIT_SET_SIM_CARD_POWER (RCM ID = 0x024C)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE mode;    // 0: UP, 1: DOWON
}sit_sim_set_sim_card_power_req;

/*
    SIT_IND_SIM_DATA_INFO (RCM ID = 0x024F)
*/
typedef struct {
    RCM_IND_HEADER hdr;
    UINT16 sim_file_id;
    UINT16 record_len;
    BYTE num_of_records;
    BYTE data_info[2];    // size is variant.
}sit_sim_file_data_info_ind;

/*
    SIT_GET_SLOT_STATUS (RCM ID = 0x024D)
*/
typedef struct
{
    BYTE card_state;
    BYTE slot_state;
    BYTE atr_len;
    BYTE atr[MAX_ATR_LEN_FOR_SLOT_STATUS];
    BYTE logicalSlotId;
    BYTE iccid_len;
    BYTE iccid[MAX_ICCID_LEN];
    BYTE eid_len;           // for RadioConfigV1.2
    BYTE eid[MAX_EID_LEN];  // for RadioConfigV1.2
} sit_sim_get_slot_status_info;

typedef struct
{
    RCM_HEADER hdr;
    BYTE num_of_info;
    sit_sim_get_slot_status_info info[MAX_SLOT_NUM];
} sit_sim_get_slot_status_resp;

/*
    SIT_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING (RCM ID = 0x024E)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE slotMaplen;
    BYTE slotMap[MAX_SLOT_NUM];
} sit_sim_set_logical_to_physical_slot_mapping_req;

/*
    SIT_IND_SIM_SLOT_STATUS_CHANGED (RCM ID = 0x024F)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE num_of_info;
    sit_sim_get_slot_status_info info[MAX_SLOT_NUM];
} sit_sim_slot_status_changed_ind;

/*
    SIT_SEND_STK_ENVELOPE_CMD (RCM ID = 0x0300)
*/
typedef struct
{
    RCM_HEADER hdr;
    WORD envelope_cmd_len;
    BYTE envelope_cmd[MAX_SIM_IO_DATA_LEN];
}sit_stk_send_stk_envelope_cmd_req;

typedef struct
{
    RCM_HEADER hdr;
    WORD envelope_rsp_len;
    BYTE envelope_rsp[MAX_SIM_IO_DATA_LEN];
}sit_stk_send_stk_envelope_cmd_rsp;

/*
    SIT_SEND_STK_TERMINAL_RSP (RCM ID = 0x0301)
*/
typedef struct
{
    RCM_HEADER hdr;
    WORD terminal_rsp_len;
    BYTE terminal_rsp[MAX_SIM_IO_DATA_LEN];
}sit_stk_send_stk_terminal_rsp_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE sw1;
    BYTE sw2;
}sit_stk_send_stk_terminal_rsp_rsp;

/*
    SIT_SEND_STK_ENVELOPE_WITH_STATUS (RCM ID = 0x0302)
*/
typedef struct
{
    RCM_HEADER hdr;
    WORD envelope_cmd_len;
    BYTE envelope_cmd[MAX_SIM_IO_DATA_LEN];
}sit_stk_send_stk_envelope_with_status_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE sw1;
    BYTE sw2;
    WORD envelope_rsp_len;
    BYTE envelope_rsp[MAX_SIM_IO_DATA_LEN];
}sit_stk_send_stk_envelope_with_status_rsp;

/*
    SIT_IND_STK_PROACTIVE_COMMAND (RCM ID = 0x0303)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    WORD proactive_cmd_len;
    BYTE proactive_cmd[MAX_SIM_IO_DATA_LEN];
}sit_stk_stk_proactive_cmd_ind;

/*
    SIT_IND_SIM_REFRESH (RCM ID = 0x0304)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE result;
}sit_stk_sim_refresh_ind;

/*
    SIT_STK_CALL_SETUP (RCM ID = 0x0305)
*/
typedef struct {
    RCM_HEADER hdr;
    BYTE user_operation;
}sit_stk_call_setup_req;

typedef null_data_format sit_stk_call_setup_rsp;

typedef enum
{
    SIT_STK_CALL_SETUP_ACCEPT = 0x00,
    SIT_STK_CALL_SETUP_REJECT
}sit_stk_setup_call_user_operation_e_type;

/*
    SIT_IND_STK_CC_ALPHA_NOTIFY (RCM ID = 0x0307)
*/
typedef struct {
    RCM_IND_HEADER hdr;
    BYTE alpha_len;
    BYTE alpha_buf[MAX_ALPHA_INFO_BUF_LEN];
}sit_stk_cc_alpha_notify_ind;

/*
    SIT_GET_IMSI (RCM ID = 0x0400)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE aid_len;
    BYTE aid[MAX_SIM_AID_LEN];
}sit_id_get_imsi_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE imsi_len;
    BYTE imsi[MAX_IMSI_LEN];
}sit_id_get_imsi_rp;

/*
    SIT_GET_IMEI (RCM ID = 0x0401)
*/
typedef null_data_format sit_id_get_imei_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE imei_len;
    BYTE imei[MAX_IMEI_LEN];
}sit_id_get_imei_rsp;


/*
    SIT_GET_IMEISV (RCM ID = 0x0402)
*/
typedef null_data_format sit_id_get_imeisv_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE imeisv_len;
    BYTE imeisv[MAX_IMEISV_LEN];
}sit_id_get_imeisv_rsp;

/*
    SIT_GET_DEVICE_ID (RCM ID = 0x0403)
*/
typedef null_data_format sit_id_get_deviceid_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE imei_len;
    BYTE imei[MAX_IMEI_LEN];
    BYTE imeisv_len;
    BYTE imesv[MAX_IMEISV_LEN];
    BYTE esn_len;
    BYTE esn[MAX_ESN_LEN];
    BYTE meid_len;
    BYTE meid[MAX_MEID_LEN];
}sit_id_get_deviceid_rsp;

/*
    SIT_SET_SGC (RCM ID = 0x0404)
*/
typedef struct
{
    RCM_HEADER hdr;
    UINT32 SGC;
    UINT32 Rsv1;
    UINT32 Rsv2;
}sit_id_set_sgc_req;

typedef enum
{
    // EUOPEN GCF
    SIT_ID_SGC_SP_GCF = 0x0000,

    //GLOBAL
    SIT_ID_SGC_SP_WHITE = 0x0100,
    SIT_ID_SGC_SP_EUROPEN = 0x0101,
    SIT_ID_SGC_SP_TMO = 0x0102,
    SIT_ID_SGC_SP_SEAOPEN = 0x0103,
    SIT_ID_SGC_SP_AUS_TELSTRA = 0x0104,
    SIT_ID_SGC_SP_SEANZC = 0x0105,
    SIT_ID_SGC_SP_HUTCH = 0x0106,
    SIT_ID_SGC_SP_ORG = 0x0107,
    SIT_ID_SGC_SP_VODA = 0x0108,
    SIT_ID_SGC_SP_AUSOPEN = 0x0109,

    //N_AMERICA
    SIT_ID_SGC_SP_RED = 0x0200,
    SIT_ID_SGC_SP_AIO = 0x0201,
    SIT_ID_SGC_SP_ATT = 0x0202,
    SIT_ID_SGC_SP_TMOUSA = 0x0203,
    SIT_ID_SGC_SP_ATT_MVNO = 0x0204,
    SIT_ID_SGC_SP_TMOUSA_MVNO = 0x0205,

    //LATIN_AMERICA
    SIT_ID_SGC_SP_PUMPKIN = 0x0400,
    SIT_ID_SGC_SP_LATIN = 0x0401,
    SIT_ID_SGC_SP_LATIN_GCF = 0x0402,

    //N_AMERICA_CAN
    SIT_ID_SGC_SP_YELLOW = 0x0800,
    SIT_ID_SGC_SP_BMC = 0x0801,
    SIT_ID_SGC_SP_TELUS = 0x0802,
    SIT_ID_SGC_SP_ROGERS = 0x0803,
    SIT_ID_SGC_SP_FREEDOM = 0x0804,

    //N_AMERICA_3GPP2
    SIT_ID_SGC_SP_GREEN = 0x1000,
    SIT_ID_SGC_SP_VZW = 0x1001,
    SIT_ID_SGC_SP_USCC = 0x1002,
    SIT_ID_SGC_SP_SPR = 0x1003,

    //CHINA
    SIT_ID_SGC_SP_BLUE = 0x2000,
    SIT_ID_SGC_SP_CHNOPEN = 0x2001,
    SIT_ID_SGC_SP_CMCC = 0x2002,
    SIT_ID_SGC_SP_HKTW = 0x2003,
    SIT_ID_SGC_SP_CTC = 0x2004,
    SIT_ID_SGC_SP_CUCC = 0x2005,
    SIT_ID_SGC_SP_CHNOPEN_GCF = 0x2006,

    //KOR
    SIT_ID_SGC_SP_INDIGO = 0x4000,
    SIT_ID_SGC_SP_KT = 0x4001,
    SIT_ID_SGC_SP_LGU = 0x4002,
    SIT_ID_SGC_SP_KOROPEN = 0x4003,
    SIT_ID_SGC_SP_SKT = 0x4004,

    //JPN
    SIT_ID_SGC_SP_VIOLET = 0x8000,
    SIT_ID_SGC_SP_NTT = 0x8001,
    SIT_ID_SGC_SP_KDDI = 0x8002,
    SIT_ID_SGC_SP_SBM = 0x8003,

    //LATAM
    SIT_ID_SGC_CLARO_AR = 0x0403,
    SIT_ID_SGC_MOV_AR = 0x0404,
    SIT_ID_SGC_TUENTI_AR = 0x0405,
    SIT_ID_SGC_NII_AR = 0x0406,
    SIT_ID_SGC_NUESTRO_AR = 0x0407,
    SIT_ID_SGC_PERSONAL_AR = 0x0407,
    SIT_ID_SGC_TIGO_BO = 0x0408,
    SIT_ID_SGC_VIVA_BO = 0x0409,
    SIT_ID_SGC_CLARO_BR = 0x040A,
    SIT_ID_SGC_VIVO_BR = 0x040B,
    SIT_ID_SGC_NII_BR = 0x040C,
    SIT_ID_SGC_OI_BR = 0x040D,
    SIT_ID_SGC_PORTO_CONECTA_BR = 0x040E,
    SIT_ID_SGC_SURF_BR = 0x040F,
    SIT_ID_SGC_TIM_BR = 0x0410,
    SIT_ID_SGC_CLARO_CL = 0x0411,
    SIT_ID_SGC_MOV_CL = 0x0412,
    SIT_ID_SGC_ENTEL_CL = 0x0413,
    SIT_ID_SGC_WOM_CL = 0x0414,
    SIT_ID_SGC_CLARO_CO = 0x0415,
    SIT_ID_SGC_MOV_CO = 0x0416,
    SIT_ID_SGC_AVANTEL_CO = 0x0417,
    SIT_ID_SGC_ETB_CO = 0x0418,
    SIT_ID_SGC_TIGO_CO = 0x0419,
    SIT_ID_SGC_CLARO_CR = 0x041A,
    SIT_ID_SGC_CLARO_DO = 0x041B,
    SIT_ID_SGC_CLARO_EC = 0x041C,
    SIT_ID_SGC_MOV_EC = 0x041D,
    SIT_ID_SGC_CNT_EC = 0x041E,
    SIT_ID_SGC_CLARO_SV = 0x041F,
    SIT_ID_SGC_TIGO_SV = 0x0420,
    SIT_ID_SGC_CLARO_GT = 0x0421,
    SIT_ID_SGC_TIGO_GT = 0x0422,
    SIT_ID_SGC_CLARO_HN = 0x0423,
    SIT_ID_SGC_TIGO_HO = 0x0424,
    SIT_ID_SGC_TELCEL_MX = 0x025,
    SIT_ID_SGC_MOV_MX = 0x0426,
    SIT_ID_SGC_ALTAN_MX = 0x0427,
    SIT_ID_SGC_ATT_MX = 0x0428,
    SIT_ID_SGC_CLARO_NI = 0x0429,
    SIT_ID_SGC_CLARO_PA = 0x042A,
    SIT_ID_SGC_CLARO_PY = 0x042B,
    SIT_ID_SGC_PERSONAL_PY = 0x042C,
    SIT_ID_SGC_TIGO_PY = 0x042D,
    SIT_ID_SGC_CLARO_PE = 0x042E,
    SIT_ID_SGC_MOV_PE = 0x042F,
    SIT_ID_SGC_ENTEL_PE = 0x0430,
    SIT_ID_SGC_CLARO_PR = 0x0431,
    SIT_ID_SGC_OPEN_MOBILE_PR = 0x0432,
    SIT_ID_SGC_CLARO_UY = 0x0433,
    SIT_ID_SGC_MOV_UY = 0x0434,
    SIT_ID_SGC_ANTEL_UY = 0x0435,
    SIT_ID_SGC_MOV_UZ = 0x0436,

    SIT_ID_SGC_SP_IDX_MAX
}sit_id_sgc_op_idx_e_type;

typedef null_data_format sit_id_set_sgc_rsp;


/*
    SIT_SET_CARRIER_INFO_IMSI_ENCRYPTION (RCM ID = 0x0406)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE mcc[MAX_MCC_LEN];
    BYTE mnc[MAX_MNC_LEN];
    INT32 carrier_len;
    BYTE carrier_key[MAX_IMSI_ENCRIPTION_KEY_LEN];
    BYTE key_id_len;
    BYTE key_id[MAX_IMSI_ENCRIPTION_KEY_LEN];
    LONG expire_time;
}sit_id_set_carrier_info_imsi_encription_req;

typedef null_data_format sit_id_set_carrier_info_imsi_encription_rsp;

/*
    SIT_IND_CARRIER_INFO_IMSI_ENCRYPTION (RCM ID = 0x0407)
*/
typedef null_ind_data_format sit_id_carrier_info_imsi_encription_ind;

/*
    SIT_HANGUP_WAITING_OR_BACKGROUND (RCM ID =0x0500)
*/
typedef null_data_format sit_ss_hangup_waiting_or_background_req;
typedef null_data_format sit_ss_hangup_waiting_or_background_rsp;

/*
    SIT_HANGUP_FORGROUND_RESUME_BACKGROUND (RCM ID =0x0501)
*/
typedef null_data_format sit_ss_hangup_foreground_resume_background_req;
typedef null_data_format sit_ss_hangup_foreground_resume_background_rsp;

/*
    SIT_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE (RCM ID =0x0502)
*/
typedef null_data_format sit_ss_switch_waiting_or_holding_and_active_req;
typedef null_data_format sit_ss_switch_waiting_or_holding_and_active_rsp;

/*
    SIT_CONFERENCE (RCM ID =0x0503)
*/
typedef null_data_format sit_ss_conference_req;
typedef null_data_format sit_ss_conference_rsp;

/*
    SIT_UDUB (RCM ID =0x0504)
*/
typedef null_data_format sit_ss_udub_req;
typedef null_data_format sit_ss_udub_rsp;

/*
    SIT_SEND_USSD (RCM ID =0x0505)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE ussd_type;
    BYTE dcs;
    BYTE ussd_len;
    BYTE ussd[MAX_USSD_DATA_LEN];
}sit_ss_send_ussd_req;

typedef null_data_format sit_ss_send_ussd_rsp;

typedef enum
{
    SIT_SS_USSD_USER_INITIATED = 0X00,
    SIT_SS_USSD_USER_RESPONSE,
    SIT_SS_USSD_USER_RELEASE,

    SIT_SS_USSD_MAX
}sit_ss_ussd_type_e_type;

/*
    SIT_CANCEL_USSD (RCM ID =0x0506)
*/
typedef null_data_format sit_ss_cancel_ussd_req;
typedef null_data_format sit_ss_cancel_ussd_rsp;

/*
    SIT_GET_CLIR (RCM ID =0x0507)
*/
typedef null_data_format sit_ss_get_clir_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 clir_status;    // INT->INT32 for compile, rkkwon 20140701
}sit_ss_get_clir_rsp;

// for aoc, refer sit_call_clir_e_type in dial

typedef enum
{
    SIT_SS_GET_CLIR_STATUS_NOT_PROVISIONED = 0,
    SIT_SS_GET_CLIR_STATUS_PROVISIONED,
    SIT_SS_GET_CLIR_STATUS_UNKNOWN,
    SIT_SS_GET_CLIR_STATUS_TEMP_RESTRICTED,
    SIT_SS_GET_CLIR_STATUS_TEMP_ALLOWED,

    SIT_SS_GET_CLIR_STATUS_MAX
}sit_ss_get_clir_status_e_type;

/*
    SIT_GET_COLP (RCM ID =0x0508)
*/
typedef null_data_format sit_ss_get_colp_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 status;
}sit_ss_get_colp_rsp;

/*
    SIT_GET_CALL_FORWARD_STATUS (RCM ID =0x0509)
*/
typedef struct
{
    RCM_HEADER hdr;
    UINT32 status;
    INT32 reason;
    INT32 service_class;
    INT32 toa;
    BYTE num_len;
    BYTE number[MAX_SS_NUM_LEN];
    INT32 timeseconds;
}sit_ss_get_call_forward_status_req;

typedef struct
{
    INT32 status;
    INT32 reason;
    INT32 service_class;
    INT32 toa;
    BYTE num_len;
    BYTE number[MAX_SS_NUM_LEN];
    INT32 timeseconds;
}sit_ss_call_forward_item;

typedef struct
{
    RCM_HEADER hdr;
    INT32 call_forward_num;
    sit_ss_call_forward_item record[MAX_CALL_FORWARD_STATUS_NUM];
}sit_ss_get_call_forward_status_rsp;

//refer sit_ss_call_forward_status_e_type in set request

typedef enum
{
    SIT_SS_CALL_FORWARD_READON_UNCONDITIONAL = 0,
    SIT_SS_CALL_FORWARD_READON_MOBILE_BUSY,
    SIT_SS_CALL_FORWARD_READON_NO_REPLY,
    SIT_SS_CALL_FORWARD_READON_NOT_REACHABLE,
    SIT_SS_CALL_FORWARD_READON_ALL,
    SIT_SS_CALL_FORWARD_READON_ALL_CONDITIONAL,

    SIT_SS_CALL_FORWARD_READON_MAX
}sit_ss_call_forward_reason_e_type;

typedef enum
{
    SIT_SS_SERVICE_CLASS_VOICE = 1,
    SIT_SS_SERVICE_CLASS_DATA = 2,
    SIT_SS_SERVICE_CLASS_FAX = 4,
    SIT_SS_SERVICE_CLASS_SMS = 8,
    SIT_SS_SERVICE_CLASS_DATA_CIRCUIT_SYNC = 16,
    SIT_SS_SERVICE_CLASS_DATA_CIRCUIT_ASYNC = 32,
    SIT_SS_SERVICE_CLASS_DEDICATED_PACKET_ACCESS = 64,
    SIT_SS_SERVICE_CLASS_DEDICATED_PAD_ACCESS = 128,

    SIT_SS_SERVICE_CLASS_MAX
}sit_ss_service_class_e_type;

typedef enum
{
    SIT_SS_TOA_UNKNOWN = 0x00,
    SIT_SS_TOA_INTERNATIONAL = 0x10,
    SIT_SS_TOA_NATIONAL = 0x20,
    SIT_SS_TOA_NETWORK_SPECIFIC = 0x30,
    SIT_SS_TOA_DEDICATED_SHORT_ACCESS_CODE = 0x40,

    SIT_SS_TOA_MAX
}sit_ss_toa_e_type;


/*
    SIT_SET_CALL_FORWARD (RCM ID =0x050A)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 status;
    INT32 reason;
    INT32 service_class;
    INT32 toa;
    BYTE num_len;
    BYTE number[MAX_SS_NUM_LEN];
    INT32 timeseconds;
}sit_ss_set_call_forward_req;

typedef null_data_format sit_ss_set_call_forward_rsp;

typedef enum
{
    SIT_SS_CALL_FARWARD_STATUS_DISABLE = 0,
    SIT_SS_CALL_FARWARD_STATUS_NOT_ACTIVE = 0, /*GET : not active*/
    SIT_SS_CALL_FARWARD_STATUS_ENABLE = 1,
    SIT_SS_CALL_FARWARD_STATUS_ACTIVE = 1,        /*GET : active*/
    SIT_SS_CALL_FARWARD_STATUS_INTERROGATE,
    SIT_SS_CALL_FARWARD_STATUS_REGISTRATION,
    SIT_SS_CALL_FARWARD_STATUS_ERASURE,

    SIT_SS_CALL_FARWARD_STATUS_MAX
}sit_ss_call_forward_status_e_type;

/*
    SIT_GET_CALL_WAITING (RCM ID =0x050B)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 service_class;
}sit_ss_get_call_waiting_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 status;
    INT32 service_class;
}sit_ss_get_call_waiting_rsp;

/*
    SIT_SET_CALL_WAITING (RCM ID =0x050C)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 status;
    INT32 service_class;
}sit_ss_set_call_waiting_req;

typedef null_data_format sit_ss_set_call_waiting_rsp;

/*
    SIT_CHG_BARRING_PWD (RCM ID =0x050D)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE pwd[4];
    BYTE new_pwd[4];
    BYTE new_pwd_again[4];
}sit_ss_change_barring_pwd_req;

typedef null_data_format sit_ss_change_barring_pwd_rsp;

/*
    SIT_SEPARATE_CONNECTION (RCM ID =0x050E)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 index;
}sit_ss_separate_connection_req;

typedef null_data_format sit_ss_separate_connection_rsp;

/*
    SIT_GET_CLIP (RCM ID =0x050F)
*/
typedef null_data_format sit_ss_get_clip_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE status;
}sit_ss_get_clip_rsp;

typedef enum
{
    SIT_SS_GET_CLIP_STATUS_NOT_PROVISIONED = 0,
    SIT_SS_GET_CLIP_STATUS_PROVISIONED,
    SIT_SS_GET_CLIP_STATUS_UNKNOWN,

    SIT_SS_GET_CLIP_STATUS_MAX
}sit_ss_get_clip_status_e_type;

/*
    SIT_IND_ON_USSD (RCM ID =0x0510)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE status;
    BYTE dcs;
    BYTE ussd_len;
    BYTE ussd[MAX_USSD_DATA_LEN];
}sit_ss_ussd_ind;

typedef enum
{
    SIT_SS_USSD_IND_STATUS_USSD_NOTIFY = 0,
    SIT_SS_USSD_IND_STATUS_USSD_REQUEST,
    SIT_SS_USSD_IND_STATUS_SESSION_TERMINATED_BY_NETWORK,
    SIT_SS_USSD_IND_STATUS_OTHER_LOCAL_CLIENT_HAS_RESPONDED,
    SIT_SS_USSD_IND_STATUS_OPERATION_NOT_SUPPORTED,
    SIT_SS_USSD_IND_STATUS_NETWORK_TIMEOUT,

    SIT_SS_USSD_IND_STATUS_MAX
}sit_ss_ussd_ind_status_e_type;


/*
    SIT_IND_SUPP_SVC_NOTIFICATION (RCM ID =0x0511)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    INT32 noti_type;
    INT32 code;
    INT32 index;
    UINT type;
    BYTE num_len;
    BYTE num[MAX_SS_NUM_LEN];
}sit_ss_supp_svc_notification_ind;

typedef enum
{
    SIT_SS_SSNOTI_TYPE_MO,
    SIT_SS_SSNOTI_TYPE_MT
}sit_ss_ssnoti_type_e_type;

typedef enum
{
    SIT_SS_CODE1_UNCOND_CF_ACTIVE = 0,        /* unconditional call forwarding is active */
    SIT_SS_CODE1_SOME_COND_CF_ACTIVE = 1,    /* some of the conditional call forwardings are active */
    SIT_SS_CODE1_CALL_FORWARDED = 2,        /* call has been forwarded */
    SIT_SS_CODE1_CALL_WAITING = 3,            /* call is waiting */
    SIT_SS_CODE1_CUG_CALL = 4,                /* this is a CUG call (also <index> present) */
    SIT_SS_CODE1_OUTGOING_BARRED = 5,        /* outgoing calls are barred */
    SIT_SS_CODE1_INCOMING_BARRED = 6,        /* incoming calls are barred */
    SIT_SS_CODE1_CLIR_SUPP_REJECTED = 7,        /* CLIR suppression rejected */
    SIT_SS_CODE1_CALL_DEFLECTED = 8            /* call has been deflected */
}sit_ss_code1_e_type;

typedef enum
{
    SIT_SS_CODE2_FORWARDED_CALL = 0,        /* this is a forwarded call (MT call setup) */
    SIT_SS_CODE2_CUG_CALL = 1,                /* this is a CUG call (also <index> present) (MT call setup) */
    SIT_SS_CODE2_HOLD = 2,                    /* call has been put on hold (during a voice call) */
    SIT_SS_CODE2_RETRIEVED = 3,                /* call has been retrieved (during a voice call) */
    SIT_SS_CODE2_MPTY_ENTERED = 4,            /* multiparty call entered (during a voice call) */
    SIT_SS_CODE2_HOLD_CALL_RELEASED = 5,    /* call on hold has been released (this is not a SS notification) (during a voice call) */
    SIT_SS_CODE2_FORWARD_CHECK_RECEIVED = 6,    /* forward check SS message received (can be received whenever) */
    SIT_SS_CODE2_EXPLICIT_CF_INCOMING = 7,    /* call is being connected (alerting) with the remote party in alerting state in explicit call transfer operation
(during a voice call) */
    SIT_SS_CODE2_EXPLICIT_CF_CONNECTED = 8,    /* call has been connected with the other remote party in explicit call transfer operation (also number and
subaddress parameters may be present) (during a voice call or MT call setup) */
    SIT_SS_CODE2_DEFLECTED_CALL = 9,            /* this is a deflected call (MT call setup) */
    SIT_SS_CODE2_ADD_INCOMING_CALL_FORWARDED = 10    /* additional incoming call forwarded */
}sit_ss_code2_e_type;

/*
    SIT_GET_COLR (RCM ID =0x0512)
*/
typedef null_data_format sit_ss_get_colr_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 status;
}sit_ss_get_colr_rsp;

/*
    SIT_IND_SS_RETURN_RESULT (RCM ID =0x0513)
*/
typedef struct
{
    RCM_IND_HEADER hdr;    // [arkade] 2014/10/29 RCM_HEADER is changed to RCM_IND_HEADER
    BYTE return_result_len;
    BYTE return_result[MAX_SS_RETURN_RESULT];
}sit_ss_return_result_ind;

/*
    SIT_IND_ON_SS (RCM ID =0x0516)
*/
typedef struct
{
    INT32 call_forward_num;
    sit_ss_call_forward_item record[MAX_CALL_FORWARD_STATUS_NUM];
}sit_ss_cf_info_type;

typedef struct
{
    RCM_IND_HEADER hdr;    // [arkade] 2014/10/29 RCM_HEADER is changed to RCM_IND_HEADER
    BYTE service_type;    // same to RIL_SsServiceType
    BYTE request_type;    // same to RIL_SsRequestType
    BYTE teleservice_type;   // same to RIL_SsTeleserviceType
    INT32 service_class;    // same to sit_ss_service_class_e_type
    INT32 result;    // RCM error
    BYTE data_type;  // sit_ss_on_ss_ind_data_type
    union {
        INT32 ss_info[MAX_SS_INFO_NUM];
        sit_ss_cf_info_type cf_info;
    }data;
}sit_ss_on_ss_ind;

typedef enum
{
    SIT_ON_SS_IND_SS_INFO = 0,
    SIT_ON_SS_IND_CF_INFO = 1,
} sit_ss_on_ss_ind_data_type;

/*
    SIT_SETUP_DATA_CALL (RCM ID = 0x0600)
*/
typedef struct
{
    INT16 status;
    BYTE cid;
    BYTE active;
    BYTE pdp_type;
    BYTE address[MAX_PDP_ADDRESS_LEN];
    BYTE dns_type;
    BYTE primary_dns[MAX_PDP_ADDRESS_LEN];
    BYTE secondary_dns[MAX_PDP_ADDRESS_LEN];
    BYTE pcscf_type;
    BYTE pcscf[MAX_PCSCF_ADDRESS_LEN];
    INT32 mtu_size; // Invalid: 0
    BYTE pco;       // Invalid: 0xFF
}sit_pdp_data_call_item;

/*
   SIT_SETUP_DATA_CALL Extension 1
   Previous Payload should be checked by Size.
   From This Extension First Two byte works as Extension header
   This extension block will be added on each DataCall in DATA_CALL_LIST_CHANGED_IND

   IPC_version (1) is reserved for old Header. Will ignore extension Bytes
   IPC_version (2) is used for pcscf extension only.
                  and first extension header should be pcscf_ext
   IPC_version (3) will be used in the future, this will use NextHeaderType
                  and first extension header should be pcscf_ext
   IPC_version > (4) is not used. But inevitably if policy should be changed,
                  these value can be used to use fixed payload format.

   Future Use
    NextHeaderType will show next Header Type.
    Reserved Type List
        (0) means no nextheader
        (1) means PCSCF extension header
*/

typedef struct
{
    BYTE IPC_version;
    BYTE NextHeaderType;
    BYTE pcscf_ipv4_ext[MAX_IPV4_ADDR_LEN * 5];
    BYTE pcscf_ipv6_ext[MAX_IPV6_ADDR_LEN * 5];
}sit_pdp_data_call_item_pcscf_ext;

typedef struct
{
    BYTE low_priority;
    INT32 suggestedRetryTime;
} sit_pdp_data_call_item_ext_lp_retrytime;


/*
 * TS 24.301 9.9.4.2 APN aggregate maximum bit rate
 *
 * octet1 : APN aggregate maximum bit rate IEI code, it's decoded by CP.
 * octet2 : Length of APN aggreate maximum bit rate contents
 * octet3 : APN-AMBR for downlink
 * octet4 : APN-AMBR for uplink
 * octet5 : APN-AMBR for downlink (extended)
 * octet6 : APN-AMBR for uplink (extended)
 * octet7 : APN-AMBR for downlink (extended2)
 * octet8 : APN-AMBR for uplink (extended2)
 */
typedef struct
{
    BYTE octet2;
    BYTE octet3;
    BYTE octet4;
    BYTE octet5;
    BYTE octet6;
    BYTE octet7;
    BYTE octet8;
} sit_pdp_data_call_item_ext_ambr;

/*
   Old definition
*/
typedef struct
{
    INT16 status;
    BYTE cid;
    BYTE active;
    BYTE pdp_type;
    BYTE address[MAX_PDP_ADDRESS_LEN];
    BYTE primary_dns[MAX_PDP_ADDRESS_LEN];
    BYTE secondary_dns[MAX_PDP_ADDRESS_LEN];
    BYTE pcscf[MAX_PCSCF_ADDRESS_LEN];
}sit_pdp_data_call_item_old;

enum dc_packet_length {
 LEN_DC_V11_IND_HDR =   (sizeof(RCM_IND_HEADER) + sizeof(BYTE)),
 LEN_DC_V10 =           (sizeof(sit_pdp_data_call_item_old)),
 LEN_DC_V11 =           (sizeof(sit_pdp_data_call_item)),
 LEN_DC_V11_PCSCF_EXT = (LEN_DC_V11 + sizeof(sit_pdp_data_call_item_pcscf_ext)),
 LEN_DC_V11_LP_RETRY =  (LEN_DC_V11_PCSCF_EXT + sizeof(sit_pdp_data_call_item_ext_lp_retrytime)),
 LEN_DC_V11_AMBR      = (LEN_DC_V11_LP_RETRY + sizeof(sit_pdp_data_call_item_ext_ambr)),
};

typedef struct
{
    RCM_HEADER hdr;
    BYTE cid;
    BYTE rat;
    BYTE data_profile;
    BYTE apn_type;
    char apn[MAX_PDP_APN_LEN];
    char username[MAX_PDP_USER_NAME_LEN];
    char password[MAX_PDP_USER_NAME_LEN];
    BYTE auth_type;
    BYTE pdp_type;
    BYTE pcscf_addr_req;
    BYTE IPv4Address[MAX_IPV4_ADDR_LEN];
    BYTE IPv6Address[MAX_IPV6_ADDR_LEN];
}sit_pdp_setup_data_call_req;

typedef struct
{
    RCM_HEADER hdr;
    sit_pdp_data_call_item data_call;
}sit_pdp_setup_data_call_rsp;

typedef struct
{
    RCM_HEADER hdr;
    sit_pdp_data_call_item_old data_call;
}sit_pdp_setup_data_call_rsp_old;

typedef enum
{
    NO_NEXT_HEADER = 0,
    EXT_PCSCF = 1,
    EXT_LP_RETRYTIME = 2,
} data_call_extension_header_type;


typedef enum
{
    SIT_RAT_TYPE_UNKNOWN = 0x00,
    SIT_RAT_TYPE_GPRS, // 0x01 (01)
    SIT_RAT_TYPE_EDGE,
    SIT_RAT_TYPE_UMTS,
    SIT_RAT_TYPE_IS95A,
    SIT_RAT_TYPE_IS95B, // 0x05 (05)
    SIT_RAT_TYPE_1xRTT,
    SIT_RAT_TYPE_EVDO_0,
    SIT_RAT_TYPE_EVDO_A,
    SIT_RAT_TYPE_HSDPA,
    SIT_RAT_TYPE_HSUPA, // 0x0A (10)
    SIT_RAT_TYPE_HSPA,
    SIT_RAT_TYPE_EVDO_B,
    SIT_RAT_TYPE_EHRPD,
    SIT_RAT_TYPE_LTE,
    SIT_RAT_TYPE_HSPAP,
    SIT_RAT_TYPE_GSM,           // 0x10 (16)
    SIT_RAT_TYPE_IWLAN,         // 0x11 (17)
    SIT_RAT_TYPE_TD_SCDMA,      // 0x12 (18)
    SIT_RAT_TYPE_HSPADCPLUS,    // 0x13 (19)

    SIT_RAT_TYPE_LTE_CA,        // 0x14 (20)
    SIT_RAT_TYPE_5G,            // 0x15 (21)
    SIT_RAT_TYPE_MAX,
    SIT_RAT_TYPE_UNSPECIFIED = 0xFF,
}sit_rat_type_e_type;

typedef enum
{
    SIT_PDP_DATA_PROFILE_DEFAULT = 0x00,
    SIT_PDP_DATA_PROFILE_TETHERED,
    SIT_PDP_DATA_PROFILE_OEM_BASE,

    SIT_PDP_DATA_PROFILE_MAX
}sit_pdp_data_profile_e_type;

typedef enum
{
    SIT_PDP_APN_TYPE_DEFAULT = 0x00,
    SIT_PDP_APN_TYPE_IMS,
    SIT_PDP_APN_TYPE_EMERGENCY,
    SIT_PDP_APN_TYPE_EMBMS,

    SIT_PDP_APN_TYPE_MAX
}sit_pdp_apn_type_e_type;

typedef enum
{
    SIT_PDP_AUTH_TYPE_NONE = 0x00,
    SIT_PDP_AUTH_TYPE_PAP,
    SIT_PDP_AUTH_TYPE_CHAP,
    SIT_PDP_AUTH_TYPE_PAP_CHAP,

    SIT_PDP_AUTH_TYPE_MAX
}sit_pdp_auth_type_e_type;

typedef enum
{
    SIT_PDP_PDP_TYPE_UNKNOWN = 0x00,
    SIT_PDP_PDP_TYPE_IPV4 = 0x01,
    SIT_PDP_PDP_TYPE_IPV6 = 0x02,
    SIT_PDP_PDP_TYPE_IPV4IPV6 = 0x03,
    SIT_PDP_PDP_TYPE_PPP,

    SIT_PDP_PDP_TYPE_MAX
}sit_pdp_pdp_type_e_type;

typedef enum
{
    SIT_PDP_PCSCF_REQ_NONE = 0x00,
    SIT_PDP_PCSCF_REQ_IPV4,
    SIT_PDP_PCSCF_REQ_IPV6,
    SIT_PDP_PCSCF_REQ_IPV4V6,

    SIT_PDP_PCSCF_REQ_MAX
}sit_pdp_pcscf_req_e_type;

typedef enum
{
    SIT_PDP_STATUS_FAIL_NONE = 0x00,
    SIT_PDP_STATUS_FAIL_OPERATOR_BARRED = 0x08,
    SIT_PDP_STATUS_FAIL_INSUFFICIENT_RESOURCES = 0x1A,
    SIT_PDP_STATUS_FAIL_MISSING_UKNOWN_APN = 0x1B,
    SIT_PDP_STATUS_FAIL_UNKNOWN_PDP_ADDRESS_TYPE = 0x1C,
    SIT_PDP_STATUS_FAIL_USER_AUTHENTICATION = 0x1D,
    SIT_PDP_STATUS_FAIL_ACTIVATION_REJECT_GGSN = 0x1E,
    SIT_PDP_STATUS_FAIL_ACTIVATION_REJECT_UNSPECIFIED = 0x1F,
    SIT_PDP_STATUS_FAIL_SERVICE_OPTION_NOT_SUPPORTED = 0x20,
    SIT_PDP_STATUS_FAIL_SERVICE_OPTION_NOT_SUBSCRIBED = 0x21,
    SIT_PDP_STATUS_FAIL_SERVICE_OPTION_OUT_OF_ORDER = 0x22,
    SIT_PDP_STATUS_FAIL_NSAPI_IN_USE = 0x23,
    SIT_PDP_STATUS_FAIL_ONLY_IPV4_ALLOWED = 0x32,
    SIT_PDP_STATUS_FAIL_ONLY_IPV6_ALLOWED = 0x33,
    SIT_PDP_STATUS_FAIL_ONLY_SINGLE_BEARER_ALLOWED = 0x34,
    SIT_PDP_STATUS_FAIL_PROTOCOL_ERRORS = 0x6F,

    SIT_PDP_STATUS_FAIL_MAX,
}sit_pdp_status_e_type;

typedef enum
{
    SIT_PDP_ACTIVE_INACTIVE = 0x00,
    SIT_PDP_ACTIVE_ACTIVE,

    SIT_PDP_ACTIVE_MAX
}sit_pdp_active_e_type;

/*
    SIT_DEACT_DATA_CALL (RCM ID = 0x0601)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE cid;
    BYTE deact_reason;
}sit_pdp_deact_data_call_req;

typedef enum
{
    SIT_PDP_DEACT_REASON_DEFAULT = 0x00,
    SIT_PDP_DEACT_REASON_RADIO_OFF,
    SIT_PDP_DEACT_REASON_PDP_RESET,

    SIT_PDP_DEACT_REASON_MAX
}sit_pdp_deact_reason_e_type;

typedef null_data_format sit_pdp_deact_data_call_rsp;

/*
    SIT_GET_DATA_CALL_LIST (RCM ID = 0x0602)
*/

typedef null_data_format sit_pdp_get_data_call_list_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE datacall_info_num;
    sit_pdp_data_call_item data_call[MAX_CID_NUM_LEN];
}sit_pdp_get_data_call_list_rsp;

typedef struct
{
    RCM_HEADER hdr;
    BYTE datacall_info_num;
    sit_pdp_data_call_item_old data_call[MAX_CID_NUM_LEN];
}sit_pdp_get_data_call_list_rsp_old;

/*
    SIT_SET_INITIAL_ATTACH_APN (RCM ID = 0x0603)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE cid;
    BYTE rat;
    BYTE data_profile;
    BYTE apn_type;
    char apn[MAX_PDP_APN_LEN];
    char username[MAX_PDP_USER_NAME_LEN];
    char password[MAX_PDP_USER_NAME_LEN];
    BYTE auth_type;
    BYTE pdp_type;
    BYTE pcscf_addr_req;
    BYTE IPv4Address[MAX_IPV4_ADDR_LEN];
    BYTE IPv6Address[MAX_IPV6_ADDR_LEN];
    BYTE profile_id;
    BYTE apn_disable_flag;
    UINT16 max_pdn_conn_per_block;
    UINT16 max_pdn_conn_timer;
    UINT16 pdn_req_wait_interval;
    BYTE roaming_pdp_type;
    BYTE roaming_pcscf_req_type;
}sit_pdp_set_initial_attach_apn_req;

typedef enum
{
    SIT_IA_PROFILE_ID_UNKNOWN = 0x00,
    SIT_IA_PROFILE_ID_VZWIMS = 0x01,
    SIT_IA_PROFILE_ID_VZWFOTA = 0x02,
    SIT_IA_PROFILE_ID_VZWDEFAULT = 0x03,
    SIT_IA_PROFILE_ID_VZWCBS = 0x04,
    SIT_IA_PROFILE_ID_VZWCAS = 0x05,
    SIT_IA_PROFILE_ID_VZWSUPL = 0x06,
    SIT_IA_PROFILE_ID_VZWMMS = 0x07,
    SIT_IA_PROFILE_ID_VZWTETHERED = 0x08,
    SIT_IA_PROFILE_ID_VZWE911 = 0x09,
    SIT_IA_PROFILE_ID_VZWEMBMS = 0x0A,
    SIT_IA_PROFILE_ID_VZWBIP = 0x0B,
    SIT_IA_PROFILE_ID_INTERNET = 0xFF,
}sit_initial_attach_profile_id_e_type;

typedef null_data_format sit_pdp_set_initial_attach_apn_rsp;

/*
    SIT_IND_DATA_CALL_LIST_CHANGED (RCM ID = 0x0604)
    This format will be still used for extension representation
    So if we need to process extension block,
    It's required to calculate index of each data_call item block.
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE datacall_info_num;
    sit_pdp_data_call_item data_call[MAX_CID_NUM_LEN];
}sit_pdp_data_call_list_changed_ind;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE datacall_info_num;
    sit_pdp_data_call_item_old data_call[MAX_CID_NUM_LEN];
}sit_pdp_data_call_list_changed_ind_old;

/*
    SIT_SET_FD_INFO (RCM ID = 0x0605)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE lcd_on;            // Set timer value for lcd on state
    BYTE lcd_off;            // Set timer value for lcd off state
    BYTE rel8_lcd_on;        // Set timer value for lcd on state when network support REL 8
    BYTE rel8_lcd_off;        // Set timer value for lcd off state when network support REL 8
}sit_pdp_set_fd_info_req;

typedef null_data_format sit_pdp_set_fd_info_rsp;

/*
    SIT_IND_DEDICATED_BEARER_INFO (RCM ID = 0x0606)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE status;
    BYTE type;
    BYTE qci;
    INT32 dl_gbr;
    INT32 ul_gbr;
    INT32 dl_max_gbr;
    INT32 ul_max_gbr;
}sit_pdp_dedicated_bearer_info_ind;

/*
   SIT_IND_NAS_TIMER_STATUS (RCM ID = 0x0607)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE timer_type;
    BYTE timer_status;
    INT32 timer_value;
    char apn[MAX_PDP_APN_LEN];
}sit_pdp_nas_timer_status_ind;

/*
   SIT_DETACH (RCM ID = 0x0608)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE detach_reason;
}sit_pdp_detach_req;

/*
    SIT_IND_DATA_STATE_CHANGE (RCM ID = 0x0609)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE expected_state;
}sit_pdp_data_state_change_ind;

/*
   SIT_START_KEEPALIVE (RCM ID = 0x060A)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE keepalive_type;    // same to RIL_KeepaliveType
    BYTE src_addr[MAX_IPV6_ADDR_LEN];
    INT32 source_port;
    BYTE dst_addr[MAX_IPV6_ADDR_LEN];
    INT32 dst_port;
    INT32 max_interval;
    INT32 cid;
}sit_pdp_start_keepalive_req;

typedef struct
{
    RCM_HEADER hdr;
    UINT32 session_handle;
    INT32 status_code;    // same to RIL_KeepaliveStatusCode
}sit_pdp_start_keepalive_rsp;

/*
   SIT_STOP_KEEPALIVE (RCM ID = 0x060B)
*/
typedef struct
{
    RCM_HEADER hdr;
    UINT32 keepalive_handle;
}sit_pdp_stop_keepalive_req;

/*
    SIT_IND_KEEPALIVE_STATUS (RCM ID = 0x060C)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    UINT32 session_handle;
    INT32 status_code;
}sit_pdp_keepalive_status_ind;

/*
    SIT_IND_PCO_DATA (RCM ID = 0x060D)
*/
typedef struct
{
    INT32 pco_id;
    BYTE contents_len; // This will be embedded in contents, first byes is length
    char *contents; // This will be increased from modemData by 1byte which is length field
} sit_pdp_pco_data_entry;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE cid;
    BYTE pdp_type;
    BYTE pco_num;
}sit_pdp_pco_data_ind;

/*
    SIT_START_LCE_INFO (RCM ID = 0x060E)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 interval;
    BYTE mode;    // 0: push, 1: pull  which is same to LceServiceMode
}sit_pdp_start_lce_info_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE status;    // 0: stopped, 1: active, 2: not supported
    INT32 interval;
}sit_pdp_start_lce_info_rsp;

/*
    SIT_STOP_LCE_INFO (RCM ID = 0x060F)
*/
typedef null_data_format sit_pdp_stop_lce_info_req;
typedef struct
{
    RCM_HEADER hdr;
    BYTE status;    // 0: stopped, 1: active, 2: not supported
    INT32 interval;
}sit_pdp_stop_lce_info_rsp;

/*
    SIT_GET_LCE_DATA (RCM ID = 0x0610)
*/
typedef null_data_format sit_pdp_get_lce_data_req;
typedef struct
{
    RCM_HEADER hdr;
    INT32 dl_lc;
    INT32 ul_lc;
    BYTE conf_lvl;
    BYTE is_suspended;    // 0: not suspended, 1: suspended
}sit_pdp_get_lce_data_rsp;

/*
    SIT_IND_LCE_DATA (RCM ID = 0x0611)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    INT32 dl_lc;
    INT32 ul_lc;
    BYTE conf_lvl;
    BYTE is_suspended;    // 0: not suspended, 1: suspended
}sit_pdp_lce_data_ind;

/*
    SIT_SET_DATA_PROFILE (RCM ID = 0x0613)
*/
typedef struct
{
    RCM_HEADER hdr;
    UINT32 profile_id;
    char apn[MAX_PDP_APN_LEN];
    BYTE pdp_type;
    BYTE roaming_pdp_type;
    BYTE auth_type;
    char username[MAX_PDP_USER_NAME_LEN];
    char password[MAX_PDP_USER_NAME_LEN];
    BYTE data_profile_info_type;
    UINT32 max_conns_time;
    UINT32 max_conns;
    UINT32 wait_time;
    BYTE enabled;
    UINT32 apn_type;
    UINT32 radio_access_family;
    UINT32 mtu;
}sit_pdp_set_data_profile_req;

/*
    SIT_SET_IMS_TEST_MODE (RCM ID = 0x0614)
*/

typedef struct
{
    RCM_HEADER hdr;
    BYTE mode;
} sit_pdp_set_ims_test_mode_req;

/*
    SIT_GET_PHONE_CAPABILITY (RCM ID = 0x0615)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE max_simultaneous_data_stack;
    BYTE max_simultaneous_internet_pdn;
    BYTE internet_lingering_support;
    BYTE max_supported_stack;
} sit_pdp_get_phone_capability_rsp;

/*
    SIT_GET_CS_REG_STATE (RCM ID = 0x0700)
*/
typedef null_data_format sit_net_get_cs_reg_state_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE reg_state;
    BYTE rej_cause;    // only available when reg_state == denied (0x03)
    BYTE rat;
    INT32 gw_lac;
    INT32 gw_cid;
    BYTE gw_psc;
    INT32 lte_tac;
    INT32 lte_pcid;
    INT32 lte_eci;
    INT32 lte_csgid;
    INT32 lte_tadv;
#ifdef SUPPORT_CDMA
    INT32 basestationid;
    INT32 basestation_latitude;
    INT32 basestation_longitude;
    BYTE concurrent;
    INT16 sid;
    INT16 nid;
    BYTE roamingindicator;
    BYTE registered_prl;
    BYTE roaming_indi_prl;
#endif
}sit_net_get_cs_reg_state_rsp;

// a data structure to support radio@1.0
typedef struct {
    BYTE reg_state;
    BYTE rej_cause;    // only available when reg_state == denied (0x03)
    BYTE rat;
    INT32 gw_lac;
    INT32 gw_cid;
    BYTE gw_psc;
    INT32 lte_tac;
    INT32 lte_pcid;
    INT32 lte_eci;
    INT32 lte_csgid;
    INT32 lte_tadv;
    INT32 basestationid;
    INT32 basestation_latitude;
    INT32 basestation_longitude;
    BYTE concurrent;
    INT16 sid;
    INT16 nid;
    BYTE roamingindicator;
    BYTE registered_prl;
    BYTE roaming_indi_prl;
    INT32 channel;         // arfcn, uarfcn, earfcn
} sit_cs_reg_state_v1_0;


typedef enum
{
    SIT_NET_REG_STATE_NOT_REG_NO_SEARCH = 0x00,
    SIT_NET_REG_STATE_REGISTERED,
    SIT_NET_REG_STATE_NOT_REG_SEARCHING,
    SIT_NET_REG_STATE_DENIED,
    SIT_NET_REG_STATE_UNKNOWN,
    SIT_NET_REG_STATE_ROAMING,

    SIT_NET_REG_STATE_NOT_REG_NO_SEARCH_EMGCALL = 0x0A,
    SIT_NET_REG_STATE_NOT_REG_SEARCHING_EMGCALL = 0x0C,
    SIT_NET_REG_STATE_DENIED_EMGCALL = 0x0D,
    SIT_NET_REG_STATE_UNKNOWN_EMGCALL = 0x0E,

    //------------------------------------------
    SIT_NET_REG_STATE_DENIED_ROAMING = 50,

    SIT_NET_REG_STATE_MAX
}sit_net_reg_state_e_type;

typedef enum
{
    SIT_NET_REJ_CAUSE_GENERAL = 0,
    SIT_NET_REJ_CAUSE_AUTH_FAIL,
    SIT_NET_REJ_CAUSE_IMSI_UNKNOWN_IN_HLR,
    SIT_NET_REJ_CAUSE_ILLEGAL_MS,
    SIT_NET_REJ_CAUSE_ILLEGAL_ME,
    SIT_NET_REJ_CAUSE_PLMN_NOT_ALLOWED,
    SIT_NET_REJ_CAUSE_LOCATION_AREA_NOT_ALLOWED,
    SIT_NET_REJ_CAUSE_ROAMING_NOT_ALLOWED,
    SIT_NET_REJ_CAUSE_NO_SUITABLE_CELLS_IN_THIS_LOCATION_AREA,
    SIT_NET_REJ_CAUSE_NETWORK_FAIL,

    ///TODO:
    /*
    SIT_NET_REJ_CAUSE_PLMN_NOT_ALLOWED,
    ... will be added after duplicated value is revised
    */

}sit_net_rej_cause_e_type;

/*
    SIT_GET_PS_REG_STATE (RCM ID = 0x0701)
*/
typedef null_data_format sit_net_get_ps_reg_state_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE reg_state;
    BYTE rej_cause;    // only available when reg_state == denied (0x03)
    BYTE max_sdc;
    BYTE rat;
    INT32 gw_lac;
    INT32 gw_cid;
    BYTE gw_psc;
    INT32 lte_tac;
    INT32 lte_pcid;
    INT32 lte_eci;
    INT32 lte_csgid;
    INT32 lte_tadv;
    BYTE volte_service;     // 0x0 : Not available 0x01 : available
    BYTE emc_service;       // 0x0 : Not available 0x01 : available
}sit_net_get_ps_reg_state_rsp;

// a data structure to support radio@1.0
typedef struct {
    BYTE reg_state;
    BYTE rej_cause;    // only available when reg_state == denied (0x03)
    BYTE max_sdc;
    BYTE rat;
    INT32 gw_lac;
    INT32 gw_cid;
    BYTE gw_psc;
    INT32 lte_tac;
    INT32 lte_pcid;
    INT32 lte_eci;
    INT32 lte_csgid;
    INT32 lte_tadv;
    BYTE volte_service;     // 0x0 : Not available 0x01 : available
    BYTE emc_service;       // 0x0 : Not available 0x01 : available
    INT32 channel;         // arfcn, uarfcn, earfcn
} sit_ps_reg_state_v1_0;

// a data structure to support NR
typedef struct {
    BYTE reg_state;
    BYTE rej_cause;    // only available when reg_state == denied (0x03)
    BYTE max_sdc;
    BYTE rat;
    INT32 gw_lac;
    INT32 gw_cid;
    BYTE gw_psc;
    INT32 lte_tac;
    INT32 lte_pcid;
    INT32 lte_eci;
    INT32 lte_csgid;
    INT32 lte_tadv;
    BYTE volte_service;     // 0x0 : Not available 0x01 : available
    BYTE emc_service;       // 0x0 : Not available 0x01 : available
    INT32 channel;         // arfcn, uarfcn, earfcn
    BYTE endc;             // 0x00 : Not available, 0x01: available
    BYTE dcnr_restricted;  // 0x00 : Not restrict,  0x01: restrict
    BYTE nr_available;     // 0x00 : Not available, 0x01: available
} sit_ps_reg_state_v1_1;

typedef enum {
    SERVICE_NOT_AVAILABLE = 0x00,
    SERVICE_AVAILABLE = 0x01,
} sit_net_service_state_e_type;

/*
    SIT_GET_OPERATOR (RCM ID = 0x0702)
*/
typedef null_data_format sit_net_get_operator_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE plmn[MAX_PLMN_LEN];
    char short_name[MAX_SHORT_NAME_LEN];
    char long_name[MAX_FULL_NAME_LEN];
}sit_net_get_operator_rsp;

typedef struct
{
    BYTE plmn[MAX_PLMN_LEN];
    char short_name[MAX_SHORT_NAME_LEN];
    char long_name[MAX_FULL_NAME_LEN];
    BYTE reg_state;
}sit_net_operator_v1_0;

typedef struct
{
    BYTE plmn[MAX_PLMN_LEN];
    char short_name[MAX_SHORT_NAME_LEN];
    char long_name[MAX_FULL_NAME_LEN];
    BYTE reg_state;
    INT32 lac;
}sit_net_operator_v1_1;

/*
    SIT_GET_NTW_MODE (RCM ID = 0x0703)
*/
typedef null_data_format sit_net_get_network_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 network_mode;
}sit_net_get_network_mode_rsp;

typedef enum
{
    SIT_NET_NETWORK_MODE_AUTOMATIC = 0x00,
    SIT_NET_NETWORK_MODE_MANUAL,

    SIT_NET_NETWORK_MODE_MAX
}sit_net_network_mode_e_type;

/*
    SIT_SET_NTW_MODE_AUTO (RCM ID = 0x0704)
*/
typedef null_data_format sit_net_set_network_mode_auto_req;

typedef null_data_format sit_net_set_network_mode_auto_rsp;

/*
    SIT_SET_NTW_MODE_MANUAL (RCM ID = 0x0705)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 rat;
    BYTE plmn[MAX_PLMN_LEN];
}sit_net_set_metwork_mode_manual_req;

typedef null_data_format sit_net_set_metwork_mode_manual_rsp;

/*
    SIT_GET_AVAILABLE_NETWORKS (RCM ID = 0x0706)
*/
typedef null_data_format sit_net_get_available_networks_req;

typedef struct {
    RCM_HEADER hdr;
    int rat;    // 0x00: all 0x01: GERAN 0x02: UTRAN 0x03: EUTRAN
} sit_net_get_available_networks_wit_rat_req;

typedef enum
{
    SIT_NET_PLMN_STATUS_UNKONWN = 0x00,
    SIT_NET_PLMN_STATUS_AVAILABLE = 0x01,
    SIT_NET_PLMN_STATUS_CURRENT = 0x02,
    SIT_NET_PLMN_STATUS_FORBIDDEN = 0x03,
    SIT_NET_PLMN_STATUS_MAX
}sit_net_plmn_status_e_type;

typedef struct
{
    INT32 rat;
    BYTE plmn[MAX_PLMN_LEN];
    INT32 plmn_status;
}sit_net_network_info_item;

typedef struct
{
    RCM_HEADER hdr;
    INT32 network_info_num;
    sit_net_network_info_item network_info[MAX_NET_INFO_COUNT];
}sit_net_get_available_networks_rsp;

/*
    SIT_CANCEL_GET_AVAILABLE_NETWORKS (RCM ID = 0x0707)
*/
typedef null_data_format sit_net_cancel_get_available_networks_req;

typedef null_data_format sit_net_cancel_get_available_networks_rsp;

/*
    SIT_SET_BAND_MODE (RCM ID = 0x0708)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 band;
}sit_net_set_band_mode_req;

typedef null_data_format sit_net_set_band_mode_rsp;

typedef enum
{
    SIT_NET_BAND_UNSPECIFIED = 0x00,
    SIT_NET_BAND_EURO,
    SIT_NET_BAND_US,
    SIT_NET_BAND_JPN,
    SIT_NET_BAND_AUS,
    SIT_NET_BAND_AUS2,
    SIT_NET_BAND_CELLULAR,
    SIT_NET_BAND_PCS,
    SIT_NET_BAND_CLASS3,
    SIT_NET_BAND_CLASS4,
    SIT_NET_BAND_CLASS5,
    SIT_NET_BAND_CLASS6,
    SIT_NET_BAND_CLASS7,
    SIT_NET_BAND_CLASS8,
    SIT_NET_BAND_CLASS9,
    SIT_NET_BAND_CLASS10,
    SIT_NET_BAND_CLASS11,
    SIT_NET_BAND_CLASS15,
    SIT_NET_BAND_CLASS16,

    SIT_NET_BAND_MAX
}sit_net_band_e_type;

/*
    SIT_GET_BAND_MODE (RCM ID = 0x0709)
*/
typedef null_data_format sit_net_get_band_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 band_info_num;
    INT32 band[MAX_NET_INFO_COUNT];
}sit_net_get_band_mode_rsp;


/*
    SIT_SET_PREFERRED_NTW_TYPE (RCM ID = 0x070A)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 pref_net_type;
}sit_net_set_pref_network_req;

/*
    SIT_GET_PREFERRED_NTW_TYPE (RCM ID = 0x070B)
*/
typedef null_data_format sit_net_get_pref_network_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 pref_net_type;
}sit_net_get_pref_network_rsp;

/*
    SIT_GET_CELL_INFO_LIST (RCM ID = 0x070C)
*/
typedef null_data_format sit_net_get_cell_info_list_req;

typedef struct
{
    BYTE plmn[MAX_PLMN_LEN];
    INT32 lac;
    INT32 cid;
    INT32 sig_str;
    INT32 sig_ber;
}cell_info_gsm;

typedef struct
{
    char plmn[MAX_PLMN_LEN];
    INT32 lac;
    INT32 cid;
    INT32 arfcn;
    BYTE bsic;
    INT32 sig_str;
    INT32 sig_ber;
    BYTE sig_ta;
}cell_info_gsm_v12;

typedef struct
{
    INT32 ntw_id;
    INT32 sys_id;
    INT32 bs_id;
    INT32 longitude;
    INT32 lat;
    INT32 sig_dbm;
    INT32 sig_ecio;
    INT32 sig_snr;
    INT32 evdo_sig_dbm;
    INT32 evdo_sig_ecio;
    INT32 evdo_sig_snr;
}cell_info_cdma;

typedef struct
{
    INT32 ntw_id;
    INT32 sys_id;
    INT32 bs_id;
    INT32 longitude;
    INT32 lat;
    INT32 sig_dbm;
    INT32 sig_ecio;
    INT32 evdo_sig_dbm;
    INT32 evdo_sig_ecio;
    INT32 evdo_sig_snr;
}cell_info_cdma_v14;

typedef struct
{
    char plmn[MAX_PLMN_LEN];
    INT32 cell_id;
    INT32 phy_cell_id;
    INT32 tac;
    INT32 sig_str;
    INT32 sig_rsrp;
    INT32 sig_rsrq;
    INT32 sig_rssnr;
    INT32 sig_cqi;
    INT32 ta;
}cell_info_lte;

typedef struct
{
    char plmn[MAX_PLMN_LEN];
    INT32 cell_id;
    INT32 phy_cell_id;
    INT32 tac;
    INT32 earfcn;
    INT32 sig_str;
    INT32 sig_rsrp;
    INT32 sig_rsrq;
    INT32 sig_rssnr;
    INT32 sig_cqi;
    INT32 ta;
}cell_info_lte_v12;

typedef struct
{
    char plmn[MAX_PLMN_LEN];
    INT32 cell_id;
    INT32 phy_cell_id;
    INT32 tac;
    INT32 earfcn;
    INT32 bandwidth;
    INT32 endc_available;
    INT32 sig_str;
    INT32 sig_rsrp;
    INT32 sig_rsrq;
    INT32 sig_rssnr;
    INT32 sig_cqi;
    INT32 ta;
}cell_info_lte_v14;

typedef struct
{
    char plmn[MAX_PLMN_LEN];
    INT32 lac;
    INT32 cid;
    INT32 psc;
    INT32 sig_str;
    INT32 sig_ber;
}cell_info_wcdma;

typedef struct
{
    char plmn[MAX_PLMN_LEN];
    INT32 lac;
    INT32 cid;
    INT32 psc;
    INT32 uarfcn;
    INT32 sig_str;
    INT32 sig_ber;
}cell_info_wcdma_v12;

typedef struct
{
    char plmn[MAX_PLMN_LEN];
    INT32 lac;
    INT32 cid;
    INT32 psc;
    INT32 uarfcn;
    INT32 sig_str;
    INT32 sig_ber;
    INT32 rscp;
    INT32 ecno;
}cell_info_wcdma_v14;

typedef struct
{
    char plmn[MAX_PLMN_LEN];
    INT32 lac;
    INT32 cid;
    INT32 cpid;
    INT32 rscp;
}cell_info_tdscdma;

typedef struct
{
    char plmn[MAX_PLMN_LEN];
    INT32 lac;
    INT32 cid;
    INT32 cpid;
    INT32 sig_str;
    INT32 ber;
    INT32 rscp;
}cell_info_tdscdma_v14;

typedef struct
{
    char plmn[MAX_PLMN_LEN];
    ULONG cell_id;
    UINT32 phy_cell_id;
    INT32 tac;
    INT32 arfcn;
    INT32 ss_rsrp;
    INT32 ss_rsrq;
    INT32 ss_sinr;
    INT32 csi_rsrp;
    INT32 csi_rsrq;
    INT32 csi_sinr;
}cell_info_nr;

typedef struct
{
    BYTE cell_info_type;
    BYTE reg_status;
    union
    {
        cell_info_gsm gsm;
        cell_info_cdma cdma;
        cell_info_lte lte;
        cell_info_wcdma wcdma;
        cell_info_tdscdma tdscdma;
    }cell_info;
}sit_net_cell_info_item;

typedef struct
{
    BYTE cell_info_type;
    BYTE reg_status;
    union
    {
        cell_info_gsm_v12 gsm;
        cell_info_cdma cdma;
        cell_info_lte_v12 lte;
        cell_info_wcdma_v12 wcdma;
        cell_info_tdscdma tdscdma;
    }cell_info;
}sit_net_cell_info_item_v12;

typedef struct
{
    BYTE cell_info_type;
    BYTE reg_status;
    BYTE cell_connection_status;
    union
    {
        cell_info_gsm_v12 gsm;
        cell_info_cdma_v14 cdma;
        cell_info_lte_v14 lte;
        cell_info_wcdma_v14 wcdma;
        cell_info_tdscdma_v14 tdscdma;
        cell_info_nr nr;
    }cell_info;
}sit_net_cell_info_item_v14;

typedef struct
{
    RCM_HEADER hdr;
    INT32 cell_info_num;
    char cell_info_list[0];
}sit_net_get_cell_info_list_rsp;

typedef enum {
    SIT_NET_CELL_INFO_TYPE_GSM = 0x00,
    SIT_NET_CELL_INFO_TYPE_CDMA = 0x01,
    SIT_NET_CELL_INFO_TYPE_LTE = 0x02,
    SIT_NET_CELL_INFO_TYPE_WCDMA = 0x03,
    SIT_NET_CELL_INFO_TYPE_TDSCDMA = 0x04,
}sit_net_cell_info_type_e_type;

typedef enum {
    SIT_NET_UNREGISTERED = 0x00,
    SIT_NET_REGISTERED = 0x01,
}sit_net_reg_status_e_type;


/*
    SIT_SET_CELL_INFO_LIST_REPORT_RATE (RCM ID = 0x070D)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 report_rate;
}sit_net_cell_info_list_report_rate_req;

typedef null_data_format sit_net_cell_info_list_report_rate_rsp;


/*
    SIT_IND_NTW_STATE_CHANGED (RCM ID = 0x070E)
*/
typedef null_ind_data_format sit_net_network_state_changed_ind;

/*
    SIT_IND_CELL_INFO_LIST (RCM ID = 0x070F)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    INT32 cell_info_num;
    char cell_info_list[0];
}sit_net_cell_info_list_ind;

/*
    SIT_SET_PS_SERVICE (RCM ID = 0x0710)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE state;
}sit_net_set_ps_service_req;
typedef null_data_format sit_net_set_ps_service_rsp;

/*
    SIT_GET_PS_SERVICE (RCM ID = 0x0711)
*/
typedef null_data_format sit_net_get_ps_service_req;
typedef struct
{
    RCM_HEADER hdr;
    BYTE state;
}sit_net_get_ps_service_rsp;

/*
   SIT_SET_EMGERGENCY_CALL_STATUS (RCM ID = 0x0712)
 */
typedef struct
{
    RCM_HEADER hdr;
    BYTE status;
    BYTE rat;
}sit_net_set_emergency_call_status_req;
typedef null_data_format sit_net_set_emergency_call_status_rsp;

/*
   SIT_IND_EMGERGENCY_ACT_INFO (RCM ID = 0x0713)
 */
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE rat;
    BYTE act_status;
}sit_net_emergency_act_info_ind;

// Emergency Call Status
enum {
    SIT_NET_EMERGENCY_CALL_START        = 0x01,
    SIT_NET_EMERGENCY_CALL_END          = 0x02,
    SIT_NET_EMERGENCY_CALL_CANCELED     = 0x03,
    SIT_NET_EMERGENCY_CALL_FAIL         = 0x04,
};

enum {
    SIT_NET_NO_MORE_ACT_EMERGENCY_CALL  = 0x00,
    SIT_NET_CURRENT_ACT_EMERGENCY_CALL  = 0x01,
    SIT_NET_RETRY_ACT_EMERGENCY_CALL    = 0x02,
};

/*
    SIT_SET_UPLMN (RCM ID = 0x0714)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE mode;
    BYTE index;
    BYTE plmn[MAX_PLMN_LEN];
    BYTE act;
}sit_net_set_uplmn_req;
typedef null_data_format sit_net_set_uplmn_rsp;

/*
    SIT_GET_UPLMN (RCM ID = 0x0715)
*/
typedef null_data_format sit_net_get_uplmn_req;

typedef struct
{
    BYTE index;
    char plmn[MAX_PLMN_LEN];
    BYTE act;
}preferred_plmn_item;

typedef struct
{
    RCM_HEADER hdr;
    INT32 plmn_list_num;
    preferred_plmn_item preffered_plmn[0];
}sit_net_get_uplmn_rsp;

typedef enum
{
    SIT_NET_PREFERRED_PLMN_MODE_INVALID = 0x00,
    SIT_NET_PREFERRED_PLMN_MODE_ADD = 0x01,
    SIT_NET_PREFERRED_PLMN_MODE_EDIT = 0x02,
    SIT_NET_PREFERRED_PLMN_MODE_DELETE = 0x03,
}sit_net_preferred_plmn_mode_e_type;

typedef enum
{
    SIT_NET_PREFERRED_PLMN_ACT_UNKNOWN = 0x00,
    SIT_NET_PREFERRED_PLMN_ACT_GSM = 0x01,
    SIT_NET_PREFERRED_PLMN_ACT_GSM_COMPACT = 0x02,
    SIT_NET_PREFERRED_PLMN_ACT_UTRAN = 0x03,
    SIT_NET_PREFERRED_PLMN_ACT_EUTRAN = 0x04,
    SIT_NET_PREFERRED_PLMN_ACT_MAX,
}sit_net_preferred_plmn_act_e_type;

/*
    SIT_SET_DS_NTW_TYPE (RCM ID = 0x0716)
*/
typedef enum
{
    SIT_NET_DS_NET_TYPE_GSM_WCDMA               = 0x00,
    SIT_NET_DS_NET_TYPE_LTE_GSM_WCDMA           = 0x01,
    SIT_NET_DS_NET_TYPE_GSM                     = 0x02,
    SIT_NET_DS_NET_TYPE_CDMA_EVDO_AUTO          = 0x03,
    SIT_NET_DS_NET_TYPE_LTE_CDMA_EVDO           = 0x04,
    SIT_NET_DS_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA = 0x05,
    SIT_NET_DS_NET_TYPE_CDMA                    = 0x06,
    SIT_NET_DS_NET_TYPE_MAX
}sit_net_pref_ds_type_e_type;

/*
    SIT_GET_RADIO_CAPABILITY (RCM ID = 0x0718)
*/
typedef null_data_format sit_net_get_rc_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 version;
    INT32 session_id;
    INT32 phase;  // same to RadioCapabilityPhase
    INT32 rc_raf;
    BYTE uuid[SIT_MAX_UUID_LENGTH];
    INT32 status;  // same to RadioCapabilityStatus
}sit_net_get_rc_rsp;

/*
    SIT_SET_RADIO_CAPABILITY (RCM ID = 0x0719)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 version;
    INT32 session_id;
    INT32 phase;  // same to RadioCapabilityPhase
    INT32 rc_raf;
    BYTE uuid[SIT_MAX_UUID_LENGTH];
    INT32 status;  // same to RadioCapabilityStatus
}sit_net_set_rc_req;

typedef null_data_format sit_net_set_rc_rsp;

/*
    SIT_SET_DUPLEX_MODE (RCM ID = 0x071B)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE duplex_mode_4g;
    BYTE duplex_mode_3g;
}sit_net_set_duplex_mode_req;

typedef null_data_format sit_net_set_duplex_mode_rsp;

typedef enum
{
    SIT_NET_DUPLEX_MODE_FDD = 0x01,
    SIT_NET_DUPLEX_MODE_TDD = 0x02,
    SIT_NET_DUPLEX_MODE_FDD_TDD = 0x03,
    SIT_NET_DUPLEX_MODE_MAX
}sit_net_duplex_mode_e_type;

/*
    SIT_GET_DUPLEX_MODE (RCM ID = 0x071A)
*/
typedef null_data_format sit_net_get_duplex_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE duplex_mode_4g;
    BYTE duplex_mode_3g;
}sit_net_get_duplex_mode_rsp;

/*
    SIT_SET_MICRO_CELL_SEARCH (RCM ID = 0x071F)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE srch_mode;
}sit_net_set_micro_cell_search_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE srch_result;
    BYTE plmn[MAX_PLMN_LEN];
}sit_net_set_micro_cell_search_rsp;

typedef enum
{
    SIT_MC_SRCH_MODE_MANUAL = 0,
    SIT_MC_SRCH_MODE_AUTO = 1,
    SIT_MC_SRCH_MODE_CANCEL = 2,
}sit_net_mc_srch_mode_type;

typedef enum
{
    SIT_MC_SRCH_RESUT_SUCCESS = 0,
    SIT_MC_SRCH_RESUT_NW_NOT_SUPPORT = 1,
    SIT_MC_SRCH_RESUT_MODE_NOT_AVAILABLE = 2,
    SIT_MC_SRCH_RESUT_NO_CELL_LIST = 3,
}sit_net_mc_srch_result_type;

/*
    SIT_SET_DUAL_NTW_AND_PS_TYPE (RCM ID = 0x72B)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 pref_net_type_for_primary;
    INT32 pref_net_type_for_secondary;
    INT32 allowed_for_primary;
    INT32 allowed_for_secondary;
}sit_net_set_dual_network_and_allow_data_req;

typedef null_data_format sit_net_set_pref_network_rsp;

typedef enum
{
    SIT_NET_PREF_NET_TYPE_GSM_WCDMA = 0x00,
    SIT_NET_PREF_NET_TYPE_GSM_ONLY,
    SIT_NET_PREF_NET_TYPE_WCDMA,
    SIT_NET_PREF_NET_TYPE_GSM_WCDMA_AUTO,
    SIT_NET_PREF_NET_TYPE_CDMA_EVDO_AUTO,
    SIT_NET_PREF_NET_TYPE_CDMA_ONLY,
    SIT_NET_PREF_NET_TYPE_EVDO_ONLY,
    SIT_NET_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO,
    SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO,
    SIT_NET_PREF_NET_TYPE_LTE_GSM_WCDMA,
    SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA,
    SIT_NET_PREF_NET_TYPE_LTE_ONLY,
    SIT_NET_PREF_NET_TYPE_LTE_WCDMA,    // 0x0C

    SIT_NET_PREF_NET_TYPE_NR_ONLY,      // 0x0D
    SIT_NET_PREF_NET_TYPE_NR_LTE,
    SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO,
    SIT_NET_PREF_NET_TYPE_NR_LTE_GSM_WCDMA,  // 0x10
    SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA,
    SIT_NET_PREF_NET_TYPE_NR_LTE_WCDMA,

    SIT_NET_PREF_NET_TYPE_NR_LTE_GSM, // 0x13
    SIT_NET_PREF_NET_TYPE_NR_WCDMA,
    SIT_NET_PREF_NET_TYPE_NR_GSM_WCDMA,
    SIT_NET_PREF_NET_TYPE_NR_GSM,

    SIT_NET_PREF_NET_TYPE_TDSCDMA_ONLY,                       /* TD-SCDMA only */
    SIT_NET_PREF_NET_TYPE_TDSCDMA_WCDMA,                      /* TD-SCDMA and WCDMA */
    SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA,                        /* TD-SCDMA and LTE */
    SIT_NET_PREF_NET_TYPE_TDSCDMA_GSM,                        /* TD-SCDMA and GSM */
    SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_GSM,                    /* TD-SCDMA,GSM and LTE */
    SIT_NET_PREF_NET_TYPE_TDSCDMA_GSM_WCDMA,                  /* TD-SCDMA, GSM/WCDMA */
    SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_WCDMA,                  /* TD-SCDMA, WCDMA and LTE */
    SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_GSM_WCDMA,              /* TD-SCDMA, GSM/WCDMA and LTE */
    SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA_EVDO_GSM_WCDMA,        /* TD-SCDMA,EvDo,CDMA,GSM/WCDMA*/
    SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA,    /* TD-SCDMA/LTE/GSM/WCDMA, CDMA, and EvDo */

    SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA, // 0x21
    SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM,
    SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA,
    SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA,
    SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA,

    SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA = 50,
    SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA_NO_EVDO,
    SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_CDMA_EVDO,
    SIT_NET_PREF_NET_TYPE_TDSCDMA_EVDO_NO_CDMA,

    SIT_NET_PREF_NET_TYPE_MAX
}sit_net_pref_network_type_e_type;

/*
    SIT_IND_TOTAL_OOS (RCM ID = 0x072C)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE pref_net_type;
}sit_net_total_oos_ind;

/*
    SIT_IND_MCC (RCM ID = 0x072D)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE pref_net_type;
    char mcc[3];
}sit_net_mcc_ind;

/*
    SIT_SET_CARRIER_RESTRICTIONS (RCM ID = 0x072E)
*/
typedef struct
{
    BYTE mcc[MAX_MCC_LEN];
    BYTE mnc[MAX_MNC_LEN];
    BYTE match_type; // same to RIL_CarrierMatchType
    BYTE match_len;
    BYTE match_data[MAX_CR_MATCH_DATA_SIZE];
}SIT_CARRIER_INFO;

typedef struct
{
    RCM_HEADER hdr;
    INT32 allowed_carriers_len;
    INT32 excluded_carriers_len;
    SIT_CARRIER_INFO allowed_carrier_list[MAX_CARRIER_INFO_NUM];
    SIT_CARRIER_INFO excluded_carrier_list[MAX_CARRIER_INFO_NUM];
}sit_net_set_carrier_restriction_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE allowed_carriers_len;
}sit_net_set_carrier_restriction_rsp;

/*
    SIT_GET_CARRIER_RESTRICTIONS (RCM ID = 0x072F)
*/
typedef null_data_format sit_net_get_carrier_restriction_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 allowed_carriers_len;
    INT32 excluded_carriers_len;
    SIT_CARRIER_INFO allowed_carrier_list[MAX_CR_MATCH_DATA_SIZE];
    SIT_CARRIER_INFO excluded_carrier_list[MAX_CR_MATCH_DATA_SIZE];
}sit_net_get_carrier_restriction_rsp;

/*
    SIT_GET_MANUAL_BAND_MODE (RCM ID = 0x0730)
*/
typedef null_data_format sit_misc_get_manual_band_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE mode_set_status;
    BYTE supported_lte[32];
    BYTE supported_wcdma[8];
    BYTE supported_gsm;
    BYTE supported_cdma[2];
    BYTE current_lte[32];
    BYTE current_wcdma[8];
    BYTE current_gsm;
    BYTE current_cdma[2];
}sit_misc_get_manual_band_mode_rsp;

/*
    SIT_SET_MANUAL_BAND_MODE (RCM ID = 0x0731)
*/

typedef struct
{
    RCM_HEADER hdr;
    BYTE mode_set;
    BYTE lte[32];
    BYTE wcdma[8];
    BYTE gsm;
    BYTE cdma[2];
}sit_misc_set_manual_band_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE cause;
}sit_misc_set_manual_band_mode_rsp;

/*
    SIT_GET_RF_DESENSE_MODE (RCM ID = 0x0732)
*/
typedef null_data_format sit_misc_get_rf_desense_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE desense_status;
    BYTE rat;
}sit_misc_get_rf_desense_mode_rsp;

/*
    SIT_SET_RF_DESENSE_MODE (RCM ID = 0x0733)
*/

typedef struct
{
    RCM_HEADER hdr;
    BYTE desense_operation;
    BYTE rat;
    BYTE lte[32];
    BYTE wcdma[8];
    BYTE gsm;
    BYTE cdma[2];
    BYTE power_level[2];
    BYTE arfcn[4];
    BYTE afc[2];
    BYTE tsc[2];
    BYTE pattern;
    BYTE duplex;
    BYTE ul_bw;
    BYTE ul_freq[4];
    BYTE config_index;
    BYTE special_sf_config_index;
    BYTE vrb_start;
    BYTE vrb_length;
    BYTE mcs;
    BYTE cdma_modulation;
}sit_misc_set_rf_desense_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE cause;
}sit_misc_set_rf_desense_mode_rsp;

/*
    SIT_START_SCANNING_NETWORKS (RCM ID = 0x0734)
*/
#define MAX_NETWORK_SCAN_SPECIFIER      8
#define MAX_NETWORK_SCAN_BAND           8
#define MAX_NETWORK_PLMN_IDS            20
#define MAX_NETWORK_SCAN_CHANNEL        32

typedef struct {
    BYTE network_type;
    UINT32 num_band;
    BYTE bands[MAX_NETWORK_SCAN_BAND];
    BYTE num_channel;
    UINT16 channels[MAX_NETWORK_SCAN_CHANNEL];
} SIT_NET_SCAN_SPECIFIER;

typedef struct {
    char mcc[3];
    char mnc[3];
} SIT_NET_SCAN_PLMN_ID;

#define MAX_NETWORK_SCAN_DATA (sizeof(SIT_NET_SCAN_SPECIFIER) * sizeof(SIT_NET_SCAN_PLMN_ID) * MAX_NETWORK_PLMN_IDS)

// scan type
enum {
    SIT_SCAN_TYPE_ONESHOT = 0,
    SIT_SCAN_TYPE_PERIODIC = 1,
    SIT_SCAN_TYPE_STOP = 2,
};

enum {
    SIT_SCAN_NO_INCREMENTAL_RESULT = 0,
    SIT_SCAN_INCREMENTAL_RESULT = 1,
};

// access network type
enum {
    SIT_NET_ACCESS_RADIO_TYPE_UNKNOWN = 0,
    SIT_NET_ACCESS_RADIO_TYPE_GERAN = 1,
    SIT_NET_ACCESS_RADIO_TYPE_UTRAN = 2,
    SIT_NET_ACCESS_RADIO_TYPE_EUTRAN = 3,
};

// GERAN bands
enum {
    SIT_GERAN_BAND_UNKNOWN = 0,
    SIT_GERAN_BAND_T380 = 1,
    SIT_GERAN_BAND_T410 = 2,
    SIT_GERAN_BAND_450 = 3,
    SIT_GERAN_BAND_480 = 4,
    SIT_GERAN_BAND_710 = 5,
    SIT_GERAN_BAND_750 = 6,
    SIT_GERAN_BAND_T810 = 7,
    SIT_GERAN_BAND_850 = 8,
    SIT_GERAN_BAND_P900 = 9,
    SIT_GERAN_BAND_E900 = 10,
    SIT_GERAN_BAND_R900 = 11,
    SIT_GERAN_BAND_DCS1800 = 12,
    SIT_GERAN_BAND_PCS1900 = 13,
    SIT_GERAN_BAND_ER900 = 14,
    STI_GERAN_BAND_MAX,
};

// UTRAN bands
enum {
    SIT_UTRAN_BAND_UNKNOWN = 0,
    SIT_UTRAN_BAND_1 = 1,
    SIT_UTRAN_BAND_26 = 26,
    SIT_UTRAN_BAND_MAX,
};

// EUTRAN bands
enum {
    SIT_EUTRAN_BAND_UNKNOWN = 0,
    SIT_EUTRAN_BAND_1 = 1,
    SIT_EUTRAN_BAND_70 = 70,
    SIT_EUTRAN_BAND_MAX,
};

typedef struct
{
    RCM_HEADER hdr;
    BYTE scan_type;
    UINT16 interval;
    UINT16 max_search_time;
    BYTE incremental_results;
    UINT16 periodicity;
    BYTE num_record;
    BYTE num_plmn;
    char data[MAX_NETWORK_SCAN_DATA];
}sit_net_start_scanning_network;

/*
    SIT_STOP_SCANNING_NETWORKS (RCM ID = 0x0735)
*/
typedef null_data_format sit_net_stop_scanning_network;

/*
    SIT_IND_SCANNING_NETWORKS (RCM ID = 0x0736)
*/
typedef enum
{
    SIT_NET_SCAN_STATUS_PARTIAL = 0x01,
    SIT_NET_SCAN_STATUS_COMPLETE = 0x02,
}sit_net_scan_type;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE scan_status;
    INT32 cell_info_num;
    char cell_info_list[0];
}sit_net_scanning_network_ind;

/*
    SIT_IND_RADIO_CAPABILITY (RCM ID = 0x0737)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    INT32 version;
    INT32 session_id;
    INT32 phase;  // same to RadioCapabilityPhase
    INT32 rc_raf;
    BYTE uuid[SIT_MAX_UUID_LENGTH];
    INT32 status;  // same to RadioCapabilityStatus
}sit_net_radio_capability_ind;

/*
    SIT_GET_MANUAL_RAT_MODE (RCM ID = 0x0738)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE manual_rat_mode_set;
    INT32 rat;
}sit_net_get_manual_rat_mode_rsp;

/*
    SIT_SET_MANUAL_RAT_MODE (RCM ID = 0x0739)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE rat_mode_set;
    INT32 rat;
}sit_net_set_manual_rat_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE cause;
}sit_net_set_manual_rat_mode_rsp;

/*
    SIT_GET_FREQUENCY_LOCK (RCM ID = 0x073A)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE freq_mode_set;
    BYTE rat;
    INT32 lte_pci;
    INT32 lte_earfcn;
    INT32 gsm_arfcn;
    INT32 wcdma_psc;
    INT32 wcdma_uarfcn;
}sit_net_get_freq_lock_rsp;

/*
    SIT_SET_FREQUENCY_LOCK (RCM ID = 0x073B)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE freq_mode_set;
    BYTE rat;
    INT32 lte_pci;
    INT32 lte_earfcn;
    INT32 gsm_arfcn;
    INT32 wcdma_psc;
    INT32 wcdma_uarfcn;
} sit_net_set_freq_lock_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE result;
}sit_net_set_freq_lock_rsp;

/*
    SIT_IND_B2_B1_CONFIG (RCM ID = 0x073C)
*/
typedef struct
{
    BYTE event_type;
    BYTE rat_type;
    BYTE eutra_threshold_type;
    INT16 eutra_threshold;
    BYTE utra_threshold_type;
    INT16 utra_threshold;
    INT16 geran_threshold;
    INT16 cdma_threshold;
    INT16 hysteresis;
    INT16 time_to_trigger;
}b2_b1_configuration_type;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE config_list_num;
    b2_b1_configuration_type b2_b1_configuration[0];
}sit_net_b2_b1_config_ind;

/*
    SIT_SET_ENDC_MODE (RCM ID = 0x073D)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE mode;
}sit_net_set_endc_mode_req;

/*
    SIT_GET_ENDC_MODE (RCM ID = 0x073E)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE mode;
}sit_net_get_endc_mode_rsp;

/*
   SIT_IND_SCG_BEARER_ALLOCATION (RCM ID = 0x073F)
     - deprecated: legacy before AOSP Q
 */
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE rat;           // sit_rat_type_e_type
    BYTE scg_status;    // 0x00 : No SCG, 0x01: SCG added
}sit_net_sgc_bearer_allocation_ind;

/*
    SIT_SET_PREFERRED_DATA_MODEM (RCM ID = 0x0740)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE stackId;
}sit_net_set_preferred_data_modem_req;
typedef null_data_format sit_net_set_preferred_data_modem_rsp;

/*
   SIT_IND_FREQUENCY_INFO (RCM ID = 0x0741)
 */
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE rat;           // sit_rat_type_e_type
    INT32 band;
    INT32 frequency;
}sit_net_frequency_info_ind;

/*
    SIT_IND_PHYSICAL_CHANNEL_CONFIG (RCM ID = 0x0742)
 */
#define MAX_SIT_PHYSICAL_CHANNEL_CONFIGS 8
#define MAX_SIT_CONTEXT_ID_LEN 9
enum {
    SIT_FREQUENCY_RANGE_NONE = 0,
    SIT_FREQUENCY_RANGE_LOW = 1,
    SIT_FREQUENCY_RANGE_MID = 2,
    SIT_FREQUENCY_RANGE_HIGH = 3,
    SIT_FREQUENCY_RANGE_MMWAVE = 4,
};

typedef struct {
    BYTE cell_status;
    INT32 cell_bandwidth_downlink;
    BYTE rat;
    BYTE frequency_range;
    INT32 channel;
    BYTE context_len;
    int context_id[MAX_SIT_CONTEXT_ID_LEN];
    UINT32 physical_cellid;
} sit_physical_channel_config;

typedef struct
{
    RCM_IND_HEADER hdr;
    int config_len;
    sit_physical_channel_config configs[MAX_SIT_PHYSICAL_CHANNEL_CONFIGS];
}sit_net_physical_channel_config_ind;

/*
    SIT_SET_LOCATION_UPDATE_SETTING (RCM ID = 0x0744)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE update_setting;
}sit_set_location_update_setting_req;

/*
    SIT_GET_FREQUENCY_INFO (RCM ID = 0x0746)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE rat;
    UINT32 band;
    UINT32 frequency;
}sit_net_get_frequency_info_rsp;

/*
    SIT_SET_RADIO_POWER (RCM ID = 0x0800)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 radio_state;
}sit_pwr_set_radio_power_req;

typedef null_data_format sit_pwr_radio_power_rsp;

typedef enum
{
    SIT_PWR_RADIO_STATE_INITIALIZED = 0x00,
    SIT_PWR_RADIO_STATE_STOP_NETWORK = 0x01,
    SIT_PWR_RADIO_STATE_START_NETWORK = 0x02,
    SIT_PWR_RADIO_STATE_POWER_OFF = 0x03,

    SIT_PWR_RADIO_STATE_MAX
}sit_pwr_radio_state_e_type;

/*
    SIT_GET_RADIO_POWER (RCM ID = 0x0801)
*/
typedef struct
{
    RCM_HEADER hdr;
}sit_pwr_get_radio_power_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 radio_state;
}sit_pwr_get_radio_power_rsp;

/*
    SIT_IND_RADIO_STATE_CHANGED (RCM ID = 0x0802)
*/
//typedef null_ind_data_format sit_pwr_radio_state_changed_ind;
typedef struct
{
    RCM_IND_HEADER hdr;
    INT32 radio_state;
}sit_pwr_radio_state_changed_ind;

/*
    SIT_IND_RADIO_READY (RCM ID = 0x0803)
*/
typedef null_ind_data_format sit_pwr_radio_ready_ind;

/*
    SIT_IND_PHONE_RESET (RCM ID = 0x0804)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE reset_type;
    BYTE reset_cause;
}sit_pwr_phone_reset_ind;

typedef enum
{
    SIT_PWR_RESET_TYPE_PHONE_ONLY = 0x01,
    SIT_PWR_RESET_TYPE_RESET_BOTH = 0x02,
    SIT_PWR_RESET_TYPE_SHUTDOWN = 0x03,

    SIT_PWR_RESET_TYPE_MAX
}sit_pwr_phone_reset_type_e_type;

typedef enum
{
    SIT_PWR_RESET_CAUSE_MAX
}sit_pwr_phone_reset_cause_e_type;

/*
    SIT_IND_MODEM_RESTART (RCM ID = 0x0805)
*/
#define SIT_PWR_MODEM_RESTART_REASON_SIZE (50)
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE reason[SIT_PWR_MODEM_RESTART_REASON_SIZE];
}sit_pwr_modem_restart_ind;

/*
    SIT_SET_STACK_STATUS (RCM ID = 0x080F)
*/
typedef enum
{
    SIT_PWR_STATCK_DISABLE,
    SIT_PWR_STATCK_ENABLE
}sit_pwr_stack_mode_e_type;

typedef struct
{
    RCM_HEADER hdr;
    BYTE mode;
}sit_pwr_set_stack_status_req;

typedef null_data_format sit_pwr_set_stack_status_rsp;

/*
    SIT_GET_STACK_STATUS (RCM ID = 0x0810)
*/
typedef null_data_format sit_pwr_get_stack_status_req;
typedef sit_pwr_set_stack_status_req sit_pwr_get_stack_status_rsp;

/*
    SIT_GET_SIGNAL_STRENGTH (RCM ID = 0x0900)
*/
typedef null_data_format sit_misc_get_signal_strength_req;

typedef struct {
    INT32 sig_str;
    INT32 ber;
} GW_SIGNAL_STRENGTH;

typedef struct {
    INT32 dbm;
    INT32 ecio;
} CDMA_SIGNAL_STRENGTH;

typedef struct {
    INT32 dbm;
    INT32 ecio;
    INT32 snr;
} EVDO_SIGNAL_STRENGTH;

typedef struct {
    INT32 sig_str;
    INT32 rsrp;
    INT32 rsrq;
    INT32 rssnr;
    INT32 cqi;
    INT32 timing_adv;
} LTE_SIGNAL_STRENGTH;

typedef struct {
    INT32 rscp;
} TD_SCDMA_SIGNAL_STRENGTH;

typedef struct {
    INT32 sig_str;
    INT32 ber;
    INT32 rscp;
} TD_SCDMA_SIGNAL_STRENGTH_V1_4;

typedef struct {
    INT32 ecno;
} UMTS_SIGNAL_STRENGTH;

typedef struct {
    INT32 sig_str;
    INT32 ber;
    INT32 ta;
} GSM_SIGNAL_STRENGTH;

typedef struct {
    INT32 sig_str;
    INT32 ber;
    INT32 rscp;
    INT32 ecno;
} WCDMA_SIGNAL_STRENGTH;

typedef struct {
    INT32 ss_rsrp;
    INT32 ss_rsrq;
    INT32 ss_sinr;
    INT32 csi_rsrp;
    INT32 csi_rsrq;
    INT32 csi_sinr;
} NR_SIGNAL_STRENGTH;

typedef struct {
    GW_SIGNAL_STRENGTH GW_SignalStrength;
    CDMA_SIGNAL_STRENGTH CDMA_SignalStrength;
    EVDO_SIGNAL_STRENGTH EVDO_SignalStrength;
    LTE_SIGNAL_STRENGTH LTE_SignalStrength;
    TD_SCDMA_SIGNAL_STRENGTH TD_SCDMA_SignalStrength;
    UMTS_SIGNAL_STRENGTH UMTS_SignalStrength;
} SIGNAL_STRENGTH;

typedef struct {
    GSM_SIGNAL_STRENGTH GSM_SignalStrength;
    WCDMA_SIGNAL_STRENGTH WCDMA_SignalStrength;
    CDMA_SIGNAL_STRENGTH CDMA_SignalStrength;
    EVDO_SIGNAL_STRENGTH EVDO_SignalStrength;
    TD_SCDMA_SIGNAL_STRENGTH_V1_4 TD_SCDMA_SignalStrength;
    LTE_SIGNAL_STRENGTH LTE_SignalStrength;
    NR_SIGNAL_STRENGTH NR_SignalStrength;
} SIGNAL_STRENGTH_V1_4;


typedef struct
{
    RCM_HEADER hdr;
    INT16 valid_rat_sig_flag;
    SIGNAL_STRENGTH sig_strength;
}sit_misc_signal_strength_rsp;

typedef struct
{
    RCM_HEADER hdr;
    INT16 valid_rat_sig_flag;
    SIGNAL_STRENGTH_V1_4 sig_strength;
}sit_misc_signal_strength_rsp_v1_4;

typedef struct
{
    RCM_IND_HEADER hdr;
    INT16 valid_rat_sig_flag;
    SIGNAL_STRENGTH sig_strength;
}sit_misc_signal_strength_ind;

typedef struct
{
    RCM_IND_HEADER hdr;
    INT16 valid_rat_sig_flag;
    SIGNAL_STRENGTH_V1_4 sig_strength;
}sit_misc_signal_strength_ind_v1_4;

typedef enum
{
    SIT_MISC_SIG_RAT_SIG_GW = 0x01,
    SIT_MISC_SIG_RAT_SIG_CDMA = 0x02,
    SIT_MISC_SIG_RAT_SIG_EVDO = 0x04,
    SIT_MISC_SIG_RAT_SIG_LTE = 0x08,
    SIT_MISC_SIG_RAT_SIG_TD_SCDMA = 0x10,
    SIT_MISC_SIG_RAT_SIG_NR = 0x20, // TO DO:: SYNC with CP SIT difinition
}sit_misc_get_signal_strength_rat_sig_e_type;

typedef enum
{
    SIT_MISC_SIG_RAT_SIG_GSM = 0x01,
    SIT_MISC_SIG_RAT_SIG_WCDMA = 0x02,
    SIT_MISC_SIG_RAT_SIG_LTE_V1_4 = 0x04,
    SIT_MISC_SIG_RAT_SIG_CDMA_V1_4 = 0x08,
    SIT_MISC_SIG_RAT_SIG_EVDO_V1_4 = 0x10,
    SIT_MISC_SIG_RAT_SIG_TD_SCDMA_V1_4 = 0x20,
    SIT_MISC_SIG_RAT_SIG_NR_V1_4 = 0x40,
}sit_misc_get_signal_strength_rat_sig_e_type_v1_4;

/* RSSI  field */
/* Radio signal strength */
typedef enum
{
    DISP_RSSI_0,     /* 0x00 */
    DISP_RSSI_1,     /* 0x01 */
    DISP_RSSI_2,     /* 0x02 */
    DISP_RSSI_3,     /* 0x03 */
    DISP_RSSI_4,     /* 0x04 */
    DISP_RSSI_5,     /* 0x05 */
    DISP_RSSI_6,     /* 0x06 */
    DISP_RSSI_MAX
}sit_misc_disp_rssi_e_type;

/*
    SIT_GET_BASEBAND_VERSION (RCM ID = 0x0901)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE ver_mask;
}sit_misc_get_baseband_version_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE ver_mask;
    char sw_version[32];
    char hw_version[32];
    char rf_cal_date[32];
    char product_code[32];
    char model_id[17];
    BYTE prl_nam_num;
    BYTE prl_version[51];
    BYTE eri_nam_num;
    BYTE eri_version[51];
    BYTE cp_chipsetname[16];
}sit_misc_get_baseband_version_rsp;

typedef enum
{
    SIT_MISC_VER_MASK_SW_VERSION = 0x01,
    SIT_MISC_VER_MASK_HW_VERSION = 0x02,
    SIT_MISC_VER_MASK_RF_CAL_DATA = 0x04,
    SIT_MISC_VER_MASK_PRODUCT_CODE = 0x08,
    SIT_MISC_VER_MASK_MODEL_ID = 0x10,    /*CDMA only*/
    SIT_MISC_VER_MASK_PRL = 0x20,    /*CDMA only*/
    SIT_MISC_VER_MASK_ERI = 0x40,    /*CDMA only*/
    SIT_MISC_VER_MASK_CP_CHIPSET = 0x80,
    SIT_MISC_VER_MASK_ALL = 0xFF
}sit_misc_baseband_version_mask_e_type;

/*
    SIT_SET_SCREEN_STATE (RCM ID = 0x0902)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 screen_state;
}sit_misc_set_screen_state_req;

typedef null_data_format sit_misc_set_screen_state_rsp;

typedef enum
{
    SIT_MISC_SCREEN_STATE_OFF = 0x00,
    SIT_MISC_SCREEN_STATE_ON
}sit_misc_screen_state_e_type;

/*
    SIT_SET_TTY_MODE (RCM ID = 0x0903)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 tty_mode;
}sit_misc_set_tty_mode_req;

typedef null_data_format sit_misc_set_tty_mode_rsp;

typedef enum
{
    SIT_MISC_TTY_MODE_OFF = 0x00,
    SIT_MISC_TTY_MODE_FULL,
    SIT_MISC_TTY_MODE_HCO,    /*hearing carryover*/
    SIT_MISC_TTY_MODE_VCO,    /*voice carryover*/

    SIT_MISC_TTY_MODE_MAX
}sit_misc_tty_mode_e_type;


/*
    SIT_GET_TTY_MODE (RCM ID = 0x0904)
*/
typedef null_data_format sit_misc_get_tty_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 tty_mode;
}sit_misc_get_tty_mode_rsp;

/*
    SIT_IND_NITZ_TIME_RECEIVED (RCM ID = 0x0905)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
//    BYTE time_info_type;
    BYTE daylight_valid;
    BYTE year;
    BYTE month;
//    BYTE month;
    BYTE day;
    BYTE hour;
    BYTE minute;
    BYTE second;
    BYTE time_zone;
    BYTE daylight_adjust;
    BYTE day_of_week;
    BYTE mminfo;
    BYTE plmn[MAX_PLMN_LEN];
}sit_misc_nits_time_received_ind;

typedef enum
{
    SIT_NITS_DAYLIGHT_INFO_VALID = 0x00,
    SIT_NITS_DAYLIGHT_INFO_INVALID = 0x01
}sit_misc_nits_daylight_valid_e_type;

typedef enum
{
    SIT_NITS_DAYLIGHT_ADJUST_NOADJUST = 0x00,
    SIT_NITS_DAYLIGHT_ADJUST_PLUS1HOUR,
    SIT_NITS_DAYLIGHT_ADJUST_PLUS2HOUR
}sit_misc_nits_daylight_adjust_e_type;

typedef enum
{
    SIT_NITS_DAY_OF_WEEK_SUN = 0x00,
    SIT_NITS_DAY_OF_WEEK_MON,
    SIT_NITS_DAY_OF_WEEK_TUE,
    SIT_NITS_DAY_OF_WEEK_WED,
    SIT_NITS_DAY_OF_WEEK_THU,
    SIT_NITS_DAY_OF_WEEK_FRI,
    SIT_NITS_DAY_OF_WEEK_SAT
}sit_misc_nits_day_of_week_e_type;


/*
    SIT_SET_CFG_DEFAULT (RCM ID = 0x0907)
*/
typedef null_data_format sit_misc_set_cfg_default_req;

/*
    SIT_SET_ENG_MODE (RCM ID = 0x0908)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE eng_mode;
}sit_misc_set_eng_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE eng_mode;
    BYTE sub_mode;
}sit_misc_set_eng_mode_ex_req;

/*
    SIT_SET_SCREEN_LINE (RCM ID = 0x0909)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE scr_line;
}sit_misc_set_scr_line_req;

/*
    SIT_SET_DEBUG_TRACE (RCM ID = 0x090B)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE debug_trace;
}sit_misc_set_debug_trace_req;

/*
    SIT_GET_ACTIVITY_INFO (RCM ID = 0x090C)
*/
typedef null_data_format sit_misc_get_activity_info_req;

typedef struct {
    RCM_HEADER hdr;
    UINT32 sleep_mode_time_ms;
    UINT32 idle_mode_time_ms;
    UINT32 tx_mode_time_ms[RIL_NUM_TX_POWER_LEVELS];
    UINT32 rx_mode_time_ms;
}sit_misc_get_activity_info_rsp;

/*
    SIT_SET_OPERATOR_INFO = 0x090D
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE plmn[MAX_PLMN_LEN];
    UINT32 openCarrierIndex;
}sit_misc_set_operator_info_req;

/*
    SIT_SET_ENG_STRING_INPUT (RCM ID = 0x0910)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE len;
    char input[255];
}sit_misc_set_eng_string_input_req;

/*
    SIT_GET_MSL_CODE (RCM ID = 0x0911)
*/
typedef null_data_format sit_misc_get_msl_code_req;

typedef struct {
    RCM_HEADER hdr;
    char msl_code[6];
} sit_misc_get_msl_code_rsp;

/*
    SIT_SET_PREFERRED_CALL_CAPABILITY (RCM ID = 0x091A)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 mode;
}sit_misc_set_preferred_call_capability_req;

typedef null_data_format sit_misc_set_preferred_call_capability_rsp;

/*
    SIT_GET_PREFERRED_CALL_CAPABILITY (RCM ID = 0x091B)
*/
typedef null_data_format sit_misc_get_preferred_call_capability_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 mode;
}sit_misc_get_preferred_call_capability_rsp;

typedef enum
{
    SIT_PREF_CALL_CAPABILITY_CS_ONLY = 0x00,
    SIT_PREF_CALL_CAPABILITY_PS_ONLY = 0x01,
    SIT_PREF_CALL_CAPABILITY_CS_PREFERRED = 0x02,
    SIT_PREF_CALL_CAPABILITY_PS_PREFERRED = 0x03,
}sit_misc_preferred_call_capability_e_type;

/*
    SIT_SET_PIN_CONTROL (RCM ID = 0x0920)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE signal;
    BYTE status;
}sit_misc_set_pin_control_req;

typedef null_data_format sit_misc_set_pin_control_rsp;

/*
    SIT_IND_PIN_CONTROL (RCM ID = 0x0921)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE signal;
    BYTE status;
}sit_misc_pin_control_ind;


#if 0   //removed
/**
 * SIT_GET_OEM_NV_ITEM (RCM ID = 0x090C)
 */
#define MAX_NV_ITEM_DATA_SIZE    128

typedef struct
{
    RCM_HEADER hdr;
    UINT32 nv_index;
} sit_misc_get_oem_nv_item_req;

typedef struct
{
    RCM_HEADER hdr;
    char data[MAX_NV_ITEM_DATA_SIZE];
} sit_misc_get_oem_nv_item_rsp;

typedef struct
{
    RCM_HEADER hdr;
    UINT32 nv_index;
    char data[MAX_NV_ITEM_DATA_SIZE];
} sit_misc_set_oem_nv_item_req;

typedef null_data_format sit_misc_set_oem_nv_item_rsp;
#endif

#ifdef SUPPORT_CDMA
/*
    SIT_GET_CDMA_SUBSCRIPTION (RCM ID = 0x090F)
*/
typedef null_data_format sit_misc_get_cdma_subscription_req;

typedef struct {
    RCM_HEADER hdr;
    BYTE mdn_size;
    BYTE mdn[MAX_CDMA_MDN_LEN];
    BYTE min[MAX_CDMA_MIN_LEN];
    WORD sid;
    WORD nid;
    UINT32 prl_version;
}sit_misc_get_cdma_subscription_rsp;
#endif // SUPPORT_CDMA

/*
    SIT_SET_VOICE_OPERATION (RCM ID = 0x091A)
*/
typedef struct
{
    RCM_HEADER hdr;
    UINT32 mode;
}sit_misc_set_voice_operation_req;

typedef null_data_format sit_misc_set_voice_operation_rsp;

/*
    SIT_SET_SW_VERSION (RCM ID = 0x0922)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE model_name[32];
    BYTE sw_version[32];
    BYTE product_name[32];
}sit_misc_set_device_info_req;

/*
    SIT_GET_HW_CONFIG (RCM ID = 0x0926)
*/
#define SIT_MAX_HW_CONFIG (2)
typedef null_data_format sit_misc_get_hw_config_req;

typedef struct
{
    INT32 ril_model;
    UINT32 rat;   // same to RIL_RadioAccessFamilyForCp
    INT32 max_voice;
    INT32 max_data;
    INT32 max_standby;
}SIT_HW_CFG_MODEM;

typedef struct
{
    char modem_uuid[SIT_MAX_UUID_LENGTH];
}SIT_HW_CFG_SIM;

typedef struct
{
    BYTE type;
    char uuid[SIT_MAX_UUID_LENGTH];
    BYTE state;
    union {
        SIT_HW_CFG_MODEM modem;
        SIT_HW_CFG_SIM sim;
    } cfg;
}SIT_HW_CONFIG;

typedef struct
{
    RCM_HEADER hdr;
    BYTE num_recodrs;
    SIT_HW_CONFIG hw_config[SIT_MAX_HW_CONFIG];
}sit_misc_get_hw_config_rsp;

/*
    SIT_IND_HW_CONFIG_CHANGED (RCM ID = 0x0927)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE num_recodrs;
    SIT_HW_CONFIG hw_config[SIT_MAX_HW_CONFIG];
}sit_misc_hw_config_change_ind;

/*
    SIT_SET_IND_CMD_FILTER (RCM ID = 0x0928)
*/
typedef enum
{
    SIT_MISC_IND_FILTER_SIGNAL_STRENGTH = (1 << 0),
    SIT_MISC_IND_FILTER_NTW_STATE_CHANGED = (1 << 1),
    SIT_MISC_IND_FILTER_DATA_CALL_LIST_CHANGED = (1 << 2),
} sit_misc_ind_cmd_filter_type;

typedef struct
{
    RCM_HEADER hdr;
    UINT ind_cmd_filter;    // same to RIL_UnsolicitedResponseFilter
}sit_misc_set_ind_cmd_filter_req;

/*
    SIT_IND_CDMA_PRL_CHANGED (RCM ID = 0x0929)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    INT32 prl_ver;
}sit_misc_cdma_prl_change_ind;

/*
 *   SIT_IND_SAR_CONTROL_STATE (RCM ID = 0x092A)
 */
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE device_state;
}sit_misc_psensor_sar_control_state_ind;

/*
 *   SIT_SET_SENSOR_STATUS (RCM ID = 0x092B)
 */
typedef struct
{
    RCM_HEADER hdr;
    INT32 psensor_status;
}sit_misc_psensor_set_psensor_status_req;

/*
    SIT_SET_RSSI_SCAN (RCM ID = 0x092D)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE rat;
    BYTE band;
    BYTE rbw;
    BYTE scan_mode;
    UINT16 start_frequency;
    UINT16 end_frequency;
    BYTE step;
    BYTE antenna_selection;
    UINT16 sampling_count;
    BYTE tx1;
    BYTE tx1_band;
    BYTE tx1_bw;
    UINT16 tx1_freq;
    UINT16 tx1_power;
    BYTE tx1_rb_num;
    BYTE tx1_rb_offset;
    BYTE tx1_mcs;
    BYTE tx2;
    BYTE tx2_band;
    BYTE tx2_bw;
    UINT16 tx2_freq;
    UINT16 tx2_power;
    BYTE tx2_rb_num;
    BYTE tx2_rb_offset;
    BYTE tx2_mcs;
	BYTE reserved[20];
}sit_misc_set_rssi_scan_req;
typedef null_data_format sit_misc_set_rssi_scan_rsp;

/*
    SIT_IND_RSSI_SCAN (RCM ID = 0x092E)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE total_page;
    BYTE current_page;
    UINT16 start_frequency;
    UINT16 end_frequency;
    BYTE step;
    INT16 result[2044];
}sit_misc_rssi_scan_ind;

enum ScanRat {
    SCAN_RAT_2G,
    SCAN_RAT_3G,
    SCAN_RAT_LTE,
    SCAN_RAT_LTE_CA,
};

enum Band2G {
    BAND_380T_GSM = 0x01,
    BAND_410_GSM = 0x02,
    BAND_450 = 0x03,
    BAND_480 = 0x04,
    BAND_710 = 0x05,
    BAND_750 = 0x06,
    BAND_810T_GSM = 0x07,
    BAND_850 = 0x08,
    BAND_900P = 0x09,
    BAND_900E = 0x0A,
    BAND_900R = 0x0B,
    BAND_1800DCS = 0x0C,
    BAND_1900PCS = 0x0D,
    BAND_2G_MAX = BAND_1900PCS,
};

 // B1 = 0x01, B2 = 0x02, ..., B255 = 0xFF
enum Band3G {
    BAND_3G_MAX = 0xFF,
};

enum ScanMode {
    SCAN_ALL = 0x00,
    SCAN_PARTIAL = 0x01,
};

enum ScanFrequency {
    FREQUENCY_MAX = 59250,
    FREQUENCY_INVALID,
    FREQUENCY_DEFAULT = 0xFFFF,
};

enum ScanAntennaSel {
    ANTENNA_ALL = 0x00,
    ANTENNA_MAIN_ONLY = 0x01,
    ANTENNA_SUB_ONLY = 0x01,
    ANTENNA_ALL_PRESET = 0x04,
    ANTENNA_MAIN_PRESET = 0x04,
    ANTENNA_SUB_PRESET = 0x05,
    ANTENNA_DEFAULT = 0xFF,
};

#define MAX_SIT_AT_COMMAND_LENGTH    1000

/*
    SIT_SET_FORWARDING_AT_COMMAND (RCM ID = 0x0931)
*/
typedef struct {
    RCM_HEADER hdr;
    unsigned short length;
    char data[MAX_SIT_AT_COMMAND_LENGTH + 1];
}sit_misc_forwarding_at_command_req;
typedef null_data_format sit_misc_forwarding_at_command_rsp;

/*
    SIT_IND_FORWARDING_AT_COMMAND (RCM ID = 0x0932)
*/
typedef struct {
    RCM_IND_HEADER hdr;
    unsigned short length;
    char data[MAX_SIT_AT_COMMAND_LENGTH + 1];
}sit_misc_forwarding_at_command_ind;

#define MAX_RADIO_NODE_DATA_LEN    256

/*
    SIT_GET_RADIO_NODE (RCM ID = 0x0936)
*/
typedef struct {
    RCM_HEADER hdr;
    char path[MAX_RADIO_NODE_DATA_LEN];
}sit_misc_get_radio_node_req;

typedef struct {
    RCM_HEADER hdr;
    char value[MAX_RADIO_NODE_DATA_LEN];
}sit_misc_get_radio_node_rsp;

/*
    SIT_SET_RADIO_NODE (RCM ID = 0x0937)
*/
typedef struct {
    RCM_HEADER hdr;
    char path[MAX_RADIO_NODE_DATA_LEN];
    char value[MAX_RADIO_NODE_DATA_LEN];
}sit_misc_set_radio_node_req;

typedef null_data_format sit_misc_set_radio_node_rsp;

/*
    SIT_GET_VOLTE_PROVISION_UPDATE (RCM ID = 0x0938)
*/
typedef null_data_format sit_misc_get_volte_provision_req;

typedef struct {
    RCM_HEADER hdr;
    BYTE status;
}sit_misc_get_volte_provision_rsp;

/*
    SIT_SET_VOLTE_PROVISION_UPDATE (RCM ID = 0x0939)
*/
typedef null_data_format sit_misc_set_volte_provision_req;

typedef struct {
    RCM_HEADER hdr;
    BYTE result;
}sit_misc_set_volte_provision_rsp;

/*
   SIT_SET_INTPS_SERVICE  (RCM ID = 0x0933)
   DEPRECATED
 */
typedef enum
{
    SIT_MISC_ENABLE_INTPS_SERVICE = 0x01,   // enable default internet service
    SIT_MISC_DISABLE_INTPS_SERVICE = 0x00   // disable default internet service
} sit_misc_intps_service_type;

typedef struct
{
    RCM_HEADER hdr;
    INT32 mode;
} sit_misc_set_intps_service_req;

/*
   SIT_SET_SELFLOG (RCM ID = 0x093A)
*/
typedef struct {
    RCM_HEADER hdr;
    BYTE mode;
    BYTE size;
}sit_misc_set_selflog_req;

typedef struct {
    RCM_HEADER hdr;
    BYTE result;
}sit_misc_set_selflog_rsp;

/*
   SIT_GET_SELFLOG_STATUS (RCM ID = 0x093B)
*/
typedef struct {
    RCM_HEADER hdr;
    BYTE status;
}sit_misc_get_selflog_status_rsp;

/*
   SIT_IND_SELFLOG_STATUS (RCM ID = 0x093C)
*/
typedef struct {
    RCM_IND_HEADER hdr;
    BYTE status;
}sit_misc_selflog_status_ind;

/*
   SIT_SET_ELEVATOR_SENSOR  (RCM ID = 0x093D)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE enable; // 0x00: Disable, 0x01: Enable
}sit_set_elevator_sensor_req;

/*
   SIT_SET_MODEM_CONFIG (RCM ID = 0x093F)
*/
typedef struct {
    RCM_HEADER hdr;
    BYTE config;
}sit_misc_set_modems_config_req;

enum {
    SIT_MODEM_CONFIG_SINGLE_SIM = 0,    // ss
    SIT_MODEM_CONFIG_MULTI_SIM = 1,     // dsds
};

/*
    SIT_SET_MODEM_LOG_DUMP (RCM ID = 0x0940)
*/
typedef struct {
    RCM_HEADER hdr;
    INT32 type;
}sit_misc_set_modem_log_dump_req;

typedef struct {
    RCM_HEADER hdr;
    BYTE result;
}sit_misc_set_modem_log_dump_rsp;

/*
    SIT_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA (RCM ID = 0x0943)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 ms;
    INT32 db;
    BYTE len;
    INT32 dbm[10];
    INT32 radio_acc_net;
}sit_misc_set_signal_strength_report_criteria_req;

/*
    SIT_SET_LINK_CAPACITY_REPORTING_CRITERIA (RCM ID = 0x0944)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 h_ms;
    INT32 h_dl_kbps;
    INT32 h_ul_kbps;
    BYTE t_dl_len;
    INT32 t_dl_kbps[20];
    BYTE t_ul_len;
    INT32 t_ul_kbps[20];
    INT32 radio_acc_net;
}sit_misc_set_link_capacity_report_criteria_req;

/*
    SIT_IND_CURRENT_LINK_CAPACITY_ESTIMATE (RCM ID = 0x0945)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    INT32 dl_capa_kbps;
    INT32 ul_capa_kbps;
}sit_misc_current_link_capa_estimate_ind;

typedef struct {
    INT32 dl_capa_kbps;
    INT32 ul_capa_kbps;
} CURRENT_LINK_CAPA_ESTIMATE;

/*
    SIT_SET_SELFLOG_PROFILE (RCM ID = 0x0942)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE data[0];
}sit_set_selflog_profile_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE result;
}sit_set_selflog_profile_rsp;


// ##############################
// #### RCM Commands for Audio/Sound ####
// ##############################

typedef enum
{
    SITRIL_AUDIO_PATH_NONE = 0,
    SITRIL_AUDIO_PATH_HANDSET = 1,                     // Phone MIC -> RECEIVER
    SITRIL_AUDIO_PATH_HEADSET = 2,                     // Headset (MIC -> SPK)
    SITRIL_AUDIO_PATH_HANDSFREE = 3,                   // Handfree (MIC -> SPK)
    SITRIL_AUDIO_PATH_BLUETOOTH = 4,                   // Narrow Band Bluetooth (MIC -> SPK)
    SITRIL_AUDIO_PATH_STEREO_BLUETOOTH = 5,            // Narrow Band Stereo Bluetooth (MIC -> SPK)
    SITRIL_AUDIO_PATH_SPEAKRERPHONE = 6,               // Phone MIC -> SPEAKER Phone
    SITRIL_AUDIO_PATH_35PI_HEADSET = 7,                // Phone MIC -> Headset SPK
    SITRIL_AUDIO_PATH_BT_NS_EC_OFF = 8,                // Narrow Band Bluetooth (MIC -> SPK) + Noise/Eco Cancellation OFF
    SITRIL_AUDIO_PATH_WB_BLUETOOTH = 9,                // Wide Band Bluetooth (MIC -> SPK)
    SITRIL_AUDIO_PATH_WB_BT_NS_EC_OFF = 10,            // Wide Band Bluetooth (MIC -> SPK) + Noise/Eco Cancellation OFF
    SITRIL_AUDIO_PATH_HANDSET_HAC = 11,                // Phone MIC -> RECEIVER for HAC(Hearing Aid Compatibility
    SITRIL_AUDIO_PATH_LINEOUT = 12,                    // Lineout

    SITRIL_AUDIO_PATH_VOLTE_HANDSET = 65,              // VoLTE : Phone MIC -> RECEIVER
    SITRIL_AUDIO_PATH_VOLTE_HEADSET = 66,              // VoLTE : Headset (MIC -> SPK)
    SITRIL_AUDIO_PATH_VOLTE_HFK = 67,                  // VoLTE : Hands Free Kit (MIC -> SPK)
    SITRIL_AUDIO_PATH_VOLTE_BLUETOOTH = 68,            // VoLTE : Narrow Band Bluetooth (MIC -> SPK)
    SITRIL_AUDIO_PATH_VOLTE_STEREO_BLUETOOTH = 69,     // VoLTE : Narrow Band Stereo Bluetooth (MIC -> SPK)
    SITRIL_AUDIO_PATH_VOLTE_SPEAKRERPHONE = 70,        // VoLTE : Phone MIC -> SPEAKER Pone
    SITRIL_AUDIO_PATH_VOLTE_35PI_HEADSET = 71,         // VoLTE : Phone MIC -> Headset SPK
    SITRIL_AUDIO_PATH_VOLTE_BT_NS_EC_OFF = 72,         // VoLTE : Narrow Band Bluetooth (MIC -> SPK) + Noise/Eco Cancellation OFF
    SITRIL_AUDIO_PATH_VOLTE_WB_BLUETOOTH = 73,         // VoLTE : Wide Band Bluetooth (MIC -> SPK)
    SITRIL_AUDIO_PATH_VOLTE_WB_BT_NS_EC_OFF = 74,      // VoLTE : Wide Band Bluetooth (MIC -> SPK) + Noise/Eco Cancellation OFF
    SITRIL_AUDIO_PATH_VOLTE_VOLTE_HANDSET_HAC = 75,    // VoLTE : Phone MIC -> RECEIVER for HAC(Hearing Aid Compatibility
    SITRIL_AUDIO_PATH_VOLTE_LINEOUT = 76,              // VoLTE Lineout

    SITRIL_AUDIO_PATH_MAX
}sit_snd_audiopath_mode;

typedef enum
{
    SITRIL_MULTIMIC_NONE = 0x00,    // Off
    SITRIL_MULTIMIC_ONE = 0x01,
    SITRIL_MULTIMIC_TWO = 0x02,
    SITRIL_MULTIMIC_THREE = 0x03,
    SITRIL_MULTIMIC_MAX
} sit_snd_multiMIC_mode;

/*
    SIT_SET_MUTE (RCM ID = 0x0A00)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 mute_mode;
}sit_snd_set_mute_req;

typedef null_data_format sit_snd_set_mute_rsp;

typedef enum
{
    SIT_SND_MUTE_MODE_DISABLE = 0x00,
    SIT_SND_MUTE_MODE_ENABLE
}sit_snd_mute_mode_e_type;

/*
    SIT_GET_MUTE (RCM ID = 0x0A01)
*/
typedef null_data_format sit_snd_get_mute_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 mute_state;
}sit_snd_get_mute_rsp;

/*
    SIT_IND_RINGBACK_TONE (RCM ID = 0x0A02)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    INT32 ringback_state;
}sit_snd_ringback_tone_ind;

typedef struct
{
    RCM_IND_HEADER hdr;
    INT32 ringback_state;
    INT32 flag;
}sit_snd_ringback_tone_with_flag_ind;

typedef enum
{
    SIT_SND_RINGBACK_STATE_STOP = 0x00,
    SIT_SND_RINGBACK_STATE_START
}sit_snd_ringback_state_e_type;

/*
    SIT_SET_VOLUME (RCM ID = 0x0A03)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 volume;    // up to AP, ex) Samsung : 0 ~ 5
}sit_snd_set_volume_req;

typedef null_data_format sit_snd_set_volume_rsp;

/*
    SIT_GET_VOLUME (RCM ID = 0x0A04)
*/
typedef null_data_format sit_snd_get_volume_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 volume;
}sit_snd_get_volume_rsp;

/*
    SIT_SET_AUDIOPATH (RCM ID = 0x0A05)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 audiopath; // refet sit_snd_audiopath_mode
}sit_snd_set_audiopath_req;

typedef null_data_format sit_snd_set_audiopath_rsp;

/*
    SIT_GET_AUDIOPATH (RCM ID = 0x0A06)
*/
typedef null_data_format sit_snd_get_audiopath_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 audiopath;
}sit_snd_get_audiopath_rsp;


/*
    SIT_SET_MULTIMIC (RCM ID = 0x0A07)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 multiMICmode; // refet sit_snd_multiMIC_mode
}sit_snd_set_multimic_req;

typedef null_data_format sit_snd_set_multimic_rsp;

/*
    SIT_GET_AUDIOPATH (RCM ID = 0x0A08)
*/
typedef null_data_format sit_snd_get_multimic_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 multiMICmode;
}sit_snd_get_multimic_rsp;

/*
    SIT_SWITCH_VOICE_CALL_AUDIO (RCM ID = 0x0A09)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE active_call_sim_info;
}sit_snd_switch_voice_call_audio_req;

typedef null_data_format sit_snd_switch_voice_call_audio_rsp;

typedef enum
{
    SIT_CALL_IS_ACTIVATED_BY_SIM1 = 0x00,
    SIT_CALL_IS_ACTIVATED_BY_SIM2
}sit_snd_active_call_sim_info_e_type;

/*
    SIT_SET_AUDIO_CLK (RCM ID = 0x0A0A)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE clock_mode;
}sit_snd_set_clock_mode_req;

typedef enum
{
    SITRIL_CLOCKMODE_TURNOFF_I2S = 0x00,    //Turn off I2S clock
    SITRIL_CLOCKMODE_TURNON_I2S = 0x01,     //Turn on I2S clock
} sit_snd_clock_mode_e_type;

/*
    SIT_SET_AUDIO_LOOPBACK (RCM ID = 0x0A0B)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 on_off;
    INT32 audio_path;
}sit_snd_set_loopback_req;

typedef enum
{
    SITRIL_AUDIO_LOOPBACK_STOP = 0,    //0: stop loop back test
    SITRIL_AUDIO_LOOPBACK_START = 1,    //1: start loop back test
} sit_snd_loopback_onoff_e_type;

typedef enum
{
    SITRIL_AUDIO_LOOPBACK_PATH_NA = 0,    //0: N/A
    SITRIL_AUDIO_LOOPBACK_PATH_HANDSET = 1,    //1: handset
    SITRIL_AUDIO_LOOPBACK_PATH_HEADSET = 2,    //2: headset
    SITRIL_AUDIO_LOOPBACK_PATH_HANDSFREE = 3,    //3: handsfree
    SITRIL_AUDIO_LOOPBACK_PATH_BT = 4,    //4: Bluetooth
    SITRIL_AUDIO_LOOPBACK_PATH_STEREO_BT = 5,    //5: stereo Bluetooth
    SITRIL_AUDIO_LOOPBACK_PATH_SPK = 6,    //6: speaker phone
    SITRIL_AUDIO_LOOPBACK_PATH_35PI_HEADSET = 7,    //7: 3.5pi headset
    SITRIL_AUDIO_LOOPBACK_PATH_BT_NS_EC_OFF = 8,    //8: BT NS/EC off
    SITRIL_AUDIO_LOOPBACK_PATH_WB_BT = 9,    //9: WB Bluetooth
    SITRIL_AUDIO_LOOPBACK_PATH_WB_BT_NS_EC_OFF = 10,    //10: WB BT NS/EC OFF
    SITRIL_AUDIO_LOOPBACK_PATH_HANDSET_HAC = 11,    //11: handset HAC
    SITRIL_AUDIO_LOOPBACK_PATH_LINEOUT = 12,                    // Lineout

    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_HANDSET = 65,  //65: VOLTE handset
    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_HEADSET = 66,  //66: VOLTE headset
    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_HANDSFREE = 67,    //67: VOLTE handsfree
    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_BT = 68,   //68: VOLTE Bluetooth
    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_STEREO_BT = 69,    //69: VOLTE stereo Bluetooth
    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_SPK = 70,  //70: VOLTE speaker phone
    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_35PI_HEADSET = 71, //71: VOLTE 3.5pi headset
    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_BT_NS_EC_OFF = 72, //72: VOLTE BT NS/EC off
    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_WB_BT = 73,    //73: VOLTE WB Bluetooth
    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_WB_BT_NS_EC_OFF = 74,  //74: VOLTE WB BT NS/EC OFF
    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_VOLTE_HANDSET_HAC = 75,    // VoLTE : Phone MIC -> RECEIVER for HAC(Hearing Aid Compatibility
    SITRIL_AUDIO_LOOPBACK_PATH_VOLTE_LINEOUT = 76,              // VoLTE Lineout

    SITRIL_AUDIO_LOOPBACK_PATH_HEADSET_MIC1 = 129,  //129: Headset ? MIC1
    SITRIL_AUDIO_LOOPBACK_PATH_HEADSET_MIC2 = 130,  //130: Headset ? MIC2
    SITRIL_AUDIO_LOOPBACK_PATH_HEADSET_MIC3 = 131,  //131: Headset ? MIC3
} sit_snd_loopback_path_e_type;

/*
    SIT_IND_WB_AMR_REPORT = 0x0A0C
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE status;
}sit_snd_wb_amr_report_ind;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE status;
    BYTE call_type;
}sit_snd_wb_amr_report_with_calltype_ind;

typedef enum
{
    SITRIL_AUDIO_WB_AMR_REPORT_STATUS_OFF = 0,   //0x00 : WB-AMR Off
    SITRIL_AUDIO_WB_AMR_REPORT_STATUS_ON = 1,    //0x01 : WB-AMR On
    SITRIL_AUDIO_WB_AMR_REPORT_STATUS_NB = 0,    // Narrow Band
    SITRIL_AUDIO_WB_AMR_REPORT_STATUS_WB = 1,    // Wide Band
    SITRIL_AUDIO_WB_AMR_REPORT_STATUS_SWB = 8,   // Super WB
    SITRIL_AUDIO_WB_AMR_REPORT_STATUS_FB = 9,    // Full Band
} sit_snd_wb_amr_report_status_e_type;

enum {
    SIT_AUDIO_CALL_TYPE_UNKNOWN = 0,
    SIT_AUDIO_CALL_TYPE_GSM     = 1,
    SIT_AUDIO_CALL_TYPE_CDMA    = 2,
    SIT_AUDIO_CALL_TYPE_IMS     = 4,
    SIT_AUDIO_CALL_TYPE_OTHERS  = 8,
};

/*
    SIT_SET_WB_CAPABILITY = 0x0A0D
*/
typedef struct
{
    RCM_HEADER hdr;
    INT32 wbamr;
}sit_snd_set_wbmar_capability_req;

typedef enum
{
    SITRIL_AUDIO_WMAMR_3GNARROW_2GNARROW = 0,    //0: 3G Narrow & 2G Narrow
    SITRIL_AUDIO_WMAMR_3GWIDE_2GNARROW = 1,    //1: 3G Wide & 2G Narrow
    SITRIL_AUDIO_WMAMR_3GNARROW_2GWIDE = 2,    //2: 3G Narrow & 2G Wide
    SITRIL_AUDIO_WMAMR_3GWIDE_2GWIDE = 3,    //3: 3G Wide & 2G Wide

    SITRIL_AUDIO_WMAMR_MAX
} sit_snd_wbamr_capability_e_type;

typedef null_data_format sit_snd_set_wbmar_capability_rsp;

/*
    SIT_SET_WB_CAPABILITY = 0x0A0E
*/
typedef null_data_format sit_snd_get_wbmar_capability_req;

typedef sit_snd_set_wbmar_capability_req sit_snd_get_wbmar_capability_rsp;

/*
    SIT_IND_RESEND_IN_CALL_MUTE (RCM ID = 0x0A0F)
*/
typedef null_ind_data_format sit_snd_resend_in_call_mute_ind;

/*
    SIT_SET_IMS_CONFIGURATION = 0x0B00
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE ConfigSelection;
    BYTE data[120];
}sit_ims_set_conf_req;

/*
    SIT_GET_IMS_CONFIGURATION = 0x0B01
*/
typedef struct
{
    RCM_HEADER hdr;
}sit_ims_get_conf_req;

//for get channel
typedef struct
{
    BYTE ChannelNum;
    BYTE Type;
}sit_ims_get_conf_codec;

//for get option
typedef struct
{
    BYTE ChannelNum;
}sit_ims_get_conf_option;

/*
    SIT_SET_EMERGENCY_CALL_STATUS = 0x0712
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE Status;
    BYTE Rat;
}sit_net_set_emergency_call_status;

/*
    SIT_IMS_GEN_REASONE_RESPONSE
    Using SIT_SET_IMS_SETUP (RCM ID = 0x0B00)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    UINT16 aims_call_type;
    BYTE aims_call_id;
    //skip below
}sit_ind_aims_call_ring;

typedef struct
{
    RCM_HEADER hdr;
    BYTE result;
    BYTE fail_reason;
}sit_ims_gen_reason_rsp;

/*
    SIT_IMS_GEN_REASONE_RESPONSE
    Using SIT_IMS_CHANNEL_STATUS = 0x0B01
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE result;
}sit_ims_gen_rsp;

/*
    SIT_IND_AIMS_REGISTRATION
    Using SIT_IND_AIMS_REGISTRATION = 0x0D05
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE state;
    BYTE feature;
    BYTE ecmp_state;
}sit_ims_ind_reg;

typedef enum
{
    SITRIL_AIMS_IMSREG_STATE_NOT_REGISTERED = 0x01,   //0x01 : AIMS_NOT_REGISTERED
    SITRIL_AIMS_IMSREG_STATE_REGISTERED = 0x02, //0x02 : AIMS_REGISTERED
    SITRIL_AIMS_IMSREG_STATE_LIMITED_REGISTERED = 0x03, //0x03 : AIMS_LIMITED_REGISTERED
    SITRIL_AIMS_IMSREG_STATE_NOT_REGISTERED_E911 = 0x04,    //0x04 : AIMS_NOT_REGISTERED_E911,
    SITRIL_AIMS_IMSREG_STATE_REGISTERED_E911 = 0x05,    //0x05 : AIMS_REGISTERED_E911

    SITRIL_AIMS_IMSREG_STATE_MAX
} sit_aims_imsreg_state_e_type;

/*
    SIT_GET_ATR (RCM ID = 0x0212)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE result;
    BYTE atr_len;
    BYTE atr[MAX_ATR_LEN];
}sit_id_get_atr_rsp;


/*
    SIT_CALL_CONFIRM_FEATURE_SET_REQ (RCM ID = 0x000E)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE enable; // 0x00: Disable, 0x01: Enable
}sit_call_set_call_confirm_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE result; // 0x00: Fail, 0x01: Success
}sit_call_set_call_confirm_rsp;

/*
    SIT_CALL_CONFIRM (RCM ID = 0x000F)
*/

typedef struct
{
    RCM_HEADER hdr;
    BYTE result; // 0x00: Fail, 0x01: Success
}sit_call_send_call_confirm_rsp;

//AIMS support start ---------------------

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[523];
}sit_aims_dial_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[3];
}sit_aims_answer_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[3];
}sit_aims_hangup_req;

typedef struct
{
    BYTE aims_call_id;
    BYTE aims_call_type;
    BYTE hangup_reason;
}sit_aims_hangup_req_ex;

typedef struct
{
    RCM_IND_HEADER hdr;
    UINT16 aims_call_type;
    BYTE aims_call_id;
    BYTE aims_call_state;
    //skip below
}sit_ind_aims_call_status;

typedef struct
{
    RCM_HEADER hdr;
}sit_aims_deregistration_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[903];
}sit_set_aims_hidden_menu_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[23];
}sit_set_aims_ims_pdn_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[2];
}sit_aims_call_manage_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[3];
}sit_aims_send_dtmf_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[10];
}sit_set_aims_frame_time_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[2];
}sit_get_aims_frame_time_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[2];
}sit_aims_modify_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[4];
}sit_aims_response_modify_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[10];
}sit_aims_time_info_req;

typedef struct
{
    INT32 number_len;
    BYTE number[513];
}sit_aims_conf_call_user_info;

typedef struct
{
    RCM_HEADER hdr;
    BYTE conf_call_id;
    INT16 number_of_participant;
    sit_aims_conf_call_user_info *user_info;
}sit_aims_conf_call_add_user;

typedef struct
{
    RCM_HEADER hdr;
    INT16 number_of_participant;
    sit_aims_conf_call_user_info *user_info;
}sit_aims_enhanced_conf_call;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[67];
}sit_aims_get_call_forward_status;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[67];
}sit_aims_set_call_forward_status;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[14];
}sit_aims_get_call_waiting;

typedef struct
{
    int service_class;
    BYTE current_time_info[10];
}sit_aims_get_call_waiting_ex;

typedef struct
{
    RCM_HEADER hdr;
    BYTE result;
    BYTE error_code;
    BYTE retry_timer;
    int service_status;
    int service_class;
}sit_aims_get_call_waiting_rsp;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[18];
}sit_aims_set_call_waiting;

typedef struct
{
    int service_status;
    int service_class;
    BYTE current_time_info[10];
}sit_aims_set_call_waiting_ex;

typedef struct
{
    RCM_HEADER hdr;
    BYTE result;
    BYTE error_code;
    BYTE retry_timer;
}sit_aims_set_call_waiting_rsp;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[18];
}sit_aims_get_call_barring;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[27];
}sit_aims_set_call_barring;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[258];
}sit_aims_send_sms;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[258];
}sit_aims_send_expect_more;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[9];
}sit_aims_send_sms_ack;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[250];
}sit_aims_send_ack_incoming_sms;

typedef struct
{
    RCM_HEADER hdr;
    BYTE data[22];
}sit_aims_chg_barring_pwd;

//AIMS support end ---------------------

/*
    SIT_VSIM_NOTIFICATION (RCM ID = 0x0E00)
*/
typedef struct
{
    RCM_HEADER hdr;
    int tid;
    int event_id;
    int sim_type;
}sit_vsim_notification_req;

typedef null_data_format sit_vsim_notification_rsp;

typedef enum
{
    SIT_VSIM_NOTI_EN_EXTERNAL_SIM = 0x00,
    SIT_VSIM_NOTI_DIS_EXTERNAL_SIM = 0x01,
    SIT_VSIM_NOTI_PLUG_OUT = 0x02,
    SIT_VSIM_NOTI_PLUG_IN = 0x03,
}sit_vsim_notification_e_type;

typedef enum
{
    SIT_VSIM_SIM_TYPE_LOCAL = 0,
    SIT_VSIM_SIM_TYPE_REMOTE = 1
}sit_vsim_sim_type_e_type;

/*
    SIT_IND_VSIM_OPERATION (RCM ID = 0x0E01)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    int tid;
    int event_id;
    int result;
    int datalength;
    BYTE data[MAX_VSIM_DATA_LEN];
}sit_vsim_opertaion_ind;

typedef enum
{
    SIT_VSIM_OPERATION_ATR = 0x00,
    SIT_VSIM_OPERATION_APDU = 0x01,
    SIT_VSIM_OPERATION_POWERDOWN = 0x02,
}sit_vsim_operation_e_type;

/*
    SIT_VSIM_OPERATION (RCM ID = 0x0E02)
*/
typedef struct
{
    RCM_HEADER hdr;
    int tid;
    int event_id;
    int result;
    int datalength;
    BYTE data[MAX_VSIM_DATA_LEN];
}sit_vsim_opertaion_req;

typedef null_data_format sit_vsim_opertaion_rsp;

// OEM Common

/*
    SIT_OEM_STORE_ADB_SERIAL_NUMBER_REQ (RCM ID = 0x4011)
*/
typedef struct
{
    RCM_HEADER hdr;
    char adbSerialNumber[MAX_ADB_SERIAL_NUMBER];
}sit_oem_store_adb_serial_number_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE rcmError;
}sit_oem_store_adb_serial_number_rsp;

/*
    SIT_OEM_READ_ADB_SERIAL_NUMBER_REQ (RCM ID = 0x4012)
*/
typedef null_data_format sit_oem_read_adb_serial_number_req;

typedef struct
{
    RCM_HEADER hdr;
    char adbSerialNumber[MAX_ADB_SERIAL_NUMBER];
}sit_oem_read_adb_serial_number_rsp;

/*
 *   SIT_OEM_GET_SAR_STATE (RCM ID = 0x4100)
 */
typedef null_data_format sit_misc_sar_get_sar_state_req;

typedef struct
{
    RCM_HEADER hdr;
    INT32 sar_state;
}sit_misc_sar_get_sar_state_rsp;

/*
 *   SIT_OEM_SET_SAR_STATE (RCM ID = 0x4101)
 */
typedef struct
{
    RCM_HEADER hdr;
    INT32 sar_status;
}sit_misc_sar_set_sar_state_req;

/*
 *   SIT_OEM_IND_RF_CONNECTION (RCM ID = 0x4102)
 */
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE rf_state;
}sit_misc_sar_rf_connection_ind;

/*
   SIT_OEM_SET_SVN (RCM ID = 0x4103)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE sv_number[2];
}sit_id_set_sv_number_req;

/*
 *   SIT_OEM_GET_SIM_LOCK_INFO (RCM ID = 0x4104)
 */
typedef null_data_format sit_sim_get_sim_lock_info_req;

enum { SIT_STATUS_UNLOCKED, SIT_STATUS_LOCKED, };
enum {
    SIT_LOCK_TYPE_UNKNOWN = -1,
    SIT_LOCK_TYPE_UNLOCKED,
    SIT_LOCK_TYPE_PN,
    SIT_LOCK_TYPE_PU,
    SIT_LOCK_TYPE_SP,
    SIT_LOCK_TYPE_CP,
};
enum {
    SIT_LOCK_CODE_DEFAULT_SIZE = 2,
    SIT_LOCK_CODE_PN_SIZE = 6,
    SIT_MAX_LOCK_CODE_DATA = 1008
};

typedef struct
{
    RCM_HEADER hdr;
    BYTE policy;
    BYTE status;
    BYTE lockType;
    BYTE maxRetryCount;
    BYTE reaminCount;
    UINT16 lockCodeCount;
    char lockCode[SIT_MAX_LOCK_CODE_DATA];
}sit_sim_get_sim_lock_info_rsp;

enum {
    SIT_NW_INFO_SCREEN_STATE = 1,
    SIT_NW_INFO_CELL_INFO = 2,
    SIT_NW_INFO_CA_MIMO_HORXD = 3,
    SIT_NW_INFO_VOLTE_INFO = 4,
    SIT_NW_INFO_CDRX = 5,
    SIT_NW_INFO_SET_BAND = 7,
    SIT_NW_INFO_HW_BAND = 8,
};

/*
 * SIT_OEM_SET_CA_BW_FILTER (RCM ID = 0x4105)
 */
typedef struct
{
    RCM_HEADER hdr;
    BYTE enable;
}sit_oem_set_ca_bw_filter_req;

/*
 * SIT_OEM_IND_CA_BW_FILTER (RCM ID = 0x4106)
 */
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE ca_config;
    UINT16 num_resource_block;
}sit_oem_ca_bw_filter_ind;

/*
 * SIT_OEM_NW_INFO (RCM ID = 0x4400)
 */
typedef struct
{
    RCM_HEADER hdr;
    int command;
    unsigned int length;     // size of data
    char data[0];   // variable
}sit_oem_nw_info_req;

typedef struct
{
    RCM_HEADER hdr;
    int command;
    unsigned int length;     // size of data
    char data[0];   // variable
}sit_oem_nw_info_rsp;

enum {
    SIT_NW_INFO_MIMO_INFO = 1,
    SIT_NW_INFO_SCG_FAIL_INFO = 2,
    SIT_NW_INFO_NR_CELL_INFO = 3,
};

/*
 * SIT_OEM_IND_NW_INFO (RCM ID = 0x4401)
 */
typedef struct
{
    RCM_IND_HEADER hdr;
    int command;
    unsigned int length;     // size of data
    char data[0];   // variable
}sit_oem_nw_info_ind;

/*
 * SIT_OEM_SET_FORBID_LTE_CELL (RCM ID = 0x4403)
 */
typedef struct
{
    RCM_HEADER hdr;
    int mode;
    int cellid;
    int forbidden_timer;
    char plmn[6];
}sit_oem_set_forbid_lte_cell_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE result;
}sit_oem_set_forbid_lte_cell_rsp;

/*
 * SIT_OEM_SET_FUNC_SWITCH_REQ (RCM ID = 0x4405)
 */
typedef struct
{
    RCM_HEADER hdr;
    /*
    0x01 = CA
    0x02 = MIMO
    0x03 = HORXD
    0x04 = HPUE
    0x05 = VOLTE
    0x06 = CDRX
    */
    int feature;
    /*
    0x00 = Disable(off)
    0x01 = Enable(on)
    */
    BYTE enable;
}sit_oem_set_func_switch_req;

typedef struct
{
    RCM_HEADER hdr;
    /*
    0x01 : Success
    0x00 : Failure
    */
    BYTE result;
}sit_oem_set_func_switch_rsp;

/*
 * SIT_OEM_SET_RTP_PKTLOSS_THRESHOLD (RCM ID = 0x4406)
 */
typedef struct
{
    RCM_HEADER hdr;
    BYTE interval;
    BYTE pktLossThr;
}sit_oem_set_rtp_pktloss_thr_req;

typedef null_data_format sit_oem_set_rtp_pktloss_thr_rsp;

/*
 * SIT_OEM_IND_RTP_PKTLOSS_THRESHOLD (RCM ID = 0x4407)
 */
typedef null_ind_data_format sit_oem_rtp_pktloss_thr_ind;

/*
 * SIT_OEM_SET_PDCP_DISCARD_TIMER (RCM ID = 0x4408)
 */
typedef struct
{
    RCM_HEADER hdr;
    /*
    PDCP discard timer value (ms)
    if the value is 0, that means control off.
    */
    int discardTimer;
}sit_oem_set_pdcp_discard_timer_req;

/*
 * SIT_OEM_GET_CQI_INFO (RCM ID = 0x4409)
 */
typedef struct
{
    RCM_HEADER hdr;
    INT16 type;
    INT16 cqi_info0;
    INT16 cqi_info1;
    INT16 ri;
}sit_oem_get_cqi_info_rsp;

/*
 * SIT_OEM_SET_SAR_SETTING (RCM ID = 0x440A)
 */
typedef struct
{
    RCM_HEADER hdr;
    INT32 dsi;
}sit_oem_set_sar_setting_req;

/*
 * SIT_OEM_SET_GMO_SWITCH (RCM ID = 0x440B)
 */
typedef struct
{
    RCM_HEADER hdr;
    INT32 feature;
}sit_oem_set_gmo_switch_req;

/*
 * SIT_OEM_SET_TCS_FCI_REQ (RCM ID = 0x440C)
 */
typedef struct
{
    RCM_HEADER hdr;
    BYTE state;
    BYTE fci[MAX_FCI_LEN];
}sit_oem_set_tcs_fci_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE result;
}sit_oem_set_tcs_fci_rsp;

/*
 * SIT_OEM_GET_TCS_FCI_INFO (RCM ID = 0x440D)
 */
typedef struct
{
    RCM_HEADER hdr;
    BYTE fci[MAX_FCI_LEN];
}sit_oem_get_tcs_fci_info_rsp;

/*
    SIT_OEM_IND_ENDC_CAPABILITY (RCM ID = 0x440E)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE endc_capability;
    BYTE cause;
}sit_oem_endc_capability_ind;

/*
 *   SIT_OEM_SET_ACTIVATE_VSIM (RCM ID = 0x4402)
 */

typedef struct
{
    RCM_HEADER hdr;
    BYTE simSlot;    // 1 = SLOT_1, 2 = SLOT_2, 3 = SLOT_3;
    BYTE iccidLen;
    BYTE iccid[MAX_ICCID_STRING_LEN];  // ASCII
    BYTE imsiLen;
    BYTE imsi[MAX_IMSI_LEN];           // ASCII
    BYTE hplmn[MAX_PLMN_LEN];          // ASCII
    BYTE vsimState;    // 0 = deactivate, 1 = activate;
    BYTE vsimCardType;    // 1 = CDMA vsim, 2 = 2G vsim, 3 = 3G vsim, 4 = 4G vsim
}sit_oem_set_activate_visim_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE result;    // 0 = activate_fail, 1 = activate_success
}sit_oem_set_activate_visim_rsp;

#ifdef SUPPORT_CDMA
// Call & SS
/*
    SIT_SET_CDMA_VOICE_PRIVACY_MODE (RCM ID =0x0011)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE vpMode;
}sit_call_set_cdma_voice_privacy_mode_req;

typedef null_data_format sit_call_set_cdma__voice_privacy_mode_rsp;
// "response" is NULL

/*
    SIT_GET_CDMA_VOICE_PRIVACY_MODE (RCM ID =0x0012)
*/
typedef null_data_format sit_call_get_preferred_voice_privacy_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE status;
}sit_call_get_preferred_voice_privacy_mode_rsp;

/*
    SIT_CDMA_BURST_DTMF (RCM ID =0x0015)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE dtmf_len;
    BYTE dtmf_digit[MAX_BURST_DTMF_LEN];
    BYTE on_len;
    BYTE dtmf_on_length[MAX_BURST_DTMF_ON_OFF_LEN];
    BYTE off_len;
    BYTE dtmf_off_length[MAX_BURST_DTMF_ON_OFF_LEN];
}sit_call_cdma_burst_dtmf_req;

typedef null_ind_data_format smi_call_cdma_burst_dtmf_rsp;

/*
    SIT_IND_OTA_PROVISION_STATUS (RCM ID =0x0016)
*/
typedef enum
{
    SIT_OTA_TYPE_OTASP = 1,
    SIT_OTA_TYPE_OTAPA,
}sit_ota_type;

typedef enum
{
    SIT_OTASP_STATUS_OK_SPL_UNLOCKED = 1,
    SIT_OTASP_STATUS_OK_AKEYEX,
    SIT_OTASP_STATUS_OK_SSDUPDT,
    SIT_OTASP_STATUS_OK_NAMDWNLD,
    SIT_OTASP_STATUS_OK_MDNDWNLD,
    SIT_OTASP_STATUS_OK_IMSIDWNLD,
    SIT_OTASP_STATUS_OK_PRLDWNLD,
    SIT_OTASP_STATUS_OK_COMMIT,
    SIT_OTASP_STATUS_OK_PROGRAMMING,
    SIT_OTASP_STATUS_SUCCESSFUL,
    SIT_OTASP_STATUS_UNSUCCESSFUL,
    SIT_OTASP_STATUS_OK_OTAPAVERIFY,
    SIT_OTASP_STATUS_PROGRESS,
    SIT_OTASP_STATUS_FAILURES_EXCESS_SPC,
    SIT_OTASP_STATUS_LOCK_CODE_PASSWORD_SET,
}sit_otasp_status_type;

typedef enum
{
    SIT_OTAPA_STATUS_CALL_STOP_MODE = 0,
    SIT_OTAPA_STATUS_CALL_START_MODE,
}sit_otapa_status_type;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE otaType;
    BYTE otaStatus;
}sit_ota_provision_status_ind;

/*
    SIT_IND_CDMA_INFO_REC (RCM ID =0x0017)
*/
typedef enum
{
    SIT_CDMA_DISPLAY_INFO_REC = 0x0,
    SIT_CDMA_CALLED_PARTY_NUMBER_INFO_REC = 0x1,
    SIT_CDMA_CALLING_PARTY_NUMBER_INFO_REC = 0x2,
    SIT_CDMA_CONNECTED_NUMBER_INFO_REC = 0x3,
    SIT_CDMA_SIGNAL_INFO_REC = 0x4,
    SIT_CDMA_REDIRECTING_NUMBER_INFO_REC = 0x5,
    SIT_CDMA_LINE_CONTROL_INFO_REC = 0x6,
    SIT_CDMA_EXTENDED_DISPLAY_INFO_REC = 0x7,
}sit_cdma_info_rec_name;

typedef enum
{
    SIT_REDIRECTING_REASON_UNKNOWN = 0x0,
    SIT_REDIRECTING_REASON_CALL_FORWARDING_BUSY = 0x1,
    SIT_REDIRECTING_REASON_CALL_FORWARDING_NO_REPLY = 0x2,
    SIT_REDIRECTING_REASON_CALLED_DTE_OUT_OF_ORDER = 0x9,
    SIT_REDIRECTING_REASON_CALL_FORWARDING_BY_THE_CALLED_DTE = 0xA,
    SIT_REDIRECTING_REASON_CALL_FORWARDING_UNCONDITIONAL = 0xF,
    SIT_REDIRECTING_REASON_RESERVED
}sit_cdma_redirecting_reason;

typedef struct
{
    BYTE alpha_len;
    BYTE alpha_buf[MAX_ALPHA_INFO_BUF_LEN];
}sit_cdma_display_info;

typedef struct
{
    BYTE len;
    BYTE buf[MAX_NUMBER_INFO_BUFFER_LEN];
    BYTE number_type;
    BYTE number_plan;
    BYTE pi;
    BYTE si;
}sit_cdma_number_info;

typedef struct
{
    BYTE is_present;
    BYTE signal_type;
    BYTE alert_pitch;
    BYTE signal;
}sit_cdma_signal_info;

typedef struct
{
    sit_cdma_number_info redirecting_number;
    BYTE reason;
}sit_cdma_redirecting_number_info;

typedef struct
{
    BYTE line_ctrl_polarity_included;
    BYTE line_ctrl_toggle;
    BYTE line_ctrl_reverse;
    BYTE line_ctrl_power_denial;
}sit_cdma_line_control_info;

typedef struct
{
    BYTE cdma_info_name;
    union
    {
        sit_cdma_display_info display;
        sit_cdma_number_info number;
        sit_cdma_signal_info signal;
        sit_cdma_redirecting_number_info redirecting_number;
        sit_cdma_line_control_info line_control;
    }cdma_info;
}sit_cdma_info_rec_ind;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE num_of_info_recs;
    sit_cdma_info_rec_ind info_rec[MAX_NUMBER_OF_INFO_RECS];
}sit_cdma_information_records;

/*
    SIT_IND_EMERGENCY_SUPPORT_RAT_MODE (RCM ID =0x0018)
*/
typedef enum
{
    SIT_SUPPORT_RAT_MODE_3GPP = 0,
    SIT_SUPPORT_RAT_MODE_3GPP2,
    SIT_SUPPORT_RAT_MODE_ALL,
}sit_emergency_support_rat_mode_type;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE available_tech;
}sit_call_emergency_support_rat_mode_ind;

/*
    SIT_SET_CDMA_FLASH_INFO (RCM ID =0x0514)
 */
typedef struct
{
    RCM_HEADER hdr;
    BYTE flash_len;
    BYTE flash[MAX_FLASH_LEN];
}sit_ss_set_cdma_flash_info_req;

/*
    SIT_IND_CDMA_CALL_WAITING (RCM ID =0x0515)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE num_len;
    BYTE number[MAX_DIAL_NUM];
    BYTE number_presentation;
    BYTE name_len;
    BYTE name[MAX_DIAL_NAME];
    sit_cdma_signal_info signal_info;
    BYTE number_type;
    BYTE number_plan;
}sit_ss_cdma_call_waiting_ind;

// Network
/*
   SIT_IND_AC_BARRING_INFO (RCM ID = 0x0720)
*/

#define SPECIAL_AC_LIST (5)
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE for_emc;
    BYTE for_mo_sig_factor;
    INT16 for_mo_sig_time;
    BYTE for_mo_sig_ac_list[SPECIAL_AC_LIST];
    BYTE for_mo_data_factor;
    INT16 for_mo_data_time;
    BYTE for_mo_data_ac_list[SPECIAL_AC_LIST];
    BYTE for_mmtel_voice_factor;
    INT16 for_mmtel_voice_time;
    BYTE for_mmtel_voice_ac_list[SPECIAL_AC_LIST];
    BYTE for_mmtel_video_factor;
    INT16 for_mmtel_video_time;
    BYTE for_mmtel_video_ac_list[SPECIAL_AC_LIST];
}sit_net_ac_barring_info_ind;

typedef struct {
    BYTE for_emc;
    BYTE for_mo_sig_factor;
    INT16 for_mo_sig_time;
    BYTE for_mo_sig_ac_list[SPECIAL_AC_LIST];
    BYTE for_mo_data_factor;
    INT16 for_mo_data_time;
    BYTE for_mo_data_ac_list[SPECIAL_AC_LIST];
    BYTE for_mmtel_voice_factor;
    INT16 for_mmtel_voice_time;
    BYTE for_mmtel_voice_ac_list[SPECIAL_AC_LIST];
    BYTE for_mmtel_video_factor;
    INT16 for_mmtel_video_time;
    BYTE for_mmtel_video_ac_list[SPECIAL_AC_LIST];
} AC_BARRING_INFO;

/*
   SIT_SET_CDMA_ROAMING_PREFERENCE (RCM ID = 0x0721)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT8 cdma_roaming_type;
}sit_net_set_cdma_roaming_req;

typedef null_data_format sit_net_set_cdma_roaming_rsp;

typedef enum
{
    SIT_CDMA_RM_HOME = 0,
    SIT_CDMA_RM_AFFILIATED,
    SIT_CDMA_RM_ANY,
}sit_net_cdma_roaming_type;

/*
   SIT_GET_CDMA_ROAMING_PREFERENCE (RCM ID = 0x0722)
*/
typedef null_data_format sit_net_query_cdma_roaming_rep;

typedef struct
{
    RCM_HEADER hdr;
    INT8 cdma_roaming_type;
}sit_net_query_cdma_roaming_rsp;

/*
    SIT_SET_CDMA_HYBRID_MODE (RCM ID = 0x723)
*/
typedef struct
{
    RCM_HEADER hdr;
    INT8 hybrid_mode;
}sit_net_set_cdma_hybrid_mode_req;

/*
    SIT_GET_CDMA_HYBRID_MODE (RCM ID = 0x724)
*/
typedef null_data_format sit_net_get_cdma_hybrid_mode_req;

typedef struct
{
    RCM_HEADER hdr;
    INT8 hybrid_mode;
}sit_net_get_cdma_hybrid_mode_rsp;

// SMS
/*
    SIT_CDMA_SEND_SMS (RCM ID = 0x0111)
*/
typedef struct
{
    RCM_HEADER hdr;
    UINT16 msg_len;
    BYTE msg[MAX_CDMA_SMS_MSG_SIZE];
}sit_sms_cdma_send_sms_req;

typedef struct
{
    RCM_HEADER hdr;
    INT16 msg_ref;
    BYTE error_class;
    BYTE error_code;
}sit_sms_cdma_send_sms_rsp;

/*
    SIT_CDMA_SEND_SMS_ACK (RCM ID = 0x0112)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE msg_tpid;
    BYTE error_class;
    BYTE error_code;
}sit_sms_cdma_send_sms_ack_req;

typedef null_ind_data_format sit_sms_cdma_send_sms_ack_rsp;

/*
    SIT_CDMA_GET_BCST_SMS_CFG (RCM ID = 0x0113)
*/
typedef null_data_format sit_sms_cdma_get_bcst_sms_cfg_req;

typedef struct
{
    INT16 svc_category;
    BYTE language;
}sit_sms_cdma_bcst_sms_cfg_item;

typedef struct
{
    RCM_HEADER hdr;
    BYTE bcst_info_num;
    sit_sms_cdma_bcst_sms_cfg_item cfgitem[MAX_CDMA_BCST_INFO_NUM];
}sit_sms_cdma_get_bcst_sms_cfg_rsp;

/*
    SIT_CDMA_SET_BCST_SMS_CFG (RCM ID = 0x0114)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE bcst_info_num;
    sit_sms_cdma_bcst_sms_cfg_item cfgitem[MAX_CDMA_BCST_INFO_NUM];
}sit_sms_cdma_set_bcst_sms_cfg_req;

typedef null_data_format sit_sms_cdma_set_bcst_sms_cfg_rsp;

/*
    SIT_CDMA_ACT_BCST_SMS (RCM ID = 0x0115)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE bcst_act;
}sit_sms_cdma_act_bcst_sms_req;

typedef null_data_format sit_sms_cdma_act_bcst_sms_rsp;

typedef enum
{
    SIT_SMS_CDMA_BCST_ACT_ACTIVATE = 0x00,
    SIT_SMS_CDMA_BCST_ACT_DEACTIVATE,

    SIT_SMS_CDMA_BCST_ACT_MAX
}sit_sms_cdma_bcst_act_e_type;

/*
    SIT_CDMA_WRITE_SMS_TO_RUIM (RCM ID = 0x0116)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE status;
    BYTE msg_len;
    BYTE msg[MAX_CDMA_SMS_RUIM_MSG_SIZE];
}sit_sms_cdma_write_sms_to_ruim_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE index;
}sit_sms_cdma_write_sms_to_ruim_rsp;

typedef enum
{
    // See 3GPP2 C.S0023 3.4.27
    SIT_RUIM_STATUS_RECEIVED_READ = 0x01,
    SIT_RUIM_STATUS_RECEIVED_UNREAD = 0x03,
    SIT_RUIM_STATUS_STORED_SENT = 0x05,
    SIT_RUIM_STATUS_STORED_UNSENT = 0x07,

    SIT_RUIM_STATUS_MAX
}sit_sms_cdma_status_e_type;

/*
    SIT_CDMA_DELETE_SMS_ON_RUIM (RCM ID = 0x0117)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE index;
}sit_sms_cdma_delete_sms_on_ruim_req;

typedef null_data_format sit_sms_cdma_delete_sms_on_ruim_rsp;

/*
    SIT_IND_CDMA_NEW_SMS (RCM ID = 0x0118)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE msg_tpid;
    UINT16 msg_len;
    BYTE msg[MAX_CDMA_SMS_MSG_SIZE];
}sit_sms_cdma_new_sms_ind;

/*
    SIT_IND_CDMA_RUIM_SMS_STORAGE_FULL (RCM ID = 0x0119)
*/
typedef null_ind_data_format sit_sms_cdma_ruim_sms_storage_full_ind;

/*
    SIT_IND_CDMA_VOICE_MSG_WAITING_INFO (RCM ID = 0x0120)
*/
typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE msg_count;
}sit_sms_cdma_voice_msg_waiting_info_ind;
#endif // SUPPORT_CDMA


// ##############################
// #### RCM Commands for GPS####
// ##############################

#define LPP_MAX_AGNSS_CNT       2
#define LPP_MAX_SVs_CNT         16

/*----------------------------------------------------------------
    GPS Control Plane MO Location Message
*/

/* QoS Type */
typedef struct
{
    UINT32              qosFlag;
    UINT8               horizontalAccuracy;
    BYTE                verticalCoordinateRequest;
    UINT8               verticalAccuracy;
    UINT8               responseTime;
}__attribute__((packed))sit_gps_qos_type;

/* GPS Satellit info Type */
typedef struct
{
    UINT8   sat_id;
    UINT8   iode;
}__attribute__((packed))sit_gps_sat_info_type;

typedef struct
{
    UINT8   beginWeek;
    UINT8   endWeek;
    UINT8   beginTow;
    UINT8   endTow;
}__attribute__((packed))sit_gps_ext_ephe_chk_type;

/* GPS Assistance Data Type */
typedef struct
{
    UINT32                      assistanceFlag;
    UINT16                      gpsWeek;
    UINT8                       gpsToe;
    UINT8                       nSat;
    UINT8                       toeLimit;
    sit_gps_sat_info_type       satInfo[15];
    UINT8                       gpsExtendedEphemeris;
    sit_gps_ext_ephe_chk_type   extEphemerisChk;
}__attribute__((packed))sit_gps_assistance_data_type;

/* Ellipsoid Point Type */
typedef struct {
    INT32                       latitude;
    INT32                       longitude;
}__attribute__((packed))sit_gps_ellipsoid_po_type;

/* Point Uncertain Circle Type */
typedef struct
{
    sit_gps_ellipsoid_po_type   point;
    INT8                        uncertainRadius;
}__attribute__((packed))sit_gps_po_unc_circle_type;

/* PointUncertain Ellipse Type */
typedef struct
{
    sit_gps_ellipsoid_po_type   point;
    INT8    semiMajorAxis;
    INT8    semiMinorAxis;
    INT8    orientationAngle;
    INT8    confidence;
}__attribute__((packed))sit_gps_po_unc_ellipse_type;

/* Point Altitude Uncertain Ellipsoid Type */
typedef struct
{
    sit_gps_ellipsoid_po_type   point;
    INT16                       altitude;
    INT8                        semiMajorAxis;
    INT8                        semiMinorAxis;
    INT8                        orientationAngle;
    INT8                        uncertainAltitude;
    INT8                        confidence;
}__attribute__((packed))sit_gps_po_alt_unc_ellipse_type;

/* Ellipsoid Arc Type */
typedef struct
{
    sit_gps_ellipsoid_po_type   point;
    INT16                       innerRadius;
    INT8                        uncertainRadius;
    INT8                        offsetAngle;
    INT8                        includedAngle;
    INT8                        confidence;
}__attribute__((packed))sit_gps_ellipsoid_arc_type;

/* Point Altitude Type */
typedef struct  {
    sit_gps_ellipsoid_po_type   point;
    INT16                       altitude;
}__attribute__((packed))sit_gps_ellipsoid_alt_type;

/* Polygon Type */
typedef struct
{
    INT8                        noOfPoints;
    sit_gps_ellipsoid_po_type   points[15];
}__attribute__((packed))sit_gps_polygon_type;

/* GPS Location Info Type  */
typedef struct
{
    UINT8   shape_type;
    sit_gps_po_unc_circle_type  p_unc_clrcle;
    sit_gps_po_unc_ellipse_type p_unc_ellipse;
    sit_gps_po_alt_unc_ellipse_type p_alt_unc_ellipse;
    sit_gps_ellipsoid_arc_type  ellipsoid_arc;
    sit_gps_ellipsoid_po_type   ellipsoid_po;
    sit_gps_ellipsoid_alt_type  ellipsoid_alt;
    sit_gps_polygon_type        polygon;
}__attribute__((packed))sit_gps_loc_info_type;

/* Deciphering Keys Type */
typedef struct
{
    UINT8   cipherKeyFlag;
    UINT8   currentDecipherKey[7];
    UINT8   nextDecipherKey[7];
}__attribute__((packed))sit_gps_deciphering_keys_type;

/*----------------------------------------------------------------
      Enhanced LTE Cell Info Message
*/

typedef struct {
    UINT8                                           flag;

    #define OEM_LTE_ECID_REQ_CAP                    0x01
    #define OEM_LTE_ECID_REQ_LOC                    0x02

    UINT8                                           responseTime;
} __attribute__((packed)) oem_gps_get_lpp_supl_ecid_Info;

typedef struct {
    UINT16                                          mcc;
    UINT16                                          mnc;
    UINT32                                          cellId;
    UINT16                                          tac;
} __attribute__((packed)) sit_cgi_info;

typedef struct {
    UINT8                                           bitMask;
    #define BM_LTE_MRL_CGI_INFO                     0x01
    #define BM_LTE_MRL_RSRP                         0x02
    #define BM_LTE_MRL_RSRQ                         0x04
    #define BM_LTE_MRL_EARFCN                       0x08

    UINT16                                          physCellId;
    sit_cgi_info                                    cgiInfo;
    UINT8                                           rsrp;
    UINT8                                           rsrq;
    UINT16                                          earfcn;
} __attribute__((packed)) sit_mrl_eutra_info;

/*----------------------------------------------------------------
    GSM/WCDMA Ehanced CELL Information Message (RRLP)
*/

typedef struct {
    UINT8                                   flag;
    #define OEM_WCDMA_ECID_REQ_CAP              0x01
    #define OEM_WCDMA_ECID_REQ_LOC              0x02

    UINT8                                   responseTime;
}  __attribute__((packed)) oem_gps_get_rrlp_supl_ecid_Info;

typedef struct {
    UINT16                                  aRfcn;
    UINT8                                   bSic;
    UINT8                                   rxLev;
}  __attribute__((packed)) sit_gsm_nmr_element;

typedef struct {
    sit_gsm_nmr_element                     nmr[15];
    UINT8                                   sizeOf;
}  __attribute__((packed)) sit_gsm_network_measured_results;

typedef struct {
    UINT8   bitMask;
    #define BM_2G_NETWORK_MEAS_RESULT       0x01
    #define BM_2G_TA                        0x02

    UINT16                                  refmcc;
    UINT16                                  refmnc;
    UINT16                                  reflac;
    UINT16                                  refci;
    sit_gsm_network_measured_results        networkMeasResult;
    UINT8                                   timingAdvance;
}  __attribute__((packed)) sit_gsm_cell_info;

typedef struct {
    UINT16                                  tA;
    UINT8                                   tAResolution;
    UINT8                                   chipRate;
}  __attribute__((packed)) sit_wcdma_timing_advance;

typedef struct {
    UINT8                                   modeSpecificInfo;
    #define WCDMA_FREQ_INFO_FDD             0x01
    #define WCDMA_FREQ_INFO_TDD             0x02
    UINT16                                  uarfcnUl;
    UINT16                                  uarfcnDl;
    UINT16                                  uarfcnNt;
}  __attribute__((packed)) sit_wcdma_frequency_info;

typedef struct {
    UINT16                                  primarycpichinfo;
    UINT8                                   cpichecn0;
    UINT8                                   cpichrscp;
    UINT8                                   pathloss;
}  __attribute__((packed)) sit_wcdma_cell_measured_results_fdd;

typedef struct {
    UINT8                                   cellparametersid;
    UINT8                                   proposedtgsn;
    UINT8                                   primaryccpch_rscp;
    UINT8                                   pathloss;
    UINT8                                   timeslotiscplist[14];
    UINT8                                   sizeoftimeslotiscplist;
}  __attribute__((packed)) sit_wcdma_cell_measured_results_tdd;

typedef struct {
    UINT8                                   modeSpecificInfo;
    #define WCDMA_CELLMEAS_RESULT_FDD       0x01
    #define WCDMA_CELLMEAS_RESULT_TDD       0x02

    UINT32                                  cellIdentity;
    union{
        sit_wcdma_cell_measured_results_fdd   fdd;
        sit_wcdma_cell_measured_results_tdd   tdd;
    }u_SpecificInfo;

}  __attribute__((packed)) sit_wcdma_cell_measured_results;

typedef struct {
    sit_wcdma_frequency_info                frequencyinfo;
    UINT8                                   utraCarrierRssi;
    sit_wcdma_cell_measured_results         cellMeasuredResultsList[16];
    UINT8                                   sizeOfCellMeasResultsList;
}  __attribute__((packed)) sit_wcdma_measured_results;

typedef struct {
    UINT8                                   bitMask;
    #define BM_3G_FREQUENCY_INFO            0x01
    #define BM_3G_PRIMARY_SCRAMBLING_CODE   0x02
    #define BM_3G_MEASURED_RESULTS_LIST     0x04
    #define BM_3G_CELL_PARAMETERS_ID        0x08
    #define BM_3G_TIMING_ADVANCE            0x10

    UINT16                                  refMcc;
    UINT16                                  refMnc;
    UINT32                                  refUc;

    sit_wcdma_frequency_info                frequencyInfo;
    UINT16                                  primaryScramblingCode;
    sit_wcdma_measured_results              measuredResultsList[8];
    UINT8                                   sizeOfMeasurementResultList;
    UINT8                                   cellParametersId;
    sit_wcdma_timing_advance                timingAdvance;
}  __attribute__((packed)) sit_wcdma_cell_info;


/*----------------------------------------------------------------
     GPS Assist Data Message
*/

/* GSM Time Type */
typedef struct
{
    BYTE        valid;
    UINT16      bcchCarrier;
    UINT16      bsic;
    UINT32      frameNumber;
    UINT16      timeSlot;
    UINT16      bitNumber;
}__attribute__((packed))sit_gps_gsm_time_type;

/* UTRAN GPS Reference Time Type */
typedef struct
{
    BYTE                        valid;
    UINT32                      cellFrames;
    BYTE                        choice_mode;
    UINT32                      UtranFdd;
    UINT32                      UtranTdd;
    UINT32                      sfn;
}__attribute__((packed))sit_gps_utran_gps_ref_time_type;

/* Utran Gps Uncertainty Type */
typedef struct
{
    BYTE        valid;
    UINT32      gpsTimeUncertainty;
}__attribute__((packed))sit_gps_utran_gps_unc_type;

/* Utran Drift Rate Type */
typedef struct
{
    BYTE        valid;
    INT32       driftRate;
}__attribute__((packed))sit_gps_drift_rate_type;

/* Utran Time Type  */
typedef struct
{
    sit_gps_utran_gps_ref_time_type     UtranGpsRefTime;
    sit_gps_utran_gps_unc_type          UtranGpsUncertainty;
    BYTE                                UtranSfnUncertainty;
    sit_gps_drift_rate_type             UtranDriftRate;
}__attribute__((packed))sit_gps_utran_time_type;

/* Gps Tow Assist Type */
typedef struct
{
    UINT16      satID;
    UINT16      tlmWord;
    BYTE        antiSpoofFlag;
    BYTE        alertFlag;
    UINT8       tmlReservedBits;
}__attribute__((packed))sit_gps_gps_tow_assist_type;

/* Reference Time Type */
typedef struct
{
    UINT32                  gpsTow;
    UINT32                  gpsWeek;
    UINT8                   nrOfSats;
    union
    {
        sit_gps_gsm_time_type       gsm_time;
        sit_gps_utran_time_type     UtranTime;
    } networkTimeInfo;
    sit_gps_gps_tow_assist_type GpsTowAssist[12];
}__attribute__((packed))sit_gps_ref_time_type;


/* Dgps Sat List Type */
typedef struct
{
    UINT8   satId;
    UINT16  iode;
    UINT8   udre;
    INT16   pseudoRangeCor;
    INT16   rangeRateCor;
}__attribute__((packed))sit_gps_dgps_sat_list_type;

/* Dgps Corrections Type*/
typedef struct
{
    UINT32              gpsTow;
    BYTE                status;
    UINT32              numberOfSat;
    sit_gps_dgps_sat_list_type  seqOfSatElement[16];
}__attribute__((packed))sit_gps_dgps_correction_type;


/* Navigation SubFrame Rsv Type*/
typedef struct
{
    UINT32  rsv1; // 0~838860
    UINT32  rsv2; // 0~16777215
    UINT32  rsv3; // 0~16777215
    UINT32  rsv4; // 0~65535
}__attribute__((packed))sit_gps_navi_subframe_rsv_type;

/* Navigation Ephemeris Type */
typedef struct
{
    UINT8                           ephemCodeOnL2; // 0~3
    UINT8                           ephemUra; // 0~15
    UINT8                           ephemSvHealth; // 0~63
    UINT16                          ephemIodc; // 0~1023
    UINT8                           ephemL2PFlag; // 0~1
    sit_gps_navi_subframe_rsv_type  NavigationSubFrameRsv;
    INT8                            ephemTgd; // -128~127
    UINT16                          ephemToc; // 0~37799
    INT8                            ephemAf2; // -128~12
    INT16                           ephemAf1; // -32768~32767
    INT32                           ephemAf0; // -2097152~2097151
    INT16                           ephemCrs; // -32768~32767
    INT16                           ephemDeltaN; // -32768~32767
    INT32                           ephemM0; // -2147483648~2147483647
    INT16                           ephemCuc; // -32768~32767
    UINT32                          ephemE; // 0~4294967295
    INT16                           ephemCus; // -32768~32767
    UINT32                          ephemAPowrHalf; // 0~4294967295
    UINT16                          ephemToe; // 0~37799
    INT8                            ephemFitFlag; // 0~1
    UINT8                           ephemAoda; // 0~31
    INT16                           ephemCic; // -32768~32767
    INT32                           ephemOmegaA0; // -2147483648~2147483647
    INT16                           ephemCis; // -32768~32767
    INT32                           ephemI0; // -2147483648~2147483647
    INT16                           ephemCrc; // -32768~32767
    INT32                           ephemW; // -2147483648~2147483647
    INT32                           ephemOmegaADot; // -8388608~8388607
    INT16                           ephemIDot; // -8192~8191
}__attribute__((packed))sit_gps_navi_ephe_type;

/* Navigtion Sat Info Type */
typedef struct
{
    UINT8                       satId;
    BYTE                        NavigationSatStatus;
    sit_gps_navi_ephe_type      NavigationEphemeris;
}__attribute__((packed))sit_gps_navi_sat_info_type;

/* Navigation Model Type */
typedef struct
{
    UINT32                      numberOfSat;
    sit_gps_navi_sat_info_type  NavigationSatInfo[16];
}__attribute__((packed))sit_gps_navi_model_type;

/* Iono Model Type */
typedef struct
{
    INT8    alfa0; // -128~127
    INT8    alfa1; // -128~127
    INT8    alfa2; // -128~127
    INT8    alfa3; // -128~127
    INT8    beta0; // -128~127
    INT8    beta1; // -128~127
    INT8    beta2; // -128~127
    INT8    beta3; // -128~127
}__attribute__((packed))sit_gps_iono_model_type;

/* Utc Model Type */
typedef struct
{
    INT32   utcA1; // -8388608~8388607
    INT32   utcA0; // -2147483648~2147483647
    UINT8   utcTot; // 0~255
    UINT8   utcWNt; // 0~255
    INT8    utcDeltaTls; // -128~127
    UINT8   utcWNlsf; // 0~255
    INT8    utcDN; // -128~127
    INT8    utcDeltaTlsf; // -128~127
}__attribute__((packed))sit_gps_utc_model_type;

/* Almanac Sat Info Type */
typedef struct
{
    INT8        dataId; // only for 3G, 0~3, if this value is -1, it means this value is invalid
    UINT8       satId;
    UINT16      almanacE; // 0~65536
    UINT8       almanacToa; // 0~255
    INT16       almanacKsii; // -32768~3276
    INT16       almanacOemgaDot; // -32768~3276
    UINT8       almanacSvHealth; // 0~255
    UINT32      almanacAPowerHalf; // 0~16777215
    INT32       almanacOmega0; // -8388608~8388607
    INT32       almanacW; // -8388608~8388607
    INT32       almanacM0; // -8388608~8388607
    INT16       almanacAf0; // -1024~1023
    INT16       almanacAf1; // -1024~1023
}__attribute__((packed))sit_gps_almanac_sat_info_type;

/* Almanac Type */
typedef struct
{
  UINT8                         almanacWNa; // 0~255
  UINT32                        numberOfSat;
  sit_gps_almanac_sat_info_type AlmanacSatInfo[64];
}__attribute__((packed))sit_gps_almanac_model_type;

/* Acquision Utran Time Type */
typedef struct
{
    sit_gps_utran_gps_ref_time_type     AcqUtranGpsRefTime;
    sit_gps_utran_gps_unc_type          AcqUtranGpsUncertainty;
}__attribute__((packed))sit_gps_acq_utran_time_type;

/* Acquisition Sat Info Type */
typedef struct
{
    UINT8                           satId;
    INT16                           doppler0; // -2048~2047 (real value is from -5120 to 5117.5 by step of 2.5)
    UINT8                           doppler1; // 0~63 (real value is from -0.966 to 0.483 by step of 0.023)
    UINT8                           dopplerUncertainty; // 0~7 (12.5, 25, 50, 100, 200)
    UINT16                          codePhase; // 0~1022
    UINT8                           intCodePhase; // 0~19
    UINT8                           gpsBitNumber; // 0~3
    UINT8                           codePhaseSearchWindow; // 0~15 (1023, 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192)
    UINT8                           azimuth; // 0~31, 11.25 degree resolution
    UINT8                           elevation; // 0~7, 11.25 degree resolution
}__attribute__((packed))sit_gps_acq_sat_info_type;

/* Acquisition Assist Type */
typedef struct
{
    UINT32      gpsTow;
    union
    {
        sit_gps_gsm_time_type   gsm_time;
        sit_gps_acq_utran_time_type AcqUtranTime;
    } acquisitionTimeInfo;
    UINT32      numberOfSat;
    sit_gps_acq_sat_info_type   lcsAcquisitionSatInfo[16];
}__attribute__((packed))sit_gps_acq_assist_type;

/* Real Time Integrity Type */
typedef struct
{
    UINT8       satId[16];
    UINT8       numOfSat;
}__attribute__((packed))sit_gps_r_time_int_type;

typedef struct
{
    UINT8 shapeType;
    UINT8 hemisphere;
    UINT16 altitude;
    UINT32 latitude;
    INT32 longitude;
    UINT8 directionOfAlt;
    UINT8 semiMajorUncert;
    UINT8 semiMinorUncert;
    UINT8 majorAxis;
    UINT8 altUncert;
    UINT8 confidence;
}__attribute__((packed))sit_gps_ref_loc_type;

/*----------------------------------------------------------------
    GPS Measure Position Message
*/

/* Accuracy Type */
typedef struct
{
    UINT32      flag;
    UINT8       horizontalAccuracy;
    UINT8       vertcalAccuracy;
}__attribute__((packed))sit_gps_m_accuracy_type;

/* GPS Measurement Element Type */
typedef struct
{
    UINT8               satId;
    UINT8               cno; // 0~63, unit of dB-Hz
    INT16               doppler; // -32768~32767, Hz and scale factor 0.2
    UINT16              wholeChips; // 0~1022
    UINT16              fracChips; // 0~1024
    BYTE                lcsMultiPath;
    UINT8               pseuRangeRmsErr; // 0~63
}__attribute__((packed))sit_gps_measuremet_element_type;

/* GPS Measure Type */
typedef struct
{
    UINT32              gpsTow; ///< GPS time of week [msec]
    UINT16              gpsWeek; ///< GPS week [0 .. 1023]
    UINT8               nrOfSats; ///< number of satellites [1 .. 16]
    sit_gps_measuremet_element_type GpsMeasure[16];
}__attribute__((packed))sit_gps_measure_type;

/* GPS Measure Location Info Type */
typedef struct
{
    UINT32              gpsTow; ///< GPS time of week [msec]
    UINT16              gpsWeek; ///< GPS week [0 .. 1023]
    BYTE                fixType; ///< Fix type. 2D(0x01) or 3D.
    sit_gps_loc_info_type   measured_loc_info;
}__attribute__((packed))sit_gps_measure_loc_info_type;

/*----------------------------------------------------------------
    GPS MTLR Notification Message
*/

/* MTLR Location Type */
typedef struct
{
    UINT16  deferredLocEventType;
    BYTE    locEstimateType;
}__attribute__((packed))sit_gps_mtlr_loc_type;

/* String type */
typedef struct
{
    UINT32  length;
    UINT8   val[63];
}__attribute__((packed))sit_gps_string_type;

/* DCS String Type */
typedef struct
{
    UINT8  dcs;
    sit_gps_string_type  str;
    BYTE  format_indicator;
}__attribute__((packed))sit_gps_dcs_string_type;

/* Code word Type */
typedef struct
{
    UINT8  dcs;
    sit_gps_string_type  str;
}__attribute__((packed))sit_gps_code_word_type;

/*----------------------------------------------------------------
    LPP Request Location Information
*/

/***CommonIEsRequestLocationInformation*/

typedef struct {
    UINT8 locationInformation;
    #define locationEstimateRequired        1
    #define locationMeasurementsRequired    2
    #define locationEstimatePreferred       3
    #define locationMeasurementsPreferred   4
}  __attribute__((packed)) sit_lpp_locationInformationType_Type;

typedef struct {
    UINT8                                   reportingAmount;
    #define ra1                         1
    #define ra2                         2
    #define ra4                         3
    #define ra8                         4
    #define ra16                        5
    #define ra32                        6
    #define ra64                        7
    #define ra_Infinity                 8
}  __attribute__((packed)) sit_lpp_reportingAmount_Type;

typedef struct {
    UINT8                                   reportingInterval;
    #define noPeriodicalReporting       1
    #define ri0_25                      2
    #define ri0_5                       3
    #define ri1                         4
    #define ri2                         5
    #define ri4                         6
    #define ri8                         7
    #define ri16                        8
    #define ri32                        9
    #define ri64                        10
}  __attribute__((packed)) sit_lpp_reportingInterval_Type;

typedef struct {
  sit_lpp_reportingAmount_Type              reportingAmount;
  sit_lpp_reportingInterval_Type            reportingInterval;
}  __attribute__((packed)) sit_lpp_periodicalReporting_Type;

typedef struct {
  UINT8                                     ReportingDuration;
}  __attribute__((packed)) sit_lpp_reporting_Duration_Type;


typedef struct {
  BOOL                                   cellChange;
  sit_lpp_reporting_Duration_Type           reportingDuration;
}  __attribute__((packed)) sit_lpp_triggeredReporting_Type;

typedef struct {
    UINT8                                       additionalInformation;
    #define OEM_onlyReturnInformationRequested  1
    #define OEM_mayReturnAditionalInformation   2
}  __attribute__((packed)) sit_lpp_additionalInformation_Type;

typedef struct {
  UINT8                                     accuracy;
  UINT8                                     confidence;
}  __attribute__((packed)) sit_lpp_horizontalAccuracy_Type;

typedef struct {
  UINT8                                     accuracy;
  UINT8                                     confidence;
}  __attribute__((packed)) sit_lpp_verticalAccuracy_Type;

typedef struct {
  UINT8                                     time;
}  __attribute__((packed)) sit_lpp_responseTime_Type;

typedef struct {
  UINT8         bitMask;
    #define BM_HORIZONTAL_ACCURACY          0x01
    #define BM_VERTICAL_ACCURACY            0x02
    #define BM_RESPONSE_TIME                0x04
  sit_lpp_horizontalAccuracy_Type           horizontalAccuracy;
  BOOL                                   verticalCoordinateRequest;
  sit_lpp_verticalAccuracy_Type             verticalAccuracy;
  sit_lpp_responseTime_Type                 responseTime;
  BOOL                                   velocityRequest;
}  __attribute__((packed)) sit_lpp_qos_Type;

typedef struct {
    UINT8                                   environment;
    #define badArea                     1
    #define notBadArea                  2
    #define mixedArea                   3
}  __attribute__((packed)) sit_lpp_environment_Type;

typedef struct {
    BOOL                                 ellipsoidPoint;
    BOOL                                 ellipsoidPointWithUncertaintyCircle;
    BOOL                                 ellipsoidPointWithUncertaintyEllipse;
    BOOL                                 polygon;
    BOOL                                 ellipsoidPointWithAltitude;
    BOOL                                 ellipsoidPointWithAltitudeAndUncertaintyEllipsoid;
    BOOL                                 ellipsoidArc;
}  __attribute__((packed)) sit_lpp_locationCoordinateTypes_Type;

typedef struct {
    UINT8                                                   velocityTypes;
    #define HORIZENTAL_VELOCITY                         1
    #define HORIZENTAL_WITH_VERTICAL_VELOCITY           2
    #define HORIZENTAL_VELOCITY_WITH_UNCERTAINTY        3
    #define HORIZENTAL_WITH_VERTICAL_VELOCITY_AND_UNC   4
} sit_lpp_velocityTypes;

typedef struct {
  UINT8   bitMask;

  #define  BM_TRIGGERED_REPORTING           0x01
  #define  BM_PERIODICAL_REPORTING          0x02
  #define  BM_ADDITIONAL_INFORMATION        0x04
  #define  BM_QOS                           0x08
  #define  BM_ENVIRONMENT                   0x10
  #define  BM_LOCATION_COORDINATE_TYPES     0x20
  #define  BM_VELOCITY_TYPES                0x40

  sit_lpp_locationInformationType_Type      locationInformationType;
  sit_lpp_triggeredReporting_Type           triggeredReporting;
  sit_lpp_periodicalReporting_Type          periodicalReporting;
  sit_lpp_additionalInformation_Type        additionalInformation;
  sit_lpp_qos_Type                          qos;
  sit_lpp_environment_Type                  environment;
  sit_lpp_locationCoordinateTypes_Type      locationCoordinateTypes;
  sit_lpp_velocityTypes                     velocityTypes;
}  __attribute__((packed)) sit_lpp_CommonIEsRequestLocationInformation;

typedef struct {
    UINT16                                  primaryCPICHInfo;
}  __attribute__ ((packed)) sit_lpp_fdd_Type;

typedef struct {
    UINT8                                   cellParameters;
}  __attribute__ ((packed)) sit_lpp_tdd_Type;

typedef struct {
    UINT8                                   modeType;
    #define FDD_TYPE                        0x01
    #define TDD_TYPE                        0x02
    union {
        sit_lpp_fdd_Type                    fdd;
        sit_lpp_tdd_Type                    tdd;
    } u_mode;
}  __attribute__((packed)) sit_lpp_mode_Type;

typedef struct {
    UINT8                                   cidType;
    #define EUTRA                           0x01
    #define UTRA                            0x02
    union {
        UINT32                              eutra;
        UINT32                              utra;
    } u_cid;
}  __attribute__((packed)) sit_lpp_cell_Identity_Type;

typedef struct {
    UINT8                                   mcc[3];
    UINT8                                   mnc[3];
    UINT8                                   sizeOfMcc;
    UINT8                                   sizeOfMnc;
}  __attribute__((packed)) sit_lpp_plmn_Identity_Type;

typedef struct {
    sit_lpp_plmn_Identity_Type              plmnIdentity;
    sit_lpp_cell_Identity_Type              cellIdentity;
}  __attribute__((packed)) sit_lpp_CellGlobalIdEUTRA_AndUTRA_Type;

typedef struct {
    UINT8                                   bitMask;
    #define            BM_CELL_GLOBAL_ID    0x01
    sit_lpp_mode_Type                       mode;
    sit_lpp_CellGlobalIdEUTRA_AndUTRA_Type  cellGlobalId;
    UINT16                                  referenceSystemFrameNumber;
}  __attribute__((packed)) sit_lpp_uTRA_Type;

typedef struct {
    UINT8                                   bitMask;
    #define     BM_REFRENCE_FNMSB           0x01
    UINT16                                  referenceFN;
    UINT8                                   referenceFNMSB;
}  __attribute__((packed)) sit_lpp_referenceFrame_Type;

typedef struct {
    sit_lpp_plmn_Identity_Type              plmnIdentity;
    UINT16                                  locationAreaCode;
    UINT16                                  cellIdentity;
}  __attribute__((packed)) sit_lpp_cellGlobalIdGERAN_Type;

typedef struct {
    UINT8                                   bitMask;

    #define        BM_CELL_GLOBAL_ID        0x01
    #define        BM_DELTA_GNSS_TOD        0x02

    UINT16                                  bcchCarrier;
    UINT8                                   bsic;
    sit_lpp_cellGlobalIdGERAN_Type          cellGlobalId;
    sit_lpp_referenceFrame_Type             referenceFrame;
    UINT16                                  delta_gnss_tod;
}  __attribute__((packed)) sit_lpp_gSM_Type;

typedef struct {
    UINT8                                   bitMask;
    #define         BM_CELL_GLOBAL_ID       0x01
    UINT16                                  physCellId;
    sit_lpp_CellGlobalIdEUTRA_AndUTRA_Type  cellGlobalId;
    UINT16                                  systemFrameNumber;
}  __attribute__((packed)) sit_lpp_eUTRA_Type;

typedef struct {
    UINT8                                   networkTimeType;
    #define EUTRA                           0x01
    #define UTRA                            0x02
    #define GSM                             0x04
    union {
        sit_lpp_eUTRA_Type                  eUTRA;
        sit_lpp_uTRA_Type                   uTRA;
        sit_lpp_gSM_Type                    gSM;
    } u_networkTime;
}  __attribute__((packed)) sit_lpp_MeasRefTime_NetworkTime_Type;

typedef struct {
    UINT8   gnss_id;
    #define GNSS_GPS                    0
    #define GNSS_SBASS                  1
    #define GNSS_QZSS                   2
    #define GNSS_GALILEO                3
    #define GNSS_GLONASS                4
}  __attribute__((packed)) sit_lpp_GNSS_ID_Type;

typedef struct {
    UINT8                                   bitMask;

    #define           BM_GNSS_TOD_FRAC      0x01
    #define           BM_GNSS_TOD_UNC       0x02
    #define           BM_NETWORK_TIME       0x04

    UINT32                                  gnssTODMsec;
    UINT16                                  gnssTODFrac;
    UINT16                                  gnssTODUnc;
    sit_lpp_GNSS_ID_Type                    gnssTimeID;
    sit_lpp_MeasRefTime_NetworkTime_Type    networkTime;
} __attribute__((packed)) sit_lpp_MeasurementReferenceTime_Type;

typedef struct {
    UINT16                                  gnssIds;
    #define GNSS_GPS_IDS                0x01
    #define GNSS_SBAS_IDS               0x02
    #define GNSS_QZSS_IDS               0x04
    #define GNSS_GALILEO_IDS            0x08
    #define GNSS_GLONASS_IDS            0x10
} __attribute__((packed)) sit_lpp_GNSS_ID_Bitmap_Type;

typedef struct {
    sit_lpp_MeasurementReferenceTime_Type   measurementReferenceTime;
    sit_lpp_GNSS_ID_Bitmap_Type             agnssList;
} __attribute__((packed)) sit_lpp_GNSS_LocationInformation_Type;

typedef struct {
    UINT8                                   mpathDet;
    #define notMeasured                 0
    #define low                         1
    #define medium                      2
    #define high                        3
} sit_lpp_mpathDet;

typedef struct {
    UINT8                                   satellite_id;
} __attribute__((packed)) sit_lpp_SV_ID_Type;

typedef struct {
    UINT8                                   bitMask;

    #define         BM_CARRIER_QUALITY_IND  0x01
    #define         BM_INTEGER_CODE_PHASE   0x02
    #define         BM_DOPPLER              0x04
    #define         BM_ADR                  0x08

    sit_lpp_SV_ID_Type                      sv_ID;
    UINT8                                   cNo;
    sit_lpp_mpathDet                        mpathDet;
    UINT8                                   carrierQuialityInd;
    UINT32                                  codePhase;
    UINT16                                  intergerCodePhase;
    UINT16                                  coePhaseRMSError;
    INT16                                   doppler;
    UINT32                                  adr;
} __attribute__((packed)) sit_lpp_GNSS_SatMeasElement_Type;

typedef struct {
    UINT8                                   signalID;
} __attribute__((packed)) sit_lpp_GNSS_SignalID_Type;

typedef struct {
    UINT8                                   bitMask;
    #define      BM_CODE_PHASE_AMBIGUITY    0x01
    sit_lpp_GNSS_SignalID_Type              signalId;
    UINT16                                  codePhaseAmbiguity;
    sit_lpp_GNSS_SatMeasElement_Type        satMeasList[LPP_MAX_SVs_CNT];
    UINT8                                   sizeOfMeasurementList;
} __attribute__((packed)) sit_lpp_GNSS_SgnMeasElement_Type;

typedef struct {
    sit_lpp_GNSS_ID_Type                    gnssId;
    sit_lpp_GNSS_SgnMeasElement_Type        gnssSgnMeasList;
    UINT8                                   sizeOfMeasurementList;
} __attribute__((packed)) sit_lpp_GNSS_MeasurementForOneGNSS_Type;

typedef struct {
    sit_lpp_MeasurementReferenceTime_Type   measureRefTime;
    sit_lpp_GNSS_MeasurementForOneGNSS_Type measurementList[LPP_MAX_AGNSS_CNT];
    UINT8                                   sizeOfMeasurementList;
} __attribute__((packed)) sit_lpp_GNSS_SignalMeasurementInformation_Type;

typedef struct {
  UINT8                                             bitMask;

  #define      BM_SIGNAL_MEASURE_INFO               0x01
  #define      BM_LOCATION_INFO                     0x02
  #define      BM_ERROR                             0x04

    sit_lpp_GNSS_SignalMeasurementInformation_Type  signalMeasurementInformation;
    sit_lpp_GNSS_LocationInformation_Type           locationInformation;
}  __attribute__((packed)) sit_lpp_A_GNSS_ProvideLocationInformation;


/*----------------------------------------------------------------
    GPS LPP Provide Assist Data
*/

typedef struct {
    UINT8                                   satelliteID;
    UINT16                                  tlmWord;
    UINT8                                   antiSpoof;
    UINT8                                   alert;
    UINT8                                   tlmRsvdBits;
} __attribute__((packed)) sit_lpp_GPS_TOW_AssistElement_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_TIME_OF_DAY_FRAC_MSEC        0x01
    #define BM_NOTI_OF_LEAP_SEC             0x02
    #define BM_TOW_ASSIST                   0x04
    sit_lpp_GNSS_ID_Type                    timeID;
    UINT16                                  dayNumber;
    UINT32                                  timeOfDay;
    UINT16                                  timeOfDayFrac_msec;
    UINT8                                   notiOfLeapSec;
    sit_lpp_GPS_TOW_AssistElement_Type      towAssist[64];
    UINT8                                   sizeOfTowAssist;
} __attribute__((packed)) sit_lpp_GNSS_SystemTime_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_CELL_GLOBAL_ID_EUTRA         0x01
    UINT16                                  physCellId;
    sit_lpp_CellGlobalIdEUTRA_AndUTRA_Type  cellGlobalIdEUTRA;
    UINT16                                  earfcn;
} __attribute__((packed)) sit_lpp_cellIDeUTRA_Type;

typedef struct {
    UINT8   bitMask;
    #define BM_CELL_GLOBAL_ID_UTRA          0x01
    sit_lpp_mode_Type                       mode;
    sit_lpp_CellGlobalIdEUTRA_AndUTRA_Type  cellGlobalIdEUTRA;
    UINT16                                  uarfcn;
} __attribute__((packed)) sit_lpp_cellIDuTRA_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_CELL_GLOBAL_ID_GSM           0x01
    UINT16                                  bcchCarrier;
    UINT8                                   bsic;
    sit_lpp_cellGlobalIdGERAN_Type          cellGlobalIdGERAN;
} __attribute__((packed)) sit_lpp_cellIDgSM_Type;

typedef struct {
    UINT8                                   cellIDType;
    #define CELL_ID_EUTRA                   0x01
    #define CELL_ID_UTRA                    0x02
    #define CELL_ID_GSM                     0x04
    union {
       sit_lpp_cellIDeUTRA_Type             eUTRA;
       sit_lpp_cellIDuTRA_Type              uTRA;
       sit_lpp_cellIDgSM_Type               gSM;
    } u_cellID;
} __attribute__((packed)) sit_lpp_cellID_Type ;

typedef struct {
    UINT8                                   bitMask;
    #define BM_FRAME_DRIFT                  0x01
    UINT16                                  secondsFromFrameStructureStart;
    UINT32                                  fractionalSecondsFromFrameStructureStart;
    UINT8                                   frameDrift;
    sit_lpp_cellID_Type                     cellId;
} __attribute__((packed)) sit_lpp_NetworkTime_Type;


typedef struct {
    UINT8                                   bitMask;
    #define BM_BS_ALIGN                     0x01
    sit_lpp_NetworkTime_Type                networkTime;
    UINT8                                   referenceTimeUnc;
    BOOL                                 bsAlign;
} __attribute__((packed)) sit_lpp_GNSS_ReferenceTimeForOneCell;

typedef struct {
    UINT8                                   bitMask;
    #define     BM_REF_TIME_UNC             0x01
    #define     BM_REF_TIME_FOR_CELLS       0x02
    sit_lpp_GNSS_SystemTime_Type            systemTime;
    UINT8                                   refTimeUnc;
    sit_lpp_GNSS_ReferenceTimeForOneCell    refTimeForCells[16];
    UINT8                                   sizeOfRefTimeForCells;
} __attribute__((packed)) sit_lpp_GNSS_ReferenceTime_Type;

typedef struct {
    UINT8                                   latitudeSign;
    #define north                       0
    #define south                       1
} sit_lpp_latitudeSign;

typedef struct {
    UINT8                                   altitudeDirection;
    #define alt_height                      0
    #define alt_depth                       1
} sit_lpp_AltitudeDirection;

typedef struct {
    sit_lpp_latitudeSign                    latitudeSign;
    UINT32                                  degreesLatitude;
    INT32                                   degreesLongitude;
    sit_lpp_AltitudeDirection               altitudeDirection;
    UINT16                                  altitude;
    UINT8                                   uncertaintySemiMajor;
    UINT8                                   uncertaintySemiMinor;
    UINT8                                   orientationMajorAxis;
    UINT8                                   uncertaintyAltitude;
    UINT8                                   confidence;
} __attribute__((packed)) sit_lpp_EllipsoidPointWithAltitudeAndUncertaintyEllipsoid_Type;

typedef struct {
    sit_lpp_EllipsoidPointWithAltitudeAndUncertaintyEllipsoid_Type  threeDlocation;
} __attribute__((packed)) sit_lpp_GNSS_ReferenceLocation_Type;

typedef struct {
    UINT8                                   dataID;
    INT8                                    alfa0;
    INT8                                    alfa1;
    INT8                                    alfa2;
    INT8                                    alfa3;
    INT8                                    beta0;
    INT8                                    beta1;
    INT8                                    beta2;
    INT8                                    beta3;
} __attribute__((packed)) sit_lpp_KlobucharModelParameter_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_IONO_STORM_FLAG1             0x01
    #define BM_IONO_STORM_FLAG2             0x02
    #define BM_IONO_STORM_FLAG3             0x04
    #define BM_IONO_STORM_FLAG4             0x08
    #define BM_IONO_STORM_FLAG5             0x10
    UINT16                                  ai0;
    UINT16                                  ai1;
    UINT16                                  ai2;
    UINT8                                   ionoStormFlag1;
    UINT8                                   ionoStormFlag2;
    UINT8                                   ionoStormFlag3;
    UINT8                                   ionoStormFlag4;
    UINT8                                   ionoStormFlag5;
} __attribute__((packed)) sit_lpp_NeQuickModelParameter_Type;

typedef struct {
  UINT8                                     bitMask;
  #define           BM_KLOBUCHAR_MODEL      0x01
  #define           BM_NE_QUICK_MODEL       0x02
    sit_lpp_KlobucharModelParameter_Type    klobucharModel;
    sit_lpp_NeQuickModelParameter_Type      neQuickModel;
} __attribute__((packed)) sit_lpp_GNSS_IonosphericModel_Type;

typedef struct {
    UINT16                                  teop;
    INT32                                   pmX;
    INT16                                   pmXdot;
    INT32                                   pmY;
    INT16                                   pmYdot;
    INT32                                   deltaUT1;
    INT32                                   deltaUT1dot;
} __attribute__((packed)) sit_lpp_GNSS_EarthOrientationParameters_Type;

typedef struct {
    UINT8                                           bitmask;
    #define BM_REF_TIME                             0x01
    #define BM_REF_LOCATION                         0x02
    #define BM_IONO_MODEL                           0x04
    #define BM_EARTH_ORIENTATION_PARAMETERS         0x08
    sit_lpp_GNSS_ReferenceTime_Type                 refTime;
    sit_lpp_GNSS_ReferenceLocation_Type             refLocation;
    sit_lpp_GNSS_IonosphericModel_Type              ionoModel;
    sit_lpp_GNSS_EarthOrientationParameters_Type    earthOrientationParameters;
} __attribute__((packed)) sit_lpp_GNSS_Common_Assist_Data_Type;

typedef struct {
    UINT8   sbas_id;
    #define GNSS_WASS                   0
    #define GNSS_EGNOS                  1
    #define GNSS_MSAS                   2
    #define GNSS_GAGAN                  3
} __attribute__((packed)) sit_lpp_SBAS_ID_Type;

typedef struct {
    UINT8                                   bitMask;

    #define BM_TA1                          0x01
    #define BM_TA2                          0x02
    #define BM_WEEK_NUMBER                  0x04
    #define BM_DELTA_T                      0x08

    UINT16                                  gnss_TimeModelRefTime;
    INT32                                   tA0;
    INT16                                   tA1;
    INT8                                    tA2;
    UINT8                                   gnssToID;
    UINT16                                  weekNumber;
    INT8                                    deltaT;
}  __attribute__((packed)) sit_lpp_GNSS_TimeModelElement_Type;

typedef struct {
    UINT8                                                       statusHealth;
    #define _1_dot_0                                         0
    #define _0_dot_75                                        1
    #define _0_dot_5                                         2
    #define _0_dot_3                                         3
    #define _0_dot_2                                         4
    #define _0_dot_1                                         5
    #define _Reference_Station_Transmission_Not_Monitored    6
    #define _Data_is_invalid_disregard                       7
} sit_lpp_statusHealth;

typedef struct {
    UINT8                                   udre;
    #define UDRE_LESS_1M                0
    #define UDRE_MORE_1M_LESS_4M        1
    #define UDRE_MORE_4M_LESS_8M        2
    #define UDRE_MORE_8M0x11            3
} sit_lpp_udre;

typedef struct{
    UINT8                                   uderGrowthRate;
    #define _1_dot_5                     0
    #define _2                           1
    #define _4                           2
    #define _6                           3
    #define _8                           4
    #define _10                          5
    #define _12                          6
    #define _16                          7
} sit_lpp_udreGrowthRate;

typedef struct{
    UINT8                                   udreValidityTime;
    #define _20                          0
    #define _40                          1
    #define _80                          2
    #define _160                         3
    #define _320                         4
    #define _640                         5
    #define _1280                        6
    #define _2560                        7
} sit_lpp_udreValidityTime;

typedef struct {
  UINT8                                     bitMask;

  #define      BM_UDRE_GROWTH_RATE          0x01
  #define      BM_UDRE_VALIDITY_TIME        0x02

  sit_lpp_SV_ID_Type                        svID;
  UINT16                                    iod;
  sit_lpp_udre                              udre;
  INT16                                     pseudoRangeCor;
  INT8                                      rangeRateCor;
    sit_lpp_udreGrowthRate                  udreGrwothRate;
    sit_lpp_udreValidityTime                udreValidityTime;
}  __attribute__((packed)) sit_lpp_DGNSS_CorrectionsElement_Type;

typedef struct {
  sit_lpp_GNSS_SignalID_Type                signalID;
  sit_lpp_statusHealth                      statusHealth;
  sit_lpp_DGNSS_CorrectionsElement_Type     dgnssSatList[LPP_MAX_SVs_CNT];
    UINT8                                   sizeOfDgnssSatList;
}  __attribute__((packed)) sit_lpp_DGNSS_SgnTypeElement_Type;

typedef struct {
    UINT16                                  refTime;
    sit_lpp_DGNSS_SgnTypeElement_Type       sgnTypeList;
}  __attribute__((packed)) sit_lpp_GNSS_DifferentialCorrections_Type;

typedef struct {
    UINT8                                   stanModelId;
    #define I_NAV                       0
    #define F_NAV                       1
} sit_lpp_stanModelId;

typedef struct {
    UINT8                                   bitMask;

    #define    BM_STAN_CLOCK_TGD            0x01
    #define    BM_STAN_MODEL_ID             0x02

    UINT16                                  stanClockToc;
    INT16                                   stanClockAF2;
    INT32                                   stanClockAF1;
    INT32                                   stanClockAF0;
    INT16                                   stanClockTgd;
    sit_lpp_stanModelId                     stanModelID;
}  __attribute__((packed)) sit_lpp_StandardClockModelElement_Type;

typedef struct {
    UINT16                                  navToc;
    INT8                                    navaf2;
    INT16                                   navaf1;
    INT32                                   navaf0;
    INT8                                    navTgd;
}  __attribute__((packed)) sit_lpp_NAV_ClockModel_Type;

typedef struct {
    UINT8                                   bitMask;

    #define               BM_CNAV_ISCL1_CP  0x01
    #define               BM_CNAV_ISCL1_CD  0x02
    #define               BM_CNAV_ISCL1_CA  0x04
    #define               BM_CNAV_ISCL2_C   0x08
    #define               BM_CNAV_ISCL5_I5  0x10
    #define               BM_CNAV_ISCL5_Q5  0x20

    UINT16                                  cnavToc;
    UINT16                                  cnavTop;
    INT8                                    cnavURA0;
    UINT8                                   cnavURA1;
    UINT8                                   cnavURA2;
    INT16                                   cnavAf2;
    INT32                                   cnavAf1;
    INT32                                   cnavAf0;
    INT16                                   cnavTgd;
    INT16                                   cnavISCl1cp;
    INT16                                   cnavISCl1cd;
    INT16                                   cnavISCl1ca;
    INT16                                   cnavISCl2c;
    INT16                                   cnavISCl5i5;
    INT16                                   cnavISCl5q5;
}  __attribute__((packed)) sit_lpp_CNAV_ClockModel_Type;

typedef struct {
    INT8                                    bitMask;
    #define     BM_GLO_DELTA_TAU            0x01
    INT32                                   gloTau;
    INT16                                   gloGamma;
    INT8                                    gloDeltaTau;
}  __attribute__((packed)) sit_lpp_GLONASS_ClockModel_Type;

typedef struct {
    UINT16                                  sbasTo;
    INT16                                   sbasAgfo;
    INT8                                    sbasAgf1;
}  __attribute__((packed)) sit_lpp_SBAS_ClockModel_Type;

typedef struct {
  UINT8                                         clockModelType;
  #define STAND_CLK_MODEL                       0x01
    #define NAV_CLK_MODEL                       0x02
    #define CNAV_CLK_MODEL                      0x04
    #define GLONASS_CLK_MODEL                   0x08
    #define SBAS_CLK_MODEL                      0x10
    union {
      sit_lpp_StandardClockModelElement_Type    standardClockModelList[2];
      sit_lpp_NAV_ClockModel_Type               navClockModel;
      sit_lpp_CNAV_ClockModel_Type              cnavClockModel;
      sit_lpp_GLONASS_ClockModel_Type           glonassClockModel;
      sit_lpp_SBAS_ClockModel_Type              sbasClockModel;
    } u_clockModel;
    UINT8                                       sizeOfStandardClockModelList;
}   __attribute__((packed)) sit_lpp_GNSS_ClockModel_Type;

typedef struct {
    UINT16                                  keplerToe;
    INT32                                   keplerW;
    INT16                                   keplerDeltaN;
    INT32                                   keplerM0;
    INT32                                   keplerOmegaDot;
    UINT32                                  keplerE;
    INT16                                   keplerIDot;
    UINT32                                  keplerAPowerHalf;
    INT32                                   keplerI0;
    INT32                                   keplerOmega0;
    INT16                                   keplerCrs;
    INT16                                   keplerCis;
    INT16                                   keplerCus;
    INT16                                   keplerCrc;
    INT16                                   keplerCic;
    INT16                                   keplerCuc;
}  __attribute__((packed)) sit_lpp_NavModelKeplerianSet_Type;

typedef struct {
    UINT32                                  reserved1;
    UINT32                                  reserved2;
    UINT32                                  reserved3;
    UINT16                                  reserved4;
}  __attribute__((packed)) sit_lpp_ephemSF1Rsvd_Type;

typedef struct {
    UINT8                                   ephemCodeOnL2;
    UINT8                                   ephemL2Pflag;
    sit_lpp_ephemSF1Rsvd_Type               ephemSF1Rsvd;
    UINT8                                   ephemAODA;
}  __attribute__((packed)) sit_lpp_addNAVparam_Type;

typedef struct {
    UINT8                                   bitMask;
    #define ADD_NAV_PARAM                   0x01
    UINT8                                   navURA;
    UINT8                                   navFitFlag;
    UINT16                                  navToe;
    INT32                                   navOmega;
    INT16                                   navDeltaN;
    INT32                                   navM0;
    INT32                                   navOmegaADot;
    UINT32                                  navE;
    INT16                                   navIDot;
    UINT32                                  navAPowerHalf;
    INT32                                   navI0;
    INT32                                   navOmegaA0;
    INT16                                   navCrs;
    INT16                                   navCis;
    INT16                                   navCus;
    INT16                                   navCrc;
    INT16                                   navCic;
    INT16                                   navCuc;
  sit_lpp_addNAVparam_Type        addNAVparam;
}  __attribute__((packed)) sit_lpp_NavModelNAV_KeplerianSet_Type;

typedef struct {
    UINT16                                  cnavTop;
    INT8                                    cnavURAindex;
    INT32                                   cnavDeltaA;
    INT32                                   cnavAdot;
    INT32                                   cnavDeltaNo;
    INT32                                   cnavDeltaNoDot;
    LONG                                   cnavMo;
    ULONG                                  cnavE;
    LONG                                   cnavOmega;
    LONG                                   cnavOMEGA0;
    INT32                                   cnavDeltaOmegaDot;
    LONG                                   cnavIo;
    INT16                                   cnavIoDot;
    INT16                                   cnavCis;
    INT16                                   cnavCic;
    INT32                                   cnavCrs;
    INT32                                   cnavCrc;
    INT32                                   cnavCus;
    INT32                                   cnavCuc;
}  __attribute__((packed)) sit_lpp_NavModelCNAV_KeplerianSet_Type;

typedef struct {
    UINT8                                   gloEn;
    UINT8                                   gloP1;
    BOOL                                 gloP2;
    UINT8                                   gloM;
    INT32                                   gloX;
    INT32                                   gloXdot;
    INT8                                    gloXdotdot;
    INT32                                   gloY;
    INT32                                   gloYdot;
    INT8                                    gloYdotdot;
    INT32                                   gloZ;
    INT32                                   gloZdot;
    INT8                                    gloZdotdot;
}  __attribute__((packed)) sit_lpp_NavModel_GLONASS_ECEF_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_SBAS_TO                      0x01
    UINT16                                  sbasTo;
    UINT8                                   sbasAccuracy;
    INT32                                   sbasXg;
    INT32                                   sbasYg;
    INT32                                   sbasZg;
    INT32                                   sbasXgDot;
    INT32                                   sbasYgDot;
    INT32                                   sbasZgDot;
    INT16                                   sbasXgDotDot;
    INT16                                   sbagYgDotDot;
    INT16                                   sbasZgDotDot;

}  __attribute__((packed)) sit_lpp_NavModel_SBAS_ECEF_Type;

typedef struct {
  UINT8                                     orbitModelType;
  #define KELP_SET                          0x01
    #define NAV_KEPL_SET                    0x02
    #define CNAV_KEPL_SET                   0x04
    #define GLONASS_ECEF                    0x08
    #define SBAS_ECEF                       0x10
    union {
  sit_lpp_NavModelKeplerianSet_Type         keplerianSet;
  sit_lpp_NavModelNAV_KeplerianSet_Type     navKeplerianSet;
  sit_lpp_NavModelCNAV_KeplerianSet_Type    cnavKeplerianSet;
  sit_lpp_NavModel_GLONASS_ECEF_Type        glonassECEF;
  sit_lpp_NavModel_SBAS_ECEF_Type           sbasECEF ;
    }u_navModel;
}  __attribute__((packed)) sit_lpp_GNSS_OrbitModel_Type;

typedef struct {
    sit_lpp_SV_ID_Type                      svID;
  UINT8                                     svHealth;
  UINT16                                    iod;
  sit_lpp_GNSS_ClockModel_Type              gnssClockModel;
  sit_lpp_GNSS_OrbitModel_Type              gnssOrbitModel;
} __attribute__((packed)) sit_lpp_GNSS_NavModelSatelliteElement_Type;

typedef struct {
  UINT8                                         nonBroadcastIndFlag;
  sit_lpp_GNSS_NavModelSatelliteElement_Type    gnssSatelliteList[LPP_MAX_SVs_CNT];
    UINT8                                       sizeOfGnssSatelliteList;
} __attribute__((packed)) sit_lpp_GNSS_NavigationModel_Type;

typedef struct {
    UINT8                                   signalIDs;
} __attribute__((packed))sit_lpp_GNSS_SignalIDs_Type;

typedef struct {
  UINT8                                     bitMask;
  #define   BM_BAD_SIGNAL_ID                0x01
    sit_lpp_SV_ID_Type                      badSVID;
    sit_lpp_GNSS_SignalIDs_Type             badSignalID;
} __attribute__((packed)) sit_lpp_BadSignalElement_Type;

typedef struct {
    sit_lpp_GNSS_SignalID_Type              signalType;
    UINT8                                   dataBits[128];
} __attribute__((packed)) sit_lpp_GNSS_DataBitsSgnElement_Type;

typedef struct {
    sit_lpp_SV_ID_Type                      svID;
    sit_lpp_GNSS_DataBitsSgnElement_Type    dataBitsSgnList;
} __attribute__((packed)) sit_lpp_GNSS_DataBitsSatElement_Type;

typedef struct {
    UINT8                                       bitMask;
    #define BM_GNSS_TOD_FRAC                    0x01
    UINT16                                      gnssTOD;
    UINT16                                      gnssTODfrac;
    sit_lpp_GNSS_DataBitsSatElement_Type        dataBitsSatList[LPP_MAX_SVs_CNT];
    UINT8                                       sizeOfGnssDataBitsSatList;
} __attribute__((packed)) sit_lpp_GNSS_DataBitAssistance_Type;

typedef struct {
    UINT8                                   dopplerUncertaintyExt_r10;
    #define _d60                         0x00
    #define _d80                         0x01
    #define _d100                        0x10
    #define _d120                        0x11
    #define _noInformation               0x100
} sit_lpp_dopplerUncertaintyExt_r10;

typedef struct {
    UINT8                                   bitMask;
    #define BM_CODE_PHASE_1023              0x01
    #define BM_DOPPLER_UNCERTAINTY_EXT_R10  0x02
    sit_lpp_SV_ID_Type                      svID;
    INT16                                   doppler0;
    UINT8                                   doppler1;
    UINT8                                   dopplerUncertainty;
    UINT16                                  codePhase;
    UINT8                                   intCodePhase;
    UINT8                                   codePhaseSearchWindow;
    UINT16                                  azimuth;
    UINT8                                   elevation;
    BOOL                                 codePhase1023;
    sit_lpp_dopplerUncertaintyExt_r10       dopplerUncertaintyExt_r10;
} __attribute__((packed)) sit_lpp_GNSS_AcquisitionAssistElement_Type;

typedef struct {
    sit_lpp_GNSS_SignalID_Type                  signalID;
    sit_lpp_GNSS_AcquisitionAssistElement_Type  acqAssistList[LPP_MAX_SVs_CNT];
    UINT8                                       sizeOfAcqAssistList;
    UINT8                                       confidence;
} __attribute__((packed)) sit_lpp_GNSS_AcquisitionAssistance_Type;

typedef struct {

    sit_lpp_SV_ID_Type                      svID;
    UINT16                                  kepAlmanacE;
    INT16                                   kepAlmanacDeltaI;
    INT16                                   kepAlmanacOmegaDot;
    UINT8                                   kepSVHealth;
    INT32                                   kepAlmanacAPowerHalf;
    INT16                                   kepAlmanacOmega0;
    INT16                                   kepAlmanacW;
    INT16                                   kepAlmanacM0;
    INT16                                   kepAlmanacAF0;
    INT16                                   kepAlmanacAF1;
} __attribute__((packed)) sit_lpp_AlmanacKeplerianSet_Type;

typedef struct {

    sit_lpp_SV_ID_Type                      svID;
    UINT16                                  navAlmE;
    INT16                                   navAlmDeltaI;
    INT16                                   navAlmOMEGADOT;
    UINT8                                   navAlmSVHealth;
    UINT32                                  navAlmSqrtA;
    INT32                                   navAlmOMEGAo;
    INT32                                   navAlmOmega;
    INT32                                   navAlmMo;
    INT16                                   navAlmaf0;
    INT16                                   navAlmaf1;

} __attribute__((packed)) sit_lpp_AlmanacNAV_KeplerianSet_Type;

typedef struct {

    sit_lpp_SV_ID_Type                      svID;
    INT8                                    redAlmDeltaA;
    INT8                                    redAlmOmega0;
    INT8                                    redAlmPhi0;
    BOOL                                 redAlmL1Health;
    BOOL                                 redAlmL2Health;
    BOOL                                 redAlmL5Health;
} __attribute__((packed)) sit_lpp_AlmanacReducedKeplerianSet_Type;

typedef struct {
    sit_lpp_SV_ID_Type                      svID;
    UINT16                                  midiAlmE;
    INT16                                   midiAlmDeltaI;
    INT16                                   midiAlmOmegaDot;
    UINT32                                  midiAlmSqrtA;
    INT16                                   midiAlmOmega0;
    INT16                                   midiAlmOmega;
    INT16                                   midiAlmMo;
    INT16                                   midiAlmaf0;
    INT16                                   midiAlmaf1;
    BOOL                                 midiAlmL1Health;
    BOOL                                 midiAlmL2Health;
    BOOL                                 midiAlmL5Health;
} __attribute__((packed)) sit_lpp_AlmanacMidiAlmanacSet_Type;

typedef struct {
    UINT8                                   bitMask;

    #define      BM_GLOALM_MA               0x01

    UINT16                                  gloAlm_NA;
    UINT8                                   gloAlmnA;
    UINT8                                   gloAlmHA;
    INT32                                   gloAlmLambdaA;
    UINT32                                  gloAlmtlambdaA;
    INT32                                   gloAlmDeltaIa;
    INT32                                   gloAlmDeltaTA;
    INT8                                    gloAlmDeltaTdotA;
    UINT16                                  gloAlmEpsilonA;
    INT16                                   gloAlmOmegaA;
    INT16                                   gloAlmTauA;
    UINT8                                   gloAlmCA;
    UINT8                                   gloAlmMA;
} __attribute__((packed)) sit_lpp_AlmanacGLONASS_AlmanacSet_Type;
typedef struct {

    UINT8                                   sbasAlmDataID;
    sit_lpp_SV_ID_Type                      svID;
    UINT8                                   sbasAlmHealth;
    INT16                                   sbasAlmXg;
    INT16                                   sbasAlmYg;
    INT16                                   sbasAlmZg;
    INT8                                    sbasAlmXgdot;
    INT8                                    sbasAlmYgDot;
    INT8                                    sbasAlmZgDot;
    UINT16                                  sbasAlmTo;
} __attribute__((packed)) sit_lpp_AlmanacECEF_SBAS_AlmanacSet_Type;

typedef struct {

    UINT8                                           almType;
    #define ALM_SET                                 0x01
    #define ALM_NAV                                 0x02
    #define ALM_REDUCED                             0x04
    #define ALM_MIDI                                0x08
    #define ALM_GLONASS                             0x10
    #define ALM_SBAS                                0x20

    union {
        sit_lpp_AlmanacKeplerianSet_Type            keplerianAlmanacSet;
        sit_lpp_AlmanacNAV_KeplerianSet_Type        keplerianNAV_Almanac;
        sit_lpp_AlmanacReducedKeplerianSet_Type     keplerianReducedAlmanac;
        sit_lpp_AlmanacMidiAlmanacSet_Type          keplerianMidiAlmanac;
        sit_lpp_AlmanacGLONASS_AlmanacSet_Type      keplerianGLONASS;
        sit_lpp_AlmanacECEF_SBAS_AlmanacSet_Type    ecefSbasAlmanac;
    } u_alm;
} __attribute__((packed)) sit_lpp_GNSS_AlmanacElement_Type;

typedef struct {
  UINT8                                     bitMask;

  #define    BM_ALM_WEEK_NUMBER             0x01
  #define    BM_TOA                         0x02
  #define    BM_IODA                        0x04

    UINT8                                   weekNumber;
    UINT8                                   toa;
    UINT8                                   ioda;
    BOOL                                 completeAlmanacProvided;
    sit_lpp_GNSS_AlmanacElement_Type        almanacList[32];
    UINT8                                   sizeOfAlmanacList;
} __attribute__((packed)) sit_lpp_GNSS_Almanac_Type;

typedef struct {
    INT32                                   utcA1wnt;
    INT32                                   utcA0wnt;
    UINT8                                   utcTot;
    UINT8                                   utcWNt;
    INT8                                    utcDeltaTls;
    UINT8                                   utcWNlsf;
    INT8                                    utcDN;
    INT8                                    utcDeltaTlsf;
    UINT8                                   utcStandardID;

} __attribute__((packed)) sit_lpp_UTC_ModelSet4_Type;

typedef struct {
  UINT8                                     bitMask;

  #define     BM_B1                         0x01
  #define     BM_B2                         0x02
  #define     BM_KP                         0x04

    UINT16                                  nA;
    INT32                                   tauC;
    INT16                                   b1;
    INT16                                   b2;
    UINT8                                   kp;
} __attribute__((packed)) sit_lpp_UTC_ModelSet3_Type;

typedef struct {

    INT16                                   utcA0;
    INT16                                   utcA1;
    INT8                                    utcA2;
    INT8                                    utcDeltaTls;
    UINT16                                  utcTot;
    UINT16                                  utcWNot;
    UINT8                                   utcWNlsf;
    UINT8                                   utcDN;
    INT8                                    utcDeltaTlsf;
} __attribute__((packed)) sit_lpp_UTC_ModelSet2_Type;

typedef struct {

    INT32                                   gnssUtcA1;
    INT32                                   gnssUtcA0;
    UINT8                                   gnssUtcTot;
    UINT8                                   gnssUtcWNt;
    INT8                                    gnssUtcDeltaTls;
    UINT8                                   gnssUtcWNlsf;
    INT8                                    gnssUtcDN;
    INT8                                    gnssUtcDeltaTlsf;
} __attribute__((packed)) sit_lpp_UTC_ModelSet1_Type;

typedef struct {

    UINT8                                   utcType;
    #define UTC_M1                          0x01
    #define UTC_M2                          0x02
    #define UTC_M3                          0x04
    #define UTC_M4                          0x08
    union {
        sit_lpp_UTC_ModelSet1_Type          utcModel1;
        sit_lpp_UTC_ModelSet2_Type          utcModel2;
        sit_lpp_UTC_ModelSet3_Type          utcModel3;
        sit_lpp_UTC_ModelSet4_Type          utcModel4;
    } u_utc;
} __attribute__((packed)) sit_lpp_GNSS_UTC_Model_Type;

typedef struct {
    sit_lpp_SV_ID_Type                      svID;
  sit_lpp_GNSS_SignalIDs_Type               signalsAvailable;
}  __attribute__((packed)) sit_lpp_GNSS_ID_GPS_SatElement_Type;

typedef struct {
  UINT8                                     bitMask;
  #define      BM_CHANNEL_NUMBER            0x01
    sit_lpp_SV_ID_Type                      svID;
    sit_lpp_GNSS_SignalIDs_Type             signalsAvailable;
  UINT8                                     channelNumber;
}  __attribute__((packed)) sit_lpp_GNSS_ID_GLONASS_SatElement_Type;

typedef struct {
    UINT8                                       auxInfoType;
    #define ID_GPS                              0x01
    #define ID_GLONASS                          0x02
    union {
        struct {
            sit_lpp_GNSS_ID_GPS_SatElement_Type     idGps[LPP_MAX_SVs_CNT];
            UINT8                                   sizeOfIdGps;
        }gpsList;

        struct {
            sit_lpp_GNSS_ID_GLONASS_SatElement_Type idGlonass[LPP_MAX_SVs_CNT];
            UINT8                                   sizeOfIdGlonass;
        }glonassList;
    } u_auxInfo;

}  __attribute__((packed)) sit_lpp_GNSS_AuxiliaryInformation_Type;

typedef struct {
    UINT16                                       bitMask;

    #define         BM_SBAS_ID                  0x0001
    #define         BM_TIME_MODELS              0x0002
    #define         BM_DIFF_CORRECTIONS         0x0004
    #define         BM_NAV_MODEL                0x0008
    #define         BM_RTI                      0x0010
    #define         BM_DATA_BIT_ASSISTANCE      0x0020
    #define         BM_ACQ_ASSISTANCE           0x0040
    #define         BM_ALMANAC                  0x0080
    #define         BM_UTC_MODEL                0x0100
    #define         BM_AUXILIARY_INFO           0x0200

    sit_lpp_GNSS_ID_Type                        gnss_ID;
    sit_lpp_SBAS_ID_Type                        sbas_ID;
    sit_lpp_GNSS_TimeModelElement_Type          timeModels;
    sit_lpp_GNSS_DifferentialCorrections_Type   diffCorrections;
    sit_lpp_GNSS_NavigationModel_Type           navModel;
    sit_lpp_BadSignalElement_Type               rti[LPP_MAX_SVs_CNT/2];
    UINT8                                       sizeOfRti;
#if FEATURE_GNSS_LPP_ADITIONAL_SUPPORT
    sit_lpp_GNSS_DataBitAssistance_Type         dataBitAssistance;
#endif
    sit_lpp_GNSS_AcquisitionAssistance_Type     acqAssistance;
    sit_lpp_GNSS_Almanac_Type                   almanac;
    sit_lpp_GNSS_UTC_Model_Type                 utcModel;
  sit_lpp_GNSS_AuxiliaryInformation_Type        auxiliaryInfo;
} __attribute__((packed)) sit_lpp_GNSS_GenericAssistDataElement_Type;

typedef struct {
    sit_lpp_GNSS_ID_Bitmap_Type             gnssMethods;
    BOOL                                 ftaMeasReq;
    BOOL                                 adrMeasReq;
    BOOL                                 multiFreqMeasReq;
    BOOL                                 assistanceAvailability;
} __attribute__((packed)) sit_lpp_gnss_PositioningInstructions_Type;

typedef struct {
    sit_lpp_gnss_PositioningInstructions_Type       posInstructions;
} __attribute__((packed)) sit_lpp_A_GNSS_RequestLocationInformation;


typedef struct {
    UINT8                                       assistType;
    #define CMN_ASSIST_DATA                  0x01
    #define GPS_GENERIC_ASSIST_DATA          0x02
    #define GLONASS_GENERIC_ASSIST_DATA      0x04
    #define MISC_GENERIC_ASSIST_DATA         0x08
    union {
    sit_lpp_GNSS_Common_Assist_Data_Type        cmnAssistData;
    sit_lpp_GNSS_GenericAssistDataElement_Type  gps_genericAssistData;
    sit_lpp_GNSS_GenericAssistDataElement_Type  glonass_genericAssistData;
    sit_lpp_GNSS_GenericAssistDataElement_Type  misc_genericAssistData;
    }u_IEs_type;
} __attribute__((packed)) sit_lpp_A_GNSS_Provide_Assistance_Data;

/*----------------------------------------------------------------
    GPS AP Provide Location Information
*/

typedef struct {
    UINT16                                  bearing;
    UINT16                                  horiSpeed;
}  __attribute__((packed)) sit_lpp_HorizontalVelocityType;

typedef struct {
    UINT8                                   vertiDirection;
    #define OEM_upward                      0
    #define OEM_downward                    1
}__attribute__((packed)) sit_lpp_vertiDirection;

typedef struct {
    UINT16                                  bearing;
    UINT16                                  horiSpeed;
    sit_lpp_vertiDirection                  vertiDirection;
    UINT8                                   vertiSpeed;
}  __attribute__((packed)) sit_lpp_HorizontalWithVerticalVelocityType;


typedef struct {
    UINT16                                  bearing;
    UINT16                                  horiSpeed;
    UINT8                                   uncertaintySpeed;
}  __attribute__((packed)) sit_lpp_HorizontalVelocityWithUncertaintyType;

typedef struct {
    UINT16                                  bearing;
    UINT16                                  horiSpeed;
    sit_lpp_vertiDirection                  vertiDirection;
    UINT8                                   vertiSpeed;
    UINT8                                   horiUncertaintySpeed;
    UINT8                                   vertiUncertaintySpeed;
}  __attribute__((packed)) sit_lpp_HorizontalWithVerticalVelocityAndUncertaintyType;

typedef struct {
    sit_lpp_velocityTypes                                               velocityType;
    #define BM_HORI_VELO                                                0x01
    #define BM_HORI_WITH_VERTI_VELO                                     0x02
    #define BM_HORI_VELO_WITH_UNCERTAINTY                               0x04
    #define BM_HORI_WITH_VERTI_VELO_WITH_UNCERTAINTY                    0x08
    union {
        sit_lpp_HorizontalVelocityType                                  horiVelo;
        sit_lpp_HorizontalWithVerticalVelocityType                      horiWithVertiVelo;
        sit_lpp_HorizontalVelocityWithUncertaintyType                   horiVeloWithUncertainty;
        sit_lpp_HorizontalWithVerticalVelocityAndUncertaintyType        horiWithVertiVeloAndUncertainty;
    }u_velocity;
}  __attribute__((packed)) sit_lpp_Velocity;

typedef struct {
    UINT8                                                       locFailCause;
    #define LPP_LOC_ERROR_UNDEFINED                             0
    #define LPP_REQUESTED_METHOD_NOT_SUPPORTED                  1
    #define LPP_POSITION_METHOD_FAILURE                         2
    #define LPP_PERIODIC_LOCATION_MEASUREMENT_NOT_AVAILABLE     3
    #define LPP_NO_LOCATION_INFORMATION                         4
}  __attribute__((packed)) sit_lpp_LocationError;

typedef struct {
    sit_lpp_latitudeSign                    latSign;
    UINT32                                  degreeLat;
    INT32                                   degreeLong;
}  __attribute__((packed)) sit_lpp_Ellipsoid_Point;

typedef struct {
    sit_lpp_latitudeSign                    latSign;
    UINT32                                  degreeLat;
    INT32                                   degreeLong;
    UINT8                                   uncertainty;
}  __attribute__((packed)) sit_lpp_Ellipsoid_PointWithUncertaintyCircle;

typedef struct {
    sit_lpp_latitudeSign                    latSign;
    UINT32                                  degreeLat;
    INT32                                   degreeLong;
    UINT8                                   uncertainty;
    UINT8                                   uncertaintySemiMaj;
    UINT8                                   uncertaintySemiMin;
    UINT8                                   orientationMajorAxis;
    UINT8                                   confidence;
}  __attribute__((packed)) sit_lpp_EllipsoidPointWithUncertaintyEllipse;

typedef struct {
    sit_lpp_latitudeSign                    latSign;
    UINT32                                  degreeLat;
    INT32                                   degreeLong;
}  __attribute__((packed)) sit_lpp_Polygon;


typedef struct {
    sit_lpp_latitudeSign                    latSign;
    UINT32                                  degreeLat;
    INT32                                   degreeLong;
    sit_lpp_AltitudeDirection               altiDirection;
    UINT16                                  alti;
}  __attribute__((packed)) sit_lpp_EllipsoidPointWithAltitude;

typedef struct {
    sit_lpp_latitudeSign                    latSign;
    UINT32                                  degreeLat;
    INT32                                   degreeLong;
    sit_lpp_AltitudeDirection               altiDirection;
    UINT16                                  alti;
    UINT8                                   uncertaintySemiMajor;
    UINT8                                   uncertaintySemiMinor;
    UINT8                                   orientationMajorAxis;
    UINT8                                   uncertaintyAltitude;
    UINT8                                   confidence;
}  __attribute__((packed)) sit_lpp_EllipsoidPointWithAltitudeAndUncertaintyEllipsoid;

typedef struct {
    sit_lpp_latitudeSign                    latSign;
    UINT32                                  degreeLat;
    INT32                                   degreeLong;
    UINT16                                  innerRadius;
    UINT8                                   uncertaintyRadius;
    UINT8                                   offsetAngle;
    UINT8                                   includedAngle;
    UINT8                                   confidence;
}  __attribute__((packed)) sit_lpp_EllipsoidArc;


typedef struct {
    UINT8   locCorrdinateType;
    #define ELLIPSOID_POINT                                             0x01
    #define ELLIPSOID_POINT_WITH_UNCERTAINTY_CIRCLE                     0x02
    #define ELLIPSOID_POINT_WITH_UNCERTAINTY_ELLIPSE                    0x04
    #define POLYGON                                                     0x08
    #define ELLIPSOID_POINT_WITH_ALTITUDE                               0x10
    #define ELLIPSOID_POINT_WITH_ALTITUDE_AND_UNCERTAINTY_ELLIPSOID     0x20
    #define ELLIPSOID_ARC                                               0x40
    union {
        sit_lpp_Ellipsoid_Point                                         ellipsoidPoint;
        sit_lpp_Ellipsoid_PointWithUncertaintyCircle                    ellipsoidPointWithUncertaintyCircle;
        sit_lpp_EllipsoidPointWithUncertaintyEllipse                    ellipsoidPointWithUncertaintyEllipse;
        sit_lpp_Polygon                                                 polygon;
        sit_lpp_EllipsoidPointWithAltitude                              ellipsoidPointWithAltitude;
        sit_lpp_EllipsoidPointWithAltitudeAndUncertaintyEllipsoid       ellipsoidPointWithAltitudeAndUncertaintyEllipsoid;
        sit_lpp_EllipsoidArc                                            ellipsoidArc;
    } u_locCorrdinateType;
}  __attribute__((packed)) sit_lpp_LocationCoordinates;

typedef struct {
    UINT8   bitMask;
    #define BM_LOCATION_ESTIMATE              0x01
    #define BM_LPP_VELOCITY_ESTIMATE          0x02
    #define BM_LOCATION_ERROR                 0x04
    sit_lpp_LocationCoordinates               locationEstimate;
    sit_lpp_Velocity                          velocityEstimate;
    sit_lpp_LocationError                     locationError;
}  __attribute__((packed)) sit_lpp_CommonIEsProvideLocationInformation;


/*----------------------------------------------------------------
    LPP Implementation
*/

typedef struct {
  UINT32                                    sid;
}__attribute__((packed)) sit_lpp_Header;

  #define  LPP_LOC_INFO_NONE                0x00
  #define  LPP_LOC_INFO_CMN                 0x01
  #define  LPP_LOC_INFO_A_GNSS              0x02
  #define  LPP_LOC_INFO_OTDOA               0x04
  #define  LPP_LOC_INFO_ECID                0x08
  #define  LPP_LOC_INFO_EPDU                0x10

typedef struct {
    UINT8                                   toIds;
    BOOL                                 deltaT;
}  __attribute__((packed)) sit_lpp_GNSS_TimeModelElementReq_Type;

typedef struct {
    sit_lpp_GNSS_SignalIDs_Type             signalsReq;
    BOOL                                 validityTimeReq;
}  __attribute__((packed)) sit_lpp_GNSS_Differential_Corrections_REQ_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_CLOCK_MODEL_ID_PREF_LIST     0x01
    #define BM_ORBIT_MODEL_ID_PREF_LIST     0x02
    #define BM_ADD_NAV_PARAM_REQ            0x04
    ULONG                                  svReqList;
    UINT8                                   clockModelIDPrefList[8];
    UINT8                                   sizeOfClockModelIDPrefList;
    UINT8                                   orbitModelIDPrefList[8];
    UINT8                                   sizeOforbitModelIDPrefList;
    BOOL                                 addNavparamReq;
}  __attribute__((packed)) sit_lpp_ReqNavListInfo_Type;

typedef struct {
    UINT8                                   bitmask;
    #define BM_CLOCK_MODEL_ID               0x01
    #define BM_ORBIT_MODEL_ID               0x02
    UINT8                                   svID;
    UINT16                                  iod;
    UINT8                                   clockModelID;
    UINT8                                   orbitModelID;
}  __attribute__((packed)) sit_lpp_Sat_List_Related_Data_Element_Type;

typedef struct {
    UINT8                                       bitMask;
    #define BM_SAT_LIST_RELATED_DATA_LIST       0x01
    UINT16                                      weekOrDay;
    UINT8                                       toe;
    UINT8                                       toeLimit;
    sit_lpp_Sat_List_Related_Data_Element_Type  satListRelatedDataList[32];
    UINT8                                       sizeOfSatListRelatedDataList;
}  __attribute__((packed)) sit_lpp_Stored_NAV_List_Info_Type;

typedef struct {
    UINT8   modelReqType;
    #define STORED_NAV_LIST                 0x01
    #define REQ_NAV_LIST                    0x02
    union {
        sit_lpp_Stored_NAV_List_Info_Type   storedNavListInfo;
        sit_lpp_ReqNavListInfo_Type         reqNavListInfo;
    } u_modelReq;
}  __attribute__((packed)) sit_lpp_GNSS_Navigation_Model_REQ_Type;

typedef struct {

}  __attribute__((packed)) sit_lpp_GNSS_Real_Time_Integrity_REQ_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_TOD_FRAC                     0x01
    #define BM_DATA_BITS_REQ                0x02
    UINT16                                  tod;
    UINT16                                  todFrac;
    UINT8                                   dataBitInterval;
    sit_lpp_GNSS_SignalIDs_Type             signalIDs;
    UINT8                                   dataBitsReq[64];
    UINT8                                   sizeOfDataBitsReq;
}  __attribute__((packed)) sit_lpp_GNSS_Data_Bit_Assistance_REQ_Type;

typedef struct {
     UINT8                                  signalID;
}  __attribute__((packed)) sit_lpp_GNSS_Acquisition_Assistance_REQ_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_MODEL_ID                     0x01
    UINT8                                   modelID;
}  __attribute__((packed)) sit_lpp_GNSS_Almanac_REQ_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_MODEL_ID                     0x01
    UINT8                                   modelID;
}  __attribute__((packed)) sit_lpp_GNSS_UTC_Model_REQ_Type;

typedef struct {

}  __attribute__((packed)) sit_lpp_GNSS_Auxiliary_Information_REQ_Type;

typedef struct {
    UINT16                                          bitMask;
    #define BM_SBAS_ID                              0x0001
    #define BM_TIME_MODELS                          0x0002
    #define BM_DIFF_CORREECTIONS                    0x0004
    #define BM_NAV_MODEL                            0x0008
    #define BM_RTI                                  0x0010
    #define BM_DATA_BIT_ASSISTANCE                  0x0020
    #define BM_ACQ_ASSISTANCE                       0x0040
    #define BM_ALM                                  0x0080
    #define BM_UTC                                  0x0100
    #define BM_AUXILIARY_INFO                       0x0200
    sit_lpp_GNSS_ID_Type                            gnssID;
    sit_lpp_SBAS_ID_Type                            sbasID;
    sit_lpp_GNSS_TimeModelElementReq_Type           timeModels;
    sit_lpp_GNSS_Differential_Corrections_REQ_Type  diffCorrections;
    sit_lpp_GNSS_Navigation_Model_REQ_Type          navModel;
    sit_lpp_GNSS_Real_Time_Integrity_REQ_Type       rti;
    sit_lpp_GNSS_Data_Bit_Assistance_REQ_Type       dataBitAssistance;
    sit_lpp_GNSS_Acquisition_Assistance_REQ_Type    acqAssistance;
    sit_lpp_GNSS_Almanac_REQ_Type                   alm;
    sit_lpp_GNSS_UTC_Model_REQ_Type                 utc;
    sit_lpp_GNSS_Auxiliary_Information_REQ_Type     auxiliaryInfo;

}  __attribute__((packed)) sit_lpp_GNSS_Generic_Assist_Data_REQ_Element_Type;

typedef struct {

}  __attribute__((packed)) sit_lpp_GNSS_Earth_Orientation_Parameters_REQ_Type;


typedef struct {
    UINT8                                   bitMask;
    #define BM_KLOBUCHAR_MODEL              0x01
    #define BM_NEQUICKMODEL                 0x02
    UINT8                                   klobucharModel;
    UINT8                                   nequickModel; //NULL
} __attribute__((packed)) sit_lpp_GNSS_Ionospheric_Model_REQ_Type;

typedef struct {
} __attribute__((packed)) sit_lpp_GNSS_Reference_Location_REQ_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_TOW                          0x01
    #define BM_NOTI_OF_LEAP_SEC             0x02
    sit_lpp_GNSS_ID_Type                    timeReqPrefList[8];
    UINT8                                   sizeOftimeReqPrefList;
    BOOL                                 tow;
    BOOL                                 notOfLeapSecReq;
} __attribute__((packed)) sit_lpp_GNSS_Reference_Time_REQ_Type;

typedef struct {
    UINT8                                               bitmask;
    #define BM_REF_TIME                                 0x01
    #define BM_REF_LOCATION                             0x02
    #define BM_IONO_MODEL                               0x04
    #define BM_EARTH_ORIENTATION                        0x08
    sit_lpp_GNSS_Reference_Time_REQ_Type                refTime;
    sit_lpp_GNSS_Reference_Location_REQ_Type            refLocation;
    sit_lpp_GNSS_Ionospheric_Model_REQ_Type             ionoModel;
    sit_lpp_GNSS_Earth_Orientation_Parameters_REQ_Type  earthOrientation;
} __attribute__((packed)) sit_lpp_GNSS_Common_Assist_Data_REQ_Type;


typedef struct {
    UINT8                                               bitmask;
    #define  BM_CMN_ASSIST                              0x01
    #define  BM_GEN_ASSIST                              0x02
    sit_lpp_GNSS_Common_Assist_Data_REQ_Type            cmnAssist;
    sit_lpp_GNSS_Generic_Assist_Data_REQ_Element_Type   genAssist[LPP_MAX_AGNSS_CNT];
    UINT8                                               sizeOfGenAssist;
} __attribute__((packed)) sit_lpp_A_GNSS_Request_Assistance_Data;

typedef struct {
    UINT8                                   cellTime;
    #define EUTRA                           0x01
    #define UTRA                            0x02
    #define GSM                             0x04
} sit_lpp_cell_time;

typedef struct {
    UINT8                                   positionModes;
    #define STANDALONE                      0x01
    #define UE_BASED                        0x02
    #define UE_ASSISTED                     0x04
}sit_lpp_position_modes;

typedef struct {
    sit_lpp_cell_time                       cellTime;
    sit_lpp_position_modes                  positionModes;
}  __attribute__((packed)) sit_lpp_FTA_MeasSupport_Type;

typedef struct {
    UINT8                                   sbasIds;
}  __attribute__((packed)) sit_lpp_GNSS_SBAS_IDs_Type;

typedef struct {
    UINT8                                   bitMask;

    #define BM_SBAS_IDS                     0x01
    #define BM_FTA_MEASURE_SUPPORT          0x02

    sit_lpp_GNSS_ID_Type                    gnssID;
    sit_lpp_GNSS_SBAS_IDs_Type              sbasIDs;
    UINT8                                   positionMode;
    sit_lpp_GNSS_SignalIDs_Type             signalIDs;
    sit_lpp_FTA_MeasSupport_Type            ftaMeasureSupport;
    BOOL                                 adrSupport;
    BOOL                                 velocityMeasure;
}  __attribute__((packed)) sit_lpp_GNSS_Support_Element_Type;

typedef struct {
    //NA
}  __attribute__((packed)) sit_lpp_GNSS_TimeModelListSupport_Type;

typedef struct {
    sit_lpp_GNSS_SignalIDs_Type             gnssSignalIDs;
    BOOL                                 validityTimeSup;
}  __attribute__((packed)) sit_lpp_GNSS_DifferentialCorrectionsSupport_Type;

typedef struct {
    UINT8                                   bitMask;

    #define  BM_CLOCK_MODEL                 0x01
    #define  BM_ORBIT_MODEL                 0x02

    UINT8                                   clockModel;
    UINT8                                   orbitModel;
}  __attribute__((packed)) sit_lpp_GNSS_NavigationModelSupport_Type;

typedef struct {
    //NA
}  __attribute__((packed)) sit_lpp_GNSS_RealTimeIntegritySupport_Type;

typedef struct {
    //NA
}  __attribute__((packed)) sit_lpp_GNSS_DataBitAssistanceSupport_Type;

typedef struct {
    UINT8                                   bitMask;

    #define BM_CONFIDENCE_SUPPORT           0x01
    #define BM_DOPPLER_UNC_EXT_SUPPORT      0x02

    BOOL                                 confidenceSupport;
    BOOL                                 dopplerUncExtSupport;
}  __attribute__((packed)) sit_lpp_GNSS_AcquisitionAssistanceSupport_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_ALM_MODEL                    0x01
    UINT8                                   almModel;
    #define MODEL_1                         0x01
    #define MODEL_2                         0x02
    #define MODEL_3                         0x04
    #define MODEL_4                         0x08
    #define MODEL_5                         0x10
    #define MODEL_6                         0x20

}  __attribute__((packed)) sit_lpp_GNSS_AlmanacSupport_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_UTC_MODE                     0x01
    UINT8                                   utcModel;
    #define MODEL_1                         0x01
    #define MODEL_2                         0x02
    #define MODEL_3                         0x04
    #define MODEL_4                         0x08

}  __attribute__((packed)) sit_lpp_GNSS_UTC_ModelSupport_Type;

typedef struct {
    //NA
}  __attribute__((packed)) sit_lpp_GNSS_AuxiliaryInformationSupport_Type;

typedef struct {
    UINT16                                              bitMask;

    #define   BM_SBAS_ID                                0x0001
    #define   BM_TIME_MODELS_SUPPORT                    0x0002
    #define   BM_DIFF_CORRECT_SUPPORT                   0x0004
    #define   BM_NAV_MODEL_SUPPORT                      0x0008
    #define   BM_RTI_SUPPORT                            0x0010
    #define   BM_DATA_BIT_ASSIST_SUPPORT                0x0020
    #define   BM_ACQ_ASSIST_SUPPORT                     0x0040
    #define   BM_ALM_SUPPORT                            0x0080
    #define   BM_UTC_MODEL_SUPPORT                      0x0100
    #define   BM_AUXILIARY_INFO_SUPPORT                 0x0200

    sit_lpp_GNSS_ID_Type                                gnssID;
    sit_lpp_SBAS_ID_Type                                sbasID;
    sit_lpp_GNSS_TimeModelListSupport_Type              timeModelsSupport;
    sit_lpp_GNSS_DifferentialCorrectionsSupport_Type    differentialCorrectionsSupport;
    sit_lpp_GNSS_NavigationModelSupport_Type            navModelSupport;
    sit_lpp_GNSS_RealTimeIntegritySupport_Type          rti_Support;
    sit_lpp_GNSS_DataBitAssistanceSupport_Type          dataBitAssistSupport;
    sit_lpp_GNSS_AcquisitionAssistanceSupport_Type      acqAssistSupport;
    sit_lpp_GNSS_AlmanacSupport_Type                    alm_Support;
    sit_lpp_GNSS_UTC_ModelSupport_Type                  utcModelSupport;
    sit_lpp_GNSS_AuxiliaryInformationSupport_Type       auxiliaryInfoSupport;
}  __attribute__((packed)) sit_lpp_GNSS_GenericAssistDataSupportElement_Type;

typedef struct {
    UINT8                                   accessTypes;
}  __attribute__((packed)) sit_lpp_AccessTypes_Type;

typedef struct {
    UINT8                                   bitMask;
    #define BM_FTA                          0x01
    sit_lpp_GNSS_ID_Bitmap_Type             gnssSystemTime;
    sit_lpp_AccessTypes_Type                fta;
}  __attribute__((packed)) sit_lpp_GNSS_ReferenceTimeSupport_Type;

typedef struct {

}  __attribute__((packed)) sit_lpp_GNSS_ReferenceLocationSupport_Type;

typedef struct {
    UINT8 ionoModel;
}  __attribute__((packed)) sit_lpp_GNSS_IonosphericModelSupport_Type;

typedef struct {

}  __attribute__((packed)) sit_lpp_GNSS_EarthOrientationParametersSupport_Type;


typedef struct {
    UINT8                                                 bitMask;
    #define BM_REF_TIME                                   0x01
    #define BM_REF_LOCATION                               0x02
    #define BM_IONO_MODEL                                 0x04
    #define BM_EARTH_ORIENTATION_PARAMETERS               0x08
    sit_lpp_GNSS_ReferenceTimeSupport_Type                refTime;
    sit_lpp_GNSS_ReferenceLocationSupport_Type            refLocation;
    sit_lpp_GNSS_IonosphericModelSupport_Type             ionoModel;
    sit_lpp_GNSS_EarthOrientationParametersSupport_Type   earthOrientationParameters;
}  __attribute__((packed)) sit_lpp_GNSS_CommonAssistanceDataSupport_Type;



typedef struct {
    sit_lpp_GNSS_CommonAssistanceDataSupport_Type     cmnAssistDataSupport;
    sit_lpp_GNSS_GenericAssistDataSupportElement_Type genAssistDataSupport[LPP_MAX_SVs_CNT];
    UINT8                                             sizeOfGenAssistDataSupport;
}  __attribute__((packed)) sit_lpp_AssistanceDataSupportList_Type;

typedef struct {
    UINT8                                   bitMask;

    #define  BM_SUPPORT_LIST                0x01
    #define  BM_ASSIST_SUPPORT              0x02
    #define  BM_LOCATION_COORDINATE_TYPE    0x04
    #define  BM_VELOCITY_TYPE               0x08

    sit_lpp_GNSS_Support_Element_Type       supportList[LPP_MAX_SVs_CNT];
    UINT8                                   sizeOfSupprotList;
    sit_lpp_AssistanceDataSupportList_Type  assistSupport;
    sit_lpp_locationCoordinateTypes_Type    locationCoordinateTypes;
    sit_lpp_velocityTypes                   velocityType;
}  __attribute__((packed)) sit_lpp_A_GNSS_Capabilities;

//OEM Data Definitions
typedef struct {
    UINT8                     molr_type;
    UINT8                                     location_method;
    sit_gps_qos_type                            qos;
    UINT8                                     client_id[82];
    UINT8                                     mlc_num[82];
    sit_gps_assistance_data_type            assistance_data;
    UINT8                                     gad_shape;
    UINT8                                     serviceTypeID;
    UINT8                                     pseudonymIndicator;
} __attribute__((packed)) oem_gps_mo_loc_req;

typedef struct {
    UINT16          port;
} __attribute__((packed)) oem_gps_supl_ni_ready_req;

typedef struct {
    UINT8 result;
} __attribute__((packed)) oem_gps_supl_ni_ready_rsp;

typedef struct {
    BYTE                                         result;  // 0x00 : SUCCESS, 0x01 : Fail
    UINT32                                       response_type;  //2
    sit_gps_measure_type                         gps_measure;
    sit_gps_measure_loc_info_type                 loc_info;
    sit_gps_assistance_data_type                 measured_assit_data;
    sit_gps_utran_gps_ref_time_type              UtranGpsRefTime; // only for 3G
} __attribute__((packed)) oem_gps_measure_position_rsp;

typedef struct {
    UINT8                 reqId;
    UINT8                 response;
} __attribute__((packed)) oem_gps_mtlr_notification;

typedef struct {
    sit_lpp_Header                          lpp_hdr;
    UINT8                                   flag;
    #define   OEM_LPP_CAPABILITIES_A_GNSS   0x01
    #define   OEM_LPP_CAPABILITIES_EPDU     0x02
    sit_lpp_A_GNSS_Capabilities                 a_gnss;
}__attribute__((packed))oem_gps_lpp_provide_capabilities;

typedef struct {
    sit_lpp_Header                          lpp_hdr;
    UINT8                                   flag;
    #define   OEM_LPP_ASSISTANCE_CMN        0x01
    #define   OEM_LPP_ASSISTANCE_A_GNSS     0x02
    #define   OEM_LPP_ASSISTANCE_OTDOA      0x04
    #define   OEM_LPP_ASSISTANCE_EPDU       0x08
    sit_lpp_A_GNSS_Request_Assistance_Data      a_gnss;
} __attribute__((packed)) oem_gps_lpp_request_assistance_data;

typedef struct {
    sit_lpp_Header                          lpp_hdr;
    UINT8                                   flag;
    #define   OEM_LPP_LOC_INFO_CMN          0x01
    #define   OEM_LPP_LOC_INFO_A_GNSS       0x02
    #define   OEM_LPP_LOC_INFO_OTDOA        0x04
    #define   OEM_LPP_LOC_INFO_ECID         0x08
    #define   OEM_LPP_OC_INFO_EPDU          0x10
    sit_lpp_CommonIEsProvideLocationInformation cmn;
    sit_lpp_A_GNSS_ProvideLocationInformation   a_gnss;
} __attribute__((packed)) oem_gps_lpp_provide_loc_info;

typedef struct {
    sit_lpp_Header                                              lpp_hdr;
    UINT8                                                       flag;
    #define OEM_LPP_ABORT                                       0x01
    #define OEM_LPP_ERROR                                       0x02

    UINT8                                                       statusCode;
    #define OEM_LPP_INVALIED_SESSION_ID                         0x01
    #define OEM_LPP_UNEXPECTED_MSG                              0x02
    #define OEM_LPP_NOT_ALL_REQ_MEASURE_POSSIBLE                0x03
    #define OEM_LPP_AUTH_NET_FAILURE                            0x04
    #define OEM_LPP_MEASURE_TIME_EXPIRED                        0x05

}__attribute__((packed)) oem_gps_lpp_error_info;

typedef struct {
    sit_lpp_Header                      lpp_hdr;
    UINT8                               flag;

    #define OEM_LPP_SUPL_OTODA_REQ_CAP                 0x01
    #define OEM_LPP_SUPL_OTDOA_PROV_ASSIST             0x02
    #define OEM_LPP_SUPL_OTDOA_REQ_LOC                 0x04
    #define OEM_LPP_SUPL_ECID_REQ_CAP                  0x08
    #define OEM_LPP_SUPL_ECID_REQ_LOC                  0x10
    #define OEM_LPP_SUPL_OTDOA_TRIGGER_REQ_ASSIST      0x20

    BYTE     prsInfo[512];
    UINT16   sizeOf;
    UINT8    responseTime;
} __attribute__((packed)) oem_lpp_supl_lppDataInfo;

typedef struct {
    UINT8           tid;
    UINT8               result;
} __attribute__((packed)) oem_gps_supl_ni_msg;

/* ========================================================
    Solicited Messages
// ========================================================*/

//     : Request GPS Frequency Aiding

typedef struct {
    RCM_HEADER                             hdr;
    UINT8                                        enable;
}__attribute__((packed))sit_gps_set_freq_aiding_req;

typedef struct {
    RCM_HEADER                             hdr;
    UINT8                                        lock_status;
    UINT8                                        afc_update;
}__attribute__((packed))sit_gps_set_freq_aiding_rsp;

//     : Request to Get LPP Enhanced Cell ID Information

typedef struct {
    RCM_HEADER  hdr;
    UINT8                                           flag;

    #define LPP_ECID_REQ_CAP                        0x01
    #define LPP_ECID_REQ_LOC                        0x02

    UINT8                                           responseTime;
} __attribute__((packed)) sit_gps_get_lpp_supl_ecid_Info_req;

typedef struct {
    RCM_HEADER          hdr;
    UINT8                                           flag;

    #define LTE_ECID_PROV_CAP                       0x01
    #define LTE_ECID_PROV_LOC                       0x02

    UINT8                                           bitMask;
    #define BM_LTE_RSRP                             0x01
    #define BM_LTE_RSRQ                             0x02
    #define BM_LTE_TA                               0x04
    #define BM_LTE_MRL_LIST                         0x08
    #define BM_LTE_EARFCN                           0x10

    UINT8                                           ecidSupported;
    sit_cgi_info                                    cgiInfo;
    UINT16                                          psyCellId;
    UINT8                                           rsrp;
    UINT8                                           rsrq;
    UINT16                                          ta;
    UINT16                                          earfcn;
    sit_mrl_eutra_info                              mrlList[8];
    UINT8                                           sizeOfMRL;
} __attribute__((packed)) sit_gps_get_lpp_supl_ecid_Info_rsp;

//     : Request to Get RRLP Enhanced Cell ID Information

typedef struct {
    RCM_HEADER                            hdr;

    UINT8                                   flag;
    #define RRLP_ECID_REQ_CAP              0x01
    #define RRLP_ECID_REQ_LOC              0x02

    UINT8                                   responseTime;
}  __attribute__((packed)) sit_gps_get_rrlp_supl_ecid_Info_req;

typedef struct {
    RCM_HEADER                            hdr;

    UINT8                                   flag;
    #define WCDMA_ECID_PROV_CAP             0x01
    #define WCDMA_ECID_PROV_LOC             0x02

    UINT8                                   ecidSupported;

    UINT8                                   cell_info_type;
    #define     GSM_CELL_INFO               0x01
    #define     WCDMA_CELL_INFO             0x02
    union{
        sit_gsm_cell_info                   gsm_cell;
        sit_wcdma_cell_info                 wcdma_cell;
    }u_cellinfo;
}  __attribute__((packed)) sit_gps_get_rrlp_supl_ecid_Info_rsp;

//     : Send MO Location Request

typedef struct {
    RCM_HEADER                  hdr;
    BYTE                          molr_type;
    BYTE                          location_method;
    sit_gps_qos_type              qos;
    BYTE                          client_id[82];
    BYTE                          mlc_num[82];
    sit_gps_assistance_data_type  assistance_data;
    BYTE                          gad_shape;
    BYTE                          serviceTypeID;
    BYTE                          pseudonymIndicator;
}__attribute__((packed))sit_gps_set_mo_loc_req;

typedef struct {
    RCM_HEADER                  hdr;
    sit_gps_loc_info_type         loc_info;
    BYTE                          no_loc;
    sit_gps_deciphering_keys_type decper_keys;
}__attribute__((packed))sit_gps_set_mo_loc_rsp;

//     : Get Serving Cell Information
typedef struct {
    RCM_HEADER                             hdr;
    UINT8                                       enable;
}__attribute__((packed))sit_gps_get_serving_cell_info_req;

typedef struct {
    RCM_HEADER                             hdr;
    BYTE                                       tac[2];
    UINT16                                       psc;
    UINT32                                       cid;
}__attribute__((packed))sit_gps_get_serving_cell_info_rsp;

//     : Request SUPL NI Ready

typedef struct {
    RCM_HEADER  hdr;
    UINT16        port;
} __attribute__((packed)) sit_gps_set_supl_ni_ready_req;

typedef struct {
    RCM_HEADER    hdr;
    UINT16          result;
} __attribute__((packed)) sit_gps_set_supl_ni_ready_rsp;


/* ========================================================
    Unsolicited Messages
// ========================================================*/

/*------------------------From AP to Network-----------------------*/

//     : GPS AP Provide Measure Position

typedef struct {
    RCM_IND_HEADER                                 hdr;
    BYTE                                         result;  // 0x00 : SUCCESS, 0x01 : Fail
    UINT32                                       response_type;
    sit_gps_measure_type                         gps_measure;
    sit_gps_measure_loc_info_type                 loc_info;
    sit_gps_assistance_data_type                 measured_assit_data;
    sit_gps_utran_gps_ref_time_type              UtranGpsRefTime; // only for 3G
}__attribute__((packed))sit_gps_ap_provide_measure_position_ind;

//     : GPS Release GPS
typedef struct {
    RCM_IND_HEADER                              hdr;
}  __attribute__((packed)) sit_gps_rel_gps_ind;

//     : GPS AP MT Location

typedef struct {
    RCM_IND_HEADER                             hdr;
    UINT8                                        reqId;
    BYTE                                         response;
}__attribute__((packed))sit_gps_ap_mtlr_ind;

//     : GPS AP Provide Capabilities

typedef struct {
    RCM_IND_HEADER                              hdr;
    sit_lpp_Header                            lpp_hdr;
    UINT8                                     flag;
    #define   LPP_CAPABILITIES_A_GNSS         0x01
    #define   LPP_CAPABILITIES_EPDU           0x02
    sit_lpp_A_GNSS_Capabilities               a_gnss;
}  __attribute__((packed)) sit_gps_ap_provide_capabilities_ind;

//     : Get LPP Request Assistance Data

typedef struct {
    RCM_IND_HEADER                              hdr;
    sit_lpp_Header                            lpp_hdr;
    UINT8                                     flag;
    #define   LPP_ASSISTANCE_CMN              0x01
    #define   LPP_ASSISTANCE_A_GNSS           0x02
    #define   LPP_ASSISTANCE_OTDOA            0x04
    #define   LPP_ASSISTANCE_EPDU             0x08
    sit_lpp_A_GNSS_Request_Assistance_Data    a_gnss;
} __attribute__((packed)) sit_gps_lpp_req_assist_data_ind;

//     : GPS AP Provide Location Information

typedef struct {
    RCM_IND_HEADER                                  hdr;
    sit_lpp_Header                                lpp_hdr;
    UINT8                                         flag;
    #define   LPP_LOC_INFO_CMN                    0x01
    #define   LPP_LOC_INFO_A_GNSS                 0x02
    #define   LPP_LOC_INFO_OTDOA                  0x04
    #define   LPP_LOC_INFO_ECID                   0x08
    #define   LPP_OC_INFO_EPDU                    0x10
    sit_lpp_CommonIEsProvideLocationInformation   cmn;
    sit_lpp_A_GNSS_ProvideLocationInformation     a_gnss;
} __attribute__((packed)) sit_gps_ap_provide_location_info_ind;

//     : GPS AP LPP Error

typedef struct {
    RCM_IND_HEADER                                            hdr;
    sit_lpp_Header                                          lpp_hdr;
    UINT8                                                   flag;
    #define LPP_ABORT                                       0x01
    #define LPP_ERROR                                       0x02
    UINT8                                                   statusCode;
    #define LPP_INVALIED_SESSION_ID                         0x01
    #define LPP_UNEXPECTED_MSG                              0x02
    #define LPP_NOT_ALL_REQ_MEASURE_POSSIBLE                0x03
    #define LPP_AUTH_NET_FAILURE                            0x04
    #define LPP_MEASURE_TIME_EXPIRED                        0x05
}__attribute__((packed)) sit_gps_ap_lpp_error_ind;

//     : GPS SUPL LPP Data Information

typedef struct {
    RCM_IND_HEADER                                    hdr;
    sit_lpp_Header                                  lpp_hdr;
    UINT8                                           flag;

    #define  LPP_SUPL_OTODA_REQ_CAP                 0x01
    #define  LPP_SUPL_OTDOA_PROV_ASSIST             0x02
    #define  LPP_SUPL_OTDOA_REQ_LOC                 0x04
    #define  LPP_SUPL_ECID_REQ_CAP                  0x08
    #define  LPP_SUPL_ECID_REQ_LOC                  0x10
    #define  LPP_SUPL_OTDOA_TRIGGER_REQ_ASSIST      0x20

    BYTE                                            prsInfo[512];
    UINT16                                          sizeOf;
    UINT8                                           responseTime;
}__attribute__((packed)) sit_gps_ap_supl_lpp_data_info_ind;

//     : GPS AP SUPL NI Message

typedef struct {
    RCM_IND_HEADER    hdr;
    UINT8           tid;
    UINT8           result;
} __attribute__((packed)) sit_gps_ap_supl_ni_message_ind;

/*------------------------From Network to AP-----------------------*/

//     : GPS Measure Position Message

typedef struct {
    RCM_IND_HEADER                          hdr;
    BYTE                                  method_type;
    sit_gps_m_accuracy_type               accuracy;
    UINT8                                 rsp_time;
    BYTE                                  use_multi_sets;
    BYTE                                  envronment_char;
    BYTE                                  cell_timing_wnt;
    BYTE                                  add_assist_req;
}__attribute__((packed))sit_gps_mea_position_ind;

//     : GPS Assist Data

typedef struct {
    RCM_IND_HEADER                          hdr;
    UINT32                                    flag;
    sit_gps_ref_time_type                     ref_time;
    sit_gps_ref_loc_type                      ref_loc;
    sit_gps_dgps_correction_type              dgps_corrections;
    sit_gps_navi_model_type                   navi_model;
    sit_gps_iono_model_type                   iono_model;
    sit_gps_utc_model_type                    utc_model;
    sit_gps_almanac_model_type                almanac;
    sit_gps_acq_assist_type                   acq_assist;
    sit_gps_r_time_int_type                   r_time_int;
}__attribute__((packed))sit_gps_assist_data_ind;

//     : Release GPS

//     : MT Location Request Message

typedef struct {
    RCM_IND_HEADER                             hdr;
    BYTE                          req_id;
    BYTE                                         notify_type;
    sit_gps_mtlr_loc_type                        loc;
    BYTE                                         client_id[82];
    sit_gps_dcs_string_type                      client_name;
    sit_gps_dcs_string_type                      requestor_id;
    sit_gps_code_word_type                       code_word;
    BYTE                                         svc_type_id;
}__attribute__((packed))sit_gps_mtlr_ind;

//     : Unsol GPS Reset Assist Data

//     : LPP Request Capabilities

typedef struct {
    RCM_IND_HEADER                              hdr;
    sit_lpp_Header                            lpp_hdr;
    BOOL                                   cmn;
    BOOL                                   a_gnss;
    BOOL                                   epdu;
}  __attribute__((packed)) sit_gps_lpp_request_capabilities_ind;

//     : GPS LPP Provide Assist Data

typedef struct {
    RCM_IND_HEADER                              hdr;
    sit_lpp_Header                            lpp_hdr;
    UINT8                                     flag;
    #define   LPP_ASSISTANCE_CMN              0x01
    #define   LPP_ASSISTANCE_A_GNSS           0x02
    #define   LPP_ASSISTANCE_OTDOA            0x04
    #define   LPP_ASSISTANCE_EPDU             0x08
    sit_lpp_A_GNSS_Provide_Assistance_Data    a_gnss;
} __attribute__((packed)) sit_gps_lpp_provide_assist_data_ind;

//     : LPP Request Location Information

typedef struct {
    RCM_IND_HEADER                                  hdr;
    sit_lpp_Header                                lpp_hdr;
    UINT8                                         flag;
    #define   LPP_LOC_INFO_CMN                    0x01
    #define   LPP_LOC_INFO_A_GNSS                 0x02
    #define   LPP_LOC_INFO_OTDOA                  0x04
    #define   LPP_LOC_INFO_ECID                   0x08
    #define   LPP_OC_INFO_EPDU                    0x10
    sit_lpp_CommonIEsRequestLocationInformation   cmn;
    sit_lpp_A_GNSS_RequestLocationInformation     a_gnss;
} __attribute__((packed)) sit_gps_lpp_request_location_info_ind;

//     : GPS LPP Error from Network

typedef struct {
    RCM_IND_HEADER                                                                hdr;
    sit_lpp_Header                                                              lpp_hdr;
    UINT8                                                                       flag;
    #define LPP_ABORT                                                           0x01
    #define LPP_ERROR                                                           0x02
    UINT8                                                                       statusCode;
    #define LPP_INVALIED_SESSION_ID                                             0x01
    #define LPP_UNEXPECTED_MSG                                                  0x02
    #define LPP_ASSIST_DATA_PARTLY_NO_SUPPORT_NO_AVAIL_SERVER                   0x03
    #define LPP_TIME_EXPIRED                                                    0x04
    #define LPP_METHOD_FAILURE                                                  0x05
    #define LPP_PARTIAL_REQ_MEASURE_POSSIBLE                                    0x06
    #define LPP_SERVER_DISCONNECTED                                             0x07
    #define LPP_UNDELIVERED_ASSIST_DATA_IS_NOT_SUPPORTED_BY_SERVER              0x08
    #define LPP_UNDELIVERED_ASSIST_DATA_IS_SUPPORTED_BUT_AVAILABLE_BY_SERVER    0x09
}__attribute__((packed)) sit_gps_network_lpp_error_ind;

//     : GPS SUPL LPP Data Information

typedef struct {
    RCM_IND_HEADER                                    hdr;
    sit_lpp_Header                                  lpp_hdr;
    UINT8                                           flag;

    #define  LPP_SUPL_OTDOA_PROV_CAP                0x01
    #define  LPP_SUPL_OTDOA_REQ_ASSIST              0x02
    #define  LPP_SUPL_OTDOA_PROV_LOC                0x04
    #define  LPP_SUPL_ECID_PROV_CAP                 0x08
    #define  LPP_SUPL_ECID_PROV_LOC                 0x10

    BYTE                                            lppInfo[512];
    UINT16                                          sizeOf;
} __attribute__((packed)) sit_gps_cp_supl_lpp_data_info_ind;

//     : GPS Network SUPL NI Message

typedef struct {
    RCM_IND_HEADER          hdr;
    UINT8               tid;
    UINT16               sizeOf;
    BYTE                suplMsg[256];
} __attribute__((packed)) sit_gps_network_supl_ni_msg_ind;

/*
SIT_SET_EMBMS_SESSION = 0x1001,
SIT_IND_EMBMS_COVERAGE = 0x1002,
SIT_GET_EMBMS_SESSION_LIST = 0x1003,
SIT_IND_EMBMS_SESSION_LIST = 0x1004,
SIT_GET_EMBMS_SIGNAL_STRENGTH = 0x1005,
SIT_IND_EMBMS_SIGNAL_STRENGTH = 0x1006,
SIT_GET_EMBMS_NETWORK_TIME = 0x1007,
SIT_IND_EMBMS_NETWORK_TIME = 0x1008,
SIT_IND_EMBMS_SAI_LIST = 0x1009,
SIT_IND_GLOBAL_CELL_ID = 0x100A,
*/

/*
    SIT_SET_EMBMS_SERVICE (RCM ID = 0x1000)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE service_state;
} sit_embms_set_service_req;

typedef null_data_format sit_embms_set_service_rsp;

/*
    SIT_SET_EMBMS_SESSION (RCM ID = 0x1001)
*/

#define MAX_SAI_LIST_LEN    (64)
#define MAX_REQ_LIST_LEN    (8)

typedef struct
{
    RCM_HEADER hdr;
    BYTE session_state;
    BYTE TMGI[6];
    BYTE deActTMGI[6];
    UINT8 sai_list_len;
    UINT16 saiList[MAX_SAI_LIST_LEN];
    UINT8 freq_list_len;
    UINT32 freqList[MAX_REQ_LIST_LEN];
} __attribute__((packed)) sit_embms_set_session_req;

/*
    SIT_IND_EMBMS_COVERAGE (RCM ID = 0x1002)
*/

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE coverage;
}sit_embms_coverage_rsp;

enum {
    EMBMS_NO_COVERAGE = 0,
    EMBMS_UNICAST_COVERAGE,
    EMBMS_FULL_COVERAGE,
    EMBMS_UNKNOW_STATE_COVERAGE = 0xFF
};

/*
    SIT_GET_EMBMS_SESSION_LIST (RCM ID = 0x1003)
*/

#define EMBMS_MAX_INTRA_SAILIST_NUMBER  (64)
#define EMBMS_TMGI_LEN (6)

typedef struct
{
    RCM_HEADER hdr;
    BYTE session_state;
} sit_embms_ssesion_list_req;

typedef struct
{
    RCM_HEADER hdr;
    BYTE session_state;
    BYTE oos_reason;
    BYTE number_record;
    BYTE tmgi[EMBMS_MAX_INTRA_SAILIST_NUMBER*EMBMS_TMGI_LEN];
} sit_embms_ssesion_list_rsp;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE session_state;
    BYTE oos_reason;
    BYTE number_record;
    BYTE tmgi[EMBMS_MAX_INTRA_SAILIST_NUMBER*EMBMS_TMGI_LEN];
} sit_embms_ssesion_list_ind;

/*
    SIT_GET_EMBMS_SIGNAL_STRENGTH (RCM ID = 0x1005)
*/

#define MAX_SIGNAL_RECORD_NUM   (8)
typedef null_data_format sit_embms_signal_strength_req;

typedef struct
{
    UINT32 SNR; // Signal Noise Radio [db]
    BYTE MBSFNAreaId;
    UINT32 ESNR;
    BYTE num_tmgi;
    BYTE tmgi[EMBMS_MAX_INTRA_SAILIST_NUMBER][EMBMS_TMGI_LEN];
} __attribute__((packed)) signal_record;

typedef struct
{
    RCM_HEADER hdr;
    BYTE number_record;
    signal_record signal_record[MAX_SIGNAL_RECORD_NUM];
} __attribute__((packed)) sit_embms_signal_strength_rsp;

typedef struct
{
    RCM_HEADER hdr;
    BYTE number_record;
    signal_record signal_record[MAX_SIGNAL_RECORD_NUM];
} __attribute__((packed)) sit_embms_signal_strength_ind;

/*
    SIT_GET_EMBMS_NETWORK_TIME (RCM ID = 0x1007)
*/
typedef struct
{
    RCM_HEADER hdr;
    BYTE Sib16Acquired;
    BYTE TimeInfoType;
    BYTE daylight_valid;
    BYTE year;
    BYTE month;
    BYTE day;
    BYTE hour;
    BYTE minute;
    BYTE second;
    BYTE time_zone;
    BYTE daylight_adjust;
    BYTE day_of_week;
    BYTE PLMN[6]; // MCC(3), MNC(3)
    unsigned long long AbsoluteTime;
    BYTE LeapSecond;
} __attribute__((packed)) sit_embms_network_time_rsp;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE Sib16Acquired;
    BYTE TimeInfoType;
    BYTE daylight_valid;
    BYTE year;
    BYTE month;
    BYTE day;
    BYTE hour;
    BYTE minute;
    BYTE second;
    BYTE time_zone;
    BYTE daylight_adjust;
    BYTE day_of_week;
    BYTE PLMN[6]; // MCC(3), MNC(3)
    unsigned long long AbsoluteTime;
    BYTE LeapSecond;
} __attribute__((packed)) sit_embms_network_time_ind;

/*
    SIT_IND_EMBMS_SAI_LIST (RCM ID = 0x1009)
*/

#define MAX_INTER_SAI_NUMBER        (64)
#define MAX_MULTI_BAND_NUMBER       (8)
#define MAX_INTRA_SAILIST_NUMBER    (64)
#define MAX_INTER_SAILIST_NUMBER    (8)

typedef struct
{
    UINT32 Frequency;
    BYTE InterSaiNumber; // max 64
    BYTE MultiBandInfoNumber; // max 8
    WORD InterSaiInfo[MAX_INTER_SAI_NUMBER];
    BYTE MultiBandInfo[MAX_MULTI_BAND_NUMBER];
} __attribute__((packed)) sit_embms_inter_sailist;

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE IntraSaiListNum; // max 64
    BYTE InterSaiListNum; // max 8
    WORD IntraSaiList[MAX_INTRA_SAILIST_NUMBER];
    sit_embms_inter_sailist InterSaiList[MAX_INTER_SAILIST_NUMBER];
} __attribute__((packed)) sit_embms_sailist;

/*
    SIT_IND_GLOBAL_CELL_ID (RCM ID = 0x100A)
*/
#define MAX_MCC_LEN (3)
#define MAX_MNC_LEN (3)
#define MAX_CELLID_LEN (4)

typedef struct
{
    RCM_IND_HEADER hdr;
    BYTE mcc[MAX_MCC_LEN];
    BYTE mnc[MAX_MNC_LEN];
    UINT32 cellId;
}sit_embms_global_cellid_rsp;

#pragma pack()

#endif /*_SIT_DEF_H_*/
