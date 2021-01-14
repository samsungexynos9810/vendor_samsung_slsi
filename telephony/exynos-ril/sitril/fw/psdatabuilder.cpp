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
 * psdatabuilder.cpp
 *
 *  Created on: 2014. 6. 30.
 *      Author: sungwoo48.choi
 */


#include <arpa/inet.h>
#include "psdatabuilder.h"
#include "rillog.h"
#include "rilproperty.h"

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define    PREFIX_IFNAME    "rmnet"

#if(RIL_VERSION < 11)
#error RIL_VERSION Definition is too old, something wrong
#endif
typedef RIL_Data_Call_Response_v11  RIL_Data_Call_Response;

class DataCallListResponse : public RilData {
public:
    RIL_Data_Call_Response m_dataCall[MAX_DATA_CALL_SIZE];
    int m_dataCallNum;
public:
    DataCallListResponse(int dataCallNum) : RilData() {
        memset(&m_dataCall, 0, sizeof(m_dataCall));
        if (dataCallNum > MAX_DATA_CALL_SIZE)
            dataCallNum = MAX_DATA_CALL_SIZE;
        m_dataCallNum = dataCallNum;
    }
    virtual ~DataCallListResponse() {
        for (int i = 0; i < m_dataCallNum; i++) {
            if (m_dataCall[i].ifname)
                delete[] m_dataCall[i].ifname;
            if (m_dataCall[i].addresses)
                delete[] m_dataCall[i].addresses;
            if (m_dataCall[i].dnses)
                delete[] m_dataCall[i].dnses;
            if (m_dataCall[i].gateways)
                delete[] m_dataCall[i].gateways;
            if (m_dataCall[i].pcscf)
                delete[] m_dataCall[i].pcscf;
        } // end for i ~
    }
public:
    virtual void *GetData() const {
        return (void *)m_dataCall;
    }
    virtual unsigned int GetDataLength() const {
        return m_dataCallNum * sizeof(RIL_Data_Call_Response);
    }
private:
    void FillPdpType(RIL_Data_Call_Response *resp, const DataCall *dataCall)
    {
        switch (dataCall->pdpType) {
        case PDP_TYPE_IPV4:
            resp->type = (char *)DATA_PROTOCOL_IP;
            break;
        case PDP_TYPE_IPV6:
            resp->type = (char *)DATA_PROTOCOL_IPV6;
            break;
        case PDP_TYPE_IPV4V6:
            resp->type = (char *)DATA_PROTOCOL_IPV4V6;
            break;
        case PDP_TYPE_PPP:
            resp->type = (char *)DATA_PROTOCOL_PPP;
            break;
        default:
            resp->type = (char *)DATA_PROTOCOL_IP;
            break;
        } // end switch ~
    }
    void FillInterfaceName(RIL_Data_Call_Response *resp, const DataCall *dataCall, const char *ifname = NULL)
    {
        // set interface name
        // PREFIX_IFNAME + (cid-1)
        // e.g., CID = 1, interface name is "rmnet0"
        if (resp->ifname)
            delete[] resp->ifname;
        resp->ifname = new char[MAX_IFNAME_LEN];
        memset(resp->ifname, 0, MAX_IFNAME_LEN);
        if (TextUtils::IsEmpty(ifname)) {
            snprintf(resp->ifname, MAX_IFNAME_LEN, "%s%d", PREFIX_IFNAME, dataCall->cid - 1);
        }
        else {
            strncpy(resp->ifname, ifname, MAX_IFNAME_LEN);
        }
    }
    void FillIpv4Addresses(RIL_Data_Call_Response *resp, const DataCall *dataCall,
                           char (&addresses)[MAX_ADDRESS_STRING_LEN],
                           char (&dnses)[MAX_ADDRESS_STRING_LEN],
                           char (&gateways)[MAX_ADDRESS_STRING_LEN],
                           char (&pcscf)[MAX_ADDRESS_STRING_LEN * 2])
    {
        int len = 0;
        char buf[MAX_ADDRESS_STRING_LEN] = {0, };

        //1. IP version 4 address setting
        if (dataCall->ipv4.valid) {
            char nullIpv4[4] = {0, };

            // IPv4 address
            len = strlen(addresses);
            if (len > 0) {
                addresses[len++] = ' ';
            }
            if (inet_ntop(AF_INET, dataCall->ipv4.addr, buf, (socklen_t)sizeof(buf)) != NULL) {
                snprintf(addresses + len, sizeof(addresses) - len - 1, "%s", buf);
            }

            // IPv4 DNS1
            if (memcmp(dataCall->ipv4.dns1, nullIpv4, 4) != 0) {
                len = strlen(dnses);
                if (len > 0) {
                    dnses[len++] = ' ';
                }
                if (inet_ntop(AF_INET, dataCall->ipv4.dns1, buf, (socklen_t)sizeof(buf)) != NULL) {
                    strncpy(dnses + len, buf, sizeof(dnses) - len - 1);
                }
            }

            // IPv4 DNS2
            if (memcmp(dataCall->ipv4.dns2, nullIpv4, 4) != 0) {
                len = strlen(dnses);
                if (len > 0) {
                    dnses[len++] = ' ';
                }
                if (inet_ntop(AF_INET, dataCall->ipv4.dns2, buf, (socklen_t)sizeof(buf)) != NULL) {
                    strncpy(dnses + len, buf, sizeof(dnses) - len - 1);
                }
            }

            // IPv4 Gateway
            if (memcmp(dataCall->ipv4.addr, nullIpv4, 4) != 0) {
                // No manipulation. Just fill 0.0.0.0 address. This is allowed
                // 32bit length(self) will work as gateway because it's ppp
                // just this is required to set default routing rule into each interface table
                // This is applicable from AOSP N and required for AOSP P version
                // which will not use main table
                len = strlen(gateways);
                if (len > 0) {
                    gateways[len++] = ' ';
                }
                strncpy(gateways + len, "0.0.0.0", sizeof(gateways) - len - 1);
            }

            // IPv4 P-CSCF
            int pcscf_count = MAX_PCSCF_NUM + dataCall->pcscf_ext_count;
            for(int i=0; i<pcscf_count; i++) {
                if (memcmp(&dataCall->ipv4.pcscf[MAX_IPV4_ADDR_LEN * i], nullIpv4, MAX_IPV4_ADDR_LEN) != 0) {
                    len = strlen(pcscf);
                    if (len > 0) {
                        pcscf[len++] = ' ';
                    }
                    if (inet_ntop(AF_INET, &dataCall->ipv4.pcscf[MAX_IPV4_ADDR_LEN * i], buf, (socklen_t)sizeof(buf)) != NULL) {
                        strncpy(pcscf + len, buf, sizeof(pcscf) - len - 1);
                    }
                }
            }
        }
    }

