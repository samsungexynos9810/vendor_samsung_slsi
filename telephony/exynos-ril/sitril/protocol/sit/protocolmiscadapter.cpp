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
 * protocolmiscadapter.cpp
 *
 *  Created on: 2014. 6. 30.
 *      Author: m.afzal
 */
#include "protocolmiscadapter.h"
#include "rildef.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

int ProtocolMiscVersionAdapter::GetMask() const
{
    int mask = 0;
    if (m_pModemData != NULL) {
        sit_misc_get_baseband_version_rsp *data = (sit_misc_get_baseband_version_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BASEBAND_VERSION) {
            mask = data->ver_mask;
        }
    }
    return mask;
}

const char * ProtocolMiscVersionAdapter::GetSwVer() const
{
    char *swver = NULL;
    if (m_pModemData != NULL) {
        sit_misc_get_baseband_version_rsp *data = (sit_misc_get_baseband_version_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BASEBAND_VERSION) {
            swver = data->sw_version;
            //strncpy(swver, data->sw_version, 32);
        }
    }
    return swver;
}

const char * ProtocolMiscVersionAdapter::GetHwVer() const
{
    char *hwver = NULL;
    if (m_pModemData != NULL) {
        sit_misc_get_baseband_version_rsp *data = (sit_misc_get_baseband_version_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BASEBAND_VERSION) {
            hwver = data->hw_version;
        }
    }
    return hwver;
}

const char * ProtocolMiscVersionAdapter::GetRfCalDate() const
{
    char *rfcaldate = NULL;
    if (m_pModemData != NULL) {
        sit_misc_get_baseband_version_rsp *data = (sit_misc_get_baseband_version_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BASEBAND_VERSION) {
            rfcaldate = data->rf_cal_date;
        }
    }
    return rfcaldate;
}

const char * ProtocolMiscVersionAdapter::GetProdCode() const
{
    char *prodcode = NULL;
    if (m_pModemData != NULL) {
        sit_misc_get_baseband_version_rsp *data = (sit_misc_get_baseband_version_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BASEBAND_VERSION) {
            prodcode = data->product_code;
        }
    }
    return prodcode;
}

const char * ProtocolMiscVersionAdapter::GetModelID() const
{
    char *modelid = NULL;
    if (m_pModemData != NULL) {
        sit_misc_get_baseband_version_rsp *data = (sit_misc_get_baseband_version_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BASEBAND_VERSION) {
            modelid = data->model_id;
        }
    }
    return modelid;
}

int ProtocolMiscVersionAdapter::GetPrlNamNum() const
{
    int prlnam = 0;
    if (m_pModemData != NULL) {
        sit_misc_get_baseband_version_rsp *data = (sit_misc_get_baseband_version_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BASEBAND_VERSION) {
            prlnam = data->prl_nam_num;
        }
    }
    return prlnam;
}

const BYTE * ProtocolMiscVersionAdapter::GetPrlVersion() const
{
    BYTE *prlver = NULL;
    if (m_pModemData != NULL) {
        sit_misc_get_baseband_version_rsp *data = (sit_misc_get_baseband_version_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BASEBAND_VERSION) {
            prlver = data->prl_version;
        }
    }
    return prlver;
}

int ProtocolMiscVersionAdapter::GetEriNamNum() const
{
    int erinam = 0;
    if (m_pModemData != NULL) {
        sit_misc_get_baseband_version_rsp *data = (sit_misc_get_baseband_version_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BASEBAND_VERSION) {
            erinam = data->eri_nam_num;
        }
    }
    return erinam;
}

const BYTE * ProtocolMiscVersionAdapter::GetEriVersion() const
{
    BYTE *eriver = NULL;
    if (m_pModemData != NULL) {
        sit_misc_get_baseband_version_rsp *data = (sit_misc_get_baseband_version_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BASEBAND_VERSION) {
            eriver = data->eri_version;
        }
    }
    return eriver;
}

const BYTE * ProtocolMiscVersionAdapter::GetCPChipSet() const
{
    BYTE *cpchip = NULL;
    if (m_pModemData != NULL) {
        sit_misc_get_baseband_version_rsp *data = (sit_misc_get_baseband_version_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BASEBAND_VERSION) {
            cpchip = data->cp_chipsetname;
        }
    }
    return cpchip;
}

int ProtocolMiscGetTtyAdapter::GetTtyMode() const
{
    int ttymode = SIT_MISC_TTY_MODE_OFF;
    if (m_pModemData != NULL) {
        sit_misc_get_tty_mode_rsp *data = (sit_misc_get_tty_mode_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_TTY_MODE) {
            ttymode = data->tty_mode;
        }
    }
    return ttymode;
}

