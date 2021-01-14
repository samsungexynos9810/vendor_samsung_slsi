/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "comprehension.h"
#include "rillog.h"
#include "tlvparser.h"

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

IMPLEMENT_MODULE_TAG(Comprehension, Comprehension)

Comprehension::Comprehension()
{
    ENTER_FUNC();
    //LOGI("this:0x%08X", this);
    m_nLength = 0;
    m_pData = NULL;
    LEAVE_FUNC();
}

/*
Comprehension::Comprehension(vector<CTLV *> vectorTlvs)
{
    Comprehension();
    Initialize();

    for(auto each : vectorTlvs) AppendTlv(each);
}
*/

Comprehension::~Comprehension()
{
    ENTER_FUNC();
    //LOGI("this:0x%08X", this);
    Finalize();
    LEAVE_FUNC();
}

Comprehension *Comprehension::Clone()
{
    Comprehension *pComprehension = new Comprehension();
    pComprehension->Set(m_vectorTlvs);
    return pComprehension;
}

void Comprehension::Initialize()
{
    if(m_pData)
    {
        delete[] m_pData;
        m_pData = NULL;
    }

    ClearVector();
}

void Comprehension::Finalize()
{
    Initialize();
}

void Comprehension::ClearVector()
{
    while(m_vectorTlvs.empty()==false)
    {
        CTLV *pTlv = m_vectorTlvs.back();
        delete pTlv;
        m_vectorTlvs.pop_back();
    }
}

void Comprehension::Set(const BYTE *pData, int nLength)
{
    ENTER_FUNC();
    //LOGI("this:0x%08X", this);
    Initialize();

    BYTE *pNext = (BYTE *) pData;
    int nRemain = nLength - (pNext - pData);
    while(pNext && nRemain>0)
    {
        LOGI("Remain Length:%d byte", nRemain);
        CTLV *pTlv = new CTLV();

        BYTE cTag = *pNext & CTLV::TLV_TAG_MASK;
        if((pTlv=NewTlv(cTag))!=NULL)
        {
            if((pNext=pTlv->Set(pNext, nRemain))!=NULL)
            {
                nRemain = nLength - (pNext - pData);
            }

            //m_vectorTlvs.push_back(pTlv);
            AppendTlv(pTlv);
            LOGI("pNext:0x%08X pData:0x%08X", pNext, pData);
        }
        else
        {
            LOGE("pTlv is NULL");
            pNext = NULL;
        }
    }

/*
    // Remained Data as Unknown
    if((m_nLength = nLength - nRemain)>0)
    {
        m_pData = new BYTE[m_nLength];
        memcpy(m_pData, pData, m_nLength);
    }
*/

    LEAVE_FUNC()
}

void Comprehension::Set(vector<CTLV *> vectorTlvs)
{
    ENTER_FUNC();
    Initialize();

    for(auto each : vectorTlvs) AppendTlv(each->Clone());

    LEAVE_FUNC()
}

// Return pointer needs to be deleted after use
/*
BYTE *Comprehension::GetRawData()
{
    int nRawDataLength = GetRawDataLength();
    BYTE *pRawData = new BYTE[nRawDataLength];
    memset(pRawData, 0, nRawDataLength);
    BYTE *ptr = pRawData;

    for(auto each : m_vectorTlvs)
    {
        if((ptr-pRawData)>=0)
        {
            int nLength = each->GetRawDataLength();
            if(nLength>0)
            {
                BYTE *pEachRawData = each->GetRawData();
                if(pEachRawData!=NULL)
                {
                    memcpy(ptr, pEachRawData, nLength);
                    ptr += nLength;
                    delete pEachRawData;
                }
            }
        }
    }

    return pRawData;
}

int Comprehension::GetRawDataLength()
{
    int nLength = 0;

    // Accumulate each tlv's raw data length
    for(auto each : m_vectorTlvs)
    {
        nLength += each->GetRawDataLength();
        LOGI("TAG: [0x%x], GetRawDataLength: [%d], nLength: [%d]", each->GetTag(), each->GetRawDataLength(), nLength);
    }

    return nLength;
}
*/

