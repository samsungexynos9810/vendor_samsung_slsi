/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "audioservice.h"
#include "servicemonitorrunnable.h"
#include "rillog.h"
#include "protocolmiscbuilder.h"
#include "protocolsoundadapter.h"
#include "protocolsoundbuilder.h"
#include "rildatabuilder.h"
#include "sounddatabuilder.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_SOUND, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_SOUND, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_SOUND, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_SOUND, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

// #### Definition for Debugging Logs ####
//#define ENABLE_LOGS_FUNC_ENTER_EXIT
#define ENABLE_ANDROID_LOG

#define LogE    RilLogE
#define LogW    RilLogW
#define LogN    RilLogI
#ifdef ENABLE_ANDROID_LOG
#define LogI    RilLogI
#define LogV    RilLogV
#endif // end  of ENABLE_ANDROID_LOG

// #### Internal Done Functions ####

INT32 AudioService::DoXXXDone(Message *pMsg)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Parsing Params from Modem
        ModemData *pModemData = pMsg->GetModemData();
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail GetModemData()",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        ProtocolRespAdapter adapter(pModemData);
        INT32 errorCode = adapter.GetErrorCode();
        if(errorCode != RIL_E_SUCCESS)
        {
            RilLogE("%s::%s() !! ERROR !!, errorCode(0x%x) in GetErrorCode()",m_szSvcName, __FUNCTION__,errorCode);

            // Complete Request for Error
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
            nRet = -3;
            break;
        }

        // Complete Request for Success
        OnRequestComplete( RIL_E_SUCCESS);

        RilLogV("%s::%s() Done ...Success",m_szSvcName, __FUNCTION__);
        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::DoGetVolumeDone(Message *pMsg)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Parsing Params from Modem
        ModemData *pModemData = pMsg->GetModemData();
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail GetModemData()",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        ProtocolSoundGetVolumeRespAdapter adapter(pModemData);
        INT32 errorCode = adapter.GetErrorCode();
        INT32 volume = adapter.GetVolume();

        if(errorCode != RIL_E_SUCCESS)
        {
            RilLogE("%s::%s() !! ERROR !!, errorCode(0x%x) in GetErrorCode()",m_szSvcName, __FUNCTION__,errorCode);

            // Complete Request for Error
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
            nRet = -3;
            break;
        }

        // Complete Request for Success
        OnRequestComplete( RIL_E_SUCCESS, &volume, sizeof(volume));

        RilLogV("%s::%s(volume:%d) Done ...Success",m_szSvcName, __FUNCTION__,volume);
        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::DoGetAudioPathDone(Message *pMsg)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Parsing Params from Modem
        ModemData *pModemData = pMsg->GetModemData();
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail GetModemData()",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        ProtocolSoundGetAudiopathRespAdapter adapter(pModemData);
        INT32 errorCode = adapter.GetErrorCode();
        INT32 audiopath = adapter.GetAudiopath();

        if(errorCode != RIL_E_SUCCESS)
        {
            RilLogE("%s::%s() !! ERROR !!, errorCode(0x%x) in GetErrorCode()",m_szSvcName, __FUNCTION__,errorCode);

            // Complete Request for Error
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
            nRet = -3;
            break;
        }

        // Complete Request for Success
        OnRequestComplete( RIL_E_SUCCESS, &audiopath, sizeof(audiopath));

        RilLogV("%s::%s(audiopath:%d) Done ...Success",m_szSvcName, __FUNCTION__,audiopath);
        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::DoGetMultiMicDone(Message *pMsg)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Parsing Params from Modem
        ModemData *pModemData = pMsg->GetModemData();
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail GetModemData()",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        ProtocolSoundGetMultiMICRespAdapter adapter(pModemData);
        INT32 errorCode = adapter.GetErrorCode();
        INT32 multiMICmode = adapter.GetMultimicmode();

        if(errorCode != RIL_E_SUCCESS)
        {
            RilLogE("%s::%s() !! ERROR !!, errorCode(0x%x) in GetErrorCode()",m_szSvcName, __FUNCTION__,errorCode);

            // Complete Request for Error
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
            nRet = -3;
            break;
        }

        // Complete Request for Success
        OnRequestComplete( RIL_E_SUCCESS, &multiMICmode, sizeof(multiMICmode));

        RilLogV("%s::%s(multiMICmode:%d) Done ...Success",m_szSvcName, __FUNCTION__,multiMICmode);
        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}