// signal strength common
static void FillSignalStrength(RIL_SignalStrength_V1_4& out, void *data, unsigned int size, int mask)
{
    if (data == NULL || size < sizeof(SIGNAL_STRENGTH)) {
        mask = 0;
    }

    if (size < sizeof(SIGNAL_STRENGTH_V1_4)) {

        SIGNAL_STRENGTH *currentSignalStrength = (SIGNAL_STRENGTH *)data;
//        bool umtsFlag = (mask != 0) &&
//                        !(currentSignalStrength->UMTS_SignalStrength.ecno < 0 ||
//                          currentSignalStrength->UMTS_SignalStrength.ecno > 49);

        if ((mask & SIT_MISC_SIG_RAT_SIG_GW)/* && !umtsFlag*/) {
            out.GSM_SignalStrength.signalStrength =
                    currentSignalStrength->GW_SignalStrength.sig_str;
            out.GSM_SignalStrength.bitErrorRate =
                    currentSignalStrength->GW_SignalStrength.ber;
            out.GSM_SignalStrength.timingAdvance = INT_MAX;
        }
        else {
            out.GSM_SignalStrength.signalStrength = 99;
            out.GSM_SignalStrength.bitErrorRate = 99;
            out.GSM_SignalStrength.timingAdvance = INT_MAX;
        }

        if ((mask & SIT_MISC_SIG_RAT_SIG_GW)/* && umtsFlag*/) {
            out.WCDMA_SignalStrength.signalStrength =
                    currentSignalStrength->GW_SignalStrength.sig_str;
            out.WCDMA_SignalStrength.bitErrorRate =
                    currentSignalStrength->GW_SignalStrength.ber;
            out.WCDMA_SignalStrength.rscp = 255;
            out.WCDMA_SignalStrength.ecno = 255;
        }
        else {
            out.WCDMA_SignalStrength.signalStrength = 99;
            out.WCDMA_SignalStrength.bitErrorRate = 99;
            out.WCDMA_SignalStrength.rscp = 255;
            out.WCDMA_SignalStrength.ecno = 255;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_CDMA) {
            out.CDMA_SignalStrength.dbm = currentSignalStrength->CDMA_SignalStrength.dbm;
            out.CDMA_SignalStrength.ecio = currentSignalStrength->CDMA_SignalStrength.ecio;
        }
        else {
            out.CDMA_SignalStrength.dbm = -1;
            out.CDMA_SignalStrength.ecio = -1;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_EVDO) {
            out.EVDO_SignalStrength.dbm = currentSignalStrength->EVDO_SignalStrength.dbm;
            out.EVDO_SignalStrength.ecio = currentSignalStrength->EVDO_SignalStrength.ecio;
            out.EVDO_SignalStrength.signalNoiseRatio =
                    currentSignalStrength->EVDO_SignalStrength.snr;
        }
        else {
            out.EVDO_SignalStrength.dbm = -1;
            out.EVDO_SignalStrength.ecio = -1;
            out.EVDO_SignalStrength.signalNoiseRatio = INT_MAX;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_LTE) {
            out.LTE_SignalStrength.signalStrength = currentSignalStrength->LTE_SignalStrength.sig_str;
            out.LTE_SignalStrength.rsrp = currentSignalStrength->LTE_SignalStrength.rsrp;
            out.LTE_SignalStrength.rsrq = currentSignalStrength->LTE_SignalStrength.rsrq;
            out.LTE_SignalStrength.rssnr = currentSignalStrength->LTE_SignalStrength.rssnr;
            out.LTE_SignalStrength.cqi = currentSignalStrength->LTE_SignalStrength.cqi;
            out.LTE_SignalStrength.timingAdvance = currentSignalStrength->LTE_SignalStrength.timing_adv;
        }
        else {
            out.LTE_SignalStrength.signalStrength = INT_MAX;
            out.LTE_SignalStrength.rsrp = INT_MAX;
            out.LTE_SignalStrength.rsrq = INT_MAX;
            out.LTE_SignalStrength.rssnr = INT_MAX;
            out.LTE_SignalStrength.cqi = INT_MAX;
            out.LTE_SignalStrength.timingAdvance = INT_MAX;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_TD_SCDMA) {
            out.TD_SCDMA_SignalStrength.signalStrength = 99;
            out.TD_SCDMA_SignalStrength.bitErrorRate = 99;
            out.TD_SCDMA_SignalStrength.rscp = currentSignalStrength->TD_SCDMA_SignalStrength.rscp;
        }
        else {
            out.TD_SCDMA_SignalStrength.signalStrength = 99;
            out.TD_SCDMA_SignalStrength.bitErrorRate = 99;
            out.TD_SCDMA_SignalStrength.rscp = 255;
        }

        // legacy doesn't support NR signal strength
        out.NR_SignalStrength.ssRsrp = INT_MAX;
        out.NR_SignalStrength.ssRsrq = INT_MAX;
        out.NR_SignalStrength.ssSinr = INT_MAX;
        out.NR_SignalStrength.csiRsrp = INT_MAX;
        out.NR_SignalStrength.csiRsrq = INT_MAX;
        out.NR_SignalStrength.csiSinr = INT_MAX;
    }
    else {
        SIGNAL_STRENGTH_V1_4 *currentSignalStrength = (SIGNAL_STRENGTH_V1_4 *)data;

        if (mask & SIT_MISC_SIG_RAT_SIG_GSM) {
            out.GSM_SignalStrength.signalStrength =
                    currentSignalStrength->GSM_SignalStrength.sig_str;
            out.GSM_SignalStrength.bitErrorRate =
                    currentSignalStrength->GSM_SignalStrength.ber;
            out.GSM_SignalStrength.timingAdvance =
                    currentSignalStrength->GSM_SignalStrength.ta;
        }
        else {
            out.GSM_SignalStrength.signalStrength = 99;
            out.GSM_SignalStrength.bitErrorRate = 99;
            out.GSM_SignalStrength.timingAdvance = INT_MAX;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_WCDMA) {
            out.WCDMA_SignalStrength.signalStrength =
                    currentSignalStrength->WCDMA_SignalStrength.sig_str;
            out.WCDMA_SignalStrength.bitErrorRate =
                    currentSignalStrength->WCDMA_SignalStrength.ber;
            out.WCDMA_SignalStrength.rscp =
                    currentSignalStrength->WCDMA_SignalStrength.rscp;
            out.WCDMA_SignalStrength.ecno =
                    currentSignalStrength->WCDMA_SignalStrength.ecno;
        }
        else {
            out.WCDMA_SignalStrength.signalStrength = 99;
            out.WCDMA_SignalStrength.bitErrorRate = 99;
            out.WCDMA_SignalStrength.rscp = 255;
            out.WCDMA_SignalStrength.ecno = 255;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_CDMA_V1_4) {
            out.CDMA_SignalStrength.dbm = currentSignalStrength->CDMA_SignalStrength.dbm;
            out.CDMA_SignalStrength.ecio = currentSignalStrength->CDMA_SignalStrength.ecio;
        }
        else {
            out.CDMA_SignalStrength.dbm = -1;
            out.CDMA_SignalStrength.ecio = -1;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_EVDO_V1_4) {
            out.EVDO_SignalStrength.dbm = currentSignalStrength->EVDO_SignalStrength.dbm;
            out.EVDO_SignalStrength.ecio = currentSignalStrength->EVDO_SignalStrength.ecio;
            out.EVDO_SignalStrength.signalNoiseRatio =
                    currentSignalStrength->EVDO_SignalStrength.snr;
        }
        else {
            out.EVDO_SignalStrength.dbm = -1;
            out.EVDO_SignalStrength.ecio = -1;
            out.EVDO_SignalStrength.signalNoiseRatio = INT_MAX;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_LTE_V1_4) {
            out.LTE_SignalStrength.signalStrength = currentSignalStrength->LTE_SignalStrength.sig_str;
            out.LTE_SignalStrength.rsrp = currentSignalStrength->LTE_SignalStrength.rsrp;
            out.LTE_SignalStrength.rsrq = currentSignalStrength->LTE_SignalStrength.rsrq;
            out.LTE_SignalStrength.rssnr = currentSignalStrength->LTE_SignalStrength.rssnr;
            out.LTE_SignalStrength.cqi = currentSignalStrength->LTE_SignalStrength.cqi;
            out.LTE_SignalStrength.timingAdvance = currentSignalStrength->LTE_SignalStrength.timing_adv;
        }
        else {
            out.LTE_SignalStrength.signalStrength = INT_MAX;
            out.LTE_SignalStrength.rsrp = INT_MAX;
            out.LTE_SignalStrength.rsrq = INT_MAX;
            out.LTE_SignalStrength.rssnr = INT_MAX;
            out.LTE_SignalStrength.cqi = INT_MAX;
            out.LTE_SignalStrength.timingAdvance = INT_MAX;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_TD_SCDMA_V1_4) {
            out.TD_SCDMA_SignalStrength.signalStrength = currentSignalStrength->TD_SCDMA_SignalStrength.sig_str;
            out.TD_SCDMA_SignalStrength.bitErrorRate = currentSignalStrength->TD_SCDMA_SignalStrength.ber;
            out.TD_SCDMA_SignalStrength.rscp = currentSignalStrength->TD_SCDMA_SignalStrength.rscp;
        }
        else {
            out.TD_SCDMA_SignalStrength.signalStrength = 99;
            out.TD_SCDMA_SignalStrength.bitErrorRate = 99;
            out.TD_SCDMA_SignalStrength.rscp = 255;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_NR_V1_4) {
            out.NR_SignalStrength.ssRsrp = currentSignalStrength->NR_SignalStrength.ss_rsrp;
            out.NR_SignalStrength.ssRsrq = currentSignalStrength->NR_SignalStrength.ss_rsrq;
            out.NR_SignalStrength.ssSinr = currentSignalStrength->NR_SignalStrength.ss_sinr;
            out.NR_SignalStrength.csiRsrp = currentSignalStrength->NR_SignalStrength.csi_rsrp;
            out.NR_SignalStrength.csiRsrq = currentSignalStrength->NR_SignalStrength.csi_rsrp;
            out.NR_SignalStrength.csiSinr = currentSignalStrength->NR_SignalStrength.csi_rsrp;
        }
        else {
            out.NR_SignalStrength.ssRsrp = INT_MAX;
            out.NR_SignalStrength.ssRsrq = INT_MAX;
            out.NR_SignalStrength.ssSinr = INT_MAX;
            out.NR_SignalStrength.csiRsrp = INT_MAX;
            out.NR_SignalStrength.csiRsrq = INT_MAX;
            out.NR_SignalStrength.csiSinr = INT_MAX;
        }
    }
}