    void AddIpv6Dns(const BYTE (&dns)[MAX_IPV6_ADDR_LEN], char (&dnses)[MAX_ADDRESS_STRING_LEN], int slot)
    {
        int len = 0;
        char buf[MAX_ADDRESS_STRING_LEN] = {0, };
        const char nullIpv6[16] = {0, };
        const char publicDNS[2][MAX_ADDRESS_STRING_LEN] = { "2001:4860:4860::8888", "2001:4860:4860::8844" };

        // IPv6 DNS1
        if (memcmp(dns, nullIpv6, 16) != 0) {
            len = strlen(dnses);
            if (len > 0) {
                dnses[len++] = ' ';
            }
            if (inet_ntop(AF_INET6, dns, buf, (socklen_t)sizeof(buf)) != NULL) {
                strncpy(dnses + len, buf, sizeof(dnses) - len - 1);
            }
        } else {
            char buf[100] = { 0 };
            property_get(RIL_PS_DEFAULT_IPV6_DNS, buf, "0");
            if (buf[0] == '1' && slot == 0) {
                len = strlen(dnses);
                if (len > 0) {
                    dnses[len++] = ' ';
                }
                // Fill Google Public DNS1
                strncpy(dnses + len, publicDNS[slot], sizeof(dnses) - len - 1);
            }
        }
    }

