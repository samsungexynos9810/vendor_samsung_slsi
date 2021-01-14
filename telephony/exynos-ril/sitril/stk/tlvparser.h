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
 * tlvparser.h
 *
 *  Created on: 2017.02.21.
 *      Author: MOX
 */

#ifndef __TLV_PARSER_H__
#define __TLV_PARSER_H__

#include <stddef.h>
#include <vector>
#include "tlv.h"

//For OpenChannel Command Qualifier
#define OP_ONDEMAND_LINK_ESTABLISHMENT  (0 << 0)
#define OP_IMMEDIATE_LINK_ESTABLISHMENT  (1 << 0)

#define OP_NO_AUTOMATIC_RECONNECTION  (0 << 0)
#define OP_AUTOMATIC_RECONNECTION  (1 << 0)

#define OP_NO_BACKGROUND_MODE  (0 << 2)
#define OP_IMMEDIATE_LINK_IN_BACKGROUND_MODE  (1 << 2)

#define OP_NO_DNS_SERVER_ADDRESS_REQUESTED  (0 << 3)
#define OP_DNS_SERVER_ADDRESS_REQUESTED  (1 << 3)

// For SendData Command Qualifier
#define SD_STORE_DATA_IN_TX_BUFFER  (0 << 0)
#define SD_SEND_IMMEDIATELY (1 << 0)

#define RFU (0 << 0)

class Address : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_ADDRESS)

public:
    typedef enum TonValue {
       TON_UNKNOWN = 0x00,
       INTERNATIONAL_NUMBER,
       NATIONAL_NUMBER,
       NETWORK_SPECIFIC_NUMBER,
       TON_RESERVED
    } TON_VALUE;

    typedef enum NpiValue {
       NPI_UNKNOWN = 0x00,
       TELEPHONY_NUMBERING_PLAN,
       DATA_NUMBERING_PLAN = 0x03,
       TELEX_NUMBERING_PLAN,
       PRIVATE_NUMBERING_PLAN = 0x09,
       NPI_RESERVED = 0x0F
    } NPI_VALUE;

    Address() : CTLV(), m_ptAddress(NULL) { }
    Address(const BYTE *pData, int nLength) { Set(pData, nLength); }
    Address(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~Address() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int nTONnNPI, BYTE *pDiallingNumber, int nTextLength);

    int GetTONnNPI() { return m_ptAddress ? (int) m_ptAddress->cTONnNPI: 0x80; }       // TON : Unknown, NPI : Unknown value 1000 0000
    int GetDiallingNumberLength() { return m_ptAddress? (int) m_ptAddress->cTextLength: 0; }
    BYTE *GetDiallingNumber() const { return m_ptAddress? m_ptAddress->acDiallingNumber: NULL; }

protected:
    ADDRESS *m_ptAddress;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

class Subaddress : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_SUB_ADDRESS)

public:
    Subaddress() : CTLV(), m_ptSubaddress(NULL) { }
    Subaddress(const BYTE *pData, int nLength) { Set(pData, nLength); }
    Subaddress(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~Subaddress() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(BYTE *pSubaddress, int nSubaddrLength);

    int GetAddressLength() { return m_ptSubaddress? (int) m_ptSubaddress->cAddressLength: 0; }
    BYTE *GetAddress() const { return m_ptSubaddress? m_ptSubaddress->acAddress: NULL; }

protected:
    SUB_ADDRESS *m_ptSubaddress;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

class CommandDetail : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_COMMAND_DETAIL)

