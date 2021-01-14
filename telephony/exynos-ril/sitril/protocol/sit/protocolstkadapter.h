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
 * protocolstkadapter.h
 *
 *  Created on: 2014. 10. 6.
 *      Author: MOX
 */

#ifndef __PROTOCOL_STK_ADAPTER_H__

#include "protocoladapter.h"

class ProtocolStkEnvelopeCommandAdapter : public ProtocolRespAdapter {
public:
    ProtocolStkEnvelopeCommandAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }

private:
    int m_nEnvelopeCmdLength;
    BYTE *m_pEnvelopeCmd;

    virtual void Init();

public:
    int GetEnvelopeCmdLength() const { return m_nEnvelopeCmdLength; }
    BYTE *GetEnvelopeCommand() const { return m_pEnvelopeCmd; }
};

class ProtocolStkTerminalRspAdapter : public ProtocolRespAdapter {
public:
    ProtocolStkTerminalRspAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }

private:
    int nSW1;
    int nSW2;

    virtual void Init();

public:
    int GetSW1() const  { return nSW1; }
    int GetSW2() const  { return nSW2; }
};

class ProtocolStkEnvelopeStatusAdapter : public ProtocolRespAdapter {
public:
    ProtocolStkEnvelopeStatusAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }

private:
    int nSW1;
    int nSW2;
    int m_nEnvelopeRspLength;
    BYTE *m_pEnvelopeRsp;

    virtual void Init();

public:
    int GetSW1() const  { return nSW1; }
    int GetSW2() const  { return nSW2; }
    int GetEnvelopeRspLength() const { return m_nEnvelopeRspLength; }
    BYTE *GetEnvelopeResponse() const { return m_pEnvelopeRsp; }
};

class ProtocolStkProactiveCommandAdapter : public ProtocolRespAdapter {
public:
    ProtocolStkProactiveCommandAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }

private:
    virtual void Init();

    static const BYTE PROACTIVE_CMD_BER_TAG = 0xD0;
    static const BYTE CAT_CMPH_TAG_MASK = 0x7F;
    static const WORD CAT_EFID_ROOT_MASK = 0xFF00;
    static const WORD CAT_EFID_ROOT_PREFIX = 0x3F00;
    static const int MAX_EFID_COUNT = 255;

    int m_nProactiveCmdLength;
    BYTE *m_pProactiveCmd;

    int m_nCount;
    UINT m_arEFID[MAX_EFID_COUNT];
    int m_nAidLen;
    BYTE m_acAID[MAX_SIM_AID_LEN];        // RID 5byte + PIX 11 byte

public:
#pragma pack(1)
    typedef struct tagBerTLV {
        BYTE cTag;
        BYTE cLength;
        BYTE acData[0];
    } BER_TLV;

    // MOX: 2015.08.06 - STK Call Setup
    typedef struct tagComprehensionCommon {
        BYTE cTag;
        BYTE cLength;
    } TAG_COMMON;

    typedef struct tagComprehensionAddress {
        TAG_COMMON common;
        BYTE cNpi : 4;
        BYTE cTon : 3;
        BYTE cTonNpiFlag : 1;
        BYTE acDialNumString[0];
    } CMPH_ADDRESS;

    typedef struct tagComprehensionAlphaIdentifier {
        TAG_COMMON common;
        BYTE acAlphaId[0];
    } CMPH_ALPHA_ID;

    typedef struct tagComprehensionCmdDetail {
        TAG_COMMON common;
        BYTE cNumber;
        BYTE cType;
        BYTE cQualifier;
    } CMPH_CMD_DETAIL;

    typedef struct tagComprehensionDeviceIdentifier {
        TAG_COMMON common;
        BYTE cSrcId;
        BYTE cDstId;
    } CMPH_DEVICE_ID;

    typedef struct tagComprehensionDuration {
        TAG_COMMON common;
        BYTE cTimeUnit;
        BYTE cTimeInterval;
    } CMPH_DURATION;

    typedef struct tagComprehensionResult {
        TAG_COMMON common;
        BYTE cGeneralResult;
        BYTE acAdditionalInfo[0];
    } CMPH_RESULT;

    typedef struct tagComprehensionTextString {
        TAG_COMMON common;
        BYTE cDataCodingScheme;
        BYTE acTextString[0];
    } CMPH_TEXT_STRING;

    typedef struct tagComprehensionFileList {
        TAG_COMMON common;
        BYTE cNumOfFiles;
        WORD awFiles[0];
    } CMPH_FILE_LIST;

    typedef struct tagComprehensionAID {
        TAG_COMMON common;
        BYTE acAID[0];
    } CMPH_AID;
