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
 * comprehension.h
 *
 *  Created on: 2017.02.21.
 *      Author: MOX
 */

#ifndef __COMPREHENSION_H__
#define __COMPREHENSION_H__

#include <stddef.h>

#include <vector>
#include "tlv.h"
using namespace std;

class Comprehension
{
    DECLARE_MODULE_TAG()

public:
    Comprehension();
    //Comprehension(vector<CTLV *> vectorTlvs);
    virtual ~Comprehension();

    virtual Comprehension *Clone();

    void Set(const BYTE *pData, int nLength);
    void Set(vector<CTLV *> vectorTlvs);
    BYTE *GetRawData() { return m_pData; }
     int GetRawDataLength() { return m_nLength; }

    int GetTlvCount() { return m_vectorTlvs.size(); }
    CTLV *GetTlv(BYTE cTag);
    CTLV *GetTlv(int nIndex);

    void AppendTlv(CTLV *pTlv);

protected:
    int m_nLength;
    BYTE *m_pData;
    vector<CTLV *> m_vectorTlvs;

    void Initialize();
    void Finalize();
    void ClearVector();

    CTLV *NewTlv(BYTE cTag);
    CTLV *NewTlv(const BYTE *pData, int nLength);
};
#endif // __COMPREHENSION_H__
