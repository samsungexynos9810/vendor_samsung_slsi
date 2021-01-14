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
 * protocolcalladapter.cpp
 *
 *  Created on: 2014. 6. 27.
 *      Author: jhdaniel.kim
 */


#include "protocolcalladapter.h"
#include "rillog.h"
#include "callreqdata.h"
#include "util.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

const int RET_VALUE_SAME_AS_SIT_COMMAND = -99;

sit_ril_match_table SitRilValueTable[] =
{
    {
        SIT_CALL_TB_IDX_STATE,
        -1,
        -1,
        {
            {SIT_CALL_STATE_DIALING,      RIL_CALL_DIALING},
            {SIT_CALL_STATE_INCOMING,     RIL_CALL_INCOMING},
            {SIT_CALL_STATE_WAITING,      RIL_CALL_WAITING},
            {SIT_CALL_STATE_ALERTING,     RIL_CALL_ALERTING},
            {SIT_CALL_STATE_ACTIVE,       RIL_CALL_ACTIVE},
            {SIT_CALL_STATE_HOLDING,      RIL_CALL_HOLDING}
        }
    },
    {
        SIT_CALL_TB_IDX_PRESENTATION,
        -1,
        -1,
        {
            {SIT_CALL_PRESENTATION_ALLOWED,          RIL_CALL_NAME_PRESENTATION_ALLOW},
            {SIT_CALL_PRESENTATION_RESTRICTED,       RIL_CALL_NAME_PRESENTATION_RESTRICT},
            {SIT_CALL_PRESENTATION_NOT_SPECIFIED,    RIL_CALL_NAME_PRESENTATION_UNKNOWN},
            {SIT_CALL_PRESENTATION_PAYPHONE,         RIL_CALL_NAME_PRESENTATION_PAYPHONE}
        }
    },
    {
        SIT_CALL_TB_IDX_UUS_TYPE,
        -1,
        -1,
        {
            {SIT_CALL_UUS_TYPE_TYPE1_IMPLICIT,            RIL_UUS_TYPE1_IMPLICIT},
            {SIT_CALL_UUS_TYPE_TYPE1_REQUIRED,            RIL_UUS_TYPE1_REQUIRED},
            {SIT_CALL_UUS_TYPE_TYPE1_NOT_REQUIRED,        RIL_UUS_TYPE1_NOT_REQUIRED},
            {SIT_CALL_UUS_TYPE_TYPE2_REQUIRED,            RIL_UUS_TYPE2_REQUIRED},
            {SIT_CALL_UUS_TYPE_TYPE2_NOT_REQUIRED,        RIL_UUS_TYPE2_NOT_REQUIRED},
            {SIT_CALL_UUS_TYPE_TYPE3_REQUIRED,            RIL_UUS_TYPE3_REQUIRED},
            {SIT_CALL_UUS_TYPE_TYPE3_NOT_REQUIRED,        RIL_UUS_TYPE3_NOT_REQUIRED}
        }
    },
    {
        SIT_CALL_TB_IDX_UUS_DCS,
        -1,
        -1,
        {
            {SIT_CALL_UUS_DCS_USP,         RIL_UUS_DCS_USP},
            {SIT_CALL_UUS_DCS_OSIHLP,      RIL_UUS_DCS_OSIHLP},
            {SIT_CALL_UUS_DCS_X244RMCF,    RIL_UUS_DCS_X244},
            {SIT_CALL_UUS_DCS_RMCF,        RIL_UUS_DCS_RMCF},
            {SIT_CALL_UUS_DCS_IA5C,        RIL_UUS_DCS_IA5c}
        }
    },
    {
        SIT_CALL_TB_IDX_TYPE,
        -1,
        -1,
        {
            {SIT_CALL_CALL_TYPE_VOICE,             CALL_TYPE_VOICE},
            {SIT_CALL_CALL_TYPE_VIDEO,             CALL_TYPE_VIDEO},
            {SIT_CALL_CALL_TYPE_EMERGENCY,         CALL_TYPE_EMERGENCY},
#ifdef SUPPORT_CDMA
            {SIT_CALL_CALL_TYPE_CDMA_VOICE,        CALL_TYPE_CDMA_VOICE},
            {SIT_CALL_CALL_TYPE_CDMA_EMERGENCY,    CALL_TYPE_CDMA_EMERGENCY}
#endif
        }
    },
    {
        SIT_CALL_TB_IDX_CLIR,
        -1,
        -1,
        {
            {SIT_CALL_CLIR_DEFAULT,         CLIR_DEFAULT},
            {SIT_CALL_CLIR_INVOCATION,      CLIR_INVOCATION},
            {SIT_CALL_CLIR_SUPPRESSION,     CLIR_SUPPRESSION}
        }
    },
/*    {
        SIT_CALL_TB_IDX_RELEASECAUSE,
        SIT_CALL_LAST_CALL_FAIL_NORMAL,
        RET_VALUE_SAME_AS_SIT_COMMAND,
        {
            {SIT_CALL_LAST_CALL_FAIL_UNOBTAINABLE_NUMBER,    CALL_FAIL_UNOBTAINABLE_NUMBER},
            {SIT_CALL_LAST_CALL_FAIL_NORMAL,                 CALL_FAIL_NORMAL},
            {SIT_CALL_LAST_CALL_FAIL_BUSY,                   CALL_FAIL_BUSY},
            {SIT_CALL_LAST_CALL_FAIL_NORMAL_UNSPECIFIED,     CALL_FAIL_NORMAL_UNSPECIFIED},
            {SIT_CALL_LAST_CALL_FAIL_CONGESTION,             CALL_FAIL_CONGESTION},
            {SIT_CALL_LAST_CALL_FAIL_ACM_LIMIT_EXCEEDED,     CALL_FAIL_ACM_LIMIT_EXCEEDED},
            {SIT_CALL_LAST_CALL_FAIL_CALL_BARRED,            CALL_FAIL_CALL_BARRED},
            {SIT_CALL_LAST_CALL_FAIL_FDN_BLOCKED,            CALL_FAIL_FDN_BLOCKED},
            {SIT_CALL_LAST_CALL_FAIL_IMSI_UNKNOWN_IN_VLR,    CALL_FAIL_IMSI_UNKNOWN_IN_VLR},
            {SIT_CALL_LAST_CALL_FAIL_IMEI_NOT_ACCEPTED,      CALL_FAIL_IMEI_NOT_ACCEPTED},
            {SIT_CALL_LAST_CALL_FAIL_CDMA_LOCKED_UNTIL_POWER_CYCLE, CALL_FAIL_CDMA_LOCKED_UNTIL_POWER_CYCLE},
            {SIT_CALL_LAST_CALL_FAIL_CDMA_DROP, CALL_FAIL_CDMA_DROP},
            {SIT_CALL_LAST_CALL_FAIL_CDMA_INTERCEPT, CALL_FAIL_CDMA_INTERCEPT},
            {SIT_CALL_LAST_CALL_FAIL_CDMA_REORDER, CALL_FAIL_CDMA_REORDER},
            {SIT_CALL_LAST_CALL_FAIL_CDMA_SO_REJECT, CALL_FAIL_CDMA_SO_REJECT},
            {SIT_CALL_LAST_CALL_FAIL_CDMA_RETRY_ORDER, CALL_FAIL_CDMA_RETRY_ORDER},
            {SIT_CALL_LAST_CALL_FAIL_CDMA_ACCESS_FAILURE, CALL_FAIL_CDMA_ACCESS_FAILURE},
            {SIT_CALL_LAST_CALL_FAIL_CDMA_PREEMPTED, CALL_FAIL_CDMA_PREEMPTED},
            {SIT_CALL_LAST_CALL_FAIL_CDMA_NOT_EMERGENCY, CALL_FAIL_CDMA_NOT_EMERGENCY},
            {SIT_CALL_LAST_CALL_FAIL_CDMA_ACCESS_BLOCKED, CALL_FAIL_CDMA_ACCESS_BLOCKED},
            // SIT_CALL_LAST_CALL_FAIL_CDMA_xxx ...

            {SIT_CALL_LAST_CALL_FAIL_ERROR_UNSPECIFIED,      CALL_FAIL_ERROR_UNSPECIFIED},
        }
    },
*/
    {
        SIT_CALL_TB_IDX_GET_CLIP_STATE,
        -1,
        -1,
        {
            {SIT_SS_GET_CLIP_STATUS_NOT_PROVISIONED,         CLIP_NOT_PROVISIONED},
            {SIT_SS_GET_CLIP_STATUS_PROVISIONED,             CLIP_PROVISIONED},
            {SIT_SS_GET_CLIP_STATUS_UNKNOWN,                 CLIP_UNKNOWN},
        }
    },
    {
        SIT_CALL_TB_IDX_SERVICE_CLASS_CF_SET,
        -1,
        -1,
        {
            {SIT_SS_CALL_FARWARD_STATUS_DISABLE,             RIL_SS_MODE_DISABLE},
            {SIT_SS_CALL_FARWARD_STATUS_ENABLE,              RIL_SS_MODE_ENABLE},
            {SIT_SS_CALL_FARWARD_STATUS_INTERROGATE,         RIL_SS_MODE_INTERROGATE},
            {SIT_SS_CALL_FARWARD_STATUS_REGISTRATION,        RIL_SS_MODE_REGISTRATION},
            {SIT_SS_CALL_FARWARD_STATUS_ERASURE,             RIL_SS_MODE_ERASURE},
        }
    },
    {
        SIT_CALL_TB_IDX_SERVICE_CLASS_CF_GET,
        -1,
        -1,
        {
            {SIT_SS_CALL_FARWARD_STATUS_NOT_ACTIVE,          RIL_SS_STATUS_NOT_ACTIVE},
            {SIT_SS_CALL_FARWARD_STATUS_ACTIVE,              RIL_SS_STATUS_ACTIVE},
        }
    },
    {
        SIT_CALL_TB_IDX_SERVICE_STATUS_CALL_WAITING,
        -1,
        -1,
        {
            {SIT_SS_CALL_FARWARD_STATUS_DISABLE,             RIL_SS_MODE_DISABLE},
            {SIT_SS_CALL_FARWARD_STATUS_ENABLE,              RIL_SS_MODE_ENABLE},
        }
    },
    {
        SIT_CALL_TB_IDX_USSD_STATUS,
        -1,
        -1,
        {
            {SIT_SS_USSD_IND_STATUS_USSD_NOTIFY,                       RIL_USSD_NOTIFY},
            {SIT_SS_USSD_IND_STATUS_USSD_REQUEST,                      RIL_USSD_REQUEST},
            {SIT_SS_USSD_IND_STATUS_SESSION_TERMINATED_BY_NETWORK,     RIL_USSD_SESSION_TERMINATED_BY_NET},
            {SIT_SS_USSD_IND_STATUS_OTHER_LOCAL_CLIENT_HAS_RESPONDED,    RIL_USSD_OTHER_LOCAL_CLIENT_HAS_RESPONDED},
            {SIT_SS_USSD_IND_STATUS_OPERATION_NOT_SUPPORTED,           RIL_USSD_OPERATION_NOT_SUPPORT},
            {SIT_SS_USSD_IND_STATUS_NETWORK_TIMEOUT,                   RIL_USSD_NETWORK_TIMEOUT},
        }
    },
    {
        SIT_CALL_TB_IDX_SSNOTI_TYPE,
        -1,
        -1,
        {
            {SIT_SS_SSNOTI_TYPE_MO,    RIL_SSNOTI_TYPE_MO},
            {SIT_SS_SSNOTI_TYPE_MT,    RIL_SSNOTI_TYPE_MT},
        }
    },
};


