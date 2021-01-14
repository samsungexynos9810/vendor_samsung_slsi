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
 * protocolradioconfigadapter.cpp
 *
 *  Created on: 2019. 8. 13.
 */
#include "protocolradioconfigadapter.h"

ProtocolPhoneCapabilityAdapter::ProtocolPhoneCapabilityAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
    // logicalModemList not supported by modem
    // generate logical modemId using index
    int size = sizeof(mLogicalModemList)/sizeof(mLogicalModemList[0]);
    for (int i = 0; i < size; i++) {
        mLogicalModemList[i] = i;
    }
}

int ProtocolPhoneCapabilityAdapter::GetMaxActiveData() const
{
    if (m_pModemData != NULL) {
        sit_pdp_get_phone_capability_rsp *data = (sit_pdp_get_phone_capability_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PHONE_CAPABILITY) {
            return data->max_simultaneous_data_stack & 0xFF;
        }
    }
    return 1;
}

int ProtocolPhoneCapabilityAdapter::GetMaxActiveInternetData() const
{
    if (m_pModemData != NULL) {
        sit_pdp_get_phone_capability_rsp *data = (sit_pdp_get_phone_capability_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PHONE_CAPABILITY) {
            return data->max_simultaneous_internet_pdn & 0xFF;
        }
    }
    return 1;
}

bool ProtocolPhoneCapabilityAdapter::IsInternetLingeringSupported() const
{
    if (m_pModemData != NULL) {
        sit_pdp_get_phone_capability_rsp *data = (sit_pdp_get_phone_capability_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PHONE_CAPABILITY) {
            return data->internet_lingering_support & 0xFF;
        }
    }
    return false;
}

int ProtocolPhoneCapabilityAdapter::GetLogicalModemListSize() const
{
    if (m_pModemData != NULL) {
        sit_pdp_get_phone_capability_rsp *data = (sit_pdp_get_phone_capability_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PHONE_CAPABILITY) {
            return data->max_supported_stack & 0xFF;
        }
    }
    return 1;
}

int *ProtocolPhoneCapabilityAdapter::GetLogicalModemList()
{
    return mLogicalModemList;
}
