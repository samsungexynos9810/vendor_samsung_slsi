/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "tlv.h"
#include "rillog.h"
#include "util.h"

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

#undef ENTER_FUNC
#define ENTER_FUNC()        { LOGI("[<--"); }
#undef LEAVE_FUNC
#define LEAVE_FUNC()        { LOGI("[-->"); }

IMPLEMENT_MODULE_TAG(CTLV, TLV)

CTLV::CTLV()
{
    m_cTag = 0;
    m_nLength = 0;
    m_pValue = NULL;

    m_pRawData = NULL;
}

CTLV::CTLV(const BYTE *pData, int nLength)
{
    CTLV();
    Set(pData, nLength);
}

CTLV::CTLV(BYTE cTag, const BYTE *pValue, int nLength)
{
    CTLV();
    Set(cTag, pValue, nLength);
}

CTLV::~CTLV()
{
    Finalize();
}

CTLV *CTLV::Clone()
{
    CTLV *pTlv = new CTLV(m_cTag, m_pValue, m_nLength);
    return pTlv;
}

void CTLV::Initialize()
{
    m_cTag = 0;
    m_nLength = 0;

    if(m_pValue!=NULL) delete []m_pValue;
    m_pValue = NULL;

    if (m_pRawData!=NULL) delete []m_pRawData;
    m_pRawData = NULL;
}

void CTLV::Finalize()
{
    Initialize();
}

const char *CTLV::GetTagString()
{
    switch(GetTag())
    {
    case TAG_COMMAND_DETAIL: return "TAG_COMMAND_DETAIL"; break;
    case TAG_DEVICE_IDENTITY: return "TAG_DEVICE_IDENTITY"; break;
    case TAG_RESULT: return "TAG_RESULT"; break;
    case TAG_DURATION: return "TAG_DURATION"; break;
    case TAG_ALPHA_IDENTIFIER: return "TAG_ALPHA_IDENTIFIER"; break;
    case TAG_ADDRESS: return "TAG_ADDRESS"; break;
    case TAG_SUB_ADDRESS: return "TAG_SUB_ADDRESS"; break;
    case TAG_TEXT_STRING: return "TAG_TEXT_STRING"; break;
    case TAG_EVENT_LIST: return "TAG_EVENT_LIST"; break;
    case TAG_ICON_IDENTIFIER: return "TAG_ICON_IDENTIFIER"; break;
    case TAG_BEARER_DESCRIPTION: return "TAG_BEARER_DESCRIPTION"; break;
    case TAG_CHANNEL_DATA: return "TAG_CHANNEL_DATA"; break;
    case TAG_CHANNEL_DATA_LENGTH: return "TAG_CHANNEL_DATA_LENGTH"; break;
    case TAG_CHANNEL_STATUS: return "TAG_CHANNEL_STATUS"; break;
    case TAG_BUFFER_SIZE: return "TAG_BUFFER_SIZE"; break;
    case TAG_UICC_TERMINAL_INTERFACE_TRANSPORT_LEVEL: return "TAG_UICC_TERMINAL_INTERFACE_TRANSPORT_LEVEL"; break;
    case TAG_OTHER_ADDRESS: return "TAG_OTHER_ADDRESS"; break;
    case TAG_NETWORK_ACCESS_NAME: return "TAG_NETWORK_ACCESS_NAME"; break;
    case TAG_REMOTE_ENTITY_ADDRESS: return "TAG_REMOTE_ENTITY_ADDRESS"; break;
    case TAG_TEXT_ATTRIBUTE: return "TAG_TEXT_ATTRIBUTE"; break;
    case TAG_FRAME_IDENTIFIER: return "TAG_FRAME_IDENTIFIER"; break;
    case TAG_UTRAN_EUTRAN_MEASUREMENT_QUALIFIER: return "TAG_UTRAN_EUTRAN_MEASUREMENT_QUALIFIER"; break;
    case TAG_CSG_ID_LIST: return "TAG_CSG_ID_LIST"; break;
    default: return "Unknown"; break;
    }
}

