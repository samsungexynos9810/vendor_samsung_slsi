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
 * protocolsimadapter.cpp
 *
 *  Created on: 2014. 6. 28.
 *      Author: mox
 */
#include "protocolsimadapter.h"
#include "rillog.h"
#include "util.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SIM, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/**
 * ProtocolSimStatusAdapter
 */

void ProtocolSimStatusAdapter::Init()
{
    memset(&m_tSimStatusRsp, 0, sizeof(m_tSimStatusRsp));
    memset(&m_tSimStatusRspExt, 0, sizeof(m_tSimStatusRspExt));

    if (m_pModemData != NULL) {
        sit_sim_get_sim_status_rsp *pData = (sit_sim_get_sim_status_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_GET_SIM_STATUS) {
            UINT nSimStatusLength = sizeof(sit_sim_get_sim_status_rsp) - (sizeof(sit_sim_apps_status_info)*MAX_SIM_APPS_INFO_COUNT);
            nSimStatusLength += (sizeof(sit_sim_apps_status_info)*pData->application_num);
            memcpy(&m_tSimStatusRsp, pData , nSimStatusLength);

            // for Radio 1.2
            if(GetParameterLength() > (nSimStatusLength-sizeof(RCM_HEADER)))
            {
                sit_sim_get_sim_status_rsp_ext *pData_ext = (sit_sim_get_sim_status_rsp_ext *) (m_pModemData->GetRawData() + nSimStatusLength);
                int sizefor_1_2 = sizeof(sit_sim_get_sim_status_rsp_ext) - (MAX_EID_LEN + 1);
                memcpy(&m_tSimStatusRspExt, pData_ext, sizefor_1_2);
                nSimStatusLength += sizefor_1_2;
                // for Radio 1.4 (eID)
                int sizefor_1_4 = MAX_EID_LEN + 1;
                if (GetParameterLength() > (nSimStatusLength-sizeof(RCM_HEADER))) {
                    m_tSimStatusRspExt.eid_length = pData_ext->eid_length;
                    memcpy(&m_tSimStatusRspExt.eid, pData_ext->eid, pData_ext->eid_length);
                    nSimStatusLength += sizefor_1_4;
                }
            }
        }
        else RilLogE("%s GetRawData() is NULL", __FUNCTION__);
    }
    else RilLogE("%s m_pModemData is NULL", __FUNCTION__);
}


char *ProtocolSimStatusAdapter::GetAID(int nIndex) const
{
    char *pAID = NULL;
    if(nIndex>=0 && nIndex<m_tSimStatusRsp.application_num)
    {
        int size = m_tSimStatusRsp.apps_status_info[nIndex].aid_len * 2 + 1;
        pAID = new char[size];
        int ret = Value2HexString(pAID, m_tSimStatusRsp.apps_status_info[nIndex].AID, m_tSimStatusRsp.apps_status_info[nIndex].aid_len);
        if (ret == -1) {
            delete [] pAID;
            return NULL;
        }
    }

    return pAID;
}

int ProtocolSimStatusAdapter::GetApplicationLabel(int nIndex, BYTE *pAppLabel) const
{
    int nAppLabelLen = 0;
    if((nIndex>=0 && nIndex<m_tSimStatusRsp.application_num)
            && m_tSimStatusRsp.apps_status_info[nIndex].app_label_len<=MAX_SIM_APP_LABEL_LEN && pAppLabel!=NULL) {
        nAppLabelLen = (int) m_tSimStatusRsp.apps_status_info[nIndex].app_label_len;
        memcpy(pAppLabel, m_tSimStatusRsp.apps_status_info[nIndex].app_label, nAppLabelLen);
    }

    return nAppLabelLen;
}

int ProtocolSimStatusAdapter::GetPinState(int nIndex, int nPinIndex) const
{
    int nPinState = 0;
    if(nIndex>=0 && nIndex<m_tSimStatusRsp.application_num)
    {
        switch(nPinIndex)
        {
        case 1: nPinState = (int) m_tSimStatusRsp.apps_status_info[nIndex].pin1_state; break;
        case 2: nPinState = (int) m_tSimStatusRsp.apps_status_info[nIndex].pin2_state; break;
        }
    }

    return nPinState;
}

int ProtocolSimStatusAdapter::GetPinRemainCount(int nIndex, int nPinIndex) const
{
    int nPinRemainCount = -1;
    if(nIndex>=0 && nIndex<m_tSimStatusRsp.application_num)
    {
        switch(nPinIndex)
        {
        case 1: nPinRemainCount = (int) m_tSimStatusRsp.apps_status_info[nIndex].pin1_remain_count; break;
        case 2: nPinRemainCount = (int) m_tSimStatusRsp.apps_status_info[nIndex].pin2_remain_count; break;
        }
    }

    return nPinRemainCount;
}