static int ConvertSitDefineToRilDefine(int TableIndex, int SitValue)
{
    if ( TableIndex >= 0 && TableIndex < SIT_CALL_TB_IDX__MAX
        && SitRilValueTable[TableIndex].Index == TableIndex )
    {

        for ( int i = 0; i < MAX_MATCH_TABLE; i++ )
        {
            if ( SitRilValueTable[TableIndex].match_table[i].SitVal == SitValue )
            {
                return SitRilValueTable[TableIndex].match_table[i].RilVal;
            }
        }

        if ( RET_VALUE_SAME_AS_SIT_COMMAND == SitRilValueTable[TableIndex].DefRilValue ) {
            RilLogW("[%s] cannot find matched ril constant value with sit value(%d), return SIT value instead", __FUNCTION__, SitValue);
            return SitValue;
        }
        else {
            RilLogW("[%s] cannot find matched ril constant value with sit value(%d), return default value(%d)", __FUNCTION__, SitValue, SitRilValueTable[TableIndex].DefRilValue);
            return SitRilValueTable[TableIndex].DefRilValue;
        }
    }
    RilLogW("[%s] cannot find matched table with index(%d)", __FUNCTION__, TableIndex);
    return -1;
}


/**
 * ProtocolGetCurrentCallAdapter
 */

int ProtocolGetCurrentCallAdapter::GetCallNum()
{
    sit_call_get_current_calls_rsp *rsp = (sit_call_get_current_calls_rsp *)m_pModemData->GetRawData();
    if ( rsp != NULL )
    {
        return rsp->number;
    }

    return 0;
}

