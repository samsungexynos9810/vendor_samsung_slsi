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
 * protocolsmsbuilder.cpp
 *
 *  Created on: 2014. 7. 3.
 *      Author: sungwoo48.choi
 */
#include "protocolsmsbuilder.h"
#include "rillog.h"

#ifndef NO_USE_SMS_DOMAIN
#include "systemproperty.h"

#define PROPERTY_SMS_DOMAIN   "vendor.radio.smsdomain"

#define CS_PREF 0
#define PS_PREF 1
#define CS_ONLY 2
#define PS_ONLY 3
#endif // NO_USE_SMS_DOMAIN

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

ModemData *ProtocolSmsBuilder::BuildSendSms(const char *smsc, int smscLen, const char *pdu, int pduSize, bool bExpectMore)
{
    if (pdu == NULL || *pdu == 0) {
        RilLogE("Wrong pdu");
        return NULL;
    }

    if (pduSize <= 0 || pduSize > MAX_GSM_SMS_TPDU_SIZE) {
        RilLogE("pduSize=%d, MAX_TPDU_SIZE=%d", pduSize, MAX_GSM_SMS_TPDU_SIZE);
        return NULL;
    }

    sit_sms_send_sms_req req;
    int length = sizeof(req);
    memset(&req, 0, length);

    if (bExpectMore == TRUE)
        InitRequestHeader(&req.hdr, SIT_SEND_SMS_EXPECT_MORE, length);
    else // bExpectMore == FALSE
        InitRequestHeader(&req.hdr, SIT_SEND_SMS, length);

#ifndef NO_USE_SMS_DOMAIN
    unsigned int domain = stoi(SystemProperty::Get(PROPERTY_SMS_DOMAIN, "0"/*default:CS pref*/));
    req.sms_domain = (domain > PS_ONLY) ? CS_PREF : domain;
#endif // NO_USE_SMS_DOMAIN
    if (smscLen <= 0 || smscLen > MAX_GSM_SMS_SERVICE_CENTER_ADDR || smsc == NULL) {
        //SMSC is not valid, we should set SMSC value as 0x00 and SMSC length will be 1.
        req.smsc[0] = 0x00;
        req.smsc_len = 0x01;
        RilLogI("SMSC is not valid. Sending SMSC with value 0x00");
    } else {
        req.smsc_len = smscLen;
        memcpy(req.smsc, smsc, smscLen);
    }
    req.sms_len = pduSize;
    memcpy(req.sms_data, pdu, pduSize);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildSmsAck(int result, int tpid, int error)
{
    if (result < 0) {
        return NULL;
    }

    if (tpid < 0) {
        return NULL;
    }

    sit_sms_send_sms_ack_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SEND_SMS_ACK, length);
    req.result = ((result & 0xFF) == 0x00 ? 0x00 : 0x01);
    req.msg_tpid = (tpid & 0xFF);
    req.error_code = error;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildSmsAck(int result, int tpid, const char *pdu, int pduSize)
{
    if (result < 0) {
        return NULL;
    }

    if (tpid < 0) {
        return NULL;
    }

    sit_sms_send_ack_incoming_sms_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SEND_ACK_INCOMING_SMS, length);
    req.result = ((result & 0xFF) == 0x00 ? 0x00 : 0x01);
    req.msg_tpid = (tpid & 0xFF);
    req.tpdu_len = pduSize;
    memcpy(req.tpdu, pdu, pduSize);

    return new ModemData((char *)&req, length);
}