int ProtocolSimStatusAdapter::GetPukRemainCount(int nIndex, int nPukIndex) const
{
    int nPukRemainCount = -1;
    if(nIndex>=0 && nIndex<m_tSimStatusRsp.application_num)
    {
        switch(nPukIndex)
        {
        case 1: nPukRemainCount = (int) m_tSimStatusRsp.apps_status_info[nIndex].puk1_remain_count; break;
        case 2: nPukRemainCount = (int) m_tSimStatusRsp.apps_status_info[nIndex].puk2_remain_count; break;
        }
    }

    return nPukRemainCount;
}

/**
 * ProtocolSimVerifyPinAdapter
 */
void ProtocolSimVerifyPinAdapter::Init()
{
    m_nPinIndex = 0;
    m_nRemainCount = -1;

    if (m_pModemData != NULL) {
        sit_sim_verify_sim_pin_rsp *pData = (sit_sim_verify_sim_pin_rsp *) m_pModemData->GetRawData();
        if (pData != NULL) {
            switch(pData->hdr.id)
            {
            case SIT_VERIFY_SIM_PIN:
            case SIT_CHG_SIM_PIN:
                m_nPinIndex = 1;
                m_nRemainCount = pData->remain_count;
                break;

            case SIT_VERIFY_SIM_PIN2:
            case SIT_CHG_SIM_PIN2:
                m_nPinIndex = 2;
                m_nRemainCount = pData->remain_count;
                break;
            }
        }
    }
}

/**
 * ProtocolSimVerifyPukAdapter
 */
void ProtocolSimVerifyPukAdapter::Init()
{
    m_nPukIndex = 0;
    m_nRemainCount = -1;

    if (m_pModemData != NULL) {
        sit_sim_verify_sim_puk_rsp *pData = (sit_sim_verify_sim_puk_rsp *) m_pModemData->GetRawData();
        if (pData != NULL) {
            switch(pData->hdr.id)
            {
            case SIT_VERIFY_SIM_PUK:
                m_nPukIndex = 1;
                m_nRemainCount = pData->remain_count;
                break;
            case SIT_VERIFY_SIM_PUK2:
                m_nPukIndex = 2;
                m_nRemainCount = pData->remain_count;
                break;
            }
        }
    }
}

/**
 * ProtocolSimChangePinAdapter
 */
void ProtocolSimChangePinAdapter::Init()
{
    m_nPinIndex = 0;
    m_nRemainCount = -1;

    if (m_pModemData != NULL) {
        sit_sim_verify_sim_puk_rsp *pData = (sit_sim_verify_sim_puk_rsp *) m_pModemData->GetRawData();
        if (pData != NULL) {
            switch(pData->hdr.id)
            {
            case SIT_CHG_SIM_PIN:
                m_nPinIndex = 1;
                m_nRemainCount = pData->remain_count;
                break;
            case SIT_CHG_SIM_PIN2:
                m_nPinIndex = 2;
                m_nRemainCount = pData->remain_count;
                break;
            }
        }
    }
}

/**
 * ProtocolSimVerifyNetLockAdapter
 */
int ProtocolSimVerifyNetLockAdapter::GetRemainCount() const
{
    int nRemainCount = -1;
    if (m_pModemData != NULL) {
        sit_sim_verify_network_lock_rsp *pData = (sit_sim_verify_network_lock_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_VERIFY_NETWORK_LOCK) {
            nRemainCount = pData->remain_count;
        }
    }

    return nRemainCount;
}

/**
 * ProtocolSimIOAdapter
 */
BYTE ProtocolSimIOAdapter::GetSw1() const
{
    BYTE sw1 = 0;
    if (m_pModemData != NULL) {
        sit_sim_sim_io_rsp *pData = (sit_sim_sim_io_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && (pData->hdr.id == SIT_SIM_IO
                    || pData->hdr.id == SIT_TRANSMIT_SIM_APDU_CHANNEL
                    || pData->hdr.id == SIT_TRANSMIT_SIM_APDU_BASIC)) {
            sw1 = pData->sw1;
        }
    }

    return sw1;
}

BYTE ProtocolSimIOAdapter::GetSw2() const
{
    BYTE sw2 = 0;
    if (m_pModemData != NULL) {
        sit_sim_sim_io_rsp *pData = (sit_sim_sim_io_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && (pData->hdr.id == SIT_SIM_IO
                    || pData->hdr.id == SIT_TRANSMIT_SIM_APDU_CHANNEL
                    || pData->hdr.id == SIT_TRANSMIT_SIM_APDU_BASIC)) {
            sw2 = pData->sw2;
        }
    }

    return sw2;
}

BYTE *ProtocolSimIOAdapter::GetResponse() const
{
    BYTE *pResponse = NULL;
    if (m_pModemData != NULL) {
        sit_sim_sim_io_rsp *pData = (sit_sim_sim_io_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && (pData->hdr.id == SIT_SIM_IO
                    || pData->hdr.id == SIT_TRANSMIT_SIM_APDU_CHANNEL
                    || pData->hdr.id == SIT_TRANSMIT_SIM_APDU_BASIC)) {
            pResponse = pData->response;
        }
    }

    return pResponse;
}

