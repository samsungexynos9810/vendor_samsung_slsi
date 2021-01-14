/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "rildef.h"

#define NETWORK_TYPE_BITMAP_GSM     (RAF_GPRS | RAF_EDGE | RAF_GSM)
#define NETWORK_TYPE_BITMAP_CDMA    (RAF_IS95A | RAF_1xRTT)
#define NETWORK_TYPE_BITMAP_EVDO    (RAF_EVDO_0 | RAF_EVDO_A | RAF_EVDO_B | RAF_EHRPD)
#define NETWORK_TYPE_BITMAP_HS      (RAF_HSDPA | RAF_HSUPA | RAF_HSPA | RAF_HSPAP)
#define NETWORK_TYPE_BITMAP_WCDMA   (NETWORK_TYPE_BITMAP_HS | RAF_UMTS)
#define NETWORK_TYPE_BITMAP_LTE     (RAF_LTE | RAF_LTE_CA)
#define NETWORK_TYPE_BITMAP_NR      (RAF_NR)
#define NETWORK_TYPE_BITMAP_TDS_CDMA (RAF_TD_SCDMA)

#define NETWORK_TYPE_BITMAP_GSM_WCDMA           (NETWORK_TYPE_BITMAP_GSM | NETWORK_TYPE_BITMAP_WCDMA)
#define NETWORK_TYPE_BITMAP_GSM_ONLY            (NETWORK_TYPE_BITMAP_GSM)
#define NETWORK_TYPE_BITMAP_WCDMA_ONLY          (NETWORK_TYPE_BITMAP_WCDMA)
#define NETWORK_TYPE_BITMAP_GSM_WCDMA_AUTO      (NETWORK_TYPE_BITMAP_GSM | NETWORK_TYPE_BITMAP_WCDMA)
#define NETWORK_TYPE_BITMAP_CDMA_EVDO_AUTO      (NETWORK_TYPE_BITMAP_CDMA | NETWORK_TYPE_BITMAP_EVDO)
#define NETWORK_TYPE_BITMAP_CDMA_ONLY           (NETWORK_TYPE_BITMAP_CDMA)
#define NETWORK_TYPE_BITMAP_EVDO_ONLY           (NETWORK_TYPE_BITMAP_EVDO)
#define NETWORK_TYPE_BITMAP_GSM_WCDMA_CDMA_EVDO_AUTO    (NETWORK_TYPE_BITMAP_GSM_WCDMA_AUTO | NETWORK_TYPE_BITMAP_CDMA_EVDO_AUTO)
#define NETWORK_TYPE_BITMAP_LTE_CDMA_EVDO       (NETWORK_TYPE_BITMAP_LTE | NETWORK_TYPE_BITMAP_CDMA_EVDO_AUTO)
#define NETWORK_TYPE_BITMAP_LTE_GSM_WCDMA       (NETWORK_TYPE_BITMAP_LTE | NETWORK_TYPE_BITMAP_GSM_WCDMA_AUTO)
#define NETWORK_TYPE_BITMAP_LTE_CMDA_EVDO_GSM_WCDMA (NETWORK_TYPE_BITMAP_LTE_GSM_WCDMA | NETWORK_TYPE_BITMAP_LTE_CDMA_EVDO)
#define NETWORK_TYPE_BITMAP_LTE_ONLY            (NETWORK_TYPE_BITMAP_LTE)
#define NETWORK_TYPE_BITMAP_LTE_WCDMA           (NETWORK_TYPE_BITMAP_LTE | NETWORK_TYPE_BITMAP_WCDMA)
#define NETWORK_TYPE_BITMAP_TD_SCDMA_ONLY       (RAF_TD_SCDMA)
#define NETWORK_TYPE_BITMAP_TD_SCDMA_WCDMA      (NETWORK_TYPE_BITMAP_WCDMA | RAF_TD_SCDMA)
#define NETWORK_TYPE_BITMAP_TD_SCDMA_LTE        (NETWORK_TYPE_BITMAP_LTE | RAF_TD_SCDMA)
#define NETWORK_TYPE_BITMAP_TD_SCDMA_GSM        (NETWORK_TYPE_BITMAP_GSM | RAF_TD_SCDMA)
#define NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_LTE    (NETWORK_TYPE_BITMAP_LTE | NETWORK_TYPE_BITMAP_GSM | RAF_TD_SCDMA)
#define NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA  (NETWORK_TYPE_BITMAP_GSM_WCDMA | RAF_TD_SCDMA)
#define NETWORK_TYPE_BITMAP_TD_SCDMA_WCDMA_LTE  (NETWORK_TYPE_BITMAP_LTE_WCDMA | RAF_TD_SCDMA)
#define NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA_LTE  (NETWORK_TYPE_BITMAP_LTE_GSM_WCDMA | RAF_TD_SCDMA)
#define NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO   (NETWORK_TYPE_BITMAP_GSM_WCDMA_CDMA_EVDO_AUTO | RAF_TD_SCDMA)
#define NETWORK_TYPE_BITMAP_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA    (NETWORK_TYPE_BITMAP_LTE_CMDA_EVDO_GSM_WCDMA | RAF_TD_SCDMA)
#define NETWORK_TYPE_BITMAP_NR_ONLY                 (RAF_NR)
#define NETWORK_TYPE_BITMAP_NR_LTE_CDMA_EVDO_AUTO   (NETWORK_TYPE_BITMAP_LTE_CDMA_EVDO | RAF_NR)
#define NETWORK_TYPE_BITMAP_NR_LTE_CDMA_GSM_WCDMA   (NETWORK_TYPE_BITMAP_LTE_GSM_WCDMA | RAF_NR)
#define NETWORK_TYPE_BITMAP_NR_LTE_CMDA_EVDO_GSM_WCDMA  (NETWORK_TYPE_BITMAP_LTE_CMDA_EVDO_GSM_WCDMA | RAF_NR)
#define NETWORK_TYPE_BITMAP_NR_LTE                  (NETWORK_TYPE_BITMAP_LTE_ONLY | RAF_NR)
#define NETWORK_TYPE_BITMAP_NR_LTE_WCDMA            (NETWORK_TYPE_BITMAP_LTE_WCDMA | RAF_NR)
#define NETWORK_TYPE_BITMAP_NR_TD_SCDMA_LTE         (NETWORK_TYPE_BITMAP_TD_SCDMA_LTE | RAF_NR)
#define NETWORK_TYPE_BITMAP_NR_TD_SCDMA_LTE_GSM     (NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_LTE | RAF_NR)
#define NETWORK_TYPE_BITMAP_NR_TD_SCDMA_WCDMA_LTE   (NETWORK_TYPE_BITMAP_TD_SCDMA_WCDMA_LTE | RAF_NR)
#define NETWORK_TYPE_BITMAP_NR_TD_SCDMA_GSM_WCDMA_LTE   (NETWORK_TYPE_BITMAP_TD_SCDMA_GSM_WCDMA_LTE | RAF_NR)
#define NETWORK_TYPE_BITMAP_NR_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA (NETWORK_TYPE_BITMAP_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA | RAF_NR)

