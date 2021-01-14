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
 * protocolnetadapter.cpp
 *
 *  Created on: 2014. 6. 27.
 *      Author: sungwoo48.choi
 */
#include <telephony/librilutils.h>
#include "mcctable.h"
#include "operatortable.h"
#include "protocolnetadapter.h"
#include "rillog.h"
#include "systemproperty.h"
#include "ts25table.h"
#include "uplmnselector.h"
#include "networkutils.h"

#include <string>

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

static bool debug = true;

static int ConvertRegStateValue(int regStatus)
{
    switch (regStatus) {
        case SIT_NET_REG_STATE_REGISTERED:
            return REGISTERED_HOME;
        case SIT_NET_REG_STATE_NOT_REG_SEARCHING:
            return SEARCHING;
        case SIT_NET_REG_STATE_DENIED:
            return DENIED;
        case SIT_NET_REG_STATE_UNKNOWN:
            return UNKNOWN;
        case SIT_NET_REG_STATE_ROAMING:
            return REGISTERED_ROAMING;

        case SIT_NET_REG_STATE_NOT_REG_NO_SEARCH_EMGCALL:
            return NOT_REGISTERED_EMERGENCY_ONLY;
        case SIT_NET_REG_STATE_NOT_REG_SEARCHING_EMGCALL:
            return SEARCHING_EMERGENCY_ONLY;
        case SIT_NET_REG_STATE_DENIED_EMGCALL:
            return DENIED_EMERGENCY_ONLY;
        case SIT_NET_REG_STATE_UNKNOWN_EMGCALL:
            return UNKNOWN_EMERGENCY_ONLY;

        case SIT_NET_REG_STATE_DENIED_ROAMING:
            return DENIED_ROAMING;
        default:
            return NOT_REGISTERED;
    }
}

static int ConvertRadioTechValue(int rat)
{
    switch (rat) {
        case SIT_RAT_TYPE_GPRS: return RADIO_TECH_GPRS;
        case SIT_RAT_TYPE_EDGE: return RADIO_TECH_EDGE;
        case SIT_RAT_TYPE_UMTS: return RADIO_TECH_UMTS;
        case SIT_RAT_TYPE_IS95A:  return RADIO_TECH_IS95A;
        case SIT_RAT_TYPE_IS95B:  return RADIO_TECH_IS95B;
        case SIT_RAT_TYPE_1xRTT:  return RADIO_TECH_1xRTT;
        case SIT_RAT_TYPE_EVDO_0: return RADIO_TECH_EVDO_0;
        case SIT_RAT_TYPE_EVDO_A: return RADIO_TECH_EVDO_A;
        case SIT_RAT_TYPE_HSDPA: return RADIO_TECH_HSDPA;
        case SIT_RAT_TYPE_HSUPA: return RADIO_TECH_HSUPA;
        case SIT_RAT_TYPE_HSPA: return RADIO_TECH_HSPA;
        case SIT_RAT_TYPE_EVDO_B: return RADIO_TECH_EVDO_B;
        case SIT_RAT_TYPE_EHRPD: return RADIO_TECH_EHRPD;
        case SIT_RAT_TYPE_LTE: return RADIO_TECH_LTE;
        case SIT_RAT_TYPE_HSPAP: return RADIO_TECH_HSPAP;
        case SIT_RAT_TYPE_GSM: return RADIO_TECH_GSM;
        case SIT_RAT_TYPE_IWLAN: return RADIO_TECH_IWLAN;
        case SIT_RAT_TYPE_TD_SCDMA: return RADIO_TECH_TD_SCDMA;
        case SIT_RAT_TYPE_HSPADCPLUS: return RADIO_TECH_HSPAP;
        case SIT_RAT_TYPE_LTE_CA: return RADIO_TECH_LTE_CA;
        case SIT_RAT_TYPE_5G: return RADIO_TECH_NR;
        default:
            return RADIO_TECH_UNKNOWN;
    }
}

static const char *ConvertRadioStateToString(int radioState)
{
    switch (radioState) {
    case SIT_PWR_RADIO_STATE_INITIALIZED:
        return "SIT_PWR_RADIO_STATE_INITIALIZED";
    case SIT_PWR_RADIO_STATE_STOP_NETWORK:
        return "SIT_PWR_RADIO_STATE_STOP_NETWORK";
    case SIT_PWR_RADIO_STATE_START_NETWORK:
        return "SIT_PWR_RADIO_STATE_START_NETWORK";
    case SIT_PWR_RADIO_STATE_POWER_OFF:
        return "SIT_PWR_RADIO_STATE_POWER_OFF";
    }
    return "SIT_PWR_RADIO_STATE_UNKNOWN";
}

static const char *ConvertPreferredNetTypeToString(int netType)
{
    switch (netType) {
    case SIT_NET_PREF_NET_TYPE_GSM_WCDMA:
        return "SIT_NET_PREF_NET_TYPE_GSM_WCDMA";
    case SIT_NET_PREF_NET_TYPE_GSM_ONLY:
        return "SIT_NET_PREF_NET_TYPE_GSM_ONLY";
    case SIT_NET_PREF_NET_TYPE_WCDMA:
        return "SIT_NET_PREF_NET_TYPE_WCDMA";
    case SIT_NET_PREF_NET_TYPE_GSM_WCDMA_AUTO:
        return "SIT_NET_PREF_NET_TYPE_GSM_WCDMA_AUTO";
    case SIT_NET_PREF_NET_TYPE_CDMA_EVDO_AUTO:
        return "SIT_NET_PREF_NET_TYPE_CDMA_EVDO_AUTO";
    case SIT_NET_PREF_NET_TYPE_CDMA_ONLY:
        return "SIT_NET_PREF_NET_TYPE_CDMA_ONLY";
    case SIT_NET_PREF_NET_TYPE_EVDO_ONLY:
        return "SIT_NET_PREF_NET_TYPE_EVDO_ONLY";
    case SIT_NET_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        return "SIT_NET_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO";
    case SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO:
        return "SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO";
    case SIT_NET_PREF_NET_TYPE_LTE_GSM_WCDMA:
        return "SIT_NET_PREF_NET_TYPE_LTE_GSM_WCDMA";
    case SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA:
        return "SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA";
    case SIT_NET_PREF_NET_TYPE_LTE_ONLY:
        return "SIT_NET_PREF_NET_TYPE_LTE_ONLY";
    case SIT_NET_PREF_NET_TYPE_LTE_WCDMA:
        return "SIT_NET_PREF_NET_TYPE_LTE_WCDMA";
    case SIT_NET_PREF_NET_TYPE_NR_ONLY:
        return "SIT_NET_PREF_NET_TYPE_NR_ONLY";
    case SIT_NET_PREF_NET_TYPE_NR_LTE:
        return "SIT_NET_PREF_NET_TYPE_NR_LTE";
    case SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO:
        return "SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO";
    case SIT_NET_PREF_NET_TYPE_NR_LTE_GSM_WCDMA:
        return "SIT_NET_PREF_NET_TYPE_NR_LTE_GSM_WCDMA";
    case SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
        return "SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA";
    case SIT_NET_PREF_NET_TYPE_NR_LTE_WCDMA:
        return "SIT_NET_PREF_NET_TYPE_NR_LTE_WCDMA";
    } // end switch ~
    return "UNSUPPORTED_PREF_NET_TYPE";
}

#ifdef SUPPORT_CDMA
static const char *ConvertCdmaHybridModeToString(int hybridMode)
{
    switch (hybridMode) {
    case HYBRID_MODE_1X_HRPD:
        return "HYBRID_MODE_1X_HRPD";
    case HYBRID_MODE_1X_ONLY:
        return "HYBRID_MODE_1X_ONLY";
    case HYBRID_MODE_HRPD_ONLY:
        return "HYBRID_MODE_HRPD_ONLY";
    case HYBRID_MODE_1X_EHRPD:
        return "HYBRID_MODE_1X_EHRPD";
    case HYBRID_MODE_EHRPD_ONLY:
        return "HYBRID_MODE_EHRPD_ONLY";
    }
    return "UNSUPPORTED_CDMA_HYBRID_MODE";
}
#endif


/**
 * ProtocolNetVoiceRegStateAdapter
 */
int ProtocolNetVoiceRegStateAdapter::GetRegState() const
{
    int regStatus = SIT_NET_REG_STATE_UNKNOWN;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            regStatus = ConvertRegStateValue(data->reg_state);
        }
    }

    return regStatus;
}

int ProtocolNetVoiceRegStateAdapter::GetRejectCause() const
{
    int rejCause = SIT_NET_REJ_CAUSE_GENERAL;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            rejCause = data->rej_cause;
        }
    }

    return rejCause;
}

int ProtocolNetVoiceRegStateAdapter::GetRadioTech() const
{
    int rat = (int)RADIO_TECH_UNKNOWN;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            rat = ConvertRadioTechValue(data->rat);
        }
    }

    return rat;
}

int ProtocolNetVoiceRegStateAdapter::GetLAC() const
{
    int lac = 0xFFFF;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            lac = data->gw_lac & 0xFFFF;
            // AcT is LTE, use TAC
            if (data->rat == SIT_RAT_TYPE_LTE || data->rat == SIT_RAT_TYPE_LTE_CA || data->rat == SIT_RAT_TYPE_5G) {
                return data->lte_tac;
            }
        }
    }

    return lac;
}

int ProtocolNetVoiceRegStateAdapter::GetCellId() const
{
    int cid = 0xFFFFFFFF;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            // AcT is LTE, use ECI
            if (data->rat == SIT_RAT_TYPE_LTE || data->rat == SIT_RAT_TYPE_LTE_CA || data->rat == SIT_RAT_TYPE_5G) {
                return data->lte_eci;
            }
            cid = data->gw_cid;
        }
    }

    return cid;
}

int ProtocolNetVoiceRegStateAdapter::GetPSC() const
{
    int psc = 0xFF;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            // AcT is LTE, use PCID
            if (data->rat == SIT_RAT_TYPE_LTE || data->rat == SIT_RAT_TYPE_LTE_CA || data->rat == SIT_RAT_TYPE_5G) {
                return data->lte_pcid;
            }
            psc = (int)(data->gw_psc & 0xFF);
        }
    }

    return psc;
}

int ProtocolNetVoiceRegStateAdapter::GetTAC() const
{
    int tac = 0;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            tac = data->lte_tac;
        }
    }

    return tac;
}

int ProtocolNetVoiceRegStateAdapter::GetPCID() const
{
    int pcid = 0;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            pcid = data->lte_pcid;
        }
    }

    return pcid;
}

int ProtocolNetVoiceRegStateAdapter::GetECI() const
{
    int eci = 0xFFFFFFFF;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            eci = data->lte_eci;
        }
    }

    return eci;
}

#ifdef SUPPORT_CDMA
int ProtocolNetVoiceRegStateAdapter::GetStationId() const
{
    int stationId = 0;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            stationId = data->basestationid;
        }
    }
    return stationId;
}

int ProtocolNetVoiceRegStateAdapter::GetStationLat() const
{
    int stationLat = 0;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            stationLat = data->basestation_latitude;
        }
    }
    return stationLat;
}

int ProtocolNetVoiceRegStateAdapter::GetStationLong() const
{
    int stationLong = 0;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            stationLong = data->basestation_longitude;
        }
    }
    return stationLong;
}

int ProtocolNetVoiceRegStateAdapter::GetConCurrent() const
{
    int conCur = 0;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            conCur = data->concurrent;
        }
    }
    return conCur;
}

int ProtocolNetVoiceRegStateAdapter::GetSystemId() const
{
    int systemId = 0;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            systemId = data->sid;
        }
    }
    return systemId;
}

int ProtocolNetVoiceRegStateAdapter::GetNetworkId() const
{
    int networkId = 0;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            networkId = data->nid;
        }
    }
    return networkId;
}

int ProtocolNetVoiceRegStateAdapter::GetRoamingInd() const
{
    int roamingInd = 0;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            roamingInd = data->roamingindicator;
        }
    }
    return roamingInd;
}

int ProtocolNetVoiceRegStateAdapter::GetRegPrl() const
{
    int regPrl = 0;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            regPrl = data->registered_prl;
        }
    }
    return regPrl;
}

int ProtocolNetVoiceRegStateAdapter::GetRoamingIndPrl() const
{
    int roamingPrl = 0;
    if (m_pModemData != NULL) {
        sit_net_get_cs_reg_state_rsp *data = (sit_net_get_cs_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CS_REG_STATE) {
            roamingPrl = data->roaming_indi_prl;
        }
    }
    return roamingPrl;
}
#endif //SUPPORT_CDMA

int ProtocolNetVoiceRegStateAdapter::getChannelNumber() const {
    int channelNumber = 0;
    if (m_pModemData != NULL) {
        if (GetId() == SIT_GET_CS_REG_STATE &&
            GetParameter() != NULL &&
            GetParameterLength() >= sizeof(sit_cs_reg_state_v1_0)) {
            sit_cs_reg_state_v1_0 *data = (sit_cs_reg_state_v1_0 *)GetParameter();
            return data->channel;
        }
    }
    return channelNumber;
}

/**
 * ProtocolNetDataRegStateAdapter
 */
int ProtocolNetDataRegStateAdapter::GetRegState() const
{
    int regStatus = NOT_REGISTERED;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            regStatus = ConvertRegStateValue(data->reg_state);
        }
    }
    return regStatus;
}

int ProtocolNetDataRegStateAdapter::GetRejectCause() const
{
    int regCause = SIT_NET_REJ_CAUSE_GENERAL;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            regCause = data->rej_cause;
        }
    }
    return regCause;
}
int ProtocolNetDataRegStateAdapter::GetMaxSDC() const
{
    int sdc = 4;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            sdc = data->max_sdc;
        }
    }
    return sdc;
}
int ProtocolNetDataRegStateAdapter::GetRadioTech() const
{
    int rat = (int)RADIO_TECH_UNKNOWN;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            rat = ConvertRadioTechValue(data->rat);
        }
    }
    return rat;
}
int ProtocolNetDataRegStateAdapter::GetLAC() const
{
    int lac = 0;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            // AcT is LTE, use TAC
            if (data->rat == SIT_RAT_TYPE_LTE || data->rat == SIT_RAT_TYPE_LTE_CA || data->rat == SIT_RAT_TYPE_5G) {
                return data->lte_tac;
            }
            lac = data->gw_lac & 0xFFFF;
        }
    }

    return lac;
}
int ProtocolNetDataRegStateAdapter::GetCellId() const
{
    int cid = 0;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            // AcT is LTE, use ECI
            if (data->rat == SIT_RAT_TYPE_LTE || data->rat == SIT_RAT_TYPE_LTE_CA || data->rat == SIT_RAT_TYPE_5G) {
                return data->lte_eci;
            }
            cid = data->gw_cid;
        }
    }

    return cid;
}
int ProtocolNetDataRegStateAdapter::GetPSC() const
{
    int psc = 0xFF;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            // AcT is LTE, use PCID
            if (data->rat == SIT_RAT_TYPE_LTE || data->rat == SIT_RAT_TYPE_LTE_CA || data->rat == SIT_RAT_TYPE_5G) {
                return data->lte_pcid;
            }
            psc = (int)(data->gw_psc & 0xFF);
        }
    }

    return psc;
}
int ProtocolNetDataRegStateAdapter::GetTAC() const
{
    int tac = 0;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            tac = data->lte_tac;
        }
    }

    return tac;
}
int ProtocolNetDataRegStateAdapter::GetPCID() const
{
    int pcid = 0;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            pcid = data->lte_pcid;
        }
    }

    return pcid;
}
int ProtocolNetDataRegStateAdapter::GetECI() const
{
    int eci = 0xFFFFFFFF;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            eci = data->lte_eci;
        }
    }

    return eci;
}
int ProtocolNetDataRegStateAdapter::GetCSGID() const
{
    int csgid = 0xFFFFFFFF;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            csgid = data->lte_csgid;
        }
    }

    return csgid;
}
int ProtocolNetDataRegStateAdapter::GetTADV() const
{
    int tadv = 0xFFFFFFFF;
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            tadv = data->lte_tadv;
        }
    }

    return tadv;
}

bool ProtocolNetDataRegStateAdapter::IsVolteServiceAvailabe() const
{
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            return (data->volte_service == SERVICE_AVAILABLE);
        }
    }
    return false;
}

bool ProtocolNetDataRegStateAdapter::IsEmergencyCallServiceAvailable() const
{
    if (m_pModemData != NULL) {
        sit_net_get_ps_reg_state_rsp *data = (sit_net_get_ps_reg_state_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_REG_STATE) {
            return (data->emc_service == SERVICE_AVAILABLE);
        }
    }
    return false;
}

int ProtocolNetDataRegStateAdapter::getChannelNumber() const {
    int channelNumber = 0;
    if (m_pModemData != NULL) {
        if (GetId() == SIT_GET_PS_REG_STATE &&
            GetParameter() != NULL &&
            GetParameterLength() >= sizeof(sit_ps_reg_state_v1_0)) {
            sit_ps_reg_state_v1_0 *data = (sit_ps_reg_state_v1_0 *)GetParameter();
            return data->channel;
        }
    }
    return channelNumber;
}

bool ProtocolNetDataRegStateAdapter::IsEndcAvailable() const {
    bool ret = false;    // E-UTRA-NR Dual Connectivity (EN-DC) is not supported. (not available)
    if (m_pModemData != NULL) {
        if (GetId() == SIT_GET_PS_REG_STATE &&
                GetParameter() != NULL &&
                GetParameterLength() >= sizeof(sit_ps_reg_state_v1_1)) {
            sit_ps_reg_state_v1_1 *data = (sit_ps_reg_state_v1_1 *)GetParameter();
            return (data->endc == SERVICE_AVAILABLE);
        }
    }
    return ret;
}

bool ProtocolNetDataRegStateAdapter::IsDcNrRestricted() const {
    int ret = false;    // use of dual connectivity with NR is restricted.
    if (m_pModemData != NULL) {
        if (GetId() == SIT_GET_PS_REG_STATE &&
                GetParameter() != NULL &&
                GetParameterLength() >= sizeof(sit_ps_reg_state_v1_1)) {
            sit_ps_reg_state_v1_1 *data = (sit_ps_reg_state_v1_1 *)GetParameter();
            return (data->dcnr_restricted == SERVICE_AVAILABLE);
        }
    }
    return ret;
}

bool ProtocolNetDataRegStateAdapter::IsNrAvailable() const {
    int ret = false;    // NR is not available
    if (m_pModemData != NULL) {
        if (GetId() == SIT_GET_PS_REG_STATE &&
                GetParameter() != NULL &&
                GetParameterLength() >= sizeof(sit_ps_reg_state_v1_1)) {
            sit_ps_reg_state_v1_1 *data = (sit_ps_reg_state_v1_1 *)GetParameter();
            return (data->nr_available == SERVICE_AVAILABLE);
        }
    }
    return ret;
}

/**
 * ProtocolNetOperatorAdapter
 */
#define NET_GR_QTELCOM_NUMERIC              "20209"
#define NET_GR_WIND_NUMERIC                 "20210"
#define NET_GR_QTELCOM_LONG_ALPHA_EONS      "Q-TELCOM"
#define NET_GR_QTELCOM_SHORT_ALPHA_EONS     "Q-TELCOM"
#define MCC_BRAZIL                          724

static void UpdateVendorCustomOperatorName(
        const char *simOperatorNumeric, const char *operatorNumeric,
        char *longAlpha, char *shortAlpha) {
    // vendor customized
    // 1. GR Q-TELCOM(20209) had been disbanded in May 2007.
    //    Q-TELCOM bands are currently used by WIND(20210)
    if (!TextUtils::IsEmpty(simOperatorNumeric)
        && TextUtils::Equals(simOperatorNumeric, NET_GR_QTELCOM_NUMERIC)) {

        if (TextUtils::Equals(operatorNumeric, NET_GR_WIND_NUMERIC)) {
            if (longAlpha != NULL) {
                int len = strlen(NET_GR_QTELCOM_LONG_ALPHA_EONS);
                strncpy(longAlpha, NET_GR_QTELCOM_LONG_ALPHA_EONS, len);
                *(longAlpha + len) = 0;
            }

            if (shortAlpha != NULL) {
                int len = strlen(NET_GR_QTELCOM_SHORT_ALPHA_EONS);
                strncpy(shortAlpha, NET_GR_QTELCOM_SHORT_ALPHA_EONS, len);
                *(shortAlpha + len) = 0;
            }
        }
    }
}

void ProtocolNetOperatorAdapter::Init()
{
    if (m_pModemData != NULL && GetErrorCode() == RCM_E_SUCCESS) {
        sit_net_get_operator_rsp *data = (sit_net_get_operator_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_OPERATOR) {
            if (*data->plmn != 0) {
                memcpy(m_szPlmn, data->plmn, 6);
                if (m_szPlmn[5] == '#') {
                    m_szPlmn[5] = 0;
                }
                if (m_szPlmn[0] == '#') {
                    m_szPlmn[0] = 0;
                }
            }

            if (*data->short_name != 0) {
                strncpy(m_szShortPlmn, data->short_name, MAX_SHORT_NAME_LEN);
            }

            if (*data->long_name != 0) {
                strncpy(m_szLongPlmn, data->long_name, MAX_FULL_NAME_LEN);
            }

            if(GetParameter() != NULL && GetParameterLength() >= sizeof(sit_net_operator_v1_0)) {
                sit_net_operator_v1_0 *param = (sit_net_operator_v1_0 *)GetParameter();
                m_regState = param->reg_state;
            }

            if(GetParameter() != NULL && GetParameterLength() >= sizeof(sit_net_operator_v1_1)) {
                sit_net_operator_v1_1 *param = (sit_net_operator_v1_1 *)GetParameter();
                m_lac = param->lac;
            }
        }
    }
}

ProtocolNetOperatorAdapter::ProtocolNetOperatorAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
    memset(m_szPlmn, 0, sizeof(m_szPlmn));
    memset(m_szShortPlmn, 0, sizeof(m_szShortPlmn));
    memset(m_szLongPlmn, 0, sizeof(m_szLongPlmn));
    m_regState = OPERATOR_REG_UNKNOWN;
    m_lac = -1;

    Init();
}

