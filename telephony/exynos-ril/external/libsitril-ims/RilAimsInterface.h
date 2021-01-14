/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_RILC_IMS_CLIENT                     3

#define RILC_REQ_IMS_SET_CONFIGURATION          30
#define RILC_REQ_IMS_GET_CONFIGURATION          31
#define RILC_REQ_IMS_SIM_AUTH                   32
#define RILC_REQ_IMS_SET_EMERGENCY_CALL_STATUS  33
#define RILC_REQ_IMS_SET_SRVCC_CALL_LIST        34
#define RILC_REQ_IMS_GET_GBA_AUTH               35
#define RILC_REQ_IMS_SIM_IO                     36
#define RILC_REQ_NET_GET_IMS_SUPPORT_SERVICE    37

//AIMS support start ---------------------
#define RILC_REQ_AIMS_DIAL                       100
#define RILC_REQ_AIMS_ANSWER                     101
#define RILC_REQ_AIMS_HANGUP                     102
#define RILC_REQ_AIMS_DEREGISTRATION             103
#define RILC_REQ_AIMS_HIDDEN_MENU                104
#define RILC_REQ_AIMS_ADD_PDN_INFO               105
#define RILC_REQ_AIMS_CALL_MANAGE                106
#define RILC_REQ_AIMS_SEND_DTMF                  107
#define RILC_REQ_AIMS_SET_FRAME_TIME             108
#define RILC_REQ_AIMS_GET_FRAME_TIME             109
#define RILC_REQ_AIMS_CALL_MODIFY                110
#define RILC_REQ_AIMS_RESPONSE_CALL_MODIFY       111
#define RILC_REQ_AIMS_TIME_INFO                  112
#define RILC_REQ_AIMS_CONF_CALL_ADD_REMOVE_USER  113
#define RILC_REQ_AIMS_ENHANCED_CONF_CALL         114
#define RILC_REQ_AIMS_GET_CALL_FORWARD_STATUS    115
#define RILC_REQ_AIMS_SET_CALL_FORWARD_STATUS    116
#define RILC_REQ_AIMS_GET_CALL_WAITING           117
#define RILC_REQ_AIMS_SET_CALL_WAITING           118
#define RILC_REQ_AIMS_GET_CALL_BARRING           119
#define RILC_REQ_AIMS_SET_CALL_BARRING           120
#define RILC_REQ_AIMS_SEND_SMS                   121
#define RILC_REQ_AIMS_SEND_EXPECT_MORE           122
#define RILC_REQ_AIMS_SEND_SMS_ACK               123
#define RILC_REQ_AIMS_SEND_ACK_INCOMING_SMS      124
#define RILC_REQ_AIMS_CHG_BARRING_PWD            125
#define RILC_REQ_AIMS_SEND_USSD_INFO             126
#define RILC_REQ_AIMS_GET_PRESENTATION_SETTINGS  127
#define RILC_REQ_AIMS_SET_PRESENTATION_SETTINGS  128
#define RILC_REQ_AIMS_SET_SELF_CAPABILITY        129
#define RILC_REQ_AIMS_HO_TO_WIFI_READY           130
#define RILC_REQ_AIMS_HO_TO_WIFI_CANCEL_IND      131
#define RILC_REQ_AIMS_HO_PAYLOAD_IND             132
#define RILC_REQ_AIMS_HO_TO_3GPP                 133
#define RILC_REQ_AIMS_SEND_ACK_INCOMING_CDMA_SMS 134
#define RILC_REQ_AIMS_MEDIA_STATE_IND            135
#define RILC_REQ_AIMS_DEL_PDN_INFO               136
#define RILC_REQ_AIMS_STACK_START_REQ            137
#define RILC_REQ_AIMS_STACK_STOP_REQ             138
#define RILC_REQ_AIMS_XCAPM_START_REQ            139
#define RILC_REQ_AIMS_XCAPM_STOP_REQ             140
#define RILC_REQ_AIMS_RTT_SEND_TEXT              141
#define RILC_REQ_AIMS_EXIT_EMERGENCY_CB_MODE     142
#define RILC_REQ_AIMS_SET_GEO_LOCATION_INFO      143
#define RILC_REQ_AIMS_CDMA_SEND_SMS              144
//AIMS support end ---------------------