class NetworkUtils {
public:
    // Radio Tech. and RadioAccessFamily
    static int getCellInfoTypeRadioTechnology(int rat);
    static int getNetworkTypeFromRaf(int networkTypeBitmap);
    static int getRafFromNetworkType(int networkType);
    static int getRadioTechnologyFromAccessNetwork(int accessNetwork);
    static int getRadioTechnologyToAccessNetworkType(int rat);
    static const char *getRadioTechnologyString(int rat);
    static const char *getRegStateString(int regState);
    static const char *getAccessNewtorkString(int accessNetwork);
    static bool isInService(int regState);
    static bool isEmergencyOnly(int regState);
    static void printSupportedNetworkTypeBitmap();

    // CellIdentity
    static int fetchMcc(const char *plmn);
    static int fetchMnc(const char *plmn);
    static void convertCellIdentityGsm(RIL_CellIdentityGsm_v12& out,
            RIL_CellIdentityGsm_V1_2& cellIdentityGsm);
    static void convertCellIdentityCdma(RIL_CellIdentityCdma& out,
            RIL_CellIdentityCdma_V1_2& cellIdentityCdma);
    static void convertCellIdentityWcdma(RIL_CellIdentityWcdma_v12& out,
            RIL_CellIdentityWcdma_V1_2& cellIdentityWcdma);
    static void convertCellIdentityLte(RIL_CellIdentityLte_v12& out,
            RIL_CellIdentityLte_V1_2& cellIdentityLte);
    static void convertCellIdentityTdscdma(RIL_CellIdentityTdscdma& out,
            RIL_CellIdentityTdscdma_V1_2& cellIdentityTdscdma);
    static void convertCellIdentity(RIL_CellIdentity_v16& out, RIL_CellIdentity_V1_2& cellIdentity);
    static void convertDataRegistrationStateResult(
            RIL_DataRegistrationStateResponse_V1_2& out,
            RIL_DataRegistrationStateResponse_V1_4& dataRegStateResult);
    static void convertDataRegistrationStateResult(
            RIL_DataRegistrationStateResponse& out,
            RIL_DataRegistrationStateResponse_V1_4& dataRegStateResult);
    static void convertVoiceRegistrationStateResult(
            RIL_VoiceRegistrationStateResponse& out,
            RIL_VoiceRegistrationStateResponse_V1_2& voiceRegStateResult);

    // Signal strength
    static void convertSignalStrengthResult(RIL_SignalStrength_v10& signalStrength,
            RIL_SignalStrength_V1_4& currentSignalStrength);
    static void convertSignalStrengthResult(RIL_SignalStrength_V1_2& signalStrength,
            RIL_SignalStrength_V1_4& currentSignalStrength);

    // Cell Info
    static void convertCellInfoGsm(RIL_CellInfoGsm_v12& out, RIL_CellInfoGsm_V1_2& gsm);
    static void convertCellInfoCdma(RIL_CellInfoCdma& out, RIL_CellInfoCdma_V1_2& cdma);
    static void convertCellInfoWcdma(RIL_CellInfoWcdma_v12& out, RIL_CellInfoWcdma_V1_2& wcdma);
    static void convertCellInfoTdscdma(RIL_CellInfoTdscdma& out, RIL_CellInfoTdscdma_V1_2& tdscdma);
    static void convertCellInfoLte(RIL_CellInfoLte_v12& out, RIL_CellInfoLte_V1_4& lte);
    static void convertCellInfoLte(RIL_CellInfoLte_V1_2& out, RIL_CellInfoLte_V1_4& lte);
    static void convertCellInfo(RIL_CellInfo_v12& out, RIL_CellInfo_V1_4& cellInfo);
    static void convertCellInfo(RIL_CellInfo_V1_2& out, RIL_CellInfo_V1_4& cellInfo);
    static void convertCellInfoList(list<RIL_CellInfo_v12>& out, list<RIL_CellInfo_V1_4>& cellInfoList);
    static void convertCellInfoList(list<RIL_CellInfo_V1_2>& out, list<RIL_CellInfo_V1_4>& cellInfoList);

    // Physical channel config
    static void convertPhysicalChannelConfig(RIL_PhysicalChannelConfig& out, const RIL_PhysicalChannelConfig_V1_4& config);
    static void dupPhysicalChannelConfig(RIL_PhysicalChannelConfig_V1_4& out, const RIL_PhysicalChannelConfig_V1_4& config);
};