bool ProtocolGetCurrentCallAdapter::HasValidLength()
{
    sit_call_get_current_calls_rsp *rsp = (sit_call_get_current_calls_rsp *)m_pModemData->GetRawData();
    int packet_len = rsp->hdr.length;
    const int CALL_LIST_MIN_LEN = sizeof(RCM_HEADER)+sizeof(rsp->number);
    const int ONE_RECORD_LEN = sizeof(sit_call_info_type);

    if ( packet_len < CALL_LIST_MIN_LEN )
    {
        RilLogE("ProtocolGetCurrentCallAdapter::HasValidLength() : packet length(%d) is less than min value(%d)", packet_len, CALL_LIST_MIN_LEN);
        return false;
    }
    else if ( packet_len != ((ONE_RECORD_LEN * rsp->number) + CALL_LIST_MIN_LEN) )
    {
        RilLogE("ProtocolGetCurrentCallAdapter::HasValidLength() : packet length(%d) is invalid. Expected length(%d)",
                packet_len, ((ONE_RECORD_LEN * rsp->number) + CALL_LIST_MIN_LEN));
        return false;
    }
    return true;
}

int ProtocolGetCurrentCallAdapter::ConvertSitToUusInfo(UusInfo* pUusInfo, int uusType, int uus_dcs, int len, BYTE* pData)
{
    if ( pUusInfo != NULL )
    {
        pUusInfo->m_uusType = (RIL_UUS_Type)ConvertSitDefineToRilDefine(SIT_CALL_TB_IDX_UUS_TYPE, uusType);
        pUusInfo->m_uusDcs = (RIL_UUS_DCS)ConvertSitDefineToRilDefine(SIT_CALL_TB_IDX_UUS_DCS, uus_dcs);
        pUusInfo->m_uusLength = MIN(len, MAX_UUS_DATA_LEN);
        if ( pUusInfo->m_uusLength > 0 && pData != NULL )
        {
            memcpy(pUusInfo->m_uusData, pData, pUusInfo->m_uusLength);
        }
        return 0;
    }
    return -1;
}

int ProtocolGetCurrentCallAdapter::GetCallInfo(CallInfo* pCallInfo, int index)
{
    sit_call_get_current_calls_rsp *rsp = (sit_call_get_current_calls_rsp *)m_pModemData->GetRawData();

    if ( pCallInfo == NULL )
    {
        return -1;
    }
    else if ( index < 0 || index >= MAX_CALL_LIST_NUM || index >= rsp->number )
    {
        return -2;
    }

    sit_call_info_type* pcallinfo = &(rsp->record[index]);

    pCallInfo->m_state = (RIL_CallState)ConvertSitDefineToRilDefine(SIT_CALL_TB_IDX_STATE, pcallinfo->state);
    pCallInfo->m_nIndex = pcallinfo->index;
    RilLogV("pCallInfo->m_nIndex : %d", pCallInfo->m_nIndex);
    pCallInfo->m_toa = (pcallinfo->type_of_address == RIL_TOA_INTERNATIONAL) ? RIL_TOA_INTERNATIONAL : RIL_TOA_UNKNOWN;
    pCallInfo->m_isMParty = (pcallinfo->is_mpty == SIT_CALL_IS_MTPY_MULTIPARTY)?true:false;
    pCallInfo->m_isMt = (pcallinfo->is_mt == SIT_CALL_IS_MT_MT);

#ifdef SUPPORT_CDMA
    if( pcallinfo->call_type <= SIT_CALL_CALL_TYPE_VOICE
            || pcallinfo->call_type == SIT_CALL_CALL_TYPE_EMERGENCY
            || pcallinfo->call_type == SIT_CALL_CALL_TYPE_VOLTE
            || pcallinfo->call_type == SIT_CALL_CALL_TYPE_CDMA_VOICE
            || pcallinfo->call_type == SIT_CALL_CALL_TYPE_CDMA_EMERGENCY )
#else
    if( pcallinfo->call_type <= SIT_CALL_CALL_TYPE_VOICE
            || pcallinfo->call_type == SIT_CALL_CALL_TYPE_EMERGENCY
            || pcallinfo->call_type == SIT_CALL_CALL_TYPE_VOLTE )
#endif
    {
        pCallInfo->m_isVoice = true;
    }
    else
    {
        pCallInfo->m_isVoice = false;
    }
    pCallInfo->m_isVideo = (pcallinfo->call_type == SIT_CALL_CALL_TYPE_VIDEO)? true: false;
    if (pCallInfo->m_isVoice != true && pCallInfo->m_isVideo != true)
    {
            return -3;
    }
    pCallInfo->m_isVoicePrivacy = (pcallinfo->is_voice_privacy == SIT_CALL_IS_VOICE_PRIVACY_INACTIVATED) ? RIL_CALL_CDMA_VOICEPRIVACY_INACTIVE : RIL_CALL_CDMA_VOICEPRIVACY_ACTIVE;
    pCallInfo->m_numPresent = (CallPresentation)ConvertSitDefineToRilDefine(SIT_CALL_TB_IDX_PRESENTATION,pcallinfo->number_presentation);
    if (pCallInfo->m_numPresent != RIL_CALL_NAME_PRESENTATION_ALLOW
            && pCallInfo->m_numPresent != RIL_CALL_NAME_PRESENTATION_UNKNOWN)
    {
        pcallinfo->num_len = 0;
        memset(pCallInfo->m_number, 0, sizeof(pCallInfo->m_number));
    }
    pCallInfo->m_namePresent = (CallPresentation)ConvertSitDefineToRilDefine(SIT_CALL_TB_IDX_PRESENTATION,pcallinfo->name_presentation);
    if (pCallInfo->m_namePresent != RIL_CALL_NAME_PRESENTATION_ALLOW
            && pCallInfo->m_namePresent != RIL_CALL_NAME_PRESENTATION_UNKNOWN)
    {
        pcallinfo->name_len = 0;
        memset(pCallInfo->m_name, 0, sizeof(pCallInfo->m_name));
    }
    pCallInfo->m_als = pcallinfo->als;
    if (pcallinfo->num_len > 0)
    {
        memset(pCallInfo->m_number, 0, sizeof(pCallInfo->m_number));
        int len = MIN(pcallinfo->num_len, MAX_DIAL_NUM);
        memcpy(pCallInfo->m_number, pcallinfo->num, len);
        pCallInfo->m_numPresent = RIL_CALL_NAME_PRESENTATION_ALLOW;
    }
    if (pcallinfo->name_len > 0)
    {
        int decodedLen;
        char decodedName[MAX_DIAL_NAME];
        memset(pCallInfo->m_name, 0, sizeof(pCallInfo->m_name));
        decodedLen = DecodingUssd(pcallinfo->name_dcs, pcallinfo->name, pcallinfo->name_len, (unsigned char*)decodedName, sizeof(decodedName));
        RilLogV("[%s] decoded name : %s [dcs:%x, len:%d]", __FUNCTION__, decodedName, pcallinfo->name_dcs, decodedLen);
        int len = MIN(decodedLen, MAX_DIAL_NAME);
        memcpy(pCallInfo->m_name, decodedName, len);

        pCallInfo->m_namePresent = RIL_CALL_NAME_PRESENTATION_ALLOW;
    }

    ConvertSitToUusInfo(&(pCallInfo->m_uusInfo), pcallinfo->uus_type, pcallinfo->uus_dcs, pcallinfo->uus_data_len, pcallinfo->uus_data);

    return 0;
}

