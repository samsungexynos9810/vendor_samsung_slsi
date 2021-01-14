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
 * stkdata.cpp
 *
 *  Created on: 2014. 10. 6.
 *      Author: MOX
 */

#include "stkdata.h"
#include "util.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/**
 * STK Data (Envelope Command, Terminal Response, Envelope With Status)
 */
StkData::StkData()
{
    m_nLength = 0;
    m_pData = NULL;
}

StkData::~StkData()
{
    Erase();
}

BOOL StkData::Parse(StringRequestData *pString)
{
    RilLogI("StkData::%s()", __FUNCTION__);
    if(pString==NULL || pString->GetString()==NULL
            || strlen(pString->GetString())==0) return FALSE;

    Erase();
    m_pData = new BYTE[strlen(pString->GetString())/2];
    m_nLength = HexString2Value(m_pData, pString->GetString());
    RilLogI("StkData::%s(), HexValue Length:%d", __FUNCTION__, m_nLength);

    return TRUE;
}

BOOL StkData::Parse(StringsRequestData *pString)
{
    if(pString==NULL || pString->GetStringCount()==0) return FALSE;

    Erase();
    m_pData = new BYTE[strlen(pString->GetString(0))/2];
    m_nLength = HexString2Value(m_pData, pString->GetString(0));

    return TRUE;
}

BOOL StkData::Parse(const BYTE *pData, int nLength)
{
    if(pData==NULL || nLength==0) return FALSE;

    Erase();
    m_pData = new BYTE[nLength];
    memcpy(m_pData, pData, nLength);
    m_nLength = nLength;

    return TRUE;
}

void StkData::Erase(void)
{
    if(m_pData!=NULL)
    {
        delete [] m_pData;
        m_pData = NULL;
    }
}

StkEfidList::StkEfidList(int nCount)
{
    m_nCount = nCount;
    m_pEFID = new UINT[nCount];
    m_nAidLength = 0;
    memset(m_acAID, 0, sizeof(m_acAID));
}

StkEfidList::~StkEfidList()
{
    if(m_pEFID!=NULL)
    {
        delete [] m_pEFID;
        m_pEFID = NULL;
    }
}

BOOL StkEfidList::Insert(int nIndex, UINT uEFID)
{
    if(nIndex<m_nCount)
    {
        m_pEFID[nIndex] = uEFID;
        return TRUE;
    }

    return FALSE;
}

UINT StkEfidList::GetAt(int nIndex)
{
    return (nIndex<m_nCount)? m_pEFID[nIndex]: 0;
}