public:
    typedef enum TypeOfCommand {
        REFRESH = 0x01,
        MORE_TIME,
        POLL_INTERVAL,
        POLLING_OFF,
        SET_UP_EVENT_LIST,
        SET_UP_CALL = 0x10,
        SEND_SS,
        SEND_USSD,
        SEND_SMS,
        SEND_DTMF,
        LAUNCH_BROWSER,
        GEOGRAPHICAL_LOCATION_REQUEST,
        PLAY_TONE = 0x20,
        DISPLAY_TEXT,
        GET_INKEY,
        GET_INPUT,
        SELECT_ITEM,
        SET_UP_MENU,
        PROVIDE_LOCAL_INFORMATION,
        TIMER_MANAGEMENT,
        SET_UP_IDLE_MODE_TEXT,
        PERFORM_CARD_APDU = 0x30,
        POWER_ON_CARD,
        GET_READER_STATUS,
        RUN_AT_COMMAND,
        LANGUAGE_NOTIFICATION,
        OPEN_CHANNEL = 0x40,
        CLOSE_CHANNEL,
        RECEIVE_DATA,
        SEND_DATA,
        GET_CHANNEL_STATUS,
        SERVICE_SEARCH,
        GET_SERVICE_INFORMATION,
        DECLARE_SERVICE,
        SET_FRAMES = 0x50,
        GET_FRAMES_STATUS,
        RETRIVE_MULTIMEDIA_MESSAGE = 0x60,
        SUBMIT_MULTIMEDIA_MESSAGE,
        DISPLAY_MULTIMEDIA_MESSAGE,
        ACTIVATE = 0x70,
        CONTACTLESS_STATE_CHANGED,
        COMMAND_CONTAINER,
        ENCAPSULATED_SESSION_CONTROL,
        END_PROACTIVE_UICC_SESSION = 0x81,
    } TYPE_OF_COMMAND;

    CommandDetail() : CTLV() { memset(&m_tCmdDetail, 0, sizeof(COMMAND_DETAIL)); }
    CommandDetail(const BYTE *pData, int nLength) { Set(pData, nLength); }
    CommandDetail(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    CommandDetail(BYTE bCmdType, BYTE bQualifier) { Set(bCmdType, bQualifier, 1); };
    CommandDetail(BYTE bCmdType, BYTE bQualifier, BYTE nCommandNumber) { Set(bCmdType, bQualifier, nCommandNumber); };
    virtual ~CommandDetail() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(BYTE bCmdType, BYTE bQualifier, BYTE nCommandNumber);

    int GetCommandNumber() { return (int) m_tCmdDetail.cNumber; }
    int GetCommandType() { return (int) m_tCmdDetail.cType; }
    const char *GetCommandTypeString();
    int GetQualifier() { return (int) m_tCmdDetail.cQulaifier; }

protected:
    COMMAND_DETAIL m_tCmdDetail;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

class DeviceIdentity : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_DEVICE_IDENTITY)

public:
    typedef enum Device_Identity {
        KEYPAD = 0x01,
        DISPLAY,
        EARPIECE,
        CHANNEL1 = 0x21,
        CHANNEL2,
        CHANNEL3,
        CHANNEL4,
        CHANNEL5,
        CHANNEL6,
        CHANNEL7,
        UICC = 0x81,
        TERMINAL,
        NETWORK
    } DEVICE_IDENTITY;

    DeviceIdentity() : CTLV() { memset(&m_tDeviceId, 0, sizeof(DEVICE_ID)); }
    DeviceIdentity(const BYTE *pData, int nLength) { Set(pData, nLength); }
    DeviceIdentity(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    DeviceIdentity(int nSourceDeviceId, int nDestinationDevieceID) { Set(nSourceDeviceId, nDestinationDevieceID); }
    virtual ~DeviceIdentity() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int nSourceDeviceId, int nDestinationDevieceID);

    int GetSourceID() { return (int) m_tDeviceId.cSource; }
    int GetDestinationID() { return (int) m_tDeviceId.cDestination; }

    static const char *GetDeviceIdString(int nDeviceId);

protected:
    DEVICE_ID m_tDeviceId;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

class Duration : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_DURATION)

