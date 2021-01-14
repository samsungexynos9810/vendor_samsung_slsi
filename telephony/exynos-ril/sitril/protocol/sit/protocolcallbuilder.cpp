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
 * protocolcallbuilder.cpp
 *
 *  Created on: 2014. 6. 27.
 *      Author: jhdaniel.kim
 */
#include "protocolcallbuilder.h"
#include "protocolcalladapter.h"
#include "util.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CALL, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

extern sit_ril_match_table SitRilValueTable[];


//#define _CHANGE_UNKNOWN_SERVICE_CLASS_
//#define _USE_SERVICE_CLASS_DEFAULT_7_

/**
 * ProtocolCallBuilder
 */

 int ProtocolCallBuilder::ConvertRilDefineToSitDefine(int TableIndex, int RilValue)
{
    if ( TableIndex >= 0 && TableIndex < SIT_CALL_TB_IDX__MAX
        && SitRilValueTable[TableIndex].Index == TableIndex )
    {

        for ( int i = 0; i < MAX_MATCH_TABLE; i++ )
        {
            if ( SitRilValueTable[TableIndex].match_table[i].RilVal == RilValue )
            {
                return SitRilValueTable[TableIndex].match_table[i].SitVal;
            }
        }
        RilLogE("[%s] cannot find matched sit constant value with ril value(%d), return default value(%d)", __FUNCTION__, RilValue, SitRilValueTable[TableIndex].DefSitValue);
        return SitRilValueTable[TableIndex].DefSitValue;
    }
    RilLogE("[%s] cannot find matched table with index(%d)", __FUNCTION__, TableIndex);
    return -1;
}

ModemData *ProtocolCallBuilder::BuildGetCallList()
{
    null_data_format data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_GET_CURRENT_CALLS, length);
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildDial(char *number, ClirType clir, UusInfo &uusInfo/*not used*/, CallType cType, UINT eccType)
{
    sit_call_dial_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_DIAL, length);

    data.call_type = ConvertRilDefineToSitDefine(SIT_CALL_TB_IDX_TYPE,cType);
    data.emergency_call_subtype = eccType;    ///TODO: convert
    data.num_len = MIN(strlen(number), MAX_DIAL_NUM);
    memset(data.num, 0x00, sizeof(data.num));
    memcpy(data.num, number, data.num_len);
    if (strncmp(number, "+", 1) == 0) {
        data.num_type = SIT_CALL_NUMBER_TYPE_INTERNATIONAL;
    } else {
        data.num_type =SIT_CALL_NUMBER_TYPE_NATIONAL;
    }
    data.num_plan = SIT_CALL_NUMBER_PLAN_ISDN;
    data.clir = ConvertRilDefineToSitDefine(SIT_CALL_TB_IDX_CLIR, clir);
    data.cug_call = SIT_CALL_CUG_CALL_DISABLED;
    data.cug_info_index = 0;
    data.cug_info = SIT_CALL_CUG_INFO_NONE;

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildAnswer()
{
    null_data_format data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_ANSWER, length);
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildExplicitCallTransfer()
{
    null_data_format data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SEND_EXPLICIT_CALL_TRANSFER, length);
    return new ModemData((char *)&data, length);
}
ModemData *ProtocolCallBuilder::BuildHangup(int callId)
{
    sit_call_hangup_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_HANGUP, length);

    data.index = callId;
    data.call_type = SIT_CALL_HANGUP_CALL_TYPE_SINGLE;
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildHangupMulti(int callId)
{
    sit_call_hangup_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_HANGUP, length);

    data.index = callId;
    data.call_type = SIT_CALL_HANGUP_CALL_TYPE_MULTI;
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildLastCallFailCause()
{
    null_data_format data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_GET_LAST_CALL_FAIL_CAUSE, length);
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildDtmfStart(bool tone_type, BYTE digit)
{
    sit_call_dtmf_start_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_DTMF_START, length);

    data.tone_type = tone_type==true?SIT_CALL_LOCAL_DTMF_ON:SIT_CALL_LOCAL_DTMF_OFF;
    data.tone_len = SIT_CALL_DTMF_TONE_LENGTH_SHORT;
    data.dtmf_digit= digit;

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildDtmf(int dtmf_length, char* dtmf)
{
    sit_call_dtmf_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_DTMF, length);

    data.dtmf_len = dtmf_length>=MAX_DTMF_LEN?MAX_DTMF_LEN:dtmf_length;
    memcpy(data.dtmf_digit,dtmf, data.dtmf_len);

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildDtmfStop()
{
    sit_call_dtmf_stop_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_DTMF_STOP, length);
    return new ModemData((char *)&data, length);
}