ModemData *ProtocolSmsBuilder::BuildSmscAddress()
{
    sit_sms_get_smsc_addr_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_SMSC_ADDR, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildSmscAddress(int sca_len, const char *sca)
{
    sit_sms_set_smsc_addr_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_SMSC_ADDR, length);
    req.sca_len = sca_len;
    memcpy(req.sca, sca, MAX_GSM_SMS_SERVICE_CENTER_ADDR);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildSmsMemoryStatus(int status)
{
    sit_sms_send_sms_mem_status_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SEND_SMS_MEM_STATUS, length);
    if (status == 0x00) {
        req.mem_status = MEMORY_CAPACITY_EXCEEDED;
    }
    else {
        req.mem_status = MEMORY_AVAILABLE;
    }

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildWriteSmsToSim(int status, int index, int pduSize, const char *pdu)
{
    if (pduSize <= 0 || pduSize > MAX_GSM_SMS_TPDU_SIZE) {
        return NULL;
    }
    //if (pdu == NULL || *pdu == 0 || pduSize <= 0) {
    if (pdu == NULL || pduSize <= 0) {
        return NULL;
    }

    sit_sms_write_sms_to_sim_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_WRITE_SMS_TO_SIM, length);

    switch (status)
    {
        case 0x0:    req.status = SIT_SIM_STATUS_RECEIVED_UNREAD;
                    break;
        case 0x1:    req.status = SIT_SIM_STATUS_RECEIVED_READ;
                    break;
        case 0x2:    req.status = SIT_SIM_STATUS_STORED_UNSENT;
                    break;
        case 0x3:    req.status = SIT_SIM_STATUS_STORED_SENT;
                    break;
        default:    RilLogE("Status is not a proper value.");
                    return NULL;
                    break;
    }

    req.index = index; // 0xFFFF: Default value all the time, except for a test.
    req.pdu_len = pduSize;
    memcpy(req.pdu_data, pdu, pduSize);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildDeleteSmsOnSim(int index)
{
    sit_sms_delete_sms_on_sim_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_DELETE_SMS_ON_SIM, length);

    req.index = index;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildGetBroadcastSmsConfig()
{
    sit_sms_get_bcst_sms_cfg_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_BCST_SMS_CFG, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildSetBroadcastSmsConfig(
   const RIL_GSM_BroadcastSmsConfigInfo *rgbsci, int num)
{
    if (rgbsci == NULL || num <= 0) {
        RilLogE("Wrong Broadcast SMS Configs Info!!!");
        return NULL;
    }

    if (num > MAX_BCST_INFO_NUM) {
        RilLogW("Broadcast SMS Configs Number = %d, MAX_BCST_INFO_NUM = %d", num,
                MAX_BCST_INFO_NUM);
        num = MAX_BCST_INFO_NUM;
    }

    sit_sms_set_bcst_sms_cfg_req req;
    int length = sizeof(req.hdr) + sizeof(req.bcst_info_num) +
        (num * sizeof(sit_sms_bcst_sms_cfg_item));
    InitRequestHeader(&req.hdr, SIT_SET_BCST_SMS_CFG, length);
    req.bcst_info_num = num;
    for (int i = 0; i < num; i++) {
        req.cfgitem[i].from_svc_id = rgbsci[i].fromServiceId;
        req.cfgitem[i].to_svc_id = rgbsci[i].toServiceId;
        req.cfgitem[i].from_code_scheme = rgbsci[i].fromCodeScheme;
        req.cfgitem[i].to_code_scheme = rgbsci[i].toCodeScheme;
        req.cfgitem[i].selected = rgbsci[i].selected;
    }

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildSmsBroadcastActivation(int bcst_act)
{
    sit_sms_act_bcst_sms_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_ACT_BCST_SMS, length);
    req.bcst_act = bcst_act;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildGetStoredSmsCount(int sim_id)
{
    sit_sms_get_stored_sms_count_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_STORED_SMS_COUNT, length);

    if (sim_id == 0x00) {
        req.sim_id= STORED_SIM;
    }
    else {
        req.sim_id= STORED_RUIM;
    }

    return new ModemData((char *)&req, length);
}

#ifdef SUPPORT_CDMA
ModemData *ProtocolSmsBuilder::BuildSendCdmaSms(const char *msg, int msgLen)
{
    if (msg == NULL || msgLen <= 0) {
        RilLogE("Wrong message data!!!");
        return NULL;
    }

    if (msgLen > MAX_CDMA_SMS_MSG_SIZE) {
        RilLogE("Message length = %d, MAX_CDMA_SMS_MSG_SIZE = %d", msgLen, MAX_CDMA_SMS_MSG_SIZE);
        return NULL;
    }

    sit_sms_cdma_send_sms_req req;
    int length = sizeof(req.hdr) + sizeof(req.msg_len) + (msgLen * sizeof(BYTE));
    InitRequestHeader(&req.hdr, SIT_CDMA_SEND_SMS, length);
    req.msg_len = msgLen;
    memcpy(req.msg, msg, msgLen);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildSendCdmaSmsAck(int tpid, int errClass, int errCode)
{
    sit_sms_cdma_send_sms_ack_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_CDMA_SEND_SMS_ACK, length);
    req.msg_tpid = tpid;
    req.error_class = errClass;
    req.error_code = errCode;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildWriteCdmaSmsToRuim(int status, const char *msg, int msgLen)
{
    if (msg == NULL || msgLen <= 0) {
        RilLogE("Wrong message data!!!");
        return NULL;
    }

    if (msgLen > MAX_CDMA_SMS_RUIM_MSG_SIZE) {
        RilLogE("Message length = %d, MAX_CDMA_SMS_RUIM_MSG_SIZE = %d", msgLen,
                MAX_CDMA_SMS_RUIM_MSG_SIZE);
        return NULL;
    }

    sit_sms_cdma_write_sms_to_ruim_req req;
    int length = sizeof(req.hdr) + sizeof(req.status) + sizeof(req.msg_len) + (msgLen * sizeof(BYTE));
    InitRequestHeader(&req.hdr, SIT_CDMA_WRITE_SMS_TO_RUIM, length);
    switch(status) {
    case RIL_RUIM_STATUS_RECEIVED_UNREAD:
        req.status = SIT_RUIM_STATUS_RECEIVED_UNREAD;
        break;
    case RIL_RUIM_STATUS_RECEIVED_READ:
        req.status = SIT_RUIM_STATUS_RECEIVED_READ;
        break;
    case RIL_RUIM_STATUS_STORED_UNSENT:
        req.status = SIT_RUIM_STATUS_STORED_UNSENT;
        break;
    case RIL_RUIM_STATUS_STORED_SENT:
        req.status = SIT_RUIM_STATUS_STORED_SENT;
        break;
    default:
        req.status = SIT_RUIM_STATUS_RECEIVED_UNREAD;
        RilLogE("SMS status(%d) is wrong!!! : set default(unread) value", status);
        break;
    }
    req.msg_len = msgLen;
    memcpy(req.msg, msg, msgLen);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildDeleteCdmaSmsOnRuim(int index)
{
    sit_sms_cdma_delete_sms_on_ruim_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_CDMA_DELETE_SMS_ON_RUIM, length);
    req.index = index;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildGetCdmaBroadcastSmsConfig()
{
    sit_sms_cdma_get_bcst_sms_cfg_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_CDMA_GET_BCST_SMS_CFG, length);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildSetCdmaBroadcastSmsConfig(
        const RIL_CDMA_BroadcastSmsConfigInfo *rcbsci, int num)
{
    if (rcbsci == NULL || num <= 0) {
        RilLogE("Wrong Broadcast SMS Configs Info!!!");
        return NULL;
    }

    if (num > MAX_CDMA_BCST_INFO_NUM) {
        RilLogW("Broadcast SMS Configs Number = %d, MAX_CDMA_BCST_INFO_NUM = %d", num,
                MAX_CDMA_BCST_INFO_NUM);
        num = MAX_CDMA_BCST_INFO_NUM;
    }

    sit_sms_cdma_set_bcst_sms_cfg_req req;
    int length = sizeof(req.hdr) + sizeof(req.bcst_info_num) +
        (num * sizeof(sit_sms_cdma_bcst_sms_cfg_item));
    InitRequestHeader(&req.hdr, SIT_CDMA_SET_BCST_SMS_CFG, length);
    req.bcst_info_num = num;
    for (int i = 0; i < num; i++) {
        req.cfgitem[i].svc_category = rcbsci[i].service_category;
        req.cfgitem[i].language = rcbsci[i].language;
    }

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolSmsBuilder::BuildCdmaSmsBroadcastActivation(int act)
{
    sit_sms_cdma_act_bcst_sms_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_CDMA_ACT_BCST_SMS, length);

    switch(act) {
    case RIL_SMS_CDMA_BCST_ACT_ACTIVATE:
        req.bcst_act = SIT_SMS_CDMA_BCST_ACT_ACTIVATE;
        break;
    case RIL_SMS_CDMA_BCST_ACT_DEACTIVATE:
        req.bcst_act = SIT_SMS_CDMA_BCST_ACT_DEACTIVATE;
        break;
    default:
        req.bcst_act = RIL_SMS_CDMA_BCST_ACT_DEACTIVATE;
        RilLogE("Undefined activation code(%d)!!!: set default(turn off) value", act);
        break;
    }

    return new ModemData((char *)&req, length);
}
#endif // SUPPORT_CDMA