const char *ProtocolNetOperatorAdapter::GetPlmn() const
{
    return (m_szPlmn[0] == 0 ? NULL : m_szPlmn);
}
const char *ProtocolNetOperatorAdapter::GetShortPlmn() const
{
    return (m_szShortPlmn[0] == 0 ? NULL : m_szShortPlmn);
}
const char *ProtocolNetOperatorAdapter::GetLongPlmn() const
{
    return (m_szLongPlmn[0] == 0 ? NULL : m_szLongPlmn);
}
int ProtocolNetOperatorAdapter::GetRegState() const
{
    return m_regState;
}
int ProtocolNetOperatorAdapter::GetLac() const
{
    return m_lac;
}

/**
 * ProtocolNetSelModeAdapter
 */
int ProtocolNetSelModeAdapter::GetNetworkSelectionMode() const
{
    if (m_pModemData != NULL) {
        sit_net_get_network_mode_rsp *data = (sit_net_get_network_mode_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_NTW_MODE) {
            int mode = data->network_mode & 0xFF;
            RilLogV("NetworkSelectionMode=%s(0x%02x)", mode == 0 ? "Automatic" : "Manual", mode);
            return mode;
        }
    }
    return SIT_NET_NETWORK_MODE_AUTOMATIC;
}

/**
 * ProtocolNetRadioStateRespAdapter
 */
int ProtocolNetRadioStateRespAdapter::GetRadioState() const {
    if (m_pModemData != NULL) {
        sit_pwr_get_radio_power_rsp *data = (sit_pwr_get_radio_power_rsp *) m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_RADIO_POWER) {
            RilLogV("RadioState=%s(0x%02x)", ConvertRadioStateToString(data->radio_state), data->radio_state);
            return ConvertRilRadioState(data->radio_state);
        }
    }
    return -1;
}

int ProtocolNetRadioStateRespAdapter::ConvertRilRadioState(int radioState) const {
    if (radioState == 0) {
        return RADIO_STATE_OFF;
    }
    else if (radioState >= 2 && radioState <= 10) {
        return RADIO_STATE_ON;
    }
    return RADIO_STATE_UNAVAILABLE;
}

/**
 * ProtocolRadioStateAdapter
 */
int ProtocolRadioStateAdapter::GetRadioState() const
{
    if (m_pModemData != NULL) {
        sit_pwr_radio_state_changed_ind *data = (sit_pwr_radio_state_changed_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_RADIO_STATE_CHANGED) {
            RilLogV("RadioState=%s(0x%02x)", ConvertRadioStateToString(data->radio_state), data->radio_state);
            return ConvertRilRadioState(data->radio_state);
        }
    }
    return -1;
}

int ProtocolRadioStateAdapter::ConvertRilRadioState(int radioState) const {
    switch (radioState) {
    case SIT_PWR_RADIO_STATE_INITIALIZED:
    case SIT_PWR_RADIO_STATE_STOP_NETWORK:
        return RADIO_STATE_OFF;
    case SIT_PWR_RADIO_STATE_START_NETWORK:
        return RADIO_STATE_ON;
    case SIT_PWR_RADIO_STATE_POWER_OFF:
        return RADIO_STATE_UNAVAILABLE;
    default:
        return RADIO_STATE_UNAVAILABLE;
    }
}

static int ConvertPrefNetworkTypeToRil(int netType)
{
    // networkTypeMap[i][0] : RIL type
    // networkTypeMap[i][1] : SIT type
    static int networkTypeMap[][2] = {
        { PREF_NET_TYPE_GSM_WCDMA, SIT_NET_PREF_NET_TYPE_GSM_WCDMA},
        { PREF_NET_TYPE_GSM_ONLY, SIT_NET_PREF_NET_TYPE_GSM_ONLY},
        { PREF_NET_TYPE_WCDMA, SIT_NET_PREF_NET_TYPE_WCDMA},
        { PREF_NET_TYPE_GSM_WCDMA_AUTO, SIT_NET_PREF_NET_TYPE_GSM_WCDMA_AUTO},
        { PREF_NET_TYPE_CDMA_EVDO_AUTO, SIT_NET_PREF_NET_TYPE_CDMA_EVDO_AUTO},
        { PREF_NET_TYPE_CDMA_ONLY, SIT_NET_PREF_NET_TYPE_CDMA_ONLY},
        { PREF_NET_TYPE_EVDO_ONLY, SIT_NET_PREF_NET_TYPE_EVDO_ONLY},
        { PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO, SIT_NET_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO},
        { PREF_NET_TYPE_LTE_CDMA_EVDO, SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO},
        { PREF_NET_TYPE_LTE_GSM_WCDMA, SIT_NET_PREF_NET_TYPE_LTE_GSM_WCDMA},
        { PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA, SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA},
        { PREF_NET_TYPE_LTE_ONLY, SIT_NET_PREF_NET_TYPE_LTE_ONLY},
        { PREF_NET_TYPE_LTE_WCDMA, SIT_NET_PREF_NET_TYPE_LTE_WCDMA}, // 12
        { PREF_NET_TYPE_TD_SCDMA_ONLY, SIT_NET_PREF_NET_TYPE_TDSCDMA_ONLY},
        { PREF_NET_TYPE_TD_SCDMA_WCDMA, SIT_NET_PREF_NET_TYPE_TDSCDMA_WCDMA},
        { PREF_NET_TYPE_TD_SCDMA_LTE, SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA},
        { PREF_NET_TYPE_TD_SCDMA_GSM, SIT_NET_PREF_NET_TYPE_TDSCDMA_GSM},
        { PREF_NET_TYPE_TD_SCDMA_GSM_LTE, SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_GSM},
        { PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA, SIT_NET_PREF_NET_TYPE_TDSCDMA_GSM_WCDMA},
        { PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE, SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_GSM_WCDMA},
        { PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO, SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA_EVDO_GSM_WCDMA},
        { PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA, SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA}, //22
        { PREF_NET_TYPE_NR_ONLY, SIT_NET_PREF_NET_TYPE_NR_ONLY},
        { PREF_NET_TYPE_NR_LTE, SIT_NET_PREF_NET_TYPE_NR_LTE},
        { PREF_NET_TYPE_NR_LTE_CDMA_EVDO, SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO},
        { PREF_NET_TYPE_NR_LTE_GSM_WCDMA, SIT_NET_PREF_NET_TYPE_NR_LTE_GSM_WCDMA},
        { PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA, SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA},
        { PREF_NET_TYPE_NR_LTE_WCDMA, SIT_NET_PREF_NET_TYPE_NR_LTE_WCDMA}, //28
        { PREF_NET_TYPE_NR_LTE_TDSCDMA, SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA},
        { PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM, SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM},
        { PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA, SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA},
        { PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA, SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA},
        { PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA, SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA}, //33
        { PREF_NET_TYPE_TD_SCDMA_CDMA, SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA}, // 50
        { PREF_NET_TYPE_TD_SCDMA_CDMA_NO_EVDO, SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA_NO_EVDO},
        { PREF_NET_TYPE_TD_SCDMA_CDMA_EVDO_LTE, SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_CDMA_EVDO},
        { PREF_NET_TYPE_TD_SCDMA_EVDO_NO_CDMA, SIT_NET_PREF_NET_TYPE_TDSCDMA_EVDO_NO_CDMA},
    };
    int size = sizeof(networkTypeMap) / sizeof(networkTypeMap[0]);
    int rilNetType = -1;
    for (int i = 0; i < size; i++) {
        if (networkTypeMap[i][1] == netType) {
            rilNetType = networkTypeMap[i][0];
            break;
        }
    } // end for i ~

    if (rilNetType < 0) {
        RilLogW("no matched network type. current netType=%d", netType);
        rilNetType = PREF_NET_TYPE_GSM_WCDMA;
    }

    return rilNetType;
}

/**
 * ProtocolNetPrefNetTypeAdapter
 */
int ProtocolNetPrefNetTypeAdapter::GetPreferredNetworkType() const
{
    if (m_pModemData != NULL) {
        sit_net_get_pref_network_rsp *data = (sit_net_get_pref_network_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PREFERRED_NTW_TYPE) {
            RilLogV("PreferredNetworkType=%s(0x%02x)", ConvertPreferredNetTypeToString(data->pref_net_type), data->pref_net_type);
            int prefNetType = ConvertPrefNetworkTypeToRil(data->pref_net_type);
            return prefNetType;
        }
    }
    return PREF_NET_TYPE_GSM_WCDMA;
}

/**
 * ProtocolNetBandModeAdapter
 */
ProtocolNetBandModeAdapter::ProtocolNetBandModeAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData), m_count(0)
{
    memset(m_bandMode, 0, sizeof(m_bandMode));
    Init();
}

void ProtocolNetBandModeAdapter::Init()
{
    if (m_pModemData != NULL && GetErrorCode() == RCM_E_SUCCESS) {
        sit_net_get_band_mode_rsp *data = (sit_net_get_band_mode_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_BAND_MODE) {
            m_count = data->band_info_num;
            for (int i = 0; i < m_count; i++) {
                m_bandMode[i] = data->band[i];
            } // end for i ~
        }
    }
}

/**
 * ProtocolNetAvailableNetworkAdapter
 */
ProtocolNetAvailableNetworkAdapter::ProtocolNetAvailableNetworkAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
}

ProtocolNetAvailableNetworkAdapter::~ProtocolNetAvailableNetworkAdapter()
{
}

int ProtocolNetAvailableNetworkAdapter::GetCount()
{
    if (m_pModemData != NULL && GetErrorCode() == RCM_E_SUCCESS) {
        sit_net_get_available_networks_rsp *data = (sit_net_get_available_networks_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_AVAILABLE_NETWORKS) {
            return data->network_info_num;
        }
    }
    return 0;
}

bool ProtocolNetAvailableNetworkAdapter::GetNetwork(NetworkInfo &nwkInfo, int index, const char *simPlmn, char *simSpn)
{
    if (m_pModemData != NULL && GetErrorCode() == RCM_E_SUCCESS) {
        sit_net_get_available_networks_rsp *data = (sit_net_get_available_networks_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_AVAILABLE_NETWORKS) {
            int count = data->network_info_num;
            if (index < 0 || index >= count) {
                return false;
            }

            sit_net_network_info_item *p = &data->network_info[index];
            if (p == NULL) {
                return false;
            }

            // convert SIT RAT to Android RAT
            switch (p->rat) {
            case SIT_RAT_TYPE_IWLAN:
                nwkInfo.rat = RADIO_TECH_UNKNOWN;
                break;
            case SIT_RAT_TYPE_TD_SCDMA:
                nwkInfo.rat = RADIO_TECH_TD_SCDMA;
                break;
            case SIT_RAT_TYPE_HSPADCPLUS:
                nwkInfo.rat = RADIO_TECH_HSPAP;
                break;
            case SIT_RAT_TYPE_LTE_CA:
                nwkInfo.rat = RADIO_TECH_LTE;
                break;
            case SIT_RAT_TYPE_5G:
                nwkInfo.rat = RADIO_TECH_NR;
                break;
            default:
                nwkInfo.rat = p->rat;
                break;
            } // end switch ~

            // copy mcc/mnc
            memset(nwkInfo.plmn, 0, sizeof(nwkInfo.plmn));
            memcpy(nwkInfo.plmn, p->plmn, 6);

            if (nwkInfo.plmn[5] == '#' /*|| mncLength == 2*/) {
                nwkInfo.plmn[5] = 0;
            }

            // check UNKNOW network
            if(MccTable::isUnknowNetwork(nwkInfo.plmn) > 0) return false;

            // PLMN status
            switch (p->plmn_status) {
            case SIT_NET_PLMN_STATUS_AVAILABLE:
                nwkInfo.status = STR_NETWORK_STATUS_AVAILABLE;
                break;
            case SIT_NET_PLMN_STATUS_CURRENT:
                nwkInfo.status = STR_NETWORK_STATUS_CURRENT;
                break;
            case SIT_NET_PLMN_STATUS_FORBIDDEN:
                nwkInfo.status = STR_NETWORK_STATUS_FORBIDDEN;
                break;
            default:
                nwkInfo.status = STR_NETWORK_STATUS_UNKNOWN;
                break;
            } // end switch ~

            // fill Long/Short EONS
            *nwkInfo.shortPlmn = 0;
            *nwkInfo.longPlmn = 0;

            UpdateNetworkNameOfNetworkScanResult(nwkInfo, simPlmn, nwkInfo.plmn, simSpn);
        }
    }

    return true;
}

static void UpdateNetworkNameFromTs25(NetworkInfo &nwkInfo, int mcc, int mnc) {
    TS25Table *ts25table = TS25Table::GetInstance();
    if (ts25table != NULL) {
        TS25Record record = ts25table->GetRecord(mcc, mnc);
        if (record.IsValid()) {
            strncpy(nwkInfo.longPlmn, record.ppcin.c_str(), MAX_FULL_NAME_LEN);
            strncpy(nwkInfo.shortPlmn, record.networkName.c_str(), MAX_SHORT_NAME_LEN);
            RilLogV("[%s]TS2Record(%d/%d/%s/%s)", __FUNCTION__,
                    record.mcc, record.mnc, record.networkName.c_str(), record.ppcin.c_str());
        }
        else {
            // Not found.
            RilLogV("%s Not found for (mcc, mnc) = (%d, %d). Keep the current status.", __FUNCTION__, mcc, mnc);
        }
    }
}

void ProtocolNetAvailableNetworkAdapter::UpdateNetworkNameOfNetworkScanResult(NetworkInfo &nwkInfo,
        const char *simOperatorNumeric, const char *operatorNumeric, const char *simSpn)
{
    int mcc = INT_MAX;
    int mnc = INT_MAX;

    if (TextUtils::IsDigitsOnly(operatorNumeric)) {
        std::string numeric = std::string(operatorNumeric);
        int len = numeric.length();
        if (len == 5 || len == 6) {
            mcc = std::stoi(numeric.substr(0, 3));
            mnc = std::stoi(numeric.substr(3));
        }
    }

    /* SIM operator numeric == network operator numeric
       Some specific countries don't use spn name */
    int lenSimOperatorNumeric = simOperatorNumeric != NULL ? strlen(simOperatorNumeric) : 0;
    int lenNwkInfoPlmn = strlen(nwkInfo.plmn);
    int cmpLen = lenSimOperatorNumeric < lenNwkInfoPlmn ? lenSimOperatorNumeric : lenNwkInfoPlmn;
    if ((lenSimOperatorNumeric > 0) && (strncmp(simOperatorNumeric, nwkInfo.plmn, cmpLen) == 0)
            && (MccTable::isUsingSpnForAvailablePlmnSrch(nwkInfo.plmn) == true)) {
        if(strlen(simSpn) > 0) {
            strncpy(nwkInfo.shortPlmn, simSpn, MAX_SHORT_NAME_LEN - 1);
            strncpy(nwkInfo.longPlmn, simSpn, MAX_FULL_NAME_LEN - 1);
        }
    }

    // operator name from RIL DB
    OperatorNameProvider *provider = OperatorNameProvider::GetInstance();
    OperatorContentValue *opname = NULL;
    if (TextUtils::IsEmpty(nwkInfo.longPlmn) && provider != NULL && (opname = provider->Find(nwkInfo.plmn)) != NULL) {
        if (!TextUtils::IsEmpty(opname->GetShortPlmn()))
            strncpy(nwkInfo.shortPlmn, opname->GetShortPlmn(), MAX_SHORT_NAME_LEN - 1);
        if (!TextUtils::IsEmpty(opname->GetLongPlmn()))
            strncpy(nwkInfo.longPlmn, opname->GetLongPlmn(), MAX_FULL_NAME_LEN - 1);
    }

    // network name from TS25 table for special case.
    // Brazil
    if (TextUtils::Equals(simOperatorNumeric, operatorNumeric) && (mcc == MCC_BRAZIL)) {
        ::UpdateNetworkNameFromTs25(nwkInfo, mcc, mnc);
    }

    // final exceptional case
    OperatorContentValue oper = OperatorNameProvider::GetVendorCustomOperatorName(simOperatorNumeric, operatorNumeric);
    if (!TextUtils::IsEmpty(oper.GetLongPlmn())) {
        memset(nwkInfo.longPlmn, 0, sizeof(nwkInfo.longPlmn));
        strncpy(nwkInfo.longPlmn, oper.GetLongPlmn(), MAX_FULL_NAME_LEN);
    }

    if (!TextUtils::IsEmpty(oper.GetShortPlmn())) {
        memset(nwkInfo.shortPlmn, 0, sizeof(nwkInfo.shortPlmn));
        strncpy(nwkInfo.shortPlmn, oper.GetShortPlmn(), MAX_SHORT_NAME_LEN);
    }

    // if longPlmn(or shortPlmn) is NULL, update it to PLMN string.
    if (*nwkInfo.shortPlmn == 0) {
        strncpy(nwkInfo.shortPlmn, nwkInfo.plmn, sizeof(nwkInfo.plmn));
    }
    if (*nwkInfo.longPlmn == 0) {
        strncpy(nwkInfo.longPlmn, nwkInfo.plmn, sizeof(nwkInfo.plmn));
    }
}