public:
    typedef enum TimeUnit {
        MINUTES = 0x00,
        SECONDS,
        TENTHS_OF_SECONDS,
        RESERVED_TU
    } TIME_UINT;

    typedef enum TimeInterval {    // range : 1~255
        RESERVED_TI = 0x00,
        UNIT_1,
        UNIT_2,
        UNIT_255 = 0xFF
    } TIME_INTERVAL;

    Duration() : CTLV() { memset(&m_tDuration, 0, sizeof(DURATION)); }
    Duration(const BYTE *pData, int nLength) { Set(pData, nLength); }
    Duration(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~Duration() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int nTimeUnit, int nTimeInterval);

    int GetTimeUnit() { return (int) m_tDuration.cTimeUnit; }
    int GetTimeInterval() { return (int) m_tDuration.cTimeInterval; }

protected:
    DURATION m_tDuration;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

class Result : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_RESULT)

public:
    typedef enum GeneralResult {
        SUCCESS = 0x00,
        PERFORM_WITH_PARTIAL,
        PERFORM_WITH_MISSING,
        REFRESH_WITH_EFS,
        SUCCESS_WITHOUT_ICON_DISPLAY,
        PERFORM_BY_CALL_CONTROL,
        SUCCESS_LIMITED_SERVICE,
        PERFORM_WITH_MODIFICATION,
        REFRESH_INACTIVE_NAA,
        SUCCESS_WITHOUT_TONE,
        PROACTIVE_SESSION_TERMINATED,
        BACKWARD_PROACTIVE_BY_USER,
        NO_RESPONSE_FROM_USER,
        RESERVED_GSM_3G,
        TERMINAL_UNABLE_PROCESS_COMMAND = 0x20,
        NETWORK_UNABLE_PROCESS_COMMAND,
        USER_UNACCEPT_PROACTIVE_COMMAND,
        USER_CLEAR_CALL_BEOFRE_CONNECTION_OR_NETWORK,
        BEYOND_TERMINAL_CAPABILITY = 0x30,
        BIP_ERROR = 0x3a,
    } GENERAL_RESULT;

    typedef enum AdditionalInformation { //for terminal problem
        NO_SPECIFIC_CAUSE,
        SCREEN_IS_BUSY,
        TERMINAL_BUSY,
        RESERVED_FOR_GSM_3G,
        NO_SERVICE,
        ACCESS_CONTROL_CLASS_BAR,
        RADIO_RESOURCE_NOT_GRANTED,
        NOT_IN_SPEECH_CALL,
        RESERVED_4_GSM_3G,
        TERMINAL_BUSY_DTMF,
        NO_NAA_ACTIVE
    } ADDITIONAL_INFORMATION;

    typedef enum AdditionalInformationBIP { //Bearer Independent Protocol
        NO_SPECIFIC_CAUSE_BIP,
        NO_CAHNNEL_AVAILABLE,
        CHANNEL_CLOSED,
        CHANNEL_ID_NOT_VAILD,
        REQUEST_BUFFERSIZE_NOT_AVAILABLE,
        SECURITY_ERROR,
        TRANSPORT_LEVEL_NOT_AVAILABLE,
        REMOTE_DEVICE_NOT_REACHABLE,
        SERVICE_ERROR,
        SERVICE_ID_UNKNOWN,
        PORT_NOT_AVAILABLE,
        PARAMETER_MISSING_INCORRECT
    } ADDITIONAL_INFORMATION_BIP;

    Result() : CTLV(), m_ptResult(NULL) { }
    Result(const BYTE *pData, int nLength) { Set(pData, nLength); }
    Result(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    Result(int nLength, int nGeneralResult, BYTE *cAdditionalInformation) { Set(nLength, nGeneralResult, cAdditionalInformation); }
    virtual ~Result() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int nLength, int nGeneralResult, BYTE *cAdditionalInformation);

    int GetGeneralResult() { return (int) m_ptResult->cGeneralResult; }
    BYTE *GetAddiotionalInformation() { return m_ptResult->cAdditionalInformation; }
    string GetAdditionalInfoString() { return m_strAddtionalInfo; }
    void SetAdditionalInformation(BYTE *pAdditionalInfo) { }