#ifdef SUPPORT_CDMA
ModemData *ProtocolCallBuilder::BuildCdmaBurstDtmf(int dtmf_length, const char* dtmf,
        int on_len, const char* on_length, int off_len, const char* off_length)
{
    sit_call_cdma_burst_dtmf_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_CDMA_BURST_DTMF, length);

    memset(data.dtmf_digit, 0x00, MAX_BURST_DTMF_LEN);
    data.dtmf_len = ((dtmf_length>=MAX_BURST_DTMF_LEN) ? MAX_BURST_DTMF_LEN : dtmf_length);
    if ( dtmf != NULL )
    {
        memcpy(data.dtmf_digit, dtmf, data.dtmf_len);
    }

    memset(data.dtmf_on_length, 0x00, MAX_BURST_DTMF_ON_OFF_LEN);
    data.on_len = ((on_len>=MAX_BURST_DTMF_ON_OFF_LEN) ? MAX_BURST_DTMF_ON_OFF_LEN : on_len);
    if ( on_length != NULL )
    {
        memcpy(data.dtmf_on_length, on_length, data.on_len);
    }

    memset(data.dtmf_off_length, 0x00, MAX_BURST_DTMF_ON_OFF_LEN);
    data.off_len = ((off_len>=MAX_BURST_DTMF_ON_OFF_LEN) ? MAX_BURST_DTMF_ON_OFF_LEN : off_len);
    if ( off_length != NULL )
    {
        memcpy(data.dtmf_off_length, off_length, data.off_len);
    }

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildCdmaSetPreferredVoicePrivacyMode(int vpMode)
{
    sit_call_set_cdma_voice_privacy_mode_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SET_CDMA_VOICE_PRIVACY_MODE, length);

    data.vpMode = vpMode;
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildCdmaQueryPreferredVoicePrivacyMode()
{
    sit_call_get_preferred_voice_privacy_mode_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_GET_CDMA_VOICE_PRIVACY_MODE, length);
    return new ModemData((char *)&data, length);
}
#endif

ModemData *ProtocolCallBuilder::BuildHangupWaitingOrBackground()
{
    sit_ss_hangup_waiting_or_background_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_HANGUP_WAITING_OR_BACKGROUND, length);
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildHangupForegroundResumeBackground()
{
    sit_ss_hangup_foreground_resume_background_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_HANGUP_FORGROUND_RESUME_BACKGROUND, length);
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildSwitchWaitingOrHoldingAndActive()
{
    sit_ss_switch_waiting_or_holding_and_active_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE, length);
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildConference()
{
    sit_ss_conference_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_CONFERENCE, length);
    return new ModemData((char *)&data, length);
}

#ifdef SUPPORT_CDMA
ModemData *ProtocolCallBuilder::BuildCdmaFlash(int flash_length, const char* flash)
{
    sit_ss_set_cdma_flash_info_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SET_CDMA_FLASH_INFO, length);

    memset(data.flash, 0x00, sizeof(data.flash));
    data.flash_len = flash_length>=MAX_FLASH_LEN?MAX_FLASH_LEN:flash_length;
    if ( flash != NULL )
    {
        memcpy(data.flash, flash, data.flash_len);
    }
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildCdmaFlash(const char* flash)
{
    sit_ss_set_cdma_flash_info_req data;
    memset(&data, 0, sizeof(data));
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SET_CDMA_FLASH_INFO, length);

    if ( flash != NULL )
    {
        int flash_length = strlen(flash);
        if (flash_length > MAX_FLASH_LEN) {
            flash_length = MAX_FLASH_LEN;
        }

        data.flash_len = flash_length;
        memcpy(data.flash, flash, flash_length);
    }
    return new ModemData((char *)&data, length);
}