static int ConvertProtocolCellInfoToRilCellInfo(RIL_CellInfo_v12 &out, const sit_net_cell_info_item *pCellInfo)
{
    int size = 2;
    if (pCellInfo != NULL) {
        out.cellInfoType = (RIL_CellInfoType)(pCellInfo->cell_info_type + 1);
        out.registered = pCellInfo->reg_status;
        out.timeStampType = RIL_TIMESTAMP_TYPE_OEM_RIL;
        if (out.cellInfoType != RIL_CELL_INFO_TYPE_CDMA) {
            switch (out.cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM:
                size += sizeof(cell_info_gsm);
                // 16-bit LAC
                out.CellInfo.gsm.cellIdentityGsm.lac = pCellInfo->cell_info.gsm.lac;
                if (out.CellInfo.gsm.cellIdentityGsm.lac < 0 || out.CellInfo.gsm.cellIdentityGsm.lac > 0xFFFF)
                    out.CellInfo.gsm.cellIdentityGsm.lac = INT32_MAX;

                // 16-bit GSM CID
                out.CellInfo.gsm.cellIdentityGsm.cid = pCellInfo->cell_info.gsm.cid;
                if (out.CellInfo.gsm.cellIdentityGsm.cid < 0 || out.CellInfo.gsm.cellIdentityGsm.cid > 0xFFFF)
                    out.CellInfo.gsm.cellIdentityGsm.cid = INT32_MAX;

                // BER (0-7,99)
                out.CellInfo.gsm.signalStrengthGsm.bitErrorRate = pCellInfo->cell_info.gsm.sig_ber;
                if (out.CellInfo.gsm.signalStrengthGsm.bitErrorRate < 0 || out.CellInfo.gsm.signalStrengthGsm.bitErrorRate > 7)
                    out.CellInfo.gsm.signalStrengthGsm.bitErrorRate = 99;

                // Signal Strength (0-31,99)
                out.CellInfo.gsm.signalStrengthGsm.signalStrength = pCellInfo->cell_info.gsm.sig_str;
                if (out.CellInfo.gsm.signalStrengthGsm.signalStrength < 0 || out.CellInfo.gsm.signalStrengthGsm.signalStrength > 31)
                    out.CellInfo.gsm.signalStrengthGsm.signalStrength = 99;
                break;
            case RIL_CELL_INFO_TYPE_WCDMA:
                size += sizeof(cell_info_wcdma);
                // 16-bit LAC
                out.CellInfo.wcdma.cellIdentityWcdma.lac = pCellInfo->cell_info.wcdma.lac;
                if (out.CellInfo.wcdma.cellIdentityWcdma.lac < 0 || out.CellInfo.wcdma.cellIdentityWcdma.lac > 0xFFFF)
                    out.CellInfo.wcdma.cellIdentityWcdma.lac = INT32_MAX;

                // 28-bit UMTS CID
                out.CellInfo.wcdma.cellIdentityWcdma.cid = pCellInfo->cell_info.wcdma.cid;
                if (out.CellInfo.wcdma.cellIdentityWcdma.cid < 0 || out.CellInfo.wcdma.cellIdentityWcdma.cid > 0xFFFFFFF)
                    out.CellInfo.wcdma.cellIdentityWcdma.cid = INT32_MAX;

                // 9-bit UMTS PSC
                out.CellInfo.wcdma.cellIdentityWcdma.psc = pCellInfo->cell_info.wcdma.psc;
                if (out.CellInfo.wcdma.cellIdentityWcdma.psc < 0 || out.CellInfo.wcdma.cellIdentityWcdma.psc > 0x1FF)
                    out.CellInfo.wcdma.cellIdentityWcdma.psc = INT32_MAX;

                // BER (0-7,99)
                out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = pCellInfo->cell_info.wcdma.sig_ber;
                if (out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate < 0 || out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate > 7)
                    out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = 99;

                // Signal Strength (0-31,99)
                out.CellInfo.wcdma.signalStrengthWcdma.signalStrength = pCellInfo->cell_info.wcdma.sig_str;
                if (out.CellInfo.wcdma.signalStrengthWcdma.signalStrength < 0 || out.CellInfo.wcdma.signalStrengthWcdma.signalStrength > 31)
                    out.CellInfo.wcdma.signalStrengthWcdma.signalStrength = pCellInfo->cell_info.wcdma.sig_str;
                break;
            case RIL_CELL_INFO_TYPE_LTE:
                size += sizeof(cell_info_lte);
                // 28-bit CID
                out.CellInfo.lte.cellIdentityLte.ci = pCellInfo->cell_info.lte.cell_id;
                if (out.CellInfo.lte.cellIdentityLte.ci < 0 || out.CellInfo.lte.cellIdentityLte.ci > 0xFFFFFFF)
                    out.CellInfo.lte.cellIdentityLte.ci = INT32_MAX;

                // Physical CID
                out.CellInfo.lte.cellIdentityLte.pci = pCellInfo->cell_info.lte.phy_cell_id;
                if (out.CellInfo.lte.cellIdentityLte.pci < 0 || out.CellInfo.lte.cellIdentityLte.pci > 503)
                    out.CellInfo.lte.cellIdentityLte.pci = INT32_MAX;

                // 16-bit TAC
                out.CellInfo.lte.cellIdentityLte.tac = pCellInfo->cell_info.lte.tac;
                if (out.CellInfo.lte.cellIdentityLte.tac < 0 || out.CellInfo.lte.cellIdentityLte.tac > 0xFFFF)
                    out.CellInfo.lte.cellIdentityLte.tac = INT32_MAX;

                // Signal Strength (0-31,99)
                out.CellInfo.lte.signalStrengthLte.signalStrength = pCellInfo->cell_info.lte.sig_str;
                if (out.CellInfo.lte.signalStrengthLte.signalStrength < 0 || out.CellInfo.lte.signalStrengthLte.signalStrength > 31)
                    out.CellInfo.lte.signalStrengthLte.signalStrength = 99;

                // RSRP (44-140)
                out.CellInfo.lte.signalStrengthLte.rsrp = pCellInfo->cell_info.lte.sig_rsrp;
                if (out.CellInfo.lte.signalStrengthLte.rsrp < 44 || out.CellInfo.lte.signalStrengthLte.rsrp > 140)
                    out.CellInfo.lte.signalStrengthLte.rsrp = INT32_MAX;

                // RSRQ (3-20)
                out.CellInfo.lte.signalStrengthLte.rsrq = pCellInfo->cell_info.lte.sig_rsrq;
                if (out.CellInfo.lte.signalStrengthLte.rsrq < 3 || out.CellInfo.lte.signalStrengthLte.rsrq > 20)
                    out.CellInfo.lte.signalStrengthLte.rsrq = INT32_MAX;

                // RSSNR (-200 - 300)
                out.CellInfo.lte.signalStrengthLte.rssnr = pCellInfo->cell_info.lte.sig_rssnr;
                if (out.CellInfo.lte.signalStrengthLte.rssnr < -200 || out.CellInfo.lte.signalStrengthLte.rssnr > 300)
                    out.CellInfo.lte.signalStrengthLte.rssnr = INT32_MAX;

                // CQI (0-15)
                out.CellInfo.lte.signalStrengthLte.cqi = pCellInfo->cell_info.lte.sig_cqi;
                if (out.CellInfo.lte.signalStrengthLte.cqi < 0 || out.CellInfo.lte.signalStrengthLte.cqi > 15)
                    out.CellInfo.lte.signalStrengthLte.cqi = INT32_MAX;

                // TAVD (0-0x7FFFFFFE)
                out.CellInfo.lte.signalStrengthLte.timingAdvance = pCellInfo->cell_info.lte.ta;
                if (out.CellInfo.lte.signalStrengthLte.timingAdvance < 0 || out.CellInfo.lte.signalStrengthLte.timingAdvance > 0x7FFFFFFE)
                    out.CellInfo.lte.signalStrengthLte.timingAdvance = INT32_MAX;
                break;

            case RIL_CELL_INFO_TYPE_TD_SCDMA:
                size += sizeof(cell_info_tdscdma);
                // 16-bit LAC
                out.CellInfo.tdscdma.cellIdentityTdscdma.lac = pCellInfo->cell_info.tdscdma.lac;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.lac < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.lac > 0xFFFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.lac = INT32_MAX;

                // 28-bit UMTS CID
                out.CellInfo.tdscdma.cellIdentityTdscdma.cid = pCellInfo->cell_info.tdscdma.cid;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.cid < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.cid > 0xFFFFFFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.cid = INT32_MAX;

                // 8-bit CPID
                out.CellInfo.tdscdma.cellIdentityTdscdma.cpid = pCellInfo->cell_info.tdscdma.cpid;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.cpid < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.cpid > 0xFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.cpid = INT32_MAX;

                // RSCP (25-120)
                out.CellInfo.tdscdma.signalStrengthTdscdma.rscp = pCellInfo->cell_info.tdscdma.rscp;
                if (out.CellInfo.tdscdma.signalStrengthTdscdma.rscp < 25 || out.CellInfo.tdscdma.signalStrengthTdscdma.rscp > 250)
                    out.CellInfo.tdscdma.signalStrengthTdscdma.rscp = INT32_MAX;
                break;

            default:
                return 0;
            } // end switch ~

            // common fields MCC/MNC
            char mcc[MAX_PLMN_LEN+1] = { 0, };
            char mnc[MAX_PLMN_LEN+1] = { 0, };
            memcpy(mcc, pCellInfo->cell_info.gsm.plmn, 3);
            memcpy(mnc, pCellInfo->cell_info.gsm.plmn + 3, 3);
            if (mnc[2] == '#') {
                mnc[2] = 0;
            }
            out.CellInfo.gsm.cellIdentityGsm.mcc = atoi(mcc);
            out.CellInfo.gsm.cellIdentityGsm.mnc =
                    ril::util::mnc::encode(atoi(mnc), strlen(mnc));
        }
        else {
            size += sizeof(cell_info_cdma);

            // 16-bit Network Id
            out.CellInfo.cdma.cellIdentityCdma.networkId = pCellInfo->cell_info.cdma.ntw_id;
            if (out.CellInfo.cdma.cellIdentityCdma.networkId < 0 || out.CellInfo.cdma.cellIdentityCdma.networkId > 0xFFFF)
                out.CellInfo.cdma.cellIdentityCdma.networkId = INT32_MAX;

            // 15-bit CDMA System Id
            out.CellInfo.cdma.cellIdentityCdma.systemId = pCellInfo->cell_info.cdma.sys_id;
            if (out.CellInfo.cdma.cellIdentityCdma.systemId < 0 || out.CellInfo.cdma.cellIdentityCdma.systemId > 0x7FFF)
                out.CellInfo.cdma.cellIdentityCdma.systemId = INT32_MAX;

            // 16-bit Base Station Id
            out.CellInfo.cdma.cellIdentityCdma.basestationId = pCellInfo->cell_info.cdma.bs_id;
            if (out.CellInfo.cdma.cellIdentityCdma.basestationId < 0 || out.CellInfo.cdma.cellIdentityCdma.basestationId > 0xFFFF)
                out.CellInfo.cdma.cellIdentityCdma.basestationId = INT32_MAX;

            // Longitude (-2592000-2592000)
            out.CellInfo.cdma.cellIdentityCdma.longitude = pCellInfo->cell_info.cdma.longitude;
            if (out.CellInfo.cdma.cellIdentityCdma.longitude < -2592000 || out.CellInfo.cdma.cellIdentityCdma.longitude > 2592000)
                out.CellInfo.cdma.cellIdentityCdma.longitude = INT32_MAX;

            // Latitude (-1296000-1296000)
            out.CellInfo.cdma.cellIdentityCdma.latitude = pCellInfo->cell_info.cdma.lat;
            if (out.CellInfo.cdma.cellIdentityCdma.latitude < -1296000 || out.CellInfo.cdma.cellIdentityCdma.latitude > 1296000)
                out.CellInfo.cdma.cellIdentityCdma.latitude = INT32_MAX;

            // actual RSSI value (multiplied by -1)
            out.CellInfo.cdma.signalStrengthCdma.dbm = pCellInfo->cell_info.cdma.sig_dbm;
            if (out.CellInfo.cdma.signalStrengthCdma.dbm < 0)
                out.CellInfo.cdma.signalStrengthCdma.dbm *= -1;

            // actual Ec/Io (multiplied by -10)
            out.CellInfo.cdma.signalStrengthCdma.ecio = pCellInfo->cell_info.cdma.sig_ecio;
            if (out.CellInfo.cdma.signalStrengthCdma.ecio < 0)
                out.CellInfo.cdma.signalStrengthCdma.ecio *= -10;

            // actual RSSI value (multiplied by -1)
            out.CellInfo.cdma.signalStrengthEvdo.dbm = pCellInfo->cell_info.cdma.evdo_sig_dbm;
            if (out.CellInfo.cdma.signalStrengthEvdo.dbm < 0)
                out.CellInfo.cdma.signalStrengthEvdo.dbm *= -1;

            // actual Ec/Io (multiplied by -10)
            out.CellInfo.cdma.signalStrengthEvdo.ecio = pCellInfo->cell_info.cdma.evdo_sig_ecio;
            if (out.CellInfo.cdma.signalStrengthEvdo.ecio < 0)
                out.CellInfo.cdma.signalStrengthEvdo.ecio *= -10;

            // signal noise ratio (0-8)
            out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = pCellInfo->cell_info.cdma.evdo_sig_snr;
            if (out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio < 0 || out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio > 8)
                out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = 0;
        }
    }
    return size;
}

static int ConvertProtocolCellInfoToRilCellInfo(RIL_CellInfo_V1_2 &out, const sit_net_cell_info_item *pCellInfo)
{
    int size = 2;
    if (pCellInfo != NULL) {
        out.cellInfoType = (RIL_CellInfoType)(pCellInfo->cell_info_type + 1);
        out.registered = pCellInfo->reg_status;
        out.timeStampType = RIL_TIMESTAMP_TYPE_OEM_RIL;
        if (out.cellInfoType != RIL_CELL_INFO_TYPE_CDMA) {
            switch (out.cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM:
                size += sizeof(cell_info_gsm);
                // 16-bit LAC
                out.CellInfo.gsm.cellIdentityGsm.lac = pCellInfo->cell_info.gsm.lac;
                if (out.CellInfo.gsm.cellIdentityGsm.lac < 0 || out.CellInfo.gsm.cellIdentityGsm.lac > 0xFFFF)
                    out.CellInfo.gsm.cellIdentityGsm.lac = INT32_MAX;

                // 16-bit GSM CID
                out.CellInfo.gsm.cellIdentityGsm.cid = pCellInfo->cell_info.gsm.cid;
                if (out.CellInfo.gsm.cellIdentityGsm.cid < 0 || out.CellInfo.gsm.cellIdentityGsm.cid > 0xFFFF)
                    out.CellInfo.gsm.cellIdentityGsm.cid = INT32_MAX;

                // BER (0-7,99)
                out.CellInfo.gsm.signalStrengthGsm.bitErrorRate = pCellInfo->cell_info.gsm.sig_ber;
                if (out.CellInfo.gsm.signalStrengthGsm.bitErrorRate < 0 || out.CellInfo.gsm.signalStrengthGsm.bitErrorRate > 7)
                    out.CellInfo.gsm.signalStrengthGsm.bitErrorRate = 99;

                // Signal Strength (0-31,99)
                out.CellInfo.gsm.signalStrengthGsm.signalStrength = pCellInfo->cell_info.gsm.sig_str;
                if (out.CellInfo.gsm.signalStrengthGsm.signalStrength < 0 || out.CellInfo.gsm.signalStrengthGsm.signalStrength > 31)
                    out.CellInfo.gsm.signalStrengthGsm.signalStrength = 99;
                break;
            case RIL_CELL_INFO_TYPE_WCDMA:
                size += sizeof(cell_info_wcdma);
                // 16-bit LAC
                out.CellInfo.wcdma.cellIdentityWcdma.lac = pCellInfo->cell_info.wcdma.lac;
                if (out.CellInfo.wcdma.cellIdentityWcdma.lac < 0 || out.CellInfo.wcdma.cellIdentityWcdma.lac > 0xFFFF)
                    out.CellInfo.wcdma.cellIdentityWcdma.lac = INT32_MAX;

                // 28-bit UMTS CID
                out.CellInfo.wcdma.cellIdentityWcdma.cid = pCellInfo->cell_info.wcdma.cid;
                if (out.CellInfo.wcdma.cellIdentityWcdma.cid < 0 || out.CellInfo.wcdma.cellIdentityWcdma.cid > 0xFFFFFFF)
                    out.CellInfo.wcdma.cellIdentityWcdma.cid = INT32_MAX;

                // 9-bit UMTS PSC
                out.CellInfo.wcdma.cellIdentityWcdma.psc = pCellInfo->cell_info.wcdma.psc;
                if (out.CellInfo.wcdma.cellIdentityWcdma.psc < 0 || out.CellInfo.wcdma.cellIdentityWcdma.psc > 0x1FF)
                    out.CellInfo.wcdma.cellIdentityWcdma.psc = INT32_MAX;

                // BER (0-7,99)
                out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = pCellInfo->cell_info.wcdma.sig_ber;
                if (out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate < 0 || out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate > 7)
                    out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = 99;

                // Signal Strength (0-31,99)
                out.CellInfo.wcdma.signalStrengthWcdma.signalStrength = pCellInfo->cell_info.wcdma.sig_str;
                if (out.CellInfo.wcdma.signalStrengthWcdma.signalStrength < 0 || out.CellInfo.wcdma.signalStrengthWcdma.signalStrength > 31)
                    out.CellInfo.wcdma.signalStrengthWcdma.signalStrength = pCellInfo->cell_info.wcdma.sig_str;
                break;
            case RIL_CELL_INFO_TYPE_LTE:
                size += sizeof(cell_info_lte);
                // 28-bit CID
                out.CellInfo.lte.cellIdentityLte.ci = pCellInfo->cell_info.lte.cell_id;
                if (out.CellInfo.lte.cellIdentityLte.ci < 0 || out.CellInfo.lte.cellIdentityLte.ci > 0xFFFFFFF)
                    out.CellInfo.lte.cellIdentityLte.ci = INT32_MAX;

                // Physical CID
                out.CellInfo.lte.cellIdentityLte.pci = pCellInfo->cell_info.lte.phy_cell_id;
                if (out.CellInfo.lte.cellIdentityLte.pci < 0 || out.CellInfo.lte.cellIdentityLte.pci > 503)
                    out.CellInfo.lte.cellIdentityLte.pci = INT32_MAX;

                // 16-bit TAC
                out.CellInfo.lte.cellIdentityLte.tac = pCellInfo->cell_info.lte.tac;
                if (out.CellInfo.lte.cellIdentityLte.tac < 0 || out.CellInfo.lte.cellIdentityLte.tac > 0xFFFF)
                    out.CellInfo.lte.cellIdentityLte.tac = INT32_MAX;

                // Signal Strength (0-31,99)
                out.CellInfo.lte.signalStrengthLte.signalStrength = pCellInfo->cell_info.lte.sig_str;
                if (out.CellInfo.lte.signalStrengthLte.signalStrength < 0 || out.CellInfo.lte.signalStrengthLte.signalStrength > 31)
                    out.CellInfo.lte.signalStrengthLte.signalStrength = 99;

                // RSRP (44-140)
                out.CellInfo.lte.signalStrengthLte.rsrp = pCellInfo->cell_info.lte.sig_rsrp;
                if (out.CellInfo.lte.signalStrengthLte.rsrp < 44 || out.CellInfo.lte.signalStrengthLte.rsrp > 140)
                    out.CellInfo.lte.signalStrengthLte.rsrp = INT32_MAX;

                // RSRQ (3-20)
                out.CellInfo.lte.signalStrengthLte.rsrq = pCellInfo->cell_info.lte.sig_rsrq;
                if (out.CellInfo.lte.signalStrengthLte.rsrq < 3 || out.CellInfo.lte.signalStrengthLte.rsrq > 20)
                    out.CellInfo.lte.signalStrengthLte.rsrq = INT32_MAX;

                // RSSNR (-200 - 300)
                out.CellInfo.lte.signalStrengthLte.rssnr = pCellInfo->cell_info.lte.sig_rssnr;
                if (out.CellInfo.lte.signalStrengthLte.rssnr < -200 || out.CellInfo.lte.signalStrengthLte.rssnr > 300)
                    out.CellInfo.lte.signalStrengthLte.rssnr = INT32_MAX;

                // CQI (0-15)
                out.CellInfo.lte.signalStrengthLte.cqi = pCellInfo->cell_info.lte.sig_cqi;
                if (out.CellInfo.lte.signalStrengthLte.cqi < 0 || out.CellInfo.lte.signalStrengthLte.cqi > 15)
                    out.CellInfo.lte.signalStrengthLte.cqi = INT32_MAX;

                // TAVD (0-0x7FFFFFFE)
                out.CellInfo.lte.signalStrengthLte.timingAdvance = pCellInfo->cell_info.lte.ta;
                if (out.CellInfo.lte.signalStrengthLte.timingAdvance < 0 || out.CellInfo.lte.signalStrengthLte.timingAdvance > 0x7FFFFFFE)
                    out.CellInfo.lte.signalStrengthLte.timingAdvance = INT32_MAX;
                break;

            case RIL_CELL_INFO_TYPE_TD_SCDMA:
                size += sizeof(cell_info_tdscdma);
                // 16-bit LAC
                out.CellInfo.tdscdma.cellIdentityTdscdma.lac = pCellInfo->cell_info.tdscdma.lac;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.lac < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.lac > 0xFFFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.lac = INT32_MAX;

                // 28-bit UMTS CID
                out.CellInfo.tdscdma.cellIdentityTdscdma.cid = pCellInfo->cell_info.tdscdma.cid;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.cid < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.cid > 0xFFFFFFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.cid = INT32_MAX;

                // 8-bit CPID
                out.CellInfo.tdscdma.cellIdentityTdscdma.cpid = pCellInfo->cell_info.tdscdma.cpid;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.cpid < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.cpid > 0xFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.cpid = INT32_MAX;

                // RSCP (25-120)
                out.CellInfo.tdscdma.signalStrengthTdscdma.rscp = pCellInfo->cell_info.tdscdma.rscp;
                if (out.CellInfo.tdscdma.signalStrengthTdscdma.rscp < 25 || out.CellInfo.tdscdma.signalStrengthTdscdma.rscp > 250)
                    out.CellInfo.tdscdma.signalStrengthTdscdma.rscp = INT32_MAX;
                break;


            default:
                return 0;
            } // end switch ~

            // common fields MCC/MNC
            char mcc[MAX_PLMN_LEN+1] = { 0, };
            char mnc[MAX_PLMN_LEN+1] = { 0, };
            memcpy(mcc, pCellInfo->cell_info.gsm.plmn, 3);
            memcpy(mnc, pCellInfo->cell_info.gsm.plmn + 3, 3);
            if (mnc[2] == '#') {
                mnc[2] = 0;
            }
            out.CellInfo.gsm.cellIdentityGsm.mcc = atoi(mcc);
            out.CellInfo.gsm.cellIdentityGsm.mnc =
                    ril::util::mnc::encode(atoi(mnc), strlen(mnc));
        }
        else {
            size += sizeof(cell_info_cdma);

            // 16-bit Network Id
            out.CellInfo.cdma.cellIdentityCdma.networkId = pCellInfo->cell_info.cdma.ntw_id;
            if (out.CellInfo.cdma.cellIdentityCdma.networkId < 0 || out.CellInfo.cdma.cellIdentityCdma.networkId > 0xFFFF)
                out.CellInfo.cdma.cellIdentityCdma.networkId = INT32_MAX;

            // 15-bit CDMA System Id
            out.CellInfo.cdma.cellIdentityCdma.systemId = pCellInfo->cell_info.cdma.sys_id;
            if (out.CellInfo.cdma.cellIdentityCdma.systemId < 0 || out.CellInfo.cdma.cellIdentityCdma.systemId > 0x7FFF)
                out.CellInfo.cdma.cellIdentityCdma.systemId = INT32_MAX;

            // 16-bit Base Station Id
            out.CellInfo.cdma.cellIdentityCdma.basestationId = pCellInfo->cell_info.cdma.bs_id;
            if (out.CellInfo.cdma.cellIdentityCdma.basestationId < 0 || out.CellInfo.cdma.cellIdentityCdma.basestationId > 0xFFFF)
                out.CellInfo.cdma.cellIdentityCdma.basestationId = INT32_MAX;

            // Longitude (-2592000-2592000)
            out.CellInfo.cdma.cellIdentityCdma.longitude = pCellInfo->cell_info.cdma.longitude;
            if (out.CellInfo.cdma.cellIdentityCdma.longitude < -2592000 || out.CellInfo.cdma.cellIdentityCdma.longitude > 2592000)
                out.CellInfo.cdma.cellIdentityCdma.longitude = INT32_MAX;

            // Latitude (-1296000-1296000)
            out.CellInfo.cdma.cellIdentityCdma.latitude = pCellInfo->cell_info.cdma.lat;
            if (out.CellInfo.cdma.cellIdentityCdma.latitude < -1296000 || out.CellInfo.cdma.cellIdentityCdma.latitude > 1296000)
                out.CellInfo.cdma.cellIdentityCdma.latitude = INT32_MAX;

            // actual RSSI value (multiplied by -1)
            out.CellInfo.cdma.signalStrengthCdma.dbm = pCellInfo->cell_info.cdma.sig_dbm;
            if (out.CellInfo.cdma.signalStrengthCdma.dbm < 0)
                out.CellInfo.cdma.signalStrengthCdma.dbm *= -1;

            // actual Ec/Io (multiplied by -10)
            out.CellInfo.cdma.signalStrengthCdma.ecio = pCellInfo->cell_info.cdma.sig_ecio;
            if (out.CellInfo.cdma.signalStrengthCdma.ecio < 0)
                out.CellInfo.cdma.signalStrengthCdma.ecio *= -10;

            // actual RSSI value (multiplied by -1)
            out.CellInfo.cdma.signalStrengthEvdo.dbm = pCellInfo->cell_info.cdma.evdo_sig_dbm;
            if (out.CellInfo.cdma.signalStrengthEvdo.dbm < 0)
                out.CellInfo.cdma.signalStrengthEvdo.dbm *= -1;

            // actual Ec/Io (multiplied by -10)
            out.CellInfo.cdma.signalStrengthEvdo.ecio = pCellInfo->cell_info.cdma.evdo_sig_ecio;
            if (out.CellInfo.cdma.signalStrengthEvdo.ecio < 0)
                out.CellInfo.cdma.signalStrengthEvdo.ecio *= -10;

            // signal noise ratio (0-8)
            out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = pCellInfo->cell_info.cdma.evdo_sig_snr;
            if (out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio < 0 || out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio > 8)
                out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = 0;
        }
    }
    return size;
}