// #### Internal Set Functions ####

INT32 AudioService::DoSetVolume(Message *pMsg)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Parsing Params
        IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
        INT32 volume = rildata->GetInt();

        if ( volume == -1 )
        {
            RilLogV("%s::%s() volume(%d) is changed to minimum(0)",m_szSvcName, __FUNCTION__,volume);
            volume = 0;
        }

        RilLogV("%s::%s() volume(%d)",m_szSvcName, __FUNCTION__,volume);

        // Building SIT Command
        ProtocolSoundBuilder builder;
        ModemData *pModemData = builder.BuildSetVolume(volume);
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail BuildSetVolume()",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        // Send a SIT Command to CP.
        if (SendRequest(pModemData, AUDIO_DEFAULT_TIMEOUT, MSG_AUDIO_SET_VOLUME_DONE) < 0)
        {
            LogE("%s::%s() !! ERROR !!, Fail SendRequest(MSG_AUDIO_SET_VOLUME)",m_szSvcName, __FUNCTION__);
            nRet = -3;
            break;
        }

        RilLogV("%s::%s() MSG_AUDIO_SET_VOLUME(%d) is sent ...",m_szSvcName, __FUNCTION__,volume);

        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::DoSetAudioPath(Message *pMsg)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Parsing Params
        IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
        INT32 audiopath = rildata->GetInt();

        RilLogV("%s::%s() audiopath(%d)",m_szSvcName, __FUNCTION__,audiopath);

        // Building SIT Command
        ProtocolSoundBuilder builder;
        ModemData *pModemData = builder.BuildSetAudioPath(audiopath);
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail BuildSetAudioPath()",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        // Send a SIT Command to CP.
        if (SendRequest(pModemData, AUDIO_DEFAULT_TIMEOUT, MSG_AUDIO_SET_AUDIOPATH_DONE) < 0)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail SendRequest(MSG_AUDIO_SET_AUDIOPATH)",m_szSvcName, __FUNCTION__);
            nRet = -3;
            break;
        }

        RilLogV("%s::%s() MSG_AUDIO_SET_AUDIOPATH(audiopath:%d) is sent ...",m_szSvcName, __FUNCTION__,audiopath);

        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::DoSetMultiMic(Message *pMsg)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Parsing Params
        IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
        INT32 multiMICmode = rildata->GetInt();

        RilLogV("%s::%s() multiMICmode(%d)",m_szSvcName, __FUNCTION__,multiMICmode);

        // Building SIT Command
        ProtocolSoundBuilder builder;
        ModemData *pModemData = builder.BuildSetMultiMic(multiMICmode);
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail BuildSetMultiMic()",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        // Send a SIT Command to CP.
        if (SendRequest(pModemData, AUDIO_DEFAULT_TIMEOUT, MSG_AUDIO_SET_MULTIMIC_DONE) < 0)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail SendRequest(MSG_AUDIO_SET_MULTIMIC)",m_szSvcName, __FUNCTION__);
            nRet = -3;
            break;
        }

        RilLogV("%s::%s() MSG_AUDIO_SET_MULTIMIC(multiMICmode:%d) is sent ...",m_szSvcName, __FUNCTION__,multiMICmode);

        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::DoSetAudioClock(Message *pMsg)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Parsing Params
        IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
        INT32 clockmode = rildata->GetInt();

        RilLogV("%s::%s() clockmode(%d)",m_szSvcName, __FUNCTION__,clockmode);

        // Building SIT Command
        ProtocolSoundBuilder builder;
        ModemData *pModemData = builder.BuildSetAudioClock(clockmode);
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        // Send a SIT Command to CP.
        if (SendRequest(pModemData, AUDIO_DEFAULT_TIMEOUT, MSG_AUDIO_SET_AUDIO_CLOCK_DONE) < 0)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail SendRequest(MSG_AUDIO_SET_AUDIO_CLOCK)",m_szSvcName, __FUNCTION__);
            nRet = -3;
            break;
        }

        RilLogV("%s::%s() MSG_AUDIO_SET_AUDIO_CLOCK(clockmode:%d) is sent ...",m_szSvcName, __FUNCTION__,clockmode);

        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::DoSetAudioLoopback(Message *pMsg)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Parsing Params
        IntsRequestData *rildata = (IntsRequestData *)pMsg->GetRequestData();
        INT32 onoff = rildata->GetInt(0);
        INT32 path = rildata->GetInt(1);

        RilLogV("%s::%s() onoff(%d), path(%d)",m_szSvcName, __FUNCTION__,onoff, path);

        // Building SIT Command
        ProtocolSoundBuilder builder;
        ModemData *pModemData = builder.BuildSetAudioLoopback(onoff, path);
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        // Send a SIT Command to CP.
        if (SendRequest(pModemData, AUDIO_DEFAULT_TIMEOUT, MSG_AUDIO_SET_AUDIO_LOOPBACK_DONE) < 0)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail SendRequest(MSG_AUDIO_SET_AUDIO_LOOPBACK)",m_szSvcName, __FUNCTION__);
            nRet = -3;
            break;
        }

        RilLogV("%s::%s() MSG_AUDIO_SET_AUDIO_LOOPBACK(onoff:%d, path:%d) is sent ...",m_szSvcName, __FUNCTION__,onoff, path);

        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

