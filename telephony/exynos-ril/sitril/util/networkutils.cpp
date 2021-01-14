/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#include "networkutils.h"
#include "rillog.h"

static bool debug = true;

int NetworkUtils::getCellInfoTypeRadioTechnology(int rat) {
    switch(rat) {
        case RADIO_TECH_GPRS:
        case RADIO_TECH_EDGE:
        case RADIO_TECH_GSM: {
            return RIL_CELL_INFO_TYPE_GSM;
        }
        case RADIO_TECH_UMTS:
        case RADIO_TECH_HSDPA:
        case RADIO_TECH_HSUPA:
        case RADIO_TECH_HSPA:
        case RADIO_TECH_HSPAP: {
            return RIL_CELL_INFO_TYPE_WCDMA;
        }
        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
        case RADIO_TECH_1xRTT:
        case RADIO_TECH_EVDO_0:
        case RADIO_TECH_EVDO_A:
        case RADIO_TECH_EVDO_B:
        case RADIO_TECH_EHRPD: {
            return RIL_CELL_INFO_TYPE_CDMA;
        }
        case RADIO_TECH_LTE:
        case RADIO_TECH_LTE_CA: {
            return RIL_CELL_INFO_TYPE_LTE;
        }
        case RADIO_TECH_TD_SCDMA: {
            return RIL_CELL_INFO_TYPE_TD_SCDMA;
        }
        case RADIO_TECH_NR:
            return RIL_CELL_INFO_TYPE_NR;
        default: {
            break;
        }
    }
    return RIL_CELL_INFO_TYPE_NONE;
}

static int getAdjustedRaf(int raf) {
    int newRaf = 0;
    newRaf = ((NETWORK_TYPE_BITMAP_GSM & raf) > 0) ? (NETWORK_TYPE_BITMAP_GSM | newRaf) : newRaf;
    newRaf = ((NETWORK_TYPE_BITMAP_WCDMA & raf) > 0) ? (NETWORK_TYPE_BITMAP_WCDMA | newRaf) : newRaf;
    newRaf = ((NETWORK_TYPE_BITMAP_CDMA & raf) > 0) ? (NETWORK_TYPE_BITMAP_CDMA | newRaf) : newRaf;
    newRaf = ((NETWORK_TYPE_BITMAP_EVDO & raf) > 0) ? (NETWORK_TYPE_BITMAP_EVDO | newRaf) : newRaf;
    newRaf = ((NETWORK_TYPE_BITMAP_LTE & raf) > 0) ? (NETWORK_TYPE_BITMAP_LTE | newRaf) : newRaf;
    newRaf = ((NETWORK_TYPE_BITMAP_NR & raf) > 0) ? (NETWORK_TYPE_BITMAP_NR | newRaf) : newRaf;
    newRaf = ((NETWORK_TYPE_BITMAP_TDS_CDMA & raf) > 0) ? (NETWORK_TYPE_BITMAP_TDS_CDMA | newRaf) : newRaf;

    return newRaf;
}

