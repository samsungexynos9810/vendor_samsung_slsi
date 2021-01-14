/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "callreqdata.h"
#include "rillog.h"
#include <telephony/ril.h>

#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

UusInfo::UusInfo()
{
    Clear();
}

UusInfo::UusInfo(RIL_UUS_Type type, RIL_UUS_DCS dcs, char *data, int len)
{
    m_uusType = type;
    m_uusDcs = dcs;
    m_uusLength = MIN(len, MAX_UUS_DATA_LEN);
    memset(m_uusData, 0, sizeof(m_uusData));

    if (data && m_uusLength > 0)
    {
        memcpy(m_uusData, data, m_uusLength);
    }
}

void UusInfo::Clear()
{
    m_uusType = RIL_UUS_TYPE1_IMPLICIT;
    m_uusDcs = RIL_UUS_DCS_USP;
    m_uusLength = 0;
    memset(m_uusData, 0, sizeof(m_uusData));
}

CallDialReqData::CallDialReqData(const int nReq,const Token tok,const ReqType type)
:RequestData(nReq,tok,type)
{
    memset(m_number,0, sizeof(m_number));

    m_clirType = CLIR_DEFAULT;
    m_callType = CALL_TYPE_VOICE;
    m_eccCategory = 0;
}

INT32 CallDialReqData::encode(char *data, unsigned int datalen)
{
    if (NULL == data || 0 == datalen)
    {
        return -1;
    }

    RIL_Dial *dial = (RIL_Dial *)data;
    strncpy(m_number, dial->address, MAX_DIAL_NUM);
    m_clirType = (ClirType)dial->clir;
    if (dial->uusInfo)
    {
        m_uusInfo.m_uusType = dial->uusInfo->uusType;
        m_uusInfo.m_uusDcs = dial->uusInfo->uusDcs;
        m_uusInfo.m_uusLength = dial->uusInfo->uusLength;
        memcpy(m_uusInfo.m_uusData, dial->uusInfo->uusData, dial->uusInfo->uusLength);
    }

    m_callType = CALL_TYPE_VOICE;
    if (datalen == (unsigned int)sizeof(RIL_Dial_Ext)) {
        m_callType = (CallType)((RIL_Dial_Ext *)dial)->callType;
    }

    return 0;
}

INT32 CallDialReqData::encodeCallNumber(char *data, unsigned int datalen)
{
    if (NULL == data || 0 == datalen)
    {
        return -1;
    }

    RIL_Dial *dial = (RIL_Dial *)data;
    strncpy(m_number, dial->address, MAX_DIAL_NUM);
    m_clirType = (ClirType)dial->clir;
    m_callType = CALL_TYPE_VOICE;
    if (datalen == (unsigned int)sizeof(RIL_Dial_Ext)) {
        m_callType = (CallType)((RIL_Dial_Ext *)dial)->callType;
    }
    return 0;
}

CallEmergencyDialReqData::CallEmergencyDialReqData(const int nReq,const Token tok,const ReqType type)
    :CallDialReqData(nReq,tok,type)
{
    m_categories = 0;
    m_renUrns = 0;
    m_urns = NULL;
    m_routing = 0;
    m_hasKnownUserIntentEmergency = false;
    m_isTesting = false;
}

CallEmergencyDialReqData::~CallEmergencyDialReqData()
{
}

INT32 CallEmergencyDialReqData::encode(char *data, unsigned int datalen)
{
    if (NULL == data || 0 == datalen) {
        return -1;
    }

    if (datalen < sizeof(RIL_EmergencyDial)) {
        return -1;
    }

    RIL_EmergencyDial *emergencyDial = (RIL_EmergencyDial *)data;
    if (CallDialReqData::encode((char *)&emergencyDial->dialInfo, sizeof(RIL_Dial)) < 0) {
        return -1;
    }

    m_categories = emergencyDial->categories;
    m_routing = emergencyDial->routing;

    return 0;
}

CallForwardReqData::CallForwardReqData(const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq,tok,type)
{
    m_status = RIL_SS_MODE_DISABLE;
    m_reason = RIL_SS_CF_REASON_UNCONDITIONAL;
    m_classType = RIL_SS_CLASS_UNKNOWN;
    m_toa = 0;
    memset(m_number, 0x00, sizeof(m_number));
    m_timeSeconds = 0;
    m_serviceClass = 0;
}