protected:
    RESULT *m_ptResult;
    string m_strAddtionalInfo;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

class AlphaIdentifier : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_ALPHA_IDENTIFIER)

public:

    AlphaIdentifier() : CTLV(), m_strAlphaIdentifier(NULL) { }
    AlphaIdentifier(const BYTE *pData, int nLength) { Set(pData, nLength); }
    AlphaIdentifier(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~AlphaIdentifier() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(const string &strAlphaIdentifier);

    string GetAlphaIdentifier() { return m_strAlphaIdentifier; }

protected:
    string m_strAlphaIdentifier;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

class IconIdentifier : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_ICON_IDENTIFIER)

public:

    IconIdentifier() : CTLV() { memset(&m_tIconIdentifier, 0, sizeof(ICON_IDENTIFIER)); }
    IconIdentifier(const BYTE *pData, int nLength) { Set(pData, nLength); }
    IconIdentifier(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~IconIdentifier() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int nIconQualifier, int nIconIdentifier);

    int GetIconQualifier() { return (int)m_tIconIdentifier.cIconQualifier; }
    int GetIconIdentifier() { return (int)m_tIconIdentifier.cIconIdentifier; }

protected:
    ICON_IDENTIFIER m_tIconIdentifier;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

class TextString : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_TEXT_STRING)

public:
    typedef enum CodingScheme {
        GSM7BITS = 0x00,
        GSM8BITS = 0x04,
        UCS2 = 0x08
    } CODING_SCHEME;

    TextString();
    TextString(const BYTE *pData, int nLength) : m_pTextString(NULL) { Set(pData, nLength); }
    TextString(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~TextString() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(CodingScheme CodeScheme, BYTE *pTextString, int nTextLength);

    int GetCodingScheme() { return m_CodingScheme; }
    int GetTextStringLength() { return  m_nTextStringLength; }
    BYTE *GetTextString() const { return m_pTextString; }

protected:
    CodingScheme m_CodingScheme;
    int m_nTextStringLength;
    BYTE *m_pTextString;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

// Bearer description
class BearerDescription : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_BEARER_DESCRIPTION)