#endif

ModemData *ProtocolCallBuilder::BuildUdub()
{
    sit_ss_udub_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_UDUB, length);
    return new ModemData((char *)&data, length);
}

/*ModemData *ProtocolCallBuilder::BuildSendUssd()
{
    sit_ss_send_ussd_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SEND_USSD, length);


    return new ModemData((char *)&data, length);
}*/

ModemData *ProtocolCallBuilder::BuildGetClip()
{
    sit_ss_get_clip_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_GET_CLIP, length);
    return new ModemData((char *)&data, length);
}

#if 0    //removed by discussion with CP, SET CLIR is implemented by AP (Vendor RIL) inside.
ModemData *ProtocolCallBuilder::BuildSetClir(int aoc)
{
    sit_ss_set_clir_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SET_CLIR, length);

    data.aoc = ConvertRilDefineToSitDefine(SIT_CALL_TB_IDX_CLIR, aoc);

    return new ModemData((char *)&data, length);
}
#endif

ModemData *ProtocolCallBuilder::BuildGetClir()
{
    sit_ss_get_clir_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_GET_CLIR, length);

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildSetCallForwarding(SsModeType status, SsCfReason reason, SsClassX service_class,
    int toa, char* number, int timeseconds)
{
    sit_ss_set_call_forward_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SET_CALL_FORWARD, length);

    data.status = ConvertRilDefineToSitDefine(SIT_CALL_TB_IDX_SERVICE_CLASS_CF_SET, status);
    data.reason = (INT32)reason;    // same reason value defined in TS 27.007 7.11 "reason"
#ifdef _CHANGE_UNKNOWN_SERVICE_CLASS_
    if ( service_class == RIL_SS_CLASS_UNKNOWN )    /* 0 */
    {
#ifdef _USE_SERVICE_CLASS_DEFAULT_7_
        service_class = RIL_SS_CLASS_DEFAULT;    /*7 <- defined in <classx> of TS 27.007 7.11*/
#else
        service_class = RIL_SS_CLASS_VOICE;     /*1*/
#endif
        RilLogE("RIL changes unknown service class(0) to [%d]", service_class);
    }
#endif
    data.service_class = (INT32)service_class;    //same class value defined in TS 27.007 "class"
    data.toa = toa;    //same toa /* "type" from TS 27.007 7.11 */    /*TS24.008 table 10.5.118*/
    data.num_len = strlen(number);
    memset(data.number, 0x00, sizeof(data.number));
    memcpy(data.number, number, data.num_len);
    if ( 0 < timeseconds  &&  timeseconds <= 30 )    // timeseconds : 1 ~ 30
    {
        data.timeseconds = timeseconds;
    }
    else
    {
        data.timeseconds = 20;    //default
    }

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildGetCallForwardingStatus(SsStatusType status, SsCfReason reason, SsClassX service_class,
    int toa, char* number, int timeseconds)
{
    sit_ss_get_call_forward_status_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_GET_CALL_FORWARD_STATUS, length);

    data.status = SIT_SS_CALL_FARWARD_STATUS_INTERROGATE; //(BYTE)ConvertRilDefineToSitDefine(SIT_CALL_TB_IDX_SERVICE_CLASS_CF_GET, status);
    data.reason = (INT32)reason;    // same reason value defined in TS 27.007 7.11 "reason"

#ifdef _CHANGE_UNKNOWN_SERVICE_CLASS_
    if ( service_class == RIL_SS_CLASS_UNKNOWN )    /* 0 */
    {
#ifdef _USE_SERVICE_CLASS_DEFAULT_7_
        service_class = RIL_SS_CLASS_DEFAULT;    /*7 <- defined in <classx> of TS 27.007 7.11*/
#else
        service_class = RIL_SS_CLASS_VOICE;     /*1*/
#endif
        RilLogE("RIL changes unknown service class(0) to [%d]", service_class);
    }