static void FillSignalStrengthFoVoWifi(RIL_SignalStrengthForVoWifi& out,
        void *data, unsigned int size, int mask)
{
    if (data == NULL || size < sizeof(SIGNAL_STRENGTH)) {
        mask = 0;
    }

    if (size < sizeof(SIGNAL_STRENGTH_V1_4)) {
        if ((mask & SIT_MISC_SIG_RAT_SIG_GW) || (mask & SIT_MISC_SIG_RAT_SIG_LTE)) {
            out.valid = true;
        }

        SIGNAL_STRENGTH *currentSignalStrength = (SIGNAL_STRENGTH *)data;

        if (mask & SIT_MISC_SIG_RAT_SIG_GW) {
            out.umts_sig_str =
                    currentSignalStrength->GW_SignalStrength.sig_str;
            out.umts_ecno =
                    currentSignalStrength->UMTS_SignalStrength.ecno;
        }
        else {
            out.umts_sig_str = 99;
            out.umts_ecno = 255;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_LTE) {
            out.lte_rsrp = currentSignalStrength->LTE_SignalStrength.rsrp;
            out.lte_rssnr = currentSignalStrength->LTE_SignalStrength.rssnr;
        }
        else {
            out.lte_rsrp = INT_MAX;
            out.lte_rssnr = INT_MAX;
        }
    }
    else {
        if ((mask & SIT_MISC_SIG_RAT_SIG_WCDMA) || (mask & SIT_MISC_SIG_RAT_SIG_LTE_V1_4)) {
            out.valid = true;
        }

        SIGNAL_STRENGTH_V1_4 *currentSignalStrength = (SIGNAL_STRENGTH_V1_4 *)data;

        if (mask & SIT_MISC_SIG_RAT_SIG_WCDMA) {
            out.umts_sig_str =
                    currentSignalStrength->WCDMA_SignalStrength.sig_str;
            out.umts_ecno =
                    currentSignalStrength->WCDMA_SignalStrength.ecno;
        }
        else {
            out.umts_sig_str = 99;
            out.umts_ecno = 255;
        }

        if (mask & SIT_MISC_SIG_RAT_SIG_LTE_V1_4) {
            out.lte_rsrp = currentSignalStrength->LTE_SignalStrength.rsrp;
            out.lte_rssnr = currentSignalStrength->LTE_SignalStrength.rssnr;
        }
        else {
            out.lte_rsrp = INT_MAX;
            out.lte_rssnr = INT_MAX;
        }
    }
}

/**
 * ProtocolMiscSigStrengthAdapter
 */
ProtocolMiscSigStrengthAdapter::ProtocolMiscSigStrengthAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
    memset(&mSignalStrength, 0, sizeof(mSignalStrength));
    memset(&mSignalStrengthForVoWifi, 0, sizeof(mSignalStrengthForVoWifi));
}

RIL_SignalStrength_V1_4& ProtocolMiscSigStrengthAdapter::GetSignalStrength()
{
    // init as default
    FillSignalStrength(mSignalStrength, NULL, sizeof(SIGNAL_STRENGTH_V1_4), 0);
    if (m_pModemData != NULL) {
        UINT dataLen = m_pModemData->GetLength();
        int mask = 0;
        UINT signalStrengthSize = 0;
        void *currentSignalStrength = NULL;
        if (dataLen >= sizeof(sit_misc_signal_strength_rsp) && dataLen < sizeof(sit_misc_signal_strength_rsp_v1_4)) {
            sit_misc_signal_strength_rsp *data =
                    (sit_misc_signal_strength_rsp *)m_pModemData->GetRawData();
            if (data->hdr.id == SIT_GET_SIGNAL_STRENGTH) {
                mask = data->valid_rat_sig_flag;
                currentSignalStrength = &data->sig_strength;
                signalStrengthSize = sizeof(SIGNAL_STRENGTH);

            }
        } else if (dataLen >= sizeof(sit_misc_signal_strength_rsp_v1_4)) {
            sit_misc_signal_strength_rsp_v1_4 *data =
                    (sit_misc_signal_strength_rsp_v1_4 *)m_pModemData->GetRawData();
            if (data->hdr.id == SIT_GET_SIGNAL_STRENGTH) {
                mask = data->valid_rat_sig_flag;
                currentSignalStrength = &data->sig_strength;
                signalStrengthSize = sizeof(SIGNAL_STRENGTH_V1_4);
            }
        }

        FillSignalStrength(mSignalStrength, currentSignalStrength, signalStrengthSize, mask);
    }
    return mSignalStrength;
}

RIL_SignalStrengthForVoWifi& ProtocolMiscSigStrengthAdapter::GetSignalStrengthForVowifi()
{
    mSignalStrengthForVoWifi.lte_rsrp = INT_MAX;
    mSignalStrengthForVoWifi.lte_rssnr = INT_MAX;
    mSignalStrengthForVoWifi.umts_sig_str = 99;
    mSignalStrengthForVoWifi.umts_ecno = 255;
    mSignalStrengthForVoWifi.valid = false;

    FillSignalStrengthFoVoWifi(mSignalStrengthForVoWifi, NULL, 0, 0);
    if (m_pModemData != NULL) {
        UINT dataLen = m_pModemData->GetLength();
        int mask = 0;
        UINT signalStrengthSize = 0;
        void *currentSignalStrength = NULL;
        if (dataLen >= sizeof(sit_misc_signal_strength_rsp) && dataLen < sizeof(sit_misc_signal_strength_rsp_v1_4)) {
            sit_misc_signal_strength_rsp *data =
                    (sit_misc_signal_strength_rsp *)m_pModemData->GetRawData();
            if (data->hdr.id == SIT_GET_SIGNAL_STRENGTH) {
                mask = data->valid_rat_sig_flag;
                currentSignalStrength = &data->sig_strength;
                signalStrengthSize = sizeof(SIGNAL_STRENGTH);

            }
        } else if (dataLen >= sizeof(sit_misc_signal_strength_rsp_v1_4)) {
            sit_misc_signal_strength_rsp_v1_4 *data =
                    (sit_misc_signal_strength_rsp_v1_4 *)m_pModemData->GetRawData();
            if (data->hdr.id == SIT_GET_SIGNAL_STRENGTH) {
                mask = data->valid_rat_sig_flag;
                currentSignalStrength = &data->sig_strength;
                signalStrengthSize = sizeof(SIGNAL_STRENGTH_V1_4);
            }
        }

        FillSignalStrengthFoVoWifi(mSignalStrengthForVoWifi, currentSignalStrength, signalStrengthSize, mask);
    }
    return mSignalStrengthForVoWifi;
}

/**
 * ProtocolMiscSigStrengthIndAdapter
 */
ProtocolMiscSigStrengthIndAdapter::ProtocolMiscSigStrengthIndAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
    memset(&mSignalStrength, 0, sizeof(mSignalStrength));
    memset(&mSignalStrengthForVoWifi, 0, sizeof(mSignalStrengthForVoWifi));
}

RIL_SignalStrength_V1_4& ProtocolMiscSigStrengthIndAdapter::GetSignalStrength()
{
    // init as default
    FillSignalStrength(mSignalStrength, NULL, sizeof(SIGNAL_STRENGTH_V1_4), 0);
    if (m_pModemData != NULL) {
        UINT dataLen = m_pModemData->GetLength();
        int mask = 0;
        UINT signalStrengthSize = 0;
        void *currentSignalStrength = NULL;
        if (dataLen >= sizeof(sit_misc_signal_strength_ind) && dataLen < sizeof(sit_misc_signal_strength_ind_v1_4)) {
            sit_misc_signal_strength_ind *data =
                    (sit_misc_signal_strength_ind *)m_pModemData->GetRawData();
            if (data->hdr.id == SIT_IND_SIGNAL_STRENGTH) {
                mask = data->valid_rat_sig_flag;
                currentSignalStrength = &data->sig_strength;
                signalStrengthSize = sizeof(SIGNAL_STRENGTH);
            }
        } else if (dataLen >= sizeof(sit_misc_signal_strength_ind_v1_4)) {
            sit_misc_signal_strength_ind_v1_4 *data =
                    (sit_misc_signal_strength_ind_v1_4 *)m_pModemData->GetRawData();
            if (data->hdr.id == SIT_IND_SIGNAL_STRENGTH) {
                mask = data->valid_rat_sig_flag;
                currentSignalStrength = &data->sig_strength;
                signalStrengthSize = sizeof(SIGNAL_STRENGTH_V1_4);
            }
        }

        FillSignalStrength(mSignalStrength, currentSignalStrength, signalStrengthSize, mask);
    }
    return mSignalStrength;
}