int ProtocolSimIOAdapter::GetResponseLength() const
{
    int nResponseLength = 0;
    if (m_pModemData != NULL) {
        sit_sim_sim_io_rsp *pData = (sit_sim_sim_io_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && (pData->hdr.id == SIT_SIM_IO
                    || pData->hdr.id == SIT_TRANSMIT_SIM_APDU_CHANNEL
                    || pData->hdr.id == SIT_TRANSMIT_SIM_APDU_BASIC)) {
            nResponseLength =  pData->response_len;
        }
    }

    return (nResponseLength>MAX_SIM_IO_DATA_LEN)? MAX_SIM_IO_DATA_LEN : nResponseLength;
}

/**
 * ProtocolSimGetFacilityLockAdapter
 */
int ProtocolSimGetFacilityLockAdapter::GetLockMode() const
{
    int nLockMode = -1;
    if (m_pModemData != NULL) {
        sit_sim_get_facility_lock_rsp *pData = (sit_sim_get_facility_lock_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_GET_FACILITY_LOCK) {
            nLockMode = (int) pData->lock_mode;
        }
    }

    return nLockMode;
}

int ProtocolSimGetFacilityLockAdapter::GetServiceClass() const
{
    int nSvcClass = 0;
    if (m_pModemData != NULL) {
        sit_sim_get_facility_lock_rsp *pData = (sit_sim_get_facility_lock_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_GET_FACILITY_LOCK) {
            nSvcClass = (int) pData->service_class;
        }
    }

    return nSvcClass;
}

/**
 * ProtocolSimSetFacilityLockAdapter
 */
int ProtocolSimSetFacilityLockAdapter::GetRemainCount() const
{
    int nRemainCount = -1;
    if (m_pModemData != NULL) {
        sit_sim_set_facility_lock_rsp *pData = (sit_sim_set_facility_lock_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_SET_FACILITY_LOCK) {
            switch(pData->hdr.ext.rsp.error)
            {
            case RCM_E_SUCCESS:
            case RCM_E_GENERIC_FAILURE:
            case RCM_E_PASSWORD_INCORRECT:
                nRemainCount = pData->remain_count;
                break;

            //case RCM_E_SIM_PUK:
            //    nRemainCount = 0;
            //    break;
            case RCM_E_SIM_PUK2:
                nRemainCount = 0;
                break;
            //case RCM_E_SIM_PIN:
            case RCM_E_SIM_PIN2:
            //case RCM_E_PERM_BLOCKED:
            //case RCM_E_PERM_BLOCKED2:
                nRemainCount = -1;    // Unknown
                break;
            }
        }
    }

    return nRemainCount;
}

/**
 * ProtocolSimGetSimAuthAdapter
 */
int ProtocolSimGetSimAuthAdapter::GetPayloadLength() const
{
    if (m_pModemData != NULL) {
        sit_sim_get_sim_auth_rsp *pData = (sit_sim_get_sim_auth_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id==SIT_GET_SIM_AUTH) {
            return (int) (pData->hdr.length - sizeof(RCM_HEADER));
        }
    }

    return 0;
}

int ProtocolSimGetSimAuthAdapter::GetAuthType() const
{
    if (m_pModemData != NULL) {
        sit_sim_get_sim_auth_rsp *pData = (sit_sim_get_sim_auth_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id==SIT_GET_SIM_AUTH) {
            return (int) pData->auth_type;
        }
    }

    return 0;
}

int ProtocolSimGetSimAuthAdapter::GetAuthLength() const
{
    if (m_pModemData != NULL) {
        sit_sim_get_sim_auth_rsp *pData = (sit_sim_get_sim_auth_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id==SIT_GET_SIM_AUTH) {
            return (int) pData->auth_len;
        }
    }

    return 0;
}

BYTE *ProtocolSimGetSimAuthAdapter::GetAuth() const
{
    if (m_pModemData != NULL) {
        sit_sim_get_sim_auth_rsp *pData = (sit_sim_get_sim_auth_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id==SIT_GET_SIM_AUTH) {
            return pData->auth;
        }
    }

    return NULL;
}

/**
 * ProtocolSimTransmitApduBasicAdapter
 */
BYTE ProtocolSimTransmitApduBasicAdapter::GetSw1() const
{
    BYTE sw1 = 0;
    if (m_pModemData != NULL) {
        sit_sim_transmit_sim_apdu_basic_rsp *pData = (sit_sim_transmit_sim_apdu_basic_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_TRANSMIT_SIM_APDU_BASIC) {
            sw1 = (GetApduLength()>=2)? pData->apdu[GetApduLength()-2] : 0;
        }
    }

    return sw1;
}

BYTE ProtocolSimTransmitApduBasicAdapter::GetSw2() const
{
    BYTE sw2 = 0;
    if (m_pModemData != NULL) {
        sit_sim_transmit_sim_apdu_basic_rsp *pData = (sit_sim_transmit_sim_apdu_basic_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_TRANSMIT_SIM_APDU_BASIC) {
            sw2 = (GetApduLength()>=2)? pData->apdu[GetApduLength()-1] : 0;
        }
    }

    return sw2;
}