    void FillIpv6Addresses(RIL_Data_Call_Response *resp, const DataCall *dataCall,
                           char (&addresses)[MAX_ADDRESS_STRING_LEN],
                           char (&dnses)[MAX_ADDRESS_STRING_LEN],
                           char (&gateways)[MAX_ADDRESS_STRING_LEN],
                           char (&pcscf)[MAX_ADDRESS_STRING_LEN * 2])
    {
        int len = 0;
        char buf[MAX_ADDRESS_STRING_LEN] = {0, };

        //2. IP version 6 address setting
        if (dataCall->ipv6.valid) {
            char nullIpv6[16] = {0, };
            // IPv6 address
            len = strlen(addresses);
            if (len > 0) {
                addresses[len++] = ' ';
            }
            if (inet_ntop(AF_INET6, dataCall->ipv6.addr, buf, (socklen_t)sizeof(buf)) != NULL) {
                snprintf(addresses + len, sizeof(addresses) - len - 1, "%s/64", buf);
            }

//#define TEST_NULL_DNS_SERVER
#if defined(TEST_NULL_DNS_SERVER)
            const BYTE nullIpTest[MAX_IPV6_ADDR_LEN] = {0,};
            AddIpv6Dns(nullIpTest, dnses, 0);
            AddIpv6Dns(nullIpTest, dnses, 1);
#else
            AddIpv6Dns(dataCall->ipv6.dns1, dnses, 0);
            AddIpv6Dns(dataCall->ipv6.dns2, dnses, 1);
#endif
            SetIpv6Gateway(dataCall, gateways);

            // IPv6 P-CSCF
            int pcscf_count = MAX_PCSCF_NUM + dataCall->pcscf_ext_count;
            for(int i = 0; i < pcscf_count; i++) {
                if (memcmp(&dataCall->ipv6.pcscf[MAX_IPV6_ADDR_LEN * i], nullIpv6, 16) != 0) {
                    len = strlen(pcscf);
                    if (len > 0) {
                        pcscf[len++] = ' ';
                    }
                    if (inet_ntop(AF_INET6, &dataCall->ipv6.pcscf[MAX_IPV6_ADDR_LEN * i], buf, (socklen_t)sizeof(buf)) != NULL) {
                        strncpy(pcscf + len, buf, sizeof(pcscf) - len - 1);
                    }
                }
            }
        }
    }
    void CopyToResponse(RIL_Data_Call_Response *resp, const DataCall *dataCall,
                        char (&addresses)[MAX_ADDRESS_STRING_LEN],
                        char (&dnses)[MAX_ADDRESS_STRING_LEN],
                        char (&gateways)[MAX_ADDRESS_STRING_LEN],
                        char (&pcscf)[MAX_ADDRESS_STRING_LEN * 2])
    {
        // copy address data
        if (resp->addresses)
            delete[] resp->addresses;
        resp->addresses = new char[MAX_ADDRESS_STRING_LEN];
        memset(resp->addresses, 0, MAX_ADDRESS_STRING_LEN);
        if (!TextUtils::IsEmpty(addresses))
            strncpy(resp->addresses, addresses, MAX_ADDRESS_STRING_LEN-1);

        if (resp->dnses)
            delete[] resp->dnses;
        resp->dnses = new char[MAX_ADDRESS_STRING_LEN];
        memset(resp->dnses, 0, MAX_ADDRESS_STRING_LEN);
        if (!TextUtils::IsEmpty(dnses))
            strncpy(resp->dnses, dnses, MAX_ADDRESS_STRING_LEN-1);

        if (resp->gateways)
            delete[] resp->gateways;
        resp->gateways = new char[MAX_ADDRESS_STRING_LEN];
        memset(resp->gateways, 0, MAX_ADDRESS_STRING_LEN);
        if (!TextUtils::IsEmpty(gateways))
            strncpy(resp->gateways, gateways, MAX_ADDRESS_STRING_LEN-1);

        if (resp->pcscf)
            delete[] resp->pcscf;
        resp->pcscf = new char[MAX_ADDRESS_STRING_LEN * 2];
        memset(resp->pcscf, 0, MAX_ADDRESS_STRING_LEN * 2);
        if (!TextUtils::IsEmpty(pcscf))
            strncpy(resp->pcscf, pcscf, MAX_ADDRESS_STRING_LEN * 2 - 1);

        // MTU value from Network
        resp->mtu = dataCall->mtu_size;
        // PCO value
        // TODO: Need to deliver pcoData IND from O version, still need this?
        // RIL_UNSOL_PCO_DATA from CP is required with more data
        //resp->pco = dataCall->pco;
    }

