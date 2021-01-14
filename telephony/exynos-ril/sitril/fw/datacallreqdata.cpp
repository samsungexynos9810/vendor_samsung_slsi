/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "datacallreqdata.h"
#include "networkutils.h"
#include "rilapplication.h"
#include "rillog.h"

#include <string>

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_DATA, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

/**
 * DataProfileInfoUtil
 */
class DataProfileInfoUtils {
public:
    static bool CopyDataProfileInfo(RIL_DataProfileInfo_V1_4 &dest, const RIL_DataProfileInfo_V1_4 &src);
    static bool CopyDataProfileInfo(RIL_DataProfileInfo_V1_4 &dest, const RIL_DataProfileInfo_v15 &src);
    static bool CopyDataProfileInfo(RIL_DataProfileInfo_v15 &dest, const RIL_DataProfileInfo_v15 &src);
    static bool CopyDataProfileInfo(RIL_DataProfileInfo_v15 &dest, const RIL_DataProfileInfo &src);
    static void FreeDataProfileInfo(RIL_DataProfileInfo_V1_4 &dpi);
};

bool DataProfileInfoUtils::CopyDataProfileInfo(RIL_DataProfileInfo_V1_4 &dest, const RIL_DataProfileInfo_V1_4 &src)
{
    if (!TextUtils::DupString(&dest.apn, src.apn) ||
        !TextUtils::DupString(&dest.user, src.user) ||
        !TextUtils::DupString(&dest.password, src.password)) {
        return false;
    }

    dest.profileId = src.profileId;
    dest.protocol = src.protocol;
    dest.roamingProtocol = src.roamingProtocol;
    dest.authType = src.authType;
    dest.type = src.type;
    dest.maxConnsTime = src.maxConnsTime;
    dest.maxConns = src.maxConns;
    dest.waitTime = src.waitTime;
    dest.enabled = src.enabled;
    dest.supportedApnTypesBitmap = src.supportedApnTypesBitmap;
    dest.bearerBitmap = src.bearerBitmap;
    dest.preferred = src.preferred;
    dest.persistent = src.persistent;

    return true;
}

bool DataProfileInfoUtils::CopyDataProfileInfo(RIL_DataProfileInfo_V1_4 &dest, const RIL_DataProfileInfo_v15 &src)
{
    if (!TextUtils::DupString(&dest.apn, src.apn) ||
        !TextUtils::DupString(&dest.user, src.user) ||
        !TextUtils::DupString(&dest.password, src.password)) {
        return false;
    }

    dest.protocol = PDP_PROTOCOL_TYPE_IP;
    if (src.protocol != NULL) {
        if (TextUtils::Equals(src.protocol, "IP")) {
            dest.protocol = PDP_PROTOCOL_TYPE_IP;
        }
        else if (TextUtils::Equals(src.protocol, "IPV6")) {
            dest.protocol = PDP_PROTOCOL_TYPE_IPV6;
        }
        else if (TextUtils::Equals(src.protocol, "IPV4V6")) {
            dest.protocol = PDP_PROTOCOL_TYPE_IPV4V6;
        }
    }

    dest.roamingProtocol = PDP_PROTOCOL_TYPE_IP;
    if (src.roamingProtocol != NULL) {
        if (TextUtils::Equals(src.roamingProtocol, "IP")) {
            dest.roamingProtocol = PDP_PROTOCOL_TYPE_IP;
        }
        else if (TextUtils::Equals(src.roamingProtocol, "IPV6")) {
            dest.roamingProtocol = PDP_PROTOCOL_TYPE_IPV6;
        }
        else if (TextUtils::Equals(src.roamingProtocol, "IPV4V6")) {
            dest.roamingProtocol = PDP_PROTOCOL_TYPE_IPV4V6;
        }
    }

    dest.profileId = src.profileId;
    dest.authType = src.authType;
    dest.type = src.type;
    dest.maxConnsTime = src.maxConnsTime;
    dest.maxConns = src.maxConns;
    dest.waitTime = src.waitTime;
    dest.enabled = src.enabled;
    dest.supportedApnTypesBitmap = src.supportedTypesBitmask;
    dest.bearerBitmap = src.bearerBitmask;
    dest.preferred = false;
    dest.persistent = true;

    return true;
}

