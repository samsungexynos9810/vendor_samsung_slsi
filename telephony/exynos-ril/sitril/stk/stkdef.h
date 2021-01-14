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
 * stkdef.h
 *
 *  Created on: 2017.03.23.
 *      Author: MOX
 */

#ifndef __STK_DEFINITION_H__
#define __STK_DEFINITION_H__

#define   SWAP16(val)     ( (((val) << 8) & 0xFF00) | (((val) >> 8) & 0x00FF) )
#define   SWAP32(val)     ( (((val) & 0x000000FF) << 24) | (((val) & 0x0000FF00) <<  8) | \
                                (((val) & 0x00FF0000) >>  8) | (((val) & 0xFF000000) >> 24) )
#define   SWAP(val)       ( sizeof(val)==2? SWAP16(val): ( sizeof(val)==4? SWAP32(val): (val)) )

typedef enum eComprehensionTlvTag {
    TAG_NONE,
    TAG_COMMAND_DETAIL,
    TAG_DEVICE_IDENTITY,
    TAG_RESULT,
    TAG_DURATION,
    TAG_ALPHA_IDENTIFIER,
    TAG_ADDRESS,
    TAG_SUB_ADDRESS = 0x08,
    TAG_TEXT_STRING = 0x0D,
    TAG_EVENT_LIST = 0x19,
    TAG_ICON_IDENTIFIER = 0x1E,
    TAG_BEARER_DESCRIPTION = 0x35,
    TAG_CHANNEL_DATA,
    TAG_CHANNEL_DATA_LENGTH,
    TAG_CHANNEL_STATUS,
    TAG_BUFFER_SIZE,
    TAG_UICC_TERMINAL_INTERFACE_TRANSPORT_LEVEL = 0x3C,
    TAG_OTHER_ADDRESS = 0x3E,
    TAG_NETWORK_ACCESS_NAME = 0x47,
    TAG_REMOTE_ENTITY_ADDRESS = 0x49,
    TAG_TEXT_ATTRIBUTE,
    TAG_FRAME_IDENTIFIER = 0x68,
    TAG_UTRAN_EUTRAN_MEASUREMENT_QUALIFIER,
    TAG_CSG_ID_LIST = 0x7E,
} COMPREHENSION_TLV_TAG;

#pragma pack(1)

typedef struct tagAddress {
   BYTE cTONnNPI;
   BYTE cTextLength;
   BYTE acDiallingNumber[0];
} ADDRESS;

typedef struct tagSubAddress {
   BYTE cAddressLength;
   BYTE acAddress[0];
} SUB_ADDRESS;

typedef struct tagCommandDetail {
    BYTE cNumber;
    BYTE cType;
    BYTE cQulaifier;
} COMMAND_DETAIL;

typedef struct tagDeviceId {
    BYTE cSource;
    BYTE cDestination;
} DEVICE_ID;

typedef struct tagDuration {
    BYTE cTimeUnit;
    BYTE cTimeInterval;
} DURATION;

typedef struct tagIconIdentifier {
    BYTE cIconQualifier;
    BYTE cIconIdentifier;
} ICON_IDENTIFIER;

typedef struct tagResult {
    BYTE cGeneralResult;
    BYTE cAdditionalInformation[0];
} RESULT;

typedef struct tagTextString {
    BYTE cCodingScheme;
    BYTE acTextString[0];
} TEXT_STRING;

typedef struct tagTransportLevel {
    BYTE cTransportType;
    WORD wPort;
} TRANSPORT_LEVEL;

typedef struct tagOtherAddress {
    BYTE cAddressType;
    BYTE acIP[16];      // IPv4(4byte), IPv6(16byte)
} OTHER_ADDRESS;

typedef struct tagCsdCbst {
    BYTE m_cSpeed;
    BYTE m_cName;
    BYTE m_cCe;
} CSD_CBST;

typedef struct tagGprsCgqreq {
    BYTE m_cPrecedence;
    BYTE m_cDelay;
    BYTE m_cReliability;
    BYTE m_cPeak;
    BYTE m_cMean;
    BYTE m_cProtocolType;
} GPRS_CGQREQ;

typedef struct tagEutranCgqreq {
    BYTE cQCI;
    BYTE cMaxBitrateUL;
    BYTE cMaxBitrateDL;
    BYTE cGuaranteedBitrateUL;
    BYTE cGuaranteedBitrateDL;
    BYTE cMaxBitrateUlExt;
    BYTE cMaxBitrateDlExt;
    BYTE cGuaranteedBitrateUlExt;
    BYTE cGuaranteedBitrateDlExt;
    BYTE cPdnType;
} EUTRAN_CGQREQ;

typedef struct tagBearerDescription {
    BYTE cType;
    BYTE acParameter[0];
    /*
    union {
        CSD_CBST tCsdCbst;
        GPRS_CGQREQ tGprsCgqReq;
    } param;
    */
} BEARER_DESC;

typedef struct tagExtendedC {
    BYTE cTrafficClass;
    BYTE acMaxBitrateUL[2];
    BYTE acMaxBitrateDL[2];
    BYTE acGuaranteedBitrateUL[2];
    BYTE acGuaranteedBitrateDL[2];
    BYTE cDeliveryOrder;
    BYTE cMaxSDUSize;
    BYTE cSDUErrorRatio;
    BYTE cResidualBitErrorRatio;
    BYTE cDeliveryOfErroneousSDUs;
    BYTE cTransferDelay;
    BYTE cTrafficHandlingPriority;
    BYTE cPdpType;
} EXTENDED_CGEQREQ;

typedef struct tagRemoteEntityAddress {
    BYTE cCodingType;
    LONG lLength;
    BYTE acAddress[0];
} REMOTE_ENTITY_ADDRESS;

typedef struct tagPlmn {
    BYTE m_cPlmnTag;
    BYTE m_cPlmnLength;
    BYTE *m_pPLMN;
} PLMN;

typedef struct tagCsgIdName {
    BYTE m_cCsgIdNameTag;
    BYTE m_cCsgIdNameLength;
    int m_cCsgId;
    BYTE *m_pHnbIdName;
} CSGIDNAME;

typedef struct CsgidList {
    PLMN m_stPlmn;
    CSGIDNAME m_stCsgIdName;
} CSGIDLIST;

#pragma pack()

#endif // __STK_DEFINITION_H__