#endif
    data.service_class = (INT32)service_class;    //same class value defined in TS 27.007 "class"
    data.toa = toa;    //same toa /* "type" from TS 27.007 7.11 */    /*TS24.008 table 10.5.118*/
    data.num_len = strlen(number);
    memset(data.number, 0x00, sizeof(data.number));
    memcpy(data.number, number, data.num_len);
    if ( 0 < timeseconds  &&  timeseconds <= 30 )    // timeseconds : 1 ~ 30
    {
        data.timeseconds = timeseconds;
    }
    else
    {
        data.timeseconds = 20;    //default
    }

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildSetCallWaiting(SsModeType status, SsClassX service_class)
{
    sit_ss_set_call_waiting_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SET_CALL_WAITING, length);

    data.status = ConvertRilDefineToSitDefine(SIT_CALL_TB_IDX_SERVICE_STATUS_CALL_WAITING, status);
#ifdef _CHANGE_UNKNOWN_SERVICE_CLASS_
    if ( service_class == RIL_SS_CLASS_UNKNOWN )    /* 0 */
    {
#ifdef _USE_SERVICE_CLASS_DEFAULT_7_
        service_class = RIL_SS_CLASS_DEFAULT;    /*7 <- defined in <classx> of TS 27.007 7.11*/
#else
        service_class = RIL_SS_CLASS_VOICE;     /*1*/
#endif
        RilLogE("RIL changes unknown service class(0) to [%d]", service_class);
    }
#endif
    data.service_class = (INT32)service_class;    //same class value defined in TS 27.007 "class"

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildGetCallWaiting(SsClassX service_class)
{
    sit_ss_get_call_waiting_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_GET_CALL_WAITING, length);

#ifdef _CHANGE_UNKNOWN_SERVICE_CLASS_
    if ( service_class == RIL_SS_CLASS_UNKNOWN )    /* 0 */
    {
#ifdef _USE_SERVICE_CLASS_DEFAULT_7_
        service_class = RIL_SS_CLASS_DEFAULT;    /*7 <- defined in <classx> of TS 27.007 7.11*/
#else
        service_class = RIL_SS_CLASS_VOICE;     /*1*/
#endif
        RilLogE("RIL changes unknown service class(0) to [%d]", service_class);
    }
#endif
    data.service_class = (INT32)service_class;    //same class value defined in TS 27.007 "class"

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildChangeBarringPwd(const char* oldpasswd, const char* newpasswd, const char* newpasswd_again)
{
    sit_ss_change_barring_pwd_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_CHG_BARRING_PWD, length);

    if ( oldpasswd != NULL && strlen(oldpasswd) != 0 )
    {
        memcpy(data.pwd, oldpasswd, sizeof(data.pwd));
    }
    if ( newpasswd != NULL && strlen(newpasswd) != 0 )
    {
        memcpy(data.new_pwd, newpasswd, sizeof(data.new_pwd));
    }
    if ( newpasswd_again != NULL && strlen(newpasswd_again) != 0 )
    {
        memcpy(data.new_pwd_again, newpasswd_again, sizeof(data.new_pwd_again));
    }
    return new ModemData((char *)&data, length);
}

static void PrintHexBytes(const char* pTitle, unsigned char* data, unsigned int len)
{
    char szBuffer[256] = {0,};
    char szTemp[4];

    snprintf(szBuffer, sizeof(szBuffer)-1, "(%d) : ", len);

    for  ( unsigned int i = 0; i < len; i++ )
    {
        if ( i > 64 )
        {
            SECURELIB::strncat(szBuffer, sizeof(szBuffer), "...", 3);
            break;
        }
        snprintf(szTemp, sizeof(szTemp)-1, "%02x ", data[i]);
        SECURELIB::strncat(szBuffer, sizeof(szBuffer), szTemp, sizeof(szTemp));

        if ( i %16 == 15 )
        {
            SECURELIB::strncat(szBuffer, sizeof(szBuffer), "\n", 1);
        }
    }
    RilLogV("%s : \n%s", pTitle, szBuffer);
}