int ProtocolSimTransmitApduBasicAdapter::GetApduLength() const
{
    int nLength = 0;
    if (m_pModemData != NULL) {
        sit_sim_transmit_sim_apdu_basic_rsp *pData = (sit_sim_transmit_sim_apdu_basic_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_TRANSMIT_SIM_APDU_BASIC) {
            nLength =  pData->apdu_len;
            if(nLength>MAX_APDU_LEN) nLength = MAX_APDU_LEN;
            if(nLength>=(int)(GetParameterLength()-2)) nLength = GetParameterLength() - 2;        // 2 is for APDU length(WORD)
        }
    }

    return nLength;
}

BYTE *ProtocolSimTransmitApduBasicAdapter::GetApdu() const
{
    BYTE *pApdu = NULL;
    if (m_pModemData != NULL) {
        sit_sim_transmit_sim_apdu_basic_rsp *pData = (sit_sim_transmit_sim_apdu_basic_rsp*) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_TRANSMIT_SIM_APDU_BASIC) {
            pApdu = pData->apdu;
        }
    }

    return pApdu;
}

/**
 * ProtocolSimOpenChannelAdapter
 */
int ProtocolSimOpenChannelAdapter::GetSessionID() const
{
    int nSessionID = 0;
    if (m_pModemData != NULL) {
        sit_sim_open_channel_rsp *pData = (sit_sim_open_channel_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && (pData->hdr.id == SIT_OPEN_SIM_CHANNEL || pData->hdr.id == SIT_OPEN_SIM_CHANNEL_WITH_P2)) {
            nSessionID = (int) pData->session_id;
        }
    }

    return nSessionID;
}

BYTE ProtocolSimOpenChannelAdapter::GetSw1() const
{
    BYTE sw1 = 0;
    if (m_pModemData != NULL) {
        sit_sim_open_channel_rsp *pData = (sit_sim_open_channel_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && (pData->hdr.id == SIT_OPEN_SIM_CHANNEL || pData->hdr.id == SIT_OPEN_SIM_CHANNEL_WITH_P2)) {
            sw1 = pData->sw1;
        }
    }

    return sw1;
}


BYTE ProtocolSimOpenChannelAdapter::GetSw2() const
{
    BYTE sw2 = 0;
    if (m_pModemData != NULL) {
        sit_sim_open_channel_rsp *pData = (sit_sim_open_channel_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && (pData->hdr.id == SIT_OPEN_SIM_CHANNEL || pData->hdr.id == SIT_OPEN_SIM_CHANNEL_WITH_P2)) {
            sw2 = pData->sw2;
        }
    }

    return sw2;
}

BYTE *ProtocolSimOpenChannelAdapter::GetResponse() const
{
    BYTE *pResponse = NULL;
    if (m_pModemData != NULL) {
        sit_sim_open_channel_rsp *pData = (sit_sim_open_channel_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && (pData->hdr.id == SIT_OPEN_SIM_CHANNEL || pData->hdr.id == SIT_OPEN_SIM_CHANNEL_WITH_P2)) {
            pResponse = pData->response;
        }
    }

    return pResponse;
}

int ProtocolSimOpenChannelAdapter::GetResponseLength() const
{
    int nResponseLength = 0;
    if (m_pModemData != NULL) {
        sit_sim_open_channel_rsp *pData = (sit_sim_open_channel_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && (pData->hdr.id == SIT_OPEN_SIM_CHANNEL || pData->hdr.id == SIT_OPEN_SIM_CHANNEL_WITH_P2)) {
            nResponseLength =  pData->response_len;
            if(nResponseLength>MAX_OPEN_CHANNEL_RSP_LEN) nResponseLength = MAX_OPEN_CHANNEL_RSP_LEN;
            if(nResponseLength>(int)(GetParameterLength()-2)) nResponseLength = GetParameterLength() - 2;        // 2 is for APDU length(WORD)
        }
    }

    return nResponseLength;
}

/**
 * ProtocolSimTransmitApduChannelAdapter
 */
BYTE ProtocolSimTransmitApduChannelAdapter::GetSw1() const
{
    BYTE sw1 = 0;
    if (m_pModemData != NULL) {
        sit_sim_transmit_sim_apdu_channel_rsp *pData = (sit_sim_transmit_sim_apdu_channel_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_TRANSMIT_SIM_APDU_CHANNEL) {
            sw1 = pData->sw1;
        }
    }

    return sw1;
}

BYTE ProtocolSimTransmitApduChannelAdapter::GetSw2() const
{
    BYTE sw2 = 0;
    if (m_pModemData != NULL) {
        sit_sim_transmit_sim_apdu_channel_rsp *pData = (sit_sim_transmit_sim_apdu_channel_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_TRANSMIT_SIM_APDU_CHANNEL) {
            sw2 = pData->sw2;
        }
    }

    return sw2;
}