// #### Internal Get Functions ####

INT32 AudioService::DoGetVolume(void)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s()++",m_szSvcName, __FUNCTION__);
#endif

    do
    {
        // Building SIT Command
        ProtocolSoundBuilder builder;
        ModemData *pModemData = builder.BuildGetVolume();
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail BuildGetVolume()",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        // Send a SIT Command to CP.
        if (SendRequest(pModemData, AUDIO_DEFAULT_TIMEOUT, MSG_AUDIO_GET_VOLUME_DONE) < 0)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail SendRequest(MSG_AUDIO_GET_VOLUME)",m_szSvcName, __FUNCTION__);
            nRet = -3;
            break;
        }

        RilLogV("%s::%s() MSG_AUDIO_GET_VOLUME is sent ...",m_szSvcName, __FUNCTION__);

        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::DoGetAudioPath(void)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s()++",m_szSvcName, __FUNCTION__);
#endif

    do
    {
        // Building SIT Command
        ProtocolSoundBuilder builder;
        ModemData *pModemData = builder.BuildGetAudioPath();
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail BuildGetAudioPath()",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        // Send a SIT Command to CP.
        if (SendRequest(pModemData, AUDIO_DEFAULT_TIMEOUT, MSG_AUDIO_GET_AUDIOPATH_DONE) < 0)
        {
            LogE("%s::%s() !! ERROR !!, Fail SendRequest(MSG_AUDIO_GET_AUDIOPATH)",m_szSvcName, __FUNCTION__);
            nRet = -3;
            break;
        }

        RilLogV("%s::%s() MSG_AUDIO_GET_AUDIOPATH is sent ...",m_szSvcName, __FUNCTION__);

        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::DoGetMultiMic(void)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s()++",m_szSvcName, __FUNCTION__);
#endif

    do
    {
        // Building SIT Command
        ProtocolSoundBuilder builder;
        ModemData *pModemData = builder.BuildGetMultiMic();
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail BuildGetMultiMic()",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        // Send a SIT Command to CP.
        if (SendRequest(pModemData, AUDIO_DEFAULT_TIMEOUT, MSG_AUDIO_GET_MULTIMIC_DONE) < 0)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail SendRequest(MSG_AUDIO_GET_MULTIMIC)",m_szSvcName, __FUNCTION__);
            nRet = -3;
            break;
        }

        RilLogV("%s::%s() MSG_AUDIO_GET_MULTIMIC is sent ...",m_szSvcName, __FUNCTION__);

        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::OnWBAMRReportNtf(Message *pMsg)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
    if(NULL == pMsg || NULL == pMsg->GetModemData())
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolSoundWBAMRReportAdapter adapter(pModemData);
    int status = adapter.GetStatus();
    int calltype = adapter.GetCallType();

    SoundDataBuilder builder;
    const RilData *rildata = builder.BuildWBAMRReportUnsolResponse(status, calltype);
    if (rildata != NULL) {
        OnUnsolicitedResponse(RIL_UNSOL_WB_AMR_REPORT_IND, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }

    return 0;
}

INT32 AudioService::DoSetWbAmrCapability(Message *pMsg)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Parsing Params
        IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
        if ( rildata == NULL )
        {
            RilLogE("%s::%s() !! ERROR !! Parsing Fail",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }
        INT32 capability = rildata->GetInt();

        RilLogV("%s::%s() capability(%d)",m_szSvcName, __FUNCTION__,capability);

        // Building SIT Command
        ProtocolSoundBuilder builder;
        ModemData *pModemData = builder.BuildSetWbAmrCapability(capability);
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !! Invalid parameter, Fail",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        // Send a SIT Command to CP.
        if (SendRequest(pModemData, AUDIO_DEFAULT_TIMEOUT, MSG_AUDIO_WB_AMR_SET_CAPABILITY_DONE) < 0)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail SendRequest(MSG_AUDIO_WB_AMR_SET_CAPABILITY)",m_szSvcName, __FUNCTION__);
            nRet = -3;
            break;
        }

        RilLogV("%s::%s() MSG_AUDIO_WB_AMR_SET_CAPABILITY(capability:%d) is sent ...",m_szSvcName, __FUNCTION__,capability);

        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::DoGetWbAmrCapability(void)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Building SIT Command
        ProtocolSoundBuilder builder;
        ModemData *pModemData = builder.BuildGetWbAmrCapability();
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        // Send a SIT Command to CP.
        if (SendRequest(pModemData, AUDIO_DEFAULT_TIMEOUT, MSG_AUDIO_WB_AMR_GET_CAPABILITY_DONE) < 0)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail SendRequest(MSG_AUDIO_WB_AMR_GET_CAPABILITY)",m_szSvcName, __FUNCTION__);
            nRet = -3;
            break;
        }

        RilLogV("%s::%s() MSG_AUDIO_WB_AMR_GET_CAPABILITY is sent ...",m_szSvcName, __FUNCTION__);

        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}

