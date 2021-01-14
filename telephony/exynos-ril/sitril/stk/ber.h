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

#ifndef __BER_H__
#define __BER_H__

#include <stddef.h>

#include "tlv.h"
#include "comprehension.h"

class CBER : public CTLV
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(0xD0)

public:
    CBER();
    CBER(const BYTE *pData, int nLength);
    CBER(BYTE cTag, const BYTE *pValue, int nLength);
    virtual ~CBER();

    //static const int TAG_PROACTIVE_CMD = 0xD0;

    virtual BYTE *Set(const BYTE *pData, int nLength);
    virtual void Set(int nLength, const BYTE *pValue);

    BOOL IsValid() { return m_bValid; }
    Comprehension *GetComprehension();

protected:
    BOOL m_bValid;

    virtual void Initialize();
    virtual void Finalize();
};

class CEnvelopeBER : public CBER
{
    DECLARE_MODULE_TAG()
    DECLARE_TLV_TAG(0xD6)

public:
    CEnvelopeBER() : CBER() { }
    CEnvelopeBER(const BYTE *pData, int nLength) : CBER(pData, nLength) { }
    CEnvelopeBER(BYTE cTag, const BYTE *pValue, int nLength) : CBER(cTag, pValue, nLength) { }
    virtual ~CEnvelopeBER();

    virtual void Set(int nLength, const BYTE *pValue);
};
#endif // __BER_H__
