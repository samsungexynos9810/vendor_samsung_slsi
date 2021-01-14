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
 * protocolvsimadapter.cpp
 *
 *  Created on: 2016. 02. 26.
 */

#include "protocolvsimadapter.h"
#include "util.h"
#include "rillog.h"

/**
 * ProtocolVsimOperationAdapter
 */
static int ConvertVsimOpSitEventIdToRilEventId(int eventid)
{
    switch(eventid)
    {
        case SIT_VSIM_OPERATION_ATR:
            return REQUEST_TYPE_ATR_EVENT;
        case SIT_VSIM_OPERATION_APDU:
            return REQUEST_TYPE_APDU_EVENT;
        case SIT_VSIM_OPERATION_POWERDOWN:
            return REQUEST_TYPE_CARD_POWER_DOWN;
        default:
            RilLogE("%s: invalid eventId:%d", __FUNCTION__, eventid);
            return -1;
    }
}

ProtocolVsimOperationAdapter::ProtocolVsimOperationAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData), mOperationData(NULL), mOperationDataLength(0)
{
    if (m_pModemData != NULL) {
        sit_vsim_opertaion_ind *data = (sit_vsim_opertaion_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_VSIM_OPERATION) {
            int dataLen = data->datalength;
            if (dataLen > 0) {
                mOperationDataLength = dataLen * 2 + 1;
                mOperationData = new char[mOperationDataLength];
                if (mOperationData != NULL) {
                    memset(mOperationData, 0, mOperationDataLength);
                    int ret = Value2HexString(mOperationData, (const BYTE*)data->data, dataLen);
                    if (ret < 0) {
                        delete[] mOperationData;
                        mOperationData = NULL;
                        mOperationDataLength = 0;
                    }
                }
            }
        }
    }
}

ProtocolVsimOperationAdapter::~ProtocolVsimOperationAdapter()
{
    if (mOperationData != NULL) {
        delete[] mOperationData;
    }
}

int ProtocolVsimOperationAdapter::GetTransactionId() const
{
    if (m_pModemData != NULL) {
        sit_vsim_opertaion_ind *data = (sit_vsim_opertaion_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_VSIM_OPERATION) {
            return data->tid;
        }
    }
    return -1;
}

int ProtocolVsimOperationAdapter::GetEventId() const
{
    if (m_pModemData != NULL) {
        sit_vsim_opertaion_ind *data = (sit_vsim_opertaion_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_VSIM_OPERATION) {
            int eventid = ConvertVsimOpSitEventIdToRilEventId(data->event_id);
            return eventid;
        }
    }
    return -1;
}

int ProtocolVsimOperationAdapter::GetResult() const
{
    if (m_pModemData != NULL) {
        sit_vsim_opertaion_ind *data = (sit_vsim_opertaion_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_VSIM_OPERATION) {
            return data->result;
        }
    }
    return -1;
}
