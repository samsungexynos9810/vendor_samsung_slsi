 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _CALL_DIAL_REQ_DATA_H_
#define _CALL_DIAL_REQ_DATA_H_

#include "requestdata.h"

/* User-to-User Signaling Information defined in 3GPP 23.087 v8.0
 * This data is passed in RIL_ExtensionRecord and rec contains this
 * structure when type is RIL_UUS_INFO_EXT_REC */
//typedef struct {
//  RIL_UUS_Type    uusType;    /* UUS Type */
//  RIL_UUS_DCS     uusDcs;     /* UUS Data Coding Scheme */
//  int             uusLength;  /* Length of UUS Data */
//  char *          uusData;    /* UUS Data */
//} RIL_UUS_Info;

class UusInfo
{
public:
    RIL_UUS_Type m_uusType;
    RIL_UUS_DCS m_uusDcs;
    int m_uusLength;
    char m_uusData[MAX_UUS_DATA_LEN];

public:
    UusInfo();
    UusInfo(RIL_UUS_Type type, RIL_UUS_DCS dcs, char *data, int len);
    ~UusInfo() {}
    void Clear();
};

class CallInfo
{
public:
    RIL_CallState GetState() {return m_state;}
    CallInfo();
    void Clear();
public:
    RIL_CallState m_state;
    INT32 m_nIndex;
    INT32 m_toa;
    BOOL m_isMParty;
    BOOL m_isMt;
    BOOL m_isVoice;
    BOOL m_isVideo;
    BOOL m_isVoicePrivacy;
    CallPresentation m_numPresent;
    CallPresentation m_namePresent;
    char m_als;
    char m_number[MAX_DIAL_NUM+1];
    char m_name[MAX_DIAL_NAME+1];
    UusInfo m_uusInfo;
    AudioQuality m_audioQuality;
};

class CallList
{
public:
    CallList();
    ~CallList();
    void Clear();
    INT32 GetCount() {return m_nCount;}
    CallInfo *GetCallInfo() {return m_szCallInfo;}
public:
    static const int MAX_CALL_LIST_COUNT = 9;
    CallInfo m_szCallInfo[MAX_CALL_LIST_COUNT];
    INT32 m_nCount;
};

class EmcInfo
{
public:
    void clear();
    void update(const char* mcc, const char* mnc, char* number, int number_len,
                int category, int conditions, int source);
    void update(const char* mcc, const char* mnc, char* number, int number_len,
                char* urn, int urn_len, int category, int conditions, int source);
    void updateUrn(const char* urn, int urn_len);
public:
    char m_mcc[MAX_MCC_LEN + 1];
    char m_mnc[MAX_MNC_LEN + 1];
    char m_number[MAX_EMERGENCY_NUMBER_LEN + 1];
    char m_urn[MAX_URN_COUNT][MAX_URN_LEN + 1];
    char *m_pUrn[MAX_URN_COUNT];
    int m_number_len;
    int m_urn_count;
    int m_source;
    int m_category;
    int m_conditions;
};

class EmcInfoList
{
public:
    INT32 m_count;
    EmcInfo m_emcInfoList[MAX_EMERGENCY_NUMBER_LIST_COUNT];
public:
    EmcInfoList();
    ~EmcInfoList();
    void clear();
    INT32 getCount() { return m_count; }
    EmcInfo *getEmcInfoList() { return m_emcInfoList; }
};

class DtmfInfo : public RequestData
{
    public:
        char m_szDtmf[MAX_DIAL_NUM+1];

    public:
        DtmfInfo(const int nReq, const Token tok, const ReqType type = REQ_FW);
        DtmfInfo(const int nReq, const Token tok, const char dtmf);
        DtmfInfo(const int nReq, const Token tok, const char *dtmf);
        ~DtmfInfo() {}
};

class CallDialReqData : public RequestData
{
public:
    CallDialReqData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    ~CallDialReqData() {}

public:
    INT32 encode(char *data, unsigned int datalen);
    INT32 encodeCallNumber(char *data, unsigned int datalen);
    char * GetNumber()
    {
        return m_number;
    }

    void SetClirType(ClirType type)
    {
        m_clirType = type;
    }

    ClirType GetClirType()
    {
        return m_clirType;
    }
    UusInfo GetUusInfo()
    {
        return m_uusInfo;
    }

    void SetCallType(CallType type)
    {
        m_callType = type;
    }

    CallType GetCallType()
    {
        return m_callType;
    }
    INT32 GetEccCategory()
    {
        return m_eccCategory;
    }

private:
    char m_number[MAX_DIAL_NUM+1];
    ClirType m_clirType;
    UusInfo m_uusInfo;
    CallType m_callType;
    INT32 m_eccCategory;
};

class CallEmergencyDialReqData : public CallDialReqData
{
public:
    CallEmergencyDialReqData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    ~CallEmergencyDialReqData();
public:
    INT32 encode(char *data, unsigned int datalen);

    INT32 GetCategories() { return m_categories; }
    INT32 GetLenUrns() { return m_renUrns; }
    char** GetUrns() { return m_urns; }
    INT32 GetRouting() { return m_routing; }
    BOOL GetHasKnownUserIntentEmergency() { return m_hasKnownUserIntentEmergency; }
    BOOL GetIsTesting() { return m_isTesting; }

private:
    INT32 m_categories;
    INT32 m_renUrns;
    char** m_urns;
    INT32 m_routing;
    BOOL m_hasKnownUserIntentEmergency;
    BOOL m_isTesting;
};

class CallForwardReqData : public RequestData
{
public:
    CallForwardReqData(const int nReq, const Token tok, const ReqType type = REQ_FW);
public:
    INT32 encode(char *data, unsigned int datalen);
    SsModeType GetStatus()
    {
        return m_status;
    }
    SsCfReason GetReason()
    {
        return m_reason;
    }
    SsClassX GetSsClassType()
    {
        return m_classType;
    }
    char* GetNumber()
    {
        return m_number;
    }
    INT32 GetToa()
    {
        return m_toa;
    }
    INT32 GetTimeSeconds()
    {
        return m_timeSeconds;
    }
private:
    SsModeType m_status;
    SsCfReason m_reason;
    SsClassX m_classType;
    INT32 m_toa;
    char m_number[MAX_DIAL_NUM+1];
    INT32 m_timeSeconds;
    INT32 m_serviceClass;
};

#if 0
class CBarringPwdReqData : public RequestData
{
public:
    CBarringPwdReqData(const int nReq, const Token tok, const ReqType type);
public:
    INT32 encode(const int len, const char *reqData);
    char * GetOldPwd() {return m_oldPwd;}
    char * GetNewPwd() {return m_newPwd;}
    char * GetFacilityStringCode() {return m_fcStringCode;}
private:
    static const int MAX_BARRING_PWD_LEN = 4;
    static const unsigned int MAX_FACILITY_STRING_CODE = 2;
    char m_oldPwd[MAX_BARRING_PWD_LEN+1];
    char m_newPwd[MAX_BARRING_PWD_LEN+1];
    char m_fcStringCode[4];
};
#endif

class SndMuteReqData : public RequestData
{
public:
    SndMuteReqData(const int nReq, const Token tok, const ReqType type = REQ_FW);
    INT32 encode(char *data, unsigned int datalen);
public:
    MuteStatus m_status;
};

#endif /*_CALL_DIAL_REQ_DATA_H_*/
