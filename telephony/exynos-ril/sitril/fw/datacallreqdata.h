/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _DATA_CALL_REQ_DATA_H_
#define _DATA_CALL_REQ_DATA_H_

#include "requestdata.h"

/**
 * SetupDataCallRequestData
 */
class SetupDataCallRequestData : public StringsRequestData {
public:
    RIL_DataProfileInfo_V1_4 mDataProfileInfo;
public:
    SetupDataCallRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~SetupDataCallRequestData();
    virtual int encode(char *data, unsigned int datalen);
    virtual SetupDataCallRequestData *Clone() const;

public:
    int GetRadioTech() const;
    int GetDataProfileId() const;
    const char *GetApn() const;
    const char *GetUsername() const;
    const char *GetPassword() const;
    int GetAuthType() const;
    const char *GetProtocolByInt(int) const;
    const char *GetProtocol() const;
    const char *GetRoamingProtocol() const;
    int GetRadioAccessFamily() const;
    bool GetAPNSettingStatus() const;
    int GetMtuSize() const;
    const char *IsDataRoamingAllowed() const;
    bool IsRoamingAllowed() const;
    int GetSupportedApnTypesBitmap() const;
    const char *GetReason() const;
    const char *GetAddresses() const;
    const char *GetDnses() const;

protected:
    int encode(RIL_SetupDataCallInfo_V1_4 *dataCallInfo);
};

/**
 * DeactivateDataCallRequestData
 */
class DeactivateDataCallRequestData : public StringsRequestData {
public:
    DeactivateDataCallRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~DeactivateDataCallRequestData();
    virtual int encode(char *data, unsigned int datalen);
    virtual DeactivateDataCallRequestData *Clone() const;

public:
    int GetCid() const;
    int GetDisconnectReason() const;
};

/**
 * SetInitialAttachApnRequestData
 */
class SetInitialAttachApnRequestData : public RequestData {
public:
    RIL_DataProfileInfo_V1_4 mDataProfileInfo;

public:
    SetInitialAttachApnRequestData(const int nReq,const Token tok,const ReqType type = REQ_FW);
    virtual ~SetInitialAttachApnRequestData();
    INT32 encode(char *data, unsigned int datalen);
    virtual SetInitialAttachApnRequestData *Clone() const;
private:
    int encode(RIL_DataProfileInfo_V1_4 *dataProfileInfo, size_t size);
    int encode(RIL_InitialAttachApn_v15 *iaa, size_t size);

public:
    const char *GetApn() const;
    const char *GetUsername() const;
    const char *GetPassword() const;
    const char *GetProtocol() const;
    const char *GetRoamingProtocol() const;
    int GetPdpProtocolType() const;
    int GetPdpRoamingProtocolType() const;
    int GetAuthType() const;
    int GetSupportedApnTypesBitmap() const;
    int GetBearerBitmap() const;
    int GetModemCognitive() const;
    int GetMtu() const;
    bool IsEnabled() const;
    bool IsPrefferred() const;
    bool IsPersist() const;
};

/**
 * SetDataProfileRequestData
 */
class SetDataProfileRequestData : public RequestData {
public:
    RIL_DataProfileInfo_V1_4 *mDataProfileInfos;
    int mSize;
public:
    SetDataProfileRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~SetDataProfileRequestData();
    virtual int encode(char *data, unsigned int datalen);
    virtual SetDataProfileRequestData *Clone() const;

public:
    const RIL_DataProfileInfo_V1_4 *GetDataProfileInfo(int index) const;
    int GetSize() const;

private:
    int encode(RIL_DataProfileInfo_V1_4 **dataProfileInfo, size_t size);
    int encode(RIL_DataProfileInfo_v15 **dataProfileInfo, size_t size);
};

class KeepaliveRequestData : public RequestData {
public:
    RIL_KeepaliveRequest m_keepAliveReq;
public:
    KeepaliveRequestData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    virtual ~KeepaliveRequestData();
    virtual int encode(char *data, unsigned int datalen);
    virtual KeepaliveRequestData *Clone() const;
};

#endif /*_DATA_CALL_REQ_DATA_H_*/
