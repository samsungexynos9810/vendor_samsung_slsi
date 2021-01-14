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
 * protocolimsbuilder.cpp
 *
 *  Created on: 2014. 11. 18.
 *      Author: Martin
 */

#include <memory.h>
#include <string.h>
#include "protocolbuilder.h"
#include "protocolimsbuilder.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_IMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

ModemData *ProtocolImsBuilder::BuildSetConfig(const char *pSetConfigData)
{
    sit_ims_set_conf_req req;
    int length = sizeof(sit_ims_set_conf_req);

    memset(&req, 0, sizeof(sit_ims_set_conf_req));
    req.ConfigSelection = pSetConfigData[0];
    memcpy(req.data, &pSetConfigData[1], sizeof(req.data));

    InitRequestHeader(&req.hdr, SIT_SET_IMS_CONFIGURATION, length);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolImsBuilder::BuildGetConfig()
{
    sit_ims_get_conf_req req;
    int length = sizeof(RCM_HEADER);

    InitRequestHeader(&req.hdr, SIT_GET_IMS_CONFIGURATION, length);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolImsBuilder::BuildEmergencyCallStatus(BYTE nStatus, BYTE nRat)
{
    sit_net_set_emergency_call_status req;
    int length = sizeof(req);

    memset(&req, 0, sizeof(sit_net_set_emergency_call_status));
    req.Status = nStatus;
    req.Rat = nRat;

    InitRequestHeader(&req.hdr, SIT_SET_EMERGENCY_CALL_STATUS, length);

    return new ModemData((char *)&req, length);
}


ModemData *ProtocolImsBuilder::BuildSetSrvccCallList(const char *pSrvccCallList)
{
    int length=0;
    sit_call_set_srvcc_call_list_req req;
    sit_call_set_srvcc_call_list *receive_data;
    memset(&req, 0, sizeof(sit_call_set_srvcc_call_list_req));
    receive_data = (sit_call_set_srvcc_call_list *)pSrvccCallList;
    req.call_list.number = (receive_data->number);
    memcpy(req.call_list.record, &(receive_data->record), req.call_list.number*sizeof(sit_call_info_type));
    RilLogV("[ProtocolImsBuilder::%s] Srvcc Call List count: %d ", __FUNCTION__, req.call_list.number);
    RilLogV("[ProtocolImsBuilder::%s] %02x %02x %02x %02x %02x %02x  ", __FUNCTION__, *pSrvccCallList, *(pSrvccCallList+1), *(pSrvccCallList+2), *(pSrvccCallList+3), *(pSrvccCallList+4), *(pSrvccCallList+5));
    length += sizeof(RCM_HEADER) + sizeof(INT32) + req.call_list.number*sizeof(sit_call_info_type);
    InitRequestHeader(&req.hdr, SIT_SET_SRVCC_CALL_LIST, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolImsBuilder::BuildAimsPDU(int requestId, void *data, unsigned int datalen)
{
    // index 0 : OEM RIL ID
    // index 1 : SIT message ID
    static const int messageMap[][2] = {
        { RIL_REQUEST_OEM_AIMS_DIAL, SIT_AIMS_DIAL },
        { RIL_REQUEST_OEM_AIMS_ANSWER, SIT_AIMS_ANSWER },
        { RIL_REQUEST_OEM_AIMS_HANGUP, SIT_AIMS_HANGUP },
        { RIL_REQUEST_OEM_AIMS_DEREGISTRATION, SIT_AIMS_DEREGISTRATION },
        { RIL_REQUEST_OEM_AIMS_HIDDEN_MENU, SIT_AIMS_HIDDEN_MENU },
        { RIL_REQUEST_OEM_AIMS_ADD_PDN_INFO, SIT_AIMS_ADD_PDN_INFO },
        { RIL_REQUEST_OEM_AIMS_CALL_MANAGE, SIT_AIMS_CALL_MANAGE },
        { RIL_REQUEST_OEM_AIMS_SEND_DTMF, SIT_AIMS_SEND_DTMF },
        { RIL_REQUEST_OEM_AIMS_SET_FRAME_TIME, SIT_AIMS_SET_FRAME_TIME },
        { RIL_REQUEST_OEM_AIMS_GET_FRAME_TIME, SIT_AIMS_GET_FRAME_TIME },
        { RIL_REQUEST_OEM_AIMS_CALL_MODIFY, SIT_AIMS_CALL_MODIFY },
        { RIL_REQUEST_OEM_AIMS_RESPONSE_CALL_MODIFY, SIT_AIMS_RESPONSE_CALL_MODIFY },
        { RIL_REQUEST_OEM_AIMS_TIME_INFO, SIT_AIMS_TIME_INFO },
        { RIL_REQUEST_OEM_AIMS_CONF_CALL_ADD_REMOVE_USER, SIT_AIMS_CONF_CALL_ADD_REMOVE_USER },
        { RIL_REQUEST_OEM_AIMS_ENHANCED_CONF_CALL, SIT_AIMS_ENHANCED_CONF_CALL },
        { RIL_REQUEST_OEM_AIMS_GET_CALL_FORWARD_STATUS, SIT_AIMS_GET_CALL_FORWARD_STATUS },
        { RIL_REQUEST_OEM_AIMS_SET_CALL_FORWARD_STATUS, SIT_AIMS_SET_CALL_FORWARD_STATUS },
        { RIL_REQUEST_OEM_AIMS_GET_CALL_WAITING, SIT_AIMS_GET_CALL_WAITING },
        { RIL_REQUEST_OEM_AIMS_SET_CALL_WAITING, SIT_AIMS_SET_CALL_WAITING },
        { RIL_REQUEST_OEM_AIMS_GET_CALL_BARRING, SIT_AIMS_GET_CALL_BARRING },
        { RIL_REQUEST_OEM_AIMS_SET_CALL_BARRING, SIT_AIMS_SET_CALL_BARRING },
        { RIL_REQUEST_OEM_AIMS_SEND_SMS, SIT_AIMS_SEND_SMS },
        { RIL_REQUEST_OEM_AIMS_SEND_EXPECT_MORE, SIT_AIMS_SEND_EXPECT_MORE },
        { RIL_REQUEST_OEM_AIMS_SEND_SMS_ACK, SIT_AIMS_SEND_SMS_ACK },
        { RIL_REQUEST_OEM_AIMS_SEND_ACK_INCOMING_SMS, SIT_AIMS_SEND_ACK_INCOMING_SMS },
        { RIL_REQUEST_OEM_AIMS_CHG_BARRING_PWD, SIT_AIMS_CHG_BARRING_PWD },
        { RIL_REQUEST_OEM_AIMS_SEND_USSD_INFO, SIT_AIMS_SEND_USSD_INFO },
        { RIL_REQUEST_OEM_AIMS_GET_PRESENTATION_SETTINGS, SIT_AIMS_GET_PRESENTATION_SETTINGS },
        { RIL_REQUEST_OEM_AIMS_SET_PRESENTATION_SETTINGS, SIT_AIMS_SET_PRESENTATION_SETTINGS },
        { RIL_REQUEST_OEM_AIMS_SET_SELF_CAPABILITY, SIT_AIMS_SET_SELF_CAPABILITY },
        { RIL_REQUEST_OEM_AIMS_HO_TO_WIFI_READY, SIT_AIMS_HO_TO_WIFI_READY_REQ },
        { RIL_REQUEST_OEM_AIMS_HO_TO_3GPP, SIT_AIMS_HO_TO_3GPP_REQ },
        { RIL_REQUEST_OEM_AIMS_SEND_ACK_INCOMING_CDMA_SMS, SIT_AIMS_SEND_ACK_INCOMING_CDMA_SMS },
        { RIL_REQUEST_OEM_AIMS_DEL_PDN_INFO, SIT_AIMS_DEL_PDN_INFO },
        { RIL_REQUEST_OEM_AIMS_STACK_START_REQ, SIT_AIMS_STACK_START_REQ },
        { RIL_REQUEST_OEM_AIMS_STACK_STOP_REQ, SIT_AIMS_STACK_STOP_REQ },
        { RIL_REQUEST_OEM_AIMS_XCAPM_START_REQ, SIT_AIMS_XCAPM_START_REQ },
        { RIL_REQUEST_OEM_AIMS_XCAPM_STOP_REQ, SIT_AIMS_XCAPM_STOP_REQ },
        { RIL_REQUEST_OEM_AIMS_RTT_SEND_TEXT, SIT_AIMS_RTT_SEND_TEXT },
        { RIL_REQUEST_OEM_AIMS_EXIT_EMERGENCY_CB_MODE, SIT_AIMS_EXIT_EMERGENCY_CB_MODE },
        { RIL_REQUEST_OEM_AIMS_SET_GEO_LOCATION_INFO, SIT_AIMS_SET_GEO_LOCATION_INFO },
        { RIL_REQUEST_OEM_AIMS_CDMA_SEND_SMS, SIT_AIMS_CDMA_SEND_SMS },
        { RIL_REQUEST_OEM_AIMS_RCS_MULTI_FRAME, SIT_AIMS_RCS_MULTI_FRAME },
        { RIL_REQUEST_OEM_AIMS_RCS_CHAT, SIT_AIMS_RCS_CHAT },
        { RIL_REQUEST_OEM_AIMS_RCS_GROUP_CHAT, SIT_AIMS_RCS_GROUP_CHAT },
        { RIL_REQUEST_OEM_AIMS_RCS_OFFLINE_MODE, SIT_AIMS_RCS_OFFLINE_MODE },
        { RIL_REQUEST_OEM_AIMS_RCS_FILE_TRANSFER, SIT_AIMS_RCS_FILE_TRANSFER },
        { RIL_REQUEST_OEM_AIMS_RCS_COMMON_MESSAGE, SIT_AIMS_RCS_COMMON_MESSAGE },
        { RIL_REQUEST_OEM_AIMS_RCS_CONTENT_SHARE, SIT_AIMS_RCS_CONTENT_SHARE },
        { RIL_REQUEST_OEM_AIMS_RCS_PRESENCE, SIT_AIMS_RCS_PRESENCE },
        { RIL_REQUEST_OEM_AIMS_XCAP_MANAGE, SIT_AIMS_RCS_XCAP_MANAGE },
        { RIL_REQUEST_OEM_AIMS_RCS_CONFIG_MANAGE, SIT_AIMS_RCS_CONFIG_MANAGE },
        { RIL_REQUEST_OEM_AIMS_RCS_TLS_MANAGE, SIT_AIMS_RCS_TLS_MANAGE },
        { RIL_REQUEST_OEM_AIMS_SET_PDN_EST_STATUS, SIT_AIMS_SET_PDN_EST_STATUS },
        { RIL_REQUEST_OEM_AIMS_SET_HIDDEN_MENU_ITEM, SIT_AIMS_SET_HIDDEN_MENU_ITEM},
        { RIL_REQUEST_OEM_AIMS_GET_HIDDEN_MENU_ITEM, SIT_AIMS_GET_HIDDEN_MENU_ITEM},
        { RIL_REQUEST_OEM_AIMS_SET_RTP_RX_STATISTICS, SIT_AIMS_SET_RTP_RX_STATISTICS },
        { RIL_REQUEST_OEM_WFC_MEDIA_CHANNEL_CONFIG, SIT_SET_WFC_MEDIA_CONFIGURATION },
        { RIL_REQUEST_OEM_WFC_DTMF_START, SIT_WFC_DTMF_START },
        { RIL_REQUEST_OEM_WFC_SET_VOWIFI_HO_THRESHOLD, SIT_SET_VOWIFI_HO_THRESHOLD },
    };

    if (requestId < 0) {
        return NULL;
    }

    // TODO check maximum RCM PDU size
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

    char *buf = new char[MAX_IMS_RCM_SIZE];
    int length = sizeof(RCM_HEADER) + datalen;
    InitRequestHeader((RCM_HEADER *)buf, protocolId, length);

    if (length > MAX_IMS_RCM_SIZE) {
        // exceed allocated buffer size
        RilLogW("Too large data(%u) for requestId=%d", datalen, requestId);
        delete[] buf;
        return NULL;
    }

    if (data != NULL && datalen > 0) {
        memcpy(buf + sizeof(RCM_HEADER), data, datalen);
    }

    ModemData *modemData = new ModemData(buf, length);
    delete[] buf;
    return modemData;
}

ModemData *ProtocolImsBuilder::BuildAimsIndPDU(int requestId, void *data, unsigned int datalen)
{
    // index 0 : OEM RIL ID
    // index 1 : SIT message ID
    static const int messageMap[][2] = {
        { RIL_REQUEST_OEM_AIMS_HO_TO_WIFI_CANCEL_IND, SIT_AIMS_HO_TO_WIFI_CANCEL_IND },
        { RIL_REQUEST_OEM_AIMS_HO_PAYLOAD_IND, SIT_IND_AIMS_PAYLOAD_INFO_IND },
        { RIL_REQUEST_OEM_AIMS_MEDIA_STATE_IND, SIT_AIMS_MEDIA_STATE_IND },
    };

    if (requestId < 0) {
        return NULL;
    }

    // TODO check maximum RCM PDU size
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

    char *buf = new char[MAX_IMS_RCM_SIZE];
    int length = sizeof(RCM_IND_HEADER) + datalen;
    InitIndRequestHeader((RCM_IND_HEADER *)buf, protocolId, length);

    if (length > MAX_IMS_RCM_SIZE) {
        // exceed allocated buffer size
        RilLogW("Too large data(%u) for requestId=%d", datalen, requestId);
        delete[] buf;
        return NULL;
    }

    if (data != NULL && datalen > 0) {
        memcpy(buf + sizeof(RCM_IND_HEADER), data, datalen);
    }

    ModemData *modemData = new ModemData(buf, length);
    delete[] buf;
    return modemData;
}