bool DataProfileInfoUtils::CopyDataProfileInfo(RIL_DataProfileInfo_v15 &dest, const RIL_DataProfileInfo_v15 &src)
{
    if (!TextUtils::DupString(&dest.apn, src.apn) ||
        !TextUtils::DupString(&dest.protocol, src.protocol) ||
        !TextUtils::DupString(&dest.roamingProtocol, src.roamingProtocol) ||
        !TextUtils::DupString(&dest.mvnoType, src.mvnoType) ||
        !TextUtils::DupString(&dest.mvnoMatchData, src.mvnoMatchData) ||
        !TextUtils::DupString(&dest.user, src.user) ||
        !TextUtils::DupString(&dest.password, src.password)) {
        return false;
    }

    dest.profileId = src.profileId;
    dest.authType = src.authType;
    dest.type = src.type;
    dest.supportedTypesBitmask = src.supportedTypesBitmask;
    dest.bearerBitmask = src.bearerBitmask;
    dest.maxConnsTime = src.maxConnsTime;
    dest.maxConns = src.maxConns;
    dest.waitTime = src.waitTime;
    dest.enabled = src.enabled;
    dest.mtu = src.mtu;
    return true;
}

bool DataProfileInfoUtils::CopyDataProfileInfo(RIL_DataProfileInfo_v15 &dest, const RIL_DataProfileInfo &src)
{
    if (!TextUtils::DupString(&dest.apn, src.apn) ||
        !TextUtils::DupString(&dest.protocol, src.protocol) ||
        !TextUtils::DupString(&dest.roamingProtocol, src.protocol) ||
        !TextUtils::DupString(&dest.user, src.user) ||
        !TextUtils::DupString(&dest.password, src.password)) {
        return false;
    }

    dest.profileId = src.profileId;
    dest.roamingProtocol = NULL;
    dest.authType = src.authType;
    dest.type = src.type;
    dest.supportedTypesBitmask = 0;
    dest.bearerBitmask = 0;
    dest.maxConnsTime = src.maxConnsTime;
    dest.maxConns = src.maxConns;
    dest.waitTime = src.waitTime;
    dest.enabled = src.enabled;
    dest.mtu = 0;
    dest.mvnoType = NULL;
    dest.mvnoMatchData = NULL;
    return true;
}

void DataProfileInfoUtils::FreeDataProfileInfo(RIL_DataProfileInfo_V1_4 &dpi)
{
    if (dpi.apn != NULL) {
        delete[] dpi.apn;
        dpi.apn = NULL;
    }

    if (dpi.user != NULL) {
        delete[] dpi.user;
        dpi.user = NULL;
    }

    if (dpi.password != NULL) {
        delete[] dpi.password;
        dpi.password = NULL;
    }
}

/**
 * SetupDataCallRequestData
 */
SetupDataCallRequestData::SetupDataCallRequestData(const int nReq, const Token tok, const ReqType type)
    : StringsRequestData(nReq, tok, type)
{
    memset(&mDataProfileInfo, 0, sizeof(mDataProfileInfo));
}

SetupDataCallRequestData::~SetupDataCallRequestData()
{
    DataProfileInfoUtils::FreeDataProfileInfo(mDataProfileInfo);
    memset(&mDataProfileInfo, 0, sizeof(mDataProfileInfo));
}

int SetupDataCallRequestData::encode(char *data, unsigned int datalen)
{
    RilLog("SetupDataCallRequestData::encode m_nReq=%d mHalVer=0x%x", m_nReq, mHalVer);

    int halVer = GetHalVersion();
    if (halVer >= HAL_VERSION_CODE(1, 4)
            && datalen == sizeof(RIL_SetupDataCallInfo_V1_4)) {
        return encode((RIL_SetupDataCallInfo_V1_4 *)data);
    }

    if (StringsRequestData::encode(data, datalen) < 0)
        return -1;

    return (m_ptrStrings == NULL || (m_nStringCount < 5) ? -1 : 0);
}