void ProtocolGetCurrentCallAdapter::DebugPrintCallInfo(CallInfo* pCallInfo)
{
    if ( pCallInfo == NULL )
    {
         return;
    }

    RilLogV("---------------------------\n");
    RilLogV("state : %d\n", pCallInfo->m_state);
    RilLogV("Index : %d\n", pCallInfo->m_nIndex);
    RilLogV("toa : %d\n", pCallInfo->m_toa);
    RilLogV("isMpty : %d\n", pCallInfo->m_isMParty);
    RilLogV("isMt : %d\n", pCallInfo->m_isMt);
    RilLogV("isVoice : %d\n", pCallInfo->m_isVoice);
    RilLogV("isVideo : %d\n", pCallInfo->m_isVideo);
    RilLogV("isVoicePrivacy : %d\n", pCallInfo->m_isVoicePrivacy);
    RilLogV("numPresent : %d\n", pCallInfo->m_numPresent);
    RilLogV("namePresent : %d\n", pCallInfo->m_namePresent);
    RilLogV("als : %d\n", pCallInfo->m_als);
    RilLogV("number : %s\n", pCallInfo->m_number);
    RilLogV("name : %s\n", pCallInfo->m_name);
    RilLogV("uusInfo(type:%d,dcs:%d,datalen:%d,[data])\n", pCallInfo->m_uusInfo.m_uusType, pCallInfo->m_uusInfo.m_uusDcs, pCallInfo->m_uusInfo.m_uusLength);
    RilLogV("---------------------------\n");
}

int ProtocolGetLastCallFailCauseAdapter::GetLastCallFailCause()
{
    sit_call_get_last_call_fail_cause_rsp *rsp = (sit_call_get_last_call_fail_cause_rsp *)m_pModemData->GetRawData();

    if ( rsp == NULL ) {
        RilLogV("invalid LastCallFailCause payload : return error_unspecified(%d)", CALL_FAIL_ERROR_UNSPECIFIED);
        return CALL_FAIL_ERROR_UNSPECIFIED;
    }
    switch(rsp->last_call_fail_cause) {
        case SIT_CALL_LAST_CALL_FAIL_UNOBTAINABLE_NUMBER:
            return CALL_FAIL_UNOBTAINABLE_NUMBER;
        case SIT_CALL_LAST_CALL_FAIL_NORMAL:
            return CALL_FAIL_NORMAL;
        case SIT_CALL_LAST_CALL_FAIL_BUSY:
            return CALL_FAIL_BUSY;
        case SIT_CALL_LAST_CALL_FAIL_NORMAL_UNSPECIFIED:
            return CALL_FAIL_NORMAL_UNSPECIFIED;
        case SIT_CALL_LAST_CALL_FAIL_CONGESTION:
            return CALL_FAIL_CONGESTION;
        case SIT_CALL_LAST_CALL_FAIL_ACM_LIMIT_EXCEEDED:
            return CALL_FAIL_ACM_LIMIT_EXCEEDED;
        case SIT_CALL_LAST_CALL_FAIL_CALL_BARRED:
            return CALL_FAIL_CALL_BARRED;
        case SIT_CALL_LAST_CALL_FAIL_FDN_BLOCKED:
            return CALL_FAIL_FDN_BLOCKED;
        case SIT_CALL_LAST_CALL_FAIL_IMSI_UNKNOWN_IN_VLR:
            return CALL_FAIL_IMSI_UNKNOWN_IN_VLR;
        case SIT_CALL_LAST_CALL_FAIL_IMEI_NOT_ACCEPTED:
            return CALL_FAIL_IMEI_NOT_ACCEPTED;

        case SIT_CALL_LAST_CALL_FAIL_CDMA_LOCKED_UNTIL_POWER_CYCLE:
            return CALL_FAIL_CDMA_LOCKED_UNTIL_POWER_CYCLE;
        case SIT_CALL_LAST_CALL_FAIL_CDMA_DROP:
            return CALL_FAIL_CDMA_DROP;
        case SIT_CALL_LAST_CALL_FAIL_CDMA_INTERCEPT:
            return CALL_FAIL_CDMA_INTERCEPT;
        case SIT_CALL_LAST_CALL_FAIL_CDMA_REORDER:
            return CALL_FAIL_CDMA_REORDER;
        case SIT_CALL_LAST_CALL_FAIL_CDMA_SO_REJECT:
            return CALL_FAIL_CDMA_SO_REJECT;
        case SIT_CALL_LAST_CALL_FAIL_CDMA_RETRY_ORDER:
            return CALL_FAIL_CDMA_RETRY_ORDER;
        case SIT_CALL_LAST_CALL_FAIL_CDMA_ACCESS_FAILURE:
            return CALL_FAIL_CDMA_ACCESS_FAILURE;
        case SIT_CALL_LAST_CALL_FAIL_CDMA_PREEMPTED:
            return CALL_FAIL_CDMA_PREEMPTED;
        case SIT_CALL_LAST_CALL_FAIL_CDMA_NOT_EMERGENCY:
            return CALL_FAIL_CDMA_NOT_EMERGENCY;
        case SIT_CALL_LAST_CALL_FAIL_CDMA_ACCESS_BLOCKED:
            return CALL_FAIL_CDMA_ACCESS_BLOCKED;

        case SIT_CALL_LAST_CALL_FAIL_ERROR_UNSPECIFIED:
            return CALL_FAIL_ERROR_UNSPECIFIED;
        default:
            RilLogW("[%s] cannot find matched ril constant value with sit value(%d), return SIT value instead", __FUNCTION__, rsp->last_call_fail_cause);
            return rsp->last_call_fail_cause;
    }
}

int ProtocolGetClipAdapter::GetClipStatus()
{
    sit_ss_get_clip_rsp *rsp = (sit_ss_get_clip_rsp*)m_pModemData->GetRawData();

    return ConvertSitDefineToRilDefine(SIT_CALL_TB_IDX_GET_CLIP_STATE, rsp->status);
}