int NetworkUtils::getNetworkTypeFromRaf(int raf)
{
    int oldRaf = raf;
    raf = getAdjustedRaf(raf);
    if (debug) {
        if (oldRaf != raf) {
            RilLogV("adjust RAF from 0x%08X to 0x%08X.", oldRaf, raf);
        }
    }

    switch (raf) {
    case NETWORK_TYPE_BITMAP_GSM_WCDMA:
        return PREF_NET_TYPE_GSM_WCDMA;
    case NETWORK_TYPE_BITMAP_GSM_ONLY:
        return PREF_NET_TYPE_GSM_ONLY;
    case NETWORK_TYPE_BITMAP_WCDMA_ONLY:
        return PREF_NET_TYPE_WCDMA;
    case NETWORK_TYPE_BITMAP_CDMA_EVDO_AUTO:
        return PREF_NET_TYPE_CDMA_EVDO_AUTO;
    case NETWORK_TYPE_BITMAP_LTE_CDMA_EVDO:
        return PREF_NET_TYPE_LTE_CDMA_EVDO;
    case NETWORK_TYPE_BITMAP_CDMA_ONLY:
        return PREF_NET_TYPE_CDMA_ONLY;
    case NETWORK_TYPE_BITMAP_EVDO_ONLY:
        return PREF_NET_TYPE_EVDO_ONLY;
    case NETWORK_TYPE_BITMAP_GSM_WCDMA_CDMA_EVDO_AUTO:
        return PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
    case NETWORK_TYPE_BITMAP_LTE_GSM_WCDMA:
        return PREF_NET_TYPE_LTE_GSM_WCDMA;
    case NETWORK_TYPE_BITMAP_LTE_CMDA_EVDO_GSM_WCDMA:
        return PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
    case NETWORK_TYPE_BITMAP_LTE_ONLY:
        return PREF_NET_TYPE_LTE_ONLY;
    case NETWORK_TYPE_BITMAP_LTE_WCDMA:
        return PREF_NET_TYPE_LTE_WCDMA;
    case NETWORK_TYPE_BITMAP_TD_SCDMA_ONLY:
        return PREF_NET_TYPE_TD_SCDMA_ONLY;
    case NETWORK_TYPE_BITMAP_TD_SCDMA_WCDMA:
        return PREF_NET_TYPE_TD_SCDMA_WCDMA;
    case NETWORK_TYPE_BITMAP_TD_SCDMA_LTE:
        return PREF_NET_TYPE_TD_SCDMA_LTE;
    case NETWORK_TYPE_BITMAP_TD_SCDMA_GSM:
        return PREF_NET_TYPE_TD_SCDMA_GSM;
    case NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_LTE:
        return PREF_NET_TYPE_TD_SCDMA_GSM_LTE;
    case NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA:
        return PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA;
    case NETWORK_TYPE_BITMAP_TD_SCDMA_WCDMA_LTE:
        return PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE;
    case NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA_LTE:
        return PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE;
    case NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
        return PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO;
    case NETWORK_TYPE_BITMAP_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
        return PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA;
    // support NR
    case NETWORK_TYPE_BITMAP_NR_ONLY:
        return PREF_NET_TYPE_NR_ONLY;
    case NETWORK_TYPE_BITMAP_NR_LTE:
        return PREF_NET_TYPE_NR_LTE;
    case NETWORK_TYPE_BITMAP_NR_LTE_CDMA_EVDO_AUTO:
        return PREF_NET_TYPE_NR_LTE_CDMA_EVDO;
    case NETWORK_TYPE_BITMAP_NR_LTE_CDMA_GSM_WCDMA:
        return PREF_NET_TYPE_NR_LTE_GSM_WCDMA;
    case NETWORK_TYPE_BITMAP_NR_LTE_CMDA_EVDO_GSM_WCDMA:
        return PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA;
    case NETWORK_TYPE_BITMAP_NR_LTE_WCDMA:
        return PREF_NET_TYPE_NR_LTE_WCDMA;
    case NETWORK_TYPE_BITMAP_NR_TD_SCDMA_LTE:
        return PREF_NET_TYPE_NR_LTE_TDSCDMA;
    case NETWORK_TYPE_BITMAP_NR_TD_SCDMA_LTE_GSM:
        return PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM;
    case NETWORK_TYPE_BITMAP_NR_TD_SCDMA_WCDMA_LTE:
        return PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA;
    case NETWORK_TYPE_BITMAP_NR_TD_SCDMA_GSM_WCDMA_LTE:
        return PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA;
    case NETWORK_TYPE_BITMAP_NR_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
        return PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA;
    default:
        break;
    }

    if (debug) {
        printSupportedNetworkTypeBitmap();
        return PREF_NET_TYPE_LTE_GSM_WCDMA;
    }

    return -1;
}