INT32 CallForwardReqData::encode(char *data, unsigned int datalen)
{
    RIL_CallForwardInfo *cf = (RIL_CallForwardInfo *)data;
    m_status = (SsModeType)cf->status;
    m_reason = (SsCfReason)cf->reason;
    m_classType = (SsClassX)cf->serviceClass;
    m_toa = cf->toa;

    memset(m_number, 0x00, sizeof(m_number));
    if (cf->number)
    {
        int len = MIN(strlen(cf->number), MAX_DIAL_NUM);
        strncpy(m_number, cf->number, len);
    }
    m_timeSeconds = cf->timeSeconds;

    return 0;
}

CallList::CallList()
{
    Clear();
}
CallList::~CallList()
{
}
void CallList::Clear()
{
    m_nCount = 0;
    for(int i=0;i<MAX_CALL_LIST_COUNT;i++)
        m_szCallInfo[i].Clear();
    //memset(m_szCallInfo, 0, sizeof(m_szCallInfo));
}

CallInfo::CallInfo()
{
    Clear();
}

void CallInfo::Clear()
{
    m_state = RIL_CALL_ACTIVE;
    m_nIndex = -1;
    m_toa = -1;
    m_isMParty = FALSE;
    m_isMt = FALSE;
    m_isVoice = FALSE;
    m_isVideo = FALSE;
    m_isVoicePrivacy = FALSE;
    m_numPresent = RIL_CALL_NAME_PRESENTATION_ALLOW;
    m_namePresent = RIL_CALL_NAME_PRESENTATION_ALLOW;
    m_als = -1;
    memset(m_number, 0, sizeof(m_number));
    memset(m_name, 0, sizeof(m_name));
    m_uusInfo.Clear();
    m_audioQuality = UNSPECIFIED;
}

EmcInfoList::EmcInfoList()
{
    clear();
}
EmcInfoList::~EmcInfoList()
{
}
void EmcInfoList::clear()
{
    m_count = 0;
    for(int i=0;i<MAX_EMERGENCY_NUMBER_LIST_COUNT;i++)
        m_emcInfoList[i].clear();
}

void EmcInfo::clear()
{
    memset(m_mcc, 0, sizeof(m_mcc));
    memset(m_mnc, 0, sizeof(m_mnc));
    memset(m_number, 0, sizeof(m_number));
    memset(m_urn, 0, sizeof(m_urn));
    m_number_len = 0;
    m_urn_count = 0;
    m_source = -1;
    m_category = -1;
    m_conditions = -1;
}

void EmcInfo::update(const char* mcc, const char* mnc, char* number, int number_len,
                     int category, int conditions, int source)
{
    this->update(mcc, mnc, number, number_len, NULL, 0, category, conditions, source);
}
void EmcInfo::update(const char* mcc, const char* mnc, char* number, int number_len,
                     char* urn, int urn_len, int category, int conditions, int source)
{
    int len = MIN(strlen(mcc), sizeof(m_mcc) - 1);
    strncpy(m_mcc, mcc, len);
    len = MIN(strlen(mnc), sizeof(m_mnc) - 1);
    strncpy(m_mnc, mnc, len);
    strncpy(m_number, number, number_len);
    m_number_len = number_len;
    m_category = category;
    m_conditions = conditions;
    m_source = source;
    this->updateUrn(urn, urn_len);
}

void EmcInfo::updateUrn(const char* urn, int urn_len) {
    if (MAX_URN_COUNT == m_urn_count) {
        RilLogI("updateUrn fail: max URN");
        return;
    }

    if (urn_len > 0) {
        strncpy(m_urn[m_urn_count], urn, urn_len);
        m_urn_count++;
    }
}

DtmfInfo::DtmfInfo(const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq,tok,type)
{
    memset(m_szDtmf, 0, sizeof(m_szDtmf));
}

DtmfInfo::DtmfInfo(const int nReq, const Token tok, const char dtmf)
    :RequestData(nReq,tok)
{
    memset(m_szDtmf, 0, sizeof(m_szDtmf));
    m_szDtmf[0] = dtmf;
}

DtmfInfo::DtmfInfo(const int nReq, const Token tok, const char* dtmf)
    :RequestData(nReq,tok)
{
    memset(m_szDtmf, 0, sizeof(m_szDtmf));

    if (dtmf == NULL)
    {
        return;
    }

    int len = MIN(strlen(dtmf), sizeof(m_szDtmf) - 1);
    strncpy(m_szDtmf, dtmf, len);
}

/*
    Sound
*/
SndMuteReqData::SndMuteReqData(const int nReq, const Token tok, const ReqType type)
    :RequestData(nReq,tok,type)
{
    m_status = RIL_MUTE_STATUS_UNMUTE;
}

INT32 SndMuteReqData::encode(char *data, unsigned int datalen)
{
    m_status = (MuteStatus)(*((int *)data));
    return 0;
}
