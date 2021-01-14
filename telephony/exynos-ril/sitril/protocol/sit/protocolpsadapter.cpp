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
 * protocolpsadapter.cpp
 *
 *  Created on: 2014. 6. 27.
 *      Author: sungwoo48.choi
 */
#include "protocolpsadapter.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define NUM_OF_ELM(a)       (int)(sizeof(a) / sizeof(a[0]))

// Map of TYPE to size
static int getSizeOfExtensionPayload(data_call_extension_header_type type)
{
    switch(type)
    {
      case EXT_PCSCF:
          return sizeof(sit_pdp_data_call_item_pcscf_ext);
      case EXT_LP_RETRYTIME:
          return sizeof(sit_pdp_data_call_item_ext_lp_retrytime);
      default:
          break;
    }
    return 0;
}

// Using Simple IPC Version to fixed size addtional block scheme
static sit_pdp_data_call_item_pcscf_ext *getExtensionPayload(void *dataCallList)
{
    char *start = NULL;
    const size_t legacyHeaderSize = LEN_DC_V11;
    // start from PCSCF extension
    start = (char *)(dataCallList) + legacyHeaderSize;

    int IPC_version = start[0];

    // IPC_version field
    if(IPC_version >= 2) {
        RilLogV("IPC version is %d, pcscf_ext blk start @ %p", start);
        return (sit_pdp_data_call_item_pcscf_ext *)start;
    }
    else if(IPC_version == 1){
        RilLogV("IPC version is 1, return NULL");
        return NULL;
    }
    return NULL;
}

static void *searchNextExtensionPayload(data_call_extension_header_type type, void *dataCallList, int remainingPayloadSize)
{
    char *start = NULL;
    const size_t legacyHeaderSize = LEN_DC_V11;
    // start from PCSCF extension
    start = (char *)(dataCallList) + legacyHeaderSize;

    remainingPayloadSize -= legacyHeaderSize;
    int IPC_version = start[0];

    // IPC_version field
    if((IPC_version == 2 || IPC_version == 3) && type == EXT_PCSCF) {
        RilLogV("IPC version is %d, start @ %p", IPC_version, start);
        return start;
    }
    else if(IPC_version == 1){
        RilLogV("IPC version is 1, return NULL");
        return NULL;
    }
    // IPC_version == 3 is reserved for EXT_PCSCF + RETRYTIMER

    data_call_extension_header_type currentEHT = EXT_PCSCF;
    data_call_extension_header_type nextEHT = (data_call_extension_header_type)start[1];
    if(type != EXT_PCSCF && IPC_version > 3)
    {
        while(nextEHT != NO_NEXT_HEADER && remainingPayloadSize > 0){
            // Loop to check next
            start += getSizeOfExtensionPayload(currentEHT);
            remainingPayloadSize -= getSizeOfExtensionPayload(currentEHT);
            // TBD: DecodeNextBlock or return
            if(nextEHT == type) return start;
            currentEHT = nextEHT;
            nextEHT = (data_call_extension_header_type)start[0];
        }
    }
    return start;
}

static void fillPcscfExtInfoForLegacy(DataCall &out)
{
    out.IPC_version = 1;
    out.pcscf_ext_count = 0;
}

static void fillExtInfoForLpRetryTime(DataCall &out, const sit_pdp_data_call_item *dataCallList, int payload_size)
{
    sit_pdp_data_call_item_ext_lp_retrytime *data =
        (sit_pdp_data_call_item_ext_lp_retrytime *)
        ( ((char *) dataCallList) + payload_size
                                  - sizeof(sit_pdp_data_call_item_ext_lp_retrytime));
    out.suggestedRetryTime = (int) data->suggestedRetryTime;
    RilLogV("retrytime:%d", data->suggestedRetryTime);
    // ignore low_priority
}

static void fillAMBR(DataCall &out, const sit_pdp_data_call_item *dataCallList, int payload_size)
{
    sit_pdp_data_call_item_ext_ambr *data =
        (sit_pdp_data_call_item_ext_ambr *)
        ( ((char *) dataCallList) + payload_size
                                  - sizeof(sit_pdp_data_call_item_ext_ambr));
    out.ambr = *((AMBR *)data);
    RilLogV("octet2~8 = %d, (%d, %d), (%d, %d), (%d, %d)",
            data->octet2, data->octet3, data->octet4,
            data->octet5, data->octet6,
            data->octet7, data->octet8);
}


static void DumpPcscfExt(DataCall &out)
{
    RilLogV("Dump PCSCF Extension");
    char buf[100];
    for(int i = 0; i < 5; i++) {
        int len = 0;
        for(int j=0; j<4; j++) {
            len += snprintf(&buf[len], 100-len, "%02x", out.ipv4.pcscf[MAX_IPV4_ADDR_LEN*MAX_PCSCF_NUM + j+4*i]);
        }
        RilLogV("IPv4[%d]: %s", i, buf);
        len = 0;
        for(int j=0; j<16; j++) {
            len += snprintf(&buf[len], 100-len, "%02x", out.ipv6.pcscf[MAX_IPV6_ADDR_LEN*MAX_PCSCF_NUM + j+16*i]);
        }
        RilLogV("IPv6[%d]: %s", i, buf);
    }
}