int NetworkUtils::getRafFromNetworkType(int networkType)
{
    switch (networkType) {
    case PREF_NET_TYPE_GSM_WCDMA:
        return NETWORK_TYPE_BITMAP_GSM_WCDMA;
    case PREF_NET_TYPE_GSM_ONLY:
        return NETWORK_TYPE_BITMAP_GSM_ONLY;
    case PREF_NET_TYPE_WCDMA:
        return NETWORK_TYPE_BITMAP_WCDMA_ONLY;
    case PREF_NET_TYPE_CDMA_EVDO_AUTO:
        return NETWORK_TYPE_BITMAP_CDMA_EVDO_AUTO;
    case PREF_NET_TYPE_LTE_CDMA_EVDO:
        return NETWORK_TYPE_BITMAP_LTE_CDMA_EVDO;
    case PREF_NET_TYPE_LTE_GSM_WCDMA:
        return NETWORK_TYPE_BITMAP_LTE_GSM_WCDMA;
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA:
        return NETWORK_TYPE_BITMAP_LTE_CMDA_EVDO_GSM_WCDMA;
    case PREF_NET_TYPE_LTE_ONLY:
        return NETWORK_TYPE_BITMAP_LTE_ONLY;
    case PREF_NET_TYPE_LTE_WCDMA:
        return NETWORK_TYPE_BITMAP_LTE_WCDMA;
    case PREF_NET_TYPE_CDMA_ONLY:
        return NETWORK_TYPE_BITMAP_CDMA_ONLY;
    case PREF_NET_TYPE_EVDO_ONLY:
        return NETWORK_TYPE_BITMAP_EVDO_ONLY;
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
        return NETWORK_TYPE_BITMAP_GSM_WCDMA_CDMA_EVDO_AUTO;
    case PREF_NET_TYPE_TD_SCDMA_ONLY:
        return NETWORK_TYPE_BITMAP_TD_SCDMA_ONLY;
    case PREF_NET_TYPE_TD_SCDMA_WCDMA:
        return NETWORK_TYPE_BITMAP_TD_SCDMA_WCDMA;
    case PREF_NET_TYPE_TD_SCDMA_LTE:
        return NETWORK_TYPE_BITMAP_TD_SCDMA_LTE;
    case PREF_NET_TYPE_TD_SCDMA_GSM:
        return (NETWORK_TYPE_BITMAP_GSM | RAF_TD_SCDMA);
    case PREF_NET_TYPE_TD_SCDMA_GSM_LTE:
        return NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_LTE;
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:
        return NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA;
    case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:
        return NETWORK_TYPE_BITMAP_TD_SCDMA_WCDMA_LTE;
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:
        return NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA_LTE;
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO:
        return NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO;
    case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA:
        return NETWORK_TYPE_BITMAP_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA;
    // support NR
    case PREF_NET_TYPE_NR_ONLY:
        return NETWORK_TYPE_BITMAP_NR_ONLY;
    case PREF_NET_TYPE_NR_LTE:
        return NETWORK_TYPE_BITMAP_NR_LTE;
    case PREF_NET_TYPE_NR_LTE_CDMA_EVDO:
        return NETWORK_TYPE_BITMAP_NR_LTE_CDMA_EVDO_AUTO;
    case PREF_NET_TYPE_NR_LTE_GSM_WCDMA:
        return NETWORK_TYPE_BITMAP_NR_LTE_CDMA_GSM_WCDMA;
    case PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
        return NETWORK_TYPE_BITMAP_NR_LTE_CMDA_EVDO_GSM_WCDMA;
    case PREF_NET_TYPE_NR_LTE_WCDMA:
        return NETWORK_TYPE_BITMAP_NR_LTE_WCDMA;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA:
        return NETWORK_TYPE_BITMAP_NR_TD_SCDMA_LTE;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM:
        return NETWORK_TYPE_BITMAP_NR_TD_SCDMA_LTE_GSM;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA:
        return NETWORK_TYPE_BITMAP_NR_TD_SCDMA_WCDMA_LTE;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA:
        return NETWORK_TYPE_BITMAP_NR_TD_SCDMA_GSM_WCDMA_LTE;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
        return NETWORK_TYPE_BITMAP_NR_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA;
    default:
        return -1;
    }
}

int NetworkUtils::getRadioTechnologyFromAccessNetwork(int accessNetwork)
{
    switch (accessNetwork) {
    case ACCESS_NETWORK_UNKNOWN:
        return RADIO_TECH_UNKNOWN;
    case ACCESS_NETWORK_GERAN:
        return RADIO_TECH_GPRS;
    case ACCESS_NETWORK_UTRAN:
        return RADIO_TECH_UMTS;
    case ACCESS_NETWORK_EUTRAN:
        return RADIO_TECH_LTE;
    case ACCESS_NETWORK_CDMA2000:
        return RADIO_TECH_1xRTT;
    default:
        break;
    }
    return RADIO_TECH_UNKNOWN;
}

int NetworkUtils::getRadioTechnologyToAccessNetworkType(int rat)
{
    switch(rat) {
    case RADIO_TECH_GPRS:
    case RADIO_TECH_EDGE:
    case RADIO_TECH_GSM:
        return ACCESS_NETWORK_UNKNOWN;
    case RADIO_TECH_UMTS:
    case RADIO_TECH_HSDPA:
    case RADIO_TECH_HSPAP:
    case RADIO_TECH_HSUPA:
    case RADIO_TECH_HSPA:
    case RADIO_TECH_TD_SCDMA:
        return ACCESS_NETWORK_UTRAN;
    case RADIO_TECH_IS95A:
    case RADIO_TECH_IS95B:
    case RADIO_TECH_1xRTT:
    case RADIO_TECH_EVDO_0:
    case RADIO_TECH_EVDO_A:
    case RADIO_TECH_EVDO_B:
    case RADIO_TECH_EHRPD:
        return ACCESS_NETWORK_CDMA2000;
    case RADIO_TECH_LTE:
    case RADIO_TECH_LTE_CA:
        return ACCESS_NETWORK_EUTRAN;
    case RADIO_TECH_IWLAN:
        return ACCESS_NETWORK_IWLAN;
    case RADIO_TECH_UNKNOWN:
    default:
        return ACCESS_NETWORK_UNKNOWN;
    }
}