RIL_SignalStrengthForVoWifi& ProtocolMiscSigStrengthIndAdapter::GetSignalStrengthForVowifi()
{
    mSignalStrengthForVoWifi.lte_rsrp = INT_MAX;
    mSignalStrengthForVoWifi.lte_rssnr = INT_MAX;
    mSignalStrengthForVoWifi.umts_sig_str = 99;
    mSignalStrengthForVoWifi.umts_ecno = 255;
    mSignalStrengthForVoWifi.valid = false;

    FillSignalStrengthFoVoWifi(mSignalStrengthForVoWifi, NULL, 0, 0);
    if (m_pModemData != NULL) {
        UINT dataLen = m_pModemData->GetLength();
        int mask = 0;
        UINT signalStrengthSize = 0;
        void *currentSignalStrength = NULL;
        if (dataLen >= sizeof(sit_misc_signal_strength_ind) && dataLen < sizeof(sit_misc_signal_strength_ind_v1_4)) {
            sit_misc_signal_strength_ind *data =
                    (sit_misc_signal_strength_ind *)m_pModemData->GetRawData();
            if (data->hdr.id == SIT_IND_SIGNAL_STRENGTH) {
                mask = data->valid_rat_sig_flag;
                currentSignalStrength = &data->sig_strength;
                signalStrengthSize = sizeof(SIGNAL_STRENGTH);
            }
        } else if (dataLen >= sizeof(sit_misc_signal_strength_ind_v1_4)) {
            sit_misc_signal_strength_ind_v1_4 *data =
                    (sit_misc_signal_strength_ind_v1_4 *)m_pModemData->GetRawData();
            if (data->hdr.id == SIT_IND_SIGNAL_STRENGTH) {
                mask = data->valid_rat_sig_flag;
                currentSignalStrength = &data->sig_strength;
                signalStrengthSize = sizeof(SIGNAL_STRENGTH_V1_4);
            }
        }

        FillSignalStrengthFoVoWifi(mSignalStrengthForVoWifi, currentSignalStrength,
                    signalStrengthSize, mask);
    }
    return mSignalStrengthForVoWifi;
}

int ProtocolMiscNITZTimeAdapter::TimeInfoType() const
{
    int timeinfo = 0;
/*
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            timeinfo = data->time_info_type;
        }
    }
*/
    return timeinfo;
}

int ProtocolMiscNITZTimeAdapter::DayLightValid() const
{
    int daylightvalid = SIT_NITS_DAYLIGHT_INFO_VALID;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            daylightvalid = data->daylight_valid;
        }
    }
    return daylightvalid;
}

int ProtocolMiscNITZTimeAdapter::Year() const
{
    int year = 0;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            year = data->year;
        }
    }
    return year;
}

int ProtocolMiscNITZTimeAdapter::Month() const
{
    int month = 0;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            month = data->month;
        }
    }
    return month;
}

int ProtocolMiscNITZTimeAdapter::Day() const
{
    int day = 0;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            day = data->day;
        }
    }
    return day;
}

int ProtocolMiscNITZTimeAdapter::Hour() const
{
    int hour = 0;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            hour = data->hour;
        }
    }
    return hour;
}

int ProtocolMiscNITZTimeAdapter::Minute() const
{
    int minute = 0;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            minute = data->minute;
        }
    }
    return minute;
}

int ProtocolMiscNITZTimeAdapter::Second() const
{
    int second = 0;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            second = data->second;
        }
    }
    return second;
}

int ProtocolMiscNITZTimeAdapter::TimeZone() const
{
    #define BITSIZE 8
    #define SIGNFLAG (1<<(BITSIZE-1))
    #define DATABITS (SIGNFLAG-1)

    int timezone = 0;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            timezone = data->time_zone;
            if ((timezone& SIGNFLAG)!=0) {                    // signflag set
                timezone= (~timezone & DATABITS) + 1;         // 2s complement without signflag
                timezone= -timezone;                              // negative number
            }
        }
    }
    return timezone;
}
int ProtocolMiscNITZTimeAdapter::DayLightAdjust() const
{
    int daylightadj = SIT_NITS_DAYLIGHT_ADJUST_NOADJUST;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            daylightadj = data->daylight_adjust;
        }
    }
    return daylightadj;
}

int ProtocolMiscNITZTimeAdapter::DayofWeek() const
{
    int dayofweek = SIT_NITS_DAY_OF_WEEK_SUN;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            dayofweek = data->day_of_week;
        }
    }
    return dayofweek;
}

int ProtocolMiscNITZTimeAdapter::GetMMInfo() const
{
    int mminfo = 0;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            mminfo = data->mminfo;
        }
    }
    return mminfo;
}

const BYTE * ProtocolMiscNITZTimeAdapter::GetPLMN() const
{
    BYTE *plmn = NULL;
    if (m_pModemData != NULL) {
        sit_misc_nits_time_received_ind *data = (sit_misc_nits_time_received_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_NITZ_TIME_RECEIVED) {
            plmn = data->plmn;
        }
    }
    return plmn;
}

int ProtocolMiscIMEIAdapter::IMEILen() const
{
    int imeilen = 0;
    if (m_pModemData != NULL) {
        sit_id_get_imei_rsp *data = (sit_id_get_imei_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_IMEI) {
            imeilen = data->imei_len;
        }
    }
    return imeilen;
}

const BYTE * ProtocolMiscIMEIAdapter::GetIMEI() const
{
    BYTE *imei = NULL;
    if (m_pModemData != NULL) {
        sit_id_get_imei_rsp *data = (sit_id_get_imei_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_IMEI) {
            imei = data->imei;
        }
    }
    return imei;
}

int ProtocolMiscIMEISVAdapter::IMEISVLen() const
{
    int imeisvlen = 0;
    if (m_pModemData != NULL) {
        sit_id_get_imeisv_rsp *data = (sit_id_get_imeisv_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_IMEISV) {
            imeisvlen = data->imeisv_len;
        }
    }
    return imeisvlen;
}

const BYTE * ProtocolMiscIMEISVAdapter::GetIMEISV() const
{
    BYTE *imeisv = NULL;
    if (m_pModemData != NULL) {
        sit_id_get_imeisv_rsp *data = (sit_id_get_imeisv_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_IMEISV) {
            imeisv = data->imeisv;
        }
    }
    return imeisv;
}

int ProtocolMiscDeviceIDAdapter::IMEILen() const
{
    int imeilen = 0;
    if (m_pModemData != NULL) {
        sit_id_get_deviceid_rsp *data = (sit_id_get_deviceid_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_DEVICE_ID) {
            imeilen = data->imei_len;
        }
    }
    return imeilen;
}

const BYTE * ProtocolMiscDeviceIDAdapter::GetIMEI() const
{
    BYTE *imei = NULL;
    if (m_pModemData != NULL) {
        sit_id_get_deviceid_rsp *data = (sit_id_get_deviceid_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_DEVICE_ID) {
            imei = data->imei;
        }
    }
    return imei;
}

int ProtocolMiscDeviceIDAdapter::IMEISVLen() const
{
    int imeisvlen = 0;
    if (m_pModemData != NULL) {
        sit_id_get_deviceid_rsp *data = (sit_id_get_deviceid_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_DEVICE_ID) {
            imeisvlen = data->imeisv_len;
        }
    }
    return imeisvlen;
}

const BYTE * ProtocolMiscDeviceIDAdapter::GetIMEISV() const
{
    BYTE *imeisv = NULL;
    if (m_pModemData != NULL) {
        sit_id_get_deviceid_rsp *data = (sit_id_get_deviceid_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_DEVICE_ID) {
            imeisv = data->imesv;
        }
    }
    return imeisv;
}

int ProtocolMiscDeviceIDAdapter::MEIDLen() const
{
    int meidlen = 0;
    if (m_pModemData != NULL) {
        sit_id_get_deviceid_rsp *data = (sit_id_get_deviceid_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_DEVICE_ID) {
            meidlen = data->meid_len;
        }
    }
    return meidlen;
}

const BYTE * ProtocolMiscDeviceIDAdapter::GetMEID() const
{
    BYTE *meid = NULL;
    if (m_pModemData != NULL) {
        sit_id_get_deviceid_rsp *data = (sit_id_get_deviceid_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_DEVICE_ID) {
            meid = data->meid;
        }
    }
    return meid;
}

int ProtocolMiscDeviceIDAdapter::ESNLen() const
{
    int esnlen = 0;
    if (m_pModemData != NULL) {
        sit_id_get_deviceid_rsp *data = (sit_id_get_deviceid_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_DEVICE_ID) {
            esnlen = data->esn_len;
        }
    }
    return esnlen;
}

