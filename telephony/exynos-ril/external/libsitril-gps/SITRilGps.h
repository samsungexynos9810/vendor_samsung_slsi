/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef SIT_RIL_GPS_H
#define SIT_RIL_GPS_H

#ifdef __cplusplus
extern "C" {
#endif

#define RILC_REQ_GPS_BASE                               200
#define RILC_REQ_GPS_SET_FREQUENCY_AIDING               (RILC_REQ_GPS_BASE)
#define RILC_REQ_GPS_GET_LPP_SUPL_REQ_ECID_INFO         (RILC_REQ_GPS_BASE + 1)
#define RILC_REQ_GPS_SET_RRLP_SUPL_REQ_ECID_INFO        (RILC_REQ_GPS_BASE + 2)
#define RILC_REQ_GPS_MO_LOCATION_REQUEST                (RILC_REQ_GPS_BASE + 3)
#define RILC_REQ_GPS_GET_LPP_REQ_SERVING_CELL_INFO      (RILC_REQ_GPS_BASE + 4)
#define RILC_REQ_GPS_SET_SUPL_NI_READY                  (RILC_REQ_GPS_BASE + 5)
#define RILC_REQ_GPS_GET_GSM_EXT_INFO_MSG               (RILC_REQ_GPS_BASE + 21)
#define RILC_REQ_GPS_CONTROL_PLANE_ENABLE               (RILC_REQ_GPS_BASE + 22)
#define RILC_REQ_GPS_GNSS_LPP_PROFILE_SET               (RILC_REQ_GPS_BASE + 23)
// Indication from AP, No resp
#define RILC_REQ_GPS_MEASURE_POS_RSP                    (RILC_REQ_GPS_BASE + 6)
#define RILC_REQ_GPS_RELEASE_GPS                        (RILC_REQ_GPS_BASE + 7)
#define RILC_REQ_GPS_MT_LOCATION_REQUEST                (RILC_REQ_GPS_BASE + 8)
#define RILC_REQ_GPS_LPP_PROVIDE_CAPABILITIES           (RILC_REQ_GPS_BASE + 9)
#define RILC_REQ_GPS_LPP_REQUEST_ASSIST_DATA            (RILC_REQ_GPS_BASE + 10)
#define RILC_REQ_GPS_LPP_PROVIDE_LOCATION_INFO          (RILC_REQ_GPS_BASE + 11)
#define RILC_REQ_GPS_LPP_GPS_ERROR_IND                  (RILC_REQ_GPS_BASE + 12)
#define RILC_REQ_GPS_SUPL_LPP_DATA_INFO                 (RILC_REQ_GPS_BASE + 13)
#define RILC_REQ_GPS_SUPL_NI_MESSAGE                    (RILC_REQ_GPS_BASE + 14)
#define RILC_REQ_GPS_RETRIEVE_LOC_INFO                  (RILC_REQ_GPS_BASE + 24)

/* CDMA & HEDGE GANSS */
#define RILC_REQ_GPS_SET_GANSS_MEAS_POS_RSP             (RILC_REQ_GPS_BASE + 15)
#define RILC_REQ_GPS_SET_GPS_LOCK_MODE                  (RILC_REQ_GPS_BASE + 16)
#define RILC_REQ_GPS_GET_REFERENCE_LOCATION             (RILC_REQ_GPS_BASE + 17)
#define RILC_REQ_GPS_SET_PSEUDO_RANGE_MEASUREMENTS      (RILC_REQ_GPS_BASE + 18)
#define RILC_REQ_GPS_GET_CDMA_PRECISE_TIME_AIDING_INFO  (RILC_REQ_GPS_BASE + 19)
#define RILC_REQ_GPS_CDMA_FREQ_AIDING                   (RILC_REQ_GPS_BASE + 25)
// Indication from AP, No resp
#define RILC_REQ_GPS_GANSS_AP_POS_CAP_RSP               (RILC_REQ_GPS_BASE + 20)


#define RILC_UNSOL_GPS_BASE                                 4000
#define RILC_UNSOL_GPS_MEASURE_POS_REQ                      (RILC_UNSOL_GPS_BASE + 1)
#define RILC_UNSOL_GPS_ASSIST_DATA                          (RILC_UNSOL_GPS_BASE + 2)
#define RILC_UNSOL_GPS_RELEASE_GPS                          (RILC_UNSOL_GPS_BASE + 3)
#define RILC_UNSOL_GPS_MT_LOCATION_REQUEST                  (RILC_UNSOL_GPS_BASE + 4)
#define RILC_UNSOL_GPS_RESET_GPS_ASSIST_DATA                (RILC_UNSOL_GPS_BASE + 5)
#define RILC_UNSOL_GPS_LPP_REQUEST_CAPABILITIES             (RILC_UNSOL_GPS_BASE + 6)
#define RILC_UNSOL_GPS_LPP_PROVIDE_ASSIST_DATA              (RILC_UNSOL_GPS_BASE + 7)
#define RILC_UNSOL_GPS_LPP_REQUEST_LOCATION_INFO            (RILC_UNSOL_GPS_BASE + 8)
#define RILC_UNSOL_GPS_LPP_GPS_ERROR_IND                    (RILC_UNSOL_GPS_BASE + 9)
#define RILC_UNSOL_GPS_SUPL_LPP_DATA_INFO                   (RILC_UNSOL_GPS_BASE + 10)
#define RILC_UNSOL_GPS_SUPL_NI_MESSAGE                      (RILC_UNSOL_GPS_BASE + 11)
#define RILC_UNSOL_GPS_SUPL_NI_READY                        (RILC_UNSOL_GPS_BASE + 18)
#define RILC_UNSOL_GPS_START_MDT_LOC                        (RILC_UNSOL_GPS_BASE + 19)
#define RILC_UNSOL_GPS_LPP_UPDATE_UE_LOC_INFO               (RILC_UNSOL_GPS_BASE + 20)
#define RILC_UNSOL_GPS_LOCK_MODE                            (RILC_UNSOL_GPS_BASE + 21)

/* CDMA & HEDGE GANSS */
#define RILC_UNSOL_GPS_3GPP_SEND_GANSS_ASSIT_DATA           (RILC_UNSOL_GPS_BASE + 12)
#define RILC_UNSOL_GPS_GANSS_MEAS_POS_MSG                   (RILC_UNSOL_GPS_BASE + 13)
#define RILC_UNSOL_GPS_CDMA_GPS_POWER_ON                    (RILC_UNSOL_GPS_BASE + 14)
#define RILC_UNSOL_GPS_CDMA_SEND_ACQUSITION_ASSIT_DATA      (RILC_UNSOL_GPS_BASE + 15)
#define RILC_UNSOL_GPS_CDMA_SESSION_CANCELLATION            (RILC_UNSOL_GPS_BASE + 16)
#define RILC_UNSOL_GPS_GANSS_AP_POS_CAP_REQ                 (RILC_UNSOL_GPS_BASE + 17)


/* SITRil Gps Errors */
typedef enum _SITRilGpsError {
    SITRIL_GPS_ERROR_NONE,
    SITRIL_GPS_ERROR_NO_FILE,
    SITRIL_GPS_ERROR_NO_LIB_OPEN_FAIL,
    SITRIL_GPS_ERROR_NO_LIB_SEND_FAIL,
    SITRIL_GPS_ERROR_NO_DEVICE,
    SITRIL_GPS_ERROR_INVALID_PARAM,
    SITRIL_GPS_ERROR_REGISTERATION_FAIL,
    SITRIL_GPS_ERROR_ALREADY_REGISTERD,
    SITRIL_GPS_ERROR_MAX
} SITRilGpsError;

typedef enum _SITRilResponseError
{
    SITRIL_GPS_SUCCESS = 0,
    SITRIL_GPS_RADIO_NOT_AVAILABLE,
    SITRIL_GPS_GENERIC_FAILURE,
}SITRilResponseError;

/* Callbacks */
typedef void (*SITRilGpsOnResponse)(unsigned int msgId, int status, void* data, size_t length, unsigned int channel);
typedef int (*SITRilGpsUnsolCallback)(unsigned int msgId, void* data, size_t length, unsigned int socket_id);

// #### API's ####

/* ----------------------------------------------------
// Description
//     : Open Gps Interaface of SIT Ril.
// Results
//    - int error value of SITRilGpsError
// ----------------------------------------------------*/
int GpsOpen();

/* ----------------------------------------------------
// Description
//     : Close Gps Interaface of SIT Ril.
// Results
//    - int error value of SITRilGpsError
// ----------------------------------------------------*/
int GpsClose();

/* ----------------------------------------------------
// Description
//     : Register Callback function to receive Unsolicited events of SITRil states for Gps
// Results
//    - int error value of SITRilGpsError
// ----------------------------------------------------*/
int RegisterCallback(SITRilGpsUnsolCallback callback);

/* ----------------------------------------------------
// Description
//     : Request function to send message to CP.
// Results
//    - int error value of SITRilGpsError
// ----------------------------------------------------*/
int RequestAGPS(unsigned int msgId, unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_SET_GPS_FREQUENCY_AIDING
// RCM ID : 0x0C00
// ----------------------------------------------------*/
int GpsSetFreqAiding(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_GET_LPP_SUPL_REQ_ECID_INFO
// RCM ID : 0x0C01
// ----------------------------------------------------*/
int GpsGetLppSuplEcidInfo(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_GET_RRLP_SUPL_REQ_ECID_INFO
// RCM ID : 0x0C02
// ----------------------------------------------------*/
int GpsGetRrlpSuplEcidInfo(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_IND_GPS_MEASURE_POSITION_RSP
// RCM ID : 0x0C04
// ----------------------------------------------------*/
int IndGpsMeasurePositionRsp(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_IND_RELEASE_GPS
// RCM ID : 0x0C06
// ----------------------------------------------------*/
int IndReleaseGps(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_GPS_MO_LOCATION_REQUEST
// RCM ID : 0x0C07
// ----------------------------------------------------*/
int GpsMoLocationReq(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_IND_GPS_MT_LOCATION_REQUEST
// RCM ID : 0x0C08
//  ----------------------------------------------------*/
int IndGpsMTLocationReq(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_GET_LPP_REQ_SERVING_CELL_INFO
// RCM ID : 0x0C0A
// ----------------------------------------------------*/
int GpsGetLppReqServingCellInfo(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_LPP_PROVIDE_CAPABILITIES_IND
// RCM ID : 0x0C0C
// ----------------------------------------------------*/
int LppProvideCapabilitiesInd(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_IND_LPP_REQUEST_ASSIST_DATA
// RCM ID : 0x0C0D
// ----------------------------------------------------*/
int IndLppRequestAssistData(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_LPP_PROVIDE_LOCATION_INFO_IND
// RCM ID : 0x0C10
// ----------------------------------------------------*/
int LppProvideLocationInfoInd(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_LPP_GPS_ERROR_IND
// RCM ID : 0x0C11
// Direction : From AP to NW
// ----------------------------------------------------*/
int LppGpsErrorInd(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_IND_SUPL_LPP_DATA_INFO
// RCM ID : 0x0C12
// Direction : From AP to NW
// ----------------------------------------------------*/
int IndSuplLppDataInfo(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_IND_SUPL_NI_MESSAGE
// RCM ID : 0x0C13
// Direction : From AP to NW
// Description
//     : Get Serving Cell Information Response
// ----------------------------------------------------*/
int IndSuplNiMessage(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_SET_GPS_SUPL_NI_READY
// RCM ID : 0x0C14
// ----------------------------------------------------*/
int SetGpsSuplNiReady(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_SET_GANSS_MEAS_POS_RSP
// RCM ID : 0x0C17
// Description
//     : This message is used to send Measure Position Response CP/server.
// ----------------------------------------------------*/
int SetGanssMeasPosRsp(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_SET_GPS_LOCK_MODE
// RCM ID : 0x0C20
// Description
//     : This message is used to set GPS lock mode based on the user setting to enable/disable location services.
//       When GPS lock is enabled,CP shall reject all NI-LR requests from server except for emergency call.
// ----------------------------------------------------*/
int SetGpsLockMode(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_GET_REFERENCE_LOCATION
// RCM ID : 0x0C21
// Description
//     : This message is used to fetch Reference location assistance data from server over TIA-801(CDMA) protocol.
// ----------------------------------------------------*/
int GetRefLocation(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_SET_PSEUDO_RANGE_MEASUREMENTS
// RCM ID : 0x0C23
// Description
//     : This message is used to receive Pseudo Range Measurements from GPS chip.
// ----------------------------------------------------*/
int SetPseudoRangeMeas(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_GET_CDMA_PRECISE_TIME_AIDING_INFO
// RCM ID : 0x0C26
// Description
//     : This message is  used to get precise time aiding info from modem.
// ----------------------------------------------------*/
int GetCdmaPreciseTimeAidingInfo(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_IND_GANSS_AP_POS_CAP_RSP
// RCM ID : 0x0C19
// Direction : from AP
// Description
//     : This message is used by AP to send GSM Position Capability Response to CP.
// ----------------------------------------------------*/
int IndGanssApPosCapRsp(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_GET_GSM_EXT_INFO_MSG
// RCM ID : 0x0C1A
// Description
//     : This message is used to get GSM extended Radio signal information.
// ----------------------------------------------------*/
int GetGsmExtInfoMsg(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_GPS_CONTROL_PLANE_ENABLE
// RCM ID : 0x0C1B
// ----------------------------------------------------*/
int GpsControlPlaneEnable(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_GNSS_LPP_PROFILE_SET
// RCM ID : 0x0C1C
// Description
//     : This message is used to enable/disable LPP features at CP.
// ----------------------------------------------------*/
int GnssLppProfileSet(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_GET_GPS_CDMA_FREQ_AIDING
// RCM ID : 0x0C27
// Description
//     : This message is used to receive TCXO clock frequency data from CP to help GPS operation.
// ----------------------------------------------------*/
int GetGpsCdmaFreqAiding(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

/* ----------------------------------------------------
// SIT CMD Name : SIT_GPS_RETRIEVE_LOC_INFO
// RCM ID : 0x0C2A
// Description
//     : This message is used to send location information from UE to network.
// ----------------------------------------------------*/
int GpsRetrieveLocInfo(unsigned char *data, int data_len, SITRilGpsOnResponse callback, int channel);

#ifdef __cplusplus
};
#endif

#endif  /* SIT_RIL_GPS_H */
