/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/**
 * @file    cdmasmsdata.cpp
 *
 * @brief   Definition of CDMA SMS data classes used in RIL.
 *
 * @author  Taesu Lee (taesu82.lee@samsung.com)
 *
 * @version Unspecipied.
 */

#include "cdmasmsdata.h"
#include "util.h"
#include "rillog.h"
#include "bitwiseiostream.h"
#include "systemproperty.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SMS, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define PROPERTY_NO_BEARER_REPLY_OPTION    "persist.vendor.radio.cdma.no_brr_reply"

CCdmaSmsMessage::CCdmaSmsMessage(BYTE *msg, INT16 len)
    :m_pbMsg(NULL), m_uMsgLen(0), m_ptRcsm(NULL)
{
    if (msg == NULL || len == 0) {
        RilLogE("Fail to create CCdmaSmsMessage Class from Transport Layer Message!!!");
    } else {
        if (len > MAX_CDMA_SMS_MSG_SIZE) {
            RilLogW("Transport Layer Message size(%d) is longer than MAX_CDMA_SMS_MSG_SIZE(%d).",\
                    len, MAX_CDMA_SMS_MSG_SIZE);
            m_uMsgLen = MAX_CDMA_SMS_MSG_SIZE;
        } else {
            m_uMsgLen = len;
        }
        m_pbMsg = new BYTE[m_uMsgLen];
        memcpy(m_pbMsg, msg, m_uMsgLen);

        m_ptRcsm = new RIL_CDMA_SMS_Message;
        memset(m_ptRcsm, 0, sizeof(RIL_CDMA_SMS_Message));
        if (ConstructRilCdmaSmsMsg() < 0) {
            RilLogE("Fail to construct RIL_CDMA_SMS_Message from Transport Layer Message.");
        }
    }
}

CCdmaSmsMessage::CCdmaSmsMessage(RIL_CDMA_SMS_Message* rcsm)
    :m_pbMsg(NULL), m_uMsgLen(0), m_ptRcsm(NULL)
{
    if (rcsm == NULL) {
        RilLogE("Fail to create CCdmaSmsMessage Class from CDMA SMS Message!!!");
    } else {
        m_ptRcsm = new RIL_CDMA_SMS_Message;
        memcpy(m_ptRcsm, rcsm, sizeof(RIL_CDMA_SMS_Message));

        if (ConstructTransportLayerMsg() < 0) {
            RilLogE("Fail to construct Transport Layer Message from RIL_CDMA_SMS_Message.");
        }
    }
}

CCdmaSmsMessage::CCdmaSmsMessage(const CCdmaSmsMessage& csm)
    :m_pbMsg(NULL), m_uMsgLen(0), m_ptRcsm(NULL)
{
    if ((csm.m_uMsgLen > 0) && csm.m_ptRcsm) {
        m_uMsgLen = csm.m_uMsgLen;
        m_pbMsg = new BYTE[m_uMsgLen];
        memcpy(m_pbMsg, csm.m_pbMsg, m_uMsgLen);
        m_ptRcsm = new RIL_CDMA_SMS_Message;
        memcpy(m_ptRcsm, csm.m_ptRcsm, sizeof(RIL_CDMA_SMS_Message));
    } else{
        RilLogE("Fail to create CCdmaSmsMessage Class!!!");
    }
}

CCdmaSmsMessage::~CCdmaSmsMessage()
{
    if (m_pbMsg) {
        delete []m_pbMsg;
    }
    if (m_ptRcsm) {
        delete m_ptRcsm;
    }
}