const char *NetworkUtils::getRadioTechnologyString(int rat)
{
    switch(rat) {
    case RADIO_TECH_GPRS:
        return "GPRS";
    case RADIO_TECH_EDGE:
        return "EDGE";
    case RADIO_TECH_GSM:
        return "GSM";
    case RADIO_TECH_UMTS:
        return "UMTS";
    case RADIO_TECH_HSDPA:
        return "HSDPA";
    case RADIO_TECH_HSPAP:
        return "HSPAP";
    case RADIO_TECH_HSUPA:
        return "HSUPA";
    case RADIO_TECH_HSPA:
        return "HSPA";
    case RADIO_TECH_TD_SCDMA:
        return "TD_SCDMA";
    case RADIO_TECH_IS95A:
        return "IS95A";
    case RADIO_TECH_IS95B:
        return "IS95B";
    case RADIO_TECH_1xRTT:
        return "1xRTT";
    case RADIO_TECH_EVDO_0:
        return "EVDO_0";
    case RADIO_TECH_EVDO_A:
        return "EVDO_A";
    case RADIO_TECH_EVDO_B:
        return "EVDO_B";
    case RADIO_TECH_EHRPD:
        return "EHRPD";
    case RADIO_TECH_LTE:
        return "LTE";
    case RADIO_TECH_LTE_CA:
        return "LTE_CA";
    case RADIO_TECH_IWLAN:
        return "IWLAN";
    case RADIO_TECH_NR:
        return "NR";
    case RADIO_TECH_UNKNOWN:
        return "UNKNOWN";
    default:
        return "UNKNOWN";
    }
}
const char *NetworkUtils::getRegStateString(int regState)
{
    switch (regState) {
    case RIL_NOT_REG_AND_NOT_SEARCHING:
        return "NOT_REG_AND_NOT_SEARCHING";
    case RIL_REG_HOME:
        return "REG_HOME";
    case RIL_NOT_REG_AND_SEARCHING:
        return "NOT_REG_AND_SEARCHING";
    case RIL_REG_DENIED:
        return "REG_DENIED";
    case RIL_UNKNOWN:
        return "UNKNOWN";
    case RIL_REG_ROAMING:
        return "REG_ROAMING";
    case RIL_NOT_REG_AND_EMERGENCY_AVAILABLE_AND_NOT_SEARCHING:
        return "NOT_REG_AND_EMERGENCY_AVAILABLE_AND_NOT_SEARCHING";
    case RIL_NOT_REG_AND_EMERGENCY_AVAILABLE_AND_SEARCHING:
        return "NOT_REG_AND_EMERGENCY_AVAILABLE_AND_SEARCHING";
    case RIL_REG_DENIED_AND_EMERGENCY_AVAILABLE:
        return "REG_DENIED_AND_EMERGENCY_AVAILABLE";
    case RIL_UNKNOWN_AND_EMERGENCY_AVAILABLE:
        return "UNKNOWN_AND_EMERGENCY_AVAILABLE";
    default:
        return "UNKNOWN";
    }
}
const char *NetworkUtils::getAccessNewtorkString(int accessNetwork)
{
    switch (accessNetwork) {
    case ACCESS_NETWORK_UNKNOWN:
        return "UNKNOWN";
    case ACCESS_NETWORK_GERAN:
        return "GERAN";
    case ACCESS_NETWORK_UTRAN:
        return "UTRAN";
    case ACCESS_NETWORK_EUTRAN:
        return "EUTRAN";
    case ACCESS_NETWORK_CDMA2000:
        return "CDMA2000";
    default:
        return "UNKNOWN";
    }
}

bool NetworkUtils::isInService(int regState)
{
    return (regState == RIL_REG_HOME) || (regState == RIL_REG_ROAMING);
}

bool NetworkUtils::isEmergencyOnly(int regState)
{
    return (regState == RIL_NOT_REG_AND_EMERGENCY_AVAILABLE_AND_NOT_SEARCHING) ||
           (regState == RIL_NOT_REG_AND_EMERGENCY_AVAILABLE_AND_SEARCHING) ||
           (regState == RIL_UNKNOWN_AND_EMERGENCY_AVAILABLE) ||
           (regState == RIL_REG_DENIED_AND_EMERGENCY_AVAILABLE);
}