INT32 AudioService::DoGetWbAmrCapabilityDone(Message *pMsg)
{
    INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg=0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {
        // Parsing Params from Modem
        ModemData *pModemData = pMsg->GetModemData();
        if(pModemData == NULL)
        {
            RilLogE("%s::%s() !! ERROR !!, Fail GetModemData()",m_szSvcName, __FUNCTION__);
            nRet = -2;
            break;
        }

        ProtocolSoundGetWBAMRCapabilityAdapter adapter(pModemData);
        INT32 errorCode = adapter.GetErrorCode();
        INT32 wbamr = adapter.GetWbAmr();

        if(errorCode != RIL_E_SUCCESS)
        {
            RilLogE("%s::%s() !! ERROR !!, errorCode(0x%x) in GetErrorCode()",m_szSvcName, __FUNCTION__,errorCode);

            // Complete Request for Error
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
            nRet = -3;
            break;
        }

        // Complete Request for Success
        OnRequestComplete( RIL_E_SUCCESS, &wbamr, sizeof(wbamr));

        RilLogV("%s::%s(wbamr:%d) Done ...Success",m_szSvcName, __FUNCTION__,wbamr);
        nRet= 0;
    }while(0);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return nRet;
}


// #### Mandantory Functions ####

AudioService::AudioService(RilContext* pRilContext)
: Service(pRilContext, RIL_SERVICE_AUDIO)
{
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pRilContext=0x%p)++",m_szSvcName, __FUNCTION__,pRilContext);
#endif

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s()--",m_szSvcName, __FUNCTION__);
#endif

}

AudioService::~AudioService()
{
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s()++",m_szSvcName, __FUNCTION__);
#endif

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s()--",m_szSvcName, __FUNCTION__);
#endif
}

int AudioService::OnCreate(RilContext *pRilContext)
{
    //INT32 nRet = -1;
#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s()++",m_szSvcName, __FUNCTION__);
#endif

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
    return 0;
}

void AudioService::OnDestroy()
{
    INT32 nRet = -1;

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s()++",m_szSvcName, __FUNCTION__);
#endif

    do
    {

        nRet = 0;
    }while(FALSE);


#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif
}