// WFC
#define RILC_REQ_WFC_MEDIA_CHANNEL_CONFIG        151
#define RILC_REQ_WFC_DTMF_START                  152
#define RILC_REQ_WFC_SET_VOWIFI_HO_THRESHOLD     153

// RCS command
#define RILC_REQ_AIMS_RCS_MULTI_FRAME            550
#define RILC_REQ_AIMS_RCS_CHAT                   551
#define RILC_REQ_AIMS_RCS_GROUP_CHAT             552
#define RILC_REQ_AIMS_RCS_OFFLINE_MODE           553
#define RILC_REQ_AIMS_RCS_FILE_TRANSFER          554
#define RILC_REQ_AIMS_RCS_COMMON_MESSAGE         555
#define RILC_REQ_AIMS_RCS_CONTENT_SHARE          556
#define RILC_REQ_AIMS_RCS_PRESENCE               557
#define RILC_REQ_AIMS_XCAP_MANAGE                558
#define RILC_REQ_AIMS_RCS_CONFIG_MANAGE          559
#define RILC_REQ_AIMS_RCS_TLS_MANAGE             560


#define RILC_UNSOL_SOCKET_ERROR                 1000    /* RIL socket closed */
#define RILC_UNSOL_AUDIO_RING                   2001    /* Ring indication */
#define RILC_UNSOL_DISPLAY_ENG_MODE             2002    /* Display Engineer Mode */
#define RILC_UNSOL_AUDIO_RINGBACK               2003    /* Ringback indication */

#define RILC_UNSOL_IMS_CONFIGURATION            3000    /* Send IMS configuration information */
#define RILC_UNSOL_IMS_DEDICATED_PDN_INFO       3001    /* Send dedicated pdn information */
#define RILC_UNSOL_IMS_EMERGENCY_ACT_INFO       3002
#define RILC_UNSOL_IMS_SRVCC_HO                 3003
#define RILC_UNSOL_IMS_EMERGENCY_CALL_LIST      3004
#define RILC_UNSOL_IMS_SUPPORT_SERVICE          3005
#define RILC_UNSOL_IMS_OPEN_CARRIER_INFO        3006

//AIMS support start ---------------------
#define RILC_UNSOL_AIMS_CALL_RING                   3010
#define RILC_UNSOL_AIMS_CALL_STATUS                 3011
#define RILC_UNSOL_AIMS_REGISTRATION                3012
#define RILC_UNSOL_AIMS_CALL_MODIFY                 3013
#define RILC_UNSOL_AIMS_FRAME_TIME                  3014
#define RILC_UNSOL_AIMS_SUPP_SVC_NOTIFICATION       3015
#define RILC_UNSOL_AIMS_NEW_SMS                     3016
#define RILC_UNSOL_AIMS_NEW_SMS_STATUS_REPORT       3017
#define RILC_UNSOL_AIMS_ON_USSD                     3018
#define RILC_UNSOL_AIMS_CONFERENCE_CALL_EVENT       3019
#define RILC_UNSOL_AIMS_HO_PAYLOAD                  3020
#define RILC_UNSOL_AIMS_VOWIFI_HO_CALL_INFO         3021
#define RILC_UNSOL_AIMS_NEW_CDMA_SMS                3022
#define RILC_UNSOL_AIMS_CALL_MANAGE                 3023
#define RILC_UNSOL_AIMS_CONF_CALL_ADD_REMOVE_USER   3024
#define RILC_UNSOL_AIMS_ENHANCED_CONF_CALL          3025
#define RILC_UNSOL_AIMS_CALL_MODIFY_RSP             3026
#define RILC_UNSOL_AIMS_DTMF_EVENT                  3031
#define RILC_UNSOL_AIMS_RTT_NEW_TEXT                3032
#define RILC_UNSOL_AIMS_RTT_FAIL_SENDING_TEXT       3033
#define RILC_UNSOL_AIMS_RCS_MULTI_FRAME             3034
#define RILC_UNSOL_AIMS_RCS_CHAT                    3035
#define RILC_UNSOL_AIMS_RCS_GROUP_CHAT              3036
#define RILC_UNSOL_AIMS_RCS_OFFLINE_MODE            3037
#define RILC_UNSOL_AIMS_RCS_FILE_TRANSFER           3038
#define RILC_UNSOL_AIMS_RCS_COMMON_MESSAGE          3039
#define RILC_UNSOL_AIMS_RCS_CONTENT_SHARE           3040
#define RILC_UNSOL_AIMS_RCS_PRESENCE                3041
#define RILC_UNSOL_AIMS_RCS_XCAP_MANAGE             3042
#define RILC_UNSOL_AIMS_RCS_CONFIG_MANAGE           3043
#define RILC_UNSOL_AIMS_RCS_TLS_MANAGE              3044
#define RILC_UNSOL_AIMS_EXIT_EMERGENCY_CB_MODE      3046
#define RILC_UNSOL_AIMS_AC_BARRING_INFO             3047
#define RILC_UNSOL_AIMS_DIALOG_INFO                 3048
//AIMS support end ----------------------