int SetupDataCallRequestData::encode(RIL_SetupDataCallInfo_V1_4 *dataCallInfo)
{
    if (dataCallInfo == NULL) {
        return -1;
    }

    if (!DataProfileInfoUtils::CopyDataProfileInfo(mDataProfileInfo, dataCallInfo->dataProfileInfo)) {
        RilLogW("[SetupDataCallRequestData] Failed to CopyDataProfileInfo");
        return -1;
    }

    //RIL_DataProfileInfo_V1_4 &dataProfileInfo = dataCallInfo->dataProfileInfo;
    const char *params[] = {
        std::to_string((int) NetworkUtils::getRadioTechnologyFromAccessNetwork(dataCallInfo->accessNetwork) + 2).c_str(),
        dataCallInfo->roamingAllow ? "1" : "0",
        std::to_string(dataCallInfo->reason).c_str(),
        dataCallInfo->addresses,
        dataCallInfo->dnses,
    };

    return StringsRequestData::encode((char *)params, sizeof(params));
}

SetupDataCallRequestData *SetupDataCallRequestData::Clone() const
{
    SetupDataCallRequestData *p = new SetupDataCallRequestData(m_nReq, m_tok, m_reqType);
    if (p != NULL) {
        DataProfileInfoUtils::CopyDataProfileInfo(p->mDataProfileInfo, mDataProfileInfo);
        p->encode((char *)m_ptrStrings, m_nStringCount * sizeof(char *));
    }
    return p;
}

int SetupDataCallRequestData::GetRadioTech() const
{
    const char *strRat = GetString(0);
    if (strRat != NULL) {
        int rat = atoi(strRat);
        // Radio Tech
        if (rat == 0) {
            rat = RADIO_TECH_IS95A;
        }
        else if (rat == 1) {
            rat = RADIO_TECH_UMTS;
        }
        else {
            rat = rat - 2;
        }
        return rat;
    }
    return RADIO_TECH_UMTS;
}

int SetupDataCallRequestData::GetDataProfileId() const
{
    return mDataProfileInfo.profileId;
}

const char *SetupDataCallRequestData::GetApn() const
{
    return mDataProfileInfo.apn;
}

const char *SetupDataCallRequestData::GetUsername() const
{
    return mDataProfileInfo.user;
}

const char *SetupDataCallRequestData::GetPassword() const
{
    return mDataProfileInfo.password;
}

int SetupDataCallRequestData::GetAuthType() const
{
    return mDataProfileInfo.authType;
}

const char *SetupDataCallRequestData::GetProtocolByInt(int protocol) const
{
    switch (protocol) {
    case PDP_PROTOCOL_TYPE_IP:
        return "IP";
    case PDP_PROTOCOL_TYPE_IPV6:
        return "IPV6";
    case PDP_PROTOCOL_TYPE_IPV4V6:
        return "IPV4V6";
    case PDP_PROTOCOL_TYPE_NON_IP:
        return "NONIP";
    default:
        break;
    }
    return "IP";
}

const char *SetupDataCallRequestData::GetProtocol() const
{
    return GetProtocolByInt(mDataProfileInfo.protocol);
}

const char *SetupDataCallRequestData::GetRoamingProtocol() const
{
    return GetProtocolByInt(mDataProfileInfo.roamingProtocol);
}

int SetupDataCallRequestData::GetSupportedApnTypesBitmap() const
{
    return mDataProfileInfo.supportedApnTypesBitmap;
}

int SetupDataCallRequestData::GetRadioAccessFamily() const
{
    return mDataProfileInfo.bearerBitmap;
}

bool SetupDataCallRequestData::GetAPNSettingStatus() const
{
    return mDataProfileInfo.enabled;
}

int SetupDataCallRequestData::GetMtuSize() const
{
    return mDataProfileInfo.mtu;
}

const char *SetupDataCallRequestData::IsDataRoamingAllowed() const
{
    return GetString(1);
}