    void TranslateToResponse(RIL_Data_Call_Response *resp, const DataCall *dataCall, const char *ifname)
    {
        FillPdpType(resp, dataCall);
        FillInterfaceName(resp, dataCall, ifname);

        // set IP addresses
        char addresses[MAX_ADDRESS_STRING_LEN] = {0, };
        char dnses[MAX_ADDRESS_STRING_LEN] = {0, };
        char gateways[MAX_ADDRESS_STRING_LEN] = {0, };
        char pcscf[MAX_ADDRESS_STRING_LEN * 2] = {0, };
#ifdef RIL_IPV4_DNS_QUERY_FIRST
        FillIpv4Addresses(resp, dataCall, addresses, dnses, gateways, pcscf);
        FillIpv6Addresses(resp, dataCall, addresses, dnses, gateways, pcscf);
#else
        FillIpv6Addresses(resp, dataCall, addresses, dnses, gateways, pcscf);
        FillIpv4Addresses(resp, dataCall, addresses, dnses, gateways, pcscf);
#endif

        CopyToResponse(resp, dataCall, addresses, dnses, gateways, pcscf);
    }
    bool SetDataCall(int index, int errorCode, const char *ifname, const DataCall *dataCall);
    void Init(RIL_Data_Call_Response *dataCallResp);
    int GetDataCallFailCause(int errorCode, int status);
    int GetSuggestedRetryTime(const DataCall *dataCall, int status);
    void SetIpv6Gateway(const DataCall *dataCall, char (&gateways)[MAX_ADDRESS_STRING_LEN])
    {
        int len = 0;
        if (dataCall->ipv6.valid) {
            const char nullIpv6[16] = {0, };
            // No manipulation. Just fill '::' address. This is allowed and work as gateway because it's ppp
            // just this is required to set default routing rule into each interface table
            // This should be replaced Router Address after ND, Use Kernel Configuration with Link-Local
            // Router addrees '/proc/sys/net/ipv6/conf/all/accept_ra_defrtr'
            // This is required when SetupDataCall gets global ipv6 address directly without RA.
            // There's no Link-Local Setting, RS from Linux kernel won't be sent.
            // ConnectivityService will try to set default route with these next-hop address
            // This make occasionally netd warning (file exist) when kernel already setup default route with RA
            // but EEXIST error is ignored and doesn't cause critical failure
            // in ModifyIpRoute() of netd RouteController.cpp
            if (memcmp(dataCall->ipv6.addr, nullIpv6, 16) != 0) {
                len = strlen(gateways);
                if (len > 0) {
                    gateways[len++] = ' ';
                }
                strncpy(gateways + len, "::", sizeof(gateways) - len - 1);
            }
        }
    }

    bool SetDataCall(int index, const DataCall *dataCall, const char *ifname = NULL) {
        if (index < 0 || index >= m_dataCallNum) {
            return false;
        }

        if (dataCall == NULL) {
            return false;
        }

        RIL_Data_Call_Response *resp = &m_dataCall[index];

        resp->status = dataCall->status;
        resp->suggestedRetryTime = dataCall->suggestedRetryTime;
        resp->cid = dataCall->cid;
        resp->active = dataCall->active;

        TranslateToResponse(resp, dataCall, ifname);

        return true;
    }
public:
    bool SetDataCall(int index, PdpContext *pPdpContext);
    bool SetDataCall(int index, int errorCode, PdpContext *pPdpContext);
    bool SetDataCall(int index, int errorCode);
};

void DataCallListResponse::Init(RIL_Data_Call_Response *dataCallResp)
{
    if (dataCallResp != NULL) {
        if (dataCallResp->ifname) delete[] dataCallResp->ifname;
        if (dataCallResp->addresses) delete[] dataCallResp->addresses;
        if (dataCallResp->dnses) delete[] dataCallResp->dnses;
        if (dataCallResp->gateways) delete[] dataCallResp->gateways;
        if (dataCallResp->pcscf) delete[] dataCallResp->pcscf;

        memset(dataCallResp, 0, sizeof(RIL_Data_Call_Response));
        dataCallResp->status = PDP_FAIL_ERROR_UNSPECIFIED;
        dataCallResp->suggestedRetryTime = RETRY_NO_SUGGESTED;
        dataCallResp->cid = 0;
        dataCallResp->active = INACTIVE;
        dataCallResp->type = (char *)DATA_PROTOCOL_IP;
    }
}