const BYTE * ProtocolMiscDeviceIDAdapter::GetESN() const
{
    BYTE *esn = NULL;
    if (m_pModemData != NULL) {
        sit_id_get_deviceid_rsp *data = (sit_id_get_deviceid_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_DEVICE_ID) {
            esn = data->esn;
        }
    }
    return esn;
}

/**
 * ProtocolMiscPhoneResetAdapter
 */
ProtocolMiscPhoneResetAdapter::ProtocolMiscPhoneResetAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
    m_ResetType = SIT_PWR_RESET_TYPE_PHONE_ONLY;
    m_ResetCause = 0;    //TBD

    if (m_pModemData != NULL) {
        sit_pwr_phone_reset_ind *data = (sit_pwr_phone_reset_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_PHONE_RESET) ) {
            m_ResetType = data->reset_type;
            m_ResetCause = data->reset_cause;
        }
    }
}

BYTE ProtocolMiscPhoneResetAdapter::GetResetType()
{
    return m_ResetType;
}

BYTE ProtocolMiscPhoneResetAdapter::GetResetCause()
{
    return m_ResetCause;
}

/**
 * ProtocolMiscDataStateChangeAdapter
 */
ProtocolMiscDataStateChangeAdapter::ProtocolMiscDataStateChangeAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
    m_ExpectedState = 0;

    if (m_pModemData != NULL) {
        sit_pdp_data_state_change_ind *data = (sit_pdp_data_state_change_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_DATA_STATE_CHANGE) ) {
            m_ExpectedState = data->expected_state;
        }
    }
}

BYTE ProtocolMiscDataStateChangeAdapter::GetExpectedState()
{
    return m_ExpectedState;
}

/**
 * ProtocolMiscNvReadItemAdapter
 */
ProtocolMiscNvReadItemAdapter::ProtocolMiscNvReadItemAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
#if 0   //removed
    memset(m_value, 0, sizeof(m_value));
#endif
}

const char *ProtocolMiscNvReadItemAdapter::GetValue()
{
#if 0   //removed
    if (m_pModemData != NULL) {
        sit_misc_get_oem_nv_item_rsp *data = (sit_misc_get_oem_nv_item_rsp *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_OEM_NV_ITEM) ) {
            strncpy(m_value, (const char *)data->data, MAX_NV_ITEM_DATA_SIZE);
            return m_value;
        }
    }
#endif
    return NULL;
}

/**
 * ProtocolMiscGetActivityInfoAdapter
 */
ProtocolMiscGetActivityInfoAdapter::ProtocolMiscGetActivityInfoAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
    if (m_pModemData != NULL) {
        //sit_misc_get_activity_info_rsp *data = (sit_misc_get_activity_info_rsp *)m_pModemData->GetRawData();
    }
}

UINT32 ProtocolMiscGetActivityInfoAdapter::GetSleepPeriod() const
{
    sit_misc_get_activity_info_rsp *data = (sit_misc_get_activity_info_rsp *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_GET_ACTIVITY_INFO) {
        return data->sleep_mode_time_ms;
    }
    return 0;
}

UINT32 ProtocolMiscGetActivityInfoAdapter::GetIdlePeriod() const
{
    sit_misc_get_activity_info_rsp *data = (sit_misc_get_activity_info_rsp *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_GET_ACTIVITY_INFO) {
        return data->idle_mode_time_ms;
    }
    return 0;
}

UINT32* ProtocolMiscGetActivityInfoAdapter::GetTxPeriod() const
{
    sit_misc_get_activity_info_rsp *data = (sit_misc_get_activity_info_rsp *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_GET_ACTIVITY_INFO) {
        return data->tx_mode_time_ms;
    }
    return 0;
}

UINT32 ProtocolMiscGetActivityInfoAdapter::GetRxPeriod() const
{
    sit_misc_get_activity_info_rsp *data = (sit_misc_get_activity_info_rsp *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_GET_ACTIVITY_INFO) {
        return data->rx_mode_time_ms;
    }
    return 0;
}

/**
 * ProtocolMiscGetActivityInfoAdapter
 */
const char * ProtocolMiscGetMslCodeAdapter::getMslCode() const
{
    char *msl = NULL;
    if (m_pModemData != NULL) {
        sit_misc_get_msl_code_rsp *data = (sit_misc_get_msl_code_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_MSL_CODE) {
            msl = data->msl_code;
        }
    }
    return msl;
}

/**
 * ProtocolMiscPinControlAdapter
 */
ProtocolMiscPinControlAdapter::ProtocolMiscPinControlAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
    m_Signal = 0;
    m_Status = 0;

    if (m_pModemData != NULL) {
        sit_misc_pin_control_ind *data = (sit_misc_pin_control_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_PIN_CONTROL) ) {
            m_Signal = data->signal;
            m_Status = data->status;
        }
    }
}

BYTE ProtocolMiscPinControlAdapter::GetSignal()
{
    return m_Signal;
}

BYTE ProtocolMiscPinControlAdapter::GetStatus()
{
    return m_Status;
}

/**
 * ProtocolMiscGetPreferredCallCapability
 */

int ProtocolMiscGetPreferredCallCapability::GetMode() const
{
    if (m_pModemData != NULL) {
        sit_misc_get_preferred_call_capability_rsp *data = (sit_misc_get_preferred_call_capability_rsp *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_PREFERRED_CALL_CAPABILITY) ) {
            return data->mode;
        }
    }
    return 0;
}

/**
 * ProtocolMiscSetManualBandModeAdapter
 */
BYTE ProtocolMiscSetManualBandModeAdapter::GetCause()
{
    if (m_pModemData != NULL) {
        sit_misc_set_manual_band_mode_rsp *data = (sit_misc_set_manual_band_mode_rsp *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_SET_MANUAL_BAND_MODE) ) {
            return data->cause;
        }
    }
    return 0;
}

/**
 * ProtocolMiscSetRfDesenseModeAdapter
 */
BYTE ProtocolMiscSetRfDesenseModeAdapter::GetCause()
{
    if (m_pModemData != NULL) {
        sit_misc_set_rf_desense_mode_rsp *data = (sit_misc_set_rf_desense_mode_rsp *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_SET_RF_DESENSE_MODE) ) {
            return data->cause;
        }
    }
    return 0;
}

/* ProtocolMiscGetHwConfigAdapter */
int ProtocolMiscGetHwConfigAdapter::GetNum() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_misc_get_hw_config_rsp *data = (sit_misc_get_hw_config_rsp *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_HW_CONFIG) ) {
            ret = data->num_recodrs;
        }
    }

    return ret;
}

int ProtocolMiscGetHwConfigAdapter::GetData(RIL_HardwareConfig *pRsp, int num) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_misc_get_hw_config_rsp *data = (sit_misc_get_hw_config_rsp *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_GET_HW_CONFIG) && pRsp != NULL) {
            for(int index = 0; index < num; index++) {
                RIL_HardwareConfig *pWrite = pRsp + index;
                SIT_HW_CONFIG *pRead = (data->hw_config) + index;

                pWrite->type = (RIL_HardwareConfig_Type)pRead->type;
                pWrite->state = (RIL_HardwareConfig_State)pRead->state;
                pWrite->uuid[MAX_UUID_LENGTH - 1] = 0;
                strncpy(pWrite->uuid, pRead->uuid, MAX_UUID_LENGTH-1);

                memset(&(pWrite->cfg), 0, sizeof(pWrite->cfg));
                if (pWrite->type == RIL_HARDWARE_CONFIG_MODEM) {
                    pWrite->cfg.modem.rilModel = pRead->cfg.modem.ril_model;
                    pWrite->cfg.modem.rat = switchRafValueForFW(pRead->cfg.modem.rat);  // bit Mask SIT - RIL conversion is needed  RIL_RadioTechnology.
                    pWrite->cfg.modem.maxVoice = pRead->cfg.modem.max_voice;
                    pWrite->cfg.modem.maxData = pRead->cfg.modem.max_data;
                    pWrite->cfg.modem.maxStandby = pRead->cfg.modem.max_standby;
                } else if (pWrite->type == RIL_HARDWARE_CONFIG_SIM) {
                    strncpy(pWrite->cfg.sim.modemUuid, pRead->cfg.sim.modem_uuid, MAX_UUID_LENGTH-1);
                } else {
                    RilLogE("(GetHwConfigAdapter)Invalid HW CFG type %d", pWrite->type);
                }
           }
        }
    }

    return ret;
}

