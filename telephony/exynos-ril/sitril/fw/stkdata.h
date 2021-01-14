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
 * stkdata.h
 *
 *  Created on: 2014. 10. 6.
 *      Author: MOX
 */

#ifndef _STK_DATA_H_
#define _STK_DATA_H_

#include "requestdata.h"


/**
 * STK Data (Envelope Command, Terminal Response, Envelope With Status)
 */
class StkData
{
public:
    StkData();
    ~StkData();

    BYTE *GetData() { return m_pData; }
    int GetLength() { return m_nLength; }

    BOOL Parse(StringRequestData *pString);
    BOOL Parse(StringsRequestData *pString);
    BOOL Parse(const BYTE *pData, int nLength);
    void Erase(void);

private:
    int m_nLength;
    BYTE *m_pData;
};

class StkEfidList
{
public:
    StkEfidList(int nCount);
    ~StkEfidList();

    int GetCount(void) { return m_nCount; }
    BOOL Insert(int nIndex, UINT uEFID);
    UINT GetAt(int nIndex);

    void SetAID(BYTE *pAID, int nLength) { memcpy(m_acAID, pAID, nLength); m_nAidLength = nLength; }
    int GetAidLength(void) { return m_nAidLength; }
    BYTE *GetAID(void) { return (BYTE *) m_acAID; }

private:
    int m_nCount;
    UINT *m_pEFID;
    int m_nAidLength;
    BYTE m_acAID[MAX_SIM_AID_LEN];
};

#endif /*_STK_DATA_H_*/
