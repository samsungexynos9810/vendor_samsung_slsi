/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stdio.h>
#include <string.h>
#include "tlvparser.h"
#include "rillog.h"

static bool debug = true;

// add category to display selective logs
#undef LOGV
#define LOGV(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_VERBOSE_LOG,  "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGI
#define LOGI(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_INFO_LOG, "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGW
#define LOGW(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_WARNING_LOG,  "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGE
#define LOGE(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_STK, CRilLog::E_RIL_CRITICAL_LOG,  "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)


// Address class
IMPLEMENT_MODULE_TAG(Address, Address)
void Address::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    if(m_ptAddress) delete m_ptAddress;
    m_ptAddress = NULL;
}

BYTE *Address::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>(int)sizeof(ADDRESS) && m_pValue!=NULL)
    {
        m_ptAddress = (ADDRESS *) m_pValue;
    }

    return pRet;
}

void Address::Set(int nTONnNPI, BYTE *pDiallingNumber, int nTextLength)
{
    Initialize();

    m_cTag = TLV_TAG;
    if(nTextLength>0)
    {
        m_ptAddress = (ADDRESS *) new BYTE[sizeof(ADDRESS)+nTextLength];
        memset(m_ptAddress, 0, sizeof(TEXT_STRING)+nTextLength);
        m_ptAddress->cTONnNPI = (BYTE) nTONnNPI;
        m_ptAddress->cTextLength = (BYTE) nTextLength;
        memcpy(m_ptAddress->acDiallingNumber, pDiallingNumber, nTextLength);
    }
    m_nLength = sizeof(TEXT_STRING) + nTextLength;
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)m_ptAddress, m_nLength);
}

// Subaddress class
IMPLEMENT_MODULE_TAG(Subaddress, Subaddress)
void Subaddress::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    if(m_ptSubaddress) delete m_ptSubaddress;
    m_ptSubaddress = NULL;
}

BYTE *Subaddress::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>(int)sizeof(SUB_ADDRESS) && m_pValue!=NULL)
    {
        m_ptSubaddress = (SUB_ADDRESS *) m_pValue;
    }

    return pRet;
}

void Subaddress::Set(BYTE *pSubaddress, int nSubaddrLength)
{
    Initialize();

    m_cTag = TLV_TAG;
    if(nSubaddrLength>0)
    {
        m_ptSubaddress = (SUB_ADDRESS *) new BYTE[sizeof(SUB_ADDRESS)+nSubaddrLength];
        memset(m_ptSubaddress, 0, sizeof(SUB_ADDRESS)+nSubaddrLength);
        m_ptSubaddress->cAddressLength = (BYTE) nSubaddrLength;
        memcpy(m_ptSubaddress->acAddress, pSubaddress, nSubaddrLength);
    }
    m_nLength = sizeof(SUB_ADDRESS) + nSubaddrLength;
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)m_ptSubaddress, m_nLength);
}

// CommandDetail class
IMPLEMENT_MODULE_TAG(CommandDetail, CommandDetail)
void CommandDetail::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    memset(&m_tCmdDetail, 0, sizeof(COMMAND_DETAIL));
}

BYTE *CommandDetail::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=(int)sizeof(COMMAND_DETAIL) && m_pValue!=NULL)
    {
        memcpy(&m_tCmdDetail, m_pValue, sizeof(COMMAND_DETAIL));
    }

    return pRet;
}

void CommandDetail::Set(BYTE bCmdType, BYTE bQualifier, BYTE nCommandNumber)
{
    Initialize();
    m_cTag = TLV_TAG | 0x80;
    m_nLength = sizeof(COMMAND_DETAIL);
    m_tCmdDetail.cNumber = nCommandNumber;
    m_tCmdDetail.cType = bCmdType;
    m_tCmdDetail.cQulaifier = bQualifier;
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)&m_tCmdDetail, m_nLength);
}