static void DumpPcscfExtPayload(char *data)
{
    RilLogV("Dump PCSCF Extension Payload");
    char buf[100];
    for(int i = 0; i < 5; i++) {
        int len = 0;
        for(int j=0; j<4; j++) {
            len += snprintf(&buf[len], 100-len, "%02x", data[2 + 4*i + j]);
        }
        RilLogV("IPv4[%d]: %s", i, buf);
        len = 0;
        for(int j=0; j<16; j++) {
            len += snprintf(&buf[len], 100-len, "%02x", data[2 + 4*5 + 16*i + j]);
        }
        RilLogV("IPv6[%d]: %s", i, buf);
    }
}

static void FillPcscfExt(DataCall &out, const sit_pdp_data_call_item_pcscf_ext *data, BYTE pcscf_type)
{
    //DumpPcscfExtPayload( (char*) data);

    if( (pcscf_type & SIT_PDP_PDP_TYPE_IPV4IPV6) & SIT_PDP_PDP_TYPE_IPV4)
    {
        memcpy(&out.ipv4.pcscf[MAX_IPV4_ADDR_LEN * MAX_PCSCF_NUM], &(data->pcscf_ipv4_ext[0]), MAX_IPV4_ADDR_LEN * MAX_PCSCF_EXT_NUM);
        RilLogV("IPV4 PCSCF Copy from %p to %p with %d bytes, offset:%d",
                &(data->pcscf_ipv4_ext[0]), &out.ipv4.pcscf[MAX_IPV4_ADDR_LEN*MAX_PCSCF_NUM],
                MAX_IPV4_ADDR_LEN * MAX_PCSCF_EXT_NUM, MAX_IPV4_ADDR_LEN * MAX_PCSCF_NUM);
    }
    if( (pcscf_type & SIT_PDP_PDP_TYPE_IPV4IPV6) & SIT_PDP_PDP_TYPE_IPV6)
    {
        memcpy(&out.ipv6.pcscf[MAX_IPV6_ADDR_LEN * MAX_PCSCF_NUM], &(data->pcscf_ipv6_ext[0]), MAX_IPV6_ADDR_LEN * MAX_PCSCF_EXT_NUM);
        RilLogV("IPV6 PCSCF Copy from %p to %p with %d bytes, offset:%d",
                &(data->pcscf_ipv6_ext[0]), &out.ipv6.pcscf[MAX_IPV6_ADDR_LEN*MAX_PCSCF_NUM],
                MAX_IPV6_ADDR_LEN * MAX_PCSCF_EXT_NUM, MAX_IPV6_ADDR_LEN * MAX_PCSCF_NUM);
    }
    RilLogV("Decoded Information IPC_version:%d @ %p", data->IPC_version, &data->IPC_version);
    out.IPC_version = data->IPC_version;
    out.pcscf_ext_count = 5;
    //DumpPcscfExt(out);
}


// Now support extension chain
// dataCallList will get each data_call_item
static int processPcscfExtension(DataCall &out, const sit_pdp_data_call_item *dataCallList, int remainingPayloadSize)
{
    if (dataCallList == NULL) {
        return 1;
    }

    RilLogV("processPcscfExtension, dataCallList @ %p, size:%d", dataCallList, remainingPayloadSize);
    // DataCall should be already parsed, Don't check validity
    //const sit_pdp_data_call_item_pcscf_ext *data = (sit_pdp_data_call_item_pcscf_ext *) searchNextExtensionPayload(EXT_PCSCF, (void *)dataCallList, remainingPayloadSize);
    const sit_pdp_data_call_item_pcscf_ext *data = (sit_pdp_data_call_item_pcscf_ext *) getExtensionPayload((void *)dataCallList);
    if( data == NULL ) {
        RilLogV("Can't find next extension payload");
        fillPcscfExtInfoForLegacy(out);
        return 1;
    }

    RilLogV("Found PCSCF Extension Payload, pcscf_ext @ %p, pcscf_type:%d", data, dataCallList->pcscf_type);
    FillPcscfExt(out, data, dataCallList->pcscf_type);
    return out.IPC_version;
}