BYTE *CTLV::GetRawData()
{
    BYTE *pData = NULL;

    if(m_pValue!=NULL && m_nLength>0)
    {
        // Single Byte Format
        if(m_nLength==1) {
            pData = new BYTE[3];
            pData[0] = m_cTag;
            pData[1] = m_nLength;
            pData[2] = m_pValue[0];
        }
        // Three-Byte Format
        else if(m_nLength==3 && *m_pValue==0x7F) { }
        else
        {
            int nLength = 3;    // Tag(1), Length(1), Value(1)
            // Length size is variable. Multi-byte should be considered futher.
            //if(m_nLength>=128 && m_nLength<=255) nLength++;
            //else if(m_nLength>=256 && m_nLength<=65535) nLength += 2;
            //else if(m_nLength>=65536) nLength += 3;
            //else return NULL;

            nLength = m_nLength - 1;    // Already 1 byte assigned
            pData = new BYTE[nLength];
            memset(pData, 0, nLength);

            int nIdx = 0;
            // tag
            pData[nIdx++] = m_cTag;
            // length
            if(m_nLength<128) pData[nIdx++] = (BYTE) m_nLength;
            else if(m_nLength>=128 && m_nLength<=255)
            {
                pData[nIdx++] = 0x81;
                pData[nIdx++] = (BYTE) m_nLength;
            }
            else if(m_nLength>=256 && m_nLength<=65535)
            {
                pData[nIdx++] = 0x82;
                BYTE acLength[4];
                memcpy(acLength, &m_nLength, 4);
                memcpy(&pData[nIdx], acLength, 2);
                nIdx += 2;
            }
            else if(m_nLength>=65536)
            {
                pData[nIdx++] = 0x83;
                BYTE acLength[4];
                memcpy(acLength, &m_nLength, 4);
                memcpy(&pData[nIdx], acLength, 3);
                nIdx += 3;
            }

            // value
            memcpy(&pData[nIdx], m_pValue, m_nLength);
            m_pRawData = pData;
        }
    }

    return pData;
}

int CTLV::GetRawDataLength()
{
    int nLength = 0;
    if(m_nLength>0)
    {
        if(m_nLength<128) nLength = 1;
        else if(m_nLength>=128 && m_nLength<=255) nLength = 2;
        else if(m_nLength>=256 && m_nLength<=65535) nLength = 3;
        else if(m_nLength>=65536) nLength = 4;

        nLength += (1 + m_nLength);     // 1 means TAG size
    }

    return nLength;
}

// return Next TLV byte stream
BYTE *CTLV::Set(const BYTE *pData, int nLength)
{
    ENTER_FUNC();
    LOGI("pData:0x%08X, nLength:%d", pData, nLength);

    if(pData==NULL || nLength==0) return NULL;

    BYTE *pValue = (BYTE *) pData;

    Initialize();

    // Single Byte Format
    if(nLength==1)
    {
        LOGI("Single byte format:0x%02X", *pValue);
        pValue++;
    }
    // Three-Byte Format
    else if(nLength==3 && *pValue==0x7F)
    {
        LOGI("Three-byte format:0x%02X, 0x%02X, 0x%02X", *pValue, *(pValue+1), *(pValue+2));
        pValue += 3;
    }
    else
    {
        m_cTag = *pValue;
        LOGI("Tag:0x%02X(%s)", m_cTag, GetTagString());
        pValue++;

        LOGI("First byte of length:0x%02X", *pValue);

        // Calculate Length
        if(nLength>1 && *pValue<0x80)
        {
            m_nLength = (int) *pValue;
            pValue++;
        }
        else if(nLength>2 && *pValue==0x81)
        {
            m_nLength = (int) *++pValue;
            pValue++;
        }
        else if(nLength>3 && *pValue==0x82)
        {
            pValue++;
            short sLen;
            memcpy(&sLen, pValue, 2);
            pValue += 2;
            m_nLength = (int) sLen;
        }
        else if(nLength>4 && *pValue==0x83)
        {
            pValue++;
            int nLen;
            memcpy(&nLen, pValue, 3);
            pValue += 3;
            m_nLength = nLen;
        }
        else
        {
            LOGE("Wrong Length:%d(0x%02X)", *pValue, *pValue);
            Initialize();
            return NULL;
        }

        LOGI("Length:%d", m_nLength);
        LOGI("pData:0x%08X, pValue:0x%08X", pData, pValue);

        // 2byte contains tag and length
        if(nLength==2)
        {
            m_nLength = 0;
            m_pValue = NULL;
        }
        else if((pValue - pData) > 0 && nLength-(pValue - pData)>=m_nLength)
        {
            m_pValue = new BYTE[m_nLength];
            memcpy(m_pValue, pValue, m_nLength);
            pValue += m_nLength;
        }
        else
        {
            LOGI("Maybe wrong length? %d", m_nLength);
            m_nLength = 0;
            m_pValue = NULL;
            pValue = NULL;
        }

        LOGI("Tag:0x%02X, Length: %d, Value:0x%08X", m_cTag, m_nLength, m_pValue);
    }

    pValue = (nLength-(pValue-pData))>0? pValue : NULL;
    LOGI("Next:0x%08X", pValue);
    LEAVE_FUNC();
    return pValue;
}

void CTLV::Set(BYTE cTag, const BYTE *pValue, int nLength)
{
    LOGI("TLV SET for tag[0x%x]", cTag);
    //PrintBufferDump("TLV", pValue, nLength);
    if(pValue==NULL || nLength==0)
    {
        LOGE("Tag:%c, pValue:%X, nLength:%d", cTag, pValue, nLength);
        return;
    }

    if(pValue!=NULL && nLength>0)
    {
        m_cTag = cTag;
        m_nLength = nLength;
        m_pValue = new BYTE[nLength];
        memcpy(m_pValue, pValue, nLength);
    }
}