INT32 CCdmaSmsMessage::ConstructTransportLayerMsg()
{
    // Construct Transport Layer Message from RIL_CDMA_SMS_Message.
    CBitwiseOutputStream *bwOutData;
    BYTE msg[MAX_CDMA_SMS_MSG_SIZE] ={0};
    UINT16 index = 0;

    BYTE messageType = MESSAGE_TYPE_SUBMIT;

    UINT8 subParamLen = 0;
    const BYTE *subParam;

    UINT8 digitLen;
    UINT8 addressLen;

    UINT8 bearerLen;

    UINT16 u16;

    // RIL_CDMA_SMS_ADDRESS_MAX + DIGIT_MODE + NUMBER_MODE/TYPE/PLAN + NUN_FIELDS + RESERVED
    BYTE address[RIL_CDMA_SMS_ADDRESS_MAX + 3] ={0};
    // RIL_CDMA_SMS_SUBADDRESS_MAX + TYPE + ODD + NUN_FIELDS + RESERVED
    BYTE subAddress[RIL_CDMA_SMS_SUBADDRESS_MAX + 2] ={0};

    subParam = GetSubparameterDataFromBearerData(SUBPARAM_MESSAGE_IDENTIFIER, &subParamLen);
    if (subParam == NULL || subParamLen == 0) {
        RilLogW("Message Identifier doesn't exist in Bearer Data!!!");
    } else {
        messageType = (*subParam >> 4)  & 0x0F;
        // ConstructTransportLayerMsg() is used for Deliver/Submit SMS only.
        // Submit SMS for sending SMS and Deliver/Submit SMS for writing to RUIM.
        if (messageType != MESSAGE_TYPE_DELIVER && messageType != MESSAGE_TYPE_SUBMIT) {
            RilLogW("Message type(%02X) is neither MESSAGE_TYPE_DELIVER nor MESSAGE_TYPE_SUBMIT.", messageType);
            messageType = MESSAGE_TYPE_SUBMIT;
        }
    }

    // SMS message type
    msg[index++] = TRANSPORT_LAYER_MESSAGE_TYPE_POINT_TO_POINT;

    // Teleservice ID
    msg[index++] = PARAMETER_TELESERVICE_IDENTIFIER;
    msg[index++] = PARAMETER_LENGTH_TELESERVICE_IDENTIFIER;
    u16 = (UINT16)m_ptRcsm->uTeleserviceID;
    msg[index++] = (BYTE)((u16 >> 8) & 0x00FF);
    msg[index++] = (BYTE)(u16 & 0x00FF);

    // Service category
    if (m_ptRcsm->uServicecategory) {
        msg[index++] = PARAMETER_SERVICE_CATEGORY;
        msg[index++] = PARAMETER_LENGTH_SERVICE_CATEGORY;
        u16 = (UINT16)m_ptRcsm->uServicecategory;
        msg[index++] = (BYTE)((u16 >> 8) & 0x00FF);
        msg[index++] = (BYTE)(u16 & 0x00FF);
    }

    // Destination/Originating address(See 3GPP2 C.S0015, 3.4.3.3)
    bwOutData = new CBitwiseOutputStream(address, sizeof(address));
    digitLen = MIN(m_ptRcsm->sAddress.number_of_digits, RIL_CDMA_SMS_ADDRESS_MAX);

    bwOutData->Write(1, m_ptRcsm->sAddress.digit_mode);
    bwOutData->Write(1, m_ptRcsm->sAddress.number_mode);
    if (m_ptRcsm->sAddress.digit_mode == RIL_CDMA_SMS_DIGIT_MODE_8_BIT) {
        bwOutData->Write(3, m_ptRcsm->sAddress.number_type);
        if (m_ptRcsm->sAddress.number_mode == RIL_CDMA_SMS_NUMBER_MODE_NOT_DATA_NETWORK) {
            bwOutData->Write(4, m_ptRcsm->sAddress.number_plan);
        }
    }
    bwOutData->Write(8, digitLen);

    if (m_ptRcsm->sAddress.digit_mode == RIL_CDMA_SMS_DIGIT_MODE_8_BIT) {
        for (UINT8 i = 0; i < digitLen; i++) {
            bwOutData->Write(8, m_ptRcsm->sAddress.digits[i]);
        }
    } else { // RIL_CDMA_SMS_DIGIT_MODE_4_BIT
        for (UINT8 i = 0; i < digitLen; i++) {
            bwOutData->Write(4, m_ptRcsm->sAddress.digits[i]);
        }
    }
    addressLen = bwOutData->GetArrayIndex() + 1;
    msg[index++] = messageType == MESSAGE_TYPE_SUBMIT ? PARAMETER_DESTINATION_ADDRESS:
        PARAMETER_ORIGINATING_ADDRESS;
    msg[index++] = addressLen;
    memcpy(&msg[index], address, addressLen);

    index += addressLen;
    delete bwOutData;

    // Destination/Originating subaddress(See 3GPP2 C.S0015, 3.4.3.4)
    // - Not supported for MO SMS in AOSP
    digitLen = MIN(m_ptRcsm->sSubAddress.number_of_digits, RIL_CDMA_SMS_SUBADDRESS_MAX);
    if (digitLen != 0) {
        bwOutData = new CBitwiseOutputStream(subAddress, sizeof(subAddress));

        bwOutData->Write(3, m_ptRcsm->sSubAddress.subaddressType);
        bwOutData->Write(1, m_ptRcsm->sSubAddress.odd);
        bwOutData->Write(8, digitLen);

        for (UINT8 i = 0; i < digitLen; i++) {
            bwOutData->Write(8, m_ptRcsm->sSubAddress.digits[i]);
        }
        addressLen = bwOutData->GetArrayIndex() + 1;
        msg[index++] = messageType == MESSAGE_TYPE_SUBMIT ? PARAMETER_DESTINATION_SUBADDRESS:
            PARAMETER_ORIGINATING_SUBADDRESS;
        msg[index++] = addressLen;
        memcpy(&msg[index], subAddress, addressLen);

        index += addressLen;
        delete bwOutData;
    }

    // Bearer Reply Option(Optional)
    if ("true" != SystemProperty::Get(PROPERTY_NO_BEARER_REPLY_OPTION, "false")) {
        msg[index++] = PARAMETER_BEARER_REPLY_OPTION;
        msg[index++] = PARAMETER_LENGTH_BEARER_REPLY_OPTION;
        msg[index++] = 0x00; // Will be updated by CP before sending.
    }

    // Bearer data
    bearerLen = MIN(m_ptRcsm->uBearerDataLen, RIL_CDMA_SMS_BEARER_DATA_MAX);
    msg[index++] = PARAMETER_BEARER_DATA;
    msg[index++] = bearerLen;
    memcpy(&msg[index], (BYTE *)m_ptRcsm->aBearerData, bearerLen);

    m_uMsgLen = index + bearerLen;
    m_pbMsg = new BYTE[m_uMsgLen];
    memcpy(m_pbMsg, msg, m_uMsgLen);

    return 0;
}

