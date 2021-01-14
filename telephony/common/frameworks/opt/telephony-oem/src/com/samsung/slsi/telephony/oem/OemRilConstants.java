/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
package com.samsung.slsi.telephony.oem;

import android.util.Log;

public class OemRilConstants {

    public static final int RILC_SOCKET_ID_1            = 0;
    public static final int RILC_SOCKET_ID_2            = 1;

    public static final int SOLICITED                   = 0;
    public static final int UNSOLICITED                 = 1;

    public static final int RILC_TRANSACTION_MAX        = 255;
    public static final int RILC_TRANSACTION_NONE       = 0xFFFFFFFF;
    public static final int RILC_REQUEST_HEADER_SIZE    = 13;
    public static final int RILC_RESPONSE_HEADER_SIZE   = 17;

    /** Error Status */
    public static final int RILC_STATUS_SUCCESS         = 0;
    public static final int RILC_STATUS_FAIL            = 1;
    public static final int RILC_STATUS_NOT_CONNECTED   = 2;
    public static final int RILC_STATUS_INVALID_PARAM   = 3;
    public static final int RILC_STATUS_IO_ERROR        = 4;
    public static final int RILC_STATUS_NO_RESOURCES    = 5;

    /** Request ID */
    public static final int RILC_REQ_SYSTEM_MODEM_DUMP          = 1;
    public static final int RILC_REQ_MISC_SET_ENG_MODE          = 2;
    public static final int RILC_REQ_MISC_SCREEN_LINE           = 3;
    public static final int RILC_REQ_MISC_DEBUG_TRACE           = 4;
    public static final int RILC_REQ_MISC_SET_CARRIER_CONFIG    = 5;
    public static final int RILC_REQ_MISC_SET_ENG_STRING_INPUT  = 6;
    public static final int RILC_REQ_MISC_GET_MSL_CODE          = 8;
    public static final int RILC_REQ_MISC_SET_PIN_CONTROL       = 9;
    public static final int RILC_REQ_AUDIO_SET_MUTE             = 10;
    public static final int RILC_REQ_AUDIO_GET_MUTE             = 11;
    public static final int RILC_REQ_AUDIO_SET_VOLUME           = 12;
    public static final int RILC_REQ_AUDIO_GET_VOLUME           = 13;
    public static final int RILC_REQ_AUDIO_SET_PATH             = 14;
    public static final int RILC_REQ_AUDIO_GET_PATH             = 15;
    public static final int RILC_REQ_AUDIO_SET_MIC              = 16;
    public static final int RILC_REQ_AUDIO_GET_MIC              = 17;
    public static final int RILC_REQ_AUDIO_SET_AUDIO_CLOCK      = 18;
    public static final int RILC_REQ_AUDIO_SET_AUDIO_LOOPBACK   = 19;
    public static final int RILC_REQ_MISC_SET_PREFERRED_CALL_CAPA = 20;
    public static final int RILC_REQ_MISC_GET_PREFERRED_CALL_CAPA = 21;
    public static final int RILC_REQ_MODEM_RESET                = 22;
    public static final int RILC_REQ_GET_MANUAL_RAT_MODE        = 23;
    public static final int RILC_REQ_SET_MANUAL_RAT_MODE        = 24;
    public static final int RILC_REQ_GET_FREQUENCY_LOCK         = 25;
    public static final int RILC_REQ_SET_FREQUENCY_LOCK         = 26;
    public static final int RILC_REQ_SET_ENDC_MODE              = 27;
    public static final int RILC_REQ_GET_ENDC_MODE              = 28;
    public static final int RILC_REQ_SET_IMS_TEST_MODE          = 38;
    public static final int RILC_REQ_SET_UICC_SUBSCRIPTION      = 40;

    public static final int RILC_REQ_MISC_GET_MANUAL_BAND_MODE  = 90;
    public static final int RILC_REQ_MISC_SET_MANUAL_BAND_MODE  = 91;
    public static final int RILC_REQ_MISC_GET_RF_DESENSE_MODE   = 92;
    public static final int RILC_REQ_MISC_SET_RF_DESENSE_MODE   = 93;
    public static final int RILC_REQ_SCAN_RSSI                  = 96;
    public static final int RILC_REQ_FORWARDING_AT_COMMAND      = 97;
    public static final int RILC_REQ_SET_MODEM_LOG_DUMP         = 98;
    public static final int RILC_REQ_SET_ELEVATOR_SENSOR        = 99;

