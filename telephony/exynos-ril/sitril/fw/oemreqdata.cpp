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
 * oemreqdata.cpp
 *
 *  Created on: 2014. 8. 20.
 *      Author: sungwoo48.choi
 */
#include "oemreqdata.h"
#include "rillog.h"

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_OEM, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_OEM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_OEM, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_OEM, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

OemRequestData::OemRequestData(const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq, tok, type), m_nOemRequestId(0), m_nPayloadSize(0)
{
    m_ptrPayload = NULL;
}

OemRequestData::~OemRequestData()
{
    if(m_ptrPayload)
        delete[] m_ptrPayload;
    m_ptrPayload = NULL;
}

INT32 OemRequestData::encode(char *data, unsigned int datalen)
{
    if(NULL == data || 0 == datalen)
        return -1;
    // Header
    /* Note: oem data is in big endian _ coming from java */
    m_nOemRequestId = ((data[0] << 8) | data[1]);

    /* Note: oem data hdr.len is in big endian _ coming from java */
    m_nPayloadSize = ((data[2] << 8) | data[3]);

    if (m_nPayloadSize > 0) {
        m_ptrPayload = new BYTE[m_nPayloadSize];
        memcpy(m_ptrPayload, (data + HEADER_SIZE), m_nPayloadSize);
    } else {
        m_ptrPayload = NULL;
    }
    return 0;
}

/**
 * OemHookRawRequestData
 */
OemHookRawRequestData::OemHookRawRequestData(const int nReq, const Token tok, const ReqType type)
    : RawRequestData(nReq, tok, type), mOemRawData(NULL), mOemRawDataLen(0)
{
}

OemHookRawRequestData::~OemHookRawRequestData()
{
    if (mOemRawData != NULL) {
        delete[] (char *)mOemRawData;
    }
}

INT32 OemHookRawRequestData::encode(char *data, unsigned int datalen)
{
    // A payload OEM_HOOK_RAW must be more than 4 bytes.
    if (data == NULL || datalen < sizeof(int)) {
        return -1;
    }

    mOemRawData = new char[datalen];
    if (mOemRawData == NULL) {
        return -1;
    }
    memcpy(mOemRawData, data, datalen);
    mOemRawDataLen = datalen;

    int *p = (int *)mOemRawData;
    // use the first 4byte(int) as OEM request ID
    // e.g., 10133(0x00002795) is RIL_REQUEST_OEM_CANCEL_AVAILABLE_NETWORKS.
    m_nReq = *p++;

    // if datalen is more than 8 bytes, the next 4 bytes is message length.
    if (datalen >= sizeof(int) * 2) {
        unsigned int payloadLen = *p++;
        // invalid value of a parameter length.
        if (payloadLen > datalen - (sizeof(int)*2)) {
            RilLogW("[OemHookRawRequestData::encode] invalid OEM_HOOK payload length");
            return -1;
        }
        return RawRequestData::encode((char *)p, payloadLen);
    }
    return 0;
}