void NetworkUtils::printSupportedNetworkTypeBitmap()
{
    struct NetworTypeBitmap {
        const char *name;
        int bitmap;
    };

    struct NetworTypeBitmap supportedList[] = {
        {"NETWORK_TYPE_BITMAP_GSM_WCDMA", NETWORK_TYPE_BITMAP_GSM_WCDMA},
        {"NETWORK_TYPE_BITMAP_GSM_ONLY", NETWORK_TYPE_BITMAP_GSM_ONLY},
        {"NETWORK_TYPE_BITMAP_WCDMA", NETWORK_TYPE_BITMAP_WCDMA},
        {"NETWORK_TYPE_BITMAP_GSM_WCDMA_AUTO", NETWORK_TYPE_BITMAP_GSM_WCDMA_AUTO},
        {"NETWORK_TYPE_BITMAP_CDMA_EVDO_AUTO", NETWORK_TYPE_BITMAP_CDMA_EVDO_AUTO},
        {"NETWORK_TYPE_BITMAP_CDMA_ONLY", NETWORK_TYPE_BITMAP_CDMA_ONLY},
        {"NETWORK_TYPE_BITMAP_EVDO_ONLY", NETWORK_TYPE_BITMAP_EVDO_ONLY},
        {"NETWORK_TYPE_BITMAP_GSM_WCDMA_CDMA_EVDO_AUTO", NETWORK_TYPE_BITMAP_GSM_WCDMA_CDMA_EVDO_AUTO},
        {"NETWORK_TYPE_BITMAP_LTE_CDMA_EVDO", NETWORK_TYPE_BITMAP_LTE_CDMA_EVDO},
        {"NETWORK_TYPE_BITMAP_LTE_GSM_WCDMA", NETWORK_TYPE_BITMAP_LTE_GSM_WCDMA},
        {"NETWORK_TYPE_BITMAP_LTE_CMDA_EVDO_GSM_WCDMA", NETWORK_TYPE_BITMAP_LTE_CMDA_EVDO_GSM_WCDMA},
        {"NETWORK_TYPE_BITMAP_LTE_ONLY", NETWORK_TYPE_BITMAP_LTE_ONLY},
        {"NETWORK_TYPE_BITMAP_LTE_WCDMA", NETWORK_TYPE_BITMAP_LTE_WCDMA},
        {"NETWORK_TYPE_BITMAP_TD_SCDMA_ONLY", NETWORK_TYPE_BITMAP_TD_SCDMA_ONLY},
        {"NETWORK_TYPE_BITMAP_TD_SCDMA_WCDMA", NETWORK_TYPE_BITMAP_TD_SCDMA_WCDMA},
        {"NETWORK_TYPE_BITMAP_TD_SCDMA_LTE", NETWORK_TYPE_BITMAP_TD_SCDMA_LTE},
        {"NETWORK_TYPE_BITMAP_TD_SCDMA_GSM", NETWORK_TYPE_BITMAP_TD_SCDMA_GSM},
        {"NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_LTE", NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_LTE},
        {"NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA", NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA},
        {"NETWORK_TYPE_BITMAP_TD_SCDMA_WCDMA_LTE", NETWORK_TYPE_BITMAP_TD_SCDMA_WCDMA_LTE},
        {"NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA_LTE", NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA_LTE},
        {"NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO", NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO},
        {"NETWORK_TYPE_BITMAP_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA", NETWORK_TYPE_BITMAP_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA},
    };

    size_t size = sizeof(supportedList) / sizeof(supportedList[0]);
    RilLog("###########printSupportedNetworkTypeBitmap###########");
    for (size_t i = 0; i < size; i++) {
        RilLogV("[%zu] %s(0x%08X)", i, supportedList[i].name, supportedList[i].bitmap);
    } // end for i ~
    RilLog("######################################################");
}

int NetworkUtils::fetchMcc(const char *plmn)
{
    if (TextUtils::IsDigitsOnly(plmn)) {
        int len = strlen(plmn);
        if (len == 5 || len == 6) {
            char mccStr[4] = {0, };
            memcpy(mccStr, plmn, 3);
            int mcc = atoi(mccStr);
            return mcc;
        }
    }
    return INT_MAX;
}

int NetworkUtils::fetchMnc(const char *plmn)
{
    char mncStr[4] = {0, };
    if (plmn != NULL) {
        memcpy(mncStr, plmn + 3, 3);
        if (mncStr[2] == '#') {
            mncStr[2] = 0;
        }

        if (TextUtils::IsDigitsOnly(mncStr)) {
            int len = strlen(plmn);
            if (len == 5 || len == 6) {
                char mncStr[4] = {0, };
                int mccLength = len - 3;
                memcpy(mncStr, plmn + 3, mccLength);
                int mnc = atoi(mncStr);
                return ril::util::mnc::encode(mnc, mccLength);
            }
        }
    }
    return INT_MAX;
}