    public static final int RILC_REQ_PSENSOR_SET_STATUS         = 401;
    public static final int RILC_REQ_VSIM_NOTIFICATION          = 451;
    public static final int RILC_REQ_VSIM_OPERATION             = 452;

    public static final int RILC_REQ_SAR_SET_SAR_STATE          = 501;
    public static final int RILC_REQ_SAR_GET_SAR_STATE          = 502;
    public static final int RILC_REQ_SET_SELFLOG                = 600;
    public static final int RILC_REQ_GET_SELFLOG_STATUS         = 601;
    public static final int RILC_REQ_MODEM_INFO                 = 602;
    public static final int RILC_REQ_SET_RTP_PKTLOSS_THRESHOLD  = 603;
    public static final int RILC_REQ_SWITCH_MODEM_FUNCTION      = 604;
    public static final int RILC_REQ_SET_PDCP_DISCARD_TIMER     = 605;
    public static final int RILC_REQ_GET_CQI_INFO               = 606;
    public static final int RILC_REQ_SET_SAR_SETTING            = 607;
    public static final int RILC_REQ_SET_GMO_SWITCH             = 608;
    public static final int RILC_REQ_SET_TCS_FCI                = 609;
    public static final int RILC_REQ_GET_TCS_FCI                = 610;
    public static final int RILC_REQ_SET_SELFLOG_PROFILE        = 611;
    public static final int RILC_REQ_SET_FORBID_LTE_CELL        = 612;
    public static final int RILC_REQ_GET_FREQUENCY_INFO         = 613;

    public static final int RILC_REQ_GET_RADIO_NODE                    = 800;
    public static final int RILC_REQ_SET_RADIO_NODE                    = 801;
    public static final int RILC_REQ_GET_PROVISION_UPDATE_REQUEST      = 802;
    public static final int RILC_REQ_SET_PROVISION_UPDATE_DONE_REQUEST = 803;
    public static final int RILC_REQ_RADIO_CONFIG_RESET                = 804;
    public static final int RILC_REQ_VERIFY_MSL                        = 805;
    public static final int RILC_REQ_GET_PLMN_NAME_FROM_SE13TABLE      = 806;
    public static final int RILC_REQ_TS25TABLE_DUMP                    = 807;
    public static final int RILC_REQ_SET_CA_BANDWIDTH_FILTER           = 808;
    public static final int RILC_REQ_ICC_DEPERSONALIZATION             = 809;
    public static final int RILC_REQ_CANCEL_GET_AVAILABLE_NETWORK      = 810;

    /** Unsolicited Response ID */
    public static final int RILC_UNSOL_SOCKET_ERROR             = 1000;
    public static final int RILC_UNSOL_DISPLAY_ENG_MODE         = 2002;
    public static final int RILC_UNSOL_PIN_CONTROL              = 2004;
    public static final int RILC_UNSOL_AM                       = 2005;
    public static final int RILC_UNSOL_SCAN_RSSI_RESULT         = 2006;
    public static final int RILC_UNSOL_FORWARDING_AT_COMMAND    = 2007;
    public static final int RILC_UNSOL_IMS_SRVCC_HO             = 3003;
    public static final int RILC_UNSOL_AIMS_SIP_MSG_INFO        = 3050;
    public static final int RILC_UNSOL_SELFLOG_STATUS           = 5100;
    public static final int RILC_UNSOL_MODEM_INFO               = 5101;
    public static final int RILC_UNSOL_RTP_PKTLOSS_THRESHOLD    = 5102;
    public static final int RILC_UNSOL_FREQUENCY_INFO           = 5103;
    public static final int RILC_UNSOL_AMBR_REPORT              = 5104;
    public static final int RILC_UNSOL_B2_B1_CONFIG_INFO        = 5105;
    public static final int RILC_UNSOL_CA_BANDWIDTH_FILTER      = 6002;
    public static final int RILC_UNSOL_VSIM_OPERATION           = 6501;
    public static final int RILC_UNSOL_SAR_RF_CONNECTION        = 7001;

