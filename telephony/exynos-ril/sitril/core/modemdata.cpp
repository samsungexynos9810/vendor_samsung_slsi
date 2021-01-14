/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "modemdata.h"
#include "protocoladapter.h"
#include "rillog.h"

ModemData::ModemData(const char * pRawData, int len, bool tx) : m_pRawData(NULL), m_nLen(0), m_bTx(tx)
{
    SetData(pRawData, len, tx);
}

ModemData::~ModemData()
{
    Release();
}

void ModemData::SetData(const char *buf, int len, bool tx)
{
    Release();

    if (buf != NULL && len > 0) {
        m_pRawData = new char[len];
        memcpy(m_pRawData, buf, len);
        m_nLen = len;
        m_bTx = tx;
    }
}

void ModemData::Release()
{
    if (m_pRawData != NULL) {
        delete m_pRawData;
        m_pRawData = NULL;
    }
    m_nLen = 0;
    m_bTx = false;
}

UINT ModemData::GetToken() const
{
    ProtocolBaseAdapter adapter(this);
    return adapter.GetToken();
}

UINT ModemData::GetType() const
{
    ProtocolBaseAdapter adapter(this);
    return adapter.GetType();
}

UINT ModemData::GetMessageId() const
{
    ProtocolBaseAdapter adapter(this);
    return adapter.GetId();
}

ModemData *ModemData::Clone() const
{
    return new ModemData(m_pRawData, m_nLen);
}

ModemData &ModemData::operator=(const ModemData &rhs)
{
    this->SetData(rhs.m_pRawData, rhs.m_nLen, rhs.IsTx());
    return *this;
}
bool ModemData::IsRequest() const
{
    ProtocolBaseAdapter adapter(this);
    return adapter.IsRequest();
}
bool ModemData::IsResponse() const
{
    ProtocolBaseAdapter adapter(this);
    return adapter.IsResponse();
}
bool ModemData::IsUnsolicitedResponse() const
{
    ProtocolBaseAdapter adapter(this);
    return adapter.IsUnsolicitedResponse();
}
bool ModemData::IsSolicitedResponse() const
{
    ProtocolBaseAdapter adapter(this);
    return adapter.IsSolicitedResponse();
}

void ModemData::Dump() const
{
    char buf[100];
    int len = 0;
    for(size_t i=0, j=0; i < m_nLen; i+=16) {
        len = 0;
        for(j=0; j<16; j++) {
            if(i+j == m_nLen) break;
            len += snprintf(&buf[len], 100-len, "%02x", m_pRawData[i+j]);
        }
        buf[len]='\0';
        RilLogV("Dump 16bytes(idx=%d~%d): %s", i, i+j, buf);
    }
}
