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
 * @file    smsdata.h
 *
 * @brief   Definition of SMS data classes used in RIL.
 *
 * @author  HyunJoon Kang (arkade.kang@samsung.com)
 *          RyongGyoung Kwon (reika.kwon@samsung.com)
 *
 * @version Unspecipied.
 */

#ifndef __SMS_DATA_H__
#define __SMS_DATA_H__

#include "requestdata.h"


#define SMS_DBG false

/**
 * PDU (Prototol Data Unit)
 */
enum MessageClass {
    MESSAGE_CLASS_0,
    MESSAGE_CLASS_1,
    MESSAGE_CLASS_2,
    MESSAGE_CLASS_3
};

enum SmsFailResult{
    FAILED_RECEIPT = 0x00,
    SUCCESS_RECEIPT = 0x01,
};

enum SmsFailRPCause{
    RP_CAUSE_NOT_SPECIFIED = -1,
    RP_CAUSE_NONE_ERROR = 0x00,
    RP_CAUSE_UNASSIGNED_NUMBER = 0x01,
    RP_CAUSE_OPERATOR_DETERMINED_BARRING = 0x08,
    RP_CAUSE_CALL_BARRED = 0x0A,
    RP_CAUSE_RESERVRED = 0x0B,
    RP_CAUSE_SMS_TRANSFER_REJECTED = 0x15,
    RP_CAUSE_MEMORY_CAP_EXCEEDED = 0x16,
    RP_CAUSE_DESTINATION_OUT_OF_ORDER = 0x1B,
    RP_CAUSE_UNIDENTIFIED_SUBSCRIBER = 0x1C,
    RP_CAUSE_FACILITY_REJECTED = 0x1D,
    RP_CAUSE_UNKNOWN_SUBSCRIBER = 0x1E,
    RP_CAUSE_NETWORK_OUT_OF_ORDER = 0x26,
    RP_CAUSE_TEMPORARY_FAILURE = 0x29,
    RP_CAUSE_CONGESTION = 0x2A,
    RP_CAUSE_RESOURCES_UNAVAILABLE = 0x2F,
    RP_CAUSE_REQUESTED_FACILITY_NOT_SUBSCRIBED = 0x32,
    RP_CAUSE_REQUESTED_FACILITY_NOT_IMPLEMENTED = 0x45,
    RP_CAUSE_INVALID_SMS_TRANSFER_REFERENCE_VALUE = 0x51,
    RP_CAUSE_SEMANTICALLY_INCORRECT_MESSAGE = 0x5F,
    RP_CAUSE_INVALID_MANDATORY_INFO = 0x60,
    RP_CAUSE_MESSAGE_TYPE_NOT_IMPLEMENTED = 0x61,
    RP_CAUSE_NOT_COMPATIBLE_PROTOCOL = 0x62,
    RP_CAUSE_INFO_ELEMENT_NOT_IMPLEMENTED = 0x63,
    RP_CAUSE_PROTOCOL_ERROR = 0x6F,
    RP_CAUSE_INTERWORKING = 0x7F,
};