/* ProtocolMiscHwConfigChangeAdapter */
int ProtocolMiscHwConfigChangeAdapter::GetNum() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_misc_hw_config_change_ind *data = (sit_misc_hw_config_change_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_HW_CONFIG_CHANGED) ) {
            ret = data->num_recodrs;
        }
    }

    return ret;
}

int ProtocolMiscHwConfigChangeAdapter::GetData(RIL_HardwareConfig *pRsp, int num) const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_misc_hw_config_change_ind *data = (sit_misc_hw_config_change_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_HW_CONFIG_CHANGED) && pRsp != NULL) {
            for(int index = 0; index < num; index++) {
                RIL_HardwareConfig *pWrite = pRsp + index;
                SIT_HW_CONFIG *pRead = (data->hw_config) + index;

                pWrite->type = (RIL_HardwareConfig_Type)pRead->type;
                pWrite->state = (RIL_HardwareConfig_State)pRead->state;
                pWrite->uuid[MAX_UUID_LENGTH - 1] = 0;
                strncpy(pWrite->uuid, pRead->uuid, MAX_UUID_LENGTH-1);

                memset(&(pWrite->cfg), 0, sizeof(pWrite->cfg));
                if (pWrite->type == RIL_HARDWARE_CONFIG_MODEM) {
                    pWrite->cfg.modem.rilModel = pRead->cfg.modem.ril_model;
                    pWrite->cfg.modem.rat = switchRafValueForFW(pRead->cfg.modem.rat);
                    pWrite->cfg.modem.maxVoice = pRead->cfg.modem.max_voice;
                    pWrite->cfg.modem.maxData = pRead->cfg.modem.max_data;
                    pWrite->cfg.modem.maxStandby = pRead->cfg.modem.max_standby;
                } else if (pWrite->type == RIL_HARDWARE_CONFIG_SIM) {
                    strncpy(pWrite->cfg.sim.modemUuid, pRead->cfg.sim.modem_uuid, MAX_UUID_LENGTH-1);
                } else {
                    RilLogE("(HwConfigChangeAdapter) Invalid HW CFG type %d", pWrite->type);
                }
           }
        }
    }

    return ret;
}

/* ProtocolMiscCdmaPrlChangeAdapter */
int ProtocolMiscCdmaPrlChangeAdapter::GetPrlVersion() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_misc_cdma_prl_change_ind *data = (sit_misc_cdma_prl_change_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_CDMA_PRL_CHANGED) ) {
            ret = data->prl_ver;
        }
    }

    return ret;
}

/**
 * ProtocolMiscStoreAdbSerialNumberAdapter
 */
BYTE ProtocolMiscStoreAdbSerialNumberAdapter::GetRcmError()
{
    if (m_pModemData != NULL) {
        sit_oem_store_adb_serial_number_rsp *data = (sit_oem_store_adb_serial_number_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_STORE_ADB_SERIAL_NUMBER_REQ) {
            return data->rcmError;
        }
    }
    return 0;
}

/**
 * ProtocolMiscReadAdbSerialNumberAdapter
 */
const char* ProtocolMiscReadAdbSerialNumberAdapter::GetAdbSerialNumber()
{
    char *adbSerialNumber = NULL;
    if (m_pModemData != NULL) {
        sit_oem_read_adb_serial_number_rsp *data = (sit_oem_read_adb_serial_number_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_READ_ADB_SERIAL_NUMBER_REQ) {
            adbSerialNumber = data->adbSerialNumber;
        }
    }
    return adbSerialNumber;
}

/* ProtocolMiscLceAdapter  */
void ProtocolMiscLceAdapter::Init()
{
    m_lceStatus = 0;
    m_actualIntervalMs = 0;
    m_dlCapacityKbps = 0;
    m_ulCapacityKbps = 0;
    m_confidenceLevel = 0;
    m_lceSuspended = 0;

    if (m_pModemData != NULL && m_pModemData->GetRawData() != NULL) {
        UINT msgId = m_pModemData->GetMessageId();
        if (msgId == SIT_START_LCE_INFO) {
            sit_pdp_start_lce_info_rsp *data = (sit_pdp_start_lce_info_rsp *) m_pModemData->GetRawData();
            m_lceStatus = data->status;
            m_actualIntervalMs = data->interval;
        } else if (msgId == SIT_STOP_LCE_INFO) {
            sit_pdp_stop_lce_info_rsp *data = (sit_pdp_stop_lce_info_rsp *) m_pModemData->GetRawData();
            m_lceStatus = data->status;
            m_actualIntervalMs = data->interval;
        } else if (msgId == SIT_GET_LCE_DATA) {
            sit_pdp_get_lce_data_rsp *data = (sit_pdp_get_lce_data_rsp *) m_pModemData->GetRawData();
            m_dlCapacityKbps = data->dl_lc;
            m_ulCapacityKbps = data->ul_lc;
            m_confidenceLevel = data->conf_lvl;
            m_lceSuspended = data->is_suspended;
        }
    }
}

RIL_Errno ProtocolMiscLceAdapter::GetRilErrorCode() const
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

/* ProtocolMiscLceIndAdapter */
int ProtocolMiscLceIndAdapter::GetDLLc() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_pdp_lce_data_ind *data = (sit_pdp_lce_data_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_LCE_DATA) ) {
            ret = data->dl_lc;
        }
    }

    return ret;
}

int ProtocolMiscLceIndAdapter::GetULLc() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_pdp_lce_data_ind *data = (sit_pdp_lce_data_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_LCE_DATA) ) {
            ret = data->ul_lc;
        }
    }

    return ret;
}

int ProtocolMiscLceIndAdapter::GetConfLevel() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_pdp_lce_data_ind *data = (sit_pdp_lce_data_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_LCE_DATA) ) {
            ret = data->conf_lvl;
        }
    }

    return ret;
}

int ProtocolMiscLceIndAdapter::GetIsSuspended() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_pdp_lce_data_ind *data = (sit_pdp_lce_data_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_LCE_DATA) ) {
            ret = data->is_suspended;
        }
    }

    return ret;
}

#ifdef SUPPORT_CDMA
/* ProtocolCdmaSubscriptionAdapter */
void ProtocolCdmaSubscriptionAdapter::Init()
{
    memset(m_szMdn, 0, MAX_CDMA_MDN_LEN);
    memset(m_szMin, 0, MAX_CDMA_MIN_LEN);
    m_wSid = m_wNid = m_uPrlVersion = 0;

    if (m_pModemData != NULL && GetErrorCode() == RCM_E_SUCCESS) {
        sit_misc_get_cdma_subscription_rsp *data = (sit_misc_get_cdma_subscription_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CDMA_SUBSCRIPTION) {
            memcpy(m_szMdn, data->mdn, data->mdn_size);
            m_szMdn[data->mdn_size] = '\0';
            memcpy(m_szMin, data->min, MAX_CDMA_MIN_LEN);
            m_szMin[MAX_CDMA_MIN_LEN] = '\0';
            m_wSid = data->sid;
            m_wNid = data->nid;
            m_uPrlVersion = data->prl_version;
        }
    }
}
#endif // SUPPORT_CDMA

/**
 * ProtocolMiscSarControlStateAdapter
 */
ProtocolMiscSarContolStateAdapter::ProtocolMiscSarContolStateAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
    m_DeviceState = 0;
    if (m_pModemData != NULL) {
        sit_misc_psensor_sar_control_state_ind *data = (sit_misc_psensor_sar_control_state_ind *) m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_SAR_CONTROL_STATE) ) {
            m_DeviceState = data->device_state;
        }
    }
}

BYTE ProtocolMiscSarContolStateAdapter::GetDeviceState()
{
    return m_DeviceState;
}

/**
 * ProtocolMiscSarRfConnectionAdapter
 */