int ProtocolGetClirAdapter::GetClirStatus()
{
    sit_ss_get_clir_rsp *rsp = (sit_ss_get_clir_rsp*)m_pModemData->GetRawData();

    return rsp->clir_status;    // same value defined in TS 27.007 7.7
}

int ProtocolGetCallForwardingStatusAdapter::GetCfNum()
{
    sit_ss_get_call_forward_status_rsp *rsp = (sit_ss_get_call_forward_status_rsp *)m_pModemData->GetRawData();
    if ( rsp != NULL )
    {
        return rsp->call_forward_num;
    }

    return 0;
}

bool ProtocolGetCallForwardingStatusAdapter::HasValidLength()
{
    sit_ss_get_call_forward_status_rsp *rsp = (sit_ss_get_call_forward_status_rsp *)m_pModemData->GetRawData();
    int packet_len = rsp->hdr.length;
    const int PACKET_MIN_LEN = sizeof(RCM_HEADER)+sizeof(rsp->call_forward_num);
    const int ONE_RECORD_LEN = sizeof(sit_ss_call_forward_item);

    if ( packet_len < PACKET_MIN_LEN )
    {
        RilLogE("ProtocolGetCallForwardingStatusAdapter::HasValidLength() : packet length(%d) is less than min value(%d)", packet_len, PACKET_MIN_LEN);
        return false;
    }
    else if ( (packet_len - PACKET_MIN_LEN) % ONE_RECORD_LEN != 0 )
    {
        RilLogE("ProtocolGetCallForwardingStatusAdapter::HasValidLength() : packet length(%d) is invalid", packet_len);
        return false;
    }
    return true;
}


int ProtocolGetCallForwardingStatusAdapter::GetCfInfo(RIL_CallForwardInfo* pCfInfo, int index)
{
    sit_ss_get_call_forward_status_rsp *rsp = (sit_ss_get_call_forward_status_rsp *)m_pModemData->GetRawData();

    if ( pCfInfo == NULL )
    {
        return -1;
    }
    else if ( index < 0 || index >= MAX_CALL_FORWARD_STATUS_NUM || index >= rsp->call_forward_num )
    {
        return -2;
    }

    char tNumber[MAX_SS_NUM_LEN];

    sit_ss_call_forward_item* p_cfinfo = &(rsp->record[index]);

    pCfInfo->status = ConvertSitDefineToRilDefine(SIT_CALL_TB_IDX_SERVICE_CLASS_CF_GET, p_cfinfo->status);
    pCfInfo->reason = p_cfinfo->reason;
    pCfInfo->serviceClass = p_cfinfo->service_class;
    pCfInfo->toa = p_cfinfo->toa;
    // TOA 0x91 means international number, add "+" to number.
    if (pCfInfo->toa == 0x91 && p_cfinfo->number != NULL) {
        // +2 is for "+" and terminating null character.
        int min = (p_cfinfo->num_len + 2) > MAX_SS_NUM_LEN ? MAX_SS_NUM_LEN : (p_cfinfo->num_len + 2);
        snprintf(tNumber, min, "+%s", (char*)p_cfinfo->number);
        strncpy((char*)p_cfinfo->number, tNumber, min);
    }
    pCfInfo->number = (char*)p_cfinfo->number;
    pCfInfo->timeSeconds = p_cfinfo->timeseconds;

    return 0;
}

void ProtocolGetCallForwardingStatusAdapter::DebugPrintCfInfo(RIL_CallForwardInfo* pCfInfo)
{
    if ( pCfInfo == NULL )
    {
         return;
    }

    RilLogV("---------------------------\n");
    RilLogV("state : %d\n", pCfInfo->status);
    RilLogV("reason : %d\n", pCfInfo->reason);
    RilLogV("serviceClass : %d\n", pCfInfo->serviceClass);
    RilLogV("toa : %d\n", pCfInfo->toa);
    RilLogV("number : %s\n", pCfInfo->number);
    RilLogV("timeSeconds : %d\n", pCfInfo->timeSeconds);
    RilLogV("---------------------------\n");
}

int ProtocolGetCallWaitingAdapter::GetServiceStatus()
{
    sit_ss_get_call_waiting_rsp *rsp = (sit_ss_get_call_waiting_rsp*)m_pModemData->GetRawData();

    return ConvertSitDefineToRilDefine(SIT_CALL_TB_IDX_SERVICE_STATUS_CALL_WAITING, rsp->status);
}

int ProtocolGetCallWaitingAdapter::GetServiceClass()
{
    sit_ss_get_call_waiting_rsp *rsp = (sit_ss_get_call_waiting_rsp*)m_pModemData->GetRawData();

    return rsp->service_class;
}

int ProtocolUssdIndAdapter::GetDecodedUssd(char* decodedUssd, size_t buf_size, int& dcs)
{
    sit_ss_ussd_ind *rsp = (sit_ss_ussd_ind*)m_pModemData->GetRawData();

    RilLogV("[%s] received ussd : %s(%d)", __FUNCTION__, rsp->ussd, rsp->ussd_len);
    if ( decodedUssd != NULL )
    {
        int decodedLen = DecodingUssd(rsp->dcs, rsp->ussd, rsp->ussd_len, (unsigned char*)decodedUssd, buf_size);
        dcs = rsp->dcs;
        RilLogV("[%s] decoded ussd : %s(%d) by dcs(%x)", __FUNCTION__, decodedUssd, decodedLen, dcs);
        return decodedLen;
        //return DecodeUssd(rsp->dcp, rsp->ussd_len, rsp->ussd, decodedUssd);
    }
    return 0;
}

int ProtocolUssdIndAdapter::GetUssdStatus()
{
    sit_ss_ussd_ind *rsp = (sit_ss_ussd_ind*)m_pModemData->GetRawData();

    return ConvertSitDefineToRilDefine(SIT_CALL_TB_IDX_USSD_STATUS, rsp->status);
}

int ProtocolSsSvcIndAdapter::GetNotificationType()
{
    sit_ss_supp_svc_notification_ind *rsp = (sit_ss_supp_svc_notification_ind*)m_pModemData->GetRawData();

    return ConvertSitDefineToRilDefine(SIT_CALL_TB_IDX_SSNOTI_TYPE, rsp->noti_type);
}

int ProtocolSsSvcIndAdapter::GetCode()
{
    sit_ss_supp_svc_notification_ind *rsp = (sit_ss_supp_svc_notification_ind*)m_pModemData->GetRawData();
    return rsp->code;
}

int ProtocolSsSvcIndAdapter::GetCugIndex()
{
    sit_ss_supp_svc_notification_ind *rsp = (sit_ss_supp_svc_notification_ind*)m_pModemData->GetRawData();
    return rsp->index;
}

