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
 * vsimdata.cpp
 *
 *  Created on: 2016. 02. 26.
 */

#include "vsimdata.h"
#include "util.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_VSIM, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_VSIM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)

/**
 * Virtual SIM Operation
 */
VsimOperationData::VsimOperationData(const int nReq, const Token tok, const ReqType type) : RequestData(nReq, tok,type)
{
    m_nTid= 0;
    m_nEventId = 0;
    m_nResult = 0;
    m_nDataLength = 0;
    m_pData = NULL;
}

VsimOperationData::~VsimOperationData()
{
    if ( m_pData != NULL )
    {
        delete[] m_pData;
    }
}

INT32 VsimOperationData::encode(char *data, unsigned int length)
{
    if((0 == length) || (NULL == data)) return -1;

    RIL_VsimOperationEvent *pVsimOp = (RIL_VsimOperationEvent *)data;

    m_nTid = pVsimOp->transaction_id;
    m_nEventId = pVsimOp->eventId;
    m_nResult = pVsimOp->result;
    m_nDataLength = pVsimOp->data_length;
    if ( pVsimOp->data!= NULL )
    {
        m_pData = new char[strlen(pVsimOp->data)+1];
        memset(m_pData, 0x00, strlen(pVsimOp->data)+1);
        if ( m_pData != NULL )
        {
            memcpy(m_pData, pVsimOp->data, strlen(pVsimOp->data));
        }
    }

    RilLogV("VsimOperationData::%s() tid(%d), eventid(%d), result(%d), datalen(%d)",
        __FUNCTION__, m_nTid, m_nEventId, m_nResult, m_nDataLength);

    return 0;
}

/**
 * Virtual SIM Operation for OEM path
 */
VsimOperationDataExt::VsimOperationDataExt(const int nReq, const Token tok, const ReqType type) : VsimOperationData(nReq, tok,type)
{
    m_nTid= 0;
    m_nEventId = 0;
    m_nResult = 0;
    m_nDataLength = 0;
    m_pData = NULL;
}

VsimOperationDataExt::~VsimOperationDataExt()
{
}

INT32 VsimOperationDataExt::encode(char *data, unsigned int length)
{
    if((0 == length) || (NULL == data)) return -1;

    m_nTid = ((int *) data)[0];
    m_nEventId = ((int *) data)[1];
    m_nResult = ((int *) data)[2];
    m_nDataLength = ((int *) data)[3];

    int totalLen = length - 16;
    if (m_nDataLength > 0 && m_nDataLength == totalLen)
    {
        m_pData = new char[m_nDataLength+1];
        memset(m_pData, 0x00, m_nDataLength+1);
        if (m_pData != NULL)
        {
            memcpy(m_pData, data+16, m_nDataLength);
        }
    }

    RilLogV("VsimOperationDataExt::%s() tid(%d), eventid(%d), result(%d), datalen(%d)",
        __FUNCTION__, m_nTid, m_nEventId, m_nResult, m_nDataLength);

    return 0;
}