bool SetupDataCallRequestData::IsRoamingAllowed() const
{
    const char *isRoamingAllowed = GetString(1);
    if (isRoamingAllowed != NULL) {
        return (atoi(isRoamingAllowed) == 1) ? true : false;
    }
    return false;
}

const char *SetupDataCallRequestData::GetReason() const
{
    return GetString(2);
}

const char *SetupDataCallRequestData::GetAddresses() const
{
    return GetString(3);
}

const char *SetupDataCallRequestData::GetDnses() const
{
    return GetString(4);
}

/**
 * DeactivateDataCallRequestData
 */
DeactivateDataCallRequestData::DeactivateDataCallRequestData(const int nReq, const Token tok, const ReqType type)
    : StringsRequestData(nReq, tok, type)
{
}

DeactivateDataCallRequestData::~DeactivateDataCallRequestData()
{
}

INT32 DeactivateDataCallRequestData::encode(char *data, unsigned int datalen)
{
    if (StringsRequestData::encode(data, datalen) < 0)
        return -1;

    return (m_ptrStrings == NULL || (m_nStringCount != 2) ? -1 : 0);
}

DeactivateDataCallRequestData *DeactivateDataCallRequestData::Clone() const
{
    DeactivateDataCallRequestData *p = new DeactivateDataCallRequestData(m_nReq, m_tok, m_reqType);
    if (p != NULL) {
        p->encode((char *)m_ptrStrings, m_nStringCount * sizeof(char *));
    }
    return p;
}

int DeactivateDataCallRequestData::GetCid() const
{
    const char *strCid = GetString(0);
    if (strCid != NULL && TextUtils::IsDigitsOnly(strCid)) {
        return atoi(strCid);
    }
    return -1;
}

int DeactivateDataCallRequestData::GetDisconnectReason() const
{
    int reason = DEACT_REASON_NORMAL;
    const char *strReason = GetString(1);
    if (strReason != NULL && TextUtils::IsDigitsOnly(strReason)) {
        reason = atoi(strReason);
        if (mHalVer >= HAL_VERSION_CODE(1,2)) {
            switch (reason) {
            case RIL_DataRequestReason::NORMAL:
                reason = DEACT_REASON_NORMAL;
                break;
            case RIL_DataRequestReason::SHUTDOWN:
                reason = DEACT_REASON_RADIO_SHUTDOWN;
                break;
            case RIL_DataRequestReason::HANDOVER:
            case RIL_DataRequestReason::PDP_RESET:
                // re-use handover as PDP_RESET
                reason = DEACT_REASON_PDP_RESET;
                break;
            default:
                reason = DEACT_REASON_NORMAL;
                break;
            }
        }
    }
    return reason;
}

/**
 * SetInitialAttachApnRequestData
 */
SetInitialAttachApnRequestData::SetInitialAttachApnRequestData(const int nReq,const Token tok,const ReqType type)
    : RequestData(nReq, tok, type)
{
    memset(&mDataProfileInfo, 0, sizeof(mDataProfileInfo));
}

SetInitialAttachApnRequestData::~SetInitialAttachApnRequestData()
{
    DataProfileInfoUtils::FreeDataProfileInfo(mDataProfileInfo);
    memset(&mDataProfileInfo, 0, sizeof(mDataProfileInfo));
}

INT32 SetInitialAttachApnRequestData::encode(char *data, unsigned int datalen)
{
    if (data == NULL || datalen == 0) {
         return -1;
    }

    if (mHalVer >= HAL_VERSION_CODE(1,4)) {
        return encode((RIL_DataProfileInfo_V1_4 *)data, datalen);
    }
    else {
        // legacy
        if (datalen == sizeof(RIL_InitialAttachApn_v15)) {
            return encode((RIL_InitialAttachApn_v15 *)data, datalen);
        }
    }
    // not to support old data structure (RIL_VERSION < 15)
    RilLogW("unexpected datalen(%lu) of RIL_DataProfileInfo_V1_4 or RIL_InitialAttachApn_v15", datalen);
    return -1;
}