const BYTE* CCdmaSmsMessage::GetSubparameterDataFromBearerData(BYTE subparam, UINT8 *len)
{
    UINT8 bearerLen;
    bearerLen = MIN(m_ptRcsm->uBearerDataLen, RIL_CDMA_SMS_BEARER_DATA_MAX);
    BYTE subParameterId;
    BYTE subParameterLen;

    UINT16 index = 0;

    while (index + 2 < bearerLen) {
        subParameterId = m_ptRcsm->aBearerData[index++];
        subParameterLen = m_ptRcsm->aBearerData[index++];
        if (subParameterId == subparam) {
            *len = subParameterLen;
            return &m_ptRcsm->aBearerData[index];
        }
        index += subParameterLen;
    }
    return NULL;
}

INT32 CCdmaSmsMessage::ConstructRilCdmaSmsMsg()
{
    // Construct RIL_CDMA_SMS_Message form Transport Layer Message.
    BYTE parameterId;
    BYTE parameterLen;

    UINT16 index = 0;

    CBitwiseInputStream *bwInData;

    if (*m_pbMsg != TRANSPORT_LAYER_MESSAGE_TYPE_POINT_TO_POINT &&
                *m_pbMsg != TRANSPORT_LAYER_MESSAGE_TYPE_BROADCAST) {
        RilLogE("Transport Layer Message type is neither POINT_TO_POINT nor BROADCAST!!!");
        return -1;
    }
    index++;

    do {
        if (index >= m_uMsgLen) break;
        parameterId = *(m_pbMsg + index++);
        if (index >= m_uMsgLen) {
            RilLogW("No Parameter Length for Parameter ID(0x%02X).", parameterId);
            break;
        }
        parameterLen = *(m_pbMsg + index++);
        if (parameterLen == 0) {
            RilLogW("Skip Parameter ID(0x%02X). Parameter Length is 0.", parameterId);
            continue;
        }
        if (index + parameterLen > m_uMsgLen) {
            RilLogW("Parameter Length(%d) for Parameter ID(0x%02X) is out of bounds.",
                    parameterLen, parameterId);
            break;
        }

        switch (parameterId) {
            case PARAMETER_TELESERVICE_IDENTIFIER:
                m_ptRcsm->uTeleserviceID = (*(m_pbMsg + index) << 8) & 0xFF00;
                m_ptRcsm->uTeleserviceID |= *(m_pbMsg + index + 1);
                break;
            case PARAMETER_SERVICE_CATEGORY:
                m_ptRcsm->bIsServicePresent = true;
                m_ptRcsm->uServicecategory = (*(m_pbMsg + index) << 8) & 0xFF00;
                m_ptRcsm->uServicecategory |= *(m_pbMsg + index + 1);
                break;
            case PARAMETER_DESTINATION_ADDRESS: // No need for MT case.
            case PARAMETER_ORIGINATING_ADDRESS:
                bwInData = new CBitwiseInputStream(m_pbMsg + index, parameterLen);
                m_ptRcsm->sAddress.digit_mode = (RIL_CDMA_SMS_DigitMode)bwInData->Read(1);
                m_ptRcsm->sAddress.number_mode = (RIL_CDMA_SMS_NumberMode)bwInData->Read(1);
                if (m_ptRcsm->sAddress.digit_mode == RIL_CDMA_SMS_DIGIT_MODE_8_BIT) {
                    m_ptRcsm->sAddress.number_type = (RIL_CDMA_SMS_NumberType)bwInData->Read(3);
                    if (m_ptRcsm->sAddress.number_mode == RIL_CDMA_SMS_NUMBER_MODE_NOT_DATA_NETWORK) {
                        m_ptRcsm->sAddress.number_plan = (RIL_CDMA_SMS_NumberPlan)bwInData->Read(4);
                    }
                }
                m_ptRcsm->sAddress.number_of_digits = bwInData->Read(8);

                if (m_ptRcsm->sAddress.digit_mode == RIL_CDMA_SMS_DIGIT_MODE_8_BIT) {
                    for (int i = 0; i < m_ptRcsm->sAddress.number_of_digits ; i++) {
                        m_ptRcsm->sAddress.digits[i] = bwInData->Read(8);
                    }
                } else { // RIL_CDMA_SMS_DIGIT_MODE_4_BIT
                    for (int i = 0; i < m_ptRcsm->sAddress.number_of_digits ; i++) {
                        m_ptRcsm->sAddress.digits[i] = bwInData->Read(4);
                    }
                }
                delete bwInData;
                break;
            case PARAMETER_DESTINATION_SUBADDRESS: // No need for MT case.
            case PARAMETER_ORIGINATING_SUBADDRESS:
                bwInData = new CBitwiseInputStream(m_pbMsg + index, parameterLen);
                m_ptRcsm->sSubAddress.subaddressType = (RIL_CDMA_SMS_SubaddressType)bwInData->Read(3);
                m_ptRcsm->sSubAddress.odd = bwInData->Read(1);
                m_ptRcsm->sSubAddress.number_of_digits = bwInData->Read(8);
                for (int i = 0; i < m_ptRcsm->sSubAddress.number_of_digits ; i++) {
                    m_ptRcsm->sSubAddress.digits[i] = bwInData->Read(8);
                }
                delete bwInData;
                break;
            case PARAMETER_BEARER_REPLY_OPTION:
                RilLogW("BEARER REPLY OPTION is not supported on AOSP.");
                break;
            case PARAMETER_CAUSE_CODES:
                RilLogW("CAUSE CODE is not supported on AOSP.");
                break;
            case PARAMETER_BEARER_DATA:
                m_ptRcsm->uBearerDataLen = parameterLen;
                memcpy(m_ptRcsm->aBearerData, m_pbMsg + index, parameterLen);
                break;
            default:
                RilLogW("Undefined Parameter ID(0x%02X) is found.", parameterId);
        }
        index += parameterLen;
    } while(1);

    return 0;
}

