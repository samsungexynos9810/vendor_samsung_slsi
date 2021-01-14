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
 * vsimbuilder.cpp
 *
 *  Created on: 2016. 02. 26.
 */
#include "vsimdatabuilder.h"
#include "rillog.h"
#include "util.h"

class VsimOperationData : public RilData
{
private:
    int m_ConvLen;
    RIL_VsimOperationEvent* m_OperationData;
public:
    VsimOperationData(int tid, int eventid, int result) : RilData() {
        m_OperationData = new RIL_VsimOperationEvent;
        if ( m_OperationData != NULL )
        {
            m_OperationData->transaction_id = tid;
            m_OperationData->eventId = eventid;
            m_OperationData->result = result;
            m_OperationData->data_length = 0;
            m_OperationData->data = NULL;
            m_ConvLen = 0;
        }
    }
    ~VsimOperationData() {
        if (m_OperationData != NULL) {
            if ( m_OperationData->data != NULL )
            {
                delete[] m_OperationData->data;
            }
            delete m_OperationData;
        }
    }
public:
    void SetOperationData(int datalen, const char* pData) {
        if ( datalen > 0 && pData != NULL && m_OperationData != NULL )
        {
            if ( datalen > MAX_VSIM_DATA_LEN )
            {
                RilLogE("VsimOperationData::%s() invalid data_len(%d), set max data_len(%d)", __FUNCTION__, datalen,MAX_VSIM_DATA_LEN);
                datalen = MAX_VSIM_DATA_LEN;
            }
            m_OperationData->data_length= datalen;
            m_OperationData->data = new char[datalen];
            if ( m_OperationData->data == NULL )
            {
                RilLogE("VsimOperationData::%s() memory alloc fail", __FUNCTION__);
                m_OperationData->data_length = 0;
                return;
            }
            memset(m_OperationData->data, 0x00, datalen);
            memcpy(m_OperationData->data, pData, datalen);
        }
    }

    void *GetData() const {
        return m_OperationData;
    }

    unsigned int GetDataLength() const { return sizeof(RIL_VsimOperationEvent); }
};

const RilData *VsimDataBuilder::BuildVsimOperation(int tid, int eventid, int result, int datalen, const char* data)
{
    VsimOperationData *rildata = new VsimOperationData(tid, eventid, result);
    if ( rildata != NULL )
    {
        if(data!=NULL && datalen>0)
        {
            rildata->SetOperationData(datalen, data);
        }
    }

    return rildata;
}

const RilData *VsimDataBuilder::BuildVsimOperationExt(int tid, int eventid, int result, int datalen, const char* data)
{
    if (datalen < 0) {
        datalen = 0;
    }

    int totalLen = 4 * 4 + datalen;
    char *buf = new char[totalLen];
    if (buf == NULL) {
        return NULL;
    }

    ((int *)buf)[0] = tid;
    ((int *)buf)[1] = eventid;
    ((int *)buf)[2] = result;
    ((int *)buf)[3] = datalen;

    if (datalen > 0 && data != NULL) {
        memcpy(&((int *)buf)[4], data, datalen);
    }

    RilDataRaw *rildata = new RilDataRaw(buf, totalLen);
    delete[] buf;

    return rildata;
}
