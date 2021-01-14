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
 * protocolimsadapter.cpp
 *
 *  Created on: 2014. 11. 19.
 *      Author: mox
 */


#include <memory.h>
#include <string.h>
#include <telephony/ril.h>
#include "protocolimsadapter.h"
#include "rillog.h"
#include "util.h"
#include "rildef.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/**
 * ProtocolImsRespAdapter
 */

BYTE ProtocolImsReasonRespAdapter::GetResult()
{
    sit_ims_gen_reason_rsp *rsp = (sit_ims_gen_reason_rsp*)m_pModemData->GetRawData();
    return rsp->result;
}

BYTE ProtocolImsReasonRespAdapter::GetFailReason()
{
    sit_ims_gen_reason_rsp *rsp = (sit_ims_gen_reason_rsp*)m_pModemData->GetRawData();
    return rsp->fail_reason;
}

BYTE ProtocolImsRespAdapter::GetResult()
{
    sit_ims_gen_rsp *rsp = (sit_ims_gen_rsp*)m_pModemData->GetRawData();
    return rsp->result;
}

/**
 * ProtocolAimsIndAdapter
 */
int ProtocolAimsIndAdapter::GetResultId() const
{

    // index 0 : OEM RIL ID
    // index 1 : SIT message ID
    const static int messageMap[][2] = {
        { RIL_UNSOL_OEM_AIMS_CALL_RING, SIT_IND_AIMS_CALL_RING },
        { RIL_UNSOL_OEM_AIMS_CALL_STATUS, SIT_IND_AIMS_CALL_STATUS  },
        { RIL_UNSOL_OEM_AIMS_REGISTRATION, SIT_IND_AIMS_REGISTRATION  },
        { RIL_UNSOL_OEM_AIMS_CALL_MODIFY, SIT_IND_AIMS_CALL_MODIFY  },
        { RIL_UNSOL_OEM_AIMS_FRAME_TIME, SIT_IND_AIMS_FRAME_TIME },
        { RIL_UNSOL_OEM_AIMS_SUPP_SVC_NOTIFICATION, SIT_IND_AIMS_SUPP_SVC_NOTIFICATION },
        { RIL_UNSOL_OEM_AIMS_NEW_SMS, SIT_IND_AIMS_NEW_SMS },
        { RIL_UNSOL_OEM_AIMS_NEW_SMS_STATUS_REPORT, SIT_IND_AIMS_NEW_SMS_STATUS_REPORT },
        { RIL_UNSOL_OEM_AIMS_ON_USSD, SIT_IND_AIMS_ON_USSD },
        { RIL_UNSOL_OEM_AIMS_CONFERENCE_CALL_EVENT, SIT_IND_AIMS_CONFERENCE_CALL_EVENT },
        { RIL_UNSOL_OEM_AIMS_PAYLOAD_INFO, SIT_IND_AIMS_PAYLOAD_INFO_IND },
        { RIL_UNSOL_OEM_AIMS_VOWIFI_HO_CALL_INFO, SIT_IND_AIMS_VOWIFI_HO_CALL_INFO },
        { RIL_UNSOL_OEM_AIMS_NEW_CDMA_SMS, SIT_IND_AIMS_NEW_CDMA_SMS },
        { RIL_UNSOL_OEM_AIMS_RINGBACK_TONE, SIT_IND_AIMS_RINGBACK_TONE },
        { RIL_UNSOL_OEM_AIMS_CALL_MANAGE, SIT_IND_AIMS_CALL_MANAGE },
        { RIL_UNSOL_OEM_AIMS_CONF_CALL_ADD_REMOVE_USER, SIT_IND_AIMS_CONF_CALL_ADD_REMOVE_USER },
        { RIL_UNSOL_OEM_AIMS_ENHANCED_CONF_CALL, SIT_IND_AIMS_ENHANCED_CONF_CALL },
        { RIL_UNSOL_OEM_AIMS_CALL_MODIFY_RSP, SIT_IND_AIMS_CALL_MODIFY_RSP },
        { RIL_UNSOL_OEM_AIMS_DTMF_EVENT, SIT_IND_AIMS_DTMF_EVENT },
        { RIL_UNSOL_OEM_AIMS_RTT_NEW_TEXT, SIT_IND_AIMS_RTT_NEW_TEXT },
        { RIL_UNSOL_OEM_AIMS_RTT_FAIL_SENDING_TEXT, SIT_IND_AIMS_RTT_FAIL_SENDING_TEXT },
        { RIL_UNSOL_OEM_AIMS_EXIT_EMERGENCY_CB_MODE, SIT_IND_AIMS_EXIT_EMERGENCY_CB_MODE },
        { RIL_UNSOL_OEM_AIMS_DIALOG_INFO, SIT_IND_AIMS_DIALOG_INFO },
        { RIL_UNSOL_OEM_AIMS_RCS_MULTI_FRAME, SIT_IND_AIMS_RCS_MULTI_FRAME},
        { RIL_UNSOL_OEM_AIMS_RCS_CHAT, SIT_IND_AIMS_RCS_CHAT},
        { RIL_UNSOL_OEM_AIMS_RCS_GROUP_CHAT, SIT_IND_AIMS_RCS_GROUP_CHAT},
        { RIL_UNSOL_OEM_AIMS_RCS_OFFLINE_MODE, SIT_IND_AIMS_RCS_OFFLINE_MODE},
        { RIL_UNSOL_OEM_AIMS_RCS_FILE_TRANSFER, SIT_IND_AIMS_RCS_FILE_TRANSFER},
        { RIL_UNSOL_OEM_AIMS_RCS_COMMON_MESSAGE, SIT_IND_AIMS_RCS_COMMON_MESSAGE},
        { RIL_UNSOL_OEM_AIMS_RCS_CONTENT_SHARE, SIT_IND_AIMS_RCS_CONTENT_SHARE},
        { RIL_UNSOL_OEM_AIMS_RCS_PRESENCE, SIT_IND_AIMS_RCS_PRESENCE},
        { RIL_UNSOL_OEM_AIMS_RCS_XCAP_MANAGE, SIT_IND_AIMS_RCS_XCAP_MANAGE},
        { RIL_UNSOL_OEM_AIMS_RCS_CONFIG_MANAGE, SIT_IND_AIMS_RCS_CONFIG_MANAGE},
        { RIL_UNSOL_OEM_AIMS_RCS_TLS_MANAGE, SIT_IND_AIMS_RCS_TLS_MANAGE},
        { RIL_UNSOL_OEM_WFC_RTP_RTCP_TIMEOUT, SIT_IND_WFC_RTP_RTCP_TIMEOUT},
        { RIL_UNSOL_OEM_WFC_FIRST_RTP, SIT_IND_WFC_FIRST_RTP},
        { RIL_UNSOL_OEM_WFC_RTCP_RX_SR, SIT_IND_WFC_RTCP_RX_SR},
        { RIL_UNSOL_OEM_WFC_RCV_DTMF_NOTI, SIT_IND_WFC_RCV_DTMF_NOTI},
        { RIL_UNSOL_OEM_AIMS_MEDIA_STATUS, SIT_IND_AIMS_MEDIA_STATUS},
        { RIL_UNSOL_OEM_AIMS_SIP_MSG_INFO, SIT_IND_AIMS_SIP_MSG_INFO},
        { RIL_UNSOL_OEM_AIMS_VOICE_RTP_QUALITY, SIT_IND_AIMS_VOICE_RTP_QUALITY},
        { RIL_UNSOL_OEM_AIMS_RTP_RX_STATISTICS, SIT_AIMS_IND_RTP_RX_STATISTICS},
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

INT32 ProtocolImsRegIndAdapter::GetRegState()
{
    sit_ims_ind_reg *rsp = (sit_ims_ind_reg*)m_pModemData->GetRawData();
    if ( rsp != NULL ) {
        switch(rsp->state)
        {
        case SITRIL_AIMS_IMSREG_STATE_REGISTERED:
            return RIL_IMS_REGISTERED;
        case SITRIL_AIMS_IMSREG_STATE_NOT_REGISTERED:
            return RIL_IMS_NOT_REGISTERED;
        default:
            return -1;
        }
    }
    else {
        return -1;
    }
}

BOOL ProtocolImsAimCallWaitingAdapter::IsEnable()
{
    BOOL bEnable = FALSE;
    sit_aims_set_call_waiting_ex* pReq = (sit_aims_set_call_waiting_ex*)m_pRawData;
    if ( pReq != NULL ) {
        bEnable = (pReq->service_status==0x01);
    }
    return bEnable;
}

INT32 ProtocolImsAimCallWaitingAdapter::GetServiceClass()
{
    INT32 service_class = 0;
    sit_aims_set_call_waiting_ex* pReq = (sit_aims_set_call_waiting_ex*)m_pRawData;
    if ( pReq != NULL ) {
        service_class = pReq->service_class;
    }
    return service_class;
}