CdmaSmsRequestData::CdmaSmsRequestData(const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq, tok, type), m_pCCsm(NULL)
{
}

CdmaSmsRequestData::~CdmaSmsRequestData()
{
    if (m_pCCsm) {
        delete m_pCCsm;
    }
}

const BYTE* CdmaSmsRequestData::GetMessage()
{
    if (m_pCCsm) {
        return m_pCCsm->GetMessage();
    } else {
        return NULL;
    }
}

UINT16 CdmaSmsRequestData::GetMessageLength()
{
    if (m_pCCsm) {
        return m_pCCsm->GetMessageLength();
    } else {
        return 0;
    }
}

INT32 CdmaSmsRequestData::encode(char *data, unsigned int datalen)
{
    if (data == NULL || datalen == 0) {
        return -1;
    }
    m_pCCsm = new CCdmaSmsMessage((RIL_CDMA_SMS_Message *)data);
    return 0;
}

CdmaSmsAckRequestData::CdmaSmsAckRequestData(const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq, tok, type), m_uErrClass(0), m_uErrCode(0)
{
}

INT32 CdmaSmsAckRequestData::encode(char *data, unsigned int datalen)
{
    if (data == NULL || datalen == 0) {
        return -1;
    }
    m_uErrClass = *data;
    m_uErrCode = *(data + 1);
    return 0;
}