// For v11
// Changed to care about one DataCall Item
// Indexing should be completed before calling.
static bool GetDataCallFromDataCallList_v11(DataCall &out, const sit_pdp_data_call_item *dataCallList)
{
    if (dataCallList == NULL) {
        return false;
    }

    const sit_pdp_data_call_item *data = dataCallList;

    out.status = (int)data->status;
    out.cid = (int)data->cid;
    out.active = (int)data->active;
    out.pdpType = (int)data->pdp_type;
    // Do not store : out.dnsType, out.pcscfType
    out.mtu_size = (int)data->mtu_size;
    out.pco = (int)data->pco;
    memset(&(out.ipv4), 0, sizeof(PDP_ADDR_V4));
    memset(&(out.ipv6), 0, sizeof(PDP_ADDR_V6));

    if (out.pdpType == SIT_PDP_PDP_TYPE_IPV4) {
        out.ipv4.valid = TRUE;
        memcpy(out.ipv4.addr, data->address, 4);
    }
    else if (out.pdpType == SIT_PDP_PDP_TYPE_IPV6) {
        out.ipv6.valid = TRUE;
        memcpy(out.ipv6.addr, data->address + 4, 16);
    }
    else if (out.pdpType == SIT_PDP_PDP_TYPE_IPV4IPV6) {
        out.ipv4.valid = TRUE;
        memcpy(out.ipv4.addr, data->address, 4);

        out.ipv6.valid = TRUE;
        memcpy(out.ipv6.addr, data->address + 4, 16);
    }

    // Add for DNS IPv4/IPv6 Chcker Routine
    if((int)data->dns_type == SIT_PDP_PDP_TYPE_IPV4)
    {
        memcpy(out.ipv4.dns1, data->primary_dns, 4);
        memcpy(out.ipv4.dns2, data->secondary_dns, 4);
    }
    else if((int)data->dns_type == SIT_PDP_PDP_TYPE_IPV6)
    {
        memcpy(out.ipv6.dns1, data->primary_dns + 4, 16);
        memcpy(out.ipv6.dns2, data->secondary_dns + 4, 16);
    }
    else if((int)data->dns_type == SIT_PDP_PDP_TYPE_IPV4IPV6)
    {
        memcpy(out.ipv4.dns1, data->primary_dns, 4);
        memcpy(out.ipv4.dns2, data->secondary_dns, 4);
        memcpy(out.ipv6.dns1, data->primary_dns + 4, 16);
        memcpy(out.ipv6.dns2, data->secondary_dns + 4, 16);
    }

    // Add for PCSCF IPv4/IPv6 Chcker Routine
#define TEST_FAKE_PCSCF_EXT (0)
#if (TEST_FAKE_PCSCF_EXT == 0)
    if((int)data->pcscf_type == SIT_PDP_PDP_TYPE_IPV4)
    {
        memcpy(out.ipv4.pcscf, &(data->pcscf[0]), MAX_IPV4_ADDR_LEN * MAX_PCSCF_NUM);
    }
    else if((int)data->pcscf_type == SIT_PDP_PDP_TYPE_IPV6)
    {
        memcpy(out.ipv6.pcscf, &(data->pcscf[MAX_IPV4_ADDR_LEN * MAX_PCSCF_NUM]),
               MAX_IPV6_ADDR_LEN * MAX_PCSCF_NUM);
    }
    else if((int)data->pcscf_type == SIT_PDP_PDP_TYPE_IPV4IPV6)
    {
        memcpy(out.ipv4.pcscf, &(data->pcscf[0]), MAX_IPV4_ADDR_LEN * MAX_PCSCF_NUM);
        memcpy(out.ipv6.pcscf, &(data->pcscf[MAX_IPV4_ADDR_LEN * MAX_PCSCF_NUM]),
               MAX_IPV6_ADDR_LEN * MAX_PCSCF_NUM);
    }
#else
    if((int)data->pcscf_type == SIT_PDP_PDP_TYPE_IPV4)
    {
        memcpy(&out.ipv4.pcscf[0], &(data->pcscf[0]), 4);
        memcpy(&out.ipv4.pcscf[4], &(data->pcscf[0]), 4);
        memcpy(&out.ipv4.pcscf[8], &(data->pcscf[0]), 4);
    }
    else if((int)data->pcscf_type == SIT_PDP_PDP_TYPE_IPV6)
    {
        memcpy(&out.ipv6.pcscf[0], &(data->pcscf[12]), MAX_IPV6_ADDR_LEN);
        memcpy(&out.ipv6.pcscf[16], &(data->pcscf[12]), MAX_IPV6_ADDR_LEN);
        memcpy(&out.ipv6.pcscf[32], &(data->pcscf[12]), MAX_IPV6_ADDR_LEN);
    }
    else if((int)data->pcscf_type == SIT_PDP_PDP_TYPE_IPV4IPV6)
    {
        memcpy(&out.ipv4.pcscf[0], &(data->pcscf[0]), 4);
        memcpy(&out.ipv4.pcscf[4], &(data->pcscf[0]), 4);
        memcpy(&out.ipv4.pcscf[8], &(data->pcscf[0]), 4);
        memcpy(&out.ipv6.pcscf[0], &(data->pcscf[12]), MAX_IPV6_ADDR_LEN);
        memcpy(&out.ipv6.pcscf[16], &(data->pcscf[12]), MAX_IPV6_ADDR_LEN);
        memcpy(&out.ipv6.pcscf[32], &(data->pcscf[12]), MAX_IPV6_ADDR_LEN);
    }
#endif

    return true;
}