public:
    typedef enum BearerType {
        CSD = 0x01,
        GPRS = 0x02,
        DEFAULT_BEARER = 0x03,
        LOCAL_LINK = 0x04,
        BLUETOOTH = 0x05,
        IRDA = 0x06,
        RS232 = 0x07,
        CDMA2000 = 0x08,
        UTRAN = 0x09,
        I_WLAN = 0x0a,
        E_UTRAN = 0x0b,
        USB = 0x10
    } BEARER_TYPE;

    /*
    typedef enum CbstSpeed {
        AUTOBAUNDING = 0,
        BPS_300_V21 = 1,
        BPS_1200_V22 = 2,
        BPS_1200_75_V23 = 3,
        BPS_2400_V22_BIS = 4,
        BPS_2400_V26_TER = 5,
        BPS_4800_V32 = 6,
        BPS_9600_V32 = 7,
        BPS_9600_V34 = 12,
        BPS_14400_V34 = 14,
        BPS_19200_V34 = 15,
        BPS_28800_V34 = 16,
        BPS_33600_V34 = 17,
        BPS_1200_V120 = 34,
        BPS_2400_V120 = 36,
        BPS_4800_V120 = 38,
        BPS_9600_V120 = 39,
        BPS_14400_V120 = 43,
        BPS_19200_V120 = 47,
        BPS_28800_V120 = 48,
        BPS_38400_V120 = 49,
        BPS_48000_V120 = 50,
        BPS_56000_V120 = 51,
        BPS_300_V110 = 65,
        BPS_1200_V110 = 66,
        BPS_2400_V110_X31 = 68,
        BPS_4800_V110_X31 = 70,
        BPS_9600_V110_X31 = 71,
        BPS_14400_V110_X31 = 75,
        BPS_19200_V110_X31 = 79,
        BPS_28800_V110_X31 = 80,
        BPS_38400_V110_X31 = 81,
        BPS_48000_V110_X31 = 82,
        BPS_56000_V110_X31 = 83,
        BPS_64000_X_31 = 84,
        BPS_56000_BIT_TRANSPARENT = 115,
        BPS_64000_BIT_TRANSPARENT = 116,
        BPS_32000_PIAFS32k = 120,
        BPS_64000_PIAFS64k = 121,
        BPS_28800_MULTIMEDIA = 130,
        BPS_32000_MULTIMEDIA = 131,
        BPS_33600_MULTIMEDIA = 132,
        BPS_56000_MULTIMEDIA = 133,
        BPS_64000_MULTIMEDIA = 134
    };

    typedef enum CbstName {
        DATA_CIRCUIT_ASYNCH_UDI = 0,
        DATA_CIRCUIT_SYNCH_UDI = 1,
        PAD_ACCESS_UDI = 2,
        PACKET_ACCESS_UDI = 3,
        DATA_CIRCUIT_ASYNCH_RDI = 4,
        DATA_CIRCUIT_SYNCH_RDI = 5,
        PAD_ACCESS_RDI = 6,
        PACKET_ACCESS_RDI = 7
    };

    typedef enum CbstCe {
        TRANSPARENT = 0,
        NON_TRANSPARENT = 1,
        BOTH_TRANSPARENT = 2,
        BOTH_NON_TRANSPARENT = 3
    };

    typedef enum ProtocolType {
        PROTOCOL_IP = 0x02
    };
    */

    BearerDescription() : CTLV(), m_ptDescription(NULL), m_strParameter(NULL) { }
    BearerDescription(const BYTE *pData, int nLength) { Set(pData, nLength); }
    BearerDescription(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    BearerDescription(BYTE cBearerType, const BYTE *pBearerParameter) { Set(cBearerType, pBearerParameter); }
    BearerDescription(BYTE cBearerType, const string &strBearerParameter) { Set(cBearerType, strBearerParameter); }
    virtual ~BearerDescription() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(BYTE cBearerType, const BYTE *pBearerParameter);
    void Set(BYTE cBearerType, const string &strBearerParameter);

    BYTE GetType() { return m_ptDescription? m_ptDescription->cType: 0; }
    string GetParamString() { return m_strParameter; }
    BYTE *GetParameter() { return m_ptDescription ? (BYTE *)(m_ptDescription->acParameter) : NULL; }

protected:
    BEARER_DESC *m_ptDescription;
    string m_strParameter;
    // it has Service Record that is associated with.
    //CServiceRecord *m_objServiceRecord;

    virtual void Initialize();
    virtual void Finalize() {  }
};

/*
// Service Record
class CServiceRecord : public CTLV
{
    DECLARE_MODULE_TAG()

public:
    typedef enum BearerTechnologyId {
        TECHNOLOGY_INDEPENDENT = 0,
        BLUETOOTH = 1,
        IRDA = 2,
        RS232 = 3,
        USB = 4,
        RFU
    };

    CServiceRecord() : CTLV() { }
    CServiceRecord(const BYTE *pData, int nLength) { Set(pData, nLength); }
    CServiceRecord(BYTE cTag, const BYTE *pValue, int nLength) : CTLV(cTag, pValue, nLength) { }
    virtual ~CServiceRecord() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    //virtual void Set(BYTE cTag, );

    virtual BYTE getLocalBearerTechnologyId() { return m_nLocalBearerTechnologyId; }
    virtual BYTE getServiceId() { return m_nServiceId; }
    virtual BYTE *getServiceRecord() { return m_pServiceRecord; }

protected:
    BYTE m_nLocalBearerTechnologyId;
    BYTE m_nServiceId;
    BYTE *m_pServiceRecord;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};
*/

