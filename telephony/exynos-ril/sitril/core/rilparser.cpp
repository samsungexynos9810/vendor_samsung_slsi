/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "rilparser.h"
#include "callreqdata.h"
#include "datacallreqdata.h"
#include "simdata.h"
#include "miscdata.h"
#include "smsdata.h"
#include "wlandata.h"
#include "oemreqdata.h"
#include "rillog.h"
#include "netdata.h"
#include "nvitemdata.h"
#include "vsimdata.h"
#include "embmsdata.h"

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define TAG "RilParser"

static ParseFuncMap_t szParseFuncTable[] =
{
    // SIM
    {RIL_REQUEST_QUERY_FACILITY_LOCK, RilParser::CreateStrings},
    {RIL_REQUEST_SET_FACILITY_LOCK, RilParser::CreateStrings},
    {RIL_REQUEST_ENTER_SIM_PIN, RilParser::CreateStrings},
    {RIL_REQUEST_ENTER_SIM_PUK, RilParser::CreateStrings},
    {RIL_REQUEST_ENTER_SIM_PIN2, RilParser::CreateStrings},
    {RIL_REQUEST_ENTER_SIM_PUK2, RilParser::CreateStrings},
    {RIL_REQUEST_CHANGE_SIM_PIN, RilParser::CreateStrings},
    {RIL_REQUEST_CHANGE_SIM_PIN2, RilParser::CreateStrings},
    {RIL_REQUEST_SIM_IO, RilParser::CreateSimIoData},
    {RIL_REQUEST_GET_IMSI, RilParser::CreateStrings},
    {RIL_REQUEST_ISIM_AUTHENTICATION, RilParser::CreateString},
    {RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION, RilParser::CreateStrings},
    {RIL_REQUEST_SET_UICC_SUBSCRIPTION, RilParser::CreateSimUiccSubscription},
    {RIL_REQUEST_SIM_AUTHENTICATION, RilParser::CreateSimAuthentication},
    {RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE, RilParser::CreateString},
    {RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND, RilParser::CreateString},
    {RIL_REQUEST_STK_SEND_ENVELOPE_WITH_STATUS, RilParser::CreateString},
    {RIL_REQUEST_STK_SET_PROFILE, RilParser::CreateString},
    {RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM, RilParser::CreateInt},
    {RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC, RilParser::CreateSimAPDU},
    {RIL_REQUEST_SIM_OPEN_CHANNEL, RilParser::CreateSimOpenChannel},
    {RIL_REQUEST_SIM_CLOSE_CHANNEL, RilParser::CreateInt},
    {RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL, RilParser::CreateSimAPDU},
    {RIL_REQUEST_SET_CARRIER_RESTRICTIONS, RilParser::CreateCarrierRestrictions},
    {RIL_REQUEST_SET_SIM_CARD_POWER, RilParser::CreateInt},
    {RIL_REQUEST_SET_LOGICAL_TO_PHYSICAL_SLOT_MAPPING, RilParser::CreateInts},

    // Data
    {RIL_REQUEST_SETUP_DATA_CALL, RilParser::CreateSetupDataCall},
    {RIL_REQUEST_DEACTIVATE_DATA_CALL, RilParser::CreateDeactivateDataCall},
    {RIL_REQUEST_DEACTIVATE_DATA_CALL_WITH_REASON, RilParser::CreateDeactivateDataCall},
    {RIL_REQUEST_SET_INITIAL_ATTACH_APN, RilParser::CreateSetInitialAttachApn},
    {RIL_REQUEST_SET_DATA_PROFILE, RilParser::CreateSetDataProfile},
    {RIL_REQUEST_START_KEEPALIVE, RilParser::CreateRequestKeepalive},
    {RIL_REQUEST_STOP_KEEPALIVE, RilParser::CreateInt},

    // Call & SS
    {RIL_REQUEST_DIAL, RilParser::CreateCallDial},
    {RIL_REQUEST_DIAL_WITH_CALL_TYPE, RilParser::CreateCallDial},
    {RIL_REQUEST_EMERGENCY_DIAL, RilParser::CreateCallEmergencyDial},
    {RIL_REQUEST_HANGUP, RilParser::CreateInt},
    {RIL_REQUEST_SET_CLIR, RilParser::CreateInt},
    {RIL_REQUEST_QUERY_CALL_WAITING, RilParser::CreateInts},
    {RIL_REQUEST_SET_CALL_WAITING, RilParser::CreateInts},
    {RIL_REQUEST_QUERY_CALL_FORWARD_STATUS, RilParser::CreateCallForward},
    {RIL_REQUEST_SET_CALL_FORWARD, RilParser::CreateCallForward},
    {RIL_REQUEST_CHANGE_BARRING_PASSWORD, RilParser::CreateStrings},
    {RIL_REQUEST_CHANGE_BARRING_PASSWORD_OVER_MMI, RilParser::CreateStrings},
    {RIL_REQUEST_SEND_USSD, RilParser::CreateString},
    {RIL_REQUEST_SEPARATE_CONNECTION, RilParser::CreateInt},
    {RIL_REQUEST_DTMF_START, RilParser::CreateStartDtmf},
    {RIL_REQUEST_DTMF, RilParser::CreateSendDtmf},
    {RIL_REQUEST_DTMF_STOP, RilParser::CreateStopDtmf},

    // Network
    {RIL_REQUEST_RADIO_POWER, RilParser::CreateInt},
    {RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE, RilParser::CreateInt},
    {RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE, RilParser::CreateInt},
    {RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL_WITH_RAT, RilParser::CreateStrings},  // RIL Extension, PLMN + RAT
    {RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL, RilParser::CreateString},   // AOSP, PLMN only
    {RIL_REQUEST_SET_BAND_MODE, RilParser::CreateInt},
    {RIL_REQUEST_ALLOW_DATA, RilParser::CreateInt},
    {RIL_REQUEST_SET_RADIO_CAPABILITY, RilParser::CreateNetRCData},
    {RIL_REQUEST_SET_DUAL_NETWORK_AND_ALLOW_DATA, RilParser::CreateInts},
    {RIL_REQUEST_START_NETWORK_SCAN, RilParser::CreateNetworkScanRequest},
    {RIL_REQUEST_SET_LOCATION_UPDATES, RilParser::CreateInt},

    // SMS
    {RIL_REQUEST_SEND_SMS, RilParser::CreateGsmSms},
    {RIL_REQUEST_SEND_SMS_EXPECT_MORE, RilParser::CreateGsmSms},
    {RIL_REQUEST_SMS_ACKNOWLEDGE, RilParser::CreateSmsAck},
    {RIL_REQUEST_SET_SMSC_ADDRESS, RilParser::CreateString},
    {RIL_REQUEST_WRITE_SMS_TO_SIM, RilParser::CreateSimSmsData},
    {RIL_REQUEST_DELETE_SMS_ON_SIM, RilParser::CreateInt},
    {RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION, RilParser::CreateInt},
    {RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG, RilParser::CreateSetSmsConfig},
    {RIL_REQUEST_REPORT_SMS_MEMORY_STATUS, RilParser::CreateInt},
    {RIL_REQUEST_ACKNOWLEDGE_INCOMING_GSM_SMS_WITH_PDU, RilParser::CreateSmsAckPdu},
    {RIL_REQUEST_GET_SMS_STORAGE_ON_SIM, RilParser::CreateInt},

    // Sound
    {RIL_REQUEST_SET_MUTE, RilParser::CreateInt},

    // Misc & etc
    {RIL_REQUEST_SCREEN_STATE, RilParser::CreateInt},
    {RIL_REQUEST_SEND_DEVICE_STATE, RilParser::CreateInts},
    {RIL_REQUEST_SET_TTY_MODE, RilParser::CreateInt},
    {RIL_REQUEST_OEM_HOOK_RAW, RilParser::CreateHookRaw},
    {RIL_REQUEST_NV_READ_ITEM, RilParser::CreateNvReadItem},
    {RIL_REQUEST_NV_WRITE_ITEM, RilParser::CreateNvWriteItem},
    {RIL_REQUEST_SET_DEVICE_INFO, RilParser::CreateStrings},
    {RIL_REQUEST_SET_VOICE_OPERATION, RilParser::CreateInt},
    {RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER, RilParser::CreateInt},
    {RIL_REQUEST_START_LCE, RilParser::CreateInts},
    {RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION, RilParser::CreateCarrierInfoForImsiEncryption},
    {RIL_REQUEST_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA, RilParser::CreateSignalStrengthReportingCriteria},
    {RIL_REQUEST_SET_LINK_CAPACITY_REPORTING_CRITERIA, RilParser::CreateLinkCapacityReportingCriteria},
    {RIL_REQUEST_SET_PREFERRED_DATA_MODEM, RilParser::CreateInt},
    {RIL_REQUEST_SET_MODEMS_CONFIG, RilParser::CreateInt},
    {RIL_REQUEST_ENABLE_MODEM, RilParser::CreateInt},
    {RIL_REQUEST_SET_ACTIVATE_VSIM, RilParser::CreateSetActivateVsim},

    /////////////////////////////////////////////////////////
    // Extension Request Messages
    /////////////////////////////////////////////////////////
    // SIM PhoneBook
    {RIL_REQUEST_READ_PB_ENTRY, RilParser::CreateInts},
    {RIL_REQUEST_UPDATE_PB_ENTRY, RilParser::CreateUpdatePbEntry},
    {RIL_REQUEST_GET_PB_STORAGE_INFO, RilParser::CreateInt},
    {RIL_REQUEST_GET_PB_ENTRY_INFO, RilParser::CreateInt},
    // Call & SS
    {RIL_REQUEST_SWITCH_VOICE_CALL, RilParser::CreateInts},
    {RIL_REQUEST_SET_CALL_CONFIRM, RilParser::CreateInt},
    // Network
    {RIL_REQUEST_SEND_ENCODED_USSD, RilParser::CreateStrings},             // RIL Extension, SEND USSD STK Function
    {RIL_REQUEST_SET_UPLMN, RilParser::CreateStrings},
    {RIL_REQUEST_SET_DS_NETWORK_TYPE, RilParser::CreateInt},
    {RIL_REQUEST_SET_DUPLEX_MODE, RilParser::CreateInt},
    {RIL_REQUEST_SET_FEMTO_CELL_SRCH, RilParser::CreateInt},

    // Audio
    {RIL_REQUEST_SET_WBAMR_CAPABILITY, RilParser::CreateInt},

    // VSIM
    {RIL_LOCAL_REQUEST_VSIM_NOTIFICATION, RilParser::CreateInts},
    {RIL_LOCAL_REQUEST_VSIM_OPERATION, RilParser::CreateVsimOperation},

    //Emergency Call
    {RIL_REQUEST_SET_EMERGENCY_CALL_STATUS, RilParser::CreateInts},

    // NR EN-DC
    {RIL_REQUEST_SET_ENDC_MODE, RilParser::CreateInt},

    /////////////////////////////////////////////////////////
    // OEM Request Messages
    /////////////////////////////////////////////////////////
    {RIL_REQUEST_OEM_VOLUME_SET, RilParser::CreateInt},
    {RIL_REQUEST_OEM_AUDIO_PATH_SET, RilParser::CreateInt},
    {RIL_REQUEST_OEM_MICROPHONE_SET, RilParser::CreateInt},
    {RIL_REQUEST_OEM_AUDIO_CLOCK_SET, RilParser::CreateInt},
    {RIL_REQUEST_OEM_AUDIO_LOOPBACK_SET, RilParser::CreateInts},
    {RIL_REQUEST_OEM_SET_TTY_MODE, RilParser::CreateInt},

    {RIL_REQUEST_OEM_IMS_SET_CONFIGURATION, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_IMS_GET_CONFIGURATION, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_IMS_SIM_AUTH, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_IMS_SET_EMERGENCY_CALL_STATUS, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_IMS_SET_SRVCC_CALL_LIST, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_IMS_GET_GBA_AUTH, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_IMS_SIM_IO, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_GET_IMS_SUPPORT_SERVICE, RilParser::CreateRawData},

    {RIL_REQUEST_OEM_SET_ENG_MODE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SET_SCR_LINE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SET_DEBUG_TRACE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SET_CARRIER_CONFIG, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SET_ENG_STRING_INPUT, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_APN_SETTINGS, RilParser::CreateApnSettings},
    {RIL_REQUEST_OEM_GET_MSL_CODE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SET_PIN_CONTROL, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_GET_MANUAL_BAND_MODE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SET_MANUAL_BAND_MODE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_GET_RF_DESENSE_MODE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SET_RF_DESENSE_MODE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_STORE_ADB_SERIAL_NUMBER, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_READ_ADB_SERIAL_NUMBER, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SET_PREFERRED_CALL_CAPABILITY, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_GET_PREFERRED_CALL_CAPABILITY, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SEND_SGC, RilParser::CreateInt},
    {RIL_REQUEST_OEM_SET_MANUAL_RAT_MODE, RilParser::CreateInts},
    {RIL_REQUEST_OEM_SET_FREQUENCY_LOCK, RilParser::CreateInts},
    {RIL_REQUEST_OEM_SET_ENDC_MODE, RilParser::CreateInt},
    {RIL_REQUEST_OEM_SET_UICC_SUBSCRIPTION, RilParser::CreateSimUiccSubscription},
    {RIL_REQUEST_OEM_GET_AVAILABLE_NETWORKS, RilParser::CreateInt},
    {RIL_REQUEST_OEM_GET_FREQUENCY_INFO, RilParser::CreateRawData},

    // for WLan
    {RIL_REQUEST_OEM_SIM_AUTHENTICATION, RilParser::CreateOemSimAuthRequest},

    // Secure Element
    {RIL_REQUEST_OEM_SIM_OPEN_CHANNEL, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SIM_TRANSMIT_APDU_LOGICAL, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SIM_TRANSMIT_APDU_BASIC, RilParser::CreateRawData},

    //AIMS support start ---------------------
    {RIL_REQUEST_OEM_AIMS_DIAL, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_ANSWER, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_HANGUP, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_DEREGISTRATION, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_HIDDEN_MENU, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_ADD_PDN_INFO, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_CALL_MANAGE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SEND_DTMF, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SET_FRAME_TIME, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_GET_FRAME_TIME, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_CALL_MODIFY, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RESPONSE_CALL_MODIFY, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_TIME_INFO, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_CONF_CALL_ADD_REMOVE_USER, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_ENHANCED_CONF_CALL, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_GET_CALL_FORWARD_STATUS, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SET_CALL_FORWARD_STATUS, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_GET_CALL_WAITING, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SET_CALL_WAITING, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_GET_CALL_BARRING, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SET_CALL_BARRING, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SEND_SMS, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SEND_EXPECT_MORE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SEND_SMS_ACK, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SEND_ACK_INCOMING_SMS, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_CHG_BARRING_PWD, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SEND_USSD_INFO, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_GET_PRESENTATION_SETTINGS, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SET_PRESENTATION_SETTINGS, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SET_SELF_CAPABILITY, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_HO_TO_WIFI_READY, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_HO_TO_WIFI_CANCEL_IND, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_HO_PAYLOAD_IND, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_HO_TO_3GPP, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SEND_ACK_INCOMING_CDMA_SMS, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_MEDIA_STATE_IND, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_DEL_PDN_INFO, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_STACK_START_REQ, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_STACK_STOP_REQ, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_XCAPM_START_REQ, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_XCAPM_STOP_REQ, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RTT_SEND_TEXT, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_EXIT_EMERGENCY_CB_MODE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SET_GEO_LOCATION_INFO, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_CDMA_SEND_SMS, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RCS_MULTI_FRAME, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RCS_CHAT, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RCS_GROUP_CHAT, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RCS_OFFLINE_MODE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RCS_FILE_TRANSFER, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RCS_COMMON_MESSAGE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RCS_CONTENT_SHARE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RCS_PRESENCE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_XCAP_MANAGE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RCS_CONFIG_MANAGE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_RCS_TLS_MANAGE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SET_PDN_EST_STATUS, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SET_HIDDEN_MENU_ITEM, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_GET_HIDDEN_MENU_ITEM, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_AIMS_SET_RTP_RX_STATISTICS, RilParser::CreateRawData},

    //AIMS support end ---------------------

    /* WFC */
    {RIL_REQUEST_OEM_WFC_MEDIA_CHANNEL_CONFIG, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_WFC_DTMF_START, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_WFC_SET_VOWIFI_HO_THRESHOLD, RilParser::CreateRawData},

    {RIL_REQUEST_OEM_IF_EXECUTE_AM, RilParser::CreateRawData},

    /* GPS */
    { RIL_REQUEST_OEM_GPS_SET_FREQUENCY_AIDING, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_GET_LPP_SUPL_REQ_ECID_INFO, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_SET_RRLP_SUPL_REQ_ECID_INFO, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_MO_LOCATION_REQUEST, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_GET_LPP_REQ_SERVING_CELL_INFO, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_SET_SUPL_NI_READY, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_GET_GSM_EXT_INFO_MSG, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_CONTROL_PLANE_ENABLE, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_GNSS_LPP_PROFILE_SET, RilParser::CreateRawData},
    // Indication from AP, No resp
    { RIL_REQUEST_OEM_GPS_MEASURE_POS_RSP, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_RELEASE_GPS, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_MT_LOCATION_REQUEST, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_LPP_PROVIDE_CAPABILITIES, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_LPP_REQUEST_ASSIST_DATA, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_LPP_PROVIDE_LOCATION_INFO, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_LPP_GPS_ERROR_IND, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_SUPL_LPP_DATA_INFO, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_SUPL_NI_MESSAGE, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_RETRIEVE_LOC_INFO, RilParser::CreateRawData},
    /* for CDMA & & HEDGE GANSS */
    { RIL_REQUEST_OEM_GPS_SET_GANSS_MEAS_POS_RSP, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_SET_GPS_LOCK_MODE, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_GET_REFERENCE_LOCATION, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_SET_PSEUDO_RANGE_MEASUREMENTS, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_GET_CDMA_PRECISE_TIME_AIDING_INFO, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GPS_CDMA_FREQ_AIDING, RilParser::CreateRawData},
    // Indication from AP, No resp
    { RIL_REQUEST_OEM_GPS_GANSS_AP_POS_CAP_RSP, RilParser::CreateRawData},
    /* GPS end */

    /* eMBMS */
    { RIL_REQUEST_OEM_EMBMS_ENABLE_SERVICE, RilParser::CreateInt},
    { RIL_REQUEST_OEM_EMBMS_DISABLE_SERVICE, RilParser::CreateInt},
    { RIL_REQUEST_OEM_EMBMS_SET_SESSION, RilParser::CreateEmbmsSessionData},
    { RIL_REQUEST_OEM_EMBMS_GET_SESSION_LIST, RilParser::CreateInt},

    //P-SENSOR
    { RIL_REQUEST_OEM_PSENSOR_SET_STATUS, RilParser::CreateInt},

    // VSIM
    { RIL_REQUEST_OEM_VSIM_NOTIFICATION, RilParser::CreateInts},
    { RIL_REQUEST_OEM_VSIM_OPERATION, RilParser::CreateVsimOperationExt},

    //SAR
    { RIL_REQUEST_OEM_SAR_SET_STATE, RilParser::CreateInt},
    { RIL_REQUEST_OEM_SET_CA_BANDWIDTH_FILTER, RilParser::CreateInt},

    // SELFLOG
    { RIL_REQUEST_OEM_SET_SELFLOG, RilParser::CreateInts},
    // CP Sleep Log
    { RIL_REQUEST_OEM_SET_MODEM_LOG_DUMP, RilParser::CreateRawData},

    // RSSI Scan
    { RIL_REQUEST_OEM_SCAN_RSSI, RilParser::CreateInts},

    // AT command through RIL
    { RIL_REQUEST_OEM_FORWARDING_AT_COMMAND, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GET_PLMN_NAME_FROM_SE13TABLE, RilParser::CreateRawData},

    { RIL_REQUEST_OEM_TS25TABLE_DUMP, RilParser::CreateRawData},

    { RIL_REQUEST_OEM_GET_RADIO_NODE, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_SET_RADIO_NODE, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_GET_PROVISION_UPDATE_REQUEST, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_SET_PROVISION_UPDATE_DONE_REQUEST, RilParser::CreateRawData},
    { RIL_REQUEST_OEM_RADIO_CONFIG_RESET, RilParser::CreateInt},
    { RIL_REQUEST_OEM_SET_IMS_TEST_MODE, RilParser::CreateInt},

    // Modem status
    { RIL_REQUEST_OEM_MODEM_INFO, RilParser::CreateRawData},

    // RTP
    { RIL_REQUEST_OEM_SET_RTP_PKTLOSS_THRESHOLD, RilParser::CreateRawData},

    {RIL_REQUEST_OEM_SWITCH_MODEM_FUNCTION, RilParser::CreateInts},
    {RIL_REQUEST_OEM_REQ_SET_PDCP_DISCARD_TIMER, RilParser::CreateInt},
    {RIL_REQUEST_OEM_GET_CQI_INFO, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SET_SAR_SETTING, RilParser::CreateInt},
    {RIL_REQUEST_OEM_SET_GMO_SWITCH, RilParser::CreateInt},
    {RIL_REQUEST_OEM_SET_TCS_FCI, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_GET_TCS_FCI, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_ICC_DEPERSONALIZATION, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SET_ELEVATOR_SENSOR, RilParser::CreateInt},
    {RIL_REQUEST_OEM_SET_SELFLOG_PROFILE, RilParser::CreateRawData},
    {RIL_REQUEST_OEM_SET_FORBID_LTE_CELL, RilParser::CreateRawData},

#ifdef SUPPORT_CDMA
    // Call & SS
    {RIL_REQUEST_CDMA_BURST_DTMF, RilParser::CreateStrings},
    {RIL_REQUEST_CDMA_FLASH, RilParser::CreateString},
    {RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE, RilParser::CreateInt},

    // Network
    {RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE, RilParser::CreateInt},
    {RIL_REQUEST_SET_CDMA_HYBRID_MODE, RilParser::CreateInt},

    // SMS
    {RIL_REQUEST_CDMA_SEND_SMS, RilParser::CreateCdmaSms},
    {RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE, RilParser::CreateCdmaSmsAck},
    {RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG, RilParser::CreateCdmaSetBroadcastSmsConfig},
    {RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION, RilParser::CreateInt},
    {RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM, RilParser::CreateRuimSmsData},
    {RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM, RilParser::CreateInt},

    // Misc
    {RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE, RilParser::CreateInt},
#endif // SUPPORT_CDMA
    { RIL_REQUEST_EMULATE_IND, RilParser::CreateInts},

    // RadioConfig
    { RIL_REQUEST_SET_PREFERRED_DATA_MODEM, RilParser::CreateInt},
    { RIL_REQUEST_SET_MODEMS_CONFIG, RilParser::CreateRawData},

};