CdmaSmsWriteToRuimRequestData::CdmaSmsWriteToRuimRequestData
    (const int nReq, const Token tok, const ReqType type)
    :CdmaSmsRequestData(nReq, tok, type), m_uStatus(0)
{
}

INT32 CdmaSmsWriteToRuimRequestData::encode(char *data, unsigned int datalen)
{
    if (data == NULL || datalen == 0) {
        return -1;
    }
    RIL_CDMA_SMS_WriteArgs *rcsw = (RIL_CDMA_SMS_WriteArgs *)data;
    switch (rcsw->status)
    {
        // See 3GPP2 C.S0023, 3.4.27
        case 0x00: m_uStatus = RIL_RUIM_STATUS_RECEIVED_UNREAD; break;
        case 0x01: m_uStatus = RIL_RUIM_STATUS_RECEIVED_READ; break;
        case 0x02: m_uStatus = RIL_RUIM_STATUS_STORED_UNSENT; break;
        case 0x03: m_uStatus = RIL_RUIM_STATUS_STORED_SENT; break;
        default:
            m_uStatus = RIL_RUIM_STATUS_RECEIVED_UNREAD;
            RilLogE("SMS status(%d) is wrong!!!", rcsw->status);
            break;
    }
    m_pCCsm = new CCdmaSmsMessage((RIL_CDMA_SMS_Message *)&rcsw->message);
    return 0;
}