const char *CommandDetail::GetCommandTypeString()
{
    switch(m_tCmdDetail.cType)
    {
    case REFRESH: return "REFRESH"; break;
    case MORE_TIME: return "MORE_TIME"; break;
    case POLL_INTERVAL: return "POLL_INTERVAL"; break;
    case POLLING_OFF: return "POLLING_OFF"; break;
    case SET_UP_EVENT_LIST: return "SET_UP_EVENT_LIST"; break;
    case SET_UP_CALL: return "SET_UP_CALL"; break;
    case SEND_SS: return "SEND_SS"; break;
    case SEND_USSD: return "SEND_USSD"; break;
    case SEND_SMS: return "SEND_SMS"; break;
    case SEND_DTMF: return "SEND_DTMF"; break;
    case LAUNCH_BROWSER: return "LAUNCH_BROWSER"; break;
    case GEOGRAPHICAL_LOCATION_REQUEST: return "GEOGRAPHICAL_LOCATION_REQUEST"; break;
    case PLAY_TONE: return "PLAY_TONE"; break;
    case DISPLAY_TEXT: return "DISPLAY_TEXT"; break;
    case GET_INKEY: return "GET_INKEY"; break;
    case GET_INPUT: return "GET_INPUT"; break;
    case SELECT_ITEM: return "SELECT_ITEM"; break;
    case SET_UP_MENU: return "SET_UP_MENU"; break;
    case PROVIDE_LOCAL_INFORMATION: return "PROVIDE_LOCAL_INFORMATION"; break;
    case TIMER_MANAGEMENT: return "TIMER_MANAGEMENT"; break;
    case SET_UP_IDLE_MODE_TEXT: return "SET_UP_IDLE_MODE_TEXT"; break;
    case PERFORM_CARD_APDU: return "PERFORM_CARD_APDU"; break;
    case POWER_ON_CARD: return "POWER_ON_CARD"; break;
    case GET_READER_STATUS: return "GET_READER_STATUS"; break;
    case RUN_AT_COMMAND: return "RUN_AT_COMMAND"; break;
    case LANGUAGE_NOTIFICATION: return "LANGUAGE_NOTIFICATION"; break;
    case OPEN_CHANNEL: return "OPEN_CHANNEL"; break;
    case CLOSE_CHANNEL: return "CLOSE_CHANNEL"; break;
    case RECEIVE_DATA: return "RECEIVE_DATA"; break;
    case SEND_DATA: return "SEND_DATA"; break;
    case GET_CHANNEL_STATUS: return "GET_CHANNEL_STATUS"; break;
    case SERVICE_SEARCH: return "SERVICE_SEARCH"; break;
    case GET_SERVICE_INFORMATION: return "GET_SERVICE_INFORMATION"; break;
    case DECLARE_SERVICE: return "DECLARE_SERVICE"; break;
    case SET_FRAMES: return "SET_FRAMES"; break;
    case GET_FRAMES_STATUS: return "GET_FRAMES_STATUS"; break;
    case RETRIVE_MULTIMEDIA_MESSAGE: return "RETRIVE_MULTIMEDIA_MESSAGE"; break;
    case SUBMIT_MULTIMEDIA_MESSAGE: return "SUBMIT_MULTIMEDIA_MESSAGE"; break;
    case DISPLAY_MULTIMEDIA_MESSAGE: return "DISPLAY_MULTIMEDIA_MESSAGE"; break;
    case ACTIVATE: return "ACTIVATE"; break;
    case CONTACTLESS_STATE_CHANGED: return "CONTACTLESS_STATE_CHANGED"; break;
    case COMMAND_CONTAINER: return "COMMAND_CONTAINER"; break;
    case ENCAPSULATED_SESSION_CONTROL: return "ENCAPSULATED_SESSION_CONTROL"; break;
    case END_PROACTIVE_UICC_SESSION: return "END_PROACTIVE_UICC_SESSION"; break;
    default: return "Unknown"; break;
    }
}

// DeviceIdentity class
IMPLEMENT_MODULE_TAG(DeviceIdentity, DeviceIdentity)
void DeviceIdentity::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    memset(&m_tDeviceId, 0, sizeof(DEVICE_ID));
}

BYTE *DeviceIdentity::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=(int)sizeof(DEVICE_ID) && m_pValue!=NULL)
    {
        memcpy(&m_tDeviceId, m_pValue, sizeof(DEVICE_ID));
    }

    return pRet;
}