int ProtocolSimTransmitApduChannelAdapter::GetApduLength() const
{
    int nLength = 0;
    if (m_pModemData != NULL) {
        sit_sim_transmit_sim_apdu_channel_rsp *pData = (sit_sim_transmit_sim_apdu_channel_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_TRANSMIT_SIM_APDU_CHANNEL) {
            nLength = pData->response_len;
            if(nLength>MAX_APDU_LEN) nLength = MAX_APDU_LEN;
            if(nLength>(int)(GetParameterLength()-2)) nLength = GetParameterLength() - 2;        // 2 is for APDU length(WORD)
        }
    }

    return nLength;
}

BYTE *ProtocolSimTransmitApduChannelAdapter::GetApdu() const
{
    BYTE *pApdu = NULL;
    if (m_pModemData != NULL) {
        sit_sim_transmit_sim_apdu_channel_rsp *pData = (sit_sim_transmit_sim_apdu_channel_rsp*) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id == SIT_TRANSMIT_SIM_APDU_CHANNEL) {
            pApdu = pData->response;
        }
    }

    return pApdu;
}


void ProtocolSimImsiAdapter::Init()
{
    memset(m_imsi, 0, sizeof(m_imsi));
    if (m_pModemData != NULL) {
        sit_id_get_imsi_rp *data = (sit_id_get_imsi_rp *)m_pModemData->GetRawData();
        if (GetErrorCode() == RIL_E_SUCCESS && data != NULL && data->hdr.id == SIT_GET_IMSI) {
            memcpy(m_imsi, data->imsi, data->imsi_len);
            //m_imsi[data->imsi_len] = 0;
            m_imsi[data->imsi_len] = '\0';
        }
    }
}

const char *ProtocolSimImsiAdapter::GetImsi() const
{
    if (strlen(m_imsi) > 0)
        return m_imsi;
    return NULL;
}

int ProtocolSimGetGbaAuthAdapter::GetGbaAuthLength() const
{
    if (m_pModemData != NULL) {
        sit_sim_get_sim_gba_auth_rsp *pData = (sit_sim_get_sim_gba_auth_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id==SIT_GET_GBA_CONTEXT) {
            return sizeof(pData->auth);
        }
    }
    return 0;
}

BYTE *ProtocolSimGetGbaAuthAdapter::GetGbaAuth() const
{
    if (m_pModemData != NULL) {
        sit_sim_get_sim_gba_auth_rsp *pData = (sit_sim_get_sim_gba_auth_rsp *) m_pModemData->GetRawData();
        if (pData != NULL && pData->hdr.id==SIT_GET_GBA_CONTEXT) {
            return pData->auth;
        }
    }
    return 0;
}

void ProtocolSimATRAdapter::Init()
{
    memset(m_atr, 0, sizeof(m_atr));
    if (m_pModemData != NULL) {
        sit_id_get_atr_rsp *data = (sit_id_get_atr_rsp *)m_pModemData->GetRawData();
        if (GetErrorCode() == RIL_E_SUCCESS && data != NULL && data->hdr.id == SIT_GET_ATR) {
            m_result = data->result;
            m_atrlen = data->atr_len;
            memcpy(m_atr, data->atr, data->atr_len);
            m_atr[data->atr_len] = '\0';
        }
    }
}

BYTE ProtocolSimATRAdapter::GetResult() const
{
    return m_result;
}

BYTE ProtocolSimATRAdapter::GetATRLength() const
{
    return m_atrlen;
}

const char *ProtocolSimATRAdapter::GetATR() const
{
    if (strlen(m_atr) > 0)
        return m_atr;
    return NULL;
}

/* ProtocolSimReadPbEntry */
void ProtocolSimReadPbEntry::Init()
{
    if (m_pModemData != NULL && GetErrorCode() == RIL_E_SUCCESS) {
        sit_read_pb_resp *data = (sit_read_pb_resp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_READ_PB_ENTRY) {
            m_pbType = data->pb_type;
            m_index = data->index;
            m_dataLen = data->data_len;
            memcpy(m_entryData, data->entry_data, m_dataLen);
        }
    }
}

/* ProtocolSimUpdatePbEntry */
void ProtocolSimUpdatePbEntry::Init()
{
    if (m_pModemData != NULL && GetErrorCode() == RIL_E_SUCCESS) {
        sit_update_pb_entry_resp *data = (sit_update_pb_entry_resp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_UPDATE_PB_ENTRY) {
            m_mode = data->mode;
            m_pbtype = data->pb_type;
            m_index = data->index;
        }
    }
}

/* ProtocolSimPbStorageInfo */
void ProtocolSimPbStorageInfoAdapter::Init()
{
    if (m_pModemData != NULL && GetErrorCode() == RIL_E_SUCCESS) {
        sit_sim_pb_storage_info_rsp *data = (sit_sim_pb_storage_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PB_STORAGE_INFO) {
            m_pbType = data->pb_type;
            m_totalCount = data->total_count;
            m_usedCount = data->used_count;
        }
    }
}

/* ProtocolSimPbStorageList */
int ProtocolSimPbStorageListAdapter::GetPbList()
{
    if (m_pModemData != NULL && GetErrorCode() == RIL_E_SUCCESS) {
        sit_sim_pb_storage_list_rsp *data = (sit_sim_pb_storage_list_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PB_STORAGE_LIST) {
            RilLogV("PbStorageList %d", data->pb_list);
            return data->pb_list;
        }
    }
    return -1;
}