int SetInitialAttachApnRequestData::encode(RIL_DataProfileInfo_V1_4 *dataProfileInfo, size_t size)
{
    if (dataProfileInfo == NULL || size != sizeof(RIL_DataProfileInfo_V1_4)) {
        RilLogW("[SetInitialAttachApnRequestData] Unexpected datalen(%zu) for RIL_DataProfileInfo_V1_4", size);
        return -1;
    }

    if (!DataProfileInfoUtils::CopyDataProfileInfo(mDataProfileInfo, *dataProfileInfo)) {
        RilLogW("[SetInitialAttachApnRequestData] Failed to CopyDataProfileInfo");
        return -1;
    }

    return 0;
}

int SetInitialAttachApnRequestData::encode(RIL_InitialAttachApn_v15 *iaa, size_t size)
{
    if (iaa == NULL || size != sizeof(RIL_InitialAttachApn_v15)) {
        RilLogW("[SetInitialAttachApnRequestData] Unexpected datalen(%zu) for RIL_InitialAttachApn_v15", size);
        return -1;
    }

    DataProfileInfoUtils::FreeDataProfileInfo(mDataProfileInfo);
    memset(&mDataProfileInfo, 0, sizeof(mDataProfileInfo));

    if (!TextUtils::DupString(&mDataProfileInfo.apn, iaa->apn) ||
        !TextUtils::DupString(&mDataProfileInfo.user, iaa->username) ||
        !TextUtils::DupString(&mDataProfileInfo.password, iaa->password)) {
        RilLogW("[SetInitialAttachApnRequestData] memory alloc error");
        return -1;
    }
    mDataProfileInfo.protocol = PDP_PROTOCOL_TYPE_IP;
    if (TextUtils::Equals(iaa->protocol, "IP")) {
        mDataProfileInfo.protocol = PDP_PROTOCOL_TYPE_IP;
    }
    else if (TextUtils::Equals(iaa->protocol, "IPV6")) {
        mDataProfileInfo.protocol = PDP_PROTOCOL_TYPE_IPV6;
    }
    else if (TextUtils::Equals(iaa->protocol, "IPV4V6")) {
        mDataProfileInfo.protocol = PDP_PROTOCOL_TYPE_IPV4V6;
    }
    mDataProfileInfo.authType = iaa->authtype;
    mDataProfileInfo.roamingProtocol = PDP_PROTOCOL_TYPE_IP;
    if (TextUtils::Equals(iaa->roamingProtocol, "IP")) {
        mDataProfileInfo.roamingProtocol = PDP_PROTOCOL_TYPE_IP;
    }
    else if (TextUtils::Equals(iaa->roamingProtocol, "IPV6")) {
        mDataProfileInfo.roamingProtocol = PDP_PROTOCOL_TYPE_IPV6;
    }
    else if (TextUtils::Equals(iaa->roamingProtocol, "IPV4V6")) {
        mDataProfileInfo.roamingProtocol = PDP_PROTOCOL_TYPE_IPV4V6;
    }
    mDataProfileInfo.enabled = true;
    mDataProfileInfo.supportedApnTypesBitmap = iaa->supportedTypesBitmask;
    mDataProfileInfo.bearerBitmap = iaa->bearerBitmask;
    mDataProfileInfo.mtu = iaa->mtu;
    return 0;
}

SetInitialAttachApnRequestData *SetInitialAttachApnRequestData::Clone() const
{
    int request = ENCODE_REQUEST(m_nReq, mHalVer);
    SetInitialAttachApnRequestData *p = new SetInitialAttachApnRequestData(request, m_tok, m_reqType);
    if (p != NULL) {
        p->encode((char *)&mDataProfileInfo, sizeof(mDataProfileInfo));
    }
    return p;
}

const char *SetInitialAttachApnRequestData::GetApn() const
{
    return mDataProfileInfo.apn;
}

const char *SetInitialAttachApnRequestData::GetUsername() const
{
    return mDataProfileInfo.user;
}