enum SmsFailTPCause{
    TP_CAUSE_NOT_SPECIFIED = -1,
    TP_CAUSE_TELE_INTERWORKING_NOT_SUPPORTED = 0x80,
    TP_CAUSE_SHORT_MESSAGE_TYPE_0_NOT_SUPPORTED = 0x81,
    TP_CAUSE_SHORT_MESSAGE_CANNOT_BE_REPLACED = 0x82,
    TP_CAUSE_UNSPECIFIED_PID_ERROR = 0x8F,
    TP_CAUSE_DCS_NOT_SUPPORTED = 0x90,
    TP_CAUSE_MESSAGE_CLASS_NOT_SUPPORTED = 0x91,
    TP_CAUSE_UNSPECIFIED_DCS_ERROR = 0x9F,
    TP_CAUSE_COMMAND_CANNOT_BE_ACTIONED = 0xA0,
    TP_CAUSE_COMMAND_UNSUPPORTED = 0xA1,
    TP_CAUSE_UNSPECIFIED_COMMAND_ERROR = 0xAF,
    TP_CAUSE_TPDU_NOT_SUPPORTED = 0xB0,
    TP_CAUSE_SC_BUSY = 0xC0,
    TP_CAUSE_NO_SC_SUBSCRIPTION = 0xC1,
    TP_CAUSE_SC_SYS_FAILURE = 0xC2,
    TP_CAUSE_INVALID_SME_ADDRESS = 0xC3,
    TP_CAUSE_DESTINATION_SME_BARRED = 0xC4,
    TP_CAUSE_SM_REJECTED_OR_DUPLICATE = 0xC5,
    TP_CAUSE_TP_VPF_NOT_SUPPORTED = 0xC6,
    TP_CAUSE_TP_VP_NOT_SUPPORTED = 0xC7,
    TP_CAUSE_SIM_SMS_STORAGE_FULL = 0xD0,
    TP_CAUSE_NO_SMS_STORAGE_CAP_IN_SIM = 0xD1,
    TP_CAUSE_MS_ERROR = 0xD2,
    TP_CAUSE_MEMORY_CAP_EXCEEDED = 0xD3,
    TP_CAUSE_SIM_APP_TOOLKIT_BUSY = 0xD4,
    TP_CAUSE_SIM_DATA_DOWNLOAD_ERROR = 0xD5,
    TP_CAUSE_UNSPECIFIED_ERROR = 0xFF,
};



class SmsMessage;

class Pdu
{
    public:
        char mData[MAX_GSM_SMS_TPDU_SIZE];           // PDU raw data
        int mLen;

    protected:
        char *mHex;

    public:
        Pdu();
        Pdu(char *data, int len);
        explicit Pdu(char *hexStr);
        Pdu(const Pdu &pdu);
        virtual ~Pdu();

        Pdu & operator=(const Pdu &pdu);

        char * ToHexString();
        void SetRawData(char *hexStr);
};

/**
 * SMS GSM Address
 */
class GsmSmsAddress
{
    public:
        char mAddr[MAX_GSM_SMS_SERVICE_CENTER_ADDR];
        int mLen;
        char *mString;

    public:
        bool operator==(const GsmSmsAddress & a) const;
        char * ToString();

    public:
        GsmSmsAddress();
        GsmSmsAddress(char *data, int len);
        virtual ~GsmSmsAddress();
};

/**
 * SMS center address
 */
class SmsCenter
{
    public:
        char mAddr[MAX_GSM_SMS_SERVICE_CENTER_ADDR];
        int mLen;
        int mType;

    protected:
        char *mHex;

    public:
        SmsCenter();
        explicit SmsCenter(SmsCenter *smsc);
        explicit SmsCenter(char *smsc);
        SmsCenter(char *smsc, int len);
        virtual ~SmsCenter();
        char * ToHexString();
        bool UseRilSmsc();
};

/*
 * GSM SM TPDU parser
 */
class PduParser
{
    private:
        Pdu *mPdu;
        int mCur;

        char *m_cPdu;
        int m_scalen;
        char m_sca[24];
        int m_smsclass;

    public:
        PduParser();
        explicit PduParser(Pdu *pdu);
        ~PduParser();

    public:
        int GetByte();
        long GetSCTimestamp();
        GsmSmsAddress * GetAddress();
        int GetScaLen(const void *pdu);
        char *GetSca(const void *pdu);
        int GetSmsClass(const void *pdu);
};

/**
 * SMS message
 */
class SmsMessage : public RequestData
{
    protected:
        char *mHex;

    public:
        Pdu *mPdu;
        int mSimIndex;

    public:
        // Network Type
        static const int NETWORK_TYPE_CDMA = 1;
        static const int NETWORK_TYPE_GSM = 2;

    public:
        SmsMessage(const int nReq, const Token tok, const ReqType type = REQ_FW);
        virtual ~SmsMessage();
        void Init();
        virtual int encode(char *data, unsigned int datalen);
};

class GsmSmsMessage : public SmsMessage
{
    public:
        SmsCenter *mSmsc;
        int mClass;                 // Message Class
        int mMessageRef;