/* ProtocolSimPbEntryInfoAdapter */
void ProtocolSimPbEntryInfoAdapter::Init()
{
    if (m_pModemData != NULL) {
        sit_sim_pb_entry_info_rsp *data = (sit_sim_pb_entry_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PB_ENTRY_INFO) {
            m_pbType = data->pb_type;
            m_indexMax = data->index_max;
            m_indexMin = data->index_min;
            m_numMax = data->num_max;
            m_textMax = data->text_max;
        }
    }
}

/* ProtocolSim3GPBCapaAdapter */
bool ProtocolSimPbCapaAdapter::GetPbCapa(int *pb, int entryNum)
{
    if (m_pModemData != NULL) {
        sit_sim_pb_capa_rsp *data = (sit_sim_pb_capa_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_3G_PB_CAPA) {
            int j = 0;
            for (int i=0; i < entryNum; i++) {
                pb[j] = data->pb_list[i].pb_type;
                pb[j+1] = data->pb_list[i].index_max;
                pb[j+2] = data->pb_list[i].entry_max;
                pb[j+3] = data->pb_list[i].used_count;
                j += 4;
            }
        }
    }

    return true;
}

int ProtocolSimPbCapaAdapter::GetEntryNum() const
{
    if (m_pModemData != NULL) {
        sit_sim_pb_capa_rsp *data = (sit_sim_pb_capa_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_3G_PB_CAPA)
            return data->entry_num;
    }

    return 0;
}

/* ProtocolSimPbReadyAdapter */
void ProtocolSimPbReadyAdapter::Init()
{
    if (m_pModemData != NULL) {
        sit_sim_pb_ready_ind *data = (sit_sim_pb_ready_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_SIM_PB_READY) {
            m_pbReady = data->pb_ready;
        }
    }
}

/* ProtocolSimIccidInfoAdapter */
void ProtocolSimIccidInfoAdapter::Init()
{
    m_nIccidLen = -1;
    memset(m_szIccId, 0x00, sizeof(m_szIccId));

    if (m_pModemData != NULL) {
        sit_sim_iccid_info_ind *data = (sit_sim_iccid_info_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_ICCID_INFO) {
            m_nIccidLen = data->iccid_len;
            if (m_nIccidLen > 0) {
                memcpy(m_szIccId, data->iccid, MAX_ICCID_LEN);
            }
        }
    }
}

const BYTE *ProtocolSimIccidInfoAdapter::GetIccId() const
{
    if (m_nIccidLen > 0) {
        return m_szIccId;
    }

    return NULL;
}

/* ProtocolSimSetCarrierRestrictionsAdapter */
int ProtocolSimSetCarrierRestrictionsAdapter::GetLenAllowedCarriers() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_net_set_carrier_restriction_rsp *data = (sit_net_set_carrier_restriction_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_SET_CARRIER_RESTRICTIONS)
            ret = data->allowed_carriers_len;
    }

    return ret;
}

/* ProtocolSimGetCarrierRestrictionsAdapter */
void ProtocolSimGetCarrierRestrictionsAdapter::Init()
{
    m_nLenAllowedCarriers = 0;
    m_nLenExcludedCarriers = 0;
    void *m_pAllowedCarriers = NULL;
    void *m_pExcludedCarriers = NULL;

    if (m_pModemData != NULL) {
        sit_net_get_carrier_restriction_rsp *data = (sit_net_get_carrier_restriction_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CARRIER_RESTRICTIONS) {
            m_nLenAllowedCarriers = data->allowed_carriers_len;
            m_nLenExcludedCarriers = data->excluded_carriers_len;
            m_pAllowedCarriers = (void *)data->allowed_carrier_list;
            m_pExcludedCarriers = (void *)data->excluded_carrier_list;
        }
    }
}

int ProtocolSimGetCarrierRestrictionsAdapter::GetAllowedCarriers(RIL_Carrier *pCarriers, int nSize) const
{
    // TODO: need to implement
    RilLogI("%s() need to implement", __FUNCTION__);

    if (pCarriers == NULL || nSize == 0 || m_pAllowedCarriers == NULL) return 0;

    return 0;
}

int ProtocolSimGetCarrierRestrictionsAdapter::GetExcludedCarriers(RIL_Carrier *pCarriers, int nSize) const
{
    // TODO: need to implement
    RilLogI("%s() need to implement", __FUNCTION__);

    if (pCarriers == NULL || nSize == 0 || m_pExcludedCarriers == NULL) return 0;

    return 0;
}

/* ProtocolUiccSubStatusChangeAdapter */
int ProtocolUiccSubStatusChangeAdapter::GetState() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_uicc_sub_state_changed_ind *data = (sit_sim_uicc_sub_state_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_UICC_SUBSCRIPTION_STATE_CHANGED)
            ret = data->state;
    }

    return ret;
}