// Channel data
class ChannelData : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_CHANNEL_DATA)

public:

    ChannelData() : CTLV() { }
    ChannelData(const BYTE *pData, int nLength) { Set(pData, nLength); }
    ChannelData(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    ChannelData(const string &strChannelData) { Set(strChannelData); }
    virtual ~ChannelData() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(const string &strChannelData);

    string GetChannelDataString() { return m_strChannelData; }

protected:
    string m_strChannelData;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

// Channel data length
class ChannelDataLength : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_CHANNEL_DATA_LENGTH)

public:

    ChannelDataLength() : CTLV(), m_nChannelDataLength(0) { }
    ChannelDataLength(const BYTE *pData, int nLength) { Set(pData, nLength); }
    ChannelDataLength(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    ChannelDataLength(int nDataLen) { Set(nDataLen); }
    virtual ~ChannelDataLength() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int m_nChannelDataLength);

    int GetChannelDataLength() { return (int)m_nChannelDataLength; }

protected:
    BYTE m_nChannelDataLength;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

// Channel Status
class ChannelStatus : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_CHANNEL_STATUS)

public:
    typedef enum CommunicationMode {
        CS_PACKET_DATA_SERVICE = 1,
        UICC_SERVER = 2,
        TERMINAL_SERVER_AND_DIRECT_COMMUNICATION_CHANNEL = 3,
        TERMINAL_SERVER_AND_UDP = 4
    } COMMUNICATION_MODE;

    typedef enum Channel_Status {
        CLOSED = 0,
        LISTEN = 1,
        ESTABLISHED = 2,
        RESERVED = 3
    } CHANNEL_STATUS;

    typedef enum FurtherInfo {
        NO_FURTHER_INFO = 0,
        LINK_DROPPED = 5
    } FUTHER_INFO;

    ChannelStatus() : CTLV() { }
    ChannelStatus(const BYTE *pData, int nLength) { Set(pData, nLength); }
    ChannelStatus(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    ChannelStatus(int nChannelId, Channel_Status channelStatus, bool isFurtherInfo, int furtherInfo) { Set(nChannelId, channelStatus, isFurtherInfo, furtherInfo); }
    virtual ~ChannelStatus() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int nChannelId, Channel_Status channelStatus, bool isFurtherInfo, int furtherInfo);

    BYTE GetRfu(int mode);
    BYTE GetChannelId() { return m_szChannelStatus[0] & 0x7; }
    BYTE isServiceEstablished() { return m_szChannelStatus[0] & 0x80;}
    BYTE GetTcpStatus() { return m_szChannelStatus[0] & 0xc0; }
    BYTE GetFurtherInfo() { return m_szChannelStatus[1]; }

protected:
    char m_szChannelStatus[2];

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

// Buffer size
class BufferSize : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_BUFFER_SIZE)

public:

    BufferSize() : CTLV(), m_nBufferSize(0) { }
    BufferSize(const BYTE *pData, int nLength) { Set(pData, nLength); }
    BufferSize(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    BufferSize(int nBufferSize) { Set(nBufferSize); }
    virtual ~BufferSize() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int nBufferSize);

    int GetBufferSize() { return m_nBufferSize; }

protected:
    int m_nBufferSize;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

// Uicc/terminal interface transport level
class TerminalInterfaceTransportLevel : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_UICC_TERMINAL_INTERFACE_TRANSPORT_LEVEL)