//WFC
#define RILC_UNSOL_WFC_RTP_RTCP_TIMEOUT        3027
#define RILC_UNSOL_WFC_FIRST_RTP           3028
#define RILC_UNSOL_WFC_RTCP_RX_SR          3029
#define RILC_UNSOL_WFC_RCV_DTMF_NOTI    3030
#define RILC_UNSOL_WFC_SIGNAL_STRENGTH  3045

// RIL-AIMS Errors
typedef enum {
    RIL_AIMS_ERROR_SUCCESS,
    RIL_AIMS_ERROR_GENERIC_FAILURE,
    RIL_AIMS_ERROR_NO_FILE,
    RIL_AIMS_ERROR_NO_LIB_OPEN_FAIL,
    RIL_AIMS_ERROR_NO_LIB_SEND_FAIL,
    RIL_AIMS_ERROR_NO_DEVICE,
    RIL_AIMS_ERROR_INVALID_PARAM,
    RIL_AIMS_ERROR_REGISTERATION_FAIL,
    RIL_AIMS_ERROR_ALREADY_REGISTERD,
    RIL_AIMS_ERROR_MAX
} RilAimsError;

// RIL-AIMS Channel ID
typedef enum {
    RIL_AIMS_CHANNEL_0,     // slot 0
    RIL_AIMS_CHANNEL_1,     // slot 1
} RIL_AIMS_CHANNEL_ID;

/* Callbacks */
typedef void (*RilAimsOnResponse)(unsigned int msgId, int status, void* data, size_t length, unsigned int channel);
typedef int (*RilAimsUnsolResponse)(unsigned int msgId, void* data, size_t length, unsigned int socket_id);

// ----------------------------------------------------
// @deprecated
// ----------------------------------------------------
int Open(int client_id);

// ----------------------------------------------------
// API Name : RilAimsOpen
// Description
//     : Open RIL-AIMS Interaface.
// Params
//    - void
// Return
//    - int error value of RilAimsError
// ----------------------------------------------------
int RilAimsOpen(int client_id);

// ----------------------------------------------------
// @deprecated
// ----------------------------------------------------
int Close(int client_id);

// ----------------------------------------------------
// API Name : RilAimsClose
// Description
//     : Close RIL-AIMS Interaface.
// Params
//    - void
// Return
//    - int error value of RilAimsError
// ----------------------------------------------------
int RilAimsClose(int client_id);

// ----------------------------------------------------
// API Name : RilAimsRegisterCallback
// Description
//     : Register Callback function to receive Unsolicited events
// Params
//    - (in) RilAimsUnsolResponse callback
//      : Callback function
// Return
//    - int error value of RilAimsError
// ----------------------------------------------------
int RilAimsRegisterCallback(int client_id, RilAimsUnsolResponse callback);

