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
 * protocolimsadapter.h
 *
 *  Created on: 2014. 11. 19.
 *      Author: MOX
 */

#ifndef __PROTOCOL_IMS_ADAPTER_H__
#define __PROTOCOL_IMS_ADAPTER_H__

#include "protocoladapter.h"

class ProtocolImsReasonRespAdapter : public ProtocolRespAdapter {
public:
    ProtocolImsReasonRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }

    BYTE GetResult();
    BYTE GetFailReason();
};

class ProtocolImsRespAdapter : public ProtocolRespAdapter {
public:
    ProtocolImsRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) { }

    BYTE GetResult();
};

class ProtocolImsGetConfRespAdapter : public ProtocolRespAdapter {
public:
    ProtocolImsGetConfRespAdapter(const ModemData *pModemData) : ProtocolRespAdapter(pModemData) {}

private:

public:
};

class ProtocolAimsIndAdapter : public ProtocolIndAdapter {
public:
    ProtocolAimsIndAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) {}

public:
    int GetResultId() const;
};

class ProtocolImsRegIndAdapter : public ProtocolIndAdapter {
public:
    ProtocolImsRegIndAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData) { }

    INT32 GetRegState();
};

class ProtocolAimsReqAdapter {
protected:
    const char *m_pRawData;
    int m_nSize;
public:
    ProtocolAimsReqAdapter(const char *pRawData, int size) { m_pRawData = pRawData; m_nSize = size; }
};

class ProtocolImsAimCallWaitingAdapter : public ProtocolAimsReqAdapter {
public:
    ProtocolImsAimCallWaitingAdapter(const char *pAimsData, int size) : ProtocolAimsReqAdapter(pAimsData, size) {}

    BOOL IsEnable();
    INT32 GetServiceClass();
};
#endif /* __PROTOCOL_IMS_ADAPTER_H__ */