int DataCallListResponse::GetDataCallFailCause(int errorCode, int status)
{
    // PDP_FAIL_* that's not specified in TS 24.008
    if (errorCode < 0)
        return errorCode;
    if (errorCode != RIL_E_SUCCESS) {
        switch (errorCode) {
        case RIL_E_OP_NOT_ALLOWED_BEFORE_REG_TO_NW:
            return PDP_FAIL_ERROR_UNSPECIFIED;
        default:
            return PDP_FAIL_ERROR_UNSPECIFIED;
        } // end switch ~
    }
    return status;
}

int DataCallListResponse::GetSuggestedRetryTime(const DataCall *dataCall, int status)
{
    /*
    int time = RETRY_NO_SUGGESTED;
    //fw will check Fail Status
    if(status != PDP_FAIL_NONE) {
        time = dataCall->suggestedRetryTime;
    }
    return time;
    */
    return dataCall->suggestedRetryTime;
}

/**
 * SetDataCall
 *
 * a response in case of error. Without PDP Context, cannot make a response of successes case.
 */
bool DataCallListResponse::SetDataCall(int index, int errorCode)
{
    if (index < 0 || index >= m_dataCallNum) {
         return false;
    }

    RIL_Data_Call_Response *resp = &m_dataCall[index];
    Init(resp);

    resp->status = GetDataCallFailCause(errorCode, PDP_FAIL_ERROR_UNSPECIFIED);
    return true;
}

bool DataCallListResponse::SetDataCall(int index, PdpContext *pPdpContext) {
    return SetDataCall(index, pPdpContext->GetDataCallInfo(), pPdpContext->GetInterfaceName());
}

bool DataCallListResponse::SetDataCall(int index, int errorCode, const char *ifname, const DataCall *dataCall)
{
    if (index < 0 || index >= m_dataCallNum) {
         return false;
    }

    if (dataCall == NULL) {
        return false;
    }

    RIL_Data_Call_Response *resp = &m_dataCall[index];
    Init(resp);

    resp->status = GetDataCallFailCause(errorCode, dataCall->status);
    resp->suggestedRetryTime = GetSuggestedRetryTime(dataCall, dataCall->status);
    resp->cid = dataCall->cid;
    resp->active = dataCall->active;

    if (resp->status == PDP_FAIL_NONE) {
        if (resp->active == ACTIVE_AND_LINKUP) {
            // Both setup data call and bring up interface are successful.
            TranslateToResponse(resp, dataCall, ifname);
        }
        else if (resp->active == ACTIVE_AND_LINKDOWN) {
            // Successes to setup data call but failed to bring up interface.
            resp->status = PDP_FAIL_ERROR_UNSPECIFIED;
            resp->suggestedRetryTime = RETRY_ASAP;
        }
        else {
            // unexpected case. set a default error status.
            resp->status = PDP_FAIL_ERROR_UNSPECIFIED;
        }
    }

    return true;
}

/**
 * SetDataCall
 *
 * if PdpContext is NULL, it should be error.
 *
 * @param index a index of data call list
 * @param errorCode a result of SETUP_DATA_CALL
 * @param pPdpContext an instance of PdpContext. It has all information that a result of SETUP_DATA_CALL.
 * @return returns true when set information of the result successfully, otherwise returns false.
 */
bool DataCallListResponse::SetDataCall(int index, int errorCode, PdpContext *pPdpContext)
{
    if (pPdpContext == NULL) {
        RilLogV("pdpContext is null");
        return SetDataCall(index, errorCode);
    }
    return SetDataCall(index, errorCode, pPdpContext->GetInterfaceName(), pPdpContext->GetDataCallInfo());
}

/**
 * PsDataBuilder
 */
// Do not use, usage is duplicated to the version with PdpContext
/*
const RilData *PsDataBuilder::BuildSetupDataCallResponse(const DataCall *dataCall)
{
    if (dataCall == NULL)
        return NULL;

    DataCallListResponse *rildata = new DataCallListResponse(1);
    if (rildata != NULL) {
        rildata->SetDataCall(0, dataCall);
    }
    return rildata;
}
*/