ProtocolMiscSarRfConnectionAdapter::ProtocolMiscSarRfConnectionAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
    m_RfState = 0;
    if (m_pModemData != NULL) {
        sit_misc_sar_rf_connection_ind *data = (sit_misc_sar_rf_connection_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_OEM_IND_RF_CONNECTION) ) {
            m_RfState = data->rf_state;
        }
    }
}

BYTE ProtocolMiscSarRfConnectionAdapter::GetRfState()
{
    return m_RfState;
}

/**
 * ProtocolMiscGetSarStateAdapter
 */
int ProtocolMiscGetSarStateAdapter::GetSarState() const
{
    if (m_pModemData != NULL) {
        sit_misc_sar_get_sar_state_rsp *data = (sit_misc_sar_get_sar_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_GET_SAR_STATE) {
            RilLogV("sar state=%d", data->sar_state);
            return data->sar_state;
        }
    }
    return 0;
}

/**
 * ProtocolMiscRssiScanResult
 */
int ProtocolMiscRssiScanResultAdapter::GetTotalPage()
{
    int totalPage = 0;
    if (m_pModemData != NULL) {
        sit_misc_rssi_scan_ind *data = (sit_misc_rssi_scan_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_RSSI_SCAN) ) {
            totalPage = data->total_page & 0xFF;
        }
    }
    return totalPage;
}

int ProtocolMiscRssiScanResultAdapter::GetCurrentPage()
{
    int currentPage = -1;
    if (m_pModemData != NULL) {
        sit_misc_rssi_scan_ind *data = (sit_misc_rssi_scan_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_RSSI_SCAN) ) {
            currentPage = data->current_page & 0xFF;
        }
    }
    return currentPage;
}

int ProtocolMiscRssiScanResultAdapter::GetStartFrequency()
{
    int startFrequency = 0;
    if (m_pModemData != NULL) {
        sit_misc_rssi_scan_ind *data = (sit_misc_rssi_scan_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_RSSI_SCAN) ) {
            startFrequency = data->start_frequency & 0xFF;
        }
    }
    return startFrequency;
}

int ProtocolMiscRssiScanResultAdapter::GetEndFrequency()
{
    int endFrequency = 0;
    if (m_pModemData != NULL) {
        sit_misc_rssi_scan_ind *data = (sit_misc_rssi_scan_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_RSSI_SCAN) ) {
            endFrequency = data->end_frequency & 0xFF;
        }
    }
    return endFrequency;
}

int ProtocolMiscRssiScanResultAdapter::GetStep()
{
    int step = 0;
    if (m_pModemData != NULL) {
        sit_misc_rssi_scan_ind *data = (sit_misc_rssi_scan_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_RSSI_SCAN) ) {
            step = data->step & 0xFF;
        }
    }
    return step;
}

int ProtocolMiscRssiScanResultAdapter::GetScanResultSize()
{
    int len = 0;
    if (m_pModemData != NULL) {
        sit_misc_rssi_scan_ind *data = (sit_misc_rssi_scan_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_RSSI_SCAN) ) {
            len = (m_pModemData->GetLength()-7)/2;
        }
    }
    return len;
}

INT16* ProtocolMiscRssiScanResultAdapter::GetScanResult()
{
    if (m_pModemData != NULL) {
        sit_misc_rssi_scan_ind *data = (sit_misc_rssi_scan_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_RSSI_SCAN) ) {
            return data->result;
        }
    }
    return 0;
}

ProtocolMiscATCommandAdapter::ProtocolMiscATCommandAdapter(const ModemData *pModemData)
: ProtocolIndAdapter(pModemData)
, mCommand(NULL)
, mCommandLength(0)
{
    Init();
}

void ProtocolMiscATCommandAdapter::Init()
{
    if (m_pModemData != NULL) {
        sit_misc_forwarding_at_command_ind *data = (sit_misc_forwarding_at_command_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_FORWARDING_AT_COMMAND) ) {
            mCommandLength = data->length & 0xFFFF;
            if (mCommandLength > MAX_SIT_AT_COMMAND_LENGTH) {
                mCommandLength = MAX_SIT_AT_COMMAND_LENGTH;
            }

            mCommand = new char[mCommandLength + 1];
            if (mCommand != NULL) {
                memset(mCommand, 0, mCommandLength + 1);
                memcpy(mCommand, data->data, mCommandLength);
            }
            else {
                mCommandLength = 0;
            }
        }
    }
}

ProtocolMiscATCommandAdapter::~ProtocolMiscATCommandAdapter()
{
    if (mCommand != NULL) {
        delete[] mCommand;
        mCommand = NULL;
    }
}

/**
 * ProtocolMiscGetRadioNodeAdapter
 */
const char *ProtocolMiscGetRadioNodeAdapter::GetValue() const
{
    char *value = NULL;
    if (m_pModemData != NULL) {
        sit_misc_get_radio_node_rsp *data = (sit_misc_get_radio_node_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_RADIO_NODE) {
            value = data->value;
        }
    }
    return value;
}

/**
 * ProtocolMiscGetVoLteProvisionUpdateAdapter
 */
int ProtocolMiscGetVoLteProvisionUpdateAdapter::GetStatus()
{
    if (m_pModemData != NULL) {
        sit_misc_get_volte_provision_rsp *data = (sit_misc_get_volte_provision_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_VOLTE_PROVISION_UPDATE) {
            return (data->status == 0x00) ? 0 : 1;
        }
    }
    return 0;
}

/**
 * ProtocolMiscSetVoLteProvisionUpdateAdapter
 */
int ProtocolMiscSetVoLteProvisionUpdateAdapter::GetResult()
{
    if (m_pModemData != NULL) {
        sit_misc_set_volte_provision_rsp *data = (sit_misc_set_volte_provision_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_SET_VOLTE_PROVISION_UPDATE) {
            return (data->result == 0x00) ? 0 : 1;
        }
    }
    return 0;
}

/*
 * ProtocolMiscGetStackStatusAdapter
 */
int ProtocolMiscGetStackStatusAdapter::GetMode()
{
    if (m_pModemData != NULL) {
        sit_pwr_get_stack_status_rsp *data = (sit_pwr_get_stack_status_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_STACK_STATUS) {
            return (data->mode == 0x00) ? SIT_PWR_STATCK_DISABLE : SIT_PWR_STATCK_ENABLE;
        }
    }
    return 0;
}

/**
 * ProtocolOemModemInfoAdapter
 */
int ProtocolOemModemInfoAdapter::GetCommandType() const
{
    if (m_pModemData != NULL) {
        sit_oem_nw_info_rsp *resp = (sit_oem_nw_info_rsp *)m_pModemData->GetRawData();
        if (resp != NULL && resp->hdr.id == SIT_OEM_NW_INFO) {
            return resp->command;
        }
    }
    return -1;
}

unsigned int ProtocolOemModemInfoAdapter::GetSize() const
{
    if (m_pModemData != NULL) {
        sit_oem_nw_info_rsp *resp = (sit_oem_nw_info_rsp *)m_pModemData->GetRawData();
        if (resp != NULL && resp->hdr.id == SIT_OEM_NW_INFO) {
            return resp->length;
        }
    }
    return 0;
}

void *ProtocolOemModemInfoAdapter::GetData()
{
    if (m_pModemData != NULL) {
        sit_oem_nw_info_rsp *resp = (sit_oem_nw_info_rsp *)m_pModemData->GetRawData();
        if (resp != NULL && resp->hdr.id == SIT_OEM_NW_INFO) {
            return resp->data;
        }
    }
    return NULL;
}

/**
 * ProtocolOemModemInfoIndAdapter
 */
int ProtocolOemModemInfoIndAdapter::GetCommandType() const
{
    if (m_pModemData != NULL) {
        sit_oem_nw_info_ind *resp = (sit_oem_nw_info_ind *)m_pModemData->GetRawData();
        if (resp != NULL && resp->hdr.id == SIT_OEM_IND_NW_INFO) {
            return resp->command;
        }
    }
    return -1;
}

unsigned int ProtocolOemModemInfoIndAdapter::GetSize() const
{
    if (m_pModemData != NULL) {
        sit_oem_nw_info_ind *resp = (sit_oem_nw_info_ind *)m_pModemData->GetRawData();
        if (resp != NULL && resp->hdr.id == SIT_OEM_IND_NW_INFO) {
            return resp->length;
        }
    }
    return 0;
}