CCdmaBroadcastSmsConfigs::CCdmaBroadcastSmsConfigs(RIL_CDMA_BroadcastSmsConfigInfo* rcbsci, UINT8 num)
    :m_uNum(0), m_ptRcbsci(NULL), m_ptRcbsciPtrs(NULL)
{
    if (rcbsci == NULL) {
        RilLogE("Fail to create CCdmaBroadcastSmsConfigs Class!!!");
    } else {
        if (num > MAX_CDMA_BCST_INFO_NUM) {
            RilLogW("Number of BCST(%d) is larger than MAX_CDMA_BCST_INFO_NUM(%d).",\
                    num, MAX_CDMA_BCST_INFO_NUM);
            m_uNum = MAX_CDMA_BCST_INFO_NUM;
        } else {
            m_uNum = num;
        }
        m_ptRcbsci = new RIL_CDMA_BroadcastSmsConfigInfo[m_uNum];
        memcpy(m_ptRcbsci, rcbsci, m_uNum * sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
        m_ptRcbsciPtrs = new RIL_CDMA_BroadcastSmsConfigInfo *[m_uNum];
        for (int i = 0; i < m_uNum; i++) {
            m_ptRcbsciPtrs[i] = &m_ptRcbsci[i];
        }
    }
}

CCdmaBroadcastSmsConfigs::CCdmaBroadcastSmsConfigs(const CCdmaBroadcastSmsConfigs& cbsc)
    :m_uNum(0), m_ptRcbsci(NULL), m_ptRcbsciPtrs(NULL)
{
    if ((cbsc.m_uNum > 0) && cbsc.m_ptRcbsci) {
        m_uNum = cbsc.m_uNum;
        m_ptRcbsci = new RIL_CDMA_BroadcastSmsConfigInfo[m_uNum];
        memcpy(m_ptRcbsci, cbsc.m_ptRcbsci, m_uNum * sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
        m_ptRcbsciPtrs = new RIL_CDMA_BroadcastSmsConfigInfo *[m_uNum];
        for (int i = 0; i < m_uNum; i++) {
            m_ptRcbsciPtrs[i] = &m_ptRcbsci[i];
        }
    } else{
        RilLogE("Fail to create CCdmaBroadcastSmsConfigs Class!!!");
    }
}

CCdmaBroadcastSmsConfigs::~CCdmaBroadcastSmsConfigs()
{
    if (m_ptRcbsci) {
        delete[] m_ptRcbsci;
    }
    if (m_ptRcbsciPtrs) {
        delete[] m_ptRcbsciPtrs;
    }
}

CdmaBroadcastSmsConfigsRequestData::CdmaBroadcastSmsConfigsRequestData
    (const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq, tok, type), m_pCCbcsc(NULL)
{
}

CdmaBroadcastSmsConfigsRequestData::~CdmaBroadcastSmsConfigsRequestData()
{
    if (m_pCCbcsc) {
        delete m_pCCbcsc;
    }
}

INT32 CdmaBroadcastSmsConfigsRequestData::encode(char *data, unsigned int datalen)
{
    if (data == NULL || datalen == 0) {
        return -1;
    }
    int num = datalen/sizeof(RIL_CDMA_BroadcastSmsConfigInfo *);
    RIL_CDMA_BroadcastSmsConfigInfo rcbsci[num];
    RIL_CDMA_BroadcastSmsConfigInfo **rcbsciPtrs = (RIL_CDMA_BroadcastSmsConfigInfo **)data;
    for (int i = 0; i < num; i++) {
        if (rcbsciPtrs[i] == NULL) {
            return -1;
        }
        memcpy(&rcbsci[i], rcbsciPtrs[i], sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
    }
    m_pCCbcsc = new CCdmaBroadcastSmsConfigs(rcbsci, num);
    return 0;
}

UINT8 CdmaBroadcastSmsConfigsRequestData::GetConfigsNumber()
{
    if (m_pCCbcsc) {
        return m_pCCbcsc->GetConfigsNumber();
    } else {
        return 0;
    }
}

const RIL_CDMA_BroadcastSmsConfigInfo* CdmaBroadcastSmsConfigsRequestData::GetConfigsInfo()
{
    if (m_pCCbcsc) {
        return m_pCCbcsc->GetConfigsInfo();
    } else {
        return NULL;
    }
}