UINT ProtocolSsSvcIndAdapter::GetSSType()
{
    sit_ss_supp_svc_notification_ind *rsp = (sit_ss_supp_svc_notification_ind*)m_pModemData->GetRawData();

    /* 0,  16, 32, 48, 64, (80, 96, 112)*/

/*
Type of number (octet 3) (Note 1)
Bits
7 6 5
0 0 0 unknown (Note 2)
0 0 1 international number (Note 3, Note 5)
0 1 0 national number (Note 3)
0 1 1 network specific number (Note 4)
1 0 0 dedicated access, short code
1 0 1 reserved
1 1 0 reserved
1 1 1 reserved for extension
*/
    return rsp->type;
}

int ProtocolSsSvcIndAdapter::GetNumberLength()
{
    sit_ss_supp_svc_notification_ind *rsp = (sit_ss_supp_svc_notification_ind*)m_pModemData->GetRawData();
    return rsp->num_len;
}

char* ProtocolSsSvcIndAdapter::GetNumber()
{
    sit_ss_supp_svc_notification_ind *rsp = (sit_ss_supp_svc_notification_ind*)m_pModemData->GetRawData();

    return (char*)rsp->num;
}

int ProtocolGetColpAdapter::GetColpStatus()
{
    sit_ss_get_colp_rsp *rsp = (sit_ss_get_colp_rsp*)m_pModemData->GetRawData();

    return rsp->status;
}

int ProtocolGetColrAdapter::GetColrStatus()
{
    sit_ss_get_colr_rsp *rsp = (sit_ss_get_colr_rsp*)m_pModemData->GetRawData();

    return rsp->status;
}

int ProtocolSetCallConfirmRespAdapter::GetResult() const
{
    sit_call_set_call_confirm_rsp *data = (sit_call_set_call_confirm_rsp *)m_pModemData->GetRawData();

    return data->result;
}

int ProtocolSendCallConfirmRespAdapter::GetResult() const
{
    sit_call_send_call_confirm_rsp *data = (sit_call_send_call_confirm_rsp *)m_pModemData->GetRawData();

    return data->result;
}

/**
 * ProtocolEmergencyCallListIndAdapter
 */

bool ProtocolEmergencyCallListIndAdapter::HasValidLength()
{
    sit_call_emergency_call_list_ind *data = (sit_call_emergency_call_list_ind *)m_pModemData->GetRawData();
    int packet_len = data->hdr.length;
    const int EMERGENCY_LIST_LEN = sizeof(sit_call_emergency_call_list_ind);

    if ( packet_len != EMERGENCY_LIST_LEN )
    {
        RilLogE("ProtocolEmergencyCallListIndAdapter::HasValidLength() : packet length(%d), list size is value(%d)", packet_len, EMERGENCY_LIST_LEN);
        return false;
    }
    return true;
}

const char* ProtocolEmergencyCallListIndAdapter::GetMcc()
{
    sit_call_emergency_call_list_ind *data = (sit_call_emergency_call_list_ind *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_IND_EMERGENCY_CALL_LIST) {
        memset(m_mcc, 0, sizeof(m_mcc));
        memcpy(m_mcc, data->mcc, MAX_MCC_LEN);
        return m_mcc;
    }
    return NULL;
}

const char* ProtocolEmergencyCallListIndAdapter::GetMnc()
{
    sit_call_emergency_call_list_ind *data = (sit_call_emergency_call_list_ind *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_IND_EMERGENCY_CALL_LIST) {
        memset(m_mnc, 0, sizeof(m_mnc));
        memcpy(m_mnc, data->mnc, MAX_MCC_LEN);
        if (m_mnc[MAX_MCC_LEN - 1] == '#') {
            m_mnc[MAX_MCC_LEN - 1] = 0;
        }
        return m_mnc;
    }
    return NULL;
}

int ProtocolEmergencyCallListIndAdapter::GetNum() const
{
    sit_call_emergency_call_list_ind *data = (sit_call_emergency_call_list_ind *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_IND_EMERGENCY_CALL_LIST) {
        return (int)(data->num);
    }
    return 0;
}

bool ProtocolEmergencyCallListIndAdapter::GetEmcInfo(EmcInfo &emcInfo, int idx) const
{
    if (idx < 0 || idx >= MAX_EMERGENCY_CALL_NUM) {
        return false;
    }

    sit_call_emergency_call_list_ind *data = (sit_call_emergency_call_list_ind *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_IND_EMERGENCY_CALL_LIST) {
        if (idx >= data->num) {
            return false;
        }

        sit_call_emergency_call_number_info *numInfo = &(data->number_info[idx]);
        if (numInfo != NULL) {
            emcInfo.m_category = (int)numInfo->category;
            emcInfo.m_number_len = (int)numInfo->emc_number_len;
            memset(emcInfo.m_number, 0, MAX_EMERGENCY_NUMBER_LEN); //32
            memcpy(emcInfo.m_number, numInfo->emc_number,
                numInfo->emc_number_len < MAX_EMERGENCY_NUMBER_LEN ? numInfo->emc_number_len : MAX_EMERGENCY_NUMBER_LEN);
            emcInfo.m_source = (int)numInfo->source;
            return true;
        }
    }
    return false;
}


#ifdef SUPPORT_CDMA
/**
 * ProtocolGetPreferredVoicePrivacyModeAdapter
 */
int ProtocolGetPreferredVoicePrivacyModeAdapter::GetPreferredVoicePrivacyStatus()
{
    sit_call_get_preferred_voice_privacy_mode_rsp *rsp = (sit_call_get_preferred_voice_privacy_mode_rsp*)m_pModemData->GetRawData();

    return rsp->status;
}

int ProtocolCdmaCallWaitingIndAdapter::GetCwInfo(RIL_CDMA_CallWaiting_v6 *pCwInfo)
{
    sit_ss_cdma_call_waiting_ind *data = (sit_ss_cdma_call_waiting_ind *)m_pModemData->GetRawData();

    if ( data == NULL )
    {
        return -1;
    }

    if ( pCwInfo == NULL )
    {
        return -1;
    }

    pCwInfo->numberPresentation = (CallPresentation)ConvertSitDefineToRilDefine(SIT_CALL_TB_IDX_PRESENTATION,
            data->number_presentation);
    if (pCwInfo->numberPresentation != RIL_CALL_NAME_PRESENTATION_ALLOW)
    {
        data->num_len = 0;
    }

    if (data->num_len > 0 )
    {
        pCwInfo->number = (char*)data->number;
        pCwInfo->numberPresentation = RIL_CALL_NAME_PRESENTATION_ALLOW;
    }

    if (data->name_len > 0 )
    {
        pCwInfo->name = (char*)data->name;
    }

    pCwInfo->signalInfoRecord.isPresent = SIGNAL_INFO_REC_PRESENT;
    pCwInfo->signalInfoRecord.signalType = data->signal_info.signal_type;
    pCwInfo->signalInfoRecord.alertPitch = data->signal_info.alert_pitch;
    pCwInfo->signalInfoRecord.signal = data->signal_info.signal;
    pCwInfo->number_type = data->number_type;
    pCwInfo->number_plan = data->number_plan;

    return 0;
}