// #### Operation Functions ####
// ======================================================
// API Name : Set IMS Configuration
// Description
//     : set configuration of IMS service like channel, codec, media, option, DTMF, audio engine
// Params
//    - (in) data :
//            typedef struct set_ImsConf {
//                unsigned char    CommandConfig;       //0x01: channel, 0x02: codec, 0x04: Option, 0x08: DTMF, 0x10:IMS PDN
                                                        //0x20: Media
//                unsigned char    ChannelMode;
//                unsigned char    ChannelStatus;
//                unsigned char    AudioIPType;
//                unsigned char    AudioIPAddress[16];
//                unsigned char    AudioIPPortNum[4];
//                unsigned char    AudioIPRTCPPortNum[4];
//                unsigned char    AudioRemoteIPType;
//                unsigned char    AudioRemoteIPAddress[16];
//                unsigned char    AudioRemoteIPPortNum[4];
//                unsigned char    AudioRemoteIPRTCPPortNum[4];
//                unsigned char    ChannelNumber;
//                unsigned char    AudioFormat;
//                unsigned char    CodecBitRate[4];
//                unsigned char    CodecPayload;
//                unsigned char    DTMFPayload;
//                unsigned char    CodecSamplingFrequency;
//                unsigned char    CodecName;
//                unsigned char    PacketTime[4];
//                unsigned char    MaxPacketTime[4];
//                unsigned char    FixedRate;
//                unsigned char    AMRCMR;
//                unsigned char    DTXStatus;
//                unsigned char    FECStatus;
//                unsigned char    RTPTimeout[4];
//                unsigned char    RTCPTimeout[4];
//                unsigned char    ChannelCount;
//                unsigned char    RTCPStatus;
//                unsigned char    RTCPInterval[4];
//                unsigned char    RTCPRetryCount[4];
//                unsigned char    RTCPTMMBR[4];
//                unsigned char    DTMFDigit;
//                unsigned char    PDNIPType;
//                unsigned char    PDNIPAddress[20];
//                unsigned char    CodecMode;
//                unsigned char    CodecStatus;
//            } __packed set_ImsConf_t;
//    - (in) data_len : data len
//    - (in) callback : response callback func.
// Return
//    - int error value of RilAimsError
// ----------------------------------------------------
int SetImsConfiguration(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);


// ======================================================
// API Name : Get IMS Configuration
// Description
//     : Get configuration of IMS service like channel, codec, media, option, DTMF, audio engine
// Params
//    - (in) data :
//            typedef struct get_ImsConf {
//            } __packed get_ImsConf_t;
//    - (in) data_len : data len = 0
//    - (in) callback : response callback func.
// Return
//    - int error value of RilAimsError
// ----------------------------------------------------
int GetImsConfiguration(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);

// ======================================================
// API Name : SIM Authentication
// Description
//     : SIM Authentication using nonce
// Params
//    - (in) data : Nonce
//    - (in) data_len : data len
//    - (in) callback : response callback func.
// Return
//    - int error value of RilAimsError
// ----------------------------------------------------
int GetSimAuth(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);

// ======================================================
// API Name : Set IMS Emergency call status
// Description
//     : inform Emergency call's status from AP to CP
//    - (in) data :
//            typedef struct set_ims_emergency_call_status {
//                unsigned char    status;
//                unsigned char    rat;
//            } __packed set_ims_emergency_call_status_t;
//    - (in) data_len : data len
//    - (in) callback : response callback func.
// Return
//    - int error value of RilAimsError
// ----------------------------------------------------
int SetEmergencyCallStatus(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);

// ======================================================
// API Name : Set Srvcc call list
// Description
//     : Set call info at Srvcc
//    - (in) data :
//            typedef struct set_ims_srvcc_call_list {
//                unsigned char    call_list_num[4];
//
//                //repeat by call_list_num
//                unsigned char    state;
//                unsigned char    index[4];
//                unsigned char    type_of_address[4];
//                unsigned char    is_mpty;
//                unsigned char    is_mt;
//                unsigned char    als;
//                unsigned char    call_type;
//                unsigned char    is_voice_privacy;
//                unsigned char    number_len;
//                unsigned char    number[82];
//                unsigned char    number_presentation[4];
//                unsigned char    name_len;
//                unsigned char    name[82];
//                unsigned char    name_presentation[4];
//                unsigned char    name_dcs_type;
//                unsigned char    uus_type[4];
//                unsigned char    uus_dcs[4];
//                unsigned char    uus_data_length;
//                unsigned char    uus_data[128];
//                unsigned char    SRVCCCall;
//            } __packed set_ims_srvcc_call_list_t;
//    - (in) data_len : data len
//    - (in) callback : response callback func.
// Return
//    - int error value of RilAimsError
// ----------------------------------------------------
int SetSrvccCallList(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);