void NetworkUtils::convertCellIdentityGsm(RIL_CellIdentityGsm_v12& out,
        RIL_CellIdentityGsm_V1_2& cellIdentityGsm)
{
    out.lac = cellIdentityGsm.lac;
    out.cid = cellIdentityGsm.cid;
    out.arfcn = cellIdentityGsm.arfcn;
    out.bsic = cellIdentityGsm.bsic;
    out.mcc = cellIdentityGsm.mcc;
    out.mnc = cellIdentityGsm.mnc;
}
void NetworkUtils::convertCellIdentityCdma(RIL_CellIdentityCdma& out,
        RIL_CellIdentityCdma_V1_2& cellIdentityCdma)
{
    out.basestationId = cellIdentityCdma.basestationId;
    out.latitude = cellIdentityCdma.latitude;
    out.longitude = cellIdentityCdma.longitude;
    out.systemId = cellIdentityCdma.systemId;
    out.networkId = cellIdentityCdma.networkId;
}
void NetworkUtils::convertCellIdentityWcdma(RIL_CellIdentityWcdma_v12& out,
        RIL_CellIdentityWcdma_V1_2& cellIdentityWcdma)
{
    out.lac = cellIdentityWcdma.lac;
    out.cid = cellIdentityWcdma.cid;
    out.psc = cellIdentityWcdma.psc;
    out.uarfcn = cellIdentityWcdma.uarfcn;
    out.mcc = cellIdentityWcdma.mcc;
    out.mnc = cellIdentityWcdma.mnc;
}
void NetworkUtils::convertCellIdentityLte(RIL_CellIdentityLte_v12& out,
        RIL_CellIdentityLte_V1_2& cellIdentityLte)
{
    out.tac = cellIdentityLte.tac;
    out.ci = cellIdentityLte.ci;
    out.pci = cellIdentityLte.pci;
    out.earfcn = cellIdentityLte.earfcn;
    out.mcc = cellIdentityLte.mcc;
    out.mnc = cellIdentityLte.mnc;
}
void NetworkUtils::convertCellIdentityTdscdma(RIL_CellIdentityTdscdma& out,
        RIL_CellIdentityTdscdma_V1_2& cellIdentityTdscdma)
{
    out.lac = cellIdentityTdscdma.lac;
    out.cid = cellIdentityTdscdma.cid;
    out.cpid = cellIdentityTdscdma.cpid;
    out.mcc = cellIdentityTdscdma.mcc;
    out.mnc = cellIdentityTdscdma.mnc;
}

void NetworkUtils::convertCellIdentity(RIL_CellIdentity_v16& out, RIL_CellIdentity_V1_2& cellIdentity)
{
    out.cellInfoType = cellIdentity.cellInfoType;
    switch (cellIdentity.cellInfoType) {
    case RIL_CELL_INFO_TYPE_GSM:
        convertCellIdentityGsm(out.cellIdentityGsm, cellIdentity.cellIdentityGsm);
        break;
    case RIL_CELL_INFO_TYPE_WCDMA:
        convertCellIdentityWcdma(out.cellIdentityWcdma, cellIdentity.cellIdentityWcdma);
        break;
    case RIL_CELL_INFO_TYPE_LTE:
        convertCellIdentityLte(out.cellIdentityLte, cellIdentity.cellIdentityLte);
        break;
    case RIL_CELL_INFO_TYPE_TD_SCDMA:
        convertCellIdentityTdscdma(out.cellIdentityTdscdma, cellIdentity.cellIdentityTdscdma);
        break;
    case RIL_CELL_INFO_TYPE_CDMA:
        convertCellIdentityCdma(out.cellIdentityCdma, cellIdentity.cellIdentityCdma);
        break;
    default:
        break;
    } // end switch ~
}

void NetworkUtils::convertDataRegistrationStateResult(
        RIL_DataRegistrationStateResponse_V1_2& out,
        RIL_DataRegistrationStateResponse_V1_4& dataRegStateResult)
{
    out.regState = dataRegStateResult.regState;
    out.rat = dataRegStateResult.rat;
    out.reasonDataDenied = dataRegStateResult.reasonDataDenied;
    out.maxDataCalls = dataRegStateResult.maxDataCalls;
    out.cellIdentity = dataRegStateResult.cellIdentity;
}

void NetworkUtils::convertDataRegistrationStateResult(
        RIL_DataRegistrationStateResponse& out,
        RIL_DataRegistrationStateResponse_V1_4& dataRegStateResult)
{
    out.regState = dataRegStateResult.regState;
    out.rat = dataRegStateResult.rat;
    out.reasonDataDenied = dataRegStateResult.reasonDataDenied;
    out.maxDataCalls = dataRegStateResult.maxDataCalls;
    convertCellIdentity(out.cellIdentity, dataRegStateResult.cellIdentity);
}

void NetworkUtils::convertVoiceRegistrationStateResult(
        RIL_VoiceRegistrationStateResponse& out,
        RIL_VoiceRegistrationStateResponse_V1_2& voiceRegStateResult)
{
    out.regState = voiceRegStateResult.regState;
    out.rat = voiceRegStateResult.rat;
    out.cssSupported = voiceRegStateResult.cssSupported;
    out.roamingIndicator = voiceRegStateResult.roamingIndicator;
    out.systemIsInPrl = voiceRegStateResult.systemIsInPrl;
    out.defaultRoamingIndicator = voiceRegStateResult.defaultRoamingIndicator;
    out.reasonForDenial = voiceRegStateResult.reasonForDenial;
    convertCellIdentity(out.cellIdentity, voiceRegStateResult.cellIdentity);
}