static int ConvertProtocolCellInfoV12ToRilCellInfo(RIL_CellInfo_v12 &out, const sit_net_cell_info_item_v12 *pCellInfo)
{
    int size = 0;
    if (pCellInfo != NULL) {
        out.cellInfoType = (RIL_CellInfoType)(pCellInfo->cell_info_type + 1);
        out.registered = pCellInfo->reg_status;
        out.timeStampType = RIL_TIMESTAMP_TYPE_OEM_RIL;
        size += 2;
        if (out.cellInfoType != RIL_CELL_INFO_TYPE_CDMA) {
            switch (out.cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM:
                size += sizeof(cell_info_gsm_v12);
                // 16-bit LAC
                out.CellInfo.gsm.cellIdentityGsm.lac = pCellInfo->cell_info.gsm.lac;
                if (out.CellInfo.gsm.cellIdentityGsm.lac < 0 || out.CellInfo.gsm.cellIdentityGsm.lac > 0xFFFF)
                    out.CellInfo.gsm.cellIdentityGsm.lac = INT32_MAX;

                // 16-bit GSM CID
                out.CellInfo.gsm.cellIdentityGsm.cid = pCellInfo->cell_info.gsm.cid;
                if (out.CellInfo.gsm.cellIdentityGsm.cid < 0 || out.CellInfo.gsm.cellIdentityGsm.cid > 0xFFFF)
                    out.CellInfo.gsm.cellIdentityGsm.cid = INT32_MAX;

                // 16-bit GSM ARFCN
                out.CellInfo.gsm.cellIdentityGsm.arfcn = pCellInfo->cell_info.gsm.arfcn;
                if (out.CellInfo.gsm.cellIdentityGsm.arfcn < 0 || out.CellInfo.gsm.cellIdentityGsm.arfcn > 0xFFFF)
                    out.CellInfo.gsm.cellIdentityGsm.arfcn = INT32_MAX;

                // 6-bit GSM BSIC
                out.CellInfo.gsm.cellIdentityGsm.bsic = pCellInfo->cell_info.gsm.bsic;
                if (out.CellInfo.gsm.cellIdentityGsm.bsic > 0x3F)
                    out.CellInfo.gsm.cellIdentityGsm.bsic = 0xFF;

                // BER (0-7,99)
                out.CellInfo.gsm.signalStrengthGsm.bitErrorRate = pCellInfo->cell_info.gsm.sig_ber;
                if (out.CellInfo.gsm.signalStrengthGsm.bitErrorRate < 0 || out.CellInfo.gsm.signalStrengthGsm.bitErrorRate > 7)
                    out.CellInfo.gsm.signalStrengthGsm.bitErrorRate = 99;

                // Signal Strength (0-31,99)
                out.CellInfo.gsm.signalStrengthGsm.signalStrength = pCellInfo->cell_info.gsm.sig_str;
                if (out.CellInfo.gsm.signalStrengthGsm.signalStrength < 0 || out.CellInfo.gsm.signalStrengthGsm.signalStrength > 31)
                    out.CellInfo.gsm.signalStrengthGsm.signalStrength = 99;

                // Timing Advance (0 ~63)
                out.CellInfo.gsm.signalStrengthGsm.timingAdvance = pCellInfo->cell_info.gsm.sig_ta;
                if (out.CellInfo.gsm.signalStrengthGsm.timingAdvance < 0 || out.CellInfo.gsm.signalStrengthGsm.timingAdvance > 63)
                    out.CellInfo.gsm.signalStrengthGsm.timingAdvance = INT32_MAX;
                break;
            case RIL_CELL_INFO_TYPE_WCDMA:
                size += sizeof(cell_info_wcdma_v12);
                // 16-bit LAC
                out.CellInfo.wcdma.cellIdentityWcdma.lac = pCellInfo->cell_info.wcdma.lac;
                if (out.CellInfo.wcdma.cellIdentityWcdma.lac < 0 || out.CellInfo.wcdma.cellIdentityWcdma.lac > 0xFFFF)
                    out.CellInfo.wcdma.cellIdentityWcdma.lac = INT32_MAX;

                // 28-bit UMTS CID
                out.CellInfo.wcdma.cellIdentityWcdma.cid = pCellInfo->cell_info.wcdma.cid;
                if (out.CellInfo.wcdma.cellIdentityWcdma.cid < 0 || out.CellInfo.wcdma.cellIdentityWcdma.cid > 0xFFFFFFF)
                    out.CellInfo.wcdma.cellIdentityWcdma.cid = INT32_MAX;

                // 9-bit UMTS PSC
                out.CellInfo.wcdma.cellIdentityWcdma.psc = pCellInfo->cell_info.wcdma.psc;
                if (out.CellInfo.wcdma.cellIdentityWcdma.psc < 0 || out.CellInfo.wcdma.cellIdentityWcdma.psc > 0x1FF)
                    out.CellInfo.wcdma.cellIdentityWcdma.psc = INT32_MAX;

                // 16-bit UMTS uarfcn
                out.CellInfo.wcdma.cellIdentityWcdma.uarfcn = pCellInfo->cell_info.wcdma.uarfcn;
                if (out.CellInfo.wcdma.cellIdentityWcdma.uarfcn < 0 || out.CellInfo.wcdma.cellIdentityWcdma.uarfcn > 0xFFFF)
                    out.CellInfo.wcdma.cellIdentityWcdma.uarfcn = INT32_MAX;

                // BER (0-7,99)
                out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = pCellInfo->cell_info.wcdma.sig_ber;
                if (out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate < 0 || out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate > 7)
                    out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = 99;

                // Signal Strength (0-31,99)
                out.CellInfo.wcdma.signalStrengthWcdma.signalStrength = pCellInfo->cell_info.wcdma.sig_str;
                if (out.CellInfo.wcdma.signalStrengthWcdma.signalStrength < 0 || out.CellInfo.wcdma.signalStrengthWcdma.signalStrength > 31)
                    out.CellInfo.wcdma.signalStrengthWcdma.signalStrength = pCellInfo->cell_info.wcdma.sig_str;
                break;
            case RIL_CELL_INFO_TYPE_LTE:
                size += sizeof(cell_info_lte_v12);
                // 28-bit CID
                out.CellInfo.lte.cellIdentityLte.ci = pCellInfo->cell_info.lte.cell_id;
                if (out.CellInfo.lte.cellIdentityLte.ci < 0 || out.CellInfo.lte.cellIdentityLte.ci > 0xFFFFFFF)
                    out.CellInfo.lte.cellIdentityLte.ci = INT32_MAX;

                // Physical CID
                out.CellInfo.lte.cellIdentityLte.pci = pCellInfo->cell_info.lte.phy_cell_id;
                if (out.CellInfo.lte.cellIdentityLte.pci < 0 || out.CellInfo.lte.cellIdentityLte.pci > 503)
                    out.CellInfo.lte.cellIdentityLte.pci = INT32_MAX;

                // 16-bit TAC
                out.CellInfo.lte.cellIdentityLte.tac = pCellInfo->cell_info.lte.tac;
                if (out.CellInfo.lte.cellIdentityLte.tac < 0 || out.CellInfo.lte.cellIdentityLte.tac > 0xFFFF)
                    out.CellInfo.lte.cellIdentityLte.tac = INT32_MAX;

                // 18-bit earfcn
                out.CellInfo.lte.cellIdentityLte.earfcn = pCellInfo->cell_info.lte.earfcn;
                if (out.CellInfo.lte.cellIdentityLte.earfcn < 0 || out.CellInfo.lte.cellIdentityLte.earfcn > 0x3FFFF)
                    out.CellInfo.lte.cellIdentityLte.earfcn = INT32_MAX;

                // Signal Strength (0-31,99)
                out.CellInfo.lte.signalStrengthLte.signalStrength = pCellInfo->cell_info.lte.sig_str;
                if (out.CellInfo.lte.signalStrengthLte.signalStrength < 0 || out.CellInfo.lte.signalStrengthLte.signalStrength > 31)
                    out.CellInfo.lte.signalStrengthLte.signalStrength = 99;

                // RSRP (44-140)
                out.CellInfo.lte.signalStrengthLte.rsrp = pCellInfo->cell_info.lte.sig_rsrp;
                if (out.CellInfo.lte.signalStrengthLte.rsrp < 44 || out.CellInfo.lte.signalStrengthLte.rsrp > 140)
                    out.CellInfo.lte.signalStrengthLte.rsrp = INT32_MAX;

                // RSRQ (3-20)
                out.CellInfo.lte.signalStrengthLte.rsrq = pCellInfo->cell_info.lte.sig_rsrq;
                if (out.CellInfo.lte.signalStrengthLte.rsrq < 3 || out.CellInfo.lte.signalStrengthLte.rsrq > 20)
                    out.CellInfo.lte.signalStrengthLte.rsrq = INT32_MAX;

                // RSSNR (-200 - 300)
                out.CellInfo.lte.signalStrengthLte.rssnr = pCellInfo->cell_info.lte.sig_rssnr;
                if (out.CellInfo.lte.signalStrengthLte.rssnr < -200 || out.CellInfo.lte.signalStrengthLte.rssnr > 300)
                    out.CellInfo.lte.signalStrengthLte.rssnr = INT32_MAX;

                // CQI (0-15)
                out.CellInfo.lte.signalStrengthLte.cqi = pCellInfo->cell_info.lte.sig_cqi;
                if (out.CellInfo.lte.signalStrengthLte.cqi < 0 || out.CellInfo.lte.signalStrengthLte.cqi > 15)
                    out.CellInfo.lte.signalStrengthLte.cqi = INT32_MAX;

                // TAVD (0-0x7FFFFFFE)
                out.CellInfo.lte.signalStrengthLte.timingAdvance = pCellInfo->cell_info.lte.ta;
                if (out.CellInfo.lte.signalStrengthLte.timingAdvance < 0 || out.CellInfo.lte.signalStrengthLte.timingAdvance > 0x7FFFFFFE)
                    out.CellInfo.lte.signalStrengthLte.timingAdvance = INT32_MAX;
                break;

            case RIL_CELL_INFO_TYPE_TD_SCDMA:
                size += sizeof(cell_info_tdscdma);
                // 16-bit LAC
                out.CellInfo.tdscdma.cellIdentityTdscdma.lac = pCellInfo->cell_info.tdscdma.lac;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.lac < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.lac > 0xFFFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.lac = INT32_MAX;

                // 28-bit UMTS CID
                out.CellInfo.tdscdma.cellIdentityTdscdma.cid = pCellInfo->cell_info.tdscdma.cid;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.cid < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.cid > 0xFFFFFFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.cid = INT32_MAX;

                // 8-bit CPID
                out.CellInfo.tdscdma.cellIdentityTdscdma.cpid = pCellInfo->cell_info.tdscdma.cpid;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.cpid < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.cpid > 0xFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.cpid = INT32_MAX;

                // RSCP (25-120)
                out.CellInfo.tdscdma.signalStrengthTdscdma.rscp = pCellInfo->cell_info.tdscdma.rscp;
                if (out.CellInfo.tdscdma.signalStrengthTdscdma.rscp < 25 || out.CellInfo.tdscdma.signalStrengthTdscdma.rscp > 250)
                    out.CellInfo.tdscdma.signalStrengthTdscdma.rscp = INT32_MAX;
                break;

            default:
                return 0;
            } // end switch ~

            // common fields MCC/MNC
            char mcc[MAX_PLMN_LEN+1] = { 0, };
            char mnc[MAX_PLMN_LEN+1] = { 0, };
            memcpy(mcc, pCellInfo->cell_info.gsm.plmn, 3);
            memcpy(mnc, pCellInfo->cell_info.gsm.plmn + 3, 3);
            if (mnc[2] == '#') {
                mnc[2] = 0;
            }
            out.CellInfo.gsm.cellIdentityGsm.mcc = atoi(mcc);
            out.CellInfo.gsm.cellIdentityGsm.mnc = atoi(mnc);
        }
        else {
            size += sizeof(cell_info_cdma);

            // 16-bit Network Id
            out.CellInfo.cdma.cellIdentityCdma.networkId = pCellInfo->cell_info.cdma.ntw_id;
            if (out.CellInfo.cdma.cellIdentityCdma.networkId < 0 || out.CellInfo.cdma.cellIdentityCdma.networkId > 0xFFFF)
                out.CellInfo.cdma.cellIdentityCdma.networkId = INT32_MAX;

            // 15-bit CDMA System Id
            out.CellInfo.cdma.cellIdentityCdma.systemId = pCellInfo->cell_info.cdma.sys_id;
            if (out.CellInfo.cdma.cellIdentityCdma.systemId < 0 || out.CellInfo.cdma.cellIdentityCdma.systemId > 0x7FFF)
                out.CellInfo.cdma.cellIdentityCdma.systemId = INT32_MAX;

            // 16-bit Base Station Id
            out.CellInfo.cdma.cellIdentityCdma.basestationId = pCellInfo->cell_info.cdma.bs_id;
            if (out.CellInfo.cdma.cellIdentityCdma.basestationId < 0 || out.CellInfo.cdma.cellIdentityCdma.basestationId > 0xFFFF)
                out.CellInfo.cdma.cellIdentityCdma.basestationId = INT32_MAX;

            // Longitude (-2592000-2592000)
            out.CellInfo.cdma.cellIdentityCdma.longitude = pCellInfo->cell_info.cdma.longitude;
            if (out.CellInfo.cdma.cellIdentityCdma.longitude < -2592000 || out.CellInfo.cdma.cellIdentityCdma.longitude > 2592000)
                out.CellInfo.cdma.cellIdentityCdma.longitude = INT32_MAX;

            // Latitude (-1296000-1296000)
            out.CellInfo.cdma.cellIdentityCdma.latitude = pCellInfo->cell_info.cdma.lat;
            if (out.CellInfo.cdma.cellIdentityCdma.latitude < -1296000 || out.CellInfo.cdma.cellIdentityCdma.latitude > 1296000)
                out.CellInfo.cdma.cellIdentityCdma.latitude = INT32_MAX;

            // actual RSSI value (multiplied by -1)
            out.CellInfo.cdma.signalStrengthCdma.dbm = pCellInfo->cell_info.cdma.sig_dbm;
            if (out.CellInfo.cdma.signalStrengthCdma.dbm < 0)
                out.CellInfo.cdma.signalStrengthCdma.dbm *= -1;

            // actual Ec/Io (multiplied by -10)
            out.CellInfo.cdma.signalStrengthCdma.ecio = pCellInfo->cell_info.cdma.sig_ecio;
            if (out.CellInfo.cdma.signalStrengthCdma.ecio < 0)
                out.CellInfo.cdma.signalStrengthCdma.ecio *= -10;

            // actual RSSI value (multiplied by -1)
            out.CellInfo.cdma.signalStrengthEvdo.dbm = pCellInfo->cell_info.cdma.evdo_sig_dbm;
            if (out.CellInfo.cdma.signalStrengthEvdo.dbm < 0)
                out.CellInfo.cdma.signalStrengthEvdo.dbm *= -1;

            // actual Ec/Io (multiplied by -10)
            out.CellInfo.cdma.signalStrengthEvdo.ecio = pCellInfo->cell_info.cdma.evdo_sig_ecio;
            if (out.CellInfo.cdma.signalStrengthEvdo.ecio < 0)
                out.CellInfo.cdma.signalStrengthEvdo.ecio *= -10;

            // signal noise ratio (0-8)
            out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = pCellInfo->cell_info.cdma.evdo_sig_snr;
            if (out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio < 0 || out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio > 8)
                out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = 0;
        }
    }
    return size;
}