    public static String requestToString(int request) {
        switch (request) {
        case RILC_REQ_SYSTEM_MODEM_DUMP:
            return "RILC_REQ_SYSTEM_MODEM_DUMP";
        case RILC_REQ_MISC_SET_ENG_MODE:
            return "RILC_REQ_MISC_SET_ENG_MODE";
        case RILC_REQ_MISC_SCREEN_LINE:
            return "RILC_REQ_MISC_SCREEN_LINE";
        case RILC_REQ_MISC_DEBUG_TRACE:
            return "RILC_REQ_MISC_DEBUG_TRACE";
        case RILC_REQ_MISC_SET_CARRIER_CONFIG:
            return "RILC_REQ_MISC_SET_CARRIER_CONFIG";
        case RILC_REQ_MISC_SET_ENG_STRING_INPUT:
            return "RILC_REQ_MISC_SET_ENG_STRING_INPUT";
        case RILC_REQ_MISC_GET_MSL_CODE:
            return "RILC_REQ_MISC_GET_MSL_CODE";
        case RILC_REQ_MISC_SET_PIN_CONTROL:
            return "RILC_REQ_MISC_SET_PIN_CONTROL";
        case RILC_REQ_AUDIO_SET_MUTE:
            return "RILC_REQ_AUDIO_SET_MUTE";
        case RILC_REQ_AUDIO_GET_MUTE:
            return "RILC_REQ_AUDIO_GET_MUTE";
        case RILC_REQ_AUDIO_SET_VOLUME:
            return "RILC_REQ_AUDIO_SET_VOLUME";
        case RILC_REQ_AUDIO_GET_VOLUME:
            return "RILC_REQ_AUDIO_GET_VOLUME";
        case RILC_REQ_AUDIO_SET_PATH:
            return "RILC_REQ_AUDIO_SET_PATH";
        case RILC_REQ_AUDIO_GET_PATH:
            return "RILC_REQ_AUDIO_GET_PATH";
        case RILC_REQ_AUDIO_SET_MIC:
            return "RILC_REQ_AUDIO_SET_MIC";
        case RILC_REQ_AUDIO_GET_MIC:
            return "RILC_REQ_AUDIO_GET_MIC";
        case RILC_REQ_AUDIO_SET_AUDIO_CLOCK:
            return "RILC_REQ_AUDIO_SET_AUDIO_CLOCK";
        case RILC_REQ_AUDIO_SET_AUDIO_LOOPBACK:
            return "RILC_REQ_AUDIO_SET_AUDIO_LOOPBACK";
        case RILC_REQ_MISC_SET_PREFERRED_CALL_CAPA:
            return "RILC_REQ_MISC_SET_PREFERRED_CALL_CAPA";
        case RILC_REQ_MISC_GET_PREFERRED_CALL_CAPA:
            return "RILC_REQ_MISC_GET_PREFERRED_CALL_CAPA";
        case RILC_REQ_MISC_GET_MANUAL_BAND_MODE:
            return "RILC_REQ_MISC_GET_MANUAL_BAND_MODE";
        case RILC_REQ_MISC_SET_MANUAL_BAND_MODE:
            return "RILC_REQ_MISC_SET_MANUAL_BAND_MODE";
        case RILC_REQ_MISC_GET_RF_DESENSE_MODE:
            return "RILC_REQ_MISC_GET_RF_DESENSE_MODE";
        case RILC_REQ_MISC_SET_RF_DESENSE_MODE:
            return "RILC_REQ_MISC_SET_RF_DESENSE_MODE";
        case RILC_REQ_SCAN_RSSI:
            return "RILC_REQ_SCAN_RSSI";
        case RILC_REQ_FORWARDING_AT_COMMAND:
            return "RILC_REQ_FORWARDING_AT_COMMAND";
        case RILC_REQ_PSENSOR_SET_STATUS:
            return "RILC_REQ_PSENSOR_SET_STATUS";
        case RILC_REQ_VSIM_NOTIFICATION:
            return "RILC_REQ_VSIM_NOTIFICATION";
        case RILC_REQ_VSIM_OPERATION:
            return "RILC_REQ_VSIM_OPERATION";
        case RILC_REQ_SAR_SET_SAR_STATE:
            return "RILC_REQ_SAR_SET_SAR_STATE";
        case RILC_REQ_SAR_GET_SAR_STATE:
            return "RILC_REQ_SAR_GET_SAR_STATE";
        case RILC_REQ_GET_RADIO_NODE:
            return "RILC_REQ_GET_RADIO_NODE";
        case RILC_REQ_SET_RADIO_NODE:
            return "RILC_REQ_SET_RADIO_NODE";
        case RILC_REQ_GET_PROVISION_UPDATE_REQUEST:
            return "RILC_REQ_GET_PROVISION_UPDATE_REQUEST";
        case RILC_REQ_SET_PROVISION_UPDATE_DONE_REQUEST:
            return "RILC_REQ_SET_PROVISION_UPDATE_DONE_REQUEST";
        case RILC_REQ_RADIO_CONFIG_RESET:
            return "RILC_REQ_RADIO_CONFIG_RESET";
        case RILC_REQ_VERIFY_MSL:
            return "RILC_REQ_VERIFY_MSL";
        case RILC_REQ_GET_PLMN_NAME_FROM_SE13TABLE:
            return "RILC_REQ_GET_PLMN_NAME_FROM_SE13TABLE";
        case RILC_REQ_TS25TABLE_DUMP:
            return "RILC_REQ_TS25_TABLE_DUMP";
        case RILC_UNSOL_DISPLAY_ENG_MODE:
            return "RILC_UNSOL_DISPLAY_ENG_MODE";
        case RILC_UNSOL_AM:
            return "RILC_UNSOL_AM";
        case RILC_UNSOL_SCAN_RSSI_RESULT:
            return "RILC_UNSOL_SCAN_RSSI_RESULT";
        case RILC_UNSOL_FORWARDING_AT_COMMAND:
            return "RILC_UNSOL_FORWARDING_AT_COMMAND";
        case RILC_UNSOL_VSIM_OPERATION:
            return "RILC_UNSOL_VSIM_OPERATION";
        case RILC_UNSOL_SAR_RF_CONNECTION:
            return "RILC_UNSOL_SAR_RF_CONNECTION";
        case RILC_REQ_MODEM_INFO:
            return "RILC_REQ_MODEM_INFO";
        case RILC_REQ_MODEM_RESET:
            return "RILC_REQ_MODEM_RESET";
        case RILC_REQ_GET_MANUAL_RAT_MODE:
            return "RILC_REQ_GET_MANUAL_RAT_MODE";
        case RILC_REQ_SET_MANUAL_RAT_MODE:
            return "RILC_REQ_SET_MANUAL_RAT_MODE";
        case RILC_REQ_GET_FREQUENCY_LOCK:
            return "RILC_REQ_GET_FREQUENCY_LOCK";
        case RILC_REQ_SET_FREQUENCY_LOCK:
            return "RILC_REQ_SET_FREQUENCY_LOCK";
        case RILC_REQ_SET_SELFLOG:
            return "RILC_REQ_SET_SELFLOG";
        case RILC_REQ_GET_SELFLOG_STATUS:
            return "RILC_REQ_GET_SELFLOG_STATUS";
        case RILC_UNSOL_MODEM_INFO:
            return "RILC_UNSOL_MODEM_INFO";
        case RILC_REQ_SET_RTP_PKTLOSS_THRESHOLD:
            return "RILC_REQ_SET_RTP_PKTLOSS_THRESHOLD";
        case RILC_REQ_SWITCH_MODEM_FUNCTION:
            return "RILC_REQ_SWITCH_MODEM_FUNCTION";
        case RILC_REQ_SET_PDCP_DISCARD_TIMER:
            return "RILC_REQ_SET_PDCP_DISCARD_TIMER";
        case RILC_REQ_SET_ENDC_MODE:
            return "RILC_REQ_SET_ENDC_MODE";
        case RILC_REQ_GET_ENDC_MODE:
            return "RILC_REQ_GET_ENDC_MODE";
        case RILC_UNSOL_SELFLOG_STATUS:
            return "RILC_UNSOL_SELFLOG_STATUS";
        case RILC_REQ_SET_CA_BANDWIDTH_FILTER:
            return "RILC_REQ_SET_CA_BANDWIDTH_FILTER";
        case RILC_REQ_ICC_DEPERSONALIZATION:
            return "RILC_REQ_ICC_DEPERSONALIZATION";
        case RILC_UNSOL_CA_BANDWIDTH_FILTER:
            return "RILC_UNSOL_CA_BANDWIDTH_FILTER";
        case RILC_REQ_CANCEL_GET_AVAILABLE_NETWORK:
            return "RILC_REQ_CANCEL_GET_AVAILABLE_NETWORK";
        case RILC_REQ_SET_MODEM_LOG_DUMP:
            return"RILC_REQ_SET_MODEM_LOG_DUMP";
        case RILC_REQ_SET_UICC_SUBSCRIPTION:
            return "RILC_REQ_SET_UICC_SUBSCRIPTION";
        case RILC_REQ_SET_SELFLOG_PROFILE:
            return "RILC_REQ_SET_SELFLOG_PROFILE";
        case RILC_UNSOL_IMS_SRVCC_HO:
            return "RILC_UNSOL_IMS_SRVCC_HO";
        case RILC_UNSOL_AIMS_SIP_MSG_INFO:
            return "RILC_UNSOL_AIMS_SIP_MSG_INFO";
        case RILC_UNSOL_AMBR_REPORT:
            return "RILC_UNSOL_AMBR_REPORT";
        case RILC_REQ_SET_FORBID_LTE_CELL:
            return "RILC_REQ_SET_FORBID_LTE_CELL";
        case RILC_REQ_GET_FREQUENCY_INFO:
            return "RILC_REQ_GET_FREQUENCY_INFO";
        case RILC_UNSOL_B2_B1_CONFIG_INFO:
            return "RILC_UNSOL_B2_B1_CONFIG_INFO";
        case RILC_REQ_SET_ELEVATOR_SENSOR:
            return "RILC_REQ_SET_ELEVATOR_SENSOR";
        case RILC_REQ_GET_CQI_INFO:
            return "RILC_REQ_GET_CQI_INFO";
        case RILC_UNSOL_FREQUENCY_INFO:
            return "RILC_UNSOL_FREQUENCY_INFO";
        case RILC_REQ_SET_SAR_SETTING:
            return "RILC_REQ_SET_SAR_SETTING";
        case RILC_REQ_SET_IMS_TEST_MODE:
            return "RILC_REQ_SET_IMS_TEST_MODE";
        case RILC_REQ_SET_GMO_SWITCH:
            return "RILC_REQ_SET_GMO_SWITCH";
        case RILC_REQ_SET_TCS_FCI:
            return "RILC_REQ_SET_TCS_FCI";
        case RILC_REQ_GET_TCS_FCI:
            return "RILC_REQ_GET_TCS_FCI";
        }
        return ("unsupported request. id="+request);
    }

    static Exception fromRilErrno(int errno) {
        switch (errno) {
        case RILC_STATUS_SUCCESS:  return null;
        case RILC_STATUS_FAIL:
            return new Exception("RILC_STATUS_FAIL");
        case RILC_STATUS_NOT_CONNECTED:
            return new Exception("RILC_STATUS_NOT_CONNECTED");
        case RILC_STATUS_INVALID_PARAM:
            return new Exception("RILC_STATUS_INVALID_PARAM");
        case RILC_STATUS_IO_ERROR:
            return new Exception("RILC_STATUS_IO_ERROR");
        case RILC_STATUS_NO_RESOURCES:
            return new Exception("RILC_STATUS_NO_RESOURCES");
        default:
            Log.e("OemRil", "Unrecognized errno " + errno);
            return new Exception("INVALID_RESPONSE");
        }
    }
}