void NetworkUtils::convertSignalStrengthResult(RIL_SignalStrength_v10& signalStrength,
        RIL_SignalStrength_V1_4& currentSignalStrength)
{
    signalStrength.GW_SignalStrength.signalStrength =
            currentSignalStrength.WCDMA_SignalStrength.signalStrength;
    if (signalStrength.GW_SignalStrength.signalStrength < 0
        || signalStrength.GW_SignalStrength.signalStrength > 31) {
        signalStrength.GW_SignalStrength.signalStrength =
                currentSignalStrength.GSM_SignalStrength.signalStrength;
    }
    signalStrength.GW_SignalStrength.bitErrorRate =
            currentSignalStrength.WCDMA_SignalStrength.bitErrorRate;
    if (signalStrength.GW_SignalStrength.bitErrorRate < 0
            || signalStrength.GW_SignalStrength.bitErrorRate > 7) {
        signalStrength.GW_SignalStrength.bitErrorRate =
                currentSignalStrength.GSM_SignalStrength.bitErrorRate;
    }

    signalStrength.CDMA_SignalStrength = currentSignalStrength.CDMA_SignalStrength;
    signalStrength.EVDO_SignalStrength = currentSignalStrength.EVDO_SignalStrength;
    signalStrength.LTE_SignalStrength = currentSignalStrength.LTE_SignalStrength;
    signalStrength.TD_SCDMA_SignalStrength.rscp =
            currentSignalStrength.TD_SCDMA_SignalStrength.rscp;
}

void NetworkUtils::convertSignalStrengthResult(RIL_SignalStrength_V1_2& signalStrength,
        RIL_SignalStrength_V1_4& currentSignalStrength)
{
    signalStrength.GSM_SignalStrength = currentSignalStrength.GSM_SignalStrength;
    signalStrength.CDMA_SignalStrength = currentSignalStrength.CDMA_SignalStrength;
    signalStrength.EVDO_SignalStrength = currentSignalStrength.EVDO_SignalStrength;
    signalStrength.LTE_SignalStrength = currentSignalStrength.LTE_SignalStrength;
    signalStrength.TD_SCDMA_SignalStrength.rscp =
            currentSignalStrength.TD_SCDMA_SignalStrength.rscp;
    signalStrength.WCDMA_SignalStrength = currentSignalStrength.WCDMA_SignalStrength;
}

// Cell Info
void NetworkUtils::convertCellInfoGsm(RIL_CellInfoGsm_v12& out, RIL_CellInfoGsm_V1_2& gsm)
{
    convertCellIdentityGsm(out.cellIdentityGsm, gsm.cellIdentityGsm);
    out.signalStrengthGsm = gsm.signalStrengthGsm;
}

void NetworkUtils::convertCellInfoCdma(RIL_CellInfoCdma& out, RIL_CellInfoCdma_V1_2& cdma)
{
    convertCellIdentityCdma(out.cellIdentityCdma, cdma.cellIdentityCdma);
    out.signalStrengthCdma = cdma.signalStrengthCdma;
    out.signalStrengthEvdo = cdma.signalStrengthEvdo;
}

void NetworkUtils::convertCellInfoWcdma(RIL_CellInfoWcdma_v12& out, RIL_CellInfoWcdma_V1_2& wcdma)
{
    convertCellIdentityWcdma(out.cellIdentityWcdma, wcdma.cellIdentityWcdma);
    out.signalStrengthWcdma.signalStrength = wcdma.signalStrengthWcdma.signalStrength;
    out.signalStrengthWcdma.bitErrorRate = wcdma.signalStrengthWcdma.bitErrorRate;
}

void NetworkUtils::convertCellInfoTdscdma(RIL_CellInfoTdscdma& out, RIL_CellInfoTdscdma_V1_2& tdscdma)
{
    convertCellIdentityTdscdma(out.cellIdentityTdscdma, tdscdma.cellIdentityTdscdma);
    out.signalStrengthTdscdma.rscp = tdscdma.signalStrengthTdscdma.rscp;
}

void NetworkUtils::convertCellInfoLte(RIL_CellInfoLte_v12& out, RIL_CellInfoLte_V1_4& lte)
{
    convertCellIdentityLte(out.cellIdentityLte, lte.cellInfo.cellIdentityLte);
    out.signalStrengthLte = lte.cellInfo.signalStrengthLte;
}