// For under v10, This function can use v11 DataCall structure
static bool GetDataCallFromDataCallList_v10(DataCall &out, int index, const sit_pdp_data_call_item_old *dataCallList, int listSize)
{
    if (index < 0 || listSize < 0 || index >= listSize) {
        return false;
    }

    if (dataCallList == NULL) {
        return false;
    }

    const sit_pdp_data_call_item_old *data = dataCallList + index;
    /*DEADCODE <- dataCallList cannot be null
    if (data == NULL) {
        return false;
    }
    */

    out.status = (int)data->status;
    out.cid = (int)data->cid;
    out.active = (int)data->active;
    out.pdpType = (int)data->pdp_type;
    memset(&(out.ipv4), 0, sizeof(PDP_ADDR_V4));
    memset(&(out.ipv6), 0, sizeof(PDP_ADDR_V6));

    if (out.pdpType == SIT_PDP_PDP_TYPE_IPV4) {
        out.ipv4.valid = TRUE;
        memcpy(out.ipv4.addr, data->address, 4);
        memcpy(out.ipv4.dns1, data->primary_dns, 4);
        memcpy(out.ipv4.dns2, data->secondary_dns, 4);
        memcpy(out.ipv4.pcscf, &(data->pcscf[0]), MAX_IPV4_ADDR_LEN * MAX_PCSCF_NUM);
    }
    else if (out.pdpType == SIT_PDP_PDP_TYPE_IPV6) {
        out.ipv6.valid = TRUE;
        memcpy(out.ipv6.addr, data->address, 16);
        memcpy(out.ipv6.dns1, data->primary_dns, 16);
        memcpy(out.ipv6.dns2, data->secondary_dns, 16);
        memcpy(out.ipv6.pcscf, &(data->pcscf[0]), MAX_IPV6_ADDR_LEN * MAX_PCSCF_NUM);
    }
    else if (out.pdpType == SIT_PDP_PDP_TYPE_IPV4IPV6) {
        out.ipv4.valid = TRUE;
        memcpy(out.ipv4.addr, data->address, 4);
        memcpy(out.ipv4.dns1, data->primary_dns, 4);
        memcpy(out.ipv4.dns2, data->secondary_dns, 4);
        memcpy(out.ipv4.pcscf, &(data->pcscf[0]), MAX_IPV4_ADDR_LEN * MAX_PCSCF_NUM);

        out.ipv6.valid = TRUE;
        memcpy(out.ipv6.addr, data->address + 4, 16);
        memcpy(out.ipv6.dns1, data->primary_dns + 4, 16);
        memcpy(out.ipv6.dns2, data->secondary_dns + 4, 16);
        memcpy(out.ipv6.pcscf, &(data->pcscf[MAX_IPV4_ADDR_LEN * MAX_PCSCF_NUM]),
               MAX_IPV6_ADDR_LEN * MAX_PCSCF_NUM);
    }

    return true;
}


/**
 * ProtocolPsSetupDataCallAdapter
 */
ProtocolPsSetupDataCallAdapter::ProtocolPsSetupDataCallAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
    memset(&m_dataCall, 0, sizeof(m_dataCall));
    Init();
}
void ProtocolPsSetupDataCallAdapter::Init()
{
    UINT payload_size = 0;
    if (m_pModemData != NULL && GetErrorCode() == RCM_E_SUCCESS) {
        sit_pdp_setup_data_call_rsp *data = (sit_pdp_setup_data_call_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_SETUP_DATA_CALL) {
            RilLogV("Modem RSP Length:%d, orgSize:%d", m_pModemData->GetLength(), sizeof(sit_pdp_setup_data_call_rsp));
            //m_pModemData->Dump();
            payload_size = sizeof(sit_pdp_setup_data_call_rsp);
            if( payload_size <= m_pModemData->GetLength() ) {
                RilLogV("Decode DC V11, payload:%d", payload_size);
                if (!GetDataCallFromDataCallList_v11(m_dataCall, &data->data_call)) {
                    memset(&m_dataCall, 0, sizeof(m_dataCall));
                }
                payload_size += sizeof(sit_pdp_data_call_item_pcscf_ext);
                if( payload_size <= m_pModemData->GetLength() ) {
                    RilLogV("Decode PCSCF EXT, payload:%d", payload_size);
                    // Extension Payload, IPC Version 2 is expected
                    // Process Extension
                    int IPC_version = processPcscfExtension(m_dataCall, &data->data_call,
                                          m_pModemData->GetLength() - sizeof(sit_pdp_setup_data_call_rsp));
                    //payload_size += sizeof(sit_pdp_data_call_item_ext_lp_retrytime);
                    //if (payload_size == m_pModemData->GetLength() ) {
                    if (IPC_version >= 3) {
                        RilLogV("Decode Retry SuggestedTime, payload:%d, data_call:%p",
                                payload_size, &data->data_call);
                        fillExtInfoForLpRetryTime(m_dataCall, &data->data_call,
                                                  LEN_DC_V11_LP_RETRY);
                    }
                    if (IPC_version == 4) {
                        RilLogV("Decode AMBR octet2~7, CID:%d, payload_size:%d", data->data_call.cid, payload_size);
                        fillAMBR(m_dataCall, &data->data_call, LEN_DC_V11_AMBR);
                    }
                } else {
                    fillPcscfExtInfoForLegacy(m_dataCall);
                    return;
                }
            } else {
                // DEPRECATED
                sit_pdp_setup_data_call_rsp_old *data = (sit_pdp_setup_data_call_rsp_old *)m_pModemData->GetRawData();
                if (!GetDataCallFromDataCallList_v10(m_dataCall, 0, &data->data_call, 1)) {
                    memset(&m_dataCall, 0, sizeof(m_dataCall));
                }
                fillPcscfExtInfoForLegacy(m_dataCall);
            }
        }
    }
}

