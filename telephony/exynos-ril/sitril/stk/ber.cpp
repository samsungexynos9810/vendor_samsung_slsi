/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "ber.h"
#include "rillog.h"
#include "comprehension.h"

static bool debug = true;

// add category to display selective logs
#undef LOGV
#define LOGV(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_VERBOSE_LOG,  "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGI
#define LOGI(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_INFO_LOG, "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGW
#define LOGW(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_WARNING_LOG,  "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)
#undef LOGE
#define LOGE(format, ...) do { if (debug) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_CRITICAL_LOG,  "%s::%s() " format, TAG, __FUNCTION__, ##__VA_ARGS__);} while(0)

IMPLEMENT_MODULE_TAG(CBER, BER)

CBER::CBER() : CTLV(), m_bValid(false)
{
}

CBER::CBER(const BYTE *pData, int nLength) : m_bValid(false)
{
    LOGI("");
    CBER();
    Set(pData, nLength);
}

CBER::CBER(BYTE cTag, const BYTE *pValue, int nLength) : CTLV(cTag, pValue, nLength), m_bValid(false)
{
    LOGI("");
}

CBER::~CBER()
{
    Finalize();
}

void CBER::Initialize()
{
    LOGI("");
    CTLV::Initialize();
    m_bValid = false;
}

void CBER::Finalize()
{
    CTLV::Finalize();
    m_bValid = false;
}

BYTE *CBER::Set(const BYTE *pData, int nLength)
{
    BYTE *pRet = CTLV::Set(pData, nLength);
    if(m_cTag == TLV_TAG) m_bValid = true;

    return pRet;
}

void CBER::Set(int nLength, const BYTE *pValue)
{
    if(pValue!=NULL && nLength>0)
    {
        Initialize();

        m_cTag = TLV_TAG;
        m_nLength = nLength;
        m_pValue = new BYTE[nLength];
        memcpy(m_pValue, pValue, nLength);
        m_bValid = true;
    }
}

Comprehension *CBER::GetComprehension()
{
    //return (m_pValue==NULL)? NULL: new Comprehension((const BYTE *) m_pValue, m_nLength);
    Comprehension *pComprehension = NULL;
    if(m_pValue)
    {
        pComprehension = new Comprehension();
        pComprehension->Set((const BYTE *) m_pValue, m_nLength);
    }
    return pComprehension;
}

IMPLEMENT_MODULE_TAG(CEnvelopeBER, EnvelopeBER)

CEnvelopeBER::~CEnvelopeBER()
{
    Finalize();
}

void CEnvelopeBER::Set(int nLength, const BYTE *pValue)
{
    if(pValue!=NULL && nLength>0)
    {
        Initialize();

        m_cTag = TLV_TAG;
        m_nLength = nLength;
        m_pValue = new BYTE[nLength];
        memcpy(m_pValue, pValue, nLength);
        m_bValid = true;
    }
}
