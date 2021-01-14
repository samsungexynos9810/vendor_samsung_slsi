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
 * nvitemdata.cpp
 *
 *  Created on: 2015. 12. 22.
 */
#include <telephony/ril_nv_items.h>
#include "nvitemdata.h"

/**
 * NvReadItemRequestData
 */
NvReadItemRequestData::NvReadItemRequestData(const int nReq, const Token tok, const ReqType type)
    : RequestData(nReq, tok, type)
{
    m_nvItemId = -1;
    param = NULL;
}

NvReadItemRequestData::~NvReadItemRequestData()
{

}

int NvReadItemRequestData::encode(char *data, unsigned int datalen)
{
    if (data == NULL || datalen == 0 || datalen != sizeof(RIL_NV_ReadItem)) {
        return -1;
    }

    RIL_NV_ReadItem *nvri = (RIL_NV_ReadItem *)data;

    m_nvItemId = (int)nvri->itemID;
    return 0;
}

NvReadItemRequestData *NvReadItemRequestData::Clone() const
{
    NvReadItemRequestData *p = new NvReadItemRequestData(m_nReq, m_tok, m_reqType);
    if (p != NULL) {
        p->encode((char *)&m_nvItemId, sizeof(m_nvItemId));
    }
    return p;
}

int NvReadItemRequestData::GetNvItemID() const
{
    return m_nvItemId;
}

/**
 * NvWriteItemRequestData
 */
NvWriteItemRequestData::NvWriteItemRequestData(const int nReq, const Token tok, const ReqType type)
    : RequestData(nReq, tok, type)
{
    m_nvItemId = -1;
    memset(m_value, 0, sizeof(m_value));
    param = NULL;
}

NvWriteItemRequestData::~NvWriteItemRequestData()
{
}

int NvWriteItemRequestData::encode(char *data, unsigned int datalen)
{
    if (data == NULL || datalen == 0 || datalen != sizeof(RIL_NV_WriteItem)) {
        return -1;
    }

    RIL_NV_WriteItem *nvwi = (RIL_NV_WriteItem *)data;
    m_nvItemId = (int)nvwi->itemID;
    memset(m_value, 0, sizeof(m_value));
    if (nvwi->value != NULL) {
        // limit MAX_NV_ITME_SIZE
        strncpy(m_value, nvwi->value, MAX_NV_ITEM_SIZE);
    }

    return 0;
}

NvWriteItemRequestData *NvWriteItemRequestData::Clone() const
{
    NvWriteItemRequestData *p = new NvWriteItemRequestData(m_nReq, m_tok, m_reqType);
    if (p != NULL) {
        char tmp[MAX_NV_ITEM_SIZE * 2] = {0, };
        memcpy(tmp, m_value, sizeof(m_value));
        RIL_NV_WriteItem nvwi = { (RIL_NV_Item)m_nvItemId, tmp };
        p->encode((char *)&nvwi, sizeof(RIL_NV_WriteItem));
    }
    return p;
}

int NvWriteItemRequestData::GetNvItemID() const
{
    return m_nvItemId;
}

const char *NvWriteItemRequestData::GetValue() const
{
    return m_value;
}

int NvWriteItemRequestData::GetValueLength() const
{
    return strlen(m_value);
}