void DeviceIdentity::Set(int nSourceDeviceId, int nDestinationDevieceID)
{
    Initialize();
    m_cTag = TLV_TAG | 0x80;
    m_nLength = sizeof(DEVICE_ID);
    m_tDeviceId.cSource = (BYTE) nSourceDeviceId;
    m_tDeviceId.cDestination = (BYTE) nDestinationDevieceID;
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)&m_tDeviceId, m_nLength);
}

const char *DeviceIdentity::GetDeviceIdString(int nDeviceId)
{
    switch(nDeviceId)
    {
    case KEYPAD: return "KEYPAD"; break;
    case DISPLAY: return "DISPLAY"; break;
    case EARPIECE: return "EARPIECE"; break;
    case CHANNEL1: return "CHANNEL1"; break;
    case CHANNEL2: return "CHANNEL2"; break;
    case CHANNEL3: return "CHANNEL3"; break;
    case CHANNEL4: return "CHANNEL4"; break;
    case CHANNEL5: return "CHANNEL5"; break;
    case CHANNEL6: return "CHANNEL6"; break;
    case CHANNEL7: return "CHANNEL7"; break;
    case UICC: return "UICC"; break;
    case TERMINAL: return "TERMINAL"; break;
    case NETWORK: return "NETWORK"; break;
    default: return "Unknown"; break;
    }

    return NULL;
}

// Duration class
IMPLEMENT_MODULE_TAG(Duration, Duration)
void Duration::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    memset(&m_tDuration, 0, sizeof(DURATION));
}

BYTE *Duration::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=(int)sizeof(DURATION) && m_pValue!=NULL)
    {
        memcpy(&m_tDuration, m_pValue, sizeof(DURATION));
    }

    return pRet;
}

void Duration::Set(int nTimeUnit, int nTimeInterval)
{
    Initialize();
    m_cTag = TLV_TAG;
    m_nLength = sizeof(DURATION);
    m_tDuration.cTimeUnit= (BYTE) nTimeUnit;
    m_tDuration.cTimeInterval = (BYTE) nTimeInterval;
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)&m_tDuration, m_nLength);
}

// Result class
IMPLEMENT_MODULE_TAG(Result, Result)
void Result::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_strAddtionalInfo = "";
    m_ptResult = NULL;
}

BYTE *Result::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=(int)sizeof(RESULT) && m_pValue!=NULL)
    {
        m_ptResult = (RESULT *)m_pValue;
        m_ptResult->cGeneralResult = m_pValue[0];
        if (m_nLength > 1)
        {
            memcpy(m_ptResult->cAdditionalInformation, m_pValue+1, m_nLength-1);
            char *szDump = new char[m_nLength-1];
            int nResult = Value2HexString(szDump, m_ptResult->cAdditionalInformation, m_nLength-1);
            if (nResult > 0) m_strAddtionalInfo = szDump;
            if(szDump!=NULL) delete[] szDump;
        }
    }
    return pRet;
}

void Result::Set(int nLength, int nGeneralResult, BYTE *cAdditionalInformation)
{
    Initialize();
    m_cTag = TLV_TAG | 0x80;
    m_nLength = nLength;
    m_pValue = new BYTE[m_nLength];
    memset(m_pValue, 0, m_nLength);
    m_ptResult = (RESULT *)m_pValue;
    m_ptResult->cGeneralResult = (BYTE) nGeneralResult;
    if (cAdditionalInformation) {
        //LOGI("AddtionalInfo");
        memcpy(m_ptResult->cAdditionalInformation, cAdditionalInformation, m_nLength-1);
        char *szDump = new char[m_nLength*2-1];
        int nResult = Value2HexString(szDump, m_ptResult->cAdditionalInformation, m_nLength-1);
        if (nResult > 0) m_strAddtionalInfo = szDump;
        if(szDump!=NULL) delete []szDump;
    }
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)m_ptResult, m_nLength);
}

// AlphaIdentifier class
IMPLEMENT_MODULE_TAG(AlphaIdentifier, AlphaIdentifier)
void AlphaIdentifier::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_strAlphaIdentifier = "";
}

BYTE *AlphaIdentifier::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>0 && m_pValue!=NULL)
    {
        char *szAlphaId = new char[m_nLength+1];
        if(szAlphaId!=NULL)
        {
            memset(szAlphaId, 0, m_nLength+1);
            memcpy(szAlphaId, (char *)m_pValue, m_nLength);
            szAlphaId[m_nLength] = '\0';
            m_strAlphaIdentifier = szAlphaId;
            delete []szAlphaId;
        }
    }

    return pRet;
}