CTLV *Comprehension::NewTlv(BYTE cTag)
{
    ENTER_FUNC();
    CTLV *pTlv = NULL;

    switch(cTag)
    {
    case TAG_COMMAND_DETAIL:
        pTlv = new CommandDetail();
        break;
    case TAG_DEVICE_IDENTITY:
        pTlv = new DeviceIdentity();
        break;
    case TAG_RESULT:
        pTlv = new Result();
        break;
    case TAG_ALPHA_IDENTIFIER:
        pTlv = new AlphaIdentifier();
        break;
    case TAG_TEXT_STRING:
        pTlv = new TextString();
        break;
    case TAG_EVENT_LIST:
        pTlv = new EventList();
        break;
    case TAG_BEARER_DESCRIPTION:
        pTlv = new BearerDescription();
        break;
    case TAG_CHANNEL_DATA:
        pTlv = new ChannelData();
        break;
    case TAG_CHANNEL_DATA_LENGTH:
        pTlv = new ChannelDataLength();
        break;
    case TAG_CHANNEL_STATUS:
        pTlv = new ChannelStatus();
        break;
    case TAG_BUFFER_SIZE:
        pTlv = new BufferSize();
        break;
    case TAG_UICC_TERMINAL_INTERFACE_TRANSPORT_LEVEL:
        pTlv = new TerminalInterfaceTransportLevel();
        break;
    case TAG_OTHER_ADDRESS:
        pTlv = new OtherAddress();
        break;
    case TAG_NETWORK_ACCESS_NAME:
        pTlv = new NetworkAccessName();
        break;
    case TAG_UTRAN_EUTRAN_MEASUREMENT_QUALIFIER:
        pTlv = new MeasurementQualifier();
        break;
    case TAG_CSG_ID_LIST:
        break;
    case TAG_ICON_IDENTIFIER:
        pTlv = new IconIdentifier();
        break;
    case TAG_ADDRESS:
        pTlv = new Address();
        break;
    case TAG_SUB_ADDRESS:
        pTlv = new Subaddress();
        break;
    case TAG_DURATION:
        pTlv = new Duration();
        break;
    case TAG_TEXT_ATTRIBUTE:
        pTlv = new TextAttribute();
        break;
    case TAG_FRAME_IDENTIFIER:
        pTlv = new FrameIdentifier();
        break;
    case TAG_REMOTE_ENTITY_ADDRESS:
        pTlv = new RemoteEntityAddress();
        break;
   default:
        LOGE("Unknown Tag:0x%02X", cTag);
        pTlv = new CTLV();
        break;
    }

    LEAVE_FUNC()
    return pTlv;
}

CTLV *Comprehension::NewTlv(const BYTE *pData, int nLength)
{
    ENTER_FUNC();
    CTLV *pTlv = NULL;
    BYTE cTag = *pData & CTLV::TLV_TAG_MASK;

    switch(cTag)
    {
    case TAG_COMMAND_DETAIL:
        pTlv = new CommandDetail(pData, nLength);
        break;
    case TAG_DEVICE_IDENTITY:
        pTlv = new DeviceIdentity(pData, nLength);
        break;
    case TAG_RESULT:
        pTlv = new Result(pData, nLength);
        break;
    case TAG_ALPHA_IDENTIFIER:
        pTlv = new AlphaIdentifier(pData, nLength);
        break;
    case TAG_TEXT_STRING:
        pTlv = new TextString(pData, nLength);
        break;
    case TAG_EVENT_LIST:
        pTlv = new EventList(pData, nLength);
        break;
    case TAG_BEARER_DESCRIPTION:
        pTlv = new BearerDescription(pData, nLength);
        break;
    case TAG_CHANNEL_DATA:
        pTlv = new ChannelData(pData, nLength);
        break;
    case TAG_CHANNEL_DATA_LENGTH:
        pTlv = new ChannelDataLength(pData, nLength);
        break;
    case TAG_CHANNEL_STATUS:
        pTlv = new ChannelStatus(pData, nLength);
        break;
    case TAG_BUFFER_SIZE:
        pTlv = new BufferSize(pData, nLength);
        break;
    case TAG_UICC_TERMINAL_INTERFACE_TRANSPORT_LEVEL:
        pTlv = new TerminalInterfaceTransportLevel(pData, nLength);
        break;
    case TAG_OTHER_ADDRESS:
        pTlv = new OtherAddress(pData, nLength);
        break;
    case TAG_NETWORK_ACCESS_NAME:
        pTlv = new NetworkAccessName(pData, nLength);
        break;
    case TAG_UTRAN_EUTRAN_MEASUREMENT_QUALIFIER:
        pTlv = new MeasurementQualifier(pData, nLength);
        break;
    case TAG_CSG_ID_LIST:
        break;
    default:
        LOGE("Unknown Tag:0x%02X", cTag);
        pTlv = new CTLV();
        break;
    }

    LEAVE_FUNC()
    return pTlv;
}

CTLV *Comprehension::GetTlv(BYTE cTag)
{
    for(auto each : m_vectorTlvs)
    {
        LOGI("Tag: 0x%02X (%s)", each->GetTag(), each->GetTagString());
        if(cTag==each->GetTag()) return each;
    }

    return NULL;
}

CTLV *Comprehension::GetTlv(int nIndex)
{
    if(nIndex>(int)m_vectorTlvs.size()) return NULL;

    return m_vectorTlvs.at(nIndex);
}

void Comprehension::AppendTlv(CTLV *pTlv)
{
    m_vectorTlvs.push_back(pTlv);

    BYTE *pTlvData = pTlv->GetRawData();
    int nTlvLength = pTlv->GetRawDataLength();

    BYTE *pData = new BYTE[m_nLength+nTlvLength];
    if(m_pData!=NULL)
    {
        memset(pData, 0, nTlvLength);
        memcpy(pData, m_pData, m_nLength);
        delete[] m_pData;
    }

    if(pTlvData!=NULL)
    {
        memcpy(pData+m_nLength, pTlvData, nTlvLength);
        delete[] pTlvData;
        m_nLength += nTlvLength;
    }
    m_pData = pData;
}