    public:
        GsmSmsMessage(const int nReq, const Token tok, const ReqType type = REQ_FW);
        //GsmSmsMessage(SmsCenter *smsc, Pdu *pdu);
        GsmSmsMessage(GsmSmsMessage *msg);
        virtual ~GsmSmsMessage();
        void Init();
        virtual int encode(char *data, unsigned int datalen);

        char *GetRawByte();
        int GetLength();
        char * ToHexString();
        char * ToString();
};


/**
 * SMS acknowledge message
 */
class SmsAcknowledge : public RequestData
{
    public:
        int mResult;
        int mFailureCause;

        static const int SMS_ACK_SUCCESS = 1;
        static const int SMS_ACK_FAILED = 2;
    public:
        SmsAcknowledge();
        SmsAcknowledge(int result, int cause);
        SmsAcknowledge(const int nReq, const Token tok, const ReqType type = REQ_FW);
        virtual int encode(char *data, unsigned int datalen);
};

class SmsAcknowledgePdu : public RequestData
{
    public:
        int mResult;
        Pdu *mPdu;             // arkade: FIX ME

    public:
        SmsAcknowledgePdu();
        SmsAcknowledgePdu(const int nReq, const Token tok, const ReqType type = REQ_FW);
        virtual int encode(char *data, unsigned int datalen);
};

/**
 * SIM SMS message
 */
class SimSmsMessage : public GsmSmsMessage
{
    public:
        int mStatus;
        int mStorage;

    public:
        SimSmsMessage(const int nReq, const Token tok, const ReqType type = REQ_FW);
        //explicit SimSmsMessage(int index);
        //SimSmsMessage(int status,  Pdu *pdu, SmsCenter *smsc, bool class2Flag);

        virtual ~SimSmsMessage() {}
        virtual int encode(char *data, unsigned int datalen);
};

class SimSmsMessageResult
    {
    public:
        int mResult;
        int mIndex;

    public:
        SimSmsMessageResult();
        SimSmsMessageResult(int result, int index);
        virtual ~SimSmsMessageResult() {}
};

/**
 * SMS Storage
 */

class SmsStorage
{
    public:
        int mMemoryType;
        int mCurrentIndex;
        int mUsedCount;
        int mMaxCount;
        int mStatus;

    public:
        /* SMS storage */
        static const int MEMORY_PHONE_NV = 0x01;
        static const int MEMORY_SIM = 0x02;
        static const int MEMORY_ALL = 0x03;

    public:
        SmsStorage();
        explicit SmsStorage(int status);
        virtual ~SmsStorage() {}
};

/**
 * BroadcastSmsConfig Info.
 */
class BroadcastSmsConfigs
{
    private:
        UINT8 m_uNum;
        RIL_GSM_BroadcastSmsConfigInfo *m_ptRgbsci;
        RIL_GSM_BroadcastSmsConfigInfo **m_ptRgbsciPtrs;

    public:
        BroadcastSmsConfigs(RIL_GSM_BroadcastSmsConfigInfo *rgbsci, UINT8 num);
        BroadcastSmsConfigs(const BroadcastSmsConfigs& bsc);
        virtual ~BroadcastSmsConfigs();

        UINT8 GetConfigsNumber() { return m_uNum; }
        const RIL_GSM_BroadcastSmsConfigInfo* GetConfigsInfo() { return m_ptRgbsci; }
        RIL_GSM_BroadcastSmsConfigInfo** GetConfigsInfoPointers() { return m_ptRgbsciPtrs; }
};

class BroadcastSmsConfigsRequestData : public RequestData
{
    private:
        BroadcastSmsConfigs *m_pCBcsc;

    public:
        BroadcastSmsConfigsRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
        virtual ~BroadcastSmsConfigsRequestData();
        virtual INT32 encode(char *data, unsigned int  datalen);

        UINT8 GetConfigsNumber();
        const RIL_GSM_BroadcastSmsConfigInfo* GetConfigsInfo();
};
#endif