#pragma pack()

    // Comprehension Tag
    enum eComprehensionTag {
        CAT_CMPH_TAG_CMD_DETAIL = 0x01,
        CAT_CMPH_TAG_DEVICE_ID,
        CAT_CMPH_TAG_RESULT,
        CAT_CMPH_TAG_DURATION,
        CAT_CMPH_TAG_ALPHA,
        CAT_CMPH_TAG_ADDRESS,
        CAT_CMPH_TAG_TEXT_STRING = 0x0D,
        CAT_CMPH_TAG_FILE_LIST = 0x12,
        CAT_CMPH_TAG_AID = 0x2F,
    };

    // Type of Command
    enum eTypeOfCommand {
        CAT_CMD_TYPE_REFRESH = 0x01,
        CAT_CMD_TYPE_DISPLAY_TEXT = 0x21,
        CAT_CMD_TYPE_SETUP_CALL = 0x10,
    };

    // Command Qualifier
    enum eCommandQualifier {
        CAT_CMD_QUALFR_SETUPCALL_NOT_BUSY = 0x00,
        CAT_CMD_QUALFR_SETUPCALL_NOT_BUSY_REDIAL,
        CAT_CMD_QUALFR_SETUPCALL_OTHERS_HOLD,
        CAT_CMD_QUALFR_SETUPCALL_OTHERS_HOLD_REDIAL,
        CAT_CMD_QUALFR_SETUPCALL_DISCONNECTING_OTHERS,
        CAT_CMD_QUALFR_SETUPCALL_DISCONNECTING_OTHERS_REDIAL
    };

    // Device Identities
    enum eDeviceIdentities {
        CAT_DEVICE_ID_KEYPAD = 0x01,
        CAT_DEVICE_ID_DISPLAY,
        CAT_DEVICE_ID_EARPIECE,
        CAT_DEVICE_ID_UICC = 0x81,
        CAT_DEVICE_ID_TERMINAL,
        CAT_DEVICE_ID_NETWORK
    };

    // Address TON
    enum eAddressTON {
        CAT_ADDR_TON_UNKNOWN = 0b000,
        CAT_ADDR_TON_INTERNATIONAL = 0b001,
        CAT_ADDR_TON_NATIONAL = 0b010,
        CAT_ADDR_TON_NET_SPECIFIC = 0b11
    };

    // Address NPI
    enum eAddressNPI {
        CAT_ADDR_NPI_UNKNOWN = 0b0000,
        CAT_ADDR_NPI_ISDN_TEL_NUM_PLAN = 0b0001,
        CAT_ADDR_NPI_DATA_NUM_PLAN = 0b0011,
        CAT_ADDR_NPI_TEL_NUM_PLAN = 0b0100,
        CAT_ADDR_NPI_PRIVATE_NUM_PLAN = 0b1001,
        CAT_ADDR_NPI_RESERVED_EXT = 0b1111
    };

    // Duration Time Unit
    enum eDurationTimeUnit {
        CAT_DURATION_TIME_UNIT_MINUTE = 0x00,
        CAT_DURATION_TIME_UNIT_SECONDS,
        CAT_DURATION_TIME_UNIT_TENTHS_SECONDS
    };

    // General Result
    enum eGeneralResult {
        CAT_GENERAL_RESULT_SUCCESS = 0x00,
        CAT_GENERAL_RESULT_USER_REJECT = 0x22,
    };

    // Text Coding Scheme
    enum eTextCodingScheme {
        CAT_TEXT_CODING_GSM7BIT = 0x00,
        CAT_TEXT_CODING_GSM8BIT = 0x04,
        CAT_TEXT_CODING_UCS2 = 0x08
    };

    int GetProactiveCmdLength() const { return m_nProactiveCmdLength; }
    BYTE *GetProactiveCommand() const { return m_pProactiveCmd; }

    int GetEfidCount(void) { return m_nCount; }
    UINT GetEFID(int nIndex) { return (nIndex<m_nCount)? m_arEFID[nIndex] : 0; }

    int GetAidLength(void) { return m_nAidLen; }
    BYTE *GetAID(void) { return m_acAID; }
};

class ProtocolStkSimRefreshAdapter : public ProtocolRespAdapter {
public:
    ProtocolStkSimRefreshAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }

private:
    int m_nResult;

    virtual void Init();

public:
    int GetResult() const { return m_nResult; }
};

class ProtocolSsReturnResultAdapter : public ProtocolRespAdapter {
public:
    ProtocolSsReturnResultAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { Init(); }

private:
    int m_nReturnResultLength;
    BYTE *m_pReturnResult;

    virtual void Init();

public:
    int GetReturnResultLength() const { return m_nReturnResultLength; }
    BYTE *GetReturnResult() const { return m_pReturnResult; }
};

class ProtocolStkCcAlphaNtfAdapter : public ProtocolIndAdapter {
public:
    ProtocolStkCcAlphaNtfAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) { }
    int GetAlphaLength() const;
    BYTE* GetAlpha() const;
};
#endif /* __PROTOCOL_STK_ADAPTER_H__ */
