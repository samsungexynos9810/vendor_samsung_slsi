/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "protocolgpsadapter.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_GPS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/**
 * ProtocolAgpsIndAdapter
 */
int ProtocolAgpsIndAdapter::GetResultId() const
{

    // index 0 : OEM RIL ID
    // index 1 : SIT message ID
    const static int messageMap[][2] = {
        { RIL_UNSOL_OEM_GPS_MEASURE_POS_REQ, SIT_IND_GPS_MEASURE_POS_REQ },
        { RIL_UNSOL_OEM_GPS_ASSIST_DATA, SIT_IND_GPS_ASSIST_DATA },
        { RIL_UNSOL_OEM_GPS_RELEASE_GPS, SIT_IND_RELEASE_GPS },
        { RIL_UNSOL_OEM_GPS_MT_LOCATION_REQUEST, SIT_IND_GPS_MT_LOCATION_REQUEST },
        { RIL_UNSOL_OEM_GPS_RESET_GPS_ASSIST_DATA, SIT_IND_RESET_GPS_ASSIST_DATA },
        { RIL_UNSOL_OEM_GPS_LPP_REQUEST_CAPABILITIES, SIT_IND_LPP_REQUEST_CAPABILITIES },
        { RIL_UNSOL_OEM_GPS_LPP_PROVIDE_ASSIST_DATA, SIT_IND_LPP_PROVIDE_ASSIST_DATA },
        { RIL_UNSOL_OEM_GPS_LPP_REQUEST_LOCATION_INFO, SIT_IND_LPP_REQUEST_LOCATION_INFO },
        { RIL_UNSOL_OEM_GPS_LPP_GPS_ERROR_IND, SIT_LPP_GPS_ERROR_IND },
        { RIL_UNSOL_OEM_GPS_SUPL_LPP_DATA_INFO, SIT_IND_SUPL_LPP_DATA_INFO },
        { RIL_UNSOL_OEM_GPS_SUPL_NI_MESSAGE, SIT_IND_SUPL_NI_MESSAGE },
        { RIL_UNSOL_OEM_GPS_SUPL_NI_READY, SIT_SET_GPS_SUPL_NI_READY },
        { RIL_UNSOL_OEM_GPS_START_MDT_LOC, SIT_IND_GPS_START_MDT_LOC },
        { RIL_UNSOL_OEM_GPS_LPP_UPDATE_UE_LOC_INFO, SIT_IND_LPP_UPDATE_UE_LOC_INFO },
        { RIL_UNSOL_OEM_GPS_LOCK_MODE, SIT_IND_GPS_LOCK_MODE },
        // CDMA & HEDGE GANSS
        { RIL_UNSOL_OEM_GPS_3GPP_SEND_GANSS_ASSIT_DATA, SIT_IND_3GPP_SEND_GANSS_ASSIT_DATA },
        { RIL_UNSOL_OEM_GPS_GANSS_MEAS_POS_MSG, SIT_IND_GANSS_MEAS_POS_MSG },
        { RIL_UNSOL_OEM_GPS_CDMA_GPS_POWER_ON, SIT_IND_CDMA_GPS_POWER_ON },
        { RIL_UNSOL_OEM_GPS_CDMA_SEND_ACQUSITION_ASSIT_DATA, SIT_IND_CDMA_SEND_ACQUSITION_ASSIT_DATA },
        { RIL_UNSOL_OEM_GPS_CDMA_SESSION_CANCELLATION, SIT_IND_CDMA_SESSION_CANCELLATION },
        { RIL_UNSOL_OEM_GPS_GANSS_AP_POS_CAP_REQ, SIT_IND_GANSS_AP_POS_CAP_REQ },
    };

    if (m_pModemData != NULL) {
        RCM_IND_HEADER *hdr = (RCM_IND_HEADER *)m_pModemData->GetRawData();
        if (hdr != NULL) {
            int size = sizeof(messageMap) / sizeof(messageMap[0]);
            for (int i = 0; i < size; i++) {
                if (hdr->id == messageMap[i][1]) {
                    return messageMap[i][0];
                }
            } // end for i ~
        }
    }

    return -1;
}