static int ConvertProtocolCellInfoV1_2ToRilCellInfo(RIL_CellInfo_V1_2 &out, const sit_net_cell_info_item_v12 *pCellInfo)
{
    int size = 0;
    if (pCellInfo != NULL) {
        out.cellInfoType = (RIL_CellInfoType)(pCellInfo->cell_info_type);
        out.registered = pCellInfo->reg_status;
        //memcpy(&(out.timeStampType), pCellInfo->time_stamp, sizeof(out.timeStampType));
        size += 14;
        if (out.cellInfoType != RIL_CELL_INFO_TYPE_CDMA) {
            switch (out.cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM:
                size += sizeof(cell_info_gsm_v12);
                // 16-bit LAC
                out.CellInfo.gsm.cellIdentityGsm.lac = pCellInfo->cell_info.gsm.lac;
                if (out.CellInfo.gsm.cellIdentityGsm.lac < 0 || out.CellInfo.gsm.cellIdentityGsm.lac > 0xFFFF)
                    out.CellInfo.gsm.cellIdentityGsm.lac = INT32_MAX;

                // 16-bit GSM CID
                out.CellInfo.gsm.cellIdentityGsm.cid = pCellInfo->cell_info.gsm.cid;
                if (out.CellInfo.gsm.cellIdentityGsm.cid < 0 || out.CellInfo.gsm.cellIdentityGsm.cid > 0xFFFF)
                    out.CellInfo.gsm.cellIdentityGsm.cid = INT32_MAX;

                // 16-bit GSM ARFCN
                out.CellInfo.gsm.cellIdentityGsm.arfcn = pCellInfo->cell_info.gsm.arfcn;
                if (out.CellInfo.gsm.cellIdentityGsm.arfcn < 0 || out.CellInfo.gsm.cellIdentityGsm.arfcn > 0xFFFF)
                    out.CellInfo.gsm.cellIdentityGsm.arfcn = INT32_MAX;

                // 6-bit GSM BSIC
                out.CellInfo.gsm.cellIdentityGsm.bsic = pCellInfo->cell_info.gsm.bsic;
                if (out.CellInfo.gsm.cellIdentityGsm.bsic > 0x3F)
                    out.CellInfo.gsm.cellIdentityGsm.bsic = 0xFF;

                // BER (0-7,99)
                out.CellInfo.gsm.signalStrengthGsm.bitErrorRate = pCellInfo->cell_info.gsm.sig_ber;
                if (out.CellInfo.gsm.signalStrengthGsm.bitErrorRate < 0 || out.CellInfo.gsm.signalStrengthGsm.bitErrorRate > 7)
                    out.CellInfo.gsm.signalStrengthGsm.bitErrorRate = 99;

                // Signal Strength (0-31,99)
                out.CellInfo.gsm.signalStrengthGsm.signalStrength = pCellInfo->cell_info.gsm.sig_str;
                if (out.CellInfo.gsm.signalStrengthGsm.signalStrength < 0 || out.CellInfo.gsm.signalStrengthGsm.signalStrength > 31)
                    out.CellInfo.gsm.signalStrengthGsm.signalStrength = 99;

                // Timing Advance (0 ~63)
                out.CellInfo.gsm.signalStrengthGsm.timingAdvance = pCellInfo->cell_info.gsm.sig_ta;
                if (out.CellInfo.gsm.signalStrengthGsm.timingAdvance < 0 || out.CellInfo.gsm.signalStrengthGsm.timingAdvance > 63)
                    out.CellInfo.gsm.signalStrengthGsm.timingAdvance = INT32_MAX;
                break;
            case RIL_CELL_INFO_TYPE_WCDMA:
                size += sizeof(cell_info_wcdma_v12);
                // 16-bit LAC
                out.CellInfo.wcdma.cellIdentityWcdma.lac = pCellInfo->cell_info.wcdma.lac;
                if (out.CellInfo.wcdma.cellIdentityWcdma.lac < 0 || out.CellInfo.wcdma.cellIdentityWcdma.lac > 0xFFFF)
                    out.CellInfo.wcdma.cellIdentityWcdma.lac = INT32_MAX;

                // 28-bit UMTS CID
                out.CellInfo.wcdma.cellIdentityWcdma.cid = pCellInfo->cell_info.wcdma.cid;
                if (out.CellInfo.wcdma.cellIdentityWcdma.cid < 0 || out.CellInfo.wcdma.cellIdentityWcdma.cid > 0xFFFFFFF)
                    out.CellInfo.wcdma.cellIdentityWcdma.cid = INT32_MAX;

                // 9-bit UMTS PSC
                out.CellInfo.wcdma.cellIdentityWcdma.psc = pCellInfo->cell_info.wcdma.psc;
                if (out.CellInfo.wcdma.cellIdentityWcdma.psc < 0 || out.CellInfo.wcdma.cellIdentityWcdma.psc > 0x1FF)
                    out.CellInfo.wcdma.cellIdentityWcdma.psc = INT32_MAX;

                // 16-bit UMTS uarfcn
                out.CellInfo.wcdma.cellIdentityWcdma.uarfcn = pCellInfo->cell_info.wcdma.uarfcn;
                if (out.CellInfo.wcdma.cellIdentityWcdma.uarfcn < 0 || out.CellInfo.wcdma.cellIdentityWcdma.uarfcn > 0xFFFF)
                    out.CellInfo.wcdma.cellIdentityWcdma.uarfcn = INT32_MAX;

                // BER (0-7,99)
                out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = pCellInfo->cell_info.wcdma.sig_ber;
                if (out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate < 0 || out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate > 7)
                    out.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = 99;

                // Signal Strength (0-31,99)
                out.CellInfo.wcdma.signalStrengthWcdma.signalStrength = pCellInfo->cell_info.wcdma.sig_str;
                if (out.CellInfo.wcdma.signalStrengthWcdma.signalStrength < 0 || out.CellInfo.wcdma.signalStrengthWcdma.signalStrength > 31)
                    out.CellInfo.wcdma.signalStrengthWcdma.signalStrength = pCellInfo->cell_info.wcdma.sig_str;
                break;
            case RIL_CELL_INFO_TYPE_LTE:
                size += sizeof(cell_info_lte_v12);
                // 28-bit CID
                out.CellInfo.lte.cellIdentityLte.ci = pCellInfo->cell_info.lte.cell_id;
                if (out.CellInfo.lte.cellIdentityLte.ci < 0 || out.CellInfo.lte.cellIdentityLte.ci > 0xFFFFFFF)
                    out.CellInfo.lte.cellIdentityLte.ci = INT32_MAX;

                // Physical CID
                out.CellInfo.lte.cellIdentityLte.pci = pCellInfo->cell_info.lte.phy_cell_id;
                if (out.CellInfo.lte.cellIdentityLte.pci < 0 || out.CellInfo.lte.cellIdentityLte.pci > 503)
                    out.CellInfo.lte.cellIdentityLte.pci = INT32_MAX;

                // 16-bit TAC
                out.CellInfo.lte.cellIdentityLte.tac = pCellInfo->cell_info.lte.tac;
                if (out.CellInfo.lte.cellIdentityLte.tac < 0 || out.CellInfo.lte.cellIdentityLte.tac > 0xFFFF)
                    out.CellInfo.lte.cellIdentityLte.tac = INT32_MAX;

                // 18-bit earfcn
                out.CellInfo.lte.cellIdentityLte.earfcn = pCellInfo->cell_info.lte.earfcn;
                if (out.CellInfo.lte.cellIdentityLte.earfcn < 0 || out.CellInfo.lte.cellIdentityLte.earfcn > 0x3FFFF)
                    out.CellInfo.lte.cellIdentityLte.earfcn = INT32_MAX;

                // Signal Strength (0-31,99)
                out.CellInfo.lte.signalStrengthLte.signalStrength = pCellInfo->cell_info.lte.sig_str;
                if (out.CellInfo.lte.signalStrengthLte.signalStrength < 0 || out.CellInfo.lte.signalStrengthLte.signalStrength > 31)
                    out.CellInfo.lte.signalStrengthLte.signalStrength = 99;

                // RSRP (44-140)
                out.CellInfo.lte.signalStrengthLte.rsrp = pCellInfo->cell_info.lte.sig_rsrp;
                if (out.CellInfo.lte.signalStrengthLte.rsrp < 44 || out.CellInfo.lte.signalStrengthLte.rsrp > 140)
                    out.CellInfo.lte.signalStrengthLte.rsrp = INT32_MAX;

                // RSRQ (3-20)
                out.CellInfo.lte.signalStrengthLte.rsrq = pCellInfo->cell_info.lte.sig_rsrq;
                if (out.CellInfo.lte.signalStrengthLte.rsrq < 3 || out.CellInfo.lte.signalStrengthLte.rsrq > 20)
                    out.CellInfo.lte.signalStrengthLte.rsrq = INT32_MAX;

                // RSSNR (-200 - 300)
                out.CellInfo.lte.signalStrengthLte.rssnr = pCellInfo->cell_info.lte.sig_rssnr;
                if (out.CellInfo.lte.signalStrengthLte.rssnr < -200 || out.CellInfo.lte.signalStrengthLte.rssnr > 300)
                    out.CellInfo.lte.signalStrengthLte.rssnr = INT32_MAX;

                // CQI (0-15)
                out.CellInfo.lte.signalStrengthLte.cqi = pCellInfo->cell_info.lte.sig_cqi;
                if (out.CellInfo.lte.signalStrengthLte.cqi < 0 || out.CellInfo.lte.signalStrengthLte.cqi > 15)
                    out.CellInfo.lte.signalStrengthLte.cqi = INT32_MAX;

                // TAVD (0-0x7FFFFFFE)
                out.CellInfo.lte.signalStrengthLte.timingAdvance = pCellInfo->cell_info.lte.ta;
                if (out.CellInfo.lte.signalStrengthLte.timingAdvance < 0 || out.CellInfo.lte.signalStrengthLte.timingAdvance > 0x7FFFFFFE)
                    out.CellInfo.lte.signalStrengthLte.timingAdvance = INT32_MAX;
                break;

            case RIL_CELL_INFO_TYPE_TD_SCDMA:
                size += sizeof(cell_info_tdscdma);
                // 16-bit LAC
                out.CellInfo.tdscdma.cellIdentityTdscdma.lac = pCellInfo->cell_info.tdscdma.lac;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.lac < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.lac > 0xFFFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.lac = INT32_MAX;

                // 28-bit UMTS CID
                out.CellInfo.tdscdma.cellIdentityTdscdma.cid = pCellInfo->cell_info.tdscdma.cid;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.cid < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.cid > 0xFFFFFFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.cid = INT32_MAX;

                // 8-bit CPID
                out.CellInfo.tdscdma.cellIdentityTdscdma.cpid = pCellInfo->cell_info.tdscdma.cpid;
                if (out.CellInfo.tdscdma.cellIdentityTdscdma.cpid < 0 || out.CellInfo.tdscdma.cellIdentityTdscdma.cpid > 0xFF)
                    out.CellInfo.tdscdma.cellIdentityTdscdma.cpid = INT32_MAX;

                // RSCP (25-120)
                out.CellInfo.tdscdma.signalStrengthTdscdma.rscp = pCellInfo->cell_info.tdscdma.rscp;
                if (out.CellInfo.tdscdma.signalStrengthTdscdma.rscp < 25 || out.CellInfo.tdscdma.signalStrengthTdscdma.rscp > 250)
                    out.CellInfo.tdscdma.signalStrengthTdscdma.rscp = INT32_MAX;
                break;

            default:
                return 0;
            } // end switch ~

            // common fields MCC/MNC
            char mcc[MAX_PLMN_LEN+1] = { 0, };
            char mnc[MAX_PLMN_LEN+1] = { 0, };
            memcpy(mcc, pCellInfo->cell_info.gsm.plmn, 3);
            memcpy(mnc, pCellInfo->cell_info.gsm.plmn + 3, 3);
            if (mnc[2] == '#') {
                mnc[2] = 0;
            }
            out.CellInfo.gsm.cellIdentityGsm.mcc = atoi(mcc);
            out.CellInfo.gsm.cellIdentityGsm.mnc = atoi(mnc);
        }
        else {
            size += sizeof(cell_info_cdma);

            // 16-bit Network Id
            out.CellInfo.cdma.cellIdentityCdma.networkId = pCellInfo->cell_info.cdma.ntw_id;
            if (out.CellInfo.cdma.cellIdentityCdma.networkId < 0 || out.CellInfo.cdma.cellIdentityCdma.networkId > 0xFFFF)
                out.CellInfo.cdma.cellIdentityCdma.networkId = INT32_MAX;

            // 15-bit CDMA System Id
            out.CellInfo.cdma.cellIdentityCdma.systemId = pCellInfo->cell_info.cdma.sys_id;
            if (out.CellInfo.cdma.cellIdentityCdma.systemId < 0 || out.CellInfo.cdma.cellIdentityCdma.systemId > 0x7FFF)
                out.CellInfo.cdma.cellIdentityCdma.systemId = INT32_MAX;

            // 16-bit Base Station Id
            out.CellInfo.cdma.cellIdentityCdma.basestationId = pCellInfo->cell_info.cdma.bs_id;
            if (out.CellInfo.cdma.cellIdentityCdma.basestationId < 0 || out.CellInfo.cdma.cellIdentityCdma.basestationId > 0xFFFF)
                out.CellInfo.cdma.cellIdentityCdma.basestationId = INT32_MAX;

            // Longitude (-2592000-2592000)
            out.CellInfo.cdma.cellIdentityCdma.longitude = pCellInfo->cell_info.cdma.longitude;
            if (out.CellInfo.cdma.cellIdentityCdma.longitude < -2592000 || out.CellInfo.cdma.cellIdentityCdma.longitude > 2592000)
                out.CellInfo.cdma.cellIdentityCdma.longitude = INT32_MAX;

            // Latitude (-1296000-1296000)
            out.CellInfo.cdma.cellIdentityCdma.latitude = pCellInfo->cell_info.cdma.lat;
            if (out.CellInfo.cdma.cellIdentityCdma.latitude < -1296000 || out.CellInfo.cdma.cellIdentityCdma.latitude > 1296000)
                out.CellInfo.cdma.cellIdentityCdma.latitude = INT32_MAX;

            // actual RSSI value (multiplied by -1)
            out.CellInfo.cdma.signalStrengthCdma.dbm = pCellInfo->cell_info.cdma.sig_dbm;
            if (out.CellInfo.cdma.signalStrengthCdma.dbm < 0)
                out.CellInfo.cdma.signalStrengthCdma.dbm *= -1;

            // actual Ec/Io (multiplied by -10)
            out.CellInfo.cdma.signalStrengthCdma.ecio = pCellInfo->cell_info.cdma.sig_ecio;
            if (out.CellInfo.cdma.signalStrengthCdma.ecio < 0)
                out.CellInfo.cdma.signalStrengthCdma.ecio *= -10;

            // actual RSSI value (multiplied by -1)
            out.CellInfo.cdma.signalStrengthEvdo.dbm = pCellInfo->cell_info.cdma.evdo_sig_dbm;
            if (out.CellInfo.cdma.signalStrengthEvdo.dbm < 0)
                out.CellInfo.cdma.signalStrengthEvdo.dbm *= -1;

            // actual Ec/Io (multiplied by -10)
            out.CellInfo.cdma.signalStrengthEvdo.ecio = pCellInfo->cell_info.cdma.evdo_sig_ecio;
            if (out.CellInfo.cdma.signalStrengthEvdo.ecio < 0)
                out.CellInfo.cdma.signalStrengthEvdo.ecio *= -10;

            // signal noise ratio (0-8)
            out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = pCellInfo->cell_info.cdma.evdo_sig_snr;
            if (out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio < 0 || out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio > 8)
                out.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = 0;
        }
    }
    return size;
}

class CellInfoListParser {
private:
    const char *mData;
    int mCur;
public:
    CellInfoListParser(const char *data) {
        mData = data;
        Rewind();
    }

public:
    bool Next(RIL_CellInfo_v12 &cellInfo, int sitDataVer = 0) {
        if (mCur < 0)
            return false;

        int ret;
        if (sitDataVer == SIT_CELL_INFO_V12) {
            sit_net_cell_info_item_v12 *pCellInfo = (sit_net_cell_info_item_v12 *)(mData + mCur);

            ret = ::ConvertProtocolCellInfoV12ToRilCellInfo(cellInfo, pCellInfo);
        } else {
            sit_net_cell_info_item *pCellInfo = (sit_net_cell_info_item *)(mData + mCur);
            ret = ::ConvertProtocolCellInfoToRilCellInfo(cellInfo, pCellInfo);
        }

        if (ret == 0) {
            mCur = -1;
            return false;
        }

        mCur += ret;
        return true;
    }

    bool Next_V1_2(RIL_CellInfo_V1_2 &cellInfo, int sitDataVer = 0) {
        if (mCur < 0)
            return false;

        int ret;
        if (sitDataVer == SIT_CELL_INFO_V12) {
            sit_net_cell_info_item_v12 *pCellInfo = (sit_net_cell_info_item_v12 *)(mData + mCur);
            ret = ::ConvertProtocolCellInfoV1_2ToRilCellInfo(cellInfo, pCellInfo);
        } else {
            sit_net_cell_info_item *pCellInfo = (sit_net_cell_info_item *)(mData + mCur);
            ret = ::ConvertProtocolCellInfoToRilCellInfo(cellInfo, pCellInfo);
        }

        if (ret == 0) {
            mCur = -1;
            return false;
        }

        mCur += ret;
        return true;
    }

    void Rewind() {
        mCur = -1;
        if (mData != NULL) {
            mCur = 0;
        }
    }
};

ProtocolNetGetPsServiceAdapter::ProtocolNetGetPsServiceAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
}

int ProtocolNetGetPsServiceAdapter::GetState()
{
    if (m_pModemData != NULL && GetErrorCode() == RCM_E_SUCCESS) {
        sit_net_get_ps_service_rsp *data = (sit_net_get_ps_service_rsp *) m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_PS_SERVICE) {
            return (data->state == 1 ? ALLOW_DATA_CALL : DISALLOW_DATA_CALL);
        }
    }
    return ALLOW_DATA_CALL;
}

/**
 * ProtocolNetUplmnListAdapter
 */
ProtocolNetUplmnListAdapter::ProtocolNetUplmnListAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
}

int ProtocolNetUplmnListAdapter::GetSize() const
{
    if (m_pModemData != NULL && GetErrorCode() == RCM_E_SUCCESS) {
        sit_net_get_uplmn_rsp *data = (sit_net_get_uplmn_rsp *) m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_UPLMN) {
            if (data->plmn_list_num < 0) {
                return 0;
            }

            return data->plmn_list_num;
        }
    }
    return 0;
}

bool ProtocolNetUplmnListAdapter::GetPreferrecPlmn(PreferredPlmn &preferredPlmn, int index)
{
    if (index < 0) {
        return false;
    }

    if (m_pModemData != NULL && GetErrorCode() == RCM_E_SUCCESS) {
        sit_net_get_uplmn_rsp *data = (sit_net_get_uplmn_rsp *) m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_UPLMN) {
            if (index < 0 || index > data->plmn_list_num) {
                return false;
            }

            preferred_plmn_item *item = (data->preffered_plmn + index);
            if (item != NULL) {
                preferredPlmn.index = (int)item->index;
                memset(preferredPlmn.plmn, 0, sizeof(preferredPlmn.plmn));
                memcpy(preferredPlmn.plmn, item->plmn, 6);
                if (preferredPlmn.plmn[5] == '#') {
                    preferredPlmn.plmn[5] = 0;
                }
                preferredPlmn.act = (int)item->act;
                return true;
            }
        }
    }

    return false;
}

/**
 * ProtocolNetDuplexModeRespAdapter
 */
int ProtocolNetDuplexModeRespAdapter::Get4gDuplexMode() const
{
    sit_net_get_duplex_mode_rsp *data = (sit_net_get_duplex_mode_rsp *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_GET_DUPLEX_MODE) {
        return data->duplex_mode_4g;
    }
    return SIT_NET_DUPLEX_MODE_MAX;
}

int ProtocolNetDuplexModeRespAdapter::Get3gDuplexMode() const
{
    sit_net_get_duplex_mode_rsp *data = (sit_net_get_duplex_mode_rsp *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_GET_DUPLEX_MODE) {
        return data->duplex_mode_3g;
    }
    return SIT_NET_DUPLEX_MODE_MAX;
}

int ProtocolNetDuplexModeRespAdapter::GetDuplexMode() const
{
    int mode = DUPLEX_MODE_INVALID;
    int duplex_mode_4g = Get4gDuplexMode();
    int duplex_mode_3g = Get3gDuplexMode();

    if(duplex_mode_4g == SIT_NET_DUPLEX_MODE_TDD && duplex_mode_3g == SIT_NET_DUPLEX_MODE_TDD) {
        mode = DUPLEX_MODE_LTG;
    }
    else if (duplex_mode_4g == SIT_NET_DUPLEX_MODE_FDD_TDD && duplex_mode_3g == SIT_NET_DUPLEX_MODE_FDD) {
        mode = DUPLEX_MODE_LWG;
    }
    else if (duplex_mode_4g == SIT_NET_DUPLEX_MODE_FDD_TDD && duplex_mode_3g == SIT_NET_DUPLEX_MODE_FDD_TDD) {
        mode = DUPLEX_MODE_GLOBAL;
    }
    return mode;
}

/**
 * ProtocolNetEmergencyActInfoAdapter
 */
ProtocolNetEmergencyActInfoAdapter::ProtocolNetEmergencyActInfoAdapter(const ModemData *pModemData) : ProtocolIndAdapter(pModemData)
{
}

int ProtocolNetEmergencyActInfoAdapter::GetRat() const
{
    sit_net_emergency_act_info_ind *data = (sit_net_emergency_act_info_ind *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_IND_EMERGENCY_ACT_INFO) {
        int rat = (int)(data->rat);
        if (rat == SIT_RAT_TYPE_TD_SCDMA) {
            return RADIO_TECH_TD_SCDMA;
        } else if (rat == SIT_RAT_TYPE_HSPADCPLUS) {
            return RADIO_TECH_HSPAP;
        } else if (rat == SIT_RAT_TYPE_LTE_CA) {
            // consider as RADIO_TECH_LTE
            return RADIO_TECH_LTE;
        } else if (rat == SIT_RAT_TYPE_5G) {
            return RADIO_TECH_NR;
        } else if (rat == SIT_RAT_TYPE_UNSPECIFIED) {
            return RADIO_TECH_UNSPECIFIED;
        }
        return rat;
    }
    return RADIO_TECH_UNKNOWN;
}

int ProtocolNetEmergencyActInfoAdapter::GetActStatus() const
{
    sit_net_emergency_act_info_ind *data = (sit_net_emergency_act_info_ind *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_IND_EMERGENCY_ACT_INFO) {
        if ((int)data->act_status == SIT_NET_CURRENT_ACT_EMERGENCY_CALL) {
            return EMERGENCY_CALL_AVAILABLE;
        } else if ((int)data->act_status == SIT_NET_RETRY_ACT_EMERGENCY_CALL) {
            return EMERGENCY_CALL_RETRY;
        }
    }
    return EMERGENCY_CALL_NOT_AVAILABLE;
}

/**
 * ProtocolNetMcSrchRespAdapter
 */
ProtocolNetMcSrchRespAdapter::ProtocolNetMcSrchRespAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
    memset(m_szPlmn, 0, sizeof(m_szPlmn));

    sit_net_set_micro_cell_search_rsp *data = (sit_net_set_micro_cell_search_rsp *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_SET_MICRO_CELL_SEARCH) {
        if (*data->plmn != 0) {
            memcpy(m_szPlmn, data->plmn, MAX_PLMN_LEN);
            if (m_szPlmn[5] == '#') {
                m_szPlmn[5] = 0;
            }
        }
    }
}

int ProtocolNetMcSrchRespAdapter::GetMcSrchResult() const
{
    sit_net_set_micro_cell_search_rsp *data = (sit_net_set_micro_cell_search_rsp *)m_pModemData->GetRawData();
    if (data != NULL && data->hdr.id == SIT_SET_MICRO_CELL_SEARCH) {
        return data->srch_result;
    }
    return -1;
}

const char *ProtocolNetMcSrchRespAdapter::GetMcSrchPlmn() const
{
    return (m_szPlmn[0] == 0 ? NULL : m_szPlmn);
}

/**
 * ProtocolGetNetworkRCRespAdapter
 */
int ProtocolGetNetworkRCRespAdapter::GetVersion() const
{
    int ret = RIL_RADIO_CAPABILITY_VERSION;
    if (m_pModemData != NULL) {
        sit_net_get_rc_rsp *data = (sit_net_get_rc_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_RADIO_CAPABILITY) {
            ret = data->version;
        }
    }
    return ret;
}

int ProtocolGetNetworkRCRespAdapter::GetSession() const
{
    int ret = -1;
    if (m_pModemData != NULL) {
        sit_net_get_rc_rsp *data = (sit_net_get_rc_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_RADIO_CAPABILITY) {
            ret = data->session_id;
        }
    }
    return ret;
}

int ProtocolGetNetworkRCRespAdapter::GetPhase() const
{
    int ret = RC_PHASE_CONFIGURED;
    if (m_pModemData != NULL) {
        sit_net_get_rc_rsp *data = (sit_net_get_rc_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_RADIO_CAPABILITY) {
            ret = data->phase;
        }
    }
    return ret;
}

int ProtocolGetNetworkRCRespAdapter::GetRafType() const
{
    int ret = RAF_CP_UNKNOWN;
    if (m_pModemData != NULL) {
        sit_net_get_rc_rsp *data = (sit_net_get_rc_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_RADIO_CAPABILITY) {
            ret = switchRafValueForFW(data->rc_raf);
        }
    }
    return ret;
}

BYTE *ProtocolGetNetworkRCRespAdapter::GetUuid() const
{
    BYTE *pRet = NULL;
    if (m_pModemData != NULL) {
        sit_net_get_rc_rsp *data = (sit_net_get_rc_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_RADIO_CAPABILITY) {
            pRet = data->uuid;
        }
    }
    return pRet;
}

int ProtocolGetNetworkRCRespAdapter::GetStatus() const
{
    int ret = RC_STATUS_NONE;
    if (m_pModemData != NULL) {
        sit_net_get_rc_rsp *data = (sit_net_get_rc_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_RADIO_CAPABILITY) {
            ret = data->status;
        }
    }
    return ret;
}

/**
 * ProtocolNetworkRCIndAdapter
 */
int ProtocolNetworkRCIndAdapter::GetVersion() const
{
    int ret = RIL_RADIO_CAPABILITY_VERSION;
    if (m_pModemData != NULL) {
        sit_net_radio_capability_ind *data = (sit_net_radio_capability_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_RADIO_CAPABILITY) {
            ret = data->version;
        }
    }
    return ret;
}

int ProtocolNetworkRCIndAdapter::GetSession() const
{
    int ret = -1;
    if (m_pModemData != NULL) {
        sit_net_radio_capability_ind *data = (sit_net_radio_capability_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_RADIO_CAPABILITY) {
            ret = data->session_id;
        }
    }
    return ret;
}

int ProtocolNetworkRCIndAdapter::GetPhase() const
{
    int ret = RC_PHASE_CONFIGURED;
    if (m_pModemData != NULL) {
        sit_net_radio_capability_ind *data = (sit_net_radio_capability_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_RADIO_CAPABILITY) {
            ret = data->phase;
        }
    }
    return ret;
}

int ProtocolNetworkRCIndAdapter::GetRafType() const
{
    int ret = RAF_CP_UNKNOWN;
    if (m_pModemData != NULL) {
        sit_net_radio_capability_ind *data = (sit_net_radio_capability_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_RADIO_CAPABILITY) {
            ret = data->rc_raf;
        }
    }
    return ret;
}

BYTE *ProtocolNetworkRCIndAdapter::GetUuid() const
{
    BYTE *pRet = NULL;
    if (m_pModemData != NULL) {
        sit_net_radio_capability_ind *data = (sit_net_radio_capability_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_RADIO_CAPABILITY) {
            pRet = data->uuid;
        }
    }
    return pRet;
}

int ProtocolNetworkRCIndAdapter::GetStatus() const
{
    int ret = RC_STATUS_NONE;
    if (m_pModemData != NULL) {
        sit_net_radio_capability_ind *data = (sit_net_radio_capability_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_RADIO_CAPABILITY) {
            ret = data->status;
        }
    }
    return ret;
}

/**
 * ProtocolNetworkSgcBearerAllocIndAdapter
 */