UINT ProtocolPsSetupDataCallAdapter::GetErrorCode() const
{
    UINT errorCode = ProtocolRespAdapter::GetErrorCode();
    switch (errorCode) {
    case RCM_E_SUCCESS:
        return RIL_E_SUCCESS;
    case RCM_E_RADIO_NOT_AVAILABLE:
        return RIL_E_RADIO_NOT_AVAILABLE;
    case RCM_E_GENERIC_FAILURE:
        return RIL_E_GENERIC_FAILURE;
    case RCM_E_REQUEST_NOT_SUPPORTED:
        return RIL_E_REQUEST_NOT_SUPPORTED;
    case RCM_E_OP_NOT_ALLOWED_DURING_VOICE_CALL:
        return RIL_E_OP_NOT_ALLOWED_DURING_VOICE_CALL;
    case RCM_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW:
        return RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW;
    } // end switch ~

    return RIL_E_GENERIC_FAILURE;
}

int ProtocolPsSetupDataCallAdapter::GetAddrInfo(PDP_ADDR *pAddr, int pdpType) const
{
    if (pAddr == NULL) {
        return -1;
    }

    memset(pAddr, 0, sizeof(PDP_ADDR));

    switch (pdpType) {
    case SIT_PDP_PDP_TYPE_IPV4:
    case SIT_PDP_PDP_TYPE_IPV6:
    case SIT_PDP_PDP_TYPE_IPV4IPV6:
    case SIT_PDP_PDP_TYPE_PPP:
        break;
    default:
        return -1;
    }

    memset(pAddr, 0, sizeof(PDP_ADDR));
    if (pdpType == SIT_PDP_PDP_TYPE_IPV4 || pdpType == SIT_PDP_PDP_TYPE_IPV4IPV6 ||
            pdpType == SIT_PDP_PDP_TYPE_PPP) {
        if (m_dataCall.ipv4.valid) {
            memcpy(&pAddr->ipv4, &m_dataCall.ipv4, sizeof(m_dataCall.ipv4));
        }
    }

    if (pdpType == SIT_PDP_PDP_TYPE_IPV6 || pdpType == SIT_PDP_PDP_TYPE_IPV4IPV6) {
        if (m_dataCall.ipv6.valid) {
            memcpy(&pAddr->ipv6, &m_dataCall.ipv6, sizeof(m_dataCall.ipv6));
        }
    }

    return 0;
}

int ProtocolPsSetupDataCallAdapter::GetAddrInfo(PDP_ADDR *pAddr) const
{
    if (pAddr == NULL) {
        return -1;
    }

    // PDP_ADDR is union type, it's same to PDP_ADDR_V6
    memset(pAddr, 0, sizeof(PDP_ADDR));
    memcpy(&pAddr->ipv4, &m_dataCall.ipv4, sizeof(m_dataCall.ipv4));
    memcpy(&pAddr->ipv6, &m_dataCall.ipv6, sizeof(m_dataCall.ipv6));
    return 0;
}

/**
 * ProtocolPsDataCallListAdapter
 */
ProtocolPsDataCallListAdapter::ProtocolPsDataCallListAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData), m_dataCallNum(0)
{
    memset(m_dataCallList, 0, sizeof(m_dataCallList));
    Init();
}
void ProtocolPsDataCallListAdapter::Init()
{
    m_dataCallNum = 0;
    dc_type dct = DC_V10;

    if (m_pModemData != NULL && GetErrorCode() == RCM_E_SUCCESS) {
        sit_pdp_get_data_call_list_rsp *data = (sit_pdp_get_data_call_list_rsp *)m_pModemData->GetRawData();
        if (data != NULL) {
            m_dataCallNum = data->datacall_info_num;
            if((m_dataCallNum * LEN_DC_V11 + LEN_DC_V11_IND_HDR)
               == m_pModemData->GetLength()) {
                dct = DC_V11;
            } else if((m_dataCallNum * LEN_DC_V11_PCSCF_EXT + LEN_DC_V11_IND_HDR)
               == m_pModemData->GetLength()) {
                dct = DC_EXTENSION;
            } else if((m_dataCallNum * LEN_DC_V11_LP_RETRY + LEN_DC_V11_IND_HDR)
               == m_pModemData->GetLength()) {
                dct = DC_RETRYTIME;
            } else if((m_dataCallNum * LEN_DC_V11_AMBR + LEN_DC_V11_IND_HDR)
               == m_pModemData->GetLength()) {
                dct = DC_AMBR;
            } else {
                dct = DC_V10;
            }
            RilLogV("dc_type = %d", dct);

            if (m_dataCallNum > NUM_OF_ELM(m_dataCallList)) {
                m_dataCallNum = NUM_OF_ELM(m_dataCallList);
            }

            FillDataCallListFromModemPayload(data, dct);
        }
    }
}

