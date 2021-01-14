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
 * tlv.h
 *
 *  Created on: 2017.03.14.
 *      Author: MOX
 */

#ifndef __TLV_H__
#define __TLV_H__

#include <stddef.h>
#include "types.h"
#include "stkdef.h"

#define DECLARE_TLV_TAG(tag)   static const BYTE TLV_TAG = tag;

class CTLV {
    DECLARE_MODULE_TAG()

public:
    CTLV();
    CTLV(const BYTE *pData, int nLength);
    CTLV(BYTE cTag, const BYTE *pValue, int nLength);
    virtual ~CTLV();

    static const BYTE TLV_TAG_MASK = 0x7F;

    virtual CTLV *Clone();

    BYTE GetTag() { return (m_cTag & TLV_TAG_MASK); }
    const char *GetTagString();
    int GetLength() { return m_nLength; }
    BYTE *GetValue() const { return m_pValue; }

    virtual BYTE *Set(const BYTE *pData, int nLength);
    void Set(BYTE cTag, const BYTE *pValue, int nLength);
    virtual BYTE *GetRawData();
    virtual int GetRawDataLength();

protected:
    BYTE m_cTag;
    int m_nLength;
    BYTE *m_pValue;

    BYTE *m_pRawData;

    virtual void Initialize();
    virtual void Finalize();
};
#endif // __TLV_H__
