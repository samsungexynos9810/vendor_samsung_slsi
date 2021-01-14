/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef    __PROTOCOL_ADAPTER_H__
#define    __PROTOCOL_ADAPTER_H__

#include "sitdef.h"
#include "rildef.h"
#include "modemdata.h"

class ProtocolAdapter {
public:
    virtual ~ProtocolAdapter() {}

public:
    virtual UINT GetType() const=0;
    virtual UINT GetId() const=0;
    virtual UINT GetLength() const=0;
    virtual UINT GetToken() const=0;
    virtual const char *GetParameter() const=0;
    virtual UINT GetParameterLength() const=0;
};

class ProtocolBaseAdapter : public ProtocolAdapter {
protected:
    const ModemData *m_pModemData;

public:
    ProtocolBaseAdapter(const ModemData *pModemData) : m_pModemData(pModemData) {}
    virtual ~ProtocolBaseAdapter() {}

public:
    UINT GetType() const;
    UINT GetId() const;
    UINT GetLength() const;
    virtual UINT GetToken() const;
    virtual const char *GetParameter() const;
    virtual UINT GetParameterLength() const;

    bool IsRequest() const;
    bool IsResponse() const;
    bool IsUnsolicitedResponse() const;
    bool IsSolicitedResponse() const;

    int switchRafValueForFW(int raf) const;
};

class ProtocolReqAdapter : public ProtocolBaseAdapter {
public:
    ProtocolReqAdapter(const ModemData *pModemData) : ProtocolBaseAdapter(pModemData) {}
    virtual ~ProtocolReqAdapter() {}
};

class ProtocolRespAdapter : public ProtocolBaseAdapter {
public:
    ProtocolRespAdapter(const ModemData *pModemData) : ProtocolBaseAdapter(pModemData) {
    }
    virtual ~ProtocolRespAdapter() {}
protected:
    virtual void Init() {}

public:
    virtual UINT GetErrorCode() const;
};

class ProtocolIndAdapter : public ProtocolBaseAdapter {
public:
    ProtocolIndAdapter(const ModemData *pModemData) : ProtocolBaseAdapter(pModemData) {
        Init();
    }
    virtual ~ProtocolIndAdapter() {}
protected:
    virtual void Init() {}

public:
    const char *GetParameter() const;
    UINT GetParameterLength() const;
    UINT GetToken() const { return TOKEN_INVALID; }
};

const char * rcmMsgToString(int rcmId);
const char * rcmErrorToString(int rspErr);


#endif // __PROTOCOL_ADAPTER_H__
