/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "protocolgpsbuilder.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

ModemData *ProtocolGpsBuilder::BuildAgpsPDU(int requestId, void *data, unsigned int datalen)
{
    // index 0 : OEM RIL ID
    // index 1 : SIT message ID
    static const int messageMap[][2] = {
        { RIL_REQUEST_OEM_GPS_SET_FREQUENCY_AIDING, SIT_SET_GPS_FREQUENCY_AIDING },
        { RIL_REQUEST_OEM_GPS_GET_LPP_SUPL_REQ_ECID_INFO, SIT_GET_LPP_SUPL_REQ_ECID_INFO },
        { RIL_REQUEST_OEM_GPS_SET_RRLP_SUPL_REQ_ECID_INFO, SIT_GET_RRLP_SUPL_REQ_ECID_INFO },
        { RIL_REQUEST_OEM_GPS_MO_LOCATION_REQUEST, SIT_GPS_MO_LOCATION_REQUEST },
        { RIL_REQUEST_OEM_GPS_GET_LPP_REQ_SERVING_CELL_INFO, SIT_GET_LPP_REQ_SERVING_CELL_INFO },
        { RIL_REQUEST_OEM_GPS_SET_SUPL_NI_READY, SIT_SET_GPS_SUPL_NI_READY },
        { RIL_REQUEST_OEM_GPS_GET_GSM_EXT_INFO_MSG, SIT_GET_GSM_EXT_INFO_MSG },
        { RIL_REQUEST_OEM_GPS_CONTROL_PLANE_ENABLE, SIT_GPS_CONTROL_PLANE_ENABLE },
        { RIL_REQUEST_OEM_GPS_GNSS_LPP_PROFILE_SET, SIT_GNSS_LPP_PROFILE_SET },
        // CDMA & HEDGE GANSS
        { RIL_REQUEST_OEM_GPS_SET_GANSS_MEAS_POS_RSP, SIT_SET_GANSS_MEAS_POS_RSP },
        { RIL_REQUEST_OEM_GPS_SET_GPS_LOCK_MODE, SIT_SET_GPS_LOCK_MODE },
        { RIL_REQUEST_OEM_GPS_GET_REFERENCE_LOCATION, SIT_GET_REFERENCE_LOCATION },
        { RIL_REQUEST_OEM_GPS_SET_PSEUDO_RANGE_MEASUREMENTS, SIT_SET_PSEUDO_RANGE_MEASUREMENTS },
        { RIL_REQUEST_OEM_GPS_GET_CDMA_PRECISE_TIME_AIDING_INFO, SIT_GET_CDMA_PRECISE_TIME_AIDING_INFO },
        { RIL_REQUEST_OEM_GPS_CDMA_FREQ_AIDING, SIT_GET_GPS_CDMA_FREQ_AIDING },
    };

    if (requestId < 0) {
        return NULL;
    }

    if (data == NULL && datalen > 0) {
        datalen = 0;
    }

    int protocolId = -1;
    int size = sizeof(messageMap) / sizeof(messageMap[0]);
    for (int i = 0; i < size; i++) {
        if (messageMap[i][0] == requestId) {
            protocolId = messageMap[i][1];
            break;
        }
    } // end for i ~

    if (protocolId < 0) {
        return NULL;
    }

    const int MAX_RCM_SIZE = (2 * 1024);
    char buf[MAX_RCM_SIZE] = { 0, };
    int length= sizeof(RCM_HEADER) + datalen;
    InitRequestHeader((RCM_HEADER *)buf, protocolId, length);

    if (data != NULL && length < MAX_RCM_SIZE) {
        memcpy(buf + sizeof(RCM_HEADER), data, datalen);
    }

    return new ModemData(buf, length);
}

ModemData *ProtocolGpsBuilder::BuildAgpsIndPDU(int requestId, void *data, unsigned int datalen)
{
    // index 0 : OEM RIL ID
    // index 1 : SIT message ID
    static const int messageMap[][2] = {
        { RIL_REQUEST_OEM_GPS_MEASURE_POS_RSP, SIT_IND_GPS_MEASURE_POSITION_RSP },
        { RIL_REQUEST_OEM_GPS_RELEASE_GPS, SIT_IND_RELEASE_GPS },
        { RIL_REQUEST_OEM_GPS_MT_LOCATION_REQUEST, SIT_IND_GPS_MT_LOCATION_REQUEST },
        { RIL_REQUEST_OEM_GPS_LPP_PROVIDE_CAPABILITIES, SIT_LPP_PROVIDE_CAPABILITIES_IND },
        { RIL_REQUEST_OEM_GPS_LPP_REQUEST_ASSIST_DATA, SIT_IND_LPP_REQUEST_ASSIST_DATA },
        { RIL_REQUEST_OEM_GPS_LPP_PROVIDE_LOCATION_INFO, SIT_LPP_PROVIDE_LOCATION_INFO_IND },
        { RIL_REQUEST_OEM_GPS_LPP_GPS_ERROR_IND, SIT_LPP_GPS_ERROR_IND },
        { RIL_REQUEST_OEM_GPS_SUPL_LPP_DATA_INFO, SIT_IND_SUPL_LPP_DATA_INFO },
        { RIL_REQUEST_OEM_GPS_SUPL_NI_MESSAGE, SIT_IND_SUPL_NI_MESSAGE },
        { RIL_REQUEST_OEM_GPS_RETRIEVE_LOC_INFO, SIT_GPS_RETRIEVE_LOC_INFO },
        // CDMA & HEDGE GANSS
        { RIL_REQUEST_OEM_GPS_GANSS_AP_POS_CAP_RSP, SIT_IND_GANSS_AP_POS_CAP_RSP },
    };

    if (requestId < 0) {
        return NULL;
    }

    if (data == NULL && datalen > 0) {
        datalen = 0;
    }

    int protocolId = -1;
    int size = sizeof(messageMap) / sizeof(messageMap[0]);
    for (int i = 0; i < size; i++) {
        if (messageMap[i][0] == requestId) {
            protocolId = messageMap[i][1];
            break;
        }
    } // end for i ~

    if (protocolId < 0) {
        return NULL;
    }

    const int MAX_RCM_SIZE = (2 * 1024);
    char buf[MAX_RCM_SIZE] = { 0, };
    int length = sizeof(RCM_IND_HEADER) + datalen;
    InitIndRequestHeader((RCM_IND_HEADER *)buf, protocolId, length);

    if (data != NULL && length < MAX_RCM_SIZE) {
        memcpy(buf + sizeof(RCM_IND_HEADER), data, datalen);
    }

    return new ModemData(buf, length);
}