void *ProtocolOemModemInfoIndAdapter::GetData()
{
    if (m_pModemData != NULL) {
        sit_oem_nw_info_ind *resp = (sit_oem_nw_info_ind *)m_pModemData->GetRawData();
        if (resp != NULL && resp->hdr.id == SIT_OEM_IND_NW_INFO) {
            return resp->data;
        }
    }
    return NULL;
}

/**
 * ProtocolOemSwitchModemFunctionAdapter
 */
BYTE ProtocolOemSwitchModemFunctionAdapter::GetResult() const
{
    if (m_pModemData != NULL) {
        sit_oem_set_func_switch_rsp *resp = (sit_oem_set_func_switch_rsp *)m_pModemData->GetRawData();
        if (resp != NULL && resp->hdr.id == SIT_OEM_SET_FUNC_SWITCH_REQ) {
            return resp->result;
        }
    }
    return -1;
}

/**
 * ProtocolMiscSetSelflogAdapter
 */
int ProtocolMiscSetSelflogAdapter::GetSelflogResult()
{
    if (m_pModemData != NULL) {
        sit_misc_set_selflog_rsp *data = (sit_misc_set_selflog_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_SET_SELFLOG) {
            return data->result;
        }
    }
    return 0;
}

/**
 * ProtocolMiscGetSelflogStatusAdapter
 */
int ProtocolMiscGetSelflogStatusAdapter::GetSelflogStatus()
{
    if (m_pModemData != NULL) {
        sit_misc_get_selflog_status_rsp *data = (sit_misc_get_selflog_status_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_SELFLOG_STATUS) {
            return data->status;
        }
    }
    return 0;
}

/**
 * ProtocolMiscSelflogStatusAdapter
 */
BYTE ProtocolMiscSelflogStatusAdapter::GetIndSelflogStatus()
{
    if (m_pModemData != NULL) {
        sit_misc_selflog_status_ind *data = (sit_misc_selflog_status_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_IND_SELFLOG_STATUS) ) {
            return data->status;
        }
    }
    return 0;
}

/**
 * ProtocolOemGetCqiInfoAdapter
 */
INT16 ProtocolOemGetCqiInfoAdapter::GetCqiType() const
{
    if (m_pModemData != NULL) {
        sit_oem_get_cqi_info_rsp *data = (sit_oem_get_cqi_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_OEM_GET_CQI_INFO) ) {
            return data->type;
        }
    }
    return 0;
}
INT16 ProtocolOemGetCqiInfoAdapter::GetCqiInfo0() const
{
    if (m_pModemData != NULL) {
        sit_oem_get_cqi_info_rsp *data = (sit_oem_get_cqi_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_OEM_GET_CQI_INFO) ) {
            return data->cqi_info0;
        }
    }
    return 0;
}
INT16 ProtocolOemGetCqiInfoAdapter::GetCqiInfo1() const
{
    if (m_pModemData != NULL) {
        sit_oem_get_cqi_info_rsp *data = (sit_oem_get_cqi_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_OEM_GET_CQI_INFO) ) {
            return data->cqi_info1;
        }
    }
    return 0;
}
INT16 ProtocolOemGetCqiInfoAdapter::GetRi() const
{
    if (m_pModemData != NULL) {
        sit_oem_get_cqi_info_rsp *data = (sit_oem_get_cqi_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_OEM_GET_CQI_INFO) ) {
            return data->ri;
        }
    }
    return 0;
}

/**
 * ProtocolMiscSetTcsFciAdapter
 */
int ProtocolMiscSetTcsFciAdapter::GetResult() const
{
    if (m_pModemData != NULL) {
        sit_oem_set_tcs_fci_rsp *data = (sit_oem_set_tcs_fci_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_SET_TCS_FCI_REQ) {
            return data->result;
        }
    }
    return 0;
}

/**
 * ProtocolMiscGetTcsFciAdapter
 */
const char * ProtocolMiscGetTcsFciAdapter::GetFci() const
{
    if (m_pModemData != NULL) {
        sit_oem_get_tcs_fci_info_rsp *data = (sit_oem_get_tcs_fci_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_OEM_GET_TCS_FCI_INFO) ) {
            return (const char *)data->fci;
        }
    }
    return NULL;
}

/**
 * ProtocolCaBandwidthFilterIndAdapter
 */
int ProtocolCaBandwidthFilterIndAdapter::GetCaConfig() const
{
    if (m_pModemData != NULL) {
        sit_oem_ca_bw_filter_ind *data = (sit_oem_ca_bw_filter_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_OEM_IND_CA_BW_FILTER) ) {
            return data->ca_config > 0 ? 1 : 0;
        }
    }
    return 0;
}

int ProtocolCaBandwidthFilterIndAdapter::GetNRB() const
{
    if (m_pModemData != NULL) {
        sit_oem_ca_bw_filter_ind *data = (sit_oem_ca_bw_filter_ind *)m_pModemData->GetRawData();
        if (data != NULL && (data->hdr.id == SIT_OEM_IND_CA_BW_FILTER) ) {
            return data->num_resource_block;
        }
    }
    return 0;
}

/**
 * ProtocolMiscSetModemLogDumpAdapter
 */
int ProtocolMiscSetModemLogDumpAdapter::GetResult() const
{
    if (m_pModemData != NULL) {
        sit_misc_set_modem_log_dump_rsp *data = (sit_misc_set_modem_log_dump_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_SET_MODEM_LOG_DUMP) {
            return data->result;
        }
    }
    return 0;
}

/*
 * ProtocolMiscCurrentLinkCapacityEstimate
 */
static void InitLinkCapaEstimate(CURRENT_LINK_CAPA_ESTIMATE &value)
{
    value.dl_capa_kbps = 0;
    value.ul_capa_kbps = 0;
}

ProtocolMiscCurrentLinkCapacityEstimate::ProtocolMiscCurrentLinkCapacityEstimate(const ModemData *pModemData) : ProtocolIndAdapter(pModemData)
{
    InitLinkCapaEstimate(mCurLinkCapaEstimate);

    if(m_pModemData != NULL) {
        sit_misc_current_link_capa_estimate_ind *data = (sit_misc_current_link_capa_estimate_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_CURRENT_LINK_CAPACITY_ESTIMATE) {
            mCurLinkCapaEstimate.dl_capa_kbps = data->dl_capa_kbps;
            mCurLinkCapaEstimate.ul_capa_kbps = data->ul_capa_kbps;
        }
    }
}

void ProtocolMiscCurrentLinkCapacityEstimate::GetCurrentLinkCapaEstimate(void *data)
{
    *((int *)data+0) = mCurLinkCapaEstimate.dl_capa_kbps;
    *((int *)data+1) = mCurLinkCapaEstimate.ul_capa_kbps;
}

/*
 * ProtocolMiscEndcCapabilityIndAdapter
 */
int ProtocolMiscEndcCapabilityIndAdapter::GetCapability() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_oem_endc_capability_ind *data = (sit_oem_endc_capability_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_IND_ENDC_CAPABILITY) {
            ret = (int)data->endc_capability;
        }
    }
    return ret;
}

int ProtocolMiscEndcCapabilityIndAdapter::GetCause() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_oem_endc_capability_ind *data = (sit_oem_endc_capability_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_IND_ENDC_CAPABILITY) {
            ret = (int)data->cause;
        }
    }
    return ret;
}


/**
 * ProtocolMiscSetSelflogProfileAdapter
 */
int ProtocolMiscSetSelflogProfileAdapter::GetResult() const
{
    if (m_pModemData != NULL) {
        sit_set_selflog_profile_rsp *data = (sit_set_selflog_profile_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_SET_SELFLOG_PROFILE) {
            return data->result;
        }
    }
    return 0;
}

/**
 * ProtocolMiscSetForbidLteCellAdapter
 */
int ProtocolMiscSetForbidLteCellAdapter::GetResult() const
{
    if (m_pModemData != NULL) {
        sit_oem_set_forbid_lte_cell_rsp *data = (sit_oem_set_forbid_lte_cell_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_OEM_SET_FORBID_LTE_CELL) {
            return data->result;
        }
    }
    return 0;
}