/**
 * ProtocolCdmaInfoListIndAdapter
 */
int ProtocolCdmaInfoListIndAdapter::GetNumberOfInfoRecs() const
{
    sit_cdma_information_records *rsp = (sit_cdma_information_records *)m_pModemData->GetRawData();

    if(rsp == NULL)
    {
        return -1;
    }

    return rsp->num_of_info_recs;
}

int ProtocolCdmaInfoListIndAdapter::GetCdmaInfo(RIL_CDMA_InformationRecord &cdmaInfo, int index) const
{
    sit_cdma_information_records *rsp = (sit_cdma_information_records *)m_pModemData->GetRawData();

    if(rsp == NULL)
    {
        return -1;
    }

    cdmaInfo.name = (RIL_CDMA_InfoRecName)rsp->info_rec[index].cdma_info_name;
    switch(cdmaInfo.name) {
        case RIL_CDMA_DISPLAY_INFO_REC:
        case RIL_CDMA_EXTENDED_DISPLAY_INFO_REC:
            cdmaInfo.rec.display.alpha_len = rsp->info_rec[index].cdma_info.display.alpha_len;
            memcpy(cdmaInfo.rec.display.alpha_buf, rsp->info_rec[index].cdma_info.display.alpha_buf,
                    cdmaInfo.rec.display.alpha_len);
            break;
        case RIL_CDMA_CALLED_PARTY_NUMBER_INFO_REC:
        case RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC:
        case RIL_CDMA_CONNECTED_NUMBER_INFO_REC:
            cdmaInfo.rec.number.len = rsp->info_rec[index].cdma_info.number.len;
            memcpy(cdmaInfo.rec.number.buf, rsp->info_rec[index].cdma_info.number.buf, cdmaInfo.rec.number.len);
            cdmaInfo.rec.number.number_type = rsp->info_rec[index].cdma_info.number.number_type;
            cdmaInfo.rec.number.number_plan = rsp->info_rec[index].cdma_info.number.number_plan;
            cdmaInfo.rec.number.pi = rsp->info_rec[index].cdma_info.number.pi;
            cdmaInfo.rec.number.si = rsp->info_rec[index].cdma_info.number.si;
            break;
        case RIL_CDMA_SIGNAL_INFO_REC:
            cdmaInfo.rec.signal.isPresent = rsp->info_rec[index].cdma_info.signal.is_present;
            cdmaInfo.rec.signal.signalType = rsp->info_rec[index].cdma_info.signal.signal_type;
            cdmaInfo.rec.signal.alertPitch = rsp->info_rec[index].cdma_info.signal.alert_pitch;
            cdmaInfo.rec.signal.signal = rsp->info_rec[index].cdma_info.signal.signal;
            break;
        case RIL_CDMA_REDIRECTING_NUMBER_INFO_REC:
            cdmaInfo.rec.redir.redirectingNumber.len = rsp->info_rec[index].cdma_info.redirecting_number.redirecting_number.len;
            memcpy(cdmaInfo.rec.redir.redirectingNumber.buf, rsp->info_rec[index].cdma_info.redirecting_number.redirecting_number.buf,
                    cdmaInfo.rec.redir.redirectingNumber.len);
            cdmaInfo.rec.redir.redirectingNumber.number_type = rsp->info_rec[index].cdma_info.redirecting_number.redirecting_number.number_type;
            cdmaInfo.rec.redir.redirectingNumber.number_plan = rsp->info_rec[index].cdma_info.redirecting_number.redirecting_number.number_plan;
            cdmaInfo.rec.redir.redirectingNumber.pi = rsp->info_rec[index].cdma_info.redirecting_number.redirecting_number.pi;
            cdmaInfo.rec.redir.redirectingNumber.si = rsp->info_rec[index].cdma_info.redirecting_number.redirecting_number.si;
            cdmaInfo.rec.redir.redirectingReason = (RIL_CDMA_RedirectingReason)rsp->info_rec[index].cdma_info.redirecting_number.reason;
            break;
        case RIL_CDMA_LINE_CONTROL_INFO_REC:
            cdmaInfo.rec.lineCtrl.lineCtrlPolarityIncluded = rsp->info_rec[index].cdma_info.line_control.line_ctrl_polarity_included;
            cdmaInfo.rec.lineCtrl.lineCtrlToggle = rsp->info_rec[index].cdma_info.line_control.line_ctrl_toggle;
            cdmaInfo.rec.lineCtrl.lineCtrlReverse = rsp->info_rec[index].cdma_info.line_control.line_ctrl_reverse;
            cdmaInfo.rec.lineCtrl.lineCtrlPowerDenial = rsp->info_rec[index].cdma_info.line_control.line_ctrl_power_denial;
            break;
        default:
            return -1;
    }

    return 0;
}

/**
 * ProtocolCdmaOtaProvisionStatusIndAdapter
 */