public:
    typedef enum TransportProtocolType {
        // all other values are reserved
        UDP_CLIENT_REMOTE = 0x01,
        TCP_CLIENT_REMOTE = 0x02,
        TCP_SERVER = 0x03,
        UDP_CLIENT_LOCAL = 0x04,
        TCP_CLIENT_LOCAL = 0x05,
        DIRECT_COMMUNICATION_CHENNEL = 0x06
    } TRANSPORT_PROTOCOL_TYPE;

    TerminalInterfaceTransportLevel() : CTLV() { memset(&m_tTransportLevel, 0, sizeof(TRANSPORT_LEVEL)); }
    TerminalInterfaceTransportLevel(const BYTE *pData, int nLength) { Set(pData, nLength); }
    TerminalInterfaceTransportLevel(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~TerminalInterfaceTransportLevel() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int nTransportType, int nPort);

    int GetTransportType() { return (int) m_tTransportLevel.cTransportType; }
    int GetPort() { return (int) m_tTransportLevel.wPort; }
protected:
    TRANSPORT_LEVEL m_tTransportLevel;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

class TextAttribute : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_TEXT_ATTRIBUTE)

public:

    TextAttribute() : CTLV() { }
    TextAttribute(const BYTE *pData, int nLength) { Set(pData, nLength); }
    TextAttribute(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~TextAttribute() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(const string &strTextFormatting);

    string GetTextAttribute() { return m_strTextFormatting; }

protected:
    string m_strTextFormatting;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

// Frame identifier
class FrameIdentifier : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_FRAME_IDENTIFIER)

public:
    FrameIdentifier() : CTLV(), m_nIdentifierOfFrame(0) { }
    FrameIdentifier(const BYTE *pData, int nLength) { Set(pData, nLength); }
    FrameIdentifier(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~FrameIdentifier() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int nIdentifierOfFrame);

    virtual int GetIdentifierOfFrame() { return m_nIdentifierOfFrame; }
protected:
    int m_nIdentifierOfFrame;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

// Other address
class OtherAddress : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_OTHER_ADDRESS)

    static const int IPV4_OCTET_SIZE = 4;
    static const int IPV6_OCTET_SIZE = 16;

public:
    typedef enum AddressType {
        ADDRESS_IPV4 = 0x21,
        ADDRESS_IPV6 = 0x57
    } ADDRESS_TYPE;

    OtherAddress() : CTLV() { memset(&m_tOtherAddress, 0, sizeof(OTHER_ADDRESS)); }
    OtherAddress(const BYTE *pData, int nLength) { Set(pData, nLength); }
    OtherAddress(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~OtherAddress() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int m_nAddressType, const string &strAddress);

    int GetAddressType() { return (int) m_tOtherAddress.cAddressType; }
    BYTE *GetAddress() { return m_tOtherAddress.acIP; }
protected:
    OTHER_ADDRESS m_tOtherAddress;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

// Network access name
class NetworkAccessName : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_NETWORK_ACCESS_NAME)

public:
    NetworkAccessName() : CTLV() { }
    NetworkAccessName(const BYTE *pData, int nLength) { Set(pData, nLength); }
    NetworkAccessName(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~NetworkAccessName() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(const string &strAddress);

    char *NetworkAccessNameParser();
    string GetAddress() { return m_strNetworkAccessName; }
protected:
    string m_strNetworkAccessName;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

// UTRAN/E-UTRAN measurement qualifier
class MeasurementQualifier : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_UTRAN_EUTRAN_MEASUREMENT_QUALIFIER)

public:
    typedef enum QualifierSpec {
        UTRAN_INTRA_FREQUENCY = 0x01,
        UTRAN_INTER_FREQUENCY = 0x02,
        UTRAN_INTER_RAT_GERAN = 0x03,
        UTRAN_INTER_RAT_EUTRAN = 0x04,
        EUTRAN_INTRA_FREQUENCY = 0x05,
        EUTRAN_INTER_FREQUENCY = 0x06,
        EUTRAN_INTER_RAT_GERAN =0x07,
        EUTRAN_INTER_RAT_UTRAN = 0x08
    } QUALIFIER_SPEC;

    MeasurementQualifier() : CTLV(), m_nQualifier(0) { }
    MeasurementQualifier(const BYTE *pData, int nLength) { Set(pData, nLength); }
    MeasurementQualifier(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~MeasurementQualifier() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int qualifier);

    virtual int GetQualifier() { return m_nQualifier; }