const char *SetInitialAttachApnRequestData::GetPassword() const
{
    return mDataProfileInfo.password;
}

const char *SetInitialAttachApnRequestData::GetProtocol() const
{
    switch (mDataProfileInfo.protocol) {
    case PDP_PROTOCOL_TYPE_IP:
        return "IP";
    case PDP_PROTOCOL_TYPE_IPV6:
        return "IPV6";
    case PDP_PROTOCOL_TYPE_IPV4V6:
        return "IPV4V6";
    case PDP_PROTOCOL_TYPE_NON_IP:
        return "NONIP";
    default:
        break;
    }
    return "IP";
}

int SetInitialAttachApnRequestData::GetPdpProtocolType() const{
    return mDataProfileInfo.protocol;
}

int SetInitialAttachApnRequestData::GetAuthType() const
{
    return mDataProfileInfo.authType;
}

const char *SetInitialAttachApnRequestData::GetRoamingProtocol() const
{
    switch (mDataProfileInfo.roamingProtocol) {
    case PDP_PROTOCOL_TYPE_IP:
        return "IP";
    case PDP_PROTOCOL_TYPE_IPV6:
        return "IPV6";
    case PDP_PROTOCOL_TYPE_IPV4V6:
        return "IPV4V6";
    case PDP_PROTOCOL_TYPE_NON_IP:
        return "NONIP";
    default:
        break;
    }
    return "IP";
}

int SetInitialAttachApnRequestData::GetPdpRoamingProtocolType() const
{
    return mDataProfileInfo.roamingProtocol;
}

int SetInitialAttachApnRequestData::GetSupportedApnTypesBitmap() const
{
    return mDataProfileInfo.supportedApnTypesBitmap;
}

int SetInitialAttachApnRequestData::GetBearerBitmap() const
{
    return mDataProfileInfo.bearerBitmap;
}

int SetInitialAttachApnRequestData::GetModemCognitive() const
{
    return mDataProfileInfo.persistent;
}

int SetInitialAttachApnRequestData::GetMtu() const
{
    return mDataProfileInfo.mtu;
}

bool SetInitialAttachApnRequestData::IsEnabled() const
{
    return mDataProfileInfo.enabled;
}

bool SetInitialAttachApnRequestData::IsPrefferred() const
{
    return mDataProfileInfo.preferred;
}

bool SetInitialAttachApnRequestData::IsPersist() const
{
    return mDataProfileInfo.persistent;
}

/**
 * SetDataProfileRequestData
 */
SetDataProfileRequestData::SetDataProfileRequestData(const int nReq,const Token tok,const ReqType type)
    : RequestData(nReq, tok, type)
{
    mDataProfileInfos = NULL;
    mSize = 0;
}

SetDataProfileRequestData::~SetDataProfileRequestData()
{
    if (mDataProfileInfos != NULL) {
        for (int i = 0; i < mSize; i++) {
            DataProfileInfoUtils::FreeDataProfileInfo(mDataProfileInfos[i]);
        } // end for i ~
        delete[] mDataProfileInfos;
        mDataProfileInfos = NULL;
    }
    mSize = 0;
}

int SetDataProfileRequestData::encode(char *data, unsigned int datalen)
{
    if (data == NULL || datalen == 0) {
        return -1;
    }

    if (datalen % sizeof(void *) != 0) {
        return -1;
    }

    size_t size = datalen / sizeof(void *);
    if (mHalVer < HAL_VERSION_CODE(1,4)) {
        return encode((RIL_DataProfileInfo_v15 **)data, size);
    }
    return encode((RIL_DataProfileInfo_V1_4 **)data, size);

}

int SetDataProfileRequestData::encode(RIL_DataProfileInfo_V1_4 **dataProfileInfo, size_t size)
{
    if (dataProfileInfo == NULL || size == 0) {
        return -1;
    }

    mDataProfileInfos = new RIL_DataProfileInfo_V1_4[size];
    if (mDataProfileInfos == NULL) {
        return -1;
    }
    mSize = size;
    for (size_t i = 0; i < size; i++) {
        RIL_DataProfileInfo_V1_4 *dpi = dataProfileInfo[i];
        if (!DataProfileInfoUtils::CopyDataProfileInfo(mDataProfileInfos[i], *dpi)) {
            return -1;
        }
    } // end for i ~

    return 0;
}