int ProtocolCdmaOtaProvisionStatusIndAdapter::GetOtaProvisionStatus() const
{
    int provisionStatus = -1;
    sit_ota_provision_status_ind *ind = (sit_ota_provision_status_ind *)m_pModemData->GetRawData();

    if(ind == NULL)
    {
        return -1;
    }

    if (ind->otaType == SIT_OTA_TYPE_OTASP) {
        switch (ind->otaStatus)
        {
            case SIT_OTASP_STATUS_OK_SPL_UNLOCKED:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_SPL_UNLOCKED;
                break;
            case SIT_OTASP_STATUS_OK_AKEYEX:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_A_KEY_EXCHANGED;
                break;
            case SIT_OTASP_STATUS_OK_SSDUPDT:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_SSD_UPDATED;
                break;
            case SIT_OTASP_STATUS_OK_NAMDWNLD:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_NAM_DOWNLOADED;
                break;
            case SIT_OTASP_STATUS_OK_MDNDWNLD:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_MDN_DOWNLOADED;
                break;
            case SIT_OTASP_STATUS_OK_IMSIDWNLD:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_IMSI_DOWNLOADED;
                break;
            case SIT_OTASP_STATUS_OK_PRLDWNLD:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_PRL_DOWNLOADED;
                break;
            case SIT_OTASP_STATUS_OK_COMMIT:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_COMMITTED;
                break;
            case SIT_OTASP_STATUS_OK_PROGRAMMING:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_COMMITTED;
                break;
            case SIT_OTASP_STATUS_SUCCESSFUL:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_COMMITTED;
                break;
            case SIT_OTASP_STATUS_UNSUCCESSFUL:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_SPL_UNLOCKED;
                break;
            case SIT_OTASP_STATUS_OK_OTAPAVERIFY:
                //provisionStatus = -1;
                break;
            case SIT_OTASP_STATUS_PROGRESS:
                //provisionStatus = -1;
                break;
            case SIT_OTASP_STATUS_FAILURES_EXCESS_SPC:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_SPC_RETRIES_EXCEEDED;
                break;
            case SIT_OTASP_STATUS_LOCK_CODE_PASSWORD_SET:
                //provisionStatus = -1;
                break;
            default:
                break;
        }
    } else if (ind->otaType == SIT_OTA_TYPE_OTAPA) {
        switch (ind->otaStatus)
        {
            case SIT_OTAPA_STATUS_CALL_STOP_MODE:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_OTAPA_STOPPED;
                break;
            case SIT_OTAPA_STATUS_CALL_START_MODE:
                provisionStatus = CDMA_OTA_PROVISION_STATUS_OTAPA_STARTED;
                break;
            default:
                break;
        }
    }

    return provisionStatus;
}
#endif

/**
 * ProtocolEmergencySupportRatModeIndAdapter
 */
int ProtocolEmergencySupportRatModeIndAdapter::GetSupportRatMode() const
{
    sit_call_emergency_support_rat_mode_ind *data = (sit_call_emergency_support_rat_mode_ind *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_IND_EMERGENCY_SUPPORT_RAT_MODE) {
        int available_tech = (int)(data->available_tech);
        if (available_tech == SIT_SUPPORT_RAT_MODE_3GPP) {
            return SUPPORT_RAT_MODE_3GPP;
        } else if (available_tech == SIT_SUPPORT_RAT_MODE_3GPP2) {
            return SUPPORT_RAT_MODE_3GPP2;
        } else if (available_tech == SIT_SUPPORT_RAT_MODE_ALL) {
            return SUPPORT_RAT_MODE_ALL;
        }
        return available_tech;
    }
    return SUPPORT_RAT_MODE_3GPP;
}

/**
 * ProtocolExitEmergencyCbModeRespAdapter
 */
bool ProtocolExitEmergencyCbModeRespAdapter::GetResult() const
{
    sit_call_exit_emergency_cb_mode_rsp *data = (sit_call_exit_emergency_cb_mode_rsp *)m_pModemData->GetRawData();

    return (data->result == SIT_RESULT_SUCCESS);
}

RIL_Errno ProtocolExitEmergencyCbModeRespAdapter::GetRilErrorCode() const
{
    if (m_pModemData != NULL) {
        const RCM_HEADER *rcmdata = (RCM_HEADER *)m_pModemData->GetRawData();
        if (rcmdata != NULL && m_pModemData->GetLength() >= (int)sizeof(RCM_HEADER)) {
            if (rcmdata->type == RCM_TYPE_RESPONSE) {
                int errorCode = rcmdata->ext.rsp.error & 0xFF;
                if ( (int)RCM_E_SUCCESS <= errorCode && errorCode <= RIL_E_NO_SUCH_ELEMENT)
                    return (RIL_Errno)errorCode;
                else if ( (int)RCM_E_UNDEFINED_CMD == errorCode)
                    return (RIL_Errno)RIL_E_REQUEST_NOT_SUPPORTED;
                else if ( (int)RCM_E_NO_SUCH_ELEMENT < errorCode && errorCode < RCM_E_MAX) {
                    return (RIL_Errno)((errorCode - RCM_E_NO_SUCH_ELEMENT) + RIL_E_OEM_ERROR_1 - 1);
                }
            }
        }
    }
    // default error code
    return RIL_E_MODEM_ERR;
}
/**
 * ProtocolUnsolOnSSAdapter
 */
int ProtocolUnsolOnSSAdapter::GetServiceType() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_ss_on_ss_ind *data = (sit_ss_on_ss_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_ON_SS) {
            ret = data->service_type;
        }
    }
    return ret;
}

int ProtocolUnsolOnSSAdapter::GetRequestType() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_ss_on_ss_ind *data = (sit_ss_on_ss_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_ON_SS) {
            ret = data->request_type;
        }
    }
    return ret;
}

int ProtocolUnsolOnSSAdapter::GetTeleServiceType() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_ss_on_ss_ind *data = (sit_ss_on_ss_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_ON_SS) {
            ret = data->teleservice_type;
        }
    }
    return ret;
}

int ProtocolUnsolOnSSAdapter::GetServiceClass() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_ss_on_ss_ind *data = (sit_ss_on_ss_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_ON_SS) {
            ret = data->service_class;
        }
    }
    return ret;
}

int ProtocolUnsolOnSSAdapter::GetResult() const
{
    RilLogW("%s need to implement", __FUNCTION__);
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_ss_on_ss_ind *data = (sit_ss_on_ss_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_ON_SS) {
            // Need to do
            // type conversion is needed from RCM error to RIL error
            //ret = data->result;
            ret = RIL_E_REQUEST_NOT_SUPPORTED;
        }
    }
    return ret;
}

int ProtocolUnsolOnSSAdapter::GetDataType() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_ss_on_ss_ind *data = (sit_ss_on_ss_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_ON_SS) {
            ret = data->data_type;
        }
    }
    return ret;
}

bool ProtocolUnsolOnSSAdapter::GetData(void *pData) const
{
    RilLogW("%s need to implement", __FUNCTION__);
    bool ret = false;

    /*
    if (m_pModemData != NULL && pData != NULL) {
        sit_ss_on_ss_ind *data = (sit_ss_on_ss_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_ON_SS) {
            int dataType = data->data_type;
            int index = 0;
            if (dataType == SIT_ON_SS_IND_SS_INFO) {
                int *pSsInfo = (int *)pData;
                for (index = 0; index < MAX_SS_INFO_NUM; index++) {
                    *(pSsInfo + index) = data->data.ss_info[index];
                }
                ret = true;
            } else if (dataType == SIT_ON_SS_IND_CF_INFO) {
                RIL_CfData *pCfData = (RIL_CfData *)pData;
                pCfData->numValidIndexes = data->data.cf_info.call_forward_num;
                for (index = 0; index < MAX_SS_INFO_NUM; index++) {
                    // need to implement
                }
                ret = true;
            } else {
                // It shoud not happen
            }
        }
    }
    */
    return ret;
}