ProtocolNetworkSgcBearerAllocIndAdapter::ProtocolNetworkSgcBearerAllocIndAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData) {
    mRat = RADIO_TECH_UNKNOWN;
    mConnectionStatus = NONE;

    if (m_pModemData != NULL) {
        sit_net_sgc_bearer_allocation_ind *data = (sit_net_sgc_bearer_allocation_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_SCG_BEARER_ALLOCATION) {
            mRat = ConvertRadioTechValue((int)data->rat);
            mConnectionStatus = (data->scg_status == 1) ? SECONDARY_SERVING : PRIMARY_SERVING;
            if (mConnectionStatus == SECONDARY_SERVING) {
                mRat = RADIO_TECH_NR;
            }
        }
    }
}

#ifdef SUPPORT_CDMA
/*
 * ProtocolNetCdmaQueryRoamingTypeAdapter
 */
int ProtocolNetCdmaQueryRoamingTypeAdapter::QueryRoamingType() const
{
    if (m_pModemData != NULL) {
        sit_net_query_cdma_roaming_rsp *data = (sit_net_query_cdma_roaming_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CDMA_ROAMING_PREFERENCE) {
            RilLogV("Cdma Query RoamingType=%d", data->cdma_roaming_type);
            switch(data->cdma_roaming_type) {
            case SIT_CDMA_RM_AFFILIATED:
                return CDMA_ROAMING_AFFILIATED_NETWORKS;
            case SIT_CDMA_RM_ANY:
                return CDMA_ROAMING_ANY_NETWORK;
            case SIT_CDMA_RM_HOME:
            default:
                return CDMA_ROAMING_HOME_ONLY;
            }
        }
    }
    return CDMA_ROAMING_HOME_ONLY;
}

/*
 * ProtocolNetCdmaHybridModeAdapter
 */
int ProtocolNetCdmaHybridModeAdapter::GetCdmaHybridMode() const
{
    if (m_pModemData != NULL) {
        sit_net_get_cdma_hybrid_mode_rsp *data = (sit_net_get_cdma_hybrid_mode_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_CDMA_HYBRID_MODE) {
            RilLogV("Get CdmaHybridMode=%s(0x%02x)", ConvertCdmaHybridModeToString(data->hybrid_mode), data->hybrid_mode);
            return data->hybrid_mode;
        }
    }
    RilLogV("Get default CdmaHybridMode=%s(0x%02x)", ConvertCdmaHybridModeToString(HYBRID_MODE_1X_HRPD), HYBRID_MODE_1X_HRPD);
    return HYBRID_MODE_1X_HRPD;
}
#endif

int ProtocolNetTotalOosAdapter::GetCurrentPrefNetworkMode() const
{
    if (m_pModemData != NULL) {
        sit_net_total_oos_ind *data = (sit_net_total_oos_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_TOTAL_OOS) {
            RilLogV("Get Pref.NetMode in TotalOos=%s(0x%02x)", ConvertPreferredNetTypeToString(data->pref_net_type), data->pref_net_type);
            return (int)(data->pref_net_type);
        }
    }
    RilLogV("Get default Pref.NetMode in TotalOos=%s(0x%02x)", ConvertPreferredNetTypeToString(SIT_NET_PREF_NET_TYPE_GSM_WCDMA), SIT_NET_PREF_NET_TYPE_GSM_WCDMA);
    return SIT_NET_PREF_NET_TYPE_GSM_WCDMA;
}

ProtocolNetMccAdapter::ProtocolNetMccAdapter(const ModemData *pModemData) : ProtocolIndAdapter (pModemData) {
    memset(mMcc, 0, sizeof(mMcc));
    if (m_pModemData != NULL) {
        sit_net_mcc_ind *data = (sit_net_mcc_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_MCC) {
            strncpy(mMcc, data->mcc, 3);
        }
    }
}

int ProtocolNetMccAdapter::GetCurrentPrefNetworkMode() const
{
    if (m_pModemData != NULL) {
        sit_net_mcc_ind *data = (sit_net_mcc_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_MCC) {
            RilLogV("Get Pref.NetMode in Mcc=%s(0x%02x)", ConvertPreferredNetTypeToString(data->pref_net_type), data->pref_net_type);
            return (int)(data->pref_net_type);
        }
    }
    return SIT_NET_PREF_NET_TYPE_GSM_WCDMA;
}

class ProtocolCellInfoList {
public:
    int mCellInfoNum;
    char *mData;
    int mDataLen;
    list<RIL_CellInfo_V1_4> mCellInfoList;
public:
    ProtocolCellInfoList(void *data, int dataLen, int cellInfoNum)
        : mCellInfoNum(0), mData(NULL), mDataLen(0) {
        // always use SIT_CELL_INFO_V14
        int sitVersion = SIT_CELL_INFO_V14;
        mCellInfoNum = cellInfoNum;
        RilLog("sitVersion=%d, mCellInfoNum=%d", sitVersion, mCellInfoNum);
        if (cellInfoNum > 0 && data != NULL && dataLen > 0) {
            mData = new char[dataLen];
            if (mData != NULL) {
                memcpy(mData, data, dataLen);
                mDataLen = dataLen;
                if (sitVersion < SIT_CELL_INFO_V14) Parse();
                else ParseV14();
            }
        }
    }
    virtual ~ProtocolCellInfoList() {
        if (mData != NULL) {
            delete[] mData;
        }
    }
    list<RIL_CellInfo_V1_4>& GetCellInfoList() { return mCellInfoList; }

    void Parse() {
        if (mData == NULL || mDataLen <= 0 || mCellInfoNum <= 0) {
            return ;
        }

        uint64_t timeStamp = ril_nano_time();
        int pos = 0;
        for (int i = 0; i < mCellInfoNum; i++) {
            sit_net_cell_info_item_v12 *p_cur = (sit_net_cell_info_item_v12 *)(mData + pos);
            RIL_CellInfo_V1_4 rilCellInfo;
            memset(&rilCellInfo, 0, sizeof(rilCellInfo));
            rilCellInfo.cellInfoType = (RIL_CellInfoType)(p_cur->cell_info_type + 1);
            rilCellInfo.registered = p_cur->reg_status;
            rilCellInfo.timeStamp = timeStamp;
            rilCellInfo.timeStampType = RIL_TIMESTAMP_TYPE_OEM_RIL;
            if (rilCellInfo.registered) {
                rilCellInfo.connectionStatus = PRIMARY_SERVING;
            }
            initCellInfo(rilCellInfo, (int)rilCellInfo.cellInfoType);
            pos += 2;   // cellInfoType, reg_status
            switch ((int)rilCellInfo.cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM:
                FillGsmCellInfo(rilCellInfo.CellInfo.gsm, p_cur->cell_info.gsm);
                pos += sizeof(cell_info_gsm_v12);
                break;
            case RIL_CELL_INFO_TYPE_CDMA:
                FillCdmaCellInfo(rilCellInfo.CellInfo.cdma, p_cur->cell_info.cdma);
                pos += sizeof(cell_info_cdma);
                break;
            case RIL_CELL_INFO_TYPE_LTE:
                FillLteCellInfo(rilCellInfo.CellInfo.lte, p_cur->cell_info.lte);
                pos += sizeof(cell_info_lte_v12);
                break;
            case RIL_CELL_INFO_TYPE_WCDMA:
                FillWcdmaCellInfo(rilCellInfo.CellInfo.wcdma, p_cur->cell_info.wcdma);
                pos += sizeof(cell_info_wcdma_v12);
                break;
            case RIL_CELL_INFO_TYPE_TD_SCDMA:
                FillTdscdmaCellInfo(rilCellInfo.CellInfo.tdscdma, p_cur->cell_info.tdscdma);
                pos += sizeof(cell_info_tdscdma);
                break;
            default:
                // TODO stop parsing because of untrusted data type
                // TODO prepare NR
                RilLogW("[%s][%d] Unsupported CellInfoType %d", __FUNCTION__, (int)rilCellInfo.cellInfoType);
                return;
            }
            mCellInfoList.push_back(rilCellInfo);
            if (debug) {
                printCellInfo(i, rilCellInfo);
            }
        } // end for i ~
    }

    void ParseV14() {
        if (mData == NULL || mDataLen <= 0 || mCellInfoNum <= 0) {
            return ;
        }

        uint64_t timeStamp = ril_nano_time();
        int pos = 0;
        for (int i = 0; i < mCellInfoNum; i++) {
            sit_net_cell_info_item_v14 *p_cur = (sit_net_cell_info_item_v14 *)(mData + pos);
            RIL_CellInfo_V1_4 rilCellInfo;
            memset(&rilCellInfo, 0, sizeof(rilCellInfo));
            rilCellInfo.cellInfoType = (RIL_CellInfoType)(p_cur->cell_info_type + 1);
            rilCellInfo.registered = p_cur->reg_status;
            rilCellInfo.timeStamp = timeStamp;
            rilCellInfo.timeStampType = RIL_TIMESTAMP_TYPE_OEM_RIL;
            rilCellInfo.connectionStatus = (RIL_CellConnectionStatus)p_cur->cell_connection_status;
            initCellInfo(rilCellInfo, (int)rilCellInfo.cellInfoType);

            pos += 3;   // cellInfoType, reg_status, cell_connection_status
            switch ((int)rilCellInfo.cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM:
                FillGsmCellInfo(rilCellInfo.CellInfo.gsm, p_cur->cell_info.gsm);
                pos += sizeof(cell_info_gsm_v12);
                break;
            case RIL_CELL_INFO_TYPE_CDMA:
                FillCdmaCellInfo(rilCellInfo.CellInfo.cdma, p_cur->cell_info.cdma);
                pos += sizeof(cell_info_cdma_v14);
                break;
            case RIL_CELL_INFO_TYPE_LTE:
                FillLteCellInfo(rilCellInfo.CellInfo.lte, p_cur->cell_info.lte);
                pos += sizeof(cell_info_lte_v14);
                break;
            case RIL_CELL_INFO_TYPE_WCDMA:
                FillWcdmaCellInfo(rilCellInfo.CellInfo.wcdma, p_cur->cell_info.wcdma);
                pos += sizeof(cell_info_wcdma_v14);
                break;
            case RIL_CELL_INFO_TYPE_TD_SCDMA:
                FillTdscdmaCellInfo(rilCellInfo.CellInfo.tdscdma, p_cur->cell_info.tdscdma);
                pos += sizeof(cell_info_tdscdma_v14);
                break;
            case RIL_CELL_INFO_TYPE_NR:
                FillNrCellInfo(rilCellInfo.CellInfo.nr, p_cur->cell_info.nr);
                pos += sizeof(cell_info_nr);
                break;
            default:
                // TODO stop parsing because of untrusted data type
                RilLogW("[%s][%d] Unsupported CellInfoType %d", __FUNCTION__, (int)rilCellInfo.cellInfoType);
                return;
            }
            mCellInfoList.push_back(rilCellInfo);
            if (debug) {
                printCellInfo(i, rilCellInfo);
            }
        } // end for i ~
    }

    void initCellInfo(RIL_CellInfo_V1_4& info, int type) {
        switch (type) {
        case RIL_CELL_INFO_TYPE_GSM:
            // RIL_CellInfoGsm_V1_2
            info.CellInfo.gsm.cellIdentityGsm.mcc = INT_MAX;
            info.CellInfo.gsm.cellIdentityGsm.mnc = INT_MAX;
            info.CellInfo.gsm.cellIdentityGsm.lac = INT_MAX;
            info.CellInfo.gsm.cellIdentityGsm.cid = INT_MAX;
            info.CellInfo.gsm.cellIdentityGsm.arfcn = INT_MAX;
            info.CellInfo.gsm.cellIdentityGsm.bsic = 0xff;

            info.CellInfo.gsm.signalStrengthGsm.signalStrength = 99;
            info.CellInfo.gsm.signalStrengthGsm.bitErrorRate = 99;
            info.CellInfo.gsm.signalStrengthGsm.timingAdvance = INT_MAX;
            break;
        case RIL_CELL_INFO_TYPE_CDMA:
            // RIL_CellInfoCdma_V1_2
            info.CellInfo.cdma.cellIdentityCdma.networkId = INT_MAX;
            info.CellInfo.cdma.cellIdentityCdma.systemId = INT_MAX;
            info.CellInfo.cdma.cellIdentityCdma.basestationId = INT_MAX;
            info.CellInfo.cdma.cellIdentityCdma.longitude = INT_MAX;
            info.CellInfo.cdma.cellIdentityCdma.latitude = INT_MAX;

            info.CellInfo.cdma.signalStrengthCdma.dbm = 0;
            info.CellInfo.cdma.signalStrengthCdma.ecio = 0;

            info.CellInfo.cdma.signalStrengthEvdo.dbm = 0;
            info.CellInfo.cdma.signalStrengthEvdo.ecio = 0;
            info.CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio = INT_MAX;
            break;
        case RIL_CELL_INFO_TYPE_LTE:
            // RIL_CellInfoLte_V1_4
            info.CellInfo.lte.cellInfo.cellIdentityLte.mcc = INT_MAX;
            info.CellInfo.lte.cellInfo.cellIdentityLte.mnc = INT_MAX;
            info.CellInfo.lte.cellInfo.cellIdentityLte.ci = INT_MAX;
            info.CellInfo.lte.cellInfo.cellIdentityLte.pci = INT_MAX;
            info.CellInfo.lte.cellInfo.cellIdentityLte.tac = INT_MAX;
            info.CellInfo.lte.cellInfo.cellIdentityLte.earfcn = INT_MAX;
            info.CellInfo.lte.cellInfo.cellIdentityLte.bandwidth = INT_MAX;

            info.CellInfo.lte.cellInfo.signalStrengthLte.signalStrength = 99;
            info.CellInfo.lte.cellInfo.signalStrengthLte.rsrp = INT_MAX;
            info.CellInfo.lte.cellInfo.signalStrengthLte.rsrq = INT_MAX;
            info.CellInfo.lte.cellInfo.signalStrengthLte.rssnr = INT_MAX;
            info.CellInfo.lte.cellInfo.signalStrengthLte.cqi = INT_MAX;
            info.CellInfo.lte.cellInfo.signalStrengthLte.timingAdvance = INT_MAX;

            info.CellInfo.lte.cellConfig.isEndcAvailable = false;
            break;
        case RIL_CELL_INFO_TYPE_WCDMA:
            // RIL_CellInfoWcdma_V1_2
            info.CellInfo.wcdma.cellIdentityWcdma.mcc = INT_MAX;
            info.CellInfo.wcdma.cellIdentityWcdma.mnc = INT_MAX;
            info.CellInfo.wcdma.cellIdentityWcdma.lac = INT_MAX;
            info.CellInfo.wcdma.cellIdentityWcdma.cid = INT_MAX;
            info.CellInfo.wcdma.cellIdentityWcdma.psc = INT_MAX;
            info.CellInfo.wcdma.cellIdentityWcdma.uarfcn = INT_MAX;

            info.CellInfo.wcdma.signalStrengthWcdma.signalStrength = 99;
            info.CellInfo.wcdma.signalStrengthWcdma.bitErrorRate = 99;
            info.CellInfo.wcdma.signalStrengthWcdma.rscp = 255;
            info.CellInfo.wcdma.signalStrengthWcdma.ecno = 255;
            break;
        case RIL_CELL_INFO_TYPE_TD_SCDMA:
            // RIL_CellInfoTdscdma_V1_2
            info.CellInfo.tdscdma.cellIdentityTdscdma.mcc = INT_MAX;
            info.CellInfo.tdscdma.cellIdentityTdscdma.mnc = INT_MAX;
            info.CellInfo.tdscdma.cellIdentityTdscdma.lac = INT_MAX;
            info.CellInfo.tdscdma.cellIdentityTdscdma.cid = INT_MAX;
            info.CellInfo.tdscdma.cellIdentityTdscdma.cpid = INT_MAX;
            info.CellInfo.tdscdma.cellIdentityTdscdma.uarfcn = INT_MAX;

            info.CellInfo.tdscdma.signalStrengthTdscdma.signalStrength = 99;
            info.CellInfo.tdscdma.signalStrengthTdscdma.bitErrorRate = 99;
            info.CellInfo.tdscdma.signalStrengthTdscdma.rscp = 255;
            break;
        case RIL_CELL_INFO_TYPE_NR:
            // RIL_CellInfoNr_V1_4
            info.CellInfo.nr.cellidentityNr.mcc = INT_MAX;
            info.CellInfo.nr.cellidentityNr.mnc = INT_MAX;
            info.CellInfo.nr.cellidentityNr.nci = LONG_MAX;
            info.CellInfo.nr.cellidentityNr.pci = 0;
            info.CellInfo.nr.cellidentityNr.tac = INT_MAX;
            info.CellInfo.nr.cellidentityNr.nrarfcn = 0;

            info.CellInfo.nr.signalStrengthNr.ssRsrp = INT_MAX;
            info.CellInfo.nr.signalStrengthNr.ssRsrq = INT_MAX;
            info.CellInfo.nr.signalStrengthNr.ssSinr = INT_MAX;
            info.CellInfo.nr.signalStrengthNr.csiRsrp = INT_MAX;
            info.CellInfo.nr.signalStrengthNr.csiRsrq = INT_MAX;
            info.CellInfo.nr.signalStrengthNr.csiSinr = INT_MAX;
            break;
        }
    }

    void printCellInfo(int index, RIL_CellInfo_V1_4& cellInfo) {
        switch ((int)cellInfo.cellInfoType) {
        case RIL_CELL_INFO_TYPE_GSM:
            RilLog("[%d] RIL_CELL_INFO_TYPE_GSM", index);
            // TODO print more information
            break;
        case RIL_CELL_INFO_TYPE_CDMA:
            RilLog("[%d] RIL_CELL_INFO_TYPE_CDMA", index);
            // TODO print more information
            break;
        case RIL_CELL_INFO_TYPE_LTE:
            RilLog("[%d] RIL_CELL_INFO_TYPE_LTE", index);
            // TODO print more information
            break;
        case RIL_CELL_INFO_TYPE_WCDMA:
            RilLog("[%d] RIL_CELL_INFO_TYPE_WCDMA", index);
            // TODO print more information
            break;
        case RIL_CELL_INFO_TYPE_TD_SCDMA:
            RilLog("[%d] RIL_CELL_INFO_TYPE_TD_SCDMA", index);
            // TODO print more information
            break;
        case RIL_CELL_INFO_TYPE_NR:
            RilLog("[%d] RIL_CELL_INFO_TYPE_NR", index);
            // TODO print more information
            break;
        default:
            RilLog("[%d] Unsupported CellInfoType %d", index, (int)cellInfo.cellInfoType);
            break;
        }
    }

    int FetchMcc(const char *plmn) {
        char mccStr[4] = {0, };
        if (plmn != NULL) {
            memcpy(mccStr, plmn, 3);
            if (TextUtils::IsDigitsOnly(mccStr)) {
                int mcc = atoi(mccStr);
#if 0
                int mccLength = 3;
                if (mcc > 0) {
                    mcc = ril::util::mnc::encode(mcc, mccLength);
                    return mcc;
                }
#else
                return mcc;
#endif
            }
        }
        return INT_MAX;
    }

    int FetchMnc(const char *plmn) {
        int mcc = FetchMcc(plmn);
        if (mcc == 0 || mcc == INT_MAX) {
            return INT_MAX;
        }

        char mncStr[4] = {0, };
        if (plmn != NULL) {
            memcpy(mncStr, plmn + 3, 3);
            if (mncStr[2] == '#') {
                mncStr[2] = 0;
            }

            if (TextUtils::IsDigitsOnly(mncStr)) {
                int mnc = atoi(mncStr);
                int mccLength = strlen(mncStr);
                mnc = ril::util::mnc::encode(mnc, mccLength);
                return mnc;
            }
        }
        return INT_MAX;
    }

    void FillOperatorName(RIL_CellIdentityOperatorNames& name, int mcc, int encodedMnc) {
        TS25Record ret;
        int mnc = atoi(ril::util::mnc::decode(encodedMnc).c_str());
        RilLog("[%s] Find TS25Table for mcc=%d mnc=%d", __FUNCTION__, mcc, mnc);
        TS25Table *table = TS25Table::GetInstance();
        if (table != NULL) {
            ret = table->GetRecord(mcc, mnc);
        }

        if (ret.IsValid()) {
            strncpy(name.alphaLong, ret.ppcin.c_str(), MAX_FULL_NAME_LEN-1);
            strncpy(name.alphaShort, ret.networkName.c_str(), MAX_SHORT_NAME_LEN-1);
            RilLog("[%s] TS25 record found %d/%d/%s/%s", __FUNCTION__,
                    mcc, mnc, name.alphaLong, name.alphaShort);
        } else {
            RilLog("[%s] No TS25 record for mcc=%d, mnc=%d", __FUNCTION__, mcc, mnc);
        }
    }

    void FillGsmCellInfo(RIL_CellInfoGsm_V1_2& out, cell_info_gsm_v12& cellInfo) {
        out.cellIdentityGsm.mcc = FetchMcc(cellInfo.plmn);
        out.cellIdentityGsm.mnc = FetchMnc(cellInfo.plmn);

        // 16-bit LAC
        out.cellIdentityGsm.lac = cellInfo.lac;
        if (out.cellIdentityGsm.lac < 0 || out.cellIdentityGsm.lac > 0xFFFF)
            out.cellIdentityGsm.lac = INT32_MAX;

        // 16-bit GSM CID
        out.cellIdentityGsm.cid = cellInfo.cid;
        if (out.cellIdentityGsm.cid < 0 || out.cellIdentityGsm.cid > 0xFFFF)
            out.cellIdentityGsm.cid = INT32_MAX;

        // 16-bit GSM ARFCN
        out.cellIdentityGsm.arfcn = cellInfo.arfcn;
        if (out.cellIdentityGsm.arfcn < 0 || out.cellIdentityGsm.arfcn > 0xFFFF)
            out.cellIdentityGsm.arfcn = INT32_MAX;

        // 6-bit GSM BSIC
        out.cellIdentityGsm.bsic = cellInfo.bsic;
        if (out.cellIdentityGsm.bsic > 0x3F)
            out.cellIdentityGsm.bsic = 0xFF;

        // Operator name
        FillOperatorName(out.cellIdentityGsm.operatorNames, out.cellIdentityGsm.mcc, out.cellIdentityGsm.mnc);

        // BER (0-7,99)
        out.signalStrengthGsm.bitErrorRate = cellInfo.sig_ber;
        if (out.signalStrengthGsm.bitErrorRate < 0 || out.signalStrengthGsm.bitErrorRate > 7)
            out.signalStrengthGsm.bitErrorRate = 99;

        // Signal Strength (0-31,99)
        out.signalStrengthGsm.signalStrength = cellInfo.sig_str;
        if (out.signalStrengthGsm.signalStrength < 0 || out.signalStrengthGsm.signalStrength > 31)
            out.signalStrengthGsm.signalStrength = 99;

        // Timing Advance (0 ~63)
        out.signalStrengthGsm.timingAdvance = cellInfo.sig_ta;
        if (out.signalStrengthGsm.timingAdvance < 0 || out.signalStrengthGsm.timingAdvance > 63)
            out.signalStrengthGsm.timingAdvance = INT32_MAX;
    }