RilParser::RilParser()
{
}

RilParser::~RilParser()
{
}


parseFunc RilParser::FindFunc(const int id)
{
    int nCount = (int)sizeof(szParseFuncTable)/sizeof(ParseFuncMap_t);
    for (int i = 0; i < nCount; i++) {
        if (id == szParseFuncTable[i].nid) {
            return szParseFuncTable[i].pFunc;
        }
    } // end for i ~

    return NULL;
}

RequestData *RilParser::GetRequestData(int id, Token tok, void *data/* = NULL*/, unsigned int datalen/* = 0*/)
{
    RequestData *result = NULL;
    parseFunc pFunc = FindFunc(id);
    if (pFunc == NULL) {
        int decodedId = DECODE_REQUEST(id);
        int halVer = DECODE_HAL(id);
        if (halVer > 0) {
            pFunc = FindFunc(decodedId);
        }
    }

    if (pFunc == NULL && datalen == 0) {
        // use default handler if a parameter is not necessary.
        pFunc = RilParser::CreateRequestData;
    }

    if (pFunc != NULL) {
        result = pFunc(id, tok, (char *)data, datalen);
    }
    return result;
}


RequestData *RilParser::CreateRequestData(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new RequestData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
    }
    return pData;
}

RequestData *RilParser::CreateInt(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new IntRequestData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateInts(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new IntsRequestData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateString(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new StringRequestData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateStrings(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new StringsRequestData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateRawData(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new RawRequestData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateHookRaw(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new OemHookRawRequestData(id, tok);

    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateOemRequest(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new OemRequestData(id, tok);

    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateCallDial(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new CallDialReqData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateCallEmergencyDial(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new CallEmergencyDialReqData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateCallForward(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new CallForwardReqData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateCallNumber(int id, Token tok, char *data, unsigned int datalen)
{
    CallDialReqData *pData = new CallDialReqData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    if (pData->encodeCallNumber(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateGsmSms(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new GsmSmsMessage(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSmsAck(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SmsAcknowledge(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSmsAckPdu(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SmsAcknowledgePdu(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSetSmsConfig(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new BroadcastSmsConfigsRequestData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;

}

RequestData *RilParser::CreateSendDtmf(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new DtmfInfo(id, tok, data);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    return pData;
}

RequestData *RilParser::CreateStartDtmf(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new DtmfInfo(id, tok, data[0]);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    return pData;
}

RequestData *RilParser::CreateStopDtmf(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new DtmfInfo(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

   return pData;
}

RequestData *RilParser::CreateSimIoData(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SimIoData(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSimUiccSubscription(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new UiccSubscription(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSimAuthentication(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SimAuthentication(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSimSmsData(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SimSmsMessage(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSetupDataCall(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SetupDataCallRequestData(id, tok);

    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateDeactivateDataCall(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new DeactivateDataCallRequestData(id,tok);

    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSetInitialAttachApn(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SetInitialAttachApnRequestData(id, tok);

    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSetDataProfile(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SetDataProfileRequestData(id, tok);

    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSimAPDU(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SimAPDU(id, tok);

    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateOemSimAuthRequest(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new OemSimAuthRequest(id, tok);
    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSimOpenChannel(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SimOpenChannel(id, tok);

    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }

    return pData;
}

RequestData *RilParser::CreateUpdatePbEntry(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SimUpdatePbEntry(id, tok);

    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateNetRCData(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new NetRCData(id, tok);

    if(NULL == pData)
    {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateNvReadItem(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new NvReadItemRequestData(id, tok);
    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateNvWriteItem(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new NvWriteItemRequestData(id, tok);
    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateVsimOperation(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new VsimOperationData(id, tok);

    if(NULL == pData)
    {
        RilLog("No enough memory");
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateVsimOperationExt(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new VsimOperationDataExt(id, tok);

    if(NULL == pData)
    {
        RilLog("No enough memory");
        return NULL;
    }
    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateApnSettings(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new ApnSettingsData(id, tok);

    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}


RequestData *RilParser::CreateCarrierRestrictions(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new CarrierRestrictionsData(id, tok);

    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateCarrierInfoForImsiEncryption(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new CarrierInfoForImsiEncryptionData(id, tok);

    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}


RequestData *RilParser::CreateRequestKeepalive(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new KeepaliveRequestData(id, tok);

    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}


RequestData *RilParser::CreateNetworkScanRequest(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new NetworkScanReqData(id, tok);

    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSetActivateVsim(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SetActivateVsimReqData(id, tok);

    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateSignalStrengthReportingCriteria(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new SignalStrengthReportingCriteria(id, tok);

    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateLinkCapacityReportingCriteria(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new LinkCapacityReportingCriteria(id, tok);

    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateEmbmsSessionData(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new EmbmsSessionData(id, tok);

    if (pData == NULL) {
        RilLogE("No enough memory");
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

#ifdef SUPPORT_CDMA
#include "cdmasmsdata.h"
RequestData *RilParser::CreateCdmaSms(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new CdmaSmsRequestData(id, tok);
    if (NULL == pData) {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateCdmaSmsAck(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new CdmaSmsAckRequestData(id, tok);
    if (NULL == pData) {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateCdmaSetBroadcastSmsConfig(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new CdmaBroadcastSmsConfigsRequestData(id, tok);
    if (NULL == pData) {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}

RequestData *RilParser::CreateRuimSmsData(int id, Token tok, char *data, unsigned int datalen)
{
    RequestData *pData = new CdmaSmsWriteToRuimRequestData(id, tok);
    if (NULL == pData) {
        RilLogE("[%s] No enough memory", TAG);
        return NULL;
    }

    if (pData->encode(data, datalen) < 0) {
        delete pData;
        return NULL;
    }
    return pData;
}
#endif // SUPPORT_CDMA