/* ProtocolSimLockInfoAdapter */
int ProtocolSimLockInfoAdapter::GetPolicy() const {
    if (m_pModemData != NULL) {
        sit_sim_get_sim_lock_info_rsp *data = (sit_sim_get_sim_lock_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_GET_SIM_LOCK_INFO) {
            return data->policy & 0xF;
        }
    }
    return -1;
}
int ProtocolSimLockInfoAdapter::GetStatus() const {
    if (m_pModemData != NULL) {
        sit_sim_get_sim_lock_info_rsp *data = (sit_sim_get_sim_lock_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_GET_SIM_LOCK_INFO) {
            return data->status & 0xF;
        }
    }
    return -1;
}
int ProtocolSimLockInfoAdapter::GetLockType() const {
    if (m_pModemData != NULL) {
        sit_sim_get_sim_lock_info_rsp *data = (sit_sim_get_sim_lock_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_GET_SIM_LOCK_INFO) {
            return data->lockType & 0xF;
        }
    }
    return -1;
}
int ProtocolSimLockInfoAdapter::GetMaxRetryCount() const {
    if (m_pModemData != NULL) {
        sit_sim_get_sim_lock_info_rsp *data = (sit_sim_get_sim_lock_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_GET_SIM_LOCK_INFO) {
            return data->maxRetryCount & 0xFF;
        }
    }
    return 0;
}
int ProtocolSimLockInfoAdapter::GetRemainCount() const {
    if (m_pModemData != NULL) {
        sit_sim_get_sim_lock_info_rsp *data = (sit_sim_get_sim_lock_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_GET_SIM_LOCK_INFO) {
            return data->reaminCount & 0xFF;
        }
    }
    return 0;
}
int ProtocolSimLockInfoAdapter::GetLockCodeCount() const {
    if (m_pModemData != NULL) {
        sit_sim_get_sim_lock_info_rsp *data = (sit_sim_get_sim_lock_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_GET_SIM_LOCK_INFO) {
            return data->lockCodeCount & 0xFFFF;
        }
    }
    return 0;
}
const char *ProtocolSimLockInfoAdapter::GetLockCode() const {
    if (m_pModemData != NULL) {
        sit_sim_get_sim_lock_info_rsp *data = (sit_sim_get_sim_lock_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_GET_SIM_LOCK_INFO) {
            return data->lockCode;
        }
    }
    return NULL;
}

int ProtocolSimLockInfoAdapter::GetLockCodeSize() const {
    int size = 0;
    int lockType = GetLockType();
    int entryCount = GetLockCodeCount();
    if (lockType != SIT_LOCK_TYPE_UNKNOWN && entryCount > 0 && GetLockCode() != NULL) {
        if (lockType == SIT_LOCK_TYPE_PN) {
            size = 6 * entryCount;
        }
        else {
            size = 2 * entryCount;
        }
    }
    return size;
}

/* Radio Config */
int ProtocolSimSlotStatusAdapter::GetNumOfSlotStatus() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_get_slot_status_resp *data = (sit_sim_get_slot_status_resp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = (int)(data->num_of_info);
    }
    return ret;
}

int ProtocolSimSlotStatusAdapter::GetCardState(int phy_slotId) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_get_slot_status_resp *data = (sit_sim_get_slot_status_resp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = (int)(data->info[phy_slotId].card_state);
    }
    return ret;
}

int ProtocolSimSlotStatusAdapter::GetSlotState(int phy_slotId) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_get_slot_status_resp *data = (sit_sim_get_slot_status_resp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = (int)(data->info[phy_slotId].slot_state);
    }
    return ret;
}

int ProtocolSimSlotStatusAdapter::GetAtrSize(int phy_slotId) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_get_slot_status_resp *data = (sit_sim_get_slot_status_resp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = ((int)(data->info[phy_slotId].atr_len)) * 2 + 1;
    }
    return ret;
}

char *ProtocolSimSlotStatusAdapter::GetAtr(int phy_slotId) const
{
    char *pAtr = NULL;
    if(m_pModemData != NULL) {
        sit_sim_get_slot_status_resp *data = (sit_sim_get_slot_status_resp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED)) {
            int size = ((int)(data->info[phy_slotId].atr_len)) * 2 + 1;
            pAtr = new char[size];
            int ret = Value2HexString(pAtr, data->info[phy_slotId].atr, (int)(data->info[phy_slotId].atr_len));
            if (ret == -1) {
                delete [] pAtr;
                return NULL;
            }
            return pAtr;
        }
    }
    return NULL;
}

int ProtocolSimSlotStatusAdapter::GetLogicalSlotId(int phy_slotId) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_get_slot_status_resp *data = (sit_sim_get_slot_status_resp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = (int) (data->info[phy_slotId].logicalSlotId);
    }
    return ret;
}