    void FillCdmaCellInfo(RIL_CellInfoCdma_V1_2& out, cell_info_cdma& cellInfo) {
        // 16-bit Network Id
        out.cellIdentityCdma.networkId = cellInfo.ntw_id;
        if (out.cellIdentityCdma.networkId < 0 || out.cellIdentityCdma.networkId > 0xFFFF)
            out.cellIdentityCdma.networkId = INT32_MAX;

        // 15-bit CDMA System Id
        out.cellIdentityCdma.systemId = cellInfo.sys_id;
        if (out.cellIdentityCdma.systemId < 0 || out.cellIdentityCdma.systemId > 0x7FFF)
            out.cellIdentityCdma.systemId = INT32_MAX;

        // 16-bit Base Station Id
        out.cellIdentityCdma.basestationId = cellInfo.bs_id;
        if (out.cellIdentityCdma.basestationId < 0 || out.cellIdentityCdma.basestationId > 0xFFFF)
            out.cellIdentityCdma.basestationId = INT32_MAX;

        // Longitude (-2592000-2592000)
        out.cellIdentityCdma.longitude = cellInfo.longitude;
        if (out.cellIdentityCdma.longitude < -2592000 || out.cellIdentityCdma.longitude > 2592000)
            out.cellIdentityCdma.longitude = INT32_MAX;

        // Latitude (-1296000-1296000)
        out.cellIdentityCdma.latitude = cellInfo.lat;
        if (out.cellIdentityCdma.latitude < -1296000 || out.cellIdentityCdma.latitude > 1296000)
            out.cellIdentityCdma.latitude = INT32_MAX;

        // actual RSSI value (multiplied by -1)
        out.signalStrengthCdma.dbm = cellInfo.sig_dbm;
        if (out.signalStrengthCdma.dbm < 0)
            out.signalStrengthCdma.dbm *= -1;

        // actual Ec/Io (multiplied by -10)
        out.signalStrengthCdma.ecio = cellInfo.sig_ecio;
        if (out.signalStrengthCdma.ecio < 0)
            out.signalStrengthCdma.ecio *= -10;

        // actual RSSI value (multiplied by -1)
        out.signalStrengthEvdo.dbm = cellInfo.evdo_sig_dbm;
        if (out.signalStrengthEvdo.dbm < 0)
            out.signalStrengthEvdo.dbm *= -1;

        // actual Ec/Io (multiplied by -10)
        out.signalStrengthEvdo.ecio = cellInfo.evdo_sig_ecio;
        if (out.signalStrengthEvdo.ecio < 0)
            out.signalStrengthEvdo.ecio *= -10;

        // signal noise ratio (0-8)
        out.signalStrengthEvdo.signalNoiseRatio = cellInfo.evdo_sig_snr;
        if (out.signalStrengthEvdo.signalNoiseRatio < 0 || out.signalStrengthEvdo.signalNoiseRatio > 8)
            out.signalStrengthEvdo.signalNoiseRatio = 0;
    }

    void FillCdmaCellInfo(RIL_CellInfoCdma_V1_2& out, cell_info_cdma_v14& cellInfo) {
        // 16-bit Network Id
        out.cellIdentityCdma.networkId = cellInfo.ntw_id;
        if (out.cellIdentityCdma.networkId < 0 || out.cellIdentityCdma.networkId > 0xFFFF)
            out.cellIdentityCdma.networkId = INT32_MAX;

        // 15-bit CDMA System Id
        out.cellIdentityCdma.systemId = cellInfo.sys_id;
        if (out.cellIdentityCdma.systemId < 0 || out.cellIdentityCdma.systemId > 0x7FFF)
            out.cellIdentityCdma.systemId = INT32_MAX;

        // 16-bit Base Station Id
        out.cellIdentityCdma.basestationId = cellInfo.bs_id;
        if (out.cellIdentityCdma.basestationId < 0 || out.cellIdentityCdma.basestationId > 0xFFFF)
            out.cellIdentityCdma.basestationId = INT32_MAX;

        // Longitude (-2592000-2592000)
        out.cellIdentityCdma.longitude = cellInfo.longitude;
        if (out.cellIdentityCdma.longitude < -2592000 || out.cellIdentityCdma.longitude > 2592000)
            out.cellIdentityCdma.longitude = INT32_MAX;

        // Latitude (-1296000-1296000)
        out.cellIdentityCdma.latitude = cellInfo.lat;
        if (out.cellIdentityCdma.latitude < -1296000 || out.cellIdentityCdma.latitude > 1296000)
            out.cellIdentityCdma.latitude = INT32_MAX;

        // actual RSSI value (multiplied by -1)
        out.signalStrengthCdma.dbm = cellInfo.sig_dbm;
        if (out.signalStrengthCdma.dbm < 0)
            out.signalStrengthCdma.dbm *= -1;

        // actual Ec/Io (multiplied by -10)
        out.signalStrengthCdma.ecio = cellInfo.sig_ecio;
        if (out.signalStrengthCdma.ecio < 0)
            out.signalStrengthCdma.ecio *= -10;

        // actual RSSI value (multiplied by -1)
        out.signalStrengthEvdo.dbm = cellInfo.evdo_sig_dbm;
        if (out.signalStrengthEvdo.dbm < 0)
            out.signalStrengthEvdo.dbm *= -1;

        // actual Ec/Io (multiplied by -10)
        out.signalStrengthEvdo.ecio = cellInfo.evdo_sig_ecio;
        if (out.signalStrengthEvdo.ecio < 0)
            out.signalStrengthEvdo.ecio *= -10;

        // signal noise ratio (0-8)
        out.signalStrengthEvdo.signalNoiseRatio = cellInfo.evdo_sig_snr;
        if (out.signalStrengthEvdo.signalNoiseRatio < 0 || out.signalStrengthEvdo.signalNoiseRatio > 8)
            out.signalStrengthEvdo.signalNoiseRatio = 0;
    }

    void FillLteCellInfo(RIL_CellInfoLte_V1_4& out, cell_info_lte_v12& cellInfo) {
        out.cellInfo.cellIdentityLte.mcc = FetchMcc(cellInfo.plmn);
        out.cellInfo.cellIdentityLte.mnc = FetchMnc(cellInfo.plmn);

        // 28-bit CID
        out.cellInfo.cellIdentityLte.ci = cellInfo.cell_id;
        if (out.cellInfo.cellIdentityLte.ci < 0 || out.cellInfo.cellIdentityLte.ci > 0xFFFFFFF)
            out.cellInfo.cellIdentityLte.ci = INT32_MAX;

        // Physical CID
        out.cellInfo.cellIdentityLte.pci = cellInfo.phy_cell_id;
        if (out.cellInfo.cellIdentityLte.pci < 0 || out.cellInfo.cellIdentityLte.pci > 503)
            out.cellInfo.cellIdentityLte.pci = INT32_MAX;

        // 16-bit TAC
        out.cellInfo.cellIdentityLte.tac = cellInfo.tac;
        if (out.cellInfo.cellIdentityLte.tac < 0 || out.cellInfo.cellIdentityLte.tac > 0xFFFF)
            out.cellInfo.cellIdentityLte.tac = INT32_MAX;

        // 18-bit earfcn
        out.cellInfo.cellIdentityLte.earfcn = cellInfo.earfcn;
        if (out.cellInfo.cellIdentityLte.earfcn < 0 || out.cellInfo.cellIdentityLte.earfcn > 0x3FFFF)
            out.cellInfo.cellIdentityLte.earfcn = INT32_MAX;

        // Operator name
        FillOperatorName(out.cellInfo.cellIdentityLte.operatorNames, out.cellInfo.cellIdentityLte.mcc, out.cellInfo.cellIdentityLte.mnc);

        // Signal Strength (0-31,99)
        out.cellInfo.signalStrengthLte.signalStrength = cellInfo.sig_str;
        if (out.cellInfo.signalStrengthLte.signalStrength < 0 || out.cellInfo.signalStrengthLte.signalStrength > 31)
            out.cellInfo.signalStrengthLte.signalStrength = 99;

        // RSRP (44-140)
        out.cellInfo.signalStrengthLte.rsrp = cellInfo.sig_rsrp;
        if (out.cellInfo.signalStrengthLte.rsrp < 44 || out.cellInfo.signalStrengthLte.rsrp > 140)
            out.cellInfo.signalStrengthLte.rsrp = INT32_MAX;

        // RSRQ (3-20)
        out.cellInfo.signalStrengthLte.rsrq = cellInfo.sig_rsrq;
        if (out.cellInfo.signalStrengthLte.rsrq < 3 || out.cellInfo.signalStrengthLte.rsrq > 20)
            out.cellInfo.signalStrengthLte.rsrq = INT32_MAX;

        // RSSNR (-200 - 300)
        out.cellInfo.signalStrengthLte.rssnr = cellInfo.sig_rssnr;
        if (out.cellInfo.signalStrengthLte.rssnr < -200 || out.cellInfo.signalStrengthLte.rssnr > 300)
            out.cellInfo.signalStrengthLte.rssnr = INT32_MAX;

        // CQI (0-15)
        out.cellInfo.signalStrengthLte.cqi = cellInfo.sig_cqi;
        if (out.cellInfo.signalStrengthLte.cqi < 0 || out.cellInfo.signalStrengthLte.cqi > 15)
            out.cellInfo.signalStrengthLte.cqi = INT32_MAX;

        // TAVD (0-0x7FFFFFFE)
        out.cellInfo.signalStrengthLte.timingAdvance = cellInfo.ta;
        if (out.cellInfo.signalStrengthLte.timingAdvance < 0 || out.cellInfo.signalStrengthLte.timingAdvance > 0x7FFFFFFE)
            out.cellInfo.signalStrengthLte.timingAdvance = INT32_MAX;
    }

    void FillLteCellInfo(RIL_CellInfoLte_V1_4& out, cell_info_lte_v14& cellInfo) {
        out.cellInfo.cellIdentityLte.mcc = FetchMcc(cellInfo.plmn);
        out.cellInfo.cellIdentityLte.mnc = FetchMnc(cellInfo.plmn);

        // 28-bit CID
        out.cellInfo.cellIdentityLte.ci = cellInfo.cell_id;
        if (out.cellInfo.cellIdentityLte.ci < 0 || out.cellInfo.cellIdentityLte.ci > 0xFFFFFFF)
            out.cellInfo.cellIdentityLte.ci = INT32_MAX;

        // Physical CID
        out.cellInfo.cellIdentityLte.pci = cellInfo.phy_cell_id;
        if (out.cellInfo.cellIdentityLte.pci < 0 || out.cellInfo.cellIdentityLte.pci > 503)
            out.cellInfo.cellIdentityLte.pci = INT32_MAX;

        // 16-bit TAC
        out.cellInfo.cellIdentityLte.tac = cellInfo.tac;
        if (out.cellInfo.cellIdentityLte.tac < 0 || out.cellInfo.cellIdentityLte.tac > 0xFFFF)
            out.cellInfo.cellIdentityLte.tac = INT32_MAX;

        // 18-bit earfcn
        out.cellInfo.cellIdentityLte.earfcn = cellInfo.earfcn;
        if (out.cellInfo.cellIdentityLte.earfcn < 0 || out.cellInfo.cellIdentityLte.earfcn > 0x3FFFF)
            out.cellInfo.cellIdentityLte.earfcn = INT32_MAX;

        out.cellInfo.cellIdentityLte.bandwidth = cellInfo.bandwidth;
        if (out.cellInfo.cellIdentityLte.bandwidth < 0)
            out.cellInfo.cellIdentityLte.bandwidth = INT32_MAX;

        // Operator name
        FillOperatorName(out.cellInfo.cellIdentityLte.operatorNames, out.cellInfo.cellIdentityLte.mcc, out.cellInfo.cellIdentityLte.mnc);

        // E-UTRA-NR Dual Connectivity available
        out.cellConfig.isEndcAvailable = (cellInfo.endc_available == 1)? true: false;

        // Signal Strength (0-31,99)
        out.cellInfo.signalStrengthLte.signalStrength = cellInfo.sig_str;
        if (out.cellInfo.signalStrengthLte.signalStrength < 0 || out.cellInfo.signalStrengthLte.signalStrength > 31)
            out.cellInfo.signalStrengthLte.signalStrength = 99;

        // RSRP (44-140)
        out.cellInfo.signalStrengthLte.rsrp = cellInfo.sig_rsrp;
        if (out.cellInfo.signalStrengthLte.rsrp < 44 || out.cellInfo.signalStrengthLte.rsrp > 140)
            out.cellInfo.signalStrengthLte.rsrp = INT32_MAX;

        // RSRQ (3-20)
        out.cellInfo.signalStrengthLte.rsrq = cellInfo.sig_rsrq;
        if (out.cellInfo.signalStrengthLte.rsrq < 3 || out.cellInfo.signalStrengthLte.rsrq > 20)
            out.cellInfo.signalStrengthLte.rsrq = INT32_MAX;

        // RSSNR (-200 - 300)
        out.cellInfo.signalStrengthLte.rssnr = cellInfo.sig_rssnr;
        if (out.cellInfo.signalStrengthLte.rssnr < -200 || out.cellInfo.signalStrengthLte.rssnr > 300)
            out.cellInfo.signalStrengthLte.rssnr = INT32_MAX;

        // CQI (0-15)
        out.cellInfo.signalStrengthLte.cqi = cellInfo.sig_cqi;
        if (out.cellInfo.signalStrengthLte.cqi < 0 || out.cellInfo.signalStrengthLte.cqi > 15)
            out.cellInfo.signalStrengthLte.cqi = INT32_MAX;

        // TAVD (0-0x7FFFFFFE)
        out.cellInfo.signalStrengthLte.timingAdvance = cellInfo.ta;
        if (out.cellInfo.signalStrengthLte.timingAdvance < 0 || out.cellInfo.signalStrengthLte.timingAdvance > 0x7FFFFFFE)
            out.cellInfo.signalStrengthLte.timingAdvance = INT32_MAX;
    }

    void FillWcdmaCellInfo(RIL_CellInfoWcdma_V1_2& out, cell_info_wcdma_v12& cellInfo) {
        out.cellIdentityWcdma.mcc = FetchMcc(cellInfo.plmn);
        out.cellIdentityWcdma.mnc = FetchMnc(cellInfo.plmn);

        // 16-bit LAC
        out.cellIdentityWcdma.lac = cellInfo.lac;
        if (out.cellIdentityWcdma.lac < 0 || out.cellIdentityWcdma.lac > 0xFFFF)
            out.cellIdentityWcdma.lac = INT32_MAX;

        // 28-bit UMTS CID
        out.cellIdentityWcdma.cid = cellInfo.cid;
        if (out.cellIdentityWcdma.cid < 0 || out.cellIdentityWcdma.cid > 0xFFFFFFF)
            out.cellIdentityWcdma.cid = INT32_MAX;

        // 9-bit UMTS PSC
        out.cellIdentityWcdma.psc = cellInfo.psc;
        if (out.cellIdentityWcdma.psc < 0 || out.cellIdentityWcdma.psc > 0x1FF)
            out.cellIdentityWcdma.psc = INT32_MAX;

        // 16-bit UMTS uarfcn
        out.cellIdentityWcdma.uarfcn = cellInfo.uarfcn;
        if (out.cellIdentityWcdma.uarfcn < 0 || out.cellIdentityWcdma.uarfcn > 0xFFFF)
            out.cellIdentityWcdma.uarfcn = INT32_MAX;

        // Operator name
        FillOperatorName(out.cellIdentityWcdma.operatorNames, out.cellIdentityWcdma.mcc, out.cellIdentityWcdma.mnc);

        // BER (0-7,99)
        out.signalStrengthWcdma.bitErrorRate = cellInfo.sig_ber;
        if (out.signalStrengthWcdma.bitErrorRate < 0 || out.signalStrengthWcdma.bitErrorRate > 7)
            out.signalStrengthWcdma.bitErrorRate = 99;

        // Signal Strength (0-31,99)
        out.signalStrengthWcdma.signalStrength = cellInfo.sig_str;
        if (out.signalStrengthWcdma.signalStrength < 0 || out.signalStrengthWcdma.signalStrength > 31)
            out.signalStrengthWcdma.signalStrength = cellInfo.sig_str;
    }

    void FillWcdmaCellInfo(RIL_CellInfoWcdma_V1_2& out, cell_info_wcdma_v14& cellInfo) {
        out.cellIdentityWcdma.mcc = FetchMcc(cellInfo.plmn);
        out.cellIdentityWcdma.mnc = FetchMnc(cellInfo.plmn);

        // 16-bit LAC
        out.cellIdentityWcdma.lac = cellInfo.lac;
        if (out.cellIdentityWcdma.lac < 0 || out.cellIdentityWcdma.lac > 0xFFFF)
            out.cellIdentityWcdma.lac = INT32_MAX;

        // 28-bit UMTS CID
        out.cellIdentityWcdma.cid = cellInfo.cid;
        if (out.cellIdentityWcdma.cid < 0 || out.cellIdentityWcdma.cid > 0xFFFFFFF)
            out.cellIdentityWcdma.cid = INT32_MAX;

        // 9-bit UMTS PSC
        out.cellIdentityWcdma.psc = cellInfo.psc;
        if (out.cellIdentityWcdma.psc < 0 || out.cellIdentityWcdma.psc > 0x1FF)
            out.cellIdentityWcdma.psc = INT32_MAX;

        // 16-bit UMTS uarfcn
        out.cellIdentityWcdma.uarfcn = cellInfo.uarfcn;
        if (out.cellIdentityWcdma.uarfcn < 0 || out.cellIdentityWcdma.uarfcn > 0xFFFF)
            out.cellIdentityWcdma.uarfcn = INT32_MAX;

        // Operator name
        FillOperatorName(out.cellIdentityWcdma.operatorNames, out.cellIdentityWcdma.mcc, out.cellIdentityWcdma.mnc);

        // BER (0-7,99)
        out.signalStrengthWcdma.bitErrorRate = cellInfo.sig_ber;
        if (out.signalStrengthWcdma.bitErrorRate < 0 || out.signalStrengthWcdma.bitErrorRate > 7)
            out.signalStrengthWcdma.bitErrorRate = 99;

        // Signal Strength (0-31,99)
        out.signalStrengthWcdma.signalStrength = cellInfo.sig_str;
        if (out.signalStrengthWcdma.signalStrength < 0 || out.signalStrengthWcdma.signalStrength > 31)
            out.signalStrengthWcdma.signalStrength = 99;

        // Received Signal Code Power (0-96,255)
        out.signalStrengthWcdma.rscp = (cellInfo.rscp < 0)? 0:cellInfo.rscp;
        if (out.signalStrengthWcdma.rscp > 96)
            out.signalStrengthWcdma.rscp = 255;

        // Ec/No Value (0-49,255)
        out.signalStrengthWcdma.ecno = (cellInfo.ecno < 0)? 0:cellInfo.ecno;
        if (out.signalStrengthWcdma.ecno > 49)
            out.signalStrengthWcdma.ecno = 255;
    }

    void FillTdscdmaCellInfo(RIL_CellInfoTdscdma_V1_2& out, cell_info_tdscdma& cellInfo) {
        out.cellIdentityTdscdma.mcc = FetchMcc(cellInfo.plmn);
        out.cellIdentityTdscdma.mnc = FetchMnc(cellInfo.plmn);

        // 16-bit LAC
        out.cellIdentityTdscdma.lac = cellInfo.lac;
        if (out.cellIdentityTdscdma.lac < 0 || out.cellIdentityTdscdma.lac > 0xFFFF)
            out.cellIdentityTdscdma.lac = INT32_MAX;

        // 28-bit UMTS CID
        out.cellIdentityTdscdma.cid = cellInfo.cid;
        if (out.cellIdentityTdscdma.cid < 0 || out.cellIdentityTdscdma.cid > 0xFFFFFFF)
            out.cellIdentityTdscdma.cid = INT32_MAX;

        // 8-bit CPID
        out.cellIdentityTdscdma.cpid = cellInfo.cpid;
        if (out.cellIdentityTdscdma.cpid < 0 || out.cellIdentityTdscdma.cpid > 0xFF)
            out.cellIdentityTdscdma.cpid = INT32_MAX;

        // Operator name
        FillOperatorName(out.cellIdentityTdscdma.operatorNames, out.cellIdentityTdscdma.mcc, out.cellIdentityTdscdma.mnc);

        // RSCP (25-120)
        out.signalStrengthTdscdma.rscp = cellInfo.rscp;
        if (out.signalStrengthTdscdma.rscp < 25 || out.signalStrengthTdscdma.rscp > 250)
            out.signalStrengthTdscdma.rscp = INT32_MAX;
    }

