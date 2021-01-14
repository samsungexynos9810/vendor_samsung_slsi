/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __MODEM_DATA_H__
#define __MODEM_DATA_H__

#include "rildef.h"

class ModemData
{
public:
    ModemData(const char * pRawData, int len, bool tx=true);
    virtual ~ModemData();

private:
    void SetData(const char *buf, int len, bool tx);
    void Release();

public:
    const char * GetRawData() const {return m_pRawData;}
    UINT GetLength() const {return m_nLen;}
    UINT GetToken() const;
    UINT GetType() const;
    UINT GetMessageId() const;
    bool IsRequest() const;
    bool IsResponse() const;
    bool IsUnsolicitedResponse() const;
    bool IsSolicitedResponse() const;

    bool IsTx() const { return m_bTx;}

    void Dump() const;

public:
    virtual ModemData *Clone() const;

public:
    virtual ModemData &operator=(const ModemData &rhs);

private:
    char * m_pRawData;
    UINT m_nLen;
    bool m_bTx;
};

#endif // __MODEM_DATA_H__