int ProtocolSimSlotStatusAdapter::GetIccIdSize(int phy_slotId) const
{
    if (m_pModemData != NULL) {
        sit_sim_get_slot_status_resp *data = (sit_sim_get_slot_status_resp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            return (int)(data->info[phy_slotId].iccid_len * 2 + 1);
    }
    return 0;
}

string ProtocolSimSlotStatusAdapter::GetIccId(int phy_slotId) const
{
    if(m_pModemData != NULL) {
        sit_sim_get_slot_status_resp *data = (sit_sim_get_slot_status_resp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED)) {
            string strIccid = bchToString(data->info[phy_slotId].iccid, (int)(data->info[phy_slotId].iccid_len));
            return strIccid;
        }
    }
    return NULL;
}

int ProtocolSimSlotStatusAdapter::GetEidSize(int phy_slotId) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_get_slot_status_resp *data = (sit_sim_get_slot_status_resp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = ((int)(data->info[phy_slotId].eid_len)) * 2 + 1;
    }
    return ret;
}

char *ProtocolSimSlotStatusAdapter::GetEid(int phy_slotId) const
{
    char *pEid = NULL;
    if(m_pModemData != NULL) {
        sit_sim_get_slot_status_resp *data = (sit_sim_get_slot_status_resp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED)) {
            int size = ((int)(data->info[phy_slotId].eid_len)) * 2 + 1;
            pEid = new char[size];
            int ret = Value2HexString(pEid, data->info[phy_slotId].eid, (int)(data->info[phy_slotId].eid_len));
            if (ret == -1) {
                delete [] pEid;
                return NULL;
            }
            return pEid;
        }
    }
    return NULL;
}

// Slot status changed
int ProtocolSlotStatusChangedAdapter::GetNumOfSlotStatus() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_slot_status_changed_ind *data = (sit_sim_slot_status_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = (int)(data->num_of_info);
    }
    return ret;
}

int ProtocolSlotStatusChangedAdapter::GetCardState(int phy_slotId) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_slot_status_changed_ind *data = (sit_sim_slot_status_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = (int)(data->info[phy_slotId].card_state);
    }
    return ret;
}

int ProtocolSlotStatusChangedAdapter::GetSlotState(int phy_slotId) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_slot_status_changed_ind *data = (sit_sim_slot_status_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = (int)(data->info[phy_slotId].slot_state);
    }
    return ret;
}

int ProtocolSlotStatusChangedAdapter::GetAtrSize(int phy_slotId) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_slot_status_changed_ind *data = (sit_sim_slot_status_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = ((int)(data->info[phy_slotId].atr_len)) * 2 + 1;
    }
    return ret;
}

char *ProtocolSlotStatusChangedAdapter::GetAtr(int phy_slotId) const
{
    char *pAtr = NULL;
    if(m_pModemData != NULL) {
        sit_sim_slot_status_changed_ind *data = (sit_sim_slot_status_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED)) {
            int size = ((int)(data->info[phy_slotId].atr_len)) * 2 + 1;
            pAtr = new char[size];
            int ret = Value2HexString(pAtr, data->info[phy_slotId].atr, (int)(data->info[phy_slotId].atr_len));
            if (ret == -1) {
                delete [] pAtr;
                return NULL;
            }
            return pAtr;
        }
    }
    return NULL;
}

int ProtocolSlotStatusChangedAdapter::GetLogicalSlotId(int phy_slotId) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_slot_status_changed_ind *data = (sit_sim_slot_status_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = (int) (data->info[phy_slotId].logicalSlotId);
    }
    return ret;
}

int ProtocolSlotStatusChangedAdapter::GetIccIdSize(int phy_slotId) const
{
    if (m_pModemData != NULL) {
        sit_sim_slot_status_changed_ind *data = (sit_sim_slot_status_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            return (int)(data->info[phy_slotId].iccid_len * 2 + 1);
    }
    return 0;
}

string ProtocolSlotStatusChangedAdapter::GetIccId(int phy_slotId) const
{
    if(m_pModemData != NULL) {
        sit_sim_slot_status_changed_ind *data = (sit_sim_slot_status_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED)) {
            string strIccid = bchToString(data->info[phy_slotId].iccid, (int)(data->info[phy_slotId].iccid_len));
            return strIccid;
        }
    }
    return NULL;
}

int ProtocolSlotStatusChangedAdapter::GetEidSize(int phy_slotId) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_sim_slot_status_changed_ind *data = (sit_sim_slot_status_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED))
            ret = ((int)(data->info[phy_slotId].eid_len)) * 2 + 1;
    }
    return ret;
}

char *ProtocolSlotStatusChangedAdapter::GetEid(int phy_slotId) const
{
    char *pEid = NULL;
    if(m_pModemData != NULL) {
        sit_sim_slot_status_changed_ind *data = (sit_sim_slot_status_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_SLOT_STATUS
                || data->hdr.id == SIT_IND_SIM_SLOT_STATUS_CHANGED)) {
            int size = ((int)(data->info[phy_slotId].eid_len)) * 2 + 1;
            pEid = new char[size];
            int ret = Value2HexString(pEid, data->info[phy_slotId].eid, (int)(data->info[phy_slotId].eid_len));
            if (ret == -1) {
                delete [] pEid;
                return NULL;
            }
            return pEid;
        }
    }
    return NULL;
}