    void FillTdscdmaCellInfo(RIL_CellInfoTdscdma_V1_2& out, cell_info_tdscdma_v14& cellInfo) {
        out.cellIdentityTdscdma.mcc = FetchMcc(cellInfo.plmn);
        out.cellIdentityTdscdma.mnc = FetchMnc(cellInfo.plmn);

        // 16-bit LAC
        out.cellIdentityTdscdma.lac = cellInfo.lac;
        if (out.cellIdentityTdscdma.lac < 0 || out.cellIdentityTdscdma.lac > 0xFFFF)
            out.cellIdentityTdscdma.lac = INT32_MAX;

        // 28-bit UMTS CID
        out.cellIdentityTdscdma.cid = cellInfo.cid;
        if (out.cellIdentityTdscdma.cid < 0 || out.cellIdentityTdscdma.cid > 0xFFFFFFF)
            out.cellIdentityTdscdma.cid = INT32_MAX;

        // 8-bit CPID
        out.cellIdentityTdscdma.cpid = cellInfo.cpid;
        if (out.cellIdentityTdscdma.cpid < 0 || out.cellIdentityTdscdma.cpid > 0xFF)
            out.cellIdentityTdscdma.cpid = INT32_MAX;

        // Operator name
        FillOperatorName(out.cellIdentityTdscdma.operatorNames, out.cellIdentityTdscdma.mcc, out.cellIdentityTdscdma.mnc);

        // Received Signal Strength Indication (RSSI) measured from TDSCDMA (0-31,99)
        out.signalStrengthTdscdma.signalStrength = (cellInfo.sig_str < 0)? 0: cellInfo.sig_str;
        if (out.signalStrengthTdscdma.signalStrength > 31)
            out.signalStrengthTdscdma.signalStrength = 99;

        // Bit error rate (0-7, 99)
        out.signalStrengthTdscdma.bitErrorRate = (cellInfo.ber < 0)? 0:cellInfo.ber;
        if (out.signalStrengthTdscdma.bitErrorRate > 7)
            out.signalStrengthTdscdma.bitErrorRate = 99;

        // RSCP (25-120)
        out.signalStrengthTdscdma.rscp = cellInfo.rscp;
        if (out.signalStrengthTdscdma.rscp < 25 || out.signalStrengthTdscdma.rscp > 250)
            out.signalStrengthTdscdma.rscp = INT32_MAX;
    }

    void FillNrCellInfo(RIL_CellInfoNr_V1_4& out, cell_info_nr& cellInfo) {
        out.cellidentityNr.mcc = FetchMcc(cellInfo.plmn);
        out.cellidentityNr.mnc = FetchMnc(cellInfo.plmn);

        // Cell Identity (36 bits)
        out.cellidentityNr.nci = cellInfo.cell_id;
        if (out.cellidentityNr.nci > 68719476735)
            out.cellidentityNr.nci = LONG_MAX;

        // Physical cell id (0-1007)
        out.cellidentityNr.pci = cellInfo.phy_cell_id;
        if (out.cellidentityNr.pci > 1007)
            out.cellidentityNr.pci = INT32_MAX;

        // 16-bit tracking area code
        out.cellidentityNr.tac = cellInfo.tac;
        if (out.cellidentityNr.tac < 0 || out.cellidentityNr.tac > 0xFFFF)
            out.cellidentityNr.tac = INT32_MAX;

        // NR Absolute Radio Frequency Channel Number (0-3279165)
        out.cellidentityNr.nrarfcn = cellInfo.arfcn;
        if (out.cellidentityNr.nrarfcn < 0 || out.cellidentityNr.nrarfcn > 3279165)
            out.cellidentityNr.nrarfcn = INT32_MAX;

        // Operator name
        FillOperatorName(out.cellidentityNr.operatorNames, out.cellidentityNr.mcc, out.cellidentityNr.mnc);

        // SS reference signal received power (44-140)
        out.signalStrengthNr.ssRsrp = cellInfo.ss_rsrp;
        if (out.signalStrengthNr.ssRsrp < 44 || out.signalStrengthNr.ssRsrp > 140)
            out.signalStrengthNr.ssRsrp = INT32_MAX;

        // SS reference signal received quality (3-20)
        out.signalStrengthNr.ssRsrq = cellInfo.ss_rsrq;
        if (out.signalStrengthNr.ssRsrq < 3 || out.signalStrengthNr.ssRsrq > 20)
            out.signalStrengthNr.ssRsrq = INT32_MAX;

        // SS signal-to-noise and interference ratio (-23~40)
        out.signalStrengthNr.ssSinr = cellInfo.ss_sinr;
        if (out.signalStrengthNr.ssSinr < -23 || out.signalStrengthNr.ssSinr > 40)
            out.signalStrengthNr.ssSinr = INT32_MAX;

        // CSI reference signal received power (44-140)
        out.signalStrengthNr.csiRsrp = cellInfo.csi_rsrp;
        if (out.signalStrengthNr.csiRsrp < 44 || out.signalStrengthNr.csiRsrp > 140)
            out.signalStrengthNr.csiRsrp = INT32_MAX;

        // CSI reference signal received quality (3-20)
        out.signalStrengthNr.csiRsrq = cellInfo.csi_rsrq;
        if (out.signalStrengthNr.csiRsrq < 3 || out.signalStrengthNr.csiRsrq > 20)
            out.signalStrengthNr.csiRsrq = INT32_MAX;

        // CSI signal-to-noise and interference ratio (-23~40)
        out.signalStrengthNr.csiSinr = cellInfo.csi_sinr;
        if (out.signalStrengthNr.csiSinr < -23 || out.signalStrengthNr.csiSinr > 40)
            out.signalStrengthNr.csiSinr = INT32_MAX;
    }
};

/**
 * ProtocolNetCellInfoListAdapter
 */
ProtocolNetCellInfoListAdapter::ProtocolNetCellInfoListAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData)
{
}

list<RIL_CellInfo_V1_4>& ProtocolNetCellInfoListAdapter::GetCellInfoList()
{
    mCellInfoList.resize(0);
    if (m_pModemData != NULL) {
        sit_net_get_cell_info_list_rsp *response = (sit_net_get_cell_info_list_rsp *)m_pModemData->GetRawData();
        if (response != NULL && response->hdr.id == SIT_GET_CELL_INFO_LIST) {
            int cellInfoSize = response->cell_info_num;
            void *data = response->cell_info_list;
            int dataLen = GetParameterLength() - sizeof(int);

            ProtocolCellInfoList cellInfoList(data, dataLen, cellInfoSize);
            mCellInfoList = cellInfoList.GetCellInfoList();
        }
    }
    return mCellInfoList;
}

/**
 * ProtocolNetCellInfoListIndAdapter
 */
ProtocolNetCellInfoListIndAdapter::ProtocolNetCellInfoListIndAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
}

list<RIL_CellInfo_V1_4>& ProtocolNetCellInfoListIndAdapter::GetCellInfoList()
{
    mCellInfoList.resize(0);
    if (m_pModemData != NULL) {
        sit_net_cell_info_list_ind *response = (sit_net_cell_info_list_ind *)m_pModemData->GetRawData();
        if (response != NULL && response->hdr.id == SIT_IND_CELL_INFO_LIST) {
            int cellInfoSize = response->cell_info_num;
            void *data = response->cell_info_list;
            int dataLen = GetParameterLength() - sizeof(int);

            ProtocolCellInfoList cellInfoList(data, dataLen, cellInfoSize);
            mCellInfoList = cellInfoList.GetCellInfoList();
        }
    }
    return mCellInfoList;
}

/**
 * ProtocolNetScanResultAdapter
 */
ProtocolNetScanResultAdapter::ProtocolNetScanResultAdapter(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
}

int ProtocolNetScanResultAdapter::GetScanStatus() const
{
    int ret = COMPLETE;
    if (m_pModemData != NULL) {
        sit_net_scanning_network_ind *data = (sit_net_scanning_network_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_SCANNING_NETWORKS) {
            if (data->scan_status == SIT_NET_SCAN_STATUS_PARTIAL)
                ret = PARTIAL;
        }
    }
    return ret;
}

int ProtocolNetScanResultAdapter::GetScanResult() const
{
    // TODO need to get result from modem
    return RIL_E_SUCCESS;
}

list<RIL_CellInfo_V1_4>& ProtocolNetScanResultAdapter::GetCellInfoList()
{
    mCellInfoList.resize(0);
    if (m_pModemData != NULL) {
        sit_net_scanning_network_ind *response = (sit_net_scanning_network_ind *)m_pModemData->GetRawData();
        if (response != NULL && response->hdr.id == SIT_IND_SCANNING_NETWORKS) {
            int cellInfoSize = response->cell_info_num;
            void *data = response->cell_info_list;
            int dataLen = GetParameterLength() - sizeof(int) + sizeof(char);

            ProtocolCellInfoList cellInfoList(data, dataLen, cellInfoSize);
            mCellInfoList = cellInfoList.GetCellInfoList();
        }
    }
    return mCellInfoList;
}

/**
 * ProtocolNetSimFileInfoAdapter
 */
ProtocolNetSimFileInfoAdapter::ProtocolNetSimFileInfoAdapter(const ModemData *pModemData) : ProtocolIndAdapter (pModemData) {
    mSimFileId = 0;
    mRecordLen = 0;
    mNumRecords = 0;
    mppData = NULL;

    if (m_pModemData != NULL) {
        sit_sim_file_data_info_ind *data = (sit_sim_file_data_info_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_SIM_DATA_INFO) {
            mSimFileId = data->sim_file_id;
            mRecordLen = data->record_len;
            mNumRecords = data->num_of_records;
            mppData = new BYTE *[mNumRecords];
            BYTE *pTmp = data->data_info;
            for (int i = 0; i < mNumRecords; i++) {
                BYTE *pInputRecord = pTmp + (i * mRecordLen);
                BYTE *record = new BYTE[mRecordLen];
                memcpy(record, pInputRecord, mRecordLen);
                *(mppData + i) = record;
            } // end for i ~
        }
    }
}

ProtocolNetSimFileInfoAdapter::~ProtocolNetSimFileInfoAdapter()
{
    // delete memory
    if (mppData != NULL) {
        BYTE **ppTmp = mppData;
        BYTE *pDel = NULL;
        int i = mNumRecords;
        while(i > 0) {
            pDel = *ppTmp++;
            if(pDel != NULL) delete [] pDel;
            i--;
        }
        delete [] mppData;
    }
}

int ProtocolNetSimFileInfoAdapter::GetSimFileId() const
{
    return mSimFileId;
}

int ProtocolNetSimFileInfoAdapter::GetRecordLen() const
{
    return mRecordLen;
}

int ProtocolNetSimFileInfoAdapter::GetNumOfRecords() const
{
    return mNumRecords;
}

BYTE **ProtocolNetSimFileInfoAdapter::GetSimFileData() const
{
    return mppData;
}

/**
 * ProtocolNetPhysicalChannelConfigs
 */
ProtocolNetPhysicalChannelConfigs::ProtocolNetPhysicalChannelConfigs(const ModemData *pModemData)
    : ProtocolIndAdapter(pModemData)
{
    memset(mConfigs, 0, sizeof(mConfigs));
    mSize = 0;
    Init();
}

ProtocolNetPhysicalChannelConfigs::~ProtocolNetPhysicalChannelConfigs()
{
    for (int i = 0; i < mSize; i++) {
        if (mConfigs[i].contextIds != NULL) {
            delete[] mConfigs[i].contextIds;
        }
    } // end for i ~
}

void ProtocolNetPhysicalChannelConfigs::Init()
{
    // fill data with considering legacy
    if (m_pModemData != NULL && m_pModemData->GetRawData() != NULL) {
        int messageId = m_pModemData->GetMessageId();
        if (messageId == SIT_IND_SCG_BEARER_ALLOCATION) {
            ProtocolNetworkSgcBearerAllocIndAdapter nrScgAdapter(m_pModemData);
            mSize = 1;
            RIL_PhysicalChannelConfig_V1_4 &config = mConfigs[0];
            config.status = nrScgAdapter.GetConnectionStatus();
            config.rat = nrScgAdapter.GetRat();
            // others, default
        }
        else if (messageId == SIT_IND_PHYSICAL_CHANNEL_CONFIG) {
            sit_net_physical_channel_config_ind *data =
                    (sit_net_physical_channel_config_ind *)m_pModemData->GetRawData();
            mSize = (data->config_len > 0) ? data->config_len : 0;
            if (mSize > MAX_PHYSICAL_CHANNEL_CONFIGS) {
                mSize = MAX_PHYSICAL_CHANNEL_CONFIGS;
            }

            for (int i = 0; i < mSize; i++) {
                RIL_PhysicalChannelConfig_V1_4 &config = mConfigs[i];
                sit_physical_channel_config &pcc = data->configs[i];
                config.status = pcc.cell_status & 0xFF;
                config.cellBandwidthDownlink = pcc.cell_bandwidth_downlink;
                config.rat = ::ConvertRadioTechValue(pcc.rat & 0xFF);
                config.rfInfoType = RF_INFO_TYPE_CHANNEL_NUMBER;
                if (config.rat == RADIO_TECH_NR) {
                    config.rfInfoType = RF_INFO_TYPE_RANGE;
                    config.rfInfo.range = pcc.frequency_range & 0xFF;
                }
                else {
                    config.rfInfo.channelNumber = pcc.channel;
                }
                config.len_contextIds = pcc.context_len & 0xFF;
                if (config.len_contextIds > 0) {
                    config.contextIds = new int[config.len_contextIds];
                    if (config.contextIds != NULL) {
                        for (int i = 0; i < config.len_contextIds; i++) {
                            config.contextIds[i] = pcc.context_id[i];
                        } // end for i ~
                    }
                    else {
                        config.len_contextIds = 0;
                    }
                }
                config.physicalCellId = pcc.physical_cellid;
            } // end for i ~
        }
    }
}

/**
 * ProtocolNetGetManualRatModeAdapter
 */

ProtocolNetGetManualRatModeAdapter::ProtocolNetGetManualRatModeAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData), m_manual_rat_mode_set(0), m_rat(0)
{
    if (m_pModemData != NULL) {
        sit_net_get_manual_rat_mode_rsp *data = (sit_net_get_manual_rat_mode_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_MANUAL_RAT_MODE) {
            m_manual_rat_mode_set = (int)data->manual_rat_mode_set;
            m_rat = data->rat;
        }
    }
}

void ProtocolNetGetManualRatModeAdapter::GetManualRatMode(void *data)
{
    *((int *)data+0) = m_manual_rat_mode_set;
    *((int *)data+1) = m_rat;
}

/**
 * ProtocolNetSetManualRatModeAdapter
 */
int ProtocolNetSetManualRatModeAdapter::GetCause() const
{
    if (m_pModemData != NULL) {
        sit_net_set_manual_rat_mode_rsp *data = (sit_net_set_manual_rat_mode_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_SET_MANUAL_RAT_MODE) {
            RilLogV("GetCause=%d", data->cause);
            return (int)data->cause;
        }
    }
    return 0;
}

/**
 * ProtocolNetGetFreqLockAdapter
 */

ProtocolNetGetFreqLockAdapter::ProtocolNetGetFreqLockAdapter(const ModemData *pModemData)
    : ProtocolRespAdapter(pModemData), m_freq_mode_set(0), m_rat(0), m_lte_pci(0), m_lte_earfcn(0), m_gsm_arfcn(0), m_wcdma_psc(0), m_wcdma_uarfcn(0)
{
    if (m_pModemData != NULL) {
        sit_net_get_freq_lock_rsp *data = (sit_net_get_freq_lock_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_FREQUENCY_LOCK) {
            m_freq_mode_set = (int)data->freq_mode_set;
            m_rat = (int)data->rat;
            m_lte_pci = data->lte_pci;
            m_lte_earfcn = data->lte_earfcn;
            m_gsm_arfcn = data->gsm_arfcn;
            m_wcdma_psc = data->wcdma_psc;
            m_wcdma_uarfcn = data->wcdma_uarfcn;
        }
    }
}

void ProtocolNetGetFreqLockAdapter::GetFrequencyLock(void *data)
{
    *((int *)data+0) = m_freq_mode_set;
    *((int *)data+1) = m_rat;
    *((int *)data+2) = m_lte_pci;
    *((int *)data+3) = m_lte_earfcn;
    *((int *)data+4) = m_gsm_arfcn;
    *((int *)data+5) = m_wcdma_psc;
    *((int *)data+6) = m_wcdma_uarfcn;
}

/**
 * ProtocolNetSetFreqLockAdapter
 */
int ProtocolNetSetFreqLockAdapter::GetResult() const
{
    if (m_pModemData != NULL) {
        sit_net_set_freq_lock_rsp *data = (sit_net_set_freq_lock_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_SET_FREQUENCY_LOCK) {
            RilLogV("GetResult=%d", data->result);
            return (int)data->result;
        }
    }
    return 0;
}

/**
 * ProtocolNetGetEndcModeAdapter
 */
int ProtocolNetGetEndcModeAdapter::GetEndcMode() const
{
    if (m_pModemData != NULL) {
        sit_net_get_endc_mode_rsp *data = (sit_net_get_endc_mode_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_ENDC_MODE) {
            RilLogV("Get endc mode=%d", data->mode);
            return (int)data->mode;
        }
    }
    return 0;
}

/**
 * ProtocolNetworkFrequencyInfoIndAdapter
 */
int ProtocolNetworkFrequencyInfoIndAdapter::GetRat() const
{
    int ret = 0;    // RAT_NONE
    if (m_pModemData != NULL) {
        sit_net_frequency_info_ind *data = (sit_net_frequency_info_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_FREQUENCY_INFO) {
            ret = (int)data->rat;
        }
    }
    return ret;
}

int ProtocolNetworkFrequencyInfoIndAdapter::GetBand() const
{
    int ret = 0;    // NONE
    if (m_pModemData != NULL) {
        sit_net_frequency_info_ind *data = (sit_net_frequency_info_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_FREQUENCY_INFO) {
            ret = (int)data->band;
        }
    }
    return ret;
}

int ProtocolNetworkFrequencyInfoIndAdapter::GetFrequency() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_net_frequency_info_ind *data = (sit_net_frequency_info_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_FREQUENCY_INFO) {
            ret = (int)data->frequency;
        }
    }
    return ret;
}

/*
 * ProtocolNetAcBarringInfo
 */
static void InitAcBarringInfo(AC_BARRING_INFO &info)
{
    info.for_emc = 0;
    info.for_mo_sig_factor = 100;
    info.for_mo_sig_time = -1;
    memset(info.for_mo_sig_ac_list, 0x00, sizeof(info.for_mo_sig_ac_list));

    info.for_mo_data_factor = 100;
    info.for_mo_data_time = -1;
    memset(info.for_mo_data_ac_list, 0x00, sizeof(info.for_mo_data_ac_list));

    info.for_mmtel_voice_factor = 100;
    info.for_mmtel_voice_time = -1;
    memset(info.for_mmtel_voice_ac_list, 0x00, sizeof(info.for_mmtel_voice_ac_list));

    info.for_mmtel_video_factor = 100;
    info.for_mmtel_video_time = -1;
    memset(info.for_mmtel_video_ac_list, 0x00, sizeof(info.for_mmtel_video_ac_list));
}

ProtocolNetAcBarringInfo::ProtocolNetAcBarringInfo(const ModemData *pModemData) : ProtocolIndAdapter(pModemData)
{
    InitAcBarringInfo(mAcBarringInfo);

    if(m_pModemData != NULL) {
        sit_net_ac_barring_info_ind *data = (sit_net_ac_barring_info_ind *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_IND_AC_BARRING_INFO) {
            mAcBarringInfo.for_emc = data->for_emc;

            mAcBarringInfo.for_mo_sig_factor = data->for_mo_sig_factor;
            mAcBarringInfo.for_mo_sig_time = data->for_mo_sig_time;
            memcpy(mAcBarringInfo.for_mo_sig_ac_list, data->for_mo_sig_ac_list, sizeof(data->for_mo_sig_ac_list));

            mAcBarringInfo.for_mo_data_factor = data->for_mo_data_factor;
            mAcBarringInfo.for_mo_data_time = data->for_mo_data_time;
            memcpy(mAcBarringInfo.for_mo_data_ac_list, data->for_mo_data_ac_list, sizeof(data->for_mo_data_ac_list));

            mAcBarringInfo.for_mmtel_voice_factor = data->for_mmtel_voice_factor;
            mAcBarringInfo.for_mmtel_voice_time = data->for_mmtel_voice_time;
            memcpy(mAcBarringInfo.for_mmtel_voice_ac_list, data->for_mmtel_voice_ac_list, sizeof(data->for_mmtel_voice_ac_list));

            mAcBarringInfo.for_mmtel_video_factor = data->for_mmtel_video_factor;
            mAcBarringInfo.for_mmtel_video_time = data->for_mmtel_video_time;
            memcpy(mAcBarringInfo.for_mmtel_video_ac_list, data->for_mmtel_video_ac_list, sizeof(data->for_mmtel_video_ac_list));
        }
    }
}

void ProtocolNetAcBarringInfo::GetAcBarringInfo(void *data, unsigned int size)
{
    *((char *)data+0) = mAcBarringInfo.for_emc;
    *((char *)data+1) = mAcBarringInfo.for_mo_sig_factor;
    *((char *)data+2) = mAcBarringInfo.for_mo_data_factor;
    *((char *)data+3) = mAcBarringInfo.for_mmtel_voice_factor;
    *((char *)data+4) = mAcBarringInfo.for_mmtel_video_factor;
}

/*
 * ProtocolNetGetFrequencyInfoAdapter
 */
int ProtocolNetGetFrequencyInfoAdapter::GetRat() const
{
    int ret = 0;    // RAT_NONE
    if (m_pModemData != NULL) {
        sit_net_get_frequency_info_rsp *data = (sit_net_get_frequency_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_FREQUENCY_INFO) {
            ret = (int)data->rat;
        }
    }
    return ret;
}

int ProtocolNetGetFrequencyInfoAdapter::GetBand() const
{
    int ret = 0;    // NONE
    if (m_pModemData != NULL) {
        sit_net_get_frequency_info_rsp *data = (sit_net_get_frequency_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_FREQUENCY_INFO) {
            ret = (int)data->band;
        }
    }
    return ret;
}

int ProtocolNetGetFrequencyInfoAdapter::GetFrequency() const
{
    int ret = 0;
    if (m_pModemData != NULL) {
        sit_net_get_frequency_info_rsp *data = (sit_net_get_frequency_info_rsp *)m_pModemData->GetRawData();
        if (data != NULL && data->hdr.id == SIT_GET_FREQUENCY_INFO) {
            ret = (int)data->frequency;
        }
    }
    return ret;
}
