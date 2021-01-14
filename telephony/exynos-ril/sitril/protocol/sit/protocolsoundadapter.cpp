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
 * protocolsoundadapter.cpp
 *
 *  Created on: 2014. 6. 28.
 *      Author: sungwoo48.choi
 */

#include "protocolsoundadapter.h"

/**
 * ProtocolSoundGetMuteRespAdapter
 */
int ProtocolSoundGetMuteRespAdapter::GetMuteState() const
{
    if (m_pModemData != NULL) {
        sit_snd_get_mute_rsp *data = (sit_snd_get_mute_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_MUTE) {
            return data->mute_state;
        }
    }
    return SIT_SND_MUTE_MODE_DISABLE;
}

/**
 * ProtocolSoundRingbackToneIndAdapter
 */
static RingbackToneType ConvertSITToRingbackToneType(int type)
{
    //RilLogV("[CscService] %s",__FUNCTION__);
    switch (type)
    {
    case SIT_SND_RINGBACK_STATE_STOP:        /* Ringback Tone End */
        return RIL_SND_RINGBACK_TONE_END;

    case SIT_SND_RINGBACK_STATE_START:   /* Ringback Tone Start */
    default:
        return RIL_SND_RINGBACK_TONE_START;
    }
}

int ProtocolSoundRingbackToneIndAdapter::GetRingbackToneState() const
{
    if (m_pModemData != NULL) {
        sit_snd_ringback_tone_ind *rawdata = (sit_snd_ringback_tone_ind *)m_pModemData->GetRawData();
        if (rawdata != NULL && rawdata->hdr.id == SIT_IND_RINGBACK_TONE) {
            return ConvertSITToRingbackToneType(rawdata->ringback_state);
        }
    }
    return RIL_SND_RINGBACK_TONE_END;
}

int ProtocolSoundRingbackToneIndAdapter::GetFlag() const
{
    if (m_pModemData != NULL) {
        sit_snd_ringback_tone_with_flag_ind *rawdata =
                (sit_snd_ringback_tone_with_flag_ind *)m_pModemData->GetRawData();
        if (rawdata != NULL && rawdata->hdr.id == SIT_IND_RINGBACK_TONE &&
            m_pModemData->GetLength() >= sizeof(sit_snd_ringback_tone_with_flag_ind)) {
            return rawdata->flag & 0xFF;
        }
    }
    return -1;
}

/**
 * ProtocolSoundGetVolumeRespAdapter
 */
int ProtocolSoundGetVolumeRespAdapter::GetVolume() const
{
    if (m_pModemData != NULL) {
        sit_snd_get_volume_rsp *data = (sit_snd_get_volume_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_VOLUME) {
            return data->volume;
        }
    }
    return -1;
}

/**
 * ProtocolSoundGetAudiopathRespAdapter
 */
int ProtocolSoundGetAudiopathRespAdapter::GetAudiopath() const
{
    if (m_pModemData != NULL) {
        sit_snd_get_audiopath_rsp *data = (sit_snd_get_audiopath_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_AUDIOPATH) {
            return data->audiopath;
        }
    }
    return -1;
}

/**
 * ProtocolSoundGetMultiMICRespAdapter
 */
int ProtocolSoundGetMultiMICRespAdapter::GetMultimicmode() const
{
    if (m_pModemData != NULL) {
        sit_snd_get_multimic_rsp *data = (sit_snd_get_multimic_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_AUDIOPATH) {
            return data->multiMICmode;
        }
    }
    return -1;
}

/**
 * ProtocolSoundWBAMRReportAdapter
 */
int ProtocolSoundWBAMRReportAdapter::GetStatus() const
{
    if (m_pModemData != NULL) {
        sit_snd_wb_amr_report_ind *data = (sit_snd_wb_amr_report_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_WB_AMR_REPORT) {
            return data->status & 0xFF;
        }
    }
    return -1;
}

int ProtocolSoundWBAMRReportAdapter::GetCallType() const
{
    int ret = AUDIO_CALL_TYPE_UNKNOWN;
    if (m_pModemData != NULL) {
        sit_snd_wb_amr_report_with_calltype_ind *data =
                (sit_snd_wb_amr_report_with_calltype_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_WB_AMR_REPORT &&
            m_pModemData->GetLength() >= sizeof(sit_snd_wb_amr_report_with_calltype_ind)) {
            // extended SIT_IND_WB_AMR_REPORT
            ret = data->call_type & 0xFF;
        }
    }
    return ret;
}

/**
 * ProtocolSoundGetWBAMRCapabilityAdapter
 */
int ProtocolSoundGetWBAMRCapabilityAdapter::GetWbAmr() const
{
    if (m_pModemData != NULL) {
        sit_snd_get_wbmar_capability_rsp *data = (sit_snd_get_wbmar_capability_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_WB_CAPABILITY) {
            return data->wbamr;
        }
    }
    return -1;
}