const RilData *PsDataBuilder::BuildSetupDataCallResponse(PdpContext *pPdpContext)
{
    if (pPdpContext == NULL)
        return NULL;

    DataCallListResponse *rildata = new DataCallListResponse(1);
    if (rildata != NULL) {
        rildata->SetDataCall(0, pPdpContext);
    }
    return rildata;
}

const RilData *PsDataBuilder::BuildSetupDataCallResponse(int errorCode, PdpContext *pPdpContext)
{
    // pPdpContext==NULL will generate Default DataCall, SetDataCall(int, int, PdpContext *) can process this.
    // if (pPdpContext == NULL)
    //     return NULL;

    DataCallListResponse *rildata = new DataCallListResponse(1);
    if (rildata != NULL) {
        rildata->SetDataCall(0, errorCode, pPdpContext);
    }
    return rildata;
}

const RilData *PsDataBuilder::BuildSetupDataCallResponse(int errorCode, int status)
{
    DataCallListResponse *rildata = new DataCallListResponse(1);
    if (rildata != NULL) {
        rildata->SetDataCall(0, errorCode);
    }
    // Override status
    rildata->m_dataCall[0].status = status;
    return rildata;
}

const RilData *PsDataBuilder::BuildPcoData(int cid, int nPdpType, int pcoId, int contentsLen, char *pContents)
{
    RIL_PCO_Data rsp;

    rsp.cid = cid;
    rsp.bearer_proto = ConvertPdpType(nPdpType);
    rsp.pco_id = pcoId;
    rsp.contents_length = contentsLen;
    rsp.contents = pContents;

    return new RilDataRaw(&rsp, sizeof(rsp));
}

char *PsDataBuilder::ConvertPdpType(int nPdpType)
{
    switch (nPdpType) {
      case PDP_TYPE_IPV4:
          return (char *)DATA_PROTOCOL_IP;
      case PDP_TYPE_IPV6:
          return (char *)DATA_PROTOCOL_IPV6;
      case PDP_TYPE_IPV4V6:
          return (char *)DATA_PROTOCOL_IPV4V6;
      case PDP_TYPE_PPP:
          return (char *)DATA_PROTOCOL_PPP;
      default:
          return (char *)DATA_PROTOCOL_IP;
    } // end switch ~
}

///////////////////////////////////////////////////////////
// PsDataCallListBuilder
///////////////////////////////////////////////////////////
int PsDataCallListBuilder::AddDataCall(PdpContext *pPdpContext)
{
    if (pPdpContext == NULL) {
        return -1;
    }
    m_PdpContextList.push_back(pPdpContext);
    return 0;
}

const RilData *PsDataCallListBuilder::Build()
{
    int size = m_PdpContextList.size();
    DataCallListResponse *rildata = new DataCallListResponse(size);
    if (rildata != NULL) {
        for (int i = 0; i < size; i++) {
            PdpContext *pPdpContext = m_PdpContextList[i];
            rildata->SetDataCall(i, pPdpContext);
        } // end for i ~
    }
    return rildata;
}

void PsDataCallListBuilder::Clear()
{
    m_PdpContextList.clear();
}

///////////////////////////////////////////////////////////
// PsDataNasTimerStatusBuilder
///////////////////////////////////////////////////////////
class NasTimerStatus : public RilData {
public:
    RIL_NasTimerStatus mResp;
    char mApn[MAX_PDP_APN_LEN];

    NasTimerStatus() {
        memset(&mResp, 0, sizeof(mResp));
        memset(mApn, 0, sizeof(mApn));
        mResp.apn = mApn;
    }
    virtual ~NasTimerStatus() {}

    void *GetData() const { return (void *)&mResp; }
    unsigned int GetDataLength() const { return sizeof(mResp); }
};

const RilData *PsDataNasTimerStatusBuilder::BuildNasTimerStatus(const SitNasTimerStatus *pNasTimerStatus)
{
    if (pNasTimerStatus == NULL)
        return NULL;

    NasTimerStatus *rildata = new NasTimerStatus();
    if (rildata != NULL) {
        rildata->mResp.type = pNasTimerStatus->type;
        rildata->mResp.status = pNasTimerStatus->status;
        rildata->mResp.value= pNasTimerStatus->value;
        strncpy(rildata->mResp.apn, pNasTimerStatus->apn, MAX_PDP_APN_LEN - 1);
    }
    return rildata;
}
