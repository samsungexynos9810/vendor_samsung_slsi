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
 * nvitemdata.h
 *
 *  Created on: 2015. 12. 22.
 */

#ifndef __NV_ITEMDATA_H__
#define __NV_ITEMDATA_H__

#include "requestdata.h"

class NvReadItemRequestData : public RequestData
{
private:
    int m_nvItemId;
    void *param;

public:
    NvReadItemRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~NvReadItemRequestData();
    virtual NvReadItemRequestData *Clone() const;
    virtual int encode(char *data, unsigned int datalen);
public:
    int GetNvItemID() const;
};

class NvWriteItemRequestData : public RequestData
{
private:
    int m_nvItemId;
    char m_value[MAX_NV_ITEM_SIZE * 2];
    void *param;

public:
    NvWriteItemRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~NvWriteItemRequestData();
    virtual NvWriteItemRequestData *Clone() const;
    virtual int encode(char *data, unsigned int datalen);
public:
    int GetNvItemID() const;
    const char *GetValue() const;
    int GetValueLength() const;
};

#endif /* __NV_ITEMDATA_H__ */