void ProtocolPsDataCallListAdapter::FillDataCallListFromModemPayload(
    const sit_pdp_get_data_call_list_rsp *data,
    dc_type dct)
{
    switch(dct) {
    case DC_AMBR:
        for (unsigned int i = 0; i < m_dataCallNum; i++) {
            sit_pdp_data_call_item *data_call =
                (sit_pdp_data_call_item *) ((char *) (&data->data_call[0]) + LEN_DC_V11_AMBR * i);
            if (!GetDataCallFromDataCallList_v11(m_dataCallList[i], data_call)) {
                memset(&m_dataCallList[i], 0, sizeof(DataCall));
            }
            int IPC_version = processPcscfExtension(m_dataCallList[i], data_call, sizeof(sit_pdp_data_call_item_pcscf_ext));
            fillExtInfoForLpRetryTime(m_dataCallList[i], data_call, LEN_DC_V11_AMBR);
            if (IPC_version == 4) {
                RilLogV("Decode AMBR octet2~7, CID:%d", data_call->cid);
                fillAMBR(m_dataCallList[i], data_call, LEN_DC_V11_AMBR);
            }
        } // end for i ~
        break;
    case DC_RETRYTIME:
        for (unsigned int i = 0; i < m_dataCallNum; i++) {
            sit_pdp_data_call_item *data_call =
                (sit_pdp_data_call_item *) ((char *) (&data->data_call[0]) + LEN_DC_V11_LP_RETRY * i);
            if (!GetDataCallFromDataCallList_v11(m_dataCallList[i], data_call)) {
                memset(&m_dataCallList[i], 0, sizeof(DataCall));
            }
            processPcscfExtension(m_dataCallList[i], data_call, sizeof(sit_pdp_data_call_item_pcscf_ext));
            fillExtInfoForLpRetryTime(m_dataCallList[i], data_call, LEN_DC_V11_LP_RETRY);
        } // end for i ~
        break;
    case DC_EXTENSION:
        // Need to reform for each DataCall item, Because each DataCall block will have extension block
        for (unsigned int i = 0; i < m_dataCallNum; i++) {
            sit_pdp_data_call_item *data_call =
                (sit_pdp_data_call_item *)((char *) (&data->data_call[0]) + LEN_DC_V11_PCSCF_EXT * i);
            if (!GetDataCallFromDataCallList_v11(m_dataCallList[i], data_call)) {
                memset(&m_dataCallList[i], 0, sizeof(DataCall));
            }
            processPcscfExtension(m_dataCallList[i], data_call, sizeof(sit_pdp_data_call_item_pcscf_ext));
        } // end for i ~
        break;
    case DC_V11:
        for (unsigned int i = 0; i < m_dataCallNum; i++) {
            if (!GetDataCallFromDataCallList_v11(m_dataCallList[i], &data->data_call[i])) {
                memset(&m_dataCallList[i], 0, sizeof(DataCall));
            }
            fillPcscfExtInfoForLegacy(m_dataCallList[i]);
        } // end for i ~
        break;
    case DC_V10:
    default:
        sit_pdp_get_data_call_list_rsp_old *data = (sit_pdp_get_data_call_list_rsp_old *) m_pModemData->GetRawData();
        for (unsigned int i = 0; i < m_dataCallNum; i++) {
            if (!GetDataCallFromDataCallList_v10(m_dataCallList[i], i, data->data_call, m_dataCallNum)) {
                memset(&m_dataCallList[i], 0, sizeof(DataCall));
            }
            fillPcscfExtInfoForLegacy(m_dataCallList[i]);
        } // end for i ~
        break;
    }
}


const DataCall *ProtocolPsDataCallListAdapter::GetDataCallList() const
{
    if (m_dataCallNum == 0)
        return NULL;
    return m_dataCallList;
}

const DataCall *ProtocolPsDataCallListAdapter::GetDataCallByIndex(unsigned int index) const
{
    if (index >= m_dataCallNum) {
        return NULL;
    }
    return &m_dataCallList[index];
}

const DataCall *ProtocolPsDataCallListAdapter::GetDataCallByCid(int cid) const
{
    for (unsigned int i = 0; i < m_dataCallNum; i++) {
        if (m_dataCallList[i].cid == cid)
            return &m_dataCallList[i];
    }
    return NULL;
}

/**
 * ProtocolPsDataCallListChangedAdapter
 */
ProtocolPsDataCallListChangedAdapter::ProtocolPsDataCallListChangedAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData), m_dataCallNum(0)
{
    memset(m_dataCallList, 0, sizeof(m_dataCallList));
    Init();
}
void ProtocolPsDataCallListChangedAdapter::Init()
{
    m_dataCallNum = 0;
    dc_type dct = DC_V10;

    if (m_pModemData != NULL) {
        sit_pdp_data_call_list_changed_ind *data = (sit_pdp_data_call_list_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_DATA_CALL_LIST_CHANGED) {

            m_dataCallNum = data->datacall_info_num;
            if((m_dataCallNum * LEN_DC_V11 + LEN_DC_V11_IND_HDR)
               == m_pModemData->GetLength()) {
                dct = DC_V11;
            } else if((m_dataCallNum * LEN_DC_V11_PCSCF_EXT + LEN_DC_V11_IND_HDR)
               == m_pModemData->GetLength()) {
                dct = DC_EXTENSION;
            } else if((m_dataCallNum * LEN_DC_V11_LP_RETRY + LEN_DC_V11_IND_HDR)
               == m_pModemData->GetLength()) {
                dct = DC_RETRYTIME;
            } else if((m_dataCallNum * LEN_DC_V11_AMBR + LEN_DC_V11_IND_HDR)
               == m_pModemData->GetLength()) {
                dct = DC_AMBR;
            } else {
                dct = DC_V10;
            }
            RilLogV("dc_type = %d", dct);

            if (m_dataCallNum > NUM_OF_ELM(m_dataCallList)) {
                m_dataCallNum = NUM_OF_ELM(m_dataCallList);
            }

            FillDataCallListFromModemPayload(data, dct);
        }
    }
}