BOOL AudioService::OnHandleRequest(Message *pMsg)
{
    INT32 nRet = -1;
    INT32 nMsgId = 0;

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg:%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

    do
    {

        if(NULL == pMsg)
        {
            LogE("%s::%s() !! ERROR !!, pMsg = NULL",m_szSvcName,__FUNCTION__);
            nRet = -2;
            break;
        }

        nMsgId = pMsg->GetMsgId();

        // Processing Received Message(pMsg)
        RilLogV("%s::%s() nMsgId=%d",m_szSvcName,__FUNCTION__,nMsgId);
        nRet = 0;
        switch (nMsgId)
        {
            case MSG_AUDIO_SET_VOLUME:
                nRet = DoSetVolume(pMsg);
                break;
            case MSG_AUDIO_GET_VOLUME:
                nRet = DoGetVolume();
                break;
            case MSG_AUDIO_SET_AUDIOPATH:
                nRet = DoSetAudioPath(pMsg);
                break;
            case MSG_AUDIO_GET_AUDIOPATH:
                nRet = DoGetAudioPath();
                break;
            case MSG_AUDIO_SET_MULTIMIC:
                nRet = DoSetMultiMic(pMsg);
                break;
            case MSG_AUDIO_GET_MULTIMIC:
                nRet = DoGetMultiMic();
                break;
            case MSG_AUDIO_SET_AUDIO_CLOCK:
                nRet = DoSetAudioClock(pMsg);
                break;
            case MSG_AUDIO_SET_AUDIO_LOOPBACK:
                nRet = DoSetAudioLoopback(pMsg);
                break;
            case MSG_AUDIO_WB_AMR_SET_CAPABILITY:
                nRet = DoSetWbAmrCapability(pMsg);
                break;
            case MSG_AUDIO_WB_AMR_GET_CAPABILITY:
                nRet = DoGetWbAmrCapability();
                break;
            case MSG_AUDIO_SET_TTY_MODE:
            case MSG_MISC_SET_TTY:
                nRet = DoSetTtyMode(pMsg);
                break;
            default:
                RilLogE("%s::%s() !! ERROR !!, Unkonwn nMsgId = %d",m_szSvcName,__FUNCTION__,nMsgId);
                nRet = -3;
                break;
        }
    }while(FALSE);


#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif

    if(0 == nRet)
        return TRUE;
    else
        return FALSE;

}

BOOL AudioService::OnHandleSolicitedResponse(Message *pMsg)
{
    INT32 nRet = -1;
    INT32 nMsgId = 0;

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg:%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

        do
        {
            if(NULL == pMsg)
            {
                RilLogE("%s::%s() !! ERROR !!, pMsg = NULL",m_szSvcName,__FUNCTION__);
                nRet = -2;
                break;
            }
            nMsgId = pMsg->GetMsgId();


            // Processing Received Message(pMsg)
            RilLogV("%s::%s() nMsgId=%d",m_szSvcName,__FUNCTION__,nMsgId);
            nRet = 0;
            switch (nMsgId)
            {
                case MSG_AUDIO_SET_VOLUME_DONE:
                case MSG_AUDIO_SET_AUDIOPATH_DONE:
                case MSG_AUDIO_SET_MULTIMIC_DONE:
                case MSG_AUDIO_SET_AUDIO_CLOCK_DONE:
                case MSG_AUDIO_SET_AUDIO_LOOPBACK_DONE:
                case MSG_AUDIO_WB_AMR_SET_CAPABILITY_DONE:
                    nRet = DoXXXDone(pMsg);
                    break;
                case MSG_AUDIO_GET_VOLUME_DONE:
                    nRet = DoGetVolumeDone(pMsg);
                    break;
                case MSG_AUDIO_GET_AUDIOPATH_DONE:
                    nRet = DoGetAudioPathDone(pMsg);
                    break;
                case MSG_AUDIO_GET_MULTIMIC_DONE:
                    nRet = DoGetMultiMicDone(pMsg);
                    break;
                case MSG_AUDIO_WB_AMR_GET_CAPABILITY_DONE:
                    nRet = DoGetWbAmrCapabilityDone(pMsg);
                    break;
                case MSG_AUDIO_SET_TTY_MODE_DONE:
                case MSG_MISC_SET_TTY_DONE:
                    nRet = DoSetTtyMode(pMsg);
                    break;
                default:
                    RilLogE("%s::%s() !! ERROR !!, Unkonwn nMsgId = %d",m_szSvcName,__FUNCTION__,nMsgId);
                    nRet = -3;
                    break;
            }
        }while(FALSE);


#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
        RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif

        if(0 == nRet)
            return TRUE;
        else
            return FALSE;

}