void AlphaIdentifier::Set(const string &strAlphaIdentifier)
{
    Initialize();
    m_cTag = TLV_TAG;
    m_nLength = strAlphaIdentifier.size();
    m_strAlphaIdentifier = strAlphaIdentifier;
    m_pValue = new BYTE[m_nLength];
    m_strAlphaIdentifier.copy((char *)m_pValue, m_strAlphaIdentifier.size());
}

// IconIdentifier class
IMPLEMENT_MODULE_TAG(IconIdentifier, IconIdentifier)
void IconIdentifier::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    memset(&m_tIconIdentifier, 0, sizeof(ICON_IDENTIFIER));
}

BYTE *IconIdentifier::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=(int)sizeof(ICON_IDENTIFIER) && m_pValue!=NULL)
    {
        memcpy(&m_tIconIdentifier, m_pValue, sizeof(ICON_IDENTIFIER));
    }

    return pRet;
}

void IconIdentifier::Set(int nIconQualifier, int nIconIdentifier)
{
    Initialize();
    m_cTag = TLV_TAG;
    m_nLength = sizeof(ICON_IDENTIFIER);
    m_tIconIdentifier.cIconQualifier = (BYTE) nIconQualifier;
    m_tIconIdentifier.cIconIdentifier = (BYTE) nIconIdentifier;
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)&m_tIconIdentifier, m_nLength);
}

// TextString class
IMPLEMENT_MODULE_TAG(TextString, TextString)
TextString::TextString() : CTLV()
{
    m_pTextString = NULL;
    m_nTextStringLength = 0;
    m_CodingScheme = GSM7BITS;
}

void TextString::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_CodingScheme = GSM8BITS;
    m_nTextStringLength = 0;
    if(m_pTextString) delete []m_pTextString;
    m_pTextString = NULL;
}

BYTE *TextString::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>(int)sizeof(TEXT_STRING) && m_pValue!=NULL)
    {
        m_nTextStringLength = m_nLength - 1;      // 1 is CodingScheme
        TEXT_STRING *ptTextString = (TEXT_STRING *) m_pValue;
        m_CodingScheme = (CodingScheme) ptTextString->cCodingScheme;
        m_pTextString = new BYTE[m_nTextStringLength+1];
        memcpy(m_pTextString, ptTextString->acTextString, m_nTextStringLength);

        // Add null-termination
        if(m_CodingScheme==GSM8BITS) m_pTextString[m_nTextStringLength] = '\0';
    }

    return pRet;
}

void TextString::Set(CodingScheme CodeScheme, BYTE *pTextString, int nTextLength)
{
    Initialize();
    m_cTag = TLV_TAG;
    if(nTextLength>0)
    {
        m_CodingScheme = CodeScheme;
        m_nTextStringLength = nTextLength;
        memcpy(m_pTextString, pTextString, nTextLength);
    }
    m_nLength = m_nTextStringLength + 1;
    m_pValue = new BYTE[m_nLength];
    m_pValue[0] = m_CodingScheme;
    memcpy(m_pValue+1, (BYTE *)m_pTextString, m_nTextStringLength);
}

// Bearer description
IMPLEMENT_MODULE_TAG(BearerDescription, BearerDescription)
void BearerDescription::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_ptDescription = NULL;
    m_strParameter = "";
}

BYTE *BearerDescription::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);

    if(m_nLength>0 && m_pValue!=NULL)
    {
        m_ptDescription = (BEARER_DESC *) m_pValue;
        int nParamLength = (m_nLength - sizeof(BEARER_DESC)) * 2;
        char *pszParam = new char[nParamLength+1];      // 1 is for null-termination
        int nHexStrLen = Value2HexString(pszParam, m_ptDescription->acParameter, m_nLength-1);
        if(nHexStrLen>0) m_strParameter = pszParam;
        if(pszParam!=NULL) delete []pszParam;
    }

    return pRet;
}