void ProtocolPsDataCallListChangedAdapter::FillDataCallListFromModemPayload(
    const sit_pdp_data_call_list_changed_ind *data,
    dc_type dct)
{
    switch(dct) {
    case DC_AMBR:
        for (unsigned int i = 0; i < m_dataCallNum; i++) {
            sit_pdp_data_call_item *data_call =
                (sit_pdp_data_call_item *) ((char *) (&data->data_call[0]) + LEN_DC_V11_AMBR * i);
            if (!GetDataCallFromDataCallList_v11(m_dataCallList[i], data_call)) {
                memset(&m_dataCallList[i], 0, sizeof(DataCall));
            }
            int IPC_version = processPcscfExtension(m_dataCallList[i], data_call, sizeof(sit_pdp_data_call_item_pcscf_ext));
            fillExtInfoForLpRetryTime(m_dataCallList[i], data_call, LEN_DC_V11_AMBR);
            if (IPC_version == 4) {
                RilLogV("Decode AMBR octet2~7, CID:%d", data_call->cid);
                fillAMBR(m_dataCallList[i], data_call, LEN_DC_V11_AMBR);
            }
        } // end for i ~
        break;
    case DC_RETRYTIME:
        for (unsigned int i = 0; i < m_dataCallNum; i++) {
            sit_pdp_data_call_item *data_call =
                (sit_pdp_data_call_item *) ((char *) (&data->data_call[0]) + LEN_DC_V11_LP_RETRY * i);
            if (!GetDataCallFromDataCallList_v11(m_dataCallList[i], data_call)) {
                memset(&m_dataCallList[i], 0, sizeof(DataCall));
            }
            processPcscfExtension(m_dataCallList[i], data_call, sizeof(sit_pdp_data_call_item_pcscf_ext));
            fillExtInfoForLpRetryTime(m_dataCallList[i], data_call, LEN_DC_V11_LP_RETRY);
        } // end for i ~
        break;
    case DC_EXTENSION:
        // Need to reform for each DataCall item, Because each DataCall block will have extension block
        for (unsigned int i = 0; i < m_dataCallNum; i++) {
            sit_pdp_data_call_item *data_call =
                (sit_pdp_data_call_item *)((char *) (&data->data_call[0]) + LEN_DC_V11_PCSCF_EXT * i);
            if (!GetDataCallFromDataCallList_v11(m_dataCallList[i], data_call)) {
                memset(&m_dataCallList[i], 0, sizeof(DataCall));
            }
            processPcscfExtension(m_dataCallList[i], data_call, sizeof(sit_pdp_data_call_item_pcscf_ext));
        } // end for i ~
        break;
    case DC_V11:
        for (unsigned int i = 0; i < m_dataCallNum; i++) {
            if (!GetDataCallFromDataCallList_v11(m_dataCallList[i], &data->data_call[i])) {
                memset(&m_dataCallList[i], 0, sizeof(DataCall));
            }
            fillPcscfExtInfoForLegacy(m_dataCallList[i]);
        } // end for i ~
        break;
    case DC_V10:
    default:
        sit_pdp_data_call_list_changed_ind_old *data = (sit_pdp_data_call_list_changed_ind_old *) m_pModemData->GetRawData();
        for (unsigned int i = 0; i < m_dataCallNum; i++) {
            if (!GetDataCallFromDataCallList_v10(m_dataCallList[i], i, data->data_call, m_dataCallNum)) {
                memset(&m_dataCallList[i], 0, sizeof(DataCall));
            }
            fillPcscfExtInfoForLegacy(m_dataCallList[i]);
        } // end for i ~
        break;
    }
}

const DataCall *ProtocolPsDataCallListChangedAdapter::GetDataCallList() const
{
    if (m_dataCallNum == 0)
        return NULL;
    return m_dataCallList;
}

const DataCall *ProtocolPsDataCallListChangedAdapter::GetDataCallByIndex(unsigned int index) const
{
    if (index >= m_dataCallNum) {
        return NULL;
    }
    return &m_dataCallList[index];
}

const DataCall *ProtocolPsDataCallListChangedAdapter::GetDataCallByCid(int cid) const
{
    for (unsigned int i = 0; i < m_dataCallNum; i++) {
        if (m_dataCallList[i].cid == cid)
            return &m_dataCallList[i];
    }
    return NULL;
}

ProtocolPsDedicatedBearInfoAdapter::ProtocolPsDedicatedBearInfoAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
}

const DedicatedBearerInfo *ProtocolPsDedicatedBearInfoAdapter::GetDedicatedBearerInfo()
{
    if (m_pModemData != NULL) {
        sit_pdp_dedicated_bearer_info_ind *data = (sit_pdp_dedicated_bearer_info_ind *)m_pModemData->GetRawData();

        if (data != NULL && data->hdr.id == SIT_IND_DEDICATED_BEARER_INFO) {
             if ((unsigned int)m_pModemData->GetLength() >= sizeof(sit_pdp_dedicated_bearer_info_ind)) {
                return (DedicatedBearerInfo *)&data->status;
            }
        }
    }
    return NULL;
}

ProtocolPsNasTimerStatusAdapter::ProtocolPsNasTimerStatusAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
}