// ======================================================
// API Name : Get GBA Auth
// Description
//     : get GBA security context of USIM or ISIM for IMS auth
//    - (in) data :
//            typedef struct get_gba_auth {
//                unsigned char    auth_type;    // 0x00 USIM auth, 0x01 ISIM auth
//                unsigned char    gba_type;     // 0x00 bootstrapping mode, 0x01 NAF derivation mode
//                unsigned char    reserved;
//
//                unsigned char    data1_length; // if bootstrapping mode: RAND length, if NAF derivation mode: NAP_ID length
//                unsigned char    data1[255];    // if bootstrapping mode: RAND, if NAF deriavtion mode: NAP_ID
//
//                unsigned char    data2_length; // if bootstrapping mode: AUTN length, if NAF derivation mode: IMPI length
//                unsigned char    data2[255];    // if bootstrapping mode: AUTN, if NAF derivation mode: IMPI
//            } __packed get_gba_auth_t;
//    - (in) data_len : data len
//    - (in) callback : response callback func.
// Return
//    - int error value of RilAimsError
// ----------------------------------------------------
int GetGBAAuth(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);

// ======================================================
// API Name : SetPreferredCallCapability
// Description
//     : This message is used to get the setting of preferred call capability.
// Parameters
//  int client_id : RIL-AIMS client id
//  unsigned int msgId : RIL-AIMS message id
//  unsigned char *data : payload
//  int data_len : a length of payload
//  RilAimsOnResponse callback : function pointer which is invoked when a request is completed.
// Return
//    - int error value of RilAimsError
// ----------------------------------------------------
int SetPreferredCallCapability(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);

//AIMS support start ---------------------
/**
 * RequestAimsForChannelId
 * @desc send AIMS request through RIL-AIMS interface
 * @params
 *  int client_id : RIL-AIMS client id
 *  unsigned int msgId : RIL-AIMS message id
 *  unsigned char *data : payload
 *  int data_len : a length of payload
 *  RilAimsOnResponse callback : function pointer which is invoked when a request is completed.
 *  int channel : Channel ID. In DSDS, channel 0 and 1 are available.
 *  @return  an error of RilAimsError
 */
int RequestAimsForChannelId(int client_id, unsigned int msgId, void *data, int data_len, RilAimsOnResponse callback, int channel);

/**
 * wrapper functions
 */
int RequestAimsDial(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsAnswer(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int ReqeustAimsHangup(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsDegregistration(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsHiddenMenu(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsPdnInfo(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsCallManage(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsSendDtmf(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsSetFrameTime(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsGetFrameTime(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsCallModify(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsResponseCallModify(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsTimeInfo(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsConfCallAddRemoveUser(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsEnhancedConfCall(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsGetCallForwardStatus(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsSetCallForwardStatus(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsGetCallWaiting(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsSetCallWaiting(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsGetCallBarring(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsSetCallBarring(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsSendSms(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsSendSmsExpectMore(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsSendSmsAck(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsAckIncomingGsmSms(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsChangeBarringPassword(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsSendUssdInfo(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsGetPresentationSettings(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsSetPresentationSettings(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsSetSelfCapability(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsHoToWifiReady(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsHoToWifiCancel(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsHoPayloadInd(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsHoTo3gpp(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsAckIncomingCdmaSms(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsMediaStateInd(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestWfcSetVowifiHoThreshold(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsRttSendText(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);
int RequestAimsExitEmergencyCbMode(int client_id, unsigned char *data, int data_len, RilAimsOnResponse callback);

//AIMS support end ---------------------

#ifdef __cplusplus
};
#endif