void BearerDescription::Set(BYTE cBearerType, const BYTE *pBearerParameter)
{

    Initialize();
    m_cTag = TLV_TAG;
    switch(cBearerType)
    {
    case CSD:
        m_nLength = sizeof(BEARER_DESC) + sizeof(CSD_CBST);
        break;
    case GPRS:
        m_nLength = sizeof(BEARER_DESC) + sizeof(GPRS_CGQREQ);
        break;
        // the terminal shall provide its default available bearer parameter configuration. X (length of
        // parameters) = 0;
    case DEFAULT_BEARER:
        m_nLength = sizeof(BEARER_DESC) + 0;
        break;
        // in this case, X = variable, Contains Service Identifier" and "Service Record"
        // fields as defined in clause 8.63 and according to the Bearer Type coding
        // Ts_102.223
    case LOCAL_LINK:
    case BLUETOOTH:
    case IRDA:
    case RS232:
    case I_WLAN:
        m_nLength = sizeof(BEARER_DESC);
        break;
    case E_UTRAN:
        //Refer 27.22.4.27.6/2 (OPEN CHANNEL, immediate link establishment, E-UTRAN, bearer type '0B')
        m_nLength = 3;
        break;
    case USB:
        m_nLength = sizeof(BEARER_DESC);
        break;
    default: Initialize(); return;
    }

    m_pValue = new BYTE[m_nLength];       // 1 means Bearer Type
    m_ptDescription = (BEARER_DESC *) m_pValue;
    m_ptDescription->cType = cBearerType;
    memcpy(m_ptDescription->acParameter, pBearerParameter, m_nLength-sizeof(BEARER_DESC));

    int nParamLength = (m_nLength - sizeof(BEARER_DESC)) * 2;
    char *pszParam = new char[nParamLength+1];      // 1 is for null-termination
    int nHexStrLen = Value2HexString(pszParam, m_ptDescription->acParameter, m_nLength-1);
    if(nHexStrLen>0) m_strParameter = pszParam;

    memcpy(m_pValue, (BYTE *)m_ptDescription, m_nLength);
    if(pszParam!=NULL) delete []pszParam;
}

void BearerDescription::Set(BYTE cBearerType, const string &strBearerParameter)
{

    Initialize();
    m_cTag = TLV_TAG;
    switch(cBearerType)
    {
    case CSD:
        m_nLength = sizeof(BEARER_DESC) + sizeof(CSD_CBST);
        break;
    case GPRS:
        m_nLength = sizeof(BEARER_DESC) + sizeof(GPRS_CGQREQ);
        break;
        // the terminal shall provide its default available bearer parameter configuration. X (length of
        // parameters) = 0;
    case DEFAULT_BEARER:
        m_nLength = sizeof(BEARER_DESC) + 0;
        break;
        // in this case, X = variable, Contains Service Identifier" and "Service Record"
        // fields as defined in clause 8.63 and according to the Bearer Type coding
        // Ts_102.223
    case LOCAL_LINK:
    case BLUETOOTH:
    case IRDA:
    case RS232:
    case I_WLAN:
        m_nLength = sizeof(BEARER_DESC);
        break;
    case E_UTRAN:
        //Refer 27.22.4.27.6/2 (OPEN CHANNEL, immediate link establishment, E-UTRAN, bearer type '0B')
        m_nLength = 3;
        break;
    case USB:
        m_nLength = sizeof(BEARER_DESC);
        break;
    default: Initialize(); return;
    }

    m_strParameter = strBearerParameter;
    LOGI("m_strParameter: [%s]", m_strParameter.c_str());
    char *pParameter = new char[m_strParameter.size()];
    m_strParameter.copy(pParameter, m_strParameter.size());

    m_pValue = new BYTE[m_nLength];       // 1 means Bearer Type
    m_ptDescription = (BEARER_DESC *) m_pValue;
    m_ptDescription->cType = cBearerType;
    //int nByteValueLen = HexString2Value(m_ptDescription->acParameter, pParameter);

    if (cBearerType == E_UTRAN)
    {
        //Refer 27.22.4.27.6/2 (OPEN CHANNEL, immediate link establishment, E-UTRAN, bearer type '0B')
        EUTRAN_CGQREQ *temp_CGQREQ = (EUTRAN_CGQREQ *)m_ptDescription->acParameter;

        m_pValue[0] = m_ptDescription->cType;
        m_pValue[1] = temp_CGQREQ->cQCI; //QCI
        m_pValue[2] = temp_CGQREQ->cPdnType; // PDN type
    }
    else
    {
        memcpy(m_pValue, (BYTE *)m_ptDescription, m_nLength);
    }

    if(pParameter!=NULL) delete []pParameter;
}

