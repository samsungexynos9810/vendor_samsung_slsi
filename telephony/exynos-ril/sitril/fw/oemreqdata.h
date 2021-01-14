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
 * oemreqdata.h
 *
 *  Created on: 2014. 8. 20.
 *      Author: sungwoo48.choi
 */

#ifndef __OEM_REQ_DATA_H__
#define __OEM_REQ_DATA_H__

#include "requestdata.h"

class OemRequestData : public RequestData
{
public:
    UINT8 GetOemRequestId() {return m_nOemRequestId;}
    INT32 GetPayloadSize(){return m_nPayloadSize;}
    BYTE * GetPayload(){return m_ptrPayload;}

    OemRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~OemRequestData();
    INT32 encode(char *data, unsigned int datalen);

private:
    // Header
    static const int HEADER_SIZE = 4;
    UINT16 m_nOemRequestId;
    INT16 m_nPayloadSize;
    BYTE *m_ptrPayload;
};

/**
 * OemHookRawRequestData
 */
class OemHookRawRequestData : public RawRequestData
{
protected:
    void *mOemRawData;
    unsigned int mOemRawDataLen;
public:
    OemHookRawRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~OemHookRawRequestData();
    virtual INT32 encode(char *data, unsigned int datalen);
};

#endif /* __OEM_REQ_DATA_H__ */