ModemData *ProtocolCallBuilder::BuildSendUssd(const char* ussd, bool user_initiated)
{
    sit_ss_send_ussd_req data;
    memset(&data, 0x00, sizeof(data));
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SEND_USSD, length);

    RilLogV("[%s] user initiated : %d", __FUNCTION__, user_initiated);
    RilLogV("[%s] org ussd : %s(%d)", __FUNCTION__, ussd, strlen(ussd));
    unsigned char encodedUssd[MAX_USSD_DATA_LEN];
    memset(encodedUssd, 0x00, sizeof(encodedUssd));
    // convert different code value between ascii table <-> gsm 7bit basic character set
    unsigned int gsmlen;
    unsigned char Gsm7[MAX_USSD_DATA_LEN];
    memset(Gsm7, 0x00, sizeof(Gsm7));
    gsmlen = convertAsciiToGsm7bitBasicCharSet((unsigned char*)ussd, strlen(ussd), Gsm7);

    int encodedLen = packing_gsm7bit(Gsm7, gsmlen, encodedUssd);
    encodedLen = MIN(encodedLen, MAX_USSD_DATA_LEN);
    //RilLogV("[%s] engoded ussd : %s(%d)", __FUNCTION__, encodedUssd, encodedLen);
    PrintHexBytes("encoded ussd", encodedUssd, encodedLen);

    data.ussd_type = user_initiated==true?SIT_SS_USSD_USER_INITIATED:SIT_SS_USSD_USER_RESPONSE;
    data.dcs = 0b00001111;    // GSM 7 bit default alphabet - language unspecified
    data.ussd_len = encodedLen;
    memcpy(data.ussd, encodedUssd, encodedLen);

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildCancelUssd()
{
    sit_ss_cancel_ussd_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_CANCEL_USSD, length);

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildSeparateConnection(int callId)
{
    sit_ss_separate_connection_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SEPARATE_CONNECTION, length);

    data.index = callId;

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildQueryColp()
{
    sit_ss_get_colp_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_GET_COLP, length);
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildQueryColr()
{
    sit_ss_get_colr_req data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_GET_COLR, length);
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildSendEncodedUssd(BYTE dcs, const char* encodedUssd, bool user_initiated)
{
    sit_ss_send_ussd_req data;
    memset(&data, 0x00, sizeof(data));
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_SEND_USSD, length);

    RilLogV("[%s] user initiated : %d", __FUNCTION__, user_initiated);
    RilLogV("[%s] dcs : %d(0x%02X)", __FUNCTION__, dcs, dcs);
    RilLogV("[%s] encode ussd : %s(%d)", __FUNCTION__, encodedUssd, strlen(encodedUssd));

    unsigned char ussdData[MAX_USSD_DATA_LEN];
    int ussdLen = HexString2Value(ussdData, encodedUssd);
    //PrintHexBytes("encoded ussd", encodedUssd, encodedLen);

    data.ussd_type = user_initiated==true?SIT_SS_USSD_USER_INITIATED:SIT_SS_USSD_USER_RESPONSE;
    data.dcs = dcs;
    data.ussd_len = ussdLen;
    memcpy(data.ussd, ussdData, ussdLen);

    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildSetCallConfirm(int mode)
{
    sit_call_set_call_confirm_req req;
    int length = sizeof(req);
    RilLogV("[frank] [%s] length=%d, mode=%d", __FUNCTION__, length, mode);
    InitRequestHeader(&req.hdr, SIT_CALL_CONFIRM_FEATURE_SET_REQ, length);
    req.enable = (BYTE)mode;
    return new ModemData((char *)&req, length);
}
ModemData *ProtocolCallBuilder::BuildSendCallConfirm()
{
    null_data_format data;
    int length = sizeof(data);
    InitRequestHeader(&data.hdr, SIT_CALL_CONFIRM, length);
    return new ModemData((char *)&data, length);
}

ModemData *ProtocolCallBuilder::BuildExitEmergencyCbMode()
{
    sit_call_exit_emergency_cb_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_EXIT_EMERGENCY_CB_MODE, length);
    return new ModemData((char *)&req, length);
}