// Channel data
IMPLEMENT_MODULE_TAG(ChannelData, ChannelData)
void ChannelData::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_strChannelData = "";
}

BYTE *ChannelData::Set(const BYTE *pData, int nLength)
{

    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>0 && m_pValue!=NULL)
    {
        char *pszParam = new char[m_nLength*2 + 1];      // 1 is for null-termination
        if(pszParam!=NULL)
        {
            int nHexStrLen = Value2HexString(pszParam, m_pValue, m_nLength);
            pszParam[nHexStrLen] = '\0';
            if(nHexStrLen>0) m_strChannelData = pszParam;
            delete []pszParam;
        }
    }

    return pRet;
}

void ChannelData::Set(const string &strChannelData)
{
    Initialize();
    m_cTag = TLV_TAG;
    m_strChannelData = strChannelData;
    char *pParameter = new char[m_strChannelData.size()];
    m_strChannelData.copy(pParameter, m_strChannelData.size());

    m_nLength = m_strChannelData.size()/2;
    m_pValue = new BYTE[m_nLength];
    //int nByteValueLen = HexString2Value(m_pValue, pParameter);
    if(pParameter!=NULL) delete []pParameter;
}

//Channel data Length
IMPLEMENT_MODULE_TAG(ChannelDataLength, ChannelDataLength)
void ChannelDataLength::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_nChannelDataLength = 0;
}

BYTE *ChannelDataLength::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=1 && m_pValue!=NULL)
    {
        m_nChannelDataLength = m_pValue[0];
    }
    return pRet;
}

void ChannelDataLength::Set(int nChannelDataLength)
{
    Initialize();
    m_cTag = TLV_TAG | 0x80;
    m_nLength = 1;
    m_nChannelDataLength = (BYTE)nChannelDataLength;
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, &m_nChannelDataLength, m_nLength);
    CTLV::Set(m_cTag, &m_nChannelDataLength, m_nLength);
}

//Channel Status
IMPLEMENT_MODULE_TAG(ChannelStatus, ChannelStatus)
void ChannelStatus::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    memset(m_szChannelStatus, 0, 2);
}

BYTE *ChannelStatus::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=2 && m_pValue!=NULL)
    {
        memcpy(m_szChannelStatus, m_pValue, m_nLength);
    }
    return pRet;
}

void ChannelStatus::Set(int nChannelId, Channel_Status channelStatus, bool isFurtherInfo, int furtherInfo)
{
    Initialize();
    m_cTag = TLV_TAG;
    m_nLength = 2;
    m_szChannelStatus[0] = ( (nChannelId & 0x7) | ((channelStatus & 0x3)<<6) );
    if(isFurtherInfo) {
        m_szChannelStatus[1] = (BYTE)furtherInfo;
    }
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)m_szChannelStatus, m_nLength);
}

BYTE ChannelStatus::GetRfu(int mode)
{
    if (mode == CS_PACKET_DATA_SERVICE)
    {
        return m_szChannelStatus[0] & 0x78;
    }
    else if (mode == UICC_SERVER || mode == TERMINAL_SERVER_AND_DIRECT_COMMUNICATION_CHANNEL)
    {
        return m_szChannelStatus[0] & 0x38;
    }
    else
    {
        return m_szChannelStatus[0] & 0xf8;
    }
}

//Buffer size
IMPLEMENT_MODULE_TAG(BufferSize, BufferSize)
void BufferSize::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_nBufferSize = 0;
}

BYTE *BufferSize::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=2 && m_pValue!=NULL)
    {
        m_nBufferSize = SWAP16( (*((int *)m_pValue)) );
    }
    return pRet;
}

void BufferSize::Set(int nBufferSize)
{
    Initialize();
    m_cTag = TLV_TAG;
    m_nLength = 2;
    m_nBufferSize = nBufferSize;
    m_pValue = new BYTE[m_nLength];
    WORD *pwSize = (WORD *) m_pValue;
    *pwSize = (WORD)(SWAP16(m_nBufferSize));
}