void NetworkUtils::convertCellInfoLte(RIL_CellInfoLte_V1_2& out, RIL_CellInfoLte_V1_4& lte)
{
    out.cellIdentityLte = lte.cellInfo.cellIdentityLte;
    out.signalStrengthLte = lte.cellInfo.signalStrengthLte;
}

void NetworkUtils::convertCellInfo(RIL_CellInfo_v12& out, RIL_CellInfo_V1_4& cellInfo)
{
    out.cellInfoType = cellInfo.cellInfoType;
    out.registered = cellInfo.registered;
    out.timeStamp = cellInfo.timeStamp;
    out.timeStampType = cellInfo.timeStampType;

    switch (out.cellInfoType) {
    case RIL_CELL_INFO_TYPE_GSM:
        convertCellInfoGsm(out.CellInfo.gsm, cellInfo.CellInfo.gsm);
        break;
    case RIL_CELL_INFO_TYPE_WCDMA:
        convertCellInfoWcdma(out.CellInfo.wcdma, cellInfo.CellInfo.wcdma);
        break;
    case RIL_CELL_INFO_TYPE_LTE:
        convertCellInfoLte(out.CellInfo.lte, cellInfo.CellInfo.lte);
        break;
    case RIL_CELL_INFO_TYPE_TD_SCDMA:
        convertCellInfoTdscdma(out.CellInfo.tdscdma, cellInfo.CellInfo.tdscdma);
        break;
    case RIL_CELL_INFO_TYPE_CDMA:
        convertCellInfoCdma(out.CellInfo.cdma, cellInfo.CellInfo.cdma);
        break;
    default:
        break;
    } // end switch ~
}

void NetworkUtils::convertCellInfo(RIL_CellInfo_V1_2& out, RIL_CellInfo_V1_4& cellInfo)
{
    out.cellInfoType = cellInfo.cellInfoType;
    out.registered = cellInfo.registered;
    out.timeStamp = cellInfo.timeStamp;
    out.timeStampType = cellInfo.timeStampType;

    switch (out.cellInfoType) {
    case RIL_CELL_INFO_TYPE_GSM:
        out.CellInfo.gsm = cellInfo.CellInfo.gsm;
        break;
    case RIL_CELL_INFO_TYPE_WCDMA:
        out.CellInfo.wcdma = cellInfo.CellInfo.wcdma;
        break;
    case RIL_CELL_INFO_TYPE_LTE:
        convertCellInfoLte(out.CellInfo.lte, cellInfo.CellInfo.lte);
        break;
    case RIL_CELL_INFO_TYPE_TD_SCDMA:
        out.CellInfo.tdscdma = cellInfo.CellInfo.tdscdma;
        break;
    case RIL_CELL_INFO_TYPE_CDMA:
        out.CellInfo.cdma = cellInfo.CellInfo.cdma;
        break;
    default:
        break;
    } // end switch ~
}

void NetworkUtils::convertCellInfoList(list<RIL_CellInfo_v12>& out, list<RIL_CellInfo_V1_4>& cellInfoList)
{
    out.resize(0);
    list<RIL_CellInfo_V1_4>::iterator iter;
    for (iter = cellInfoList.begin(); iter != cellInfoList.end(); iter++) {
        RIL_CellInfo_v12 cellInfo;
        NetworkUtils::convertCellInfo(cellInfo, *iter);
        out.push_back(cellInfo);
    } // end for ~
}

void NetworkUtils::convertCellInfoList(list<RIL_CellInfo_V1_2>& out, list<RIL_CellInfo_V1_4>& cellInfoList)
{
    out.resize(0);
    list<RIL_CellInfo_V1_4>::iterator iter;
    for (iter = cellInfoList.begin(); iter != cellInfoList.end(); iter++) {
        RIL_CellInfo_V1_2 cellInfo;
        NetworkUtils::convertCellInfo(cellInfo, *iter);
        out.push_back(cellInfo);
    } // end for ~
}

void NetworkUtils::convertPhysicalChannelConfig(RIL_PhysicalChannelConfig& out,
        const RIL_PhysicalChannelConfig_V1_4& config)
{
    out.status = (RIL_CellConnectionStatus)config.status;
    out.cellBandwidthDownlink = config.cellBandwidthDownlink;
}

void NetworkUtils::dupPhysicalChannelConfig(RIL_PhysicalChannelConfig_V1_4& out,
        const RIL_PhysicalChannelConfig_V1_4& config)
{
    out = config;
    if (config.len_contextIds > 0 && config.contextIds != NULL) {
        out.contextIds = new int[config.len_contextIds];
        if (out.contextIds != NULL) {
            memcpy(out.contextIds, config.contextIds, sizeof(int) * config.len_contextIds);
        }
        else {
            out.len_contextIds = 0;
        }
    }
}
