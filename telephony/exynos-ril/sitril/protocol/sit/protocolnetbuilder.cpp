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
 * protocolnetbuilder.cpp
 *
 *  Created on: 2014. 6. 27.
 *      Author: sungwoo48.choi
 */

#include "protocolnetbuilder.h"
#include "uplmnselector.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

ModemData *ProtocolNetworkBuilder::BuildNetworkRegistrationState(int domain)
{
    if (domain == NETWORK_DOMAIN_CS) {
        sit_net_get_cs_reg_state_req req;
        int length = sizeof(req);
        InitRequestHeader(&req.hdr, SIT_GET_CS_REG_STATE, length);
        return new ModemData((char *)&req, length);
    }
    else if (domain == NETWORK_DOMAIN_PS) {
        sit_net_get_ps_reg_state_req req;
        int length = sizeof(req);
        InitRequestHeader(&req.hdr, SIT_GET_PS_REG_STATE, length);
        return new ModemData((char *)&req, length);
    }
    return NULL;
}

ModemData *ProtocolNetworkBuilder::BuildOperator()
{
    sit_net_get_operator_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_OPERATOR, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildRadioPower(int powerState)
{
    sit_pwr_set_radio_power_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_RADIO_POWER, length);
    req.radio_state = (powerState == 0 ? SIT_PWR_RADIO_STATE_STOP_NETWORK : SIT_PWR_RADIO_STATE_START_NETWORK);

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildRestartModem()
{
    sit_pwr_set_radio_power_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_RADIO_POWER, length);
    req.radio_state = 4;    // fixed value, 0x04 = Phone reset

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildShutdown()
{
    sit_pwr_set_radio_power_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_RADIO_POWER, length);
    req.radio_state = SIT_PWR_RADIO_STATE_POWER_OFF;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildGetRadioState()
{
    sit_pwr_get_radio_power_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_RADIO_POWER, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildQueryNetworkSelectionMode()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_NTW_MODE, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetNetworkSelectionAuto()
{
    sit_net_set_network_mode_auto_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_NTW_MODE_AUTO, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetNetworkSelectionManual(int rat, const char *plmn)
{
    if (plmn == NULL || *plmn == 0) {
        return NULL;
    }

    int len = strlen(plmn);
    if (!(len == 5 || len == 6)) {
        return NULL;
    }

    sit_net_set_metwork_mode_manual_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_NTW_MODE_MANUAL, length);

    if ((int)RADIO_TECH_UNKNOWN <= rat && rat <= (int)RADIO_TECH_GSM) {
        req.rat = rat;
    }
    else if (rat == (int)RADIO_TECH_TD_SCDMA) {
        req.rat = (int)SIT_RAT_TYPE_TD_SCDMA;
    }
    else if (rat == (int)RADIO_TECH_NR) {
        req.rat = (int)SIT_RAT_TYPE_5G;
    }
    else {
        req.rat = (int)SIT_RAT_TYPE_UNKNOWN;
    }

    req.plmn[5] = '#';
    memcpy(req.plmn, plmn, len);

    return new ModemData((char *)&req, length);
}

int ProtocolNetworkBuilder::translateNetworktype(int netType)
{
    int net_type = SIT_NET_PREF_NET_TYPE_MAX;
    switch(netType) {
    case PREF_NET_TYPE_GSM_WCDMA:   //=0; /* GSM/WCDMA (WCDMA preferred) */
        net_type = SIT_NET_PREF_NET_TYPE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_GSM_ONLY:    //= 1; /* GSM only */
        net_type = SIT_NET_PREF_NET_TYPE_GSM_ONLY;
        break;
    case PREF_NET_TYPE_WCDMA:   //= 2; /* WCDMA only */
        net_type = SIT_NET_PREF_NET_TYPE_WCDMA;
        break;
    case PREF_NET_TYPE_GSM_WCDMA_AUTO:  //= 3; /* GSM/WCDMA (auto mode, according to PRL)
        net_type = SIT_NET_PREF_NET_TYPE_GSM_WCDMA_AUTO;
        break;
    case PREF_NET_TYPE_CDMA_EVDO_AUTO:  //= 4; /* CDMA and EvDo (auto mode, according to PRL)
        net_type = SIT_NET_PREF_NET_TYPE_CDMA_EVDO_AUTO;
        break;
    case PREF_NET_TYPE_CDMA_ONLY:   //= 5; /* CDMA only */
        net_type = SIT_NET_PREF_NET_TYPE_CDMA_ONLY;
        break;
    case PREF_NET_TYPE_EVDO_ONLY:   //= 6; /* EvDo only */
        net_type = SIT_NET_PREF_NET_TYPE_EVDO_ONLY;
        break;
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:    //= 7; /* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL)
        net_type = SIT_NET_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
        break;
    case PREF_NET_TYPE_LTE_CDMA_EVDO:   //= 8; /* LTE, CDMA and EvDo */
        net_type = SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO;
        break;
    case PREF_NET_TYPE_LTE_GSM_WCDMA:   //= 9; /* LTE, GSM/WCDMA */
        net_type = SIT_NET_PREF_NET_TYPE_LTE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA: //10; /* LTE, CDMA, EvDo, GSM/WCDMA */
        net_type = SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_LTE_ONLY:    //= 11; /* LTE Only mode. */
        net_type = SIT_NET_PREF_NET_TYPE_LTE_ONLY;
        break;
    case PREF_NET_TYPE_LTE_WCDMA:   //= 12; /* LTE/WCDMA */
        net_type = SIT_NET_PREF_NET_TYPE_LTE_WCDMA;
        break;

    // enum value is added from Android N. refer NETWORK_MODE_* in RILConstants.java
    case PREF_NET_TYPE_TD_SCDMA_ONLY:    //= 13; /* TD-SCDMA only */
        net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_ONLY;
        break;
    case PREF_NET_TYPE_TD_SCDMA_WCDMA:   //= 14; /* TD-SCDMA and WCDMA */
        net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_LTE: //= 15; /* TD-SCDMA and LTE */
        net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM: //= 16; /* TD-SCDMA and GSM */
        net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_GSM;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM_LTE: //= 17; /* TD-SCDMA,GSM and LTE */
        net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_GSM;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:   //= 18; /* TD-SCDMA, GSM/WCDMA */
        net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:   //= 19; /* TD-SCDMA, WCDMA and LTE */
        net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:   //= 20; /* TD-SCDMA, GSM/WCDMA and LTE */
        net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO: //= 21; /*TD-SCDMA,EvDo,CDMA,GSM/WCDMA*/
        net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA: //= 22; /* TD-SCDMA/LTE/GSM/WCDMA, CDMA, and EvDo */
        net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;
        break;

    // Add NR feature
    case PREF_NET_TYPE_NR_ONLY:    //= 23
        net_type = SIT_NET_PREF_NET_TYPE_NR_ONLY;
        break;
    case PREF_NET_TYPE_NR_LTE:
        net_type = SIT_NET_PREF_NET_TYPE_NR_LTE;
        break;
    case PREF_NET_TYPE_NR_LTE_CDMA_EVDO:
        net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO;
        break;
    case PREF_NET_TYPE_NR_LTE_GSM_WCDMA:
        net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM:
        net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA:
        net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
        net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:    //= 33
        net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_WCDMA:
        net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA:
        net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA:
        net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA;
        break;

    case PREF_NET_TYPE_TD_SCDMA_CDMA: //= 50; /* TD-SCDMA , CDMA and EvDo */
        net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_CDMA_NO_EVDO: //= 51; /* TD-SCDMA , CDMA */
        net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA_NO_EVDO;
        break;
    case PREF_NET_TYPE_TD_SCDMA_CDMA_EVDO_LTE: //= 52; /* TD-SCDMA , LTE, CDMA and EvDo */
        net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_CDMA_EVDO;
        break;
    case PREF_NET_TYPE_TD_SCDMA_EVDO_NO_CDMA: //= 53; /* TD-SCDMA , EVDO */
        net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_EVDO_NO_CDMA;
        break;

    default:
        net_type = SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA;
        break;
    }

    return net_type;
}

ModemData *ProtocolNetworkBuilder::BuildSetPreferredNetworkType(int netType)
{
    sit_net_set_pref_network_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_PREFERRED_NTW_TYPE, length);

    switch(netType) {
    case PREF_NET_TYPE_GSM_WCDMA:   //=0; /* GSM/WCDMA (WCDMA preferred) */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_GSM_ONLY:    //= 1; /* GSM only */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_GSM_ONLY;
        break;
    case PREF_NET_TYPE_WCDMA:   //= 2; /* WCDMA only */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_WCDMA;
        break;
    case PREF_NET_TYPE_GSM_WCDMA_AUTO:  //= 3; /* GSM/WCDMA (auto mode, according to PRL)
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_GSM_WCDMA_AUTO;
        break;
    case PREF_NET_TYPE_CDMA_EVDO_AUTO:  //= 4; /* CDMA and EvDo (auto mode, according to PRL)
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_CDMA_EVDO_AUTO;
        break;
    case PREF_NET_TYPE_CDMA_ONLY:   //= 5; /* CDMA only */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_CDMA_ONLY;
        break;
    case PREF_NET_TYPE_EVDO_ONLY:   //= 6; /* EvDo only */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_EVDO_ONLY;
        break;
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:    //= 7; /* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL)
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
        break;
    case PREF_NET_TYPE_LTE_CDMA_EVDO:   //= 8; /* LTE, CDMA and EvDo */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO;
        break;
    case PREF_NET_TYPE_LTE_GSM_WCDMA:   //= 9; /* LTE, GSM/WCDMA */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA: //10; /* LTE, CDMA, EvDo, GSM/WCDMA */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_LTE_ONLY:    //= 11; /* LTE Only mode. */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_ONLY;
        break;
    case PREF_NET_TYPE_LTE_WCDMA:   //= 12; /* LTE/WCDMA */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_WCDMA;
        break;

    // enum value is added from Android N. refer NETWORK_MODE_* in RILConstants.java
    case PREF_NET_TYPE_TD_SCDMA_ONLY:    //= 13; /* TD-SCDMA only */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_ONLY;
        break;
    case PREF_NET_TYPE_TD_SCDMA_WCDMA:   //= 14; /* TD-SCDMA and WCDMA */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_LTE: //= 15; /* TD-SCDMA and LTE */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM: //= 16; /* TD-SCDMA and GSM */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_GSM;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM_LTE: //= 17; /* TD-SCDMA,GSM and LTE */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_GSM;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:   //= 18; /* TD-SCDMA, GSM/WCDMA */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:   //= 19; /* TD-SCDMA, WCDMA and LTE */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:   //= 20; /* TD-SCDMA, GSM/WCDMA and LTE */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO: //= 21; /*TD-SCDMA,EvDo,CDMA,GSM/WCDMA*/
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA: //= 22; /* TD-SCDMA/LTE/GSM/WCDMA, CDMA, and EvDo */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;
        break;

    // Add NR feature
    case PREF_NET_TYPE_NR_ONLY:    //= 23
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_NR_ONLY;
        break;
    case PREF_NET_TYPE_NR_LTE:
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_NR_LTE;
        break;
    case PREF_NET_TYPE_NR_LTE_CDMA_EVDO:
        // use SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA instead of
        // SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO (not in use)
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_GSM_WCDMA:
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM:
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA:
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:    //= 33
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_WCDMA:
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA:
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA:
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA;
        break;

    case PREF_NET_TYPE_TD_SCDMA_CDMA: //= 50; /* TD-SCDMA , CDMA and EvDo */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_CDMA_NO_EVDO: //= 51; /* TD-SCDMA , CDMA */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_CDMA_NO_EVDO;
        break;
    case PREF_NET_TYPE_TD_SCDMA_CDMA_EVDO_LTE: //= 52; /* TD-SCDMA , LTE, CDMA and EvDo */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_TDSCDMA_CDMA_EVDO;
        break;
    case PREF_NET_TYPE_TD_SCDMA_EVDO_NO_CDMA: //= 53; /* TD-SCDMA , EVDO */
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_TDSCDMA_EVDO_NO_CDMA;
        break;

    default:
        req.pref_net_type = SIT_NET_PREF_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA;
        break;
    }
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildGetPreferredNetworkType()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_PREFERRED_NTW_TYPE, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildQueryAvailableNetwork()
{
    sit_net_get_available_networks_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_AVAILABLE_NETWORKS, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildQueryAvailableNetwork(int ran)
{
    int rat = 0;    // 0x00: all 0x01: GERAN 0x02: UTRAN 0x03: EUTRAN
    switch (ran) {
    case ACCESS_NETWORK_GERAN:
        rat = 1;
        break;
    case ACCESS_NETWORK_UTRAN:
        rat = 2;
        break;
    case ACCESS_NETWORK_EUTRAN:
        rat = 3;
        break;
    default:
        rat = 0;
        break;
    } // end switch ~

    sit_net_get_available_networks_wit_rat_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_AVAILABLE_NETWORKS, length);
    req.rat = rat;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildCancelQueryAvailableNetwork()
{
    sit_net_cancel_get_available_networks_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_CANCEL_GET_AVAILABLE_NETWORKS, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetBandMode(int bandMode)
{
    sit_net_set_band_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_BAND_MODE, length);
    req.band = bandMode;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildQueryAvailableBandMode()
{
    sit_net_get_band_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_BAND_MODE , length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildGetCellInfoList()
{
    sit_net_get_cell_info_list_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_CELL_INFO_LIST , length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetCellInfoListReportRate(int rate)
{
    sit_net_cell_info_list_report_rate_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_CELL_INFO_LIST_REPORT_RATE , length);

    // rate 0 means report SIT_SET_CELL_INFO_LIST_REPORT_RATE when information is changed.
    // rate 0x7FFFFFFF means never report SIT_IND_CELL_INFO_LIST
    if (rate < 0) {
        rate = 0x7FFFFFFF;
    }
    req.report_rate = rate;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildAllowData(int state)
{
    sit_net_set_ps_service_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_PS_SERVICE, length);
    req.state = (BYTE) (state == ALLOW_DATA_CALL ? 1 : 0);
    return new ModemData((char *) &req, length);
}

ModemData *ProtocolNetworkBuilder::BuildGetPsService()
{
    sit_net_get_ps_service_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_PS_SERVICE, length);
    return new ModemData((char *) &req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetUplmn(int mode, int index, const char *plmn, int act)
{
    sit_net_set_uplmn_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_UPLMN, length);

    // mode
    if (UplmnSelector::IsValidUplmnMode(mode)) {
        req.mode = (BYTE)mode;
    }
    else {
        return NULL;
    }

    if (UplmnSelector::IsValidIndex(index)) {
        req.index = (BYTE)index;
    }
    else {
        return NULL;
    }

    if (mode == UPLMN_MODE_ADD || mode == UPLMN_MODE_EDIT) {
        if (!UplmnSelector::IsValidPlmn(plmn)) {
            return NULL;
        }

        memset(req.plmn, 0, sizeof(req.plmn));
        memcpy(req.plmn, plmn, MAX_PLMN_LEN);
        if (req.plmn[5] == 0) {
            req.plmn[5] = '#';
        }

        if (UplmnSelector::IsValidUplmnAct(act)) {
            req.act = (BYTE)act;
        }
        else {
            req.act = (BYTE)UPLMN_ACT_BIT_UNKNOWN;
        }
    }
    else if (mode == UPLMN_MODE_DELETE) {
        memset(req.plmn, 0, sizeof(req.plmn));
        req.act = 0;
    }
    else {
        return NULL;
    }


    return new ModemData((char *) &req, length);
}

ModemData *ProtocolNetworkBuilder::BuildGetUplmn()
{
    sit_net_get_uplmn_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_UPLMN, length);
    return new ModemData((char *) &req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetDSNetworkType(int netType)
{
    sit_net_set_pref_network_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_DS_NTW_TYPE, length);
#if 0 // bypass
    req.pref_net_type = netType;
#else
    RilLogI("BuildSetDSNetworkType: netType=%d", netType);
//    SIT_NET_DS_NET_TYPE_GSM_WCDMA               = 0x00,
//    SIT_NET_DS_NET_TYPE_LTE_GSM_WCDMA           = 0x01,
//    SIT_NET_DS_NET_TYPE_GSM                     = 0x02,
//    SIT_NET_DS_NET_TYPE_CDMA_EVDO_AUTO          = 0x03,
//    SIT_NET_DS_NET_TYPE_LTE_CDMA_EVDO           = 0x04,
//    SIT_NET_DS_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA = 0x05,
    switch (netType) {
    // 3G + 2G
    case PREF_NET_TYPE_GSM_WCDMA:
    case PREF_NET_TYPE_GSM_WCDMA_AUTO:
    case PREF_NET_TYPE_TD_SCDMA_GSM:
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
        RilLogV("set to DS_NET_TYPE_GSM_WCDMA");
        req.pref_net_type = SIT_NET_DS_NET_TYPE_GSM_WCDMA;
        break;
    // 4G + 3G + 2G
    case PREF_NET_TYPE_LTE_GSM_WCDMA:
    case PREF_NET_TYPE_LTE_WCDMA:
    case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
        RilLogV("set to DS_NET_TYPE_LTE_GSM_WCDMA");
        req.pref_net_type = SIT_NET_DS_NET_TYPE_LTE_GSM_WCDMA;
        break;
    // 2G
    case PREF_NET_TYPE_GSM_ONLY:
        RilLogV("set to DS_NET_TYPE_GSM");
        req.pref_net_type = SIT_NET_DS_NET_TYPE_GSM;
        break;
    // CDMA
    case PREF_NET_TYPE_CDMA_EVDO_AUTO:
        RilLogV("set to DS_NET_TYPE_CDMA_EVDO_AUTO");
        req.pref_net_type = SIT_NET_DS_NET_TYPE_CDMA_EVDO_AUTO;
        break;
    // 4G + CDMA
    case PREF_NET_TYPE_LTE_CDMA_EVDO:
        RilLogV("set to DS_NET_TYPE_LTE_CDMA_EVDO");
        req.pref_net_type = SIT_NET_DS_NET_TYPE_LTE_CDMA_EVDO;
        break;
    // 4G + 3G + 2G + CDMA
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
    case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
        RilLogV("set to DS_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA");
        req.pref_net_type = SIT_NET_DS_NET_TYPE_LTE_CDMA_EVDO_GSM_WCDMA;
        break;
    // CDMA only or EVDO only
    case PREF_NET_TYPE_CDMA_ONLY:
    case PREF_NET_TYPE_EVDO_ONLY:
        RilLogV("set to DS_NET_TYPE_CDMA");
        req.pref_net_type = SIT_NET_DS_NET_TYPE_CDMA;
        break;

    // not clarified
    case PREF_NET_TYPE_WCDMA:
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
    case PREF_NET_TYPE_LTE_ONLY:
    case PREF_NET_TYPE_TD_SCDMA_ONLY:
    case PREF_NET_TYPE_TD_SCDMA_WCDMA:
    case PREF_NET_TYPE_TD_SCDMA_LTE:
    case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
    case PREF_NET_TYPE_TD_SCDMA_CDMA:
    case PREF_NET_TYPE_TD_SCDMA_CDMA_NO_EVDO:
    case PREF_NET_TYPE_TD_SCDMA_CDMA_EVDO_LTE:
    case PREF_NET_TYPE_TD_SCDMA_EVDO_NO_CDMA:
        RilLogV("not clarified netType. set to default");
        req.pref_net_type = SIT_NET_DS_NET_TYPE_GSM;
        break;
    default:
        RilLogV("not clarified netType. set to default");
        req.pref_net_type = SIT_NET_DS_NET_TYPE_GSM;
        break;
    } // end switch ~
#endif

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetRCNetworkType(int rcVersion, int rcSession, int rcPhase, int rcRaf, char *pUuid, int rcStatus)
{
    sit_net_set_rc_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_SET_RADIO_CAPABILITY, length);

    req.version = rcVersion;
    req.session_id = rcSession;
    req.phase = rcPhase;
    req.rc_raf = switchRafValueForCP(rcRaf);

    if (pUuid != NULL) {
        int uuidLen = strlen(pUuid);
        memcpy(req.uuid, pUuid, (uuidLen > SIT_MAX_UUID_LENGTH)? SIT_MAX_UUID_LENGTH:uuidLen );
    }
    req.status = rcStatus;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildGetRCNetworkType()
{
    sit_net_get_rc_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_RADIO_CAPABILITY, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetDuplexMode(BYTE mode_4g, BYTE mode_3g)
{
    sit_net_set_duplex_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_DUPLEX_MODE, length);

    req.duplex_mode_4g = mode_4g;
    req.duplex_mode_3g = mode_3g;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetDuplexMode(int mode)
{
    BYTE duplex_mode_4g = (BYTE)SIT_NET_DUPLEX_MODE_FDD_TDD;
    BYTE duplex_mode_3g = (BYTE)SIT_NET_DUPLEX_MODE_FDD_TDD;

    if(mode == DUPLEX_MODE_LTG) {
        duplex_mode_4g = (BYTE)SIT_NET_DUPLEX_MODE_TDD;
        duplex_mode_3g = (BYTE)SIT_NET_DUPLEX_MODE_TDD;
    } else if(mode == DUPLEX_MODE_LWG) {
        duplex_mode_4g = (BYTE)SIT_NET_DUPLEX_MODE_FDD_TDD;
        duplex_mode_3g = (BYTE)SIT_NET_DUPLEX_MODE_FDD;
    } else if (mode == DUPLEX_MODE_GLOBAL) {
        duplex_mode_4g = (BYTE)SIT_NET_DUPLEX_MODE_FDD_TDD;
        duplex_mode_3g = (BYTE)SIT_NET_DUPLEX_MODE_FDD_TDD;
    } else {
        // invalid duplex mode request
    }

    sit_net_set_duplex_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_DUPLEX_MODE, length);

    req.duplex_mode_4g = duplex_mode_4g;
    req.duplex_mode_3g = duplex_mode_3g;

    return new ModemData((char *)&req, length);

    return NULL;
}


ModemData *ProtocolNetworkBuilder::BuildGetDuplexMode()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_DUPLEX_MODE, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetEmergencyCallStatus(int status, int rat)
{
    sit_net_set_emergency_call_status_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_EMERGENCY_CALL_STATUS, length);

    if (status < EMERGENCY_CALL_STATUS_START || status > EMERGENCY_CALL_STATUS_FAIL) {
        // invalid statuss
        return NULL;
    }
    int emergencyCallStatus = status;
    int emergencyRadioTech = SIT_RAT_TYPE_UNKNOWN;

    if (0 < rat && rat <= RADIO_TECH_GSM) {
        emergencyRadioTech = rat;
    }
    else if (rat == RADIO_TECH_TD_SCDMA) {
        emergencyRadioTech = SIT_RAT_TYPE_TD_SCDMA;
    }
    else if (rat == RADIO_TECH_IWLAN) {
        emergencyRadioTech = SIT_RAT_TYPE_IWLAN;
    }
    else if (rat == RADIO_TECH_NR) {
        emergencyRadioTech = SIT_RAT_TYPE_5G;
    }
    else if (rat < 0 || rat == RADIO_TECH_UNSPECIFIED) {
        emergencyRadioTech = SIT_RAT_TYPE_UNSPECIFIED;
    }
    else {
        // consider as LTE in temporary
        if (rat == RADIO_TECH_LTE_CA) {
            emergencyRadioTech = SIT_RAT_TYPE_LTE;
        }
        // not supported yet
        // SIT_NET_EMERGENCY_AVAILABLE_RAT_HSPADCPLUS, SIT_NET_EMERGENCY_AVAILABLE_RAT_LTE_CA
    }

    req.status = emergencyCallStatus & 0xFF;
    req.rat = emergencyRadioTech & 0xFF;

    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetMicroCellSearch(BYTE srch_mode)
{
    sit_net_set_micro_cell_search_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_MICRO_CELL_SEARCH, length);

    req.srch_mode = srch_mode;

    return new ModemData((char *)&req, length);
}

#ifdef SUPPORT_CDMA
ModemData *ProtocolNetworkBuilder::BuildSetCdmaSetRoamingType(int cdmaRoamingType)
{
    sit_net_set_cdma_roaming_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_CDMA_ROAMING_PREFERENCE, length);
    switch(cdmaRoamingType) {
    case CDMA_ROAMING_AFFILIATED_NETWORKS:
        req.cdma_roaming_type = SIT_CDMA_RM_AFFILIATED;
        break;
    case CDMA_ROAMING_ANY_NETWORK:
        req.cdma_roaming_type = SIT_CDMA_RM_ANY;
        break;
    case CDMA_ROAMING_HOME_ONLY:
        req.cdma_roaming_type = SIT_CDMA_RM_HOME;
        break;
    }
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildQueryCdmaRoamingType()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_CDMA_ROAMING_PREFERENCE, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetCdmaHybridMode(int hybridMode)
{
    sit_net_set_cdma_hybrid_mode_req req;
    int length = sizeof(req);
    if (hybridMode >= (int)HYBRID_MODE_1X_HRPD && hybridMode <= (int)HYBRID_MODE_EHRPD_ONLY) {
        InitRequestHeader(&req.hdr, SIT_SET_CDMA_HYBRID_MODE, length);
        req.hybrid_mode = hybridMode;
        return new ModemData((char *)&req, length);
    }
    return NULL;
}

ModemData *ProtocolNetworkBuilder::BuildGetCdmaHybridMode()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_CDMA_HYBRID_MODE, length);
    return new ModemData((char *)&req, length);
}
#endif

ModemData *ProtocolNetworkBuilder::BuildSetDualNetworkAndAllowData(int typeForPrimary, int typeForSecondary, int allowedForPrimary, int allowedForSecondary)
{
    sit_net_set_dual_network_and_allow_data_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_DUAL_NTW_AND_PS_TYPE, length);

    req.pref_net_type_for_primary = translateNetworktype(typeForPrimary);
    req.pref_net_type_for_secondary = translateNetworktype(typeForSecondary);
    req.allowed_for_primary = allowedForPrimary;
    req.allowed_for_secondary = allowedForSecondary;
    return new ModemData((char *)&req, length);
}

static int ConvertNetworkScanType(int rilScanType) {
    if (rilScanType == RIL_ONE_SHOT) {
        return SIT_SCAN_TYPE_ONESHOT;
    }
    else if (rilScanType == RIL_PERIODIC) {
        return SIT_SCAN_TYPE_PERIODIC;
    }
    // not to start if invalid scan type
    return SIT_SCAN_TYPE_STOP;
}

static int ConvertNetworkType(int accessNetworkType) {
    switch (accessNetworkType) {
    case GERAN:
        return SIT_NET_ACCESS_RADIO_TYPE_GERAN;
    case UTRAN:
        return SIT_NET_ACCESS_RADIO_TYPE_UTRAN;
    case EUTRAN:
        return SIT_NET_ACCESS_RADIO_TYPE_EUTRAN;
    }
    return SIT_NET_ACCESS_RADIO_TYPE_UNKNOWN;
}

ModemData *ProtocolNetworkBuilder::BuildStartNetworkScan(int scantype, int timeInterval, int lenSpecifiers,
                                                            RIL_RadioAccessSpecifier *pSpecifiers)
{
    return this->BuildStartNetworkScan(scantype, timeInterval, lenSpecifiers, pSpecifiers,
            300, true, 3, 0, NULL);
}

ModemData *ProtocolNetworkBuilder::BuildStartNetworkScan(
        int scantype, int timeInterval, int specifiersLength, RIL_RadioAccessSpecifier *specifiers,
        int maxSearchTime, bool incrementalResults, int incrementalResultsPeriodicity,
        int numOfMccMncs, char **mccMncs) {

    // VtsHalRadioV1_4Target#RadioHidlTest_v1_4.startNetworkScan
    // Test IRadio.startNetworkScan() with invalid specifier.
    if (specifiersLength <= 0 || specifiers == NULL) {
        RilLogW("Assert. Test IRadio.startNetworkScan() with invalid specifier.");
        return NULL;
    }

    // Test IRadio.startNetworkScan() with invalid interval.
    if (timeInterval < SCAN_INTERVAL_MIN || timeInterval > SCAN_INTERVAL_MAX) {
        RilLogW("Assert. Test IRadio.startNetworkScan() with invalid interval.");
        return NULL;
    }

    if (maxSearchTime != 0) {
        // Test IRadio.startNetworkScan() with invalid max search time.
        if (maxSearchTime < MAX_SEARCH_TIME_MIN || maxSearchTime > MAX_SEARCH_TIME_MAX) {
            RilLogW("Assert. Test IRadio.startNetworkScan() with invalid  max search time.");
            return NULL;
        }

        // Test IRadio.startNetworkScan() with invalid periodicity.
        if (incrementalResultsPeriodicity < INCREMENT_PERIODIC_MIN
                || incrementalResultsPeriodicity > INCREMENT_PERIODIC_MAX) {
            RilLogW("Assert. Test IRadio.startNetworkScan() with invalid periodicity.");
            return NULL;
        }
    }

    sit_net_start_scanning_network req;
    memset(&req, 0, sizeof(req));
    // init header in later b/c data is variable
    int length = sizeof(sit_net_start_scanning_network) - MAX_NETWORK_SCAN_DATA; // default length without variable data

    req.scan_type = ConvertNetworkScanType(scantype);
    req.interval = 0;
    if (scantype == RIL_PERIODIC) {
        req.interval = timeInterval;
    }

    req.max_search_time = maxSearchTime;

    req.incremental_results =
            incrementalResults ? SIT_SCAN_INCREMENTAL_RESULT : SIT_SCAN_NO_INCREMENTAL_RESULT;
    if (incrementalResultsPeriodicity < 3) {
        incrementalResultsPeriodicity = 3;
    }
    else if (incrementalResultsPeriodicity > 10) {
        incrementalResultsPeriodicity = 10;
    }
    req.periodicity = incrementalResultsPeriodicity;

    // radio specifiers
    if (specifiersLength > MAX_RADIO_ACCESS_NETWORKS) {
        specifiersLength = MAX_RADIO_ACCESS_NETWORKS;
    }
    req.num_record = (BYTE)specifiersLength;

    SIT_NET_SCAN_SPECIFIER *records = (SIT_NET_SCAN_SPECIFIER *)req.data;
    for (int p = 0; p < specifiersLength; p++) {
        RIL_RadioAccessSpecifier *ras = &specifiers[p];
        SIT_NET_SCAN_SPECIFIER *out = &records[p];
        int networkType = ConvertNetworkType((int)ras->radio_access_network);
        // skip unknown network type
        if (networkType == SIT_NET_ACCESS_RADIO_TYPE_UNKNOWN) {
            continue;
        }
        // bands
        out->network_type = (BYTE)networkType;
        int bandsLength = ras->bands_length;
        if (bandsLength > MAX_BANDS) {
            bandsLength = MAX_BANDS;
        }
        out->num_band = (UINT32)bandsLength;

        for (int i = 0; i < bandsLength; i++ ) {
            switch (networkType) {
            case SIT_NET_ACCESS_RADIO_TYPE_GERAN:
                out->bands[i] = (BYTE)ras->bands.geran_bands[i];
                break;
            case SIT_NET_ACCESS_RADIO_TYPE_UTRAN:
                out->bands[i] = (BYTE)ras->bands.utran_bands[i];
                break;
            case SIT_NET_ACCESS_RADIO_TYPE_EUTRAN:
                out->bands[i] = (BYTE)ras->bands.eutran_bands[i];
                break;
            }
        } // end for i ~

        // channels
        int channelsLength = ras->channels_length;
        if (channelsLength > MAX_CHANNELS) {
            channelsLength = MAX_CHANNELS;
        }
        out->num_channel = channelsLength;

        for (int i = 0; i < channelsLength; i++ ) {
            out->channels[i] = (UINT16)(ras->channels[i] & 0xFFFF);
        } // end for i ~
    } // end for p ~
    int sizeOfSpecifierData = sizeof(SIT_NET_SCAN_SPECIFIER) * specifiersLength;
    length += sizeOfSpecifierData;

    // PLMN IDs
    if (numOfMccMncs < 0) {
        numOfMccMncs = 0;
        mccMncs = NULL;
    }
    if (numOfMccMncs > MAX_NETWORK_PLMN_IDS) {
        numOfMccMncs = MAX_NETWORK_PLMN_IDS;
    }
    req.num_plmn = (BYTE)numOfMccMncs;
    SIT_NET_SCAN_PLMN_ID *plmnIds = (SIT_NET_SCAN_PLMN_ID *)(req.data + sizeOfSpecifierData);
    for (int i = 0; i < numOfMccMncs; i++) {
        char *mccMnc = mccMncs[i];
        memset(&plmnIds[i], 0, sizeof(SIT_NET_SCAN_PLMN_ID));
        if (TextUtils::IsDigitsOnly(mccMnc)) {
            int len = strlen(mccMnc);
            if (len == 5 || len == 6) {
                memcpy(plmnIds[i].mcc, mccMnc, 3);
                memcpy(plmnIds[i].mnc, mccMnc, len - 3);
                if (len == 5) {
                    plmnIds[i].mnc[2] = '#';
                }
            }
        }
    } // end for i ~
    length += sizeof(SIT_NET_SCAN_PLMN_ID) * numOfMccMncs;
    InitRequestHeader(&req.hdr, SIT_START_SCANNING_NETWORKS, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildStopNetworkScan()
{
    sit_net_start_scanning_network req;
    memset(&req, 0, sizeof(req));
    int length = sizeof(sit_net_start_scanning_network) - MAX_NETWORK_SCAN_DATA; // default length without variable data
    InitRequestHeader(&req.hdr, SIT_START_SCANNING_NETWORKS, length);
    req.scan_type = SIT_SCAN_TYPE_STOP;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSvNumber(char* svn)
{
    sit_id_set_sv_number_req req;
    int length = sizeof(req);
    memset(&req, 0, length);
    InitRequestHeader(&req.hdr, SIT_OEM_SET_SVN, length);

    if (svn != NULL) {
        memcpy(req.sv_number, svn, sizeof(req.sv_number));
    }
    return new ModemData((char *) &req, length);
}

ModemData *ProtocolNetworkBuilder::BuildGetManualRatMode()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_MANUAL_RAT_MODE, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetManualRatMode(int mode, int rat)
{
    sit_net_set_manual_rat_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_MANUAL_RAT_MODE, length);

    req.rat_mode_set = (BYTE)mode;
    req.rat = rat;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildGetFrequencyLock()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_FREQUENCY_LOCK, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetFrequencyLock(int mode, int rat, int ltePci, int lteEarfcn, int gsmArfcn, int wcdmaPsc, int wcdmaUarfcn)
{
    sit_net_set_freq_lock_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_FREQUENCY_LOCK, length);

    req.freq_mode_set = (BYTE)mode;
    req.rat = (BYTE)rat;
    req.lte_pci  = ltePci;
    req.lte_earfcn  = lteEarfcn;
    req.gsm_arfcn  = gsmArfcn;
    req.wcdma_psc  = wcdmaPsc;
    req.wcdma_uarfcn  = wcdmaUarfcn;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildSetEndcMode(int mode)
{
    sit_net_set_endc_mode_req req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_SET_ENDC_MODE, length);

    req.mode = (BYTE)mode;
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildGetEndcMode()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_ENDC_MODE, length);
    return new ModemData((char *)&req, length);
}

ModemData *ProtocolNetworkBuilder::BuildGetFrequencyInfo()
{
    null_data_format req;
    int length = sizeof(req);
    InitRequestHeader(&req.hdr, SIT_GET_FREQUENCY_INFO, length);
    return new ModemData((char *)&req, length);
}