// Uicc/terminal interface transport level
IMPLEMENT_MODULE_TAG(TerminalInterfaceTransportLevel, TerminalInterfaceTransportLevel)
void TerminalInterfaceTransportLevel::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    memset(&m_tTransportLevel, 0, sizeof(TRANSPORT_LEVEL));
}

BYTE *TerminalInterfaceTransportLevel::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=(int)sizeof(TRANSPORT_LEVEL) && m_pValue!=NULL)
    {
        //memcpy(&m_tTransportLevel, m_pValue, sizeof(TRANSPORT_LEVEL));
        m_tTransportLevel.cTransportType = m_pValue[0];
        m_tTransportLevel.wPort = SWAP16( (*((WORD *)(m_pValue+1))) );
    }

    return pRet;
}

void TerminalInterfaceTransportLevel::Set(int nTransportType, int nPort)
{
    Initialize();
    m_cTag = TLV_TAG;
    m_nLength = sizeof(TRANSPORT_LEVEL);
    m_tTransportLevel.cTransportType = (BYTE) nTransportType;
    m_tTransportLevel.wPort = (WORD) nPort;
    m_pValue = new BYTE[m_nLength];
    m_pValue[0] = m_tTransportLevel.cTransportType;
    WORD *pwSize = (WORD *) (m_pValue+1);
    *pwSize = (WORD)(SWAP16(m_tTransportLevel.wPort));
}

//other address
IMPLEMENT_MODULE_TAG(OtherAddress, OtherAddress)
void OtherAddress::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    memset(&m_tOtherAddress, 0, sizeof(OTHER_ADDRESS));
}

BYTE *OtherAddress::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>IPV4_OCTET_SIZE && m_pValue!=NULL)
    {
        memcpy(&m_tOtherAddress, m_pValue, m_nLength);
    }

    return pRet;
}

void OtherAddress::Set(int nAddressType, const string &strAddress)
{
    Initialize();
    m_cTag = TLV_TAG;
    m_nLength = strAddress.size();
    m_tOtherAddress.cAddressType = (BYTE) nAddressType;
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)&m_tOtherAddress, m_nLength);
}

// Network access name
IMPLEMENT_MODULE_TAG(NetworkAccessName, NetworkAccessName)
void NetworkAccessName::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_strNetworkAccessName = "";
}

BYTE *NetworkAccessName::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>0 && m_pValue!=NULL)
    {
        char *pszNetAccessName = NetworkAccessNameParser();
        if(pszNetAccessName!=NULL)
        {
            m_strNetworkAccessName = pszNetAccessName;
            delete [] pszNetAccessName;
            if (m_strNetworkAccessName == "") LOGE("Network Access Name is NULL!!");
        }
    }
    return pRet;
}

char *NetworkAccessName::NetworkAccessNameParser()
{
    int len = m_nLength;
    int strLen = 0;
    int index = 0, strIndex = 0;
    char *strResult = new char[m_nLength * 2];

    while (len > 1)
    {
        strLen = m_pValue[index++];
        if (strLen < len)
        {
            strncpy(strResult+strIndex, (char *)(m_pValue+index), strLen);
            strIndex+=strLen;
            index+=strLen;

            len = len - (strLen + 1);
            if (len > 1)
            {
                strResult[strIndex++] = '.';
            }
            else
            {
                break;
            }
        }
    }
    strResult[strIndex] = '\0';
    return strResult;
}

void NetworkAccessName::Set(const string &strAddress)
{
    Initialize();
    m_cTag = TLV_TAG;
    m_nLength = strAddress.size();
    m_strNetworkAccessName = strAddress;
    LOGI("network access name : [%s]", m_strNetworkAccessName.c_str());
    m_pValue = new BYTE[m_nLength];
    m_strNetworkAccessName.copy((char *)m_pValue, m_strNetworkAccessName.size());
}

// UTRAN/E-UTRAN measurement qualifier
IMPLEMENT_MODULE_TAG(MeasurementQualifier, MeasurementQualifier)

void MeasurementQualifier::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_nQualifier = 0;
}

BYTE *MeasurementQualifier::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=1 && m_pValue!=NULL)
    {
        m_nQualifier = (int) m_pValue[0];
    }

    return pRet;
}