const SitNasTimerStatus *ProtocolPsNasTimerStatusAdapter::GetNasTimerStatus()
{
    if (m_pModemData != NULL) {
        sit_pdp_nas_timer_status_ind *data = (sit_pdp_nas_timer_status_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NAS_TIMER_STATUS) {
             if ((unsigned int)m_pModemData->GetLength() >= sizeof(sit_pdp_nas_timer_status_ind)) {
                return (SitNasTimerStatus *)&data->timer_type;
            }
        }
    }
    return NULL;
}

/**
 * ProtocolPsStartKeepAliveAdapter
 */
unsigned int ProtocolPsStartKeepAliveAdapter::getSessionHandle() const
{
    unsigned int ret = 0;
    if (m_pModemData != NULL) {
        sit_pdp_start_keepalive_rsp *data = (sit_pdp_start_keepalive_rsp *)m_pModemData->GetRawData();
        if (data != NULL ) {
            ret = data->session_handle;
        }
    }
    return ret;
}

int ProtocolPsStartKeepAliveAdapter::getCode() const
{
    int status = KEEPALIVE_INACTIVE;
    if (m_pModemData != NULL) {
        sit_pdp_start_keepalive_rsp *data = (sit_pdp_start_keepalive_rsp *)m_pModemData->GetRawData();
        if (data != NULL ) {
            status = data->status_code;
        }
    }
    return status;
}

/**
 * ProtocolPsKeepAliveStatusAdapter
 */
unsigned int ProtocolPsKeepAliveStatusAdapter::getSessionHandle() const
{
    unsigned int ret = 0;
    if (m_pModemData != NULL) {
        sit_pdp_keepalive_status_ind *data = (sit_pdp_keepalive_status_ind *)m_pModemData->GetRawData();
        if (data != NULL ) {
            ret = data->session_handle;
        }
    }
    return ret;
}

int ProtocolPsKeepAliveStatusAdapter::getCode() const
{
    int status = KEEPALIVE_INACTIVE;
    if (m_pModemData != NULL) {
        sit_pdp_keepalive_status_ind *data = (sit_pdp_keepalive_status_ind *)m_pModemData->GetRawData();
        if (data != NULL ) {
            status = data->status_code;
        }
    }
    return status;
}

/**
 * ProtocolPsPcoDataAdapter
 */
ProtocolPsPcoDataAdapter::ProtocolPsPcoDataAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData)
{
    if (m_pModemData != NULL) {
        sit_pdp_pco_data_ind *data = (sit_pdp_pco_data_ind *)m_pModemData->GetRawData();
        remainPcoBlocks = data->pco_num;
    } else {
        remainPcoBlocks = 0;
    }
    nextPcoBlockPos = 0;
}

int ProtocolPsPcoDataAdapter::GetCid() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_pdp_pco_data_ind *data = (sit_pdp_pco_data_ind *)m_pModemData->GetRawData();
        if (data != NULL ) {
            ret = data->cid;
        }
    }
    return ret;
}

int ProtocolPsPcoDataAdapter::GetPdpType() const
{
    int ret = PDP_TYPE_IPV4;
    if (m_pModemData != NULL) {
        sit_pdp_pco_data_ind *data = (sit_pdp_pco_data_ind *)m_pModemData->GetRawData();
        if (data != NULL ) {
            ret = data->pdp_type;
        }
    }
    return ret;
}

int ProtocolPsPcoDataAdapter::GetPcoData(sit_pdp_pco_data_entry &pco_data)
{
    if (remainPcoBlocks == 0) return -1;

    if (m_pModemData != NULL) {
        //m_pModemData->Dump();
        sit_pdp_pco_data_ind *data = (sit_pdp_pco_data_ind *)m_pModemData->GetRawData();
        char *pco_block = (char*)m_pModemData->GetRawData() + sizeof(sit_pdp_pco_data_ind) + nextPcoBlockPos;
        //int total_length = data->hdr.length - 4; // HEADER.type(1)+reserved(1)+id(2)=4
        //RilLogV("total_length:%d pco_block = %p, nextPcoBlockPos:%d, remainPcoBlocks:%d",
        //        total_length, pco_block, nextPcoBlockPos, remainPcoBlocks);
        pco_data.pco_id = ((INT32*)(pco_block))[0];
        pco_data.contents_len = pco_block[4];
        unsigned int current_blocksize = sizeof(INT32) + sizeof(BYTE) + pco_data.contents_len;
        pco_data.contents =(char *)(data) +
            sizeof(sit_pdp_pco_data_ind) + nextPcoBlockPos +
            sizeof(INT32) /* pco_id */ + sizeof(BYTE); /* contents_len */
        //RilLogV("pco_id=%x(%d), contents_len = %d, current_blocksize=%d, contents:%p",
        //        pco_data.pco_id, pco_data.pco_id, pco_data.contents_len, current_blocksize, pco_data.contents);
        nextPcoBlockPos += current_blocksize;
    }

    return --remainPcoBlocks;
}

int ProtocolPsPcoDataAdapter::GetPcoNum() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_pdp_pco_data_ind *data = (sit_pdp_pco_data_ind *)m_pModemData->GetRawData();
        if (data != NULL ) {
            ret = data->pco_num;
        }
    }
    return ret;
}