protected:
    int m_nQualifier;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

// Remote entity address
class RemoteEntityAddress : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_REMOTE_ENTITY_ADDRESS)

public:
    typedef enum CodingType {
        ADDR_BIT48 = 0x00, // IEEE 802.16 , 48-bit address;
        ADDR_BIT32,    // 32-bit IrDA device address;
        ADDR_RESERVED, // 02 to FF are reserved values
    } CONDING_TYPE;

    RemoteEntityAddress() : CTLV(), m_ptRemoteEntityAddress(NULL) { }
    RemoteEntityAddress(const BYTE *pData, int nLength) { Set(pData, nLength); }
    RemoteEntityAddress(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    virtual ~RemoteEntityAddress() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(int nCodingType, BYTE *pAddress, LONG lLength);

    int GetCodingType() { return m_ptRemoteEntityAddress? (int) m_ptRemoteEntityAddress->cCodingType: ADDR_RESERVED; }
    LONG GetTextLength() { return m_ptRemoteEntityAddress? m_ptRemoteEntityAddress->lLength: 0; }
    BYTE *GetTextString() const { return m_ptRemoteEntityAddress? m_ptRemoteEntityAddress->acAddress: NULL; }

protected:
    REMOTE_ENTITY_ADDRESS *m_ptRemoteEntityAddress;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};

class EventList : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(TAG_EVENT_LIST)

public:
    typedef enum EventType {
        MT_CALL = 0x00,
        CALL_CONNECTED = 0x01,
        CALL_DISCONNECTED = 0x02,
        LOCATION_STATUS = 0x03,
        USER_ACTIVITY = 0x04,
        IDLE_SCREEN_AVAILABLE = 0x05,
        CARD_READER_STATUS = 0x06,
        LANGUAGE_SELECTION = 0x07,
        BROWSER_TERMINATION = 0x08,
        DATA_AVAILABLE_EVENT = 0x09,
        CHANNEL_STATUS_EVENT = 0x0A,
        SINGLE_ACCESS_TECHNOLOGY_CHANGE = 0x0B,
        DISPLAY_PARAMETERS_CHANGED = 0x0C,
        LOCAL_CONNECTION = 0x0D,
        NETWORK_SEARCH_MODE_CHANGE = 0x0E,
        BROWSING_STATUS = 0x0F,
        FRAMES_INFORMATION_CHANGE = 0x10,
        IWLAN_ACCESS_STATUS = 0x11,
        NETWORK_REJECTION = 0x12,
        HCI_CONNECTIVITY_EVENT = 0x13,
        MULTIPLE_ACCESS_TECHNOLOGY_CHANGE = 0x14,
        CSG_CELL_SELECTION = 0x15,
        CONTACTLESS_STATE_REQUEST = 0x16,
        IMS_REGISTRATION = 0x17,
        IMS_INCOMMING_DATA = 0x18,
        PROFILE_CONTAINER = 0x19,
        VOID = 0x1A,
        SECURED_PROFILE_CONTAINER = 0x1B,
        POLL_INTERVAL = 0x1C,
        UNKNOWN_EVENT = 0xFF,
    } EVENT_TYPE;

    EventList() : CTLV(), m_pEventList(NULL) { }
    EventList(const BYTE *pData, int nLength) { Set(pData, nLength); }
    EventList(int nLength, const BYTE *pValue) : CTLV(TLV_TAG, pValue, nLength) { }
    EventList(BYTE cEvent) { Set(cEvent); }
    virtual ~EventList() { Finalize(); }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(BYTE cEvent);

    BYTE GetEventList() { return (m_pEventList)? m_pEventList[0]: UNKNOWN_EVENT; }

protected:
    BYTE *m_pEventList;

    virtual void Initialize();
    virtual void Finalize() { CTLV::Finalize(); }
};
#endif // __TLV_PARSER_H__