void MeasurementQualifier::Set(int nQualifier)
{
    Initialize();
    m_cTag = TLV_TAG;
    m_nLength = 1;
    m_nQualifier = nQualifier;
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)&m_nQualifier, m_nLength);
}

// TextAttribute class
IMPLEMENT_MODULE_TAG(TextAttribute, TextAttribute)
void TextAttribute::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_strTextFormatting = "";
}

BYTE *TextAttribute::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>0 && m_pValue!=NULL)
    {
        m_strTextFormatting = (char *) m_pValue;
    }

    return pRet;
}

void TextAttribute::Set(const string &strTextFormatting)
{
   Initialize();
   m_cTag = TLV_TAG;
   m_nLength = strTextFormatting.size();
   m_strTextFormatting = strTextFormatting;
   m_pValue = new BYTE[m_nLength];
   m_strTextFormatting.copy((char *)m_pValue, m_strTextFormatting.size());
}

// Frame identifier
IMPLEMENT_MODULE_TAG(FrameIdentifier, FrameIdentifier)
void FrameIdentifier::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_nIdentifierOfFrame = 0;
}

BYTE *FrameIdentifier::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=1 && m_pValue!=NULL)
    {
        m_nIdentifierOfFrame = (int) m_pValue[0];
    }

    return pRet;
}

void FrameIdentifier::Set(int nIdentifierOfFrame)
{
    Initialize();
    m_cTag = TLV_TAG;
    m_nLength = 1;
    m_nIdentifierOfFrame = nIdentifierOfFrame;
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)&m_nIdentifierOfFrame, m_nLength);
}

// RemoteEntityAddress class
IMPLEMENT_MODULE_TAG(RemoteEntityAddress, RemoteEntityAddress)
void RemoteEntityAddress::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    if(m_ptRemoteEntityAddress) delete m_ptRemoteEntityAddress;
    m_ptRemoteEntityAddress = NULL;
}

BYTE *RemoteEntityAddress::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>(int)sizeof(REMOTE_ENTITY_ADDRESS) && m_pValue!=NULL)
    {
        m_ptRemoteEntityAddress = (REMOTE_ENTITY_ADDRESS *) m_pValue;
    }

    return pRet;
}

void RemoteEntityAddress::Set(int nCodingType, BYTE *pAddress, LONG lLength)
{
    Initialize();
    m_cTag = TLV_TAG;
    if(lLength>0)
    {
        m_ptRemoteEntityAddress = (REMOTE_ENTITY_ADDRESS *) new BYTE[sizeof(REMOTE_ENTITY_ADDRESS)+lLength];
        memset(m_ptRemoteEntityAddress, 0, sizeof(REMOTE_ENTITY_ADDRESS)+lLength);
        m_ptRemoteEntityAddress->cCodingType = (BYTE) nCodingType;
        m_ptRemoteEntityAddress->lLength = lLength;
        memcpy(m_ptRemoteEntityAddress->acAddress, pAddress, lLength);
    }
    m_nLength = sizeof(REMOTE_ENTITY_ADDRESS) + lLength;
    m_pValue = new BYTE[m_nLength];
    memcpy(m_pValue, (BYTE *)m_ptRemoteEntityAddress, m_nLength);
}

// EventList class
IMPLEMENT_MODULE_TAG(EventList, EventList)
void EventList::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_pEventList = NULL;
}

BYTE *EventList::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_nLength>=1 && m_pValue!=NULL)
    {
        m_pEventList = new BYTE[m_nLength];
        memcpy(m_pEventList, m_pValue, m_nLength);
    }

    return pRet;
}

void EventList::Set(BYTE cEvent)
{
    m_cTag = TLV_TAG;
    m_nLength = 1;
    m_pEventList = new BYTE[m_nLength];
    if(m_pEventList==NULL)
    {
        LOGE("m_pEventList = NULL");
        return;
    }

    m_pEventList[0] = cEvent;
    m_pValue = new BYTE[m_nLength];
    if(m_pValue==NULL)
    {
        LOGE("m_pValue = NULL");
        delete [] m_pEventList;
        m_pEventList = NULL;
        return;
    }

    memcpy(m_pValue, m_pEventList, m_nLength);
    delete []m_pEventList;
    m_pEventList = NULL;
}