BOOL AudioService::OnHandleUnsolicitedResponse(Message *pMsg)
{
    INT32 nRet = -1;
    INT32 nMsgId = 0;

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg:%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif

        do
        {
            if(NULL == pMsg)
            {
                RilLogE("%s::%s() !! ERROR !!, pMsg = NULL",m_szSvcName,__FUNCTION__);
                nRet = -2;
                break;
            }
            nMsgId = pMsg->GetMsgId();

            RilLogV("%s::%s() nMsgId=%d",m_szSvcName,__FUNCTION__,nMsgId);
            nRet = 0;
            switch (nMsgId)
            {
                case MSG_AUDIO_WB_AMR_REPORT_NTF:
                    OnWBAMRReportNtf(pMsg);
                    break;
                default:
                    RilLogE("%s::%s() !! ERROR !!, Unkonwn nMsgId = %d",m_szSvcName,__FUNCTION__,nMsgId);
                    nRet = -3;
                    break;
            }
        }while(FALSE);


#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
        RilLogI("%s::%s(),nRet=%d.--",m_szSvcName, __FUNCTION__,nRet);
#endif

        if(0 == nRet)
            return TRUE;
        else
            return FALSE;

}

BOOL AudioService::OnHandleInternalMessage(Message* pMsg)
{
    INT32 nRet = -1;
    INT32 nMsgId = 0;
    //char *pData = NULL;

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(pMsg:0x%p)++",m_szSvcName, __FUNCTION__,pMsg);
#endif


    do
    {
        if(NULL == pMsg)
        {
            RilLogE("%s::%s() !! ERROR !!, pMsg = NULL",m_szSvcName,__FUNCTION__);
            nRet = -2;
            break;
        }

        nMsgId = pMsg->GetMsgId();
        //pData = pMsg->GetInternalData();

        RilLogV("%s::%s() nMsgId=%d",m_szSvcName,__FUNCTION__,nMsgId);
        nRet = 0;
        switch(nMsgId)
        {
            default:
                RilLogE("%s::%s() !! ERROR !!, Unkonwn nMsgId = %d",m_szSvcName,__FUNCTION__,nMsgId);
                break;
        }

    }while(FALSE);

#ifdef ENABLE_LOGS_FUNC_ENTER_EXIT
    RilLogI("%s::%s(),TRUE--",m_szSvcName, __FUNCTION__);
#endif

    return TRUE;
}

bool AudioService::IsPossibleToPassInRadioOffState(int request_id)
{
    switch (request_id) {
        case RIL_REQUEST_OEM_VOLUME_SET:
        case RIL_REQUEST_OEM_VOLUME_GET:
        case RIL_REQUEST_OEM_AUDIO_PATH_SET:
        case RIL_REQUEST_OEM_AUDIO_PATH_GET:
        case RIL_REQUEST_OEM_MICROPHONE_SET:
        case RIL_REQUEST_OEM_MICROPHONE_GET:
        case RIL_REQUEST_OEM_AUDIO_CLOCK_SET:
        case RIL_REQUEST_OEM_AUDIO_LOOPBACK_SET:
        case RIL_REQUEST_SET_TTY_MODE:
        case RIL_REQUEST_OEM_SET_TTY_MODE:
            break;
        default:
            return false;
    }
    return true;
}

int AudioService::DoSetTtyMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    if (rildata->GetReqId() == RIL_REQUEST_SET_TTY_MODE) {
        RilLogV("[%d] SetTtyMode request from frameworks. OnRequestComplete(RIL_E_SUCCESS)", GetRilSocketId());
        OnRequestComplete(RIL_E_SUCCESS);
        return 0;
    }
    else {
        RilLogV("[%d] SetTtyMode request from OEM", GetRilSocketId());
    }

    int ttyMode = rildata->GetInt();
    RilLogV("SetTtyMode request rildata->GetInt() is = %d", rildata->GetInt());
    RilLogV("SetTtyMode request is = %d", ttyMode);
    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.SetTtyMode(ttyMode);
    if (SendRequest(pModemData, AUDIO_DEFAULT_TIMEOUT, MSG_AUDIO_SET_TTY_MODE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int AudioService::OnSetTtyModeDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);

    return 0;
}
