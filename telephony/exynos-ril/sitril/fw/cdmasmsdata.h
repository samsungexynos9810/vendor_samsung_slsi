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
 * @file    cdmasmsdata.h
 *
 * @brief   Definition of CDMA SMS data classes used in RIL.
 *
 * @author  Taesu Lee (taesu82.lee@samsung.com)
 *
 * @version Unspecipied.
 */

#ifndef __CDMA_SMS_DATA_H__
#define __CDMA_SMS_DATA_H__

#include "requestdata.h"

// Transport Layer Message Types(See 3GPP2 C.S0015, table 3.4-1)
#define TRANSPORT_LAYER_MESSAGE_TYPE_POINT_TO_POINT 0x00
#define TRANSPORT_LAYER_MESSAGE_TYPE_BROADCAST      0x01
#define TRANSPORT_LAYER_MESSAGE_TYPE_ACKNOWLEDGE    0x02

// Parameter Identifiers(See 3GPP2 C.S0015, table 3.4.3-1)
#define PARAMETER_TELESERVICE_IDENTIFIER        0x00
#define PARAMETER_SERVICE_CATEGORY              0x01
#define PARAMETER_ORIGINATING_ADDRESS           0x02
#define PARAMETER_ORIGINATING_SUBADDRESS        0x03
#define PARAMETER_DESTINATION_ADDRESS           0x04
#define PARAMETER_DESTINATION_SUBADDRESS        0x05
#define PARAMETER_BEARER_REPLY_OPTION           0x06
#define PARAMETER_CAUSE_CODES                   0x07
#define PARAMETER_BEARER_DATA                   0x08

// Parameter Length
#define PARAMETER_LENGTH_TELESERVICE_IDENTIFIER    0x02
#define PARAMETER_LENGTH_SERVICE_CATEGORY          0x02
#define PARAMETER_LENGTH_BEARER_REPLY_OPTION       0x01

// Teleservice Identifier

/**
 * The following are defined as extensions to the standard teleservices
 */
// Voice mail notification through Message Waiting Indication in CDMA mode or Analog mode.
// Defined in 3GPP2 C.S-0005, 3.7.5.6, an Info Record containing an 8-bit number with the
// number of messages waiting, it's used by some CDMA carriers for a voice mail count.
#define TELESERVICE_IDENTIFIER_MWI  0x40000

// Messge types for CDMA SMS messages(See 3GPP2 C.S0015, table 4.5.1-1)
#define MESSAGE_TYPE_DELIVER        0x01
#define MESSAGE_TYPE_SUBMIT         0x02
#define MESSAGE_TYPE_CANCELLATION   0x03
#define MESSAGE_TYPE_DELIVERY_ACK   0x04
#define MESSAGE_TYPE_USER_ACK       0x05
#define MESSAGE_TYPE_READ_ACK       0x06
#define MESSAGE_TYPE_DELIVER_REPORT 0x07
#define MESSAGE_TYPE_SUBMIT_REPORT  0x08

// Bearer Data Subparameter Identifiers(See 3GPP2 C.S0015, table 4.5-1)
#define SUBPARAM_MESSAGE_IDENTIFIER 0x00

// Error Class(See 3GPP2 C.S0015, 3.4.3.6 Cause Codes)
#define ERROR_CLASS_NO_ERROR        0
#define ERROR_CLASS_RESERVED        1
#define ERROR_CLASS_TEMPORARY_ERROR 2
#define ERROR_CLASS_PERMANENT_ERROR 3

/**
 * CMDA SMS Class
 */

class CCdmaSmsMessage
{
    private:
        BYTE *m_pbMsg;      //CDMA SMS Transport Layer Message pointer
        UINT16 m_uMsgLen;    //Length of CDMA SMS Transport Layer Message
        RIL_CDMA_SMS_Message *m_ptRcsm;

    public:
        CCdmaSmsMessage(BYTE *msg, INT16 len);
        CCdmaSmsMessage(RIL_CDMA_SMS_Message *rcsm);
        CCdmaSmsMessage(const CCdmaSmsMessage& csm);
        virtual ~CCdmaSmsMessage();

        const BYTE* GetMessage() { return m_pbMsg; }
        UINT16 GetMessageLength() { return m_uMsgLen; }
        const RIL_CDMA_SMS_Message* GetRilCdmaSmsMsg() { return m_ptRcsm; }

    private:
        INT32 ConstructTransportLayerMsg();
        INT32 ConstructRilCdmaSmsMsg();
        const BYTE* GetSubparameterDataFromBearerData(BYTE subparam, UINT8 *len);
};

class CdmaSmsRequestData : public RequestData
{
    protected:
        CCdmaSmsMessage *m_pCCsm;

    public:
        CdmaSmsRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
        virtual ~CdmaSmsRequestData();
        virtual INT32 encode(char *data, unsigned int datalen);

        const BYTE* GetMessage();
        UINT16 GetMessageLength();
};

class CdmaSmsAckRequestData : public RequestData
{
    private:
        UINT8 m_uErrClass;
        UINT8 m_uErrCode;
    public:
        CdmaSmsAckRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
        virtual ~CdmaSmsAckRequestData() {};
        virtual INT32 encode(char *data, unsigned int datalen);

        UINT8 GetErrorClass() { return m_uErrClass; }
        UINT8 GetErrorCode() { return m_uErrCode; }
};

class CdmaSmsWriteToRuimRequestData : public CdmaSmsRequestData
{
    private:
        UINT8 m_uStatus;

    public:
        CdmaSmsWriteToRuimRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
        virtual ~CdmaSmsWriteToRuimRequestData() {}
        virtual INT32 encode(char *data, unsigned int datalen);

        BYTE GetStatus() { return m_uStatus; }
};

class CCdmaBroadcastSmsConfigs
{
    private:
        UINT8 m_uNum;
        RIL_CDMA_BroadcastSmsConfigInfo *m_ptRcbsci;
        RIL_CDMA_BroadcastSmsConfigInfo **m_ptRcbsciPtrs;

    public:
        CCdmaBroadcastSmsConfigs(RIL_CDMA_BroadcastSmsConfigInfo *rcbsci, UINT8 num);
        CCdmaBroadcastSmsConfigs(const CCdmaBroadcastSmsConfigs& cbsc);
        virtual ~CCdmaBroadcastSmsConfigs();

        UINT8 GetConfigsNumber() { return m_uNum; }
        const RIL_CDMA_BroadcastSmsConfigInfo* GetConfigsInfo() { return m_ptRcbsci; }
        RIL_CDMA_BroadcastSmsConfigInfo** GetConfigsInfoPointers() { return m_ptRcbsciPtrs; }
};

class CdmaBroadcastSmsConfigsRequestData : public RequestData
{
    private:
        CCdmaBroadcastSmsConfigs *m_pCCbcsc;

    public:
        CdmaBroadcastSmsConfigsRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
        virtual ~CdmaBroadcastSmsConfigsRequestData();
        virtual INT32 encode(char *data, unsigned int  datalen);

        UINT8 GetConfigsNumber();
        const RIL_CDMA_BroadcastSmsConfigInfo* GetConfigsInfo();
};

#endif /*__CDMA_SMS_DATA_H__*/