int SetDataProfileRequestData::encode(RIL_DataProfileInfo_v15 **dataProfileInfo, size_t size)
{
    if (dataProfileInfo == NULL || size == 0) {
        return -1;
    }

    mDataProfileInfos = new RIL_DataProfileInfo_V1_4[size];
    if (mDataProfileInfos == NULL) {
        return -1;
    }
    mSize = size;

    for (int i = 0; i < size; i++) {
        RIL_DataProfileInfo_v15 *dpi = ((RIL_DataProfileInfo_v15 **) dataProfileInfo)[i];
        DataProfileInfoUtils::CopyDataProfileInfo(mDataProfileInfos[i], *dpi);
    } // end for i ~

    return 0;
}


SetDataProfileRequestData *SetDataProfileRequestData::Clone() const
{
    SetDataProfileRequestData *p = new SetDataProfileRequestData(ENCODE_REQUEST(m_nReq, mHalVer), m_tok, m_reqType);
    if (p != NULL) {
        if (mSize > 0) {
            RIL_DataProfileInfo_V1_4 **dataProfilePtrs = new RIL_DataProfileInfo_V1_4*[mSize];
            if (dataProfilePtrs != NULL) {
                for (int i = 0; i < mSize; i++) {
                    dataProfilePtrs[i] = &mDataProfileInfos[i];
                }
                p->encode((char *) dataProfilePtrs, sizeof(RIL_DataProfileInfo_V1_4 *) * mSize);
                delete[] dataProfilePtrs;
            }
        }
    }
    return p;
}

const RIL_DataProfileInfo_V1_4 *SetDataProfileRequestData::GetDataProfileInfo(int index) const
{
    if (index < 0 || index >= mSize)
        return NULL;
    return mDataProfileInfos + index;
}

int SetDataProfileRequestData::GetSize() const
{
    return mSize;
}



/**
 * KeepaliveRequestData
 */
KeepaliveRequestData::KeepaliveRequestData(const int nReq, const Token tok, const ReqType type)
    : RequestData(nReq, tok, type)
{
    memset(&m_keepAliveReq, 0, sizeof(RIL_KeepaliveRequest));
}

KeepaliveRequestData::~KeepaliveRequestData()
{
}

int KeepaliveRequestData::encode(char *data, unsigned int datalen)
{
    int lengthStr;
    if((0 == datalen) || (NULL == data)) return -1;

    RIL_KeepaliveRequest *reqData = (RIL_KeepaliveRequest *)data;

    m_keepAliveReq.type = reqData->type;
    lengthStr = strlen(reqData->sourceAddress);
    if (lengthStr > MAX_INADDR_LEN ) lengthStr = MAX_INADDR_LEN;
    memcpy(m_keepAliveReq.sourceAddress, reqData->sourceAddress, lengthStr);
    m_keepAliveReq.sourcePort = reqData->sourcePort;

    lengthStr = strlen(reqData->destinationAddress);
    if (lengthStr > MAX_INADDR_LEN ) lengthStr = MAX_INADDR_LEN;
    memcpy(m_keepAliveReq.destinationAddress, reqData->destinationAddress, lengthStr);
    m_keepAliveReq.destinationPort = reqData->destinationPort;

    m_keepAliveReq.maxKeepaliveIntervalMillis = reqData->maxKeepaliveIntervalMillis;
    m_keepAliveReq.cid = reqData->cid;

    return 0;
}

KeepaliveRequestData *KeepaliveRequestData::Clone() const
{
    KeepaliveRequestData *p = new KeepaliveRequestData(m_nReq, m_tok, m_reqType);
    if (p != NULL) {
        p->encode((char *)&m_keepAliveReq, sizeof(RIL_KeepaliveRequest));
    }
    return p;
}
