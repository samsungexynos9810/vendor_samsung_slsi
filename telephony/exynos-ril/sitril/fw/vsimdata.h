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
 * vsimdata.h
 *
 *  Created on: 2016. 02. 26.
 */

#ifndef _VSIM_DATA_H_
#define _VSIM_DATA_H_

#include "requestdata.h"

#define MAX_UUID_LENGTH 64

/**
 * Virtual SIM Operation
 */

class VsimOperationData : public RequestData
{
public:
    VsimOperationData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~VsimOperationData();

    int GetTid() { return m_nTid; }
    int GetEventId() { return m_nEventId; }
    int GetResult() { return m_nResult; }
    int GetDataLength() { return m_nDataLength; }
    char *GetData() { return m_pData; }

    virtual INT32 encode(char *data, unsigned int length);

    INT32 m_nTid;
    INT32 m_nEventId;
    INT32 m_nResult;
    INT32 m_nDataLength;
    char* m_pData;
};

/**
 * Virtual SIM Operation for OEM path
 */
class VsimOperationDataExt : public VsimOperationData
{
public:
    VsimOperationDataExt(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~VsimOperationDataExt();

    virtual INT32 encode(char *data, unsigned int length);
};

#endif /*_VSIM_DATA_H_*/
