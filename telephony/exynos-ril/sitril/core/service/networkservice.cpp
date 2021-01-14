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
 * networkservice.cpp
 *
 *  Created on: 2014. 6. 23.
 *      Author: sungwoo48.choi
 */
#include "networkservice.h"
#include "netdata.h"
#include "servicemgr.h"
#include "servicemonitorrunnable.h"
#include "servicestate.h"
#include "rillog.h"
#include "protocolnetadapter.h"
#include "protocolnetbuilder.h"
#include "netdatabuilder.h"
#include "operatortable.h"
#include "rilparser.h"
#include "oemreqdata.h"
#include "util.h"
#include "customproductfeature.h"
#include "rilproperty.h"
#include "mcctable.h"
#include "rilapplication.h"
#include <sstream>
#include "EccListLoader.h"
#include "callreqdata.h"
#include "cscservice.h"
#include "imsservice.h"

#define TIMEOUT_NET_5SEC    5000

#define REASON_QUERY_PS_DOMAIN_STATE                    "reason_query_ps_domain_state"
#define REASON_ALLOW_DATA                               "reason_allow_data"
#define REASON_SET_DUAL_NETWORK_TYPE_AND_ALLOW_DATA     "reason_set_dual_network_type_and_allow_data"
#define REASON_SET_DS_NETWORK_TYPE                      "reason_set_ds_network_type"

#define NET_SIM_EF_PNN    (0x6fc5)
#define NET_SIM_EF_OPL    (0x6fc6)

#define VALID_PLMN      "11111"
#define INVALID_PLMN    "00000"
#define PREFIX_ICCID    "ICCID_"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_NET, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define writeRilEvent(format1, format2, ...)   CRilEventLog::writeRilEvent(RIL_LOG_CAT_NET, format1, format2, ##__VA_ARGS__)

#define ENABLE_SET_RC_COMMAND (0)  /* TODO: enable when SIT is implemented */

#define TEST_CARD_IMSI "00101"

NetworkService::NetworkService(RilContext* pRilContext)
    : Service(pRilContext, RIL_SERVICE_NETWORK)
{
    m_radioState = RADIO_STATE_UNAVAILABLE;
    m_desiredRadioState = RADIO_STATE_UNAVAILABLE;
    mDelayedRadioPower = false;
    m_nRadioReady = RADIO_NOT_READY;
    m_nVoiceRat = RADIO_TECH_UNKNOWN;
    m_nDataRat = RADIO_TECH_UNKNOWN;
    m_nDataRegState = NOT_REGISTERED;
    m_cardState = RIL_CARDSTATE_ABSENT;
    m_appState = RIL_APPSTATE_UNKNOWN;
    mCurrentSim = 0;

    m_bShutdown = false;
    m_nRegState = NOT_REGISTERED;

    m_rcVersion = RIL_RADIO_CAPABILITY_VERSION;
    m_rcSession = -1;
    m_rcPhase = RC_PHASE_CONFIGURED;
    m_rcRaf = RAF_UNKNOWN;
    m_rcStatus = RC_STATUS_NONE;
    memset(m_rcModemuuid, 0, sizeof(m_rcModemuuid));

    m_cdmaRoamingType = -1;
    mIsWfcEnabled = false;

    m_emcInfoListFromDatabase.clear();
    memset(m_emergencyNumberList, 0x00, sizeof(m_emergencyNumberList));

    memset(&mNrPhysicalChannelConfigs, 0, sizeof(mNrPhysicalChannelConfigs));
    mNrPhysicalChannelConfigs.rat = 0;
    mNrPhysicalChannelConfigs.status = NONE;

    mIsNrTestMode = SystemProperty::GetInt("persist.vendor.radio.nr_test", 0) == 1;
    RilLogV("[%s] mIsNrTestMode=%d", GetServiceName(), mIsNrTestMode);
}

NetworkService::~NetworkService()
{
}

int NetworkService::OnCreate(RilContext *pRilContext)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);

    return 0;
}

void NetworkService::OnStart()
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
}

BOOL NetworkService::OnHandleRequest(Message* pMsg)
{
    int ret = -1;
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    case MSG_NET_VOICE_REGISTRATION_STATE:
        ret = DoVoiceRegistrationState(pMsg);
        break;
    case MSG_NET_DATA_REGISTRATION_STATE:
        ret = DoDataRegistrationState(pMsg);
        break;
    case MSG_NET_OPERATOR:
        ret = DoOperator(pMsg);
        break;
    case MSG_NET_RADIO_POWER:
        ret = DoRadioPower(pMsg);
        break;
    case MSG_NET_GET_RADIO_STATE:
        ret = DoGetRadioState(pMsg);
        break;
    case MSG_NET_QUERY_NETWORK_SELECTION_MODE:
        ret = DoQueryNetworkSelectionMode(pMsg);
        break;
    case MSG_NET_SET_NETWORK_SELECTION_AUTO:
        ret = DoSetNetworkSelectionAuto(pMsg);
        break;
    case MSG_NET_SET_NETWORK_SELECTION_MANUAL:
        ret = DoSetNetworkSelectionManual(pMsg);
        break;
    case MSG_NET_SET_NETWORK_SELECTION_MANUAL_WITH_RAT:
        ret = DoSetNetworkSelectionManualWithRat(pMsg);
        break;
    case MSG_NET_QUERY_AVAILABLE_NETWORKS:
        ret = DoQueryAvailableNetwork(pMsg);
        break;
    case MSG_NET_QUERY_BPLMN_SEARCH:
        ret = DoQueryBplmnSearch(pMsg);
        break;
    case MSG_NET_SET_PREF_NETWORK_TYPE:
        ret = DoSetPreferredNetworkType(pMsg);
        break;
    case MSG_NET_GET_PREF_NETWORK_TYPE:
        ret = DoGetPreferredNetworkType(pMsg);
        break;
    case MSG_NET_QUERY_AVAILABLE_BAND_MODE:
        ret = DoQueryAvailableBandMode(pMsg);
        break;
    case MSG_NET_SET_BAND_MODE:
        ret = DoSetBandMode(pMsg);
        break;
    case MSG_NET_VOICE_RADIO_TECH:
        ret = DoVoiceRadioTech(pMsg);
        break;
    case MSG_NET_GET_CELL_INFO_LIST:
        ret = DoGetCellInfoList(pMsg);
        break;
    case MSG_NET_SET_UNSOL_CELL_INFO_LIST_RATE:
        ret = DoSetUnsolCellInfoListRate(pMsg);
        break;
    case MSG_NET_ALLOW_DATA:
        ret = DoAllowData(pMsg);
        break;
    case MSG_NET_SHUTDOWN:
        ret = DoShutdown(pMsg);
        break;
    case MSG_NET_SET_UPLMN:
        ret = DoSetUplmn(pMsg);
        break;
    case MSG_NET_GET_UPLMN:
        ret = DoGetUplmn(pMsg);
        break;
    case MSG_NET_SET_DS_NTW_TYPE:
        ret = DoSetDSNetworkType(pMsg);
        break;
    case MSG_NET_GET_RC_NTW_TYPE:
        ret = DoGetRCNetworkType(pMsg);
        break;
    case MSG_NET_SET_RC_NTW_TYPE:
        ret = DoSetRCNetworkType(pMsg);
        break;
    case MSG_NET_QUERY_AVAILABLE_EMERGENCY_CALL_STATUS:
        ret = DoQueryEmergencyCallAvailableRadioTech(pMsg);
        break;
    case MSG_NET_SET_EMERGENCY_CALL_STATUS:
        ret = DoSetEmergencyCallStatus(pMsg);
        break;
    case MSG_NET_OEM_SET_PS_SERVICE:
        ret = DoOemSetPsService(pMsg);
        break;
    case MSG_NET_OEM_GET_PS_SERVICE:
        ret = DoOemGetPsService(pMsg);
        break;
    case MSG_NET_OEM_GET_IMS_SUPPORT_SERVICE:
        ret = DoOemGetImsSupportService(pMsg);
        break;
    case MSG_NET_GET_DUPLEX_MODE:
        ret = DoGetDuplexMode(pMsg);
        break;
    case MSG_NET_SET_DUPLEX_MODE:
        ret = DoSetDuplexMode(pMsg);
        break;
    case MSG_NET_SET_MC_SRCH:
        ret = DoSetMicroCellSrch(pMsg);
        break;
#ifdef SUPPORT_CDMA
    case MSG_NET_CDMA_SET_ROAMING:
        ret = DoCdmaSetRoaming(pMsg);
        break;
    case MSG_NET_CDMA_QUERY_ROAMING:
        ret = DoCdmaQueryRoaming(pMsg);
        break;
    case MSG_NET_SET_CDMA_HYBRID_MODE:
        ret = DoSetCdmaHybridMode(pMsg);
        break;
    case MSG_NET_GET_CDMA_HYBRID_MODE:
        ret = DoGetCdmaHybridMode(pMsg);
        break;
#endif
    case MSG_NET_SET_DUAL_NTW_AND_ALLOW_DATA:
        ret = DoSetDualNetworkTypeAndAllowData(pMsg);
        break;
    case MSG_NET_START_NETWORK_SCAN:
        ret = DoStartNetworkScan(pMsg);
        break;
    case MSG_NET_STOP_NETWORK_SCAN:
        ret = DoStopNetworkScan(pMsg);
        break;
    case MSG_NET_OEM_GET_MANUAL_RAT_MODE:
        ret = DoGetManualRatMode(pMsg);
        break;
    case MSG_NET_OEM_SET_MANUAL_RAT_MODE:
        ret = DoSetManualRatMode(pMsg);
        break;
    case MSG_NET_OEM_GET_FREQUENCY_LOCK:
        ret = DoGetFrequencyLock(pMsg);
        break;
    case MSG_NET_OEM_SET_FREQUENCY_LOCK:
        ret = DoSetFrequencyLock(pMsg);
        break;
    case MSG_NET_OEM_SET_ENDC_MODE:
        ret = DoSetEndcMode(pMsg);
        break;
    case MSG_NET_OEM_GET_ENDC_MODE:
        ret = DoGetEndcMode(pMsg);
        break;
    case MSG_NET_GET_FREQUENCY_INFO:
        ret = DoGetFrequencyInfo(pMsg);
        break;

    default:
        // TODO log unsupported message id
        return FALSE;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL NetworkService::OnHandleSolicitedResponse(Message* pMsg)
{
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return FALSE;
    }

    int ret = -1;

    switch (pMsg->GetMsgId()) {
    case MSG_NET_VOICE_REGISTRATION_STATE_DONE:
        ret = OnVoiceRegistrationStateDone(pMsg);
        break;
    case MSG_NET_DATA_REGISTRATION_STATE_DONE:
        ret = OnDataRegistrationStateDone(pMsg);
        break;
    case MSG_NET_OPERATOR_DONE:
        ret = OnOperatorDone(pMsg);
        break;
    case MSG_NET_RADIO_POWER_DONE:
        ret = OnRadioPowerDone(pMsg);
        break;
    case MSG_NET_GET_RADIO_STATE_DONE:
        ret = OnGetRadioStateDone(pMsg);
        break;
    case MSG_NET_QUERY_NETWORK_SELECTION_MODE_DONE:
        ret = OnQueryNetworkSelectionModeDone(pMsg);
        break;
    case MSG_NET_SET_NETWORK_SELECTION_AUTO_DONE:
        ret = OnSetNetworkSelectionAutoDone(pMsg);
        break;
    case MSG_NET_SET_NETWORK_SELECTION_MANUAL_DONE:
        ret = OnSetNetworkSelectionManualDone(pMsg);
        break;
    case MSG_NET_QUERY_AVAILABLE_NETWORKS_DONE:
        ret = OnQueryAvailableNetworkDone(pMsg);
        break;
    case MSG_NET_QUERY_BPLMN_SEARCH_DONE:
        ret = OnQueryBplmnSearchDone(pMsg);
        break;
    case MSG_NET_SET_PREF_NETWORK_TYPE_DONE:
        ret = OnSetPreferredNetworkTypeDone(pMsg);
        break;
    case MSG_NET_GET_PREF_NETWORK_TYPE_DONE:
        ret = OnGetPreferredNetworkTypeDone(pMsg);
        break;
    case MSG_NET_QUERY_AVAILABLE_BAND_MODE_DONE:
        ret = OnQueryAvailableBandModeDone(pMsg);
        break;
    case MSG_NET_SET_BAND_MODE_DONE:
        ret = OnSetBandModeDone(pMsg);
        break;
    case MSG_NET_GET_CELL_INFO_LIST_DONE:
        ret = OnGetCellInfoListDone(pMsg);
        break;
    case MSG_NET_SET_UNSOL_CELL_INFO_LIST_RATE_DONE:
        ret = OnSetUnsolCellInfoListRateDone(pMsg);
        break;
    case MSG_NET_ALLOW_DATA_DONE:
        ret = OnAllowDataDone(pMsg);
        break;
    case MSG_NET_SHUTDOWN_DONE:
        ret = OnShutdownDone(pMsg);
        break;
    case MSG_NET_SET_UPLMN_DONE:
        ret = OnSetUplmnDone(pMsg);
        break;
    case MSG_NET_GET_UPLMN_DONE:
        ret = OnGetUplmnDone(pMsg);
        break;
    case MSG_NET_SET_DS_NTW_TYPE_DONE:
        ret = OnSetDSNetworkTypeDone(pMsg);
        break;
    case MSG_NET_SET_RC_NTW_TYPE_DONE:
        ret = OnSetRCNetworkTypeDone(pMsg);
        break;
    case MSG_NET_GET_RC_NTW_TYPE_DONE:
        ret = OnGetRCNetworkTypeDone(pMsg);
        break;
    case MSG_NET_SET_EMERGENCY_CALL_STATUS_DONE:
        ret = OnSetEmergencyCallStatusDone(pMsg);
        break;
    case MSG_NET_OEM_SET_PS_SERVICE_DONE:
        ret = OnOemSetPsServiceDone(pMsg);
        break;
    case MSG_NET_OEM_GET_PS_SERVICE_DONE:
        ret = OnOemGetPsServiceDone(pMsg);
        break;
    case MSG_NET_OEM_GET_IMS_SUPPORT_SERVICE_DONE:
        ret = OnOemGetImsSupportServiceDone(pMsg);
        break;
    case MSG_NET_SET_DUPLEX_MODE_DONE:
        ret = OnSetDuplexModeDone(pMsg);
        break;
    case MSG_NET_GET_DUPLEX_MODE_DONE:
        ret = OnGetDuplexModeDone(pMsg);
        break;
    case MSG_NET_SET_MC_SRCH_DONE:
        ret = OnSetMicroCellSrchDone(pMsg);
        break;
#ifdef SUPPORT_CDMA
    case MSG_NET_CDMA_SET_ROAMING_DONE:
        ret = OnCdmaSetRoamingDone(pMsg);
        break;
    case MSG_NET_CDMA_QUERY_ROAMING_DONE:
        ret = OnCdmaQueryRoamingDone(pMsg);
        break;
    case MSG_NET_SET_CDMA_HYBRID_MODE_DONE:
        ret = OnSetCdmaHybridModeDone(pMsg);
        break;
    case MSG_NET_GET_CDMA_HYBRID_MODE_DONE:
        ret = OnGetCdmaHybridModeDone(pMsg);
        break;
#endif
    case MSG_NET_SET_DUAL_NTW_AND_ALLOW_DATA_DONE:
        ret = OnSetDualNetworkTypeAndAllowDataDone(pMsg);
        break;
    case MSG_NET_START_NETWORK_SCAN_DONE:
        ret = OnStartNetworkScanDone(pMsg);
        break;
    case MSG_NET_STOP_NETWORK_SCAN_DONE:
        ret = OnStopNetworkScanDone(pMsg);
        break;
    case MSG_NET_OEM_GET_MANUAL_RAT_MODE_DONE:
        ret = OnGetManualRatModeDone(pMsg);
        break;
    case MSG_NET_OEM_SET_MANUAL_RAT_MODE_DONE:
        ret = OnSetManualRatModeDone(pMsg);
        break;
    case MSG_NET_OEM_GET_FREQUENCY_LOCK_DONE:
        ret = OnGetFrequencyLockDone(pMsg);
        break;
    case MSG_NET_OEM_SET_FREQUENCY_LOCK_DONE:
        ret = OnSetFrequencyLockDone(pMsg);
        break;
    case MSG_NET_OEM_SET_ENDC_MODE_DONE:
        ret = OnSetEndcModeDone(pMsg);
        break;
    case MSG_NET_OEM_GET_ENDC_MODE_DONE:
        ret = OnGetEndcModeDone(pMsg);
        break;
    case MSG_NET_GET_FREQUENCY_INFO_DONE:
        ret = OnGetFrequencyInfoDone(pMsg);
        break;

    default:
        // TODO log unsupported message id
        return FALSE;
    } // end switch ~
    return (ret < 0 ? FALSE : TRUE);
}

BOOL NetworkService::OnHandleUnsolicitedResponse(Message* pMsg)
{
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return FALSE;
    }

    int ret = -1;
    switch (pMsg->GetMsgId()) {
    case MSG_NET_NETWORK_STATE_CHANGED:
        ret = OnNetworkStateChanged(pMsg);
        break;
    case MSG_NET_RADIO_STATE_CHANGED:
        ret = OnRadioStateChanged(pMsg);
        break;
    case MSG_NET_RADIO_READY:
        ret = OnRadioReady(pMsg);
        break;
    case MSG_NET_CELL_INFO_LIST_RECEIVED:
        ret = OnCellInfoListReceived(pMsg);
        break;
    case MSG_NET_RC_INFO_RECV:
        ret = OnRCInfoReceived(pMsg);
        break;
    case MSG_NET_SCG_BEARER_INFO_RECV:
    case MSG_NET_PHYSICAL_CHANNEL_CONFIGS_RECV:
        ret = OnCurrentPhysicalChannelConfigsReceived(pMsg);
        break;
    case MSG_NET_EMERGENCY_ACT_INFO_RECEIVED:
        ret = OnEmergencyActInfoReceived(pMsg);
        break;
    case MSG_NET_IND_TOTAL_OOS:
        ret = OnTotalOosHappened(pMsg);
        break;
    case MSG_NET_IND_MCC:
        ret = OnMccReceived(pMsg);
        break;
    case MSG_NET_IND_NETWORK_SCAN_RESULT:
        ret = OnNetworkScanResultReceived(pMsg);
        break;
    case MSG_NET_IND_SIM_FILE_INFO:
        ret = OnSimFileInfo(pMsg);
        break;
    case MSG_NET_IND_FREQUENCY_INFO:
        ret = OnFrequencyInfo(pMsg);
        break;
    case MSG_NET_IND_B2_B1_CONFIG:
        ret = OnB2B1ConfigInfo(pMsg);
        break;
    case MSG_NET_IND_AC_BARRING_INFO:
        ret = OnAcBarringInfo(pMsg);
        break;

    default:
        // log unsupported message id
        RilLogE("Unsupported Message ID=%04d", pMsg->GetMsgId());
        return FALSE;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

BOOL NetworkService::OnHandleInternalMessage(Message* pMsg)
{
    if (pMsg == NULL) {
        return FALSE;
    }

    switch (pMsg->GetMsgId()) {
    default:
        break;
    }
    return true;
}

BOOL NetworkService::OnHandleRequestTimeout(Message* pMsg)
{
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return FALSE;
    }

    int ret = -1;
    switch (pMsg->GetMsgId()) {
    case MSG_NET_GET_RADIO_STATE:
        ret = OnGetRadioStateTimeout(pMsg);
        break;
    case MSG_NET_QUERY_AVAILABLE_NETWORKS:
        ret = OnQueryAvailableNetworkTimeout(pMsg);
        break;
    case MSG_NET_SET_MC_SRCH:
        ret = OnSetMicroCellSrchTimeout(pMsg);
        break;
    } // end switch ~

    return (ret < 0 ? FALSE : TRUE);
}

void NetworkService::OnModemOnline()
{
    RilLog("[%s] %s ", GetServiceName(), __FUNCTION__);
    if (m_nRadioReady != RADIO_READY) {
        RilLog("[%d] query current radio state from modem.", GetRilSocketId());
        if (GetRilContext() != NULL) {
            GetRilContext()->OnRequest(RIL_REQUEST_OEM_GET_RADIO_STATE, NULL, 0, 0);
        }
    }
}

void NetworkService::OnReset()
{
    RilLog("[%s] %s", GetServiceName(), __FUNCTION__);
    RilLog("[%d] current radioState=%d m_radioState=%d", GetRilSocketId(), GetRadioState(), m_radioState);
    UpdateRadioState(RADIO_STATE_UNAVAILABLE, true);
}

void NetworkService::OnRadioStateChanged(int radioState)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    m_radioState = (RIL_RadioState)radioState;
}

void NetworkService::OnRadioNotAvailable()
{
    RilLog("[%s] %s", GetServiceName(), __FUNCTION__);
    m_cardState = RIL_CARDSTATE_ABSENT;
}

void NetworkService::OnRadioOffOrNotAvailable()
{
    RilLog("[%s] %s", GetServiceName(), __FUNCTION__);

    m_desiredRadioState = RADIO_STATE_UNAVAILABLE;
    m_appState = RIL_APPSTATE_UNKNOWN;
    m_nVoiceRat = RADIO_TECH_UNKNOWN;
    m_nDataRat = RADIO_TECH_UNKNOWN;

    // clear VoLTE and EMC network status
    WriteVolteEmcServiceStatus(false, false, RADIO_TECH_UNKNOWN, NOT_REGISTERED, true);

    // NrPhysicalChannelConfigs
    mNrPhysicalChannelConfigs.rat = RADIO_TECH_UNKNOWN;
    mNrPhysicalChannelConfigs.status = NONE;
}

void NetworkService::OnRadioAvailable()
{
    RilLog("[%s] %s", GetServiceName(), __FUNCTION__);
    if (mDelayedRadioPower) {
        mDelayedRadioPower = false;
        if (m_desiredRadioState != GetRadioState()) {
            if (GetRilContext() != NULL) {
                GetRilContext()->OnRequest(RIL_REQUEST_RADIO_POWER, &m_desiredRadioState, sizeof(m_desiredRadioState), 0);
            }
        }
    }
}

void NetworkService::OnSimStatusChanged(int cardState, int appState)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    // Current USIM is present and
    // any app state -> SIM READY
    if (cardState == RIL_CARDSTATE_PRESENT &&
        m_appState != appState && appState == RIL_APPSTATE_READY) {
        // try to update current PS available state
        QueryCurrentPsDomainState();
    }
    else if (cardState == RIL_CARDSTATE_PRESENT && m_cardState != cardState) {
        // try to update current PS available state
        QueryCurrentPsDomainState();
        resetEmergencyId();
    }
    else if ( cardState == RIL_CARDSTATE_ABSENT ) {
        // RIL handle error for pending commands only when card is removed (e.g. hotswap)
        // ignore no SIM cases at start up
        if ( m_cardState == RIL_CARDSTATE_PRESENT ) {
            if ( m_pCurReqMsg != NULL ) {
                RilLogW("[%s] %s() return error for pending request (%d)", m_szSvcName, __FUNCTION__, m_pCurReqMsg->GetMsgId());
                OnRequestComplete(RIL_E_GENERIC_FAILURE);
            }

            //update Main SIM to invalid
            RilProperty *property = GetRilApplicationProperty();
            if (property != NULL) {
                RilLogV("[%s] Update Main SIM slot to unknown", __FUNCTION__, RIL_APP_MAIN_SIM, RIL_SOCKET_NUM);
                property->Put(RIL_APP_MAIN_SIM, RIL_SOCKET_NUM);
            }

            resetEmergencyId();
        }
        m_EonsResolver.reset();
    }

    m_cardState = cardState;
    m_appState = appState;

    updateEmergencyNumberListFromDb();
}

void NetworkService::OnServiceStateChanged(const ServiceState& state)
{
    // template of OnServiceStateChanged callback
    RilLogV("[%s] %s  ServiceState=%s", GetServiceName(), __FUNCTION__, state.toString().c_str());
}

void NetworkService::UpdateRadioState(RIL_RadioState radioState, bool notifyResult/* = false*/)
{
    switch (radioState) {
    case RADIO_STATE_OFF:
    case RADIO_STATE_UNAVAILABLE:
    case RADIO_STATE_ON:
        RilLogV("New radioState : %d", radioState);
        break;
    default:
        RilLogW("Unknown Radio State : %d", radioState);
        return ;
    }

    if (m_radioState != radioState) {
        RilLogV("UpdateRadioState : %d -> %d", m_radioState, radioState);
        m_radioState = radioState; //real RIL radio state

        ServiceMgr *serviceMgr = GetRilContext()->GetServiceManager();
        if (serviceMgr != NULL) {
            RilDataInts *data = new RilDataInts(1);
            if (data != NULL) {
                data->SetInt(0, (int)radioState);
                serviceMgr->BroadcastSystemMessage(MSG_SYSTEM_RADIO_STATE_CHANGED, data);
            }
        }

        if (notifyResult) {
            RilProperty *pProperty = GetRilContextProperty();
            if (pProperty->GetInt(RIL_CONTEXT_UICC_STATUS, -1) != RIL_CURRENT_SIM_DEACTIVATED) {
                OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, (int *)&m_radioState, sizeof(int));
            }
        }
    }
    else {
        RilLogV("Current Radio State : %d ", radioState);
    }
}

void NetworkService::OnRadioReady()
{
    // set radio ready state to RADIO_READY
    m_nRadioReady = RADIO_READY;
    UpdateRadioState(RADIO_STATE_UNAVAILABLE, true);
}

int NetworkService::OnRadioReady(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    writeRilEvent(m_szSvcName, __FUNCTION__);
    OnRadioReady();
    return 0;
}

int NetworkService::DoRadioPower(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if ( m_bShutdown == true )
    {
        RilLogE("[%s] Under Shutting down, ignore radio power commands", m_szSvcName);
        return -1;
    }

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }
    BOOL desiredPowerState = rildata->GetInt() > 0 ? TRUE : FALSE;

    //SIM deact:
    RilProperty *pProperty = GetRilContextProperty();
    if (m_cardState == RIL_CARDSTATE_PRESENT && !pProperty->GetInt(RIL_CONTEXT_UICC_STATUS, -1)) { //card present && prop=0(deactive)
        OnRequestComplete(RIL_E_SUCCESS);

        //send f/w the desired radio state (instead of real RIL radio state)
        int radioState = desiredPowerState ? RADIO_STATE_ON : RADIO_STATE_OFF;
        OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED, (int*)&radioState, sizeof(int));

        return 0;
    }

    // CP is not ready yet
    bool radioUnavailable = m_nRadioReady != RADIO_READY;
    if (radioUnavailable) {
        RilLogW("Not ready to request Radio Power state");
        mDelayedRadioPower = true;
        OnRequestComplete(RIL_E_RADIO_NOT_AVAILABLE);
        return 0;
    }

    m_desiredRadioState = (desiredPowerState ? RADIO_STATE_ON : RADIO_STATE_OFF);
    RilLogV("m_desiredRadioState is %s", (m_desiredRadioState == RADIO_STATE_ON) ? "RADIO_STATE_ON" : "RADIO_STATE_OFF");
    writeRilEvent(m_szSvcName, __FUNCTION__, "radio power(%s)", m_desiredRadioState == RADIO_STATE_ON ? "ON" : "OFF");

    bool requestRadioPower = ((m_radioState == RADIO_STATE_OFF && desiredPowerState) ||
                            (m_radioState == RADIO_STATE_ON && !desiredPowerState));

    if (!requestRadioPower) {
        OnRequestComplete(RIL_E_SUCCESS);
        return 0;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildRadioPower(desiredPowerState);
    if (SendRequest(pModemData, TIMEOUT_NET_RADIO_POWER, MSG_NET_RADIO_POWER_DONE) < 0) {
        RilLogE("SendRequest error");
        m_desiredRadioState = RADIO_STATE_OFF;
        mDelayedRadioPower = true;
        return -1;
    }

    return 0;
}

int NetworkService::OnRadioPowerDone(Message *pMsg)
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
    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%d)", errorCode);
    OnRequestComplete(errorCode);
    return 0;
}

int NetworkService::DoShutdown(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildShutdown();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_SHUTDOWN_DONE) < 0) {
        return -1;
    }

    m_bShutdown = true;

    return 0;
}

int NetworkService::OnShutdownDone(Message *pMsg)
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

    // Telephony framework doesn't care the result error code
    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);

    // update radio state to RADIO_STATE_UNAVAILABLE by force
    // it's a final message during phone goes to power-off.
    UpdateRadioState(RADIO_STATE_UNAVAILABLE, true);

    return 0;
}

int NetworkService::OnRadioStateChanged(Message *pMsg)
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

    ProtocolRadioStateAdapter adapter(pModemData);
    int radioState = adapter.GetRadioState();
    writeRilEvent(m_szSvcName, __FUNCTION__, "Radio State(%d)", radioState);

    if (radioState >= 0) {
        UpdateRadioState((RIL_RadioState)radioState, true);
    }

    return 0;
}

int NetworkService::DoGetRadioState(Message *pMsg) {
    RilLogI("[%s] %s", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    if (m_nRadioReady == RADIO_READY) {
        OnRequestComplete(RIL_E_SUCCESS, &m_radioState, sizeof(m_radioState));
        return 0;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetRadioState();
    writeRilEvent(m_szSvcName, __FUNCTION__);
    if (SendRequest(pModemData, TIMEOUT_NET_5SEC, MSG_NET_GET_RADIO_STATE_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnGetRadioStateDone(Message *pMsg) {
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolNetRadioStateRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        OnRadioReady();

        int radioState = adapter.GetRadioState();
        UpdateRadioState((RIL_RadioState)radioState, true);
        OnRequestComplete(RIL_E_SUCCESS, &radioState, sizeof(radioState));
    }
    else {
        OnRequestComplete(RIL_E_MODEM_ERR);
    }
    return 0;
}

int NetworkService::OnGetRadioStateTimeout(Message *pMsg) {
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);
    OnRequestComplete(RIL_E_MODEM_ERR);
    return 0;
}

int NetworkService::DoVoiceRegistrationState(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    if ( m_radioState == RADIO_STATE_UNAVAILABLE ) {

        int halVer = HAL_VERSION_CODE(1, 4);
        VoiceRegStateResultBuilder builder(halVer);
        builder.SetRegistrationState(NOT_REGISTERED, RADIO_TECH_UNKNOWN, 0);
        const RilData *rildata = builder.Build();
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_RADIO_NOT_AVAILABLE);
        }
        return 0;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildNetworkRegistrationState(DOMAIN_VOICE_NETWORK);
    writeRilEvent(m_szSvcName, __FUNCTION__);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_VOICE_REGISTRATION_STATE_DONE) < 0) {
        return -1;
    }
    return 0;
}

static const char *ConvertRegStateToString(int regState)
{
    switch (regState) {
    case NOT_REGISTERED:
        return "NOT_REGISTERED";
    case REGISTERED_HOME:
        return "REGISTERED_HOME";
    case SEARCHING:
        return "SEARCHING";
    case DENIED:
        return "DENIED";
    case UNKNOWN:
        return "UNKNOWN";
    case REGISTERED_ROAMING:
        return "REGISTERED_ROAMING";

    case NOT_REGISTERED_EMERGENCY_ONLY:
        return "NOT_REGISTERED_EMERGENCY_ONLY";
    case SEARCHING_EMERGENCY_ONLY:
        return "SEARCHING_EMERGENCY_ONLY";
    case DENIED_EMERGENCY_ONLY:
        return "DENIED_EMERGENCY_ONLY";
    case UNKNOWN_EMERGENCY_ONLY:
        return "UNKNOWN_EMERGENCY_ONLY";

    case DENIED_ROAMING:
        return "DENIED_ROAMING";
    } // end switch ~
    return "<NOT-DEFINED>";
}

static const char *ConvertRadioTechToString(int rat)
{
    switch (rat) {
        case RADIO_TECH_GPRS: return "RADIO_TECH_GPRS";
        case RADIO_TECH_EDGE: return "RADIO_TECH_EDGE";
        case RADIO_TECH_UMTS: return "RADIO_TECH_UMTS";
        case RADIO_TECH_IS95A:  return "RADIO_TECH_IS95A";
        case RADIO_TECH_IS95B:  return "RADIO_TECH_IS95B";
        case RADIO_TECH_1xRTT:  return "RADIO_TECH_1xRTT";
        case RADIO_TECH_EVDO_0: return "RADIO_TECH_EVDO_0";
        case RADIO_TECH_EVDO_A: return "RADIO_TECH_EVDO_A";
        case RADIO_TECH_HSDPA: return "RADIO_TECH_HSDPA";
        case RADIO_TECH_HSUPA: return "RADIO_TECH_HSUPA";
        case RADIO_TECH_HSPA: return "RADIO_TECH_HSPA";
        case RADIO_TECH_EVDO_B: return "RADIO_TECH_EVDO_B";
        case RADIO_TECH_EHRPD: return "RADIO_TECH_EHRPD";
        case RADIO_TECH_LTE: return "RADIO_TECH_LTE";
        case RADIO_TECH_HSPAP: return "RADIO_TECH_HSPAP";
        case RADIO_TECH_GSM: return "RADIO_TECH_GSM";
        case RADIO_TECH_IWLAN: return "RADIO_TECH_IWLAN";
        case RADIO_TECH_TD_SCDMA: return "RADIO_TECH_TD_SCDMA";
        //case RADIO_TECH_HSPADCPLUS: return "RADIO_TECH_HSPADCPLUS";
        case RADIO_TECH_LTE_CA: return "RADIO_TECH_LTE_CA";
        case RADIO_TECH_NR: return "RADIO_TECH_NR";
        default:
            return "RADIO_TECH_UNKNOWN";
    }
}

int NetworkService::OnVoiceRegistrationStateDone(Message *pMsg)
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

    ProtocolNetVoiceRegStateAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int reg = adapter.GetRegState();
        int lac = adapter.GetLAC();
        int cid = adapter.GetCellId();
        int rat = (RIL_RadioTechnology)adapter.GetRadioTech();
        int rejectCause = adapter.GetRejectCause();
        int psc = adapter.GetPSC();
        int tac = adapter.GetTAC();
        int pcid = adapter.GetPCID();
        int eci = adapter.GetECI();
#ifdef SUPPORT_CDMA
        int stationid = adapter.GetStationId();
        int stationidLat = adapter.GetStationLat();
        int stationidLong = adapter.GetStationLong();
        int concur = adapter.GetConCurrent();
        int sid = adapter.GetSystemId();
        int nid = adapter.GetNetworkId();
        int roaming = adapter.GetRoamingInd();
        int regPrl = adapter.GetRegPrl();
        int roaming_prl = adapter.GetRoamingIndPrl();
#endif
        int channel = adapter.getChannelNumber();

        if ((reg == NOT_REGISTERED || reg == SEARCHING)
                && (m_nDataRegState == REGISTERED_HOME || m_nDataRegState == REGISTERED_ROAMING)) {
            RilProperty* propertyImsReg = GetRilContextProperty();
            int imsReg = RIL_IMS_NOT_REGISTERED;
            if (propertyImsReg != NULL) {
                imsReg = propertyImsReg->GetInt(RIL_CONTEXT_IMS_REGISTRATION);
            }

            if (imsReg == RIL_IMS_REGISTERED && m_nDataRat != RADIO_TECH_IWLAN) {
                RilLogV("Change Voice Reg state from [NOT_REGISTERED] to [REGISTERED] due to IMS registration %d", imsReg);
                reg = REGISTERED_HOME;
                rat = m_nDataRat;
            }
        }

        if ( ((reg == REGISTERED_HOME || reg ==  REGISTERED_ROAMING) && m_nVoiceRat != rat)
        || ((m_nRegState != REGISTERED_HOME && m_nRegState != REGISTERED_ROAMING)
                &&(reg == REGISTERED_HOME || reg ==  REGISTERED_ROAMING)) ) {
            OnUnsolicitedResponse(RIL_UNSOL_VOICE_RADIO_TECH_CHANGED, &rat, sizeof(rat));
        }

        if (m_nRegState != reg) {
            m_nRegState = reg;
            mergeAndUpdateEmergencyNumberList();
        }

        if (reg == DENIED) {
            if (rejectCause == NET_REJ_CAUSE_NO_SUITABLE_CELLS_IN_THIS_LOCATION_AREA) {
                reg = NOT_REGISTERED;
                RilLogV("Change Voice Reg state from [denied] to [not_reg] due to reject cause %d", rejectCause);
            } else {
                reg = DENIED_EMERGENCY_ONLY;
            }
        } else if (reg == SEARCHING) {
            reg = SEARCHING_EMERGENCY_ONLY;
        }

        if (reg == NOT_REGISTERED || reg == SEARCHING
                || reg == DENIED || reg == UNKNOWN || reg == DENIED_ROAMING)
            rat = m_nVoiceRat = RADIO_TECH_UNKNOWN;
        else
            m_nVoiceRat = (RIL_RadioTechnology)rat;

        RilLogI("[%s] Voice RegState=%s(0x%02x), RAT=%s(0x%02x)",
            m_szSvcName, ConvertRegStateToString(reg), reg, ConvertRadioTechToString(rat), rat);

        RilLogV("Voice Registration State{Act=%d,Reg=%d,Reject=%d,LAC=%04X,CID=%08X,PSC=%d}",
                rat, reg, rejectCause, lac, cid, psc);
        writeRilEvent(m_szSvcName, __FUNCTION__, "RAT(%d), Reg(%d), Reject(%d), LAC(%d), CID(%d), PSC(%d)", rat, reg, rejectCause, lac, cid, psc);
#ifdef SUPPORT_CDMA
        RilLogV("Voice Registration State{id=%d,idLat=%d,idLong=%d,concur=%d,sid=%d,nid=%d,roaming=%d,regPrl=%d,roamingPrl=%d}",
                stationid, stationidLat, stationidLong, concur, sid, nid, roaming, regPrl, roaming_prl);
        writeRilEvent(m_szSvcName, __FUNCTION__, "id(%d), idLat(%d), idLong(%d), concur(%d), sid(%d), nid(%d), roaming(%d), regPrl(%d), roamingPrl(%d)",
                stationid, stationidLat, stationidLong, concur, sid, nid, roaming, regPrl, roaming_prl);
#endif

        RilProperty* property = GetRilApplicationProperty();
        if ( property != NULL ) {
            int target_op = property->GetInt(RIL_APP_TARGET_OPER);
            RilLogV("[%s] Check target operator=%d", m_szSvcName, target_op);
            if ( TARGET_OPER_ATT == target_op) {
                RilLogI("[%s] ATT specification - request to show notification to user according to rejectCause", m_szSvcName);
                OnVoiceRegistrationCustomNotification(reg, rejectCause);
            }
        }

#ifdef SUPPORT_CDMA
        /// Set system propert to use NID, SID, BSSID, CID
        char sidKeyName[32];
        char nidKeyName[32];
        char cidKeyName[32];
        char bssidKeyName[32];
        char sidValue[16];
        char nidValue[16];
        char cidValue[16];
        char bssidValue[16];

        int socketid = (int)GetRilSocketId();

        snprintf(sidKeyName, sizeof(sidKeyName), "vendor.ril.cdma.sid%d", socketid);
        snprintf(nidKeyName, sizeof(nidKeyName), "vendor.ril.cdma.nid%d", socketid);
        snprintf(cidKeyName, sizeof(nidKeyName), "vendor.ril.gsm.cid%d", socketid);
        snprintf(bssidKeyName, sizeof(bssidKeyName), "vendor.ril.cdma.bssid%d", socketid);
        if (reg == REGISTERED_HOME || reg == REGISTERED_ROAMING) {
            snprintf(sidValue, 16, "%d", sid);
            snprintf(nidValue, 16, "%d", nid);
            snprintf(cidValue, 16, "%d", cid);
            snprintf(bssidValue, 16, "%d", stationid);
        } else {
            snprintf(sidValue, 16, "%d", -1);
            snprintf(nidValue, 16, "%d", -1);
            snprintf(cidValue, 16, "%d", -1);
            snprintf(bssidValue, 16, "%d", -1);
        }
        SystemProperty::Set(sidKeyName, sidValue);
        SystemProperty::Set(nidKeyName, nidValue);
        SystemProperty::Set(bssidKeyName, bssidValue);
        SystemProperty::Set(cidKeyName, cidValue);

#endif

        int halVer = HAL_VERSION_CODE(1, 4);
        VoiceRegStateResultBuilder builder(halVer);
        builder.SetRegistrationState(reg, rat, rejectCause);
        ServiceState ss;
        if (GetRilContext() != NULL) {
            ss = GetRilContext()->GetServiceState();
        }
        builder.SetCellIdentity(ss.getOperatorNumeric().c_str(),
                ss.getOperatorAlphaLong().c_str(),  ss.getOperatorAlphaShort().c_str());
        builder.SetCellIdentity(lac, cid, psc, tac, pcid, eci, channel);
        builder.SetCdmaState(concur, roaming, regPrl, roaming_prl);
        builder.SetCdmaCellIdentity(stationid, stationidLat, stationidLong, sid, nid);
        const RilData *rildata = builder.Build();
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
            return 0;
        }
        else {
            OnRequestComplete(RIL_E_INTERNAL_ERR);
        }
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

void NetworkService::OnVoiceRegistrationCustomNotification(int regState, int rejectCause)
{
    if ( regState == DENIED || regState == DENIED_EMERGENCY_ONLY )
    {
        stringstream ss;
        ss << "broadcast -a com.samsung.slsi.action.REJECT_REG -n com.samsung.slsi.telephony.testmode/.TestModeReceiver";
        ss << " --ei rej_cause ";
        ss << rejectCause;
        string intent = ss.str();

        char szBuff[PROP_VALUE_MAX];
        memset(szBuff, 0x00, sizeof(szBuff));
        sprintf(szBuff, "%d", rejectCause);
        property_set(RIL_NET_REJECT_CAUSE, szBuff);
        RilLogV("send reject cause display ind, (Reject=%d)", rejectCause);
        OnUnsolicitedResponse(RIL_UNSOL_OEM_AM, intent.c_str(), intent.length());
    }
}

int NetworkService::OnNetworkStateChanged(Message *pMsg)
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

    writeRilEvent(m_szSvcName, __FUNCTION__);
    OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
    return 0;
}

int NetworkService::DoDataRegistrationState(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    if ( m_radioState == RADIO_STATE_UNAVAILABLE ) {

        int halVer = RilApplication::RIL_HalVersionCode;
        DataRegStateResultBuilder builder(halVer);
        builder.SetRegistrationState(NOT_REGISTERED, RADIO_TECH_UNKNOWN, 0, 0);
        const RilData *rildata = builder.Build();
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_RADIO_NOT_AVAILABLE);
        }
        return 0;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildNetworkRegistrationState(DOMAIN_DATA_NETWORK);
    writeRilEvent(m_szSvcName, __FUNCTION__);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_DATA_REGISTRATION_STATE_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnDataRegistrationStateDone(Message *pMsg)
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

    ProtocolNetDataRegStateAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int reg = adapter.GetRegState();
        int lac = adapter.GetLAC();
        int cid = adapter.GetCellId();
        int rat = m_nDataRat = (RIL_RadioTechnology)adapter.GetRadioTech();
        int sdc = adapter.GetMaxSDC();
        int rejectCause = adapter.GetRejectCause();
        int psc = adapter.GetPSC();
        int tac = adapter.GetTAC();
        int pcid = adapter.GetPCID();
        int eci = adapter.GetECI();
        int csgid = adapter.GetCSGID();
        int tadv = adapter.GetTADV();
        bool volteAvailable = adapter.IsVolteServiceAvailabe();
        bool emcAvailable = adapter.IsEmergencyCallServiceAvailable();
        int channel = adapter.getChannelNumber();
        // NR (> 1.4)
        bool isEndcAvailable = adapter.IsEndcAvailable();
        bool isDcNrRestricted = adapter.IsDcNrRestricted();
        bool isNrAvailable = adapter.IsNrAvailable();

        // NR test only
        if (mIsNrTestMode) {
            int val = SystemProperty::GetInt("persist.vendor.radio.nr_avail", 0);
            RilLogV("persist.vendor.radio.nr_avail [%d]", val);
            isNrAvailable = (val == 1);

            val = SystemProperty::GetInt("persist.vendor.radio.dcnr_restrict", 0);
            RilLogV("persist.vendor.radio.dcnr_restrict [%d]", val);
            isDcNrRestricted = (val == 1);

            val = SystemProperty::GetInt("persist.vendor.radio.endc_avail", 0);
            RilLogV("persist.vendor.radio.endc_avail [%d]", val);
            isEndcAvailable = (val == 1);
        }

        char szProp[64]  ={0,};
        memset(szProp, 0x00, sizeof(szProp));
        sprintf(szProp, "%s%d", RIL_NET_DINIED_ROAM, (int)GetRilSocketId());
        if ( DENIED_ROAMING == reg ) {
            RilLogV("Reg state : denied_roaming");
            property_set(szProp, "2");   // 0:N/A 1: CS denied roam, 2: PS denied roam
            reg = DENIED;
        }
        else {
            property_set(szProp, "0");   // 0:N/A 1: CS denied roam, 2: PS denied roam
        }

        if (reg == DENIED) {
            if (rejectCause == NET_REJ_CAUSE_NO_SUITABLE_CELLS_IN_THIS_LOCATION_AREA) {
                reg = NOT_REGISTERED;
                RilLogV("Change Voice Reg state from [denied] to [not_reg] due to reject cause %d", rejectCause);
            } else {
                reg = DENIED_EMERGENCY_ONLY;
            }
        }

        m_nDataRegState = reg;

        // update VoLTE and EMC network status
        WriteVolteEmcServiceStatus(volteAvailable, emcAvailable, rat, reg, true);

        // update IWLAN state using data reg state from modem
        bool isWfcEnabledOld = mIsWfcEnabled;
        mIsWfcEnabled = (rat == RADIO_TECH_IWLAN &&
                         reg == REGISTERED_HOME);
        if (mIsWfcEnabled != isWfcEnabledOld) {
            RilLog("[%d] WfcEnabled state changed %d -> %d", GetRilSocketId(), isWfcEnabledOld, mIsWfcEnabled);
            RilLog("[%d] current radioState=%d", GetRilSocketId(), m_radioState);
            OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
        }

        if (reg == NOT_REGISTERED || reg == SEARCHING || reg == DENIED || reg == UNKNOWN) {
            rat = RADIO_TECH_UNKNOWN;
        }

        RilLogI("[%s] Data RegState=%s(0x%02x), RAT=%s(0x%02x)",
            m_szSvcName, ConvertRegStateToString(reg), reg, ConvertRadioTechToString(rat), rat);

        RilLogV("Data Registration State{Act=%d,Reg=%d,Reject=%d,LAC=%04X,CID=%08X}", rat, reg, rejectCause, lac, cid);
        RilLogV("Data Registration State{TAC=%d,PCID=%d,ECI=%d,CSGID=%d,TADV=%d}", tac, pcid, eci, csgid, tadv);
        RilLogV("Data Registration State{VoLTE=%d,EMC=%d}", volteAvailable, emcAvailable);

        writeRilEvent(m_szSvcName, __FUNCTION__, "Act=%d,Reg=%d,Reject=%d,LAC=%04X,CID=%08X, VoLTE=%d,EMC=%d", rat, reg, rejectCause, lac, cid, volteAvailable, emcAvailable);

        /// Set System Property for Data registration
        char data_cidKeyName[32];
        char data_cidValue[16];
        int socketid = (int)GetRilSocketId();
        snprintf(data_cidKeyName, sizeof(data_cidKeyName), "vendor.ril.gsm.data_cid%d", socketid);
        if (reg == REGISTERED_HOME || reg == REGISTERED_ROAMING)
            snprintf(data_cidValue, 16, "%d", cid);
        else
            snprintf(data_cidValue, 16, "%d", -1);
        SystemProperty::Set(data_cidKeyName, data_cidValue);

        // UpdateNrIndicators
        RilLog("[%d] Update NrIndicators status (isEndcAvailable=%s,isDcNrRestricted=%s,isNrAvailable=%s)",
                GetRilSocketId(),
                isEndcAvailable ? "true" : "false",
                isDcNrRestricted ? "true" : "false",
                isNrAvailable ? "true" : "false");
        if (!isDcNrRestricted && IsNrScgAdded()) {
            // forcibly update if
            //   DCNR not restricted
            //   SCG already added
            if (!isEndcAvailable) {
                RilLog("  NR SCG already added. isEndcAvailable %s -> true", isEndcAvailable ? "true" : "false");
            }

            if (!isNrAvailable) {
                RilLog("  NR SCG already added. isNrAvailable %s -> true", isNrAvailable ? "true" : "false");
            }
            isEndcAvailable = true;
            isNrAvailable = true;
        }

        int halVer = RilApplication::RIL_HalVersionCode;
        DataRegStateResultBuilder builder(halVer);
        builder.SetRegistrationState(reg, rat, rejectCause, sdc);
        ServiceState ss;
        if (GetRilContext() != NULL) {
            ss = GetRilContext()->GetServiceState();
        }
        builder.SetCellIdentity(ss.getOperatorNumeric().c_str(),
                ss.getOperatorAlphaLong().c_str(),  ss.getOperatorAlphaShort().c_str());
        builder.SetCellIdentity(lac, cid, psc, tac, pcid, eci, channel);
        builder.SetLteVopsInfo(volteAvailable, emcAvailable);
        builder.SetNrIndicators(isEndcAvailable, isDcNrRestricted, isNrAvailable);
        const RilData *rildata = builder.Build();
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_INTERNAL_ERR);
        }
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int NetworkService::DoOperator(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    if ( m_radioState == RADIO_STATE_OFF
        || m_radioState == RADIO_STATE_UNAVAILABLE ) {

        string plmn = "";
        string alphaLong = "";
        string alphaShort = "";

        // update alphaLong/alphaShort using SIM operator numeric and TS.25 table
        // when WFC enabled
        if (mIsWfcEnabled) {
            string numeric = GetSimOperatorNumeric();
            plmn = numeric;
            TS25Record record = GetOperatorName(numeric.c_str());
            if (record.IsValid()) {
                alphaLong = record.ppcin;
                alphaShort = record.networkName;
                RilLog("[%s] %s TS25 record found %s/%s/%s", GetServiceName(), __FUNCTION__,
                        plmn.c_str(), alphaLong.c_str(), alphaShort.c_str());
            }
            else {
                RilLog("[%s] %s No TS25 record for %s", GetServiceName(), __FUNCTION__, numeric.c_str());
            }
        }

        NetworkDataBuilder builder;
        const RilData *rildata = builder.BuildOperatorResponse(plmn.c_str(), alphaLong.c_str(), alphaShort.c_str());
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_RADIO_NOT_AVAILABLE);
        }
        return 0;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildOperator();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_OPERATOR_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnOperatorDone(Message *pMsg)
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

    ProtocolNetOperatorAdapter adapter(pModemData);

    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        string eonsPnnOpl;
        const char *plmn = adapter.GetPlmn();
        const char *longPlmn = adapter.GetLongPlmn();
        const char *shortPlmn = adapter.GetShortPlmn();
        int regState = adapter.GetRegState();
        int lac = adapter.GetLac();
        char simSpn[MAX_FULL_NAME_LEN + 1] = { 0, };
        string iccId = "";

        bool isNitzAvailable = false;

        // used for deciding whether to use SPN as EONS
        bool isUseSpnInRegHome = false;
        bool nitzHasPriorityThanSpn = false;

        RilProperty *property = GetRilContextProperty();
        if (property != NULL) {
             property->Put(RIL_CONTEXT_NET_CURRENT_PLMN, plmn);
             iccId = property->GetString(RIL_CONTEXT_SIM_ICC_ID);
        }

        RilProperty *pRilAppProperty = GetRilApplicationProperty();
        if (pRilAppProperty != NULL && UplmnSelector::IsValidPlmn(plmn) &&
            (strncmp(plmn, INVALID_PLMN, strlen(INVALID_PLMN)))) {
            RilLogV("Operator set the valid plmn=%s", plmn);
            pRilAppProperty->Put(RIL_APP_NET_LAST_VALID_PLMN, plmn);
        }

        OperatorNameProvider *provider = OperatorNameProvider::GetInstance();
        OperatorContentValue *opname = NULL;
        EonsContentsValue *eons = NULL;
        string strSimPlmn = GetSimOperatorNumeric();
        const char *simPlmn = strSimPlmn.c_str();
        const char *srcPlmn = NULL;

        // if reg home, need to use sim plmn insteade of network plmn for searching in EONS cash & DB. (for MVNO case, Network shareing case)
        if(regState == OPERATOR_REG_HOME) {
            srcPlmn = (simPlmn != NULL && simPlmn[0] != 0) ? simPlmn : plmn;
        }
        else {
            srcPlmn = plmn;
        }
        RilLogV("Operator {PLMN=%s, Long PLMN=%s, Short PLMN=%s, RegState=%d, lac=%d, srcPLMN=%s, finalIccId=%s}",
            plmn, longPlmn, shortPlmn, regState, lac, srcPlmn,iccId.c_str());

        if( longPlmn == NULL && shortPlmn != NULL) longPlmn = shortPlmn;
        if( longPlmn != NULL && shortPlmn == NULL) shortPlmn = longPlmn;
        if( longPlmn != NULL && shortPlmn != NULL) isNitzAvailable = true;

        // check NITZ has priority than SPN
        if (isNitzAvailable) {
            nitzHasPriorityThanSpn = MccTable::isNitzHasPriority(simPlmn, plmn);
        }

        // In order to make sure that longPlmn & shortPlmn has string
        if (regState > OPERATOR_REG_DEFAULT && regState <= OPERATOR_REG_UNKNOWN) {
            // EONS cash update when NITZ name string is available. (iccid, network plmm) is the key.
            if(isNitzAvailable && provider != NULL) {
                if(provider->UpdateEons(iccId.c_str(),plmn,longPlmn,shortPlmn)== false ) {
                   if (provider->InsertEons(iccId.c_str(), plmn, longPlmn, shortPlmn) == true) {
                       RilLogV("Operator: insert to cache");
                   };
                }
            }

            // When NITZ name string is not available, find name frome EONS cash and EONS DB.
            if ((!isNitzAvailable) && provider != NULL){
                eons = provider->FindEons(iccId.c_str(), plmn);
                if(eons != NULL) {
                    if( !TextUtils::IsEmpty(eons->GetLongPlmn())){
                        longPlmn = eons->GetLongPlmn();
                        shortPlmn = eons->GetShortPlmn();
                    }
                    else {
                        RilLogV("Operator: need to check EONS cash {longPlmn=%s, shortPlmn=%s}",eons->GetLongPlmn(),eons->GetShortPlmn());
                        if (provider->Contains(srcPlmn)) {
                            opname = provider->Find(srcPlmn);
                            if(opname != NULL) longPlmn = opname->GetLongPlmn();
                            if(opname != NULL) shortPlmn = opname->GetShortPlmn();
                        }
                        else{
                            RilLogV("Operator: RIL DB does not contain srcPlmn");
                        }
                    }
                }
                else {
                    if (provider->Contains(srcPlmn)) {
                        opname = provider->Find(srcPlmn);
                        if(opname != NULL) longPlmn = opname->GetLongPlmn();
                        if(opname != NULL) shortPlmn = opname->GetShortPlmn();
                    }
                    else{
                        RilLogV("Operator: EONS cache & RIL DB does not contain plmn");
                    }
                }
            }

            // If no name string, use network plmn nummeric
            if( plmn != NULL && longPlmn == NULL) longPlmn = plmn;
            if( plmn != NULL && shortPlmn == NULL) shortPlmn = plmn;
        }

        // check whether SPN shall be used or not
        isUseSpnInRegHome = MccTable::isUsingSpnForOperatorNameInRegHome(simPlmn);

        // use SPN when sim plmn and network plmn is same for certian sim PLMN.
        if (isUseSpnInRegHome == true && MccTable::isNeedCheckPlmnMatcingForSpnUsing(simPlmn) == true) {
            isUseSpnInRegHome = TextUtils::Equals(simPlmn, plmn);
        }

        if(regState == OPERATOR_REG_HOME && (!nitzHasPriorityThanSpn) && (isUseSpnInRegHome /*|| MccTable::isMvnoNetwork(simPlmn) > 0*/)) {
            string spn = GetSimSpn(GetRilSocketId());
            int lenSpn = spn.length();
            if(lenSpn > 0) {
                strncpy(simSpn, spn.c_str(), (lenSpn < MAX_FULL_NAME_LEN)? lenSpn : MAX_FULL_NAME_LEN);
                longPlmn = shortPlmn = simSpn;
            }
        }

        // update operator name based on EF_PNN/EF_OPL when IN servce state.
        if (MccTable::IsCarrierUsePnnOplForEons(simPlmn)
                && (regState > OPERATOR_REG_DEFAULT && regState <= OPERATOR_REG_UNKNOWN)) {
            string newPlmn = (plmn == NULL) ? "" : plmn;
            string newSimPlmn = (simPlmn == NULL) ? "" : simPlmn;
            eonsPnnOpl = updateOperatorNameFromEonsResolver(newPlmn, newSimPlmn, lac, EONS_RESOLVER_REG_OPERATOR);
            if(eonsPnnOpl.length() > 0) {
                longPlmn = shortPlmn = eonsPnnOpl.c_str();
            }
        }
        //RilLogV("Final Operator Name (%s, %s) for (%s lac:%d) ", longPlmn, shortPlmn, plmn, lac);

        OperatorContentValue oper = OperatorNameProvider::GetVendorCustomOperatorName(simPlmn, plmn);
        if (!TextUtils::IsEmpty(oper.GetLongPlmn())) {
            longPlmn = oper.GetLongPlmn();
        }

        if (!TextUtils::IsEmpty(oper.GetShortPlmn())) {
            shortPlmn = oper.GetShortPlmn();
        }

        NetworkDataBuilder builder;
        const RilData *rildata = builder.BuildOperatorResponse(plmn, longPlmn, shortPlmn);
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }

        updateEmergencyNumberListFromDb();
    }
    else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int NetworkService::DoQueryNetworkSelectionMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    if ( m_radioState == RADIO_STATE_OFF
        || m_radioState == RADIO_STATE_UNAVAILABLE ) {

        NetworkDataBuilder builder;
        const RilData *rildata = builder.BuildNetSelectModeResponse(0);
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_RADIO_NOT_AVAILABLE);
        }
        return 0;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildQueryNetworkSelectionMode();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_QUERY_NETWORK_SELECTION_MODE_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnQueryNetworkSelectionModeDone(Message *pMsg)
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

    ProtocolNetSelModeAdapter adapter(pModemData);
    int netSelectionMode = adapter.GetNetworkSelectionMode();

    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        NetworkDataBuilder builder;
        const RilData *rildata = builder.BuildNetSelectModeResponse(netSelectionMode);
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int NetworkService::DoSetNetworkSelectionAuto(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RilProperty *pRilProperty = GetRilApplicationProperty();
    // overallCardState
    // bit 0 : SIM1 card state, bit 1 : SIM2 card state
    // 0 : No Sim, 1 : Only SIM1, 2 : Only SIM2, 3 = Dual SIM
    int overallCardState = pRilProperty->GetInt(RIL_SIM_OVERALL_CARDSTATE, 0);
    RilLogV("DoSetNetworkSelectionAuto overallCardState=0x%02X", overallCardState);
    if (overallCardState == 0) {
        OnRequestComplete(RIL_E_OPERATION_NOT_ALLOWED);
        return 0;
    }

/*
    KAN8895-2010 : CP team ask to remove this blocking codes.

    UINT nRet = IsOppsiteStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during other stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }

#ifdef RIL_FEATURE_FUNCTION_CHECK_CURRENT_STACK_BUSY
    nRet = IsCurrentStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }
#endif
*/
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetNetworkSelectionAuto();
    if (SendRequest(pModemData, TIMEOUT_NET_SET_NETWORK_MODE, MSG_NET_SET_NETWORK_SELECTION_AUTO_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnSetNetworkSelectionAutoDone(Message *pMsg)
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

int NetworkService::DoSetNetworkSelectionManual(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    UINT nRet = IsOppsiteStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during other stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }

#ifdef RIL_FEATURE_FUNCTION_CHECK_CURRENT_STACK_BUSY
    nRet = IsCurrentStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }
#endif

    int rat = RADIO_TECH_UNKNOWN;
    const char *plmn = NULL;
    // parameter : PLMN only (AOSP)
    StringRequestData *rildata = (StringRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }
    plmn = rildata->GetString();

    RilLogV("[%s] Select PLMN=%s RAT=%d", m_szSvcName, plmn, rat);
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetNetworkSelectionManual(rat, plmn);
    if (SendRequest(pModemData, TIMEOUT_NET_SET_NETWORK_MODE, MSG_NET_SET_NETWORK_SELECTION_MANUAL_DONE, pMsg) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::DoSetNetworkSelectionManualWithRat(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    UINT nRet = IsOppsiteStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during other stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }

#ifdef RIL_FEATURE_FUNCTION_CHECK_CURRENT_STACK_BUSY
    nRet = IsCurrentStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }
#endif

    int rat = RADIO_TECH_UNKNOWN;
    const char *plmn = NULL;
    // parameter : PLMN or (PLMN + RAT)
    StringsRequestData *rildata = (StringsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }
    plmn = rildata->GetString(0);
    if (rildata->GetStringCount() > 1) {
        if (!TextUtils::IsEmpty(rildata->GetString(1))) {
            rat = atoi(rildata->GetString(1));
        }
    }

    RilLogV("[%s] Select PLMN=%s RAT=%d", m_szSvcName, plmn, rat);
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetNetworkSelectionManual(rat, plmn);
    if (SendRequest(pModemData, TIMEOUT_NET_SET_NETWORK_MODE, MSG_NET_SET_NETWORK_SELECTION_MANUAL_DONE, pMsg) < 0) {
        return -1;
    }

    return 0;
}


int NetworkService::OnSetNetworkSelectionManualDone(Message *pMsg)
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
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS, NULL, 0, pMsg);
    }
    else {
        OnRequestComplete(RIL_E_INVALID_STATE, NULL, 0, pMsg);
    }
    return 0;
}

int NetworkService::DoQueryAvailableNetwork(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    UINT nRet = IsOppsiteStackOccupyRF();
    if ( nRet == OCCUPY_RF_STATUS_PLMN_SEARCH )
    {
        RilLogV("[%s] %s() Cancel opposite PLMN searching", m_szSvcName, __FUNCTION__);
        RequestData *pData = RilParser::CreateRawData(RIL_REQUEST_OEM_CANCEL_AVAILABLE_NETWORKS, 0, NULL, 0);

        if (pData != NULL) {
            Message *msg = Message::ObtainMessage(pData, RIL_SERVICE_MISC, MSG_MISC_OEM_CANCEL_AVAILABLE_NETWORKS);
            if ( m_pRilContext != NULL ) {
                RilContext* pTargetRilContext = m_pRilContext->GetOppositeRilContext();
                if ( pTargetRilContext != NULL ) {
                    if ( pTargetRilContext->GetServiceManager()->SendMessage(msg) < 0) {
                        if (msg) {
                            delete msg;
                        }
                    }
                } else {
                    delete msg;
                }
            } else {
                delete msg;
            }

        }

        UINT nWaitCount = 250;
        //RilLogV("[%s] %s(), waiting opposite PLMN searching end", m_szSvcName, __FUNCTION__);
        do
        {
            nRet = IsOppsiteStackOccupyRF();
            usleep(20);
        }while ( nRet == OCCUPY_RF_STATUS_PLMN_SEARCH && nWaitCount-- > 0 );
    }

    nRet = IsOppsiteStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during other stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }

#ifdef RIL_FEATURE_FUNCTION_CHECK_CURRENT_STACK_BUSY
    nRet = IsCurrentStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }
#endif

    int ran = 0; // default all
    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata != NULL && rildata->GetReqId() == RIL_REQUEST_OEM_GET_AVAILABLE_NETWORKS) {
        ran = rildata->GetInt();
    }
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildQueryAvailableNetwork(ran);
    if (SendRequest(pModemData, TIMEOUT_NET_QUERY_AVAILABLE_NETWORK, MSG_NET_QUERY_AVAILABLE_NETWORKS_DONE, pMsg) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::DoQueryBplmnSearch(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    UINT nRet = IsOppsiteStackOccupyRF();
    if ( nRet == OCCUPY_RF_STATUS_PLMN_SEARCH )
    {
        RilLogV("[%s] %s() Cancel opposite PLMN searching", m_szSvcName, __FUNCTION__);
        RequestData *pData = RilParser::CreateRawData(RIL_REQUEST_OEM_CANCEL_AVAILABLE_NETWORKS, 0, NULL, 0);

        if (pData != NULL) {
            Message *msg = Message::ObtainMessage(pData, RIL_SERVICE_MISC, MSG_MISC_OEM_CANCEL_AVAILABLE_NETWORKS);
            if ( m_pRilContext != NULL ) {
                RilContext* pTargetRilContext = m_pRilContext->GetOppositeRilContext();
                if ( pTargetRilContext != NULL ) {
                    if ( pTargetRilContext->GetServiceManager()->SendMessage(msg) < 0) {
                        if (msg) {
                            delete msg;
                        }
                    }
                } else {
                    delete msg;
                }
            } else {
                delete msg;
            }
        }

        UINT nWaitCount = 250;
        //RilLogV("[%s] %s(), waiting opposite PLMN searching end", m_szSvcName, __FUNCTION__);
        do
        {
            nRet = IsOppsiteStackOccupyRF();
            usleep(20);
        }while ( nRet == OCCUPY_RF_STATUS_PLMN_SEARCH && nWaitCount-- > 0 );
    }

    nRet = IsOppsiteStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during other stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }

#ifdef RIL_FEATURE_FUNCTION_CHECK_CURRENT_STACK_BUSY
    nRet = IsCurrentStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }
#endif

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildQueryAvailableNetwork(0);
    if (SendRequest(pModemData, TIMEOUT_NET_QUERY_AVAILABLE_NETWORK, MSG_NET_QUERY_BPLMN_SEARCH_DONE, pMsg) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnQueryAvailableNetworkDone(Message *pMsg)
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

    ProtocolNetAvailableNetworkAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    if (errorCode == RIL_E_SUCCESS) {
        int count = adapter.GetCount();
        string strSimPlmn = GetSimOperatorNumeric();
        const char *simPlmn = strSimPlmn.c_str();
        char simSpn[MAX_FULL_NAME_LEN + 1] = { 0, };
        string spn = GetSimSpn(GetRilSocketId());
        int lenSpn = spn.length();
        if(lenSpn > 0) {
            strncpy(simSpn, spn.c_str(), (lenSpn < MAX_FULL_NAME_LEN)? lenSpn : MAX_FULL_NAME_LEN);
        }

        NetworkDataAvailableNetworkListBuilder builder;
        for (int i = 0; i < count; i++) {
            NetworkInfo nwkInfo;
            memset(&nwkInfo, 0, sizeof(nwkInfo));
            if (adapter.GetNetwork(nwkInfo, i, simPlmn, simSpn)) {
                // update operator name based on EF_PNN/EF_OPL when IN servce state.
                if (MccTable::IsCarrierUsePnnOplForEons(simPlmn)) {
                    string eonsPnnOpl = updateOperatorNameFromEonsResolver(nwkInfo.plmn, simPlmn, -1, EONS_RESOLVER_AVAILABLE_NETWORK);
                    if(eonsPnnOpl.length() > 0) {
                        strncpy(nwkInfo.longPlmn, eonsPnnOpl.c_str(), MAX_FULL_NAME_LEN);
                        strncpy(nwkInfo.shortPlmn, eonsPnnOpl.c_str(),MAX_SHORT_NAME_LEN);
                        nwkInfo.longPlmn[MAX_FULL_NAME_LEN] = 0;
                        nwkInfo.shortPlmn[MAX_SHORT_NAME_LEN] = 0;
                    }
                }
                builder.AddNetworkInfo(nwkInfo);
                RilLogV("[%s] [%d]NeworkInfo {%s/%s/%s/%s RAT=%d}", m_szSvcName, i,
                        nwkInfo.plmn, nwkInfo.longPlmn, nwkInfo.shortPlmn, nwkInfo.status, nwkInfo.rat);
            }
        } // end for i ~

        const RilData *rildata = builder.Build();
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength(), pMsg);
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_INTERNAL_ERR, NULL, 0, pMsg);
        }
    }
    else {
        OnRequestComplete(RIL_E_MODEM_ERR, NULL, 0, pMsg);
    }
    return 0;
}

int NetworkService::OnQueryBplmnSearchDone(Message *pMsg)
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

    ProtocolNetAvailableNetworkAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    if (errorCode == RIL_E_SUCCESS) {
        int count = adapter.GetCount();
        string strSimPlmn = GetSimOperatorNumeric();
        const char *simPlmn = strSimPlmn.c_str();
        char simSpn[MAX_FULL_NAME_LEN + 1] = { 0, };
        string spn = GetSimSpn(GetRilSocketId());
        int lenSpn = spn.length();
        if(lenSpn > 0) {
            strncpy(simSpn, spn.c_str(), (lenSpn < MAX_FULL_NAME_LEN)? lenSpn : MAX_FULL_NAME_LEN);
        }

        NetworkDataBplmnListBuilder builder;
        for (int i = 0; i < count; i++) {
            NetworkInfo nwkInfo;
            memset(&nwkInfo, 0, sizeof(nwkInfo));
            if (adapter.GetNetwork(nwkInfo, i, simPlmn, simSpn)) {
                // update operator name based on EF_PNN/EF_OPL when IN servce state.
                if (MccTable::IsCarrierUsePnnOplForEons(simPlmn)) {
                    string eonsPnnOpl = updateOperatorNameFromEonsResolver(nwkInfo.plmn, simPlmn, -1, EONS_RESOLVER_AVAILABLE_NETWORK);
                    if(eonsPnnOpl.length() > 0) {
                        strncpy(nwkInfo.longPlmn, eonsPnnOpl.c_str(), MAX_FULL_NAME_LEN);
                        strncpy(nwkInfo.shortPlmn, eonsPnnOpl.c_str(),MAX_SHORT_NAME_LEN);
                        nwkInfo.longPlmn[MAX_FULL_NAME_LEN] = 0;
                        nwkInfo.shortPlmn[MAX_SHORT_NAME_LEN] = 0;
                    }
                }
                builder.AddNetworkInfo(nwkInfo);
                RilLogV("[%s] [%d]NeworkInfo {%s/%s/%s/%s RAT=%d}", m_szSvcName, i,
                        nwkInfo.plmn, nwkInfo.longPlmn, nwkInfo.shortPlmn, nwkInfo.status, nwkInfo.rat);
            }
        } // end for i ~

        const RilData *rildata = builder.Build();
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength(), pMsg);
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_GENERIC_FAILURE, NULL, 0, pMsg);
        }
    }
    else {
        OnRequestComplete(errorCode, NULL, 0, pMsg);
    }
    return 0;
}

int NetworkService::OnQueryAvailableNetworkTimeout(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    // Cancel requested PLMN searching
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildCancelQueryAvailableNetwork();
    if (pModemData != NULL) {
        if (SendRequest(pModemData) < 0) {
            RilLogW("Fail to send Cancel query available networks request ");
        }
        delete pModemData;
    }

    // explicitly error
    OnRequestComplete(RIL_E_GENERIC_FAILURE);
    return 0;
}

int NetworkService::DoSetBandMode(Message *pMsg)
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


    int bandMode = rildata->GetInt();
    RilLogV("Set Bandmode=%d", bandMode);
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetBandMode(bandMode);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_SET_BAND_MODE_DONE) < 0) {
        return -1;
    }

    return 0;
}
int NetworkService::OnSetBandModeDone(Message *pMsg)
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

int NetworkService::DoQueryAvailableBandMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildQueryAvailableBandMode();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_QUERY_AVAILABLE_BAND_MODE_DONE) < 0) {
        OnRequestComplete(RIL_E_INTERNAL_ERR);
        return 0;
    }

    return 0;
}

int NetworkService::OnQueryAvailableBandModeDone(Message *pMsg)
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

    ProtocolNetBandModeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    if (errorCode == RIL_E_SUCCESS) {
        const int *bandMode = adapter.GetAvialableBandMode();
        int count = adapter.GetCount();

        RilLogV("Available Bandmode count=%d", count);

        NetworkDataBuilder builder;
        const RilData *rildata = builder.BuildNetAvailableBandModeResponse(bandMode, count);
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_MODEM_ERR);
        }
    }
    else {
        OnRequestComplete(RIL_E_MODEM_ERR);
    }
    return 0;
}

int NetworkService::DoSetPreferredNetworkType(Message *pMsg)
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

    UINT nRet = IsOppsiteStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during other stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }

#ifdef RIL_FEATURE_FUNCTION_CHECK_CURRENT_STACK_BUSY
    nRet = IsCurrentStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }
#endif

    int netType = rildata->GetInt();
    int halVer = rildata->GetHalVersion();
    if (halVer >= HAL_VERSION_CODE(1, 4)) {
        int netTypeBitmap = netType;
        RilLogV("[%d] NetworkTypeBitmap=0x%02X", GetRilSocketId(), netTypeBitmap);
        netType = NetworkUtils::getNetworkTypeFromRaf(netTypeBitmap);
    }
    RilLog("Preferred Network Type : %d", netType);
    writeRilEvent(m_szSvcName, __FUNCTION__, "Pref NetType(%d)", netType);

#ifdef RIL_FEATURE_BLOCK_PREFERRED_NETWORK_BEFORE_SIMREADY
    if (m_appState != RIL_APPSTATE_READY) {
        RilLogW("(U)SIM application state is not READY! Do not allow Set Preferred network type");
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
        return 0;
    }
#endif

    int reqType = netType;
    if ( CustomProductFeature::SupportOnly5Mode() == true ) {
        reqType = netType;
        netType = FilteroutCdmaNetworkType(netType);
        RilLogI("[%s] %s() requested type : %d, changed type : %d", m_szSvcName, __FUNCTION__, reqType, netType);
    }
    if ( CustomProductFeature::SupportOnlyBasicPreferredNetworkType() == true ) {
        reqType = netType;
        netType = FilteroutExtendedNetworkType(netType);
        RilLogI("[%s] %s() requested type : %d, changed type : %d", m_szSvcName, __FUNCTION__, reqType, netType);
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetPreferredNetworkType(netType);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_SET_PREF_NETWORK_TYPE_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::FilteroutExtendedNetworkType(int reqType)
{
    int retType = PREF_NET_TYPE_GSM_ONLY;

    switch(reqType) {
    case PREF_NET_TYPE_GSM_WCDMA:   //=0; /* GSM/WCDMA (WCDMA preferred) */
    case PREF_NET_TYPE_GSM_ONLY:    //= 1; /* GSM only */
    case PREF_NET_TYPE_WCDMA:   //= 2; /* WCDMA only */
    case PREF_NET_TYPE_GSM_WCDMA_AUTO:  //= 3; /* GSM/WCDMA (auto mode, according to PRL)
    case PREF_NET_TYPE_CDMA_EVDO_AUTO:  //= 4; /* CDMA and EvDo (auto mode, according to PRL)
    case PREF_NET_TYPE_CDMA_ONLY:   //= 5; /* CDMA only */
    case PREF_NET_TYPE_EVDO_ONLY:   //= 6; /* EvDo only */
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:    //= 7; /* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL)
    case PREF_NET_TYPE_LTE_CDMA_EVDO:   //= 8; /* LTE, CDMA and EvDo */
    case PREF_NET_TYPE_LTE_GSM_WCDMA:   //= 9; /* LTE, GSM/WCDMA */
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA: //10; /* LTE, CDMA, EvDo, GSM/WCDMA */
    case PREF_NET_TYPE_LTE_ONLY:    //= 11; /* LTE Only mode. */
    case PREF_NET_TYPE_LTE_WCDMA:   //= 12; /* LTE/WCDMA */
        retType = reqType;
        break;

    // enum value is added from Android N. refer NETWORK_MODE_* in RILConstants.java
    case PREF_NET_TYPE_TD_SCDMA_ONLY:    //= 13; /* TD-SCDMA only */
    case PREF_NET_TYPE_TD_SCDMA_WCDMA:   //= 14; /* TD-SCDMA and WCDMA */
        retType = PREF_NET_TYPE_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_LTE: //= 15; /* TD-SCDMA and LTE */
    case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:   //= 19; /* TD-SCDMA, WCDMA and LTE */
        retType = PREF_NET_TYPE_LTE_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM: //= 16; /* TD-SCDMA and GSM */
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:   //= 18; /* TD-SCDMA, GSM/WCDMA */
        retType = PREF_NET_TYPE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM_LTE: //= 17; /* TD-SCDMA,GSM and LTE */
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:   //= 20; /* TD-SCDMA, GSM/WCDMA and LTE */
        retType = PREF_NET_TYPE_LTE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO: //= 21; /*TD-SCDMA,EvDo,CDMA,GSM/WCDMA*/
        retType = PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO;
        break;
    case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA: //= 22; /* TD-SCDMA/LTE/GSM/WCDMA, CDMA, and EvDo */
        retType = PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA;
        break;

    // Add NR feature
    case PREF_NET_TYPE_NR_ONLY:
    case PREF_NET_TYPE_NR_LTE:
    case PREF_NET_TYPE_NR_LTE_CDMA_EVDO:
    case PREF_NET_TYPE_NR_LTE_GSM_WCDMA:
    case PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
    case PREF_NET_TYPE_NR_LTE_WCDMA:
        retType = reqType;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA:
        retType = PREF_NET_TYPE_NR_LTE;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM:
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA:
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA:
        retType = PREF_NET_TYPE_NR_LTE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
        retType = PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA;
        break;

    case PREF_NET_TYPE_TD_SCDMA_CDMA: //= 50; /* TD-SCDMA , CDMA and EvDo */
    case PREF_NET_TYPE_TD_SCDMA_CDMA_NO_EVDO: //= 51; /* TD-SCDMA , CDMA */
        retType = PREF_NET_TYPE_CDMA_ONLY;
        break;
    case PREF_NET_TYPE_TD_SCDMA_CDMA_EVDO_LTE: //= 52; /* TD-SCDMA , LTE, CDMA and EvDo */
        retType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
        break;
    case PREF_NET_TYPE_TD_SCDMA_EVDO_NO_CDMA: //= 53; /* TD-SCDMA , EVDO */
        retType = PREF_NET_TYPE_EVDO_ONLY;
        break;
    default:
        RilLogI("[%s] %s() no matched reqNetType(%d)", m_szSvcName, __FUNCTION__, reqType);
        break;
    }

    return retType;
}

int NetworkService::FilteroutCdmaNetworkType(int reqType)
{
    int retType = PREF_NET_TYPE_GSM_ONLY;

    switch(reqType) {
    case PREF_NET_TYPE_GSM_WCDMA:   //=0; /* GSM/WCDMA (WCDMA preferred) */
    case PREF_NET_TYPE_GSM_ONLY:    //= 1; /* GSM only */
    case PREF_NET_TYPE_WCDMA:   //= 2; /* WCDMA only */
    case PREF_NET_TYPE_GSM_WCDMA_AUTO:  //= 3; /* GSM/WCDMA (auto mode, according to PRL)
    case PREF_NET_TYPE_LTE_GSM_WCDMA:   //= 9; /* LTE, GSM/WCDMA */
    case PREF_NET_TYPE_LTE_ONLY:    //= 11; /* LTE Only mode. */
    case PREF_NET_TYPE_LTE_WCDMA:   //= 12; /* LTE/WCDMA */
    case PREF_NET_TYPE_TD_SCDMA_ONLY:    //= 13; /* TD-SCDMA only */
    case PREF_NET_TYPE_TD_SCDMA_WCDMA:   //= 14; /* TD-SCDMA and WCDMA */
    case PREF_NET_TYPE_TD_SCDMA_LTE: //= 15; /* TD-SCDMA and LTE */
    case PREF_NET_TYPE_TD_SCDMA_WCDMA_LTE:   //= 19; /* TD-SCDMA, WCDMA and LTE */
    case PREF_NET_TYPE_TD_SCDMA_GSM: //= 16; /* TD-SCDMA and GSM */
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA:   //= 18; /* TD-SCDMA, GSM/WCDMA */
    case PREF_NET_TYPE_TD_SCDMA_GSM_LTE: //= 17; /* TD-SCDMA,GSM and LTE */
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_LTE:   //= 20; /* TD-SCDMA, GSM/WCDMA and LTE */
    case PREF_NET_TYPE_NR_ONLY:
    case PREF_NET_TYPE_NR_LTE:
    case PREF_NET_TYPE_NR_LTE_GSM_WCDMA:
    case PREF_NET_TYPE_NR_LTE_WCDMA:
    case PREF_NET_TYPE_NR_LTE_TDSCDMA:
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM:
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_WCDMA:
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_GSM_WCDMA:
        retType = reqType;
        break;

    case PREF_NET_TYPE_CDMA_EVDO_AUTO:  //= 4; /* CDMA and EvDo (auto mode, according to PRL)
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:    //= 7; /* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL)
    case PREF_NET_TYPE_TD_SCDMA_GSM_WCDMA_CDMA_EVDO_AUTO: //= 21; /*TD-SCDMA,EvDo,CDMA,GSM/WCDMA*/
        retType = PREF_NET_TYPE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_EVDO_ONLY:   //= 6; /* EvDo only */
    case PREF_NET_TYPE_TD_SCDMA_EVDO_NO_CDMA: //= 53; /* TD-SCDMA , EVDO */
        retType = PREF_NET_TYPE_WCDMA;
        break;
    case PREF_NET_TYPE_CDMA_ONLY:   //= 5; /* CDMA only */
    case PREF_NET_TYPE_TD_SCDMA_CDMA: //= 50; /* TD-SCDMA , CDMA and EvDo */
    case PREF_NET_TYPE_TD_SCDMA_CDMA_NO_EVDO: //= 51; /* TD-SCDMA , CDMA */
        retType = PREF_NET_TYPE_GSM_ONLY;
        break;
    case PREF_NET_TYPE_LTE_CDMA_EVDO:   //= 8; /* LTE, CDMA and EvDo */
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA: //10; /* LTE, CDMA, EvDo, GSM/WCDMA */
    case PREF_NET_TYPE_TD_SCDMA_LTE_CDMA_EVDO_GSM_WCDMA: //= 22; /* TD-SCDMA/LTE/GSM/WCDMA, CDMA, and EvDo */
    case PREF_NET_TYPE_TD_SCDMA_CDMA_EVDO_LTE: //= 52; /* TD-SCDMA , LTE, CDMA and EvDo */
        retType = PREF_NET_TYPE_LTE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_NR_LTE_CDMA_EVDO:
    case PREF_NET_TYPE_NR_LTE_CDMA_EVDO_GSM_WCDMA:
    case PREF_NET_TYPE_NR_LTE_TDSCDMA_CDMA_EVDO_GSM_WCDMA:
        retType = PREF_NET_TYPE_NR_LTE_GSM_WCDMA;
        break;

    default:
        RilLogI("[%s] %s() no matched reqNetType(%d)", m_szSvcName, __FUNCTION__, reqType);
        break;
    }

    return retType;
}

int NetworkService::OnSetPreferredNetworkTypeDone(Message *pMsg)
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
    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%s)", errorCode == RIL_E_SUCCESS ? "SUCCESS" : "GENERIC_FAILURE");
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else {
        // VtsHalRadioV1_4Target#RadioHidlTest_v1_4.setPreferredNetworkTypeBitmap
        // an expected error code is always RIL_E_SUCCESS
        RilLogW("[%d] setPreferredNetworkType error by modem", GetRilSocketId());
        OnRequestComplete(RIL_E_SUCCESS);
    }
    return 0;
}

int NetworkService::DoGetPreferredNetworkType(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetPreferredNetworkType();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_GET_PREF_NETWORK_TYPE_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnGetPreferredNetworkTypeDone(Message *pMsg)
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

    ProtocolNetPrefNetTypeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int netType = adapter.GetPreferredNetworkType();
        writeRilEvent(m_szSvcName, __FUNCTION__, "Pref NetType(%d)", netType);
        RilLogV("[%d] Pref NetType(%d)", GetRilSocketId(), netType);

        RequestData *rildata = GetCurrentReqeustData();
        if (rildata != NULL) {
            int halVer = rildata->GetHalVersion();
            if (halVer >= HAL_VERSION_CODE(1, 4)) {
                netType = NetworkUtils::getRafFromNetworkType(netType);
                RilLogV("[%d] NetworkTypeBitmap=0x%02X", GetRilSocketId(), netType);
            }
        }
        OnRequestComplete(RIL_E_SUCCESS, &netType, sizeof(int));
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int NetworkService::DoVoiceRadioTech(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    if (m_nVoiceRat == RADIO_TECH_UNKNOWN ||
        !(m_nRegState == REGISTERED_HOME || m_nRegState == REGISTERED_ROAMING)) {
        RilLogI("[%s] %s() Voice Radio Tech is unknown or not registered", m_szSvcName, __FUNCTION__);
        OnRequestComplete(RIL_E_SUCCESS, &m_nVoiceRat, sizeof(int));
        return 0;
    }

    OnRequestComplete(RIL_E_SUCCESS, &m_nVoiceRat, sizeof(int));

    return 0;
}

int NetworkService::DoGetCellInfoList(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetCellInfoList();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_GET_CELL_INFO_LIST_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnGetCellInfoListDone(Message *pMsg)
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

    ProtocolNetCellInfoListAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        list<RIL_CellInfo_V1_4> &cellInfoList = adapter.GetCellInfoList();

        int halVer = RilApplication::RIL_HalVersionCode;
        CellInfoListBuilder builder(halVer);
        const RilData *rildata = builder.Build(cellInfoList);
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_INTERNAL_ERR);
        }
    }
    else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int NetworkService::OnCellInfoListReceived(Message *pMsg)
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

    ProtocolNetCellInfoListIndAdapter adapter(pModemData);

    int halVer = RilApplication::RIL_HalVersionCode;
    CellInfoListBuilder builder(halVer);
    const RilData *rildata = builder.Build(adapter.GetCellInfoList());
    if (rildata != NULL) {
        OnUnsolicitedResponse(RIL_UNSOL_CELL_INFO_LIST, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }

    return 0;
}

int NetworkService::DoSetUnsolCellInfoListRate(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntRequestData *rildata = static_cast<IntRequestData *>(pMsg->GetRequestData());
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int rate = rildata->GetInt();
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetCellInfoListReportRate(rate);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_SET_UNSOL_CELL_INFO_LIST_RATE_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnSetUnsolCellInfoListRateDone(Message *pMsg)
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
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

int NetworkService::DoAllowData(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntRequestData *rildata = static_cast<IntRequestData *>(pMsg->GetRequestData());
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int state = rildata->GetInt();
    RilLogV("%s", state == ALLOW_DATA_CALL ? "ALLOW_DATA_CALL" : "DISALLOW_DATA_CALL");

    // For dual volte.
    bool isDualVoLTE = false;
    char buf[PROP_VALUE_MAX] = {0, };
    property_get(RIL_VENDOR_RADIO_DUAL_VOLTE, buf, "0");
    if ( strcmp(buf, "1") == 0 ) {
        isDualVoLTE = true;
    }

    writeRilEvent(m_szSvcName, __FUNCTION__, "state(%s)", state == ALLOW_DATA_CALL ? "ALLOW_DATA_CALL" : "DISALLOW_DATA_CALL");

    RilProperty *pRilProperty = GetRilApplicationProperty();
    // overallCardState
    // bit 0 : SIM1 card state, bit 1 : SIM2 card state
    // 0 : No Sim, 1 : Only SIM1, 2 : Only SIM2, 3 = Dual SIM
    int overallCardState = pRilProperty->GetInt(RIL_SIM_OVERALL_CARDSTATE, 0);
    RilLogV("overallCardState=0x%02X", overallCardState);

    // If it supports dual volte, it doesn't have to wait.
    if (state == ALLOW_DATA_CALL && overallCardState == 0x3 && !isDualVoLTE) {
        unsigned int waitTimeMs = 100;    // 100 ms
        stringstream ss;
        ss << RIL_APP_NET_PS_SERVICE_ALLOW << (GetRilSocketId() + 1) % 2;;
        string strPropName = ss.str();
        bool enabled = false;
        int count = (TIMEOUT_NET_ALLOW_DATA / waitTimeMs) + 10;
        RilLogW("[%d] Waiting PS Service available", GetRilSocketId());
        while (count-- > 0) {
            enabled = pRilProperty->GetBool(strPropName);
            if (!enabled) {
                break;
            }
            usleep(waitTimeMs * 1000);
        } // end while ~
        RilLogV("[%d] GetBool(%s)=%d", GetRilSocketId(), strPropName.c_str(), enabled);
    }
    if (state == DISALLOW_DATA_CALL && overallCardState != 0x3) {
        // Just consider as completion. CP will always return fails and no PS reg
        OnRequestComplete(RIL_E_SUCCESS);
        return 0;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildAllowData(state);
    if (SendRequest(pModemData, TIMEOUT_NET_ALLOW_DATA, MSG_NET_ALLOW_DATA_DONE) < 0) {
        return -1;
    }

    // set default data subscription PhoneID
    //update ACTIVE PS data SIM
    RilProperty *property = GetRilApplicationProperty();
    if (property != NULL && (state == ALLOW_DATA_CALL)) {
        RilLogV("[%s] Update Active data SIM slot{%s=%d}", __FUNCTION__, RIL_APP_PS_ACTIVE_SIM, GetRilSocketId());
        property->Put(RIL_APP_PS_ACTIVE_SIM, GetRilSocketId());

        if (m_cardState == RIL_CARDSTATE_PRESENT) {
            RilLogV("[%s] SIM card present: update registration state", m_szSvcName);
            OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
        }
    }

    return 0;
}

int NetworkService::OnAllowDataDone(Message *pMsg)
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
    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%s)", errorCode == RIL_E_SUCCESS ? "SUCCESS" : "GENERIC_FAILURE");
    RilProperty *pRilProperty = GetRilApplicationProperty();
    if (errorCode == RIL_E_SUCCESS) {
        IntRequestData *rildata = (IntRequestData *)GetCurrentMsg()->GetRequestData();
        int state = rildata->GetInt();
        stringstream ss;
        ss << RIL_APP_NET_PS_SERVICE_ALLOW << GetRilSocketId();;
        string strPropName = ss.str();
        pRilProperty->Put(strPropName, (state == ALLOW_DATA_CALL));
        RilLogV("Put(%s, %d)", strPropName.c_str(), state);
        usleep(500000);

        pRilProperty->Put(RIL_APP_PS_ALLOW_SYNCDONE, 1);
        OnRequestComplete(RIL_E_SUCCESS);

        // update allowDataState in system
        SetAllowDataState((state == ALLOW_DATA_CALL), REASON_ALLOW_DATA,  GetRilSocketId());
    }
    else {
        // HAL expects RadioError:NONE at all times, and it expects processing correctly someday.
        // All error from CP should be processed internally, and make it correct at last
        // property->Put(RIL_APP_PS_ACTIVE_SIM, GetRilSocketId()); will be there.
        // Need to retry with that value
        int overallCardState = pRilProperty->GetInt(RIL_SIM_OVERALL_CARDSTATE, 0);
        int isPresent = (overallCardState & (0x01<<GetRilSocketId())) >> GetRilSocketId();
        // ErrorCase 1: Current Socket is not SIM CARD PRESENT
        RilLogV("SOCKET:%d is has card state:%d(overall:%d)", GetRilSocketId(), isPresent, overallCardState);
        // Check Actual DataAllow State from CP
        // Queueing DataAllow request to sync up Framework's request and CP state

        if (m_cardState == RIL_CARDSTATE_PRESENT) {
            RilLogV("[%s] SIM card present and received failed", m_szSvcName);
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }
        else
        {
            RilLogV("[%s] SIM card absent or PIN locked, just ignore", m_szSvcName);
            OnRequestComplete(RIL_E_SUCCESS);
        }
    }
    return 0;
}

int NetworkService::DoSetUplmn(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    StringsRequestData *rildata = static_cast<StringsRequestData *>(pMsg->GetRequestData());
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int mode = UPLMN_MODE_INVALID;
    int index = UPLMN_INDEX_INVALID;
    const char *plmn = NULL;
    int act = UPLMN_ACT_BIT_UNKNOWN;

    if (!TextUtils::IsEmpty(rildata->GetString(0))) {
        mode = atoi(rildata->GetString(0));
    }

    if (!TextUtils::IsEmpty(rildata->GetString(1))) {
        index = atoi(rildata->GetString(1));
    }

    RilLogV("mode=%d index=%d", mode, index);

    // GET operation SHOULD be preceded.
    if (UplmnSelector::IsValidUplmnMode(mode) && UplmnSelector::IsValidIndex(index)) {
        if (mode == UPLMN_MODE_ADD || mode == UPLMN_MODE_EDIT) {
            plmn = rildata->GetString(2);
            if (!TextUtils::IsEmpty(rildata->GetString(3))) {
                act = atoi(rildata->GetString(3));
            }

            RilLogV("PLMN=%s AcT=%d", plmn, act);

            if (!UplmnSelector::IsValidUplmnAct(act)) {
                RilLogW("UPLMN unsupported AcT: act=%d", act);
                return -1;
            }
#if 0
            if (!m_UplmnSelector.Prepared()) {
                RilLogW("UPLMN Selector is not prepared yet. Need to be preceded GET operation");
                return -1;
            }

            bool exist = m_UplmnSelector.IsExists(index);
            if ((mode == UPLMN_MODE_ADD && exist) ||
                (mode == UPLMN_MODE_EDIT && !exist)) {
                RilLogW("Invalid UPLMN operation: mode=%d index=%d existed=%d", mode, index, exist);
                return -1;
            }
#endif
        }
    }
    else {
        RilLogW("Invalid UPLMN operation: mode=%d index=%d", mode, index);
        return -1;
    }
#if 0
    // set transaction
    m_UplmnSelector.SetTransaction(mode, index, plmn, act);
#endif

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetUplmn(mode, index, plmn, act);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_SET_UPLMN_DONE) < 0) {
        RilLogW("Error: Invalid modem data");
        return -1;
    }

    return 0;
}

int NetworkService::OnSetUplmnDone(Message *pMsg)
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
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
#if 0
        // transaction completed
        m_UplmnSelector.Commit();
#endif
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);

        // transaction failure
        m_UplmnSelector.Cancel();
    }

    return 0;
}

int NetworkService::DoGetUplmn(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetUplmn();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_GET_UPLMN_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnGetUplmnDone(Message *pMsg)
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

    ProtocolNetUplmnListAdapter adapter(pModemData);
    NetworkDataUplmnListBuilder builder;
    int size = adapter.GetSize();
    RilLogV("[%s] %s: Preferred PLMN List size: %d", m_szSvcName, __FUNCTION__, size);
    m_UplmnSelector.Reset();
    for (int i = 0; i < size; i++) {
        PreferredPlmn preferredPlmn;
        if (adapter.GetPreferrecPlmn(preferredPlmn, i)) {
            RilLogV("[%d]index=%d,PLMN=%s,AcT=%d", i, preferredPlmn.index, preferredPlmn.plmn, preferredPlmn.act);
#if 0
            // update UplmnSelector
            m_UplmnSelector.Set(preferredPlmn);
#endif
            // add into NetworkDataUplmnListBuilder for response data
            builder.AddPreferredPlmn(preferredPlmn);
        }
    } // end for i ~
    m_UplmnSelector.Prepare();

    const RilData *rildata = builder.Build(size);
    if (rildata != NULL) {
        OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }

    return 0;
}

int NetworkService::DoGetRCNetworkType(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetRCNetworkType();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_GET_RC_NTW_TYPE_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnGetRCNetworkTypeDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolGetNetworkRCRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        RIL_RadioCapability resp;
        resp.version = adapter.GetVersion();
        resp.session = adapter.GetSession();
        resp.phase = adapter.GetPhase();
        resp.rat = adapter.GetRafType();

        memset(resp.logicalModemUuid, 0, sizeof(resp.logicalModemUuid));
        BYTE *pUuid = adapter.GetUuid();
        if (pUuid != NULL) memcpy(resp.logicalModemUuid, pUuid,(strlen((char *)pUuid)<=(MAX_UUID_LENGTH - 1))? strlen((char *)pUuid): MAX_UUID_LENGTH - 1);
        else memcpy(resp.logicalModemUuid, DEFAULT_UUID, (strlen(DEFAULT_UUID)<=(MAX_UUID_LENGTH - 1))? strlen(DEFAULT_UUID): MAX_UUID_LENGTH - 1);

        resp.status = adapter.GetStatus();

#if ENABLE_SET_RC_COMMAND
        // nothing to do
#else
        resp.version = RIL_RADIO_CAPABILITY_VERSION;
        resp.session = 1;
        resp.phase = RC_PHASE_CONFIGURED;
        resp.rat = 0x000FFFFE;
        memcpy(resp.logicalModemUuid, DEFAULT_UUID, (strlen(DEFAULT_UUID)<=(MAX_UUID_LENGTH - 1))? strlen(DEFAULT_UUID): MAX_UUID_LENGTH - 1);
        resp.status = 0;
#endif
        OnRequestComplete(RIL_E_SUCCESS, &resp, sizeof(RIL_RadioCapability));
    }
    else {
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
    }
    return 0;
}

int NetworkService::DoSetRCNetworkType(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    NetRCData *rildata = (NetRCData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    RilProperty *pRilProperty = GetRilApplicationProperty();
    // overallCardState
    // bit 0 : SIM1 card state, bit 1 : SIM2 card state
    // 0 : No Sim, 1 : Only SIM1, 2 : Only SIM2, 3 = Dual SIM
    int overallCardState = pRilProperty->GetInt(RIL_SIM_OVERALL_CARDSTATE, 0);
    RilLogV("DoSetRCNetworkType overallCardState=0x%02X", overallCardState);
    if (overallCardState == 0) {
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
        return 0;
    }


#if ENABLE_SET_RC_COMMAND
    int req_rcSession = rildata->GetSession();
    int req_rcPhase = rildata->GetPhase();

    // state transition check or parameter check if it is needed
    if ( req_rcPhase ==  RC_PHASE_START) {
        m_rcVersion = RIL_RADIO_CAPABILITY_VERSION; //rildata->GetVersion() is dummy in ril_service.cpp
        m_rcSession = req_rcSession;
        m_rcPhase = req_rcPhase;
        m_rcRaf = rildata->GetRat();

        memset(m_rcModemuuid, 0, sizeof(m_rcModemuuid));
        char *pUuid = rildata->GetString();
        if (pUuid != NULL) memcpy(m_rcModemuuid, pUuid, MAX_UUID_LENGTH - 1);
        else memcpy(m_rcModemuuid, DEFAULT_UUID, MAX_UUID_LENGTH - 1);

        m_rcStatus = rildata->GetStatus();
    } else if ( m_rcSession != req_rcSession) {
        RilLogI("Session mismatch: req/ongoing Id(%d, %d), req/ongoing phase(%d, %d)\n",
            req_rcSession, m_rcSession, req_rcPhase, m_rcPhase);
        return -1;
    } else {
        m_rcPhase = req_rcPhase;
        m_rcRaf = rildata->GetRat();
        m_rcStatus = rildata->GetStatus();
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetRCNetworkType(m_rcVersion, m_rcSession, m_rcPhase,
                        m_rcRaf, m_rcModemuuid, m_rcStatus);

    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_SET_RC_NTW_TYPE_DONE) < 0) {
        return -1;
    }
#else
    OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
#endif
    return 0;
}

int NetworkService::OnSetRCNetworkTypeDone(Message *pMsg)
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

    ProtocolSetNetworkRCRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        RIL_RadioCapability resp;
        resp.version = m_rcVersion;
        resp.session = m_rcSession;
        resp.phase = m_rcPhase;
        resp.rat = m_rcRaf;
        memset(resp.logicalModemUuid, 0, sizeof(resp.logicalModemUuid));
        memcpy(resp.logicalModemUuid, m_rcModemuuid, MAX_UUID_LENGTH - 1);
        resp.status = m_rcStatus;

        OnRequestComplete(RIL_E_SUCCESS, &resp, sizeof(RIL_RadioCapability));
    }
    else {
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
    }

    // If the FINISH state, reset the variable
    if (m_rcPhase == RC_PHASE_FINISH) {
        m_rcSession = -1;
        m_rcPhase = RC_PHASE_CONFIGURED;
    }

    return 0;
}

int NetworkService::OnRCInfoReceived(Message *pMsg)
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

    /* need to implement when SIT is defined */
    ProtocolNetworkRCIndAdapter adapter(pModemData);
    RIL_RadioCapability resp;

    resp.version = adapter.GetVersion();
    resp.session = adapter.GetSession();
    resp.phase = adapter.GetPhase();
    resp.rat = adapter.GetRafType();

    memset(resp.logicalModemUuid, 0, sizeof(resp.logicalModemUuid));
    BYTE *pUuid = adapter.GetUuid();
    if (pUuid != NULL) memcpy(resp.logicalModemUuid, pUuid, MAX_UUID_LENGTH - 1);
    else memcpy(resp.logicalModemUuid, m_rcModemuuid, MAX_UUID_LENGTH - 1);

    resp.status = adapter.GetStatus();

    if (resp.session == m_rcSession) {
        // Phase update
        m_rcPhase = RC_PHASE_UNSOL_RSP;
        OnUnsolicitedResponse(RIL_UNSOL_RADIO_CAPABILITY, &resp, sizeof(RIL_RadioCapability));
    }

    return 0;
}

int NetworkService::DoGetDuplexMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RilProperty *property = GetRilContextProperty();
    property = GetRilApplicationProperty();
    int curServiceNum = (strrchr(m_szSvcName, '0') != NULL)? 0: 1;
    int socketid = property->GetInt(RIL_APP_PS_ACTIVE_SIM, -1);
    if(socketid == -1) {
        socketid = mCurrentSim;
    } else {
        mCurrentSim = socketid;
    }
    RilLogV("get duplex curServiceNum(%d), socketid(%d), card_state(%d)\n", curServiceNum, socketid, m_cardState);
    if((curServiceNum == socketid) && (m_cardState == RIL_CARDSTATE_PRESENT)) {
        ProtocolNetworkBuilder builder;
        ModemData *pModemData = builder.BuildGetDuplexMode();
        if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_GET_DUPLEX_MODE_DONE) < 0) {
            return -1;
        }
        return 0;
    } else {
        RilLogE("Get Duplex mode no main sim\n");
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
        return 0;
    }
}

int NetworkService::OnGetDuplexModeDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    int mode = 0;

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetDuplexModeRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int duplex_mode_4g = (int)adapter.Get4gDuplexMode();
        int duplex_mode_3g = (int)adapter.Get3gDuplexMode();
        RilLogV("Get Duplex mode done 4g=%d, 3g=%d", duplex_mode_4g, duplex_mode_3g);

        mode = adapter.GetDuplexMode();
        if ( mode == DUPLEX_MODE_INVALID ) {
            RilLogE("Get Duplex mode wrong value 4g=%d, 3g=%d", duplex_mode_4g, duplex_mode_3g);
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }
        else {
            OnRequestComplete(RIL_E_SUCCESS, &mode, sizeof(int));
        }
    }
    else {
        RilLogE("Get Duplex mode error code(%d)", errorCode);
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }
    return 0;
}

int NetworkService::DoSetDuplexMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    //BYTE duplex_mode_4g = (BYTE)3;
    //BYTE duplex_mode_3g = (BYTE)3;

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntRequestData* pReq = (IntRequestData*)pMsg->GetRequestData();
    if (pReq == NULL) {
        RilLogE("pReq is NULL");
        return -1;
    }

    RilProperty *property = GetRilContextProperty();
    property = GetRilApplicationProperty();
    int curServiceNum = (strrchr(m_szSvcName, '0') != NULL)? 0: 1;
    int socketid = property->GetInt(RIL_APP_PS_ACTIVE_SIM, -1);
    if(socketid == -1) {
        socketid = mCurrentSim;
    } else {
        mCurrentSim = socketid;
    }
    int mode = pReq->GetInt();

    RilLogV("SIM%d - Set Duplex mode=%d", curServiceNum, mode);

    ProtocolNetworkBuilder builder;
    if((curServiceNum == socketid) && (m_cardState == RIL_CARDSTATE_PRESENT)) {
        ModemData *pModemData = builder.BuildSetDuplexMode(mode);
        if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT*2, MSG_NET_SET_DUPLEX_MODE_DONE) < 0) {
            return -1;
        }
        return 0;
    } else {
        RilLogE("Set Duplex mode no main sim\n");
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
        return 0;
    }
}

int NetworkService::OnSetDuplexModeDone(Message *pMsg)
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

    ProtocolNetDuplexModeRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        RilLogV("Set Duplex mode done\n");
        OnRequestComplete(RIL_E_SUCCESS);
    } else {
        RilLogE("Set Duplex mode error code(%d)", errorCode);
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }
    return 0;
}

int NetworkService::DoSetDSNetworkType(Message *pMsg)
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

    int netType = rildata->GetInt();
    RilLogI("DualSim Network Type : %d", netType);

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetDSNetworkType(netType);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_SET_DS_NTW_TYPE_DONE) < 0) {
        return -1;
    }

    //update ACTIVE PS data SIM
    RilProperty *property = GetRilApplicationProperty();
    if (property != NULL) {
        RilLogV("[%s] Update Active data SIM slot{%s=%d}", __FUNCTION__, RIL_APP_PS_ACTIVE_SIM, GetRilSocketId());
        property->Put(RIL_APP_PS_ACTIVE_SIM, GetRilSocketId());
        property->Put(RIL_APP_MAIN_SIM, GetRilSocketId());

        if (m_cardState == RIL_CARDSTATE_PRESENT) {
            RilLogV("[%s] SIM card present: update registration state", m_szSvcName);
            OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
        }
    }

    return 0;
}

int NetworkService::OnSetDSNetworkTypeDone(Message *pMsg)
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
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
        stringstream ss;
        ss << RIL_APP_NET_PS_SERVICE_ALLOW << GetRilSocketId();;
        string strPropName = ss.str();
        GetRilApplicationProperty()->Put(strPropName, true);
        RilLogV("Put(%s, %d)", strPropName.c_str(), true);

        // set false to opposit sim.
        ss.str("");
        ss << RIL_APP_NET_PS_SERVICE_ALLOW << (GetRilSocketId() + 1) % 2;
        strPropName = ss.str();
        GetRilApplicationProperty()->Put(strPropName, false);

        // update allowDataState in system
        // Active PS status by SetDSNetworkType should be exclusive
        SetAllowDataState(true, REASON_SET_DS_NETWORK_TYPE, GetRilSocketId());
        SetAllowDataState(false, REASON_SET_DS_NETWORK_TYPE, (GetRilSocketId() + 1) % 2);
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }
    return 0;
}

int NetworkService::DoQueryEmergencyCallAvailableRadioTech(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetEmergencyCallStatus(EMERGENCY_CALL_STATUS_START, RADIO_TECH_UNSPECIFIED);
    if (SendRequest(pModemData, TIMEOUT_NET_EMERGENCY_CALL, MSG_NET_SET_EMERGENCY_CALL_STATUS_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::DoSetEmergencyCallStatus(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    // TODO fetch correct rildata
    IntsRequestData *rildata = (IntsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    // TODO set correct status and rat info
    int status = rildata->GetInt(0);
    int rat = rildata->GetInt(1);
    writeRilEvent(m_szSvcName, __FUNCTION__, "status(%d), rat(%d)", status, rat);
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetEmergencyCallStatus(status, rat);
    if (SendRequest(pModemData, TIMEOUT_NET_EMERGENCY_CALL, MSG_NET_SET_EMERGENCY_CALL_STATUS_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnSetEmergencyCallStatusDone(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);
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
    int errorCode =adapter.GetErrorCode();
    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%s)", errorCode == RIL_E_SUCCESS ? "SUCCESS" : "GENERIC_FAILURE");
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

int NetworkService::OnEmergencyActInfoReceived(Message *pMsg)
{
    RilLog("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolNetEmergencyActInfoAdapter adapter(pModemData);
    int rat = adapter.GetRat();
    int actStatus = adapter.GetActStatus();
    //bool emcAvailable = (rat == RADIO_TECH_LTE || rat == RADIO_TECH_LTE_CA) &&
    //    (actStatus == EMERGENCY_CALL_AVAILABLE);
    RilLogV("RAT=%d actStatus=%d", rat, actStatus);
    writeRilEvent(m_szSvcName, __FUNCTION__, "RAT=%d actStatus=%d", rat, actStatus);

    // notify result to the IMS stack
    // WriteVolteEmcServiceStatus(false, emcAvailable, rat, false, true);

    int result[] = { rat, actStatus };
    OnUnsolicitedResponse(RIL_UNSOL_EMERGENCY_ACT_INFO, result, sizeof(int) * 2);

    return 0;
}

int GetPreferrecNetworkTypeForNextSerach(int currentNetworkType, bool supportCdma)
{
    // if current mode is PREF_NET_TYPE_LTE_GSM_WCDMA,
    //   CP can handle network mode automatically considering with CSIM.
    // NR type never be reported in MCC_IND or TOTAL_OOS_IND

    RilLogV("currentNetworkType from CP : %d");
    RilLogV("supportCdma : %d", supportCdma);

    int preferredNetworkType = currentNetworkType;
    if (supportCdma) {
        RilLogV("CDMA Support scenario");
        switch (currentNetworkType) {
        case PREF_NET_TYPE_GSM_WCDMA: /* GSM/WCDMA (WCDMA preferred) */
        case PREF_NET_TYPE_WCDMA: /* WCDMA  */
        case PREF_NET_TYPE_GSM_WCDMA_AUTO: /* GSM/WCDMA (auto mode, according to PRL) */
            preferredNetworkType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
            break;
        case PREF_NET_TYPE_GSM_ONLY: /* GSM only */
            preferredNetworkType = PREF_NET_TYPE_CDMA_ONLY;
            break;
        case PREF_NET_TYPE_CDMA_EVDO_AUTO: /* CDMA and EvDo (auto mode, according to PRL) */
        case PREF_NET_TYPE_EVDO_ONLY: /* EvDo only */
            preferredNetworkType = PREF_NET_TYPE_GSM_WCDMA;
            break;
        case PREF_NET_TYPE_CDMA_ONLY: /* CDMA only */
            preferredNetworkType = PREF_NET_TYPE_GSM_ONLY;
            break;
        case PREF_NET_TYPE_LTE_GSM_WCDMA: /* LTE, GSM/WCDMA */
        case PREF_NET_TYPE_LTE_ONLY: /* LTE only */
        case PREF_NET_TYPE_LTE_WCDMA: /* LTE/WCDMA */
            // no need to set
            // CP can handle automatically considering with CSIM
            break;
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO: /* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL) */
        case PREF_NET_TYPE_LTE_CDMA_EVDO: /* LTE, CDMA and EvDo */
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA: /* LTE, CDMA, EvDo, GSM/WCDMA */
        default:
            // no need to set
            break;
        }
    } else {
        RilLogV("Support 3GPP only");
        switch (currentNetworkType) {
        case PREF_NET_TYPE_CDMA_EVDO_AUTO: /* CDMA and EvDo (auto mode, according to PRL) */
        case PREF_NET_TYPE_EVDO_ONLY: /* EvDo only */
            preferredNetworkType = PREF_NET_TYPE_GSM_WCDMA;
            break;
        case PREF_NET_TYPE_CDMA_ONLY: /* CDMA only */
            preferredNetworkType = PREF_NET_TYPE_GSM_ONLY;
            break;
        case PREF_NET_TYPE_LTE_CDMA_EVDO: /* LTE, CDMA and EvDo */
            preferredNetworkType = PREF_NET_TYPE_LTE_GSM_WCDMA;
            break;
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO: /* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL) */
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA: /* LTE, CDMA, EvDo, GSM/WCDMA */
            // no need to set
            // CP can handle automatically considering with CSIM
            break;
        case PREF_NET_TYPE_LTE_GSM_WCDMA: /* LTE, GSM/WCDMA */
        case PREF_NET_TYPE_LTE_ONLY: /* LTE only */
        case PREF_NET_TYPE_LTE_WCDMA: /* LTE/WCDMA */
        case PREF_NET_TYPE_GSM_ONLY: /* GSM only */
        case PREF_NET_TYPE_GSM_WCDMA: /* GSM/WCDMA (WCDMA preferred) */
        case PREF_NET_TYPE_WCDMA: /* WCDMA  */
        case PREF_NET_TYPE_GSM_WCDMA_AUTO: /* GSM/WCDMA (auto mode, according to PRL) */
        default:
            // no need to set
            break;
        }
    }

    return preferredNetworkType;
}

int NetworkService::OnTotalOosHappened(Message *pMsg)
{
    RilLog("[%s] %s()", GetServiceName(), __FUNCTION__);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    int cdma_subscription_app_index = GetRilContextProperty()->GetInt("cdma_subscription_app_index", -1);
    bool supportCdma = (cdma_subscription_app_index >= 0);
    RilLogV("[%d] cdma_subscription_app_index=%d supportCdma=%d", GetRilSocketId(), cdma_subscription_app_index, supportCdma);

    // Change Network Mode
    m_totalOosCnt++;

    if ( m_totalOosCnt >= 1 ) {
        m_totalOosCnt = 0;

        ProtocolNetTotalOosAdapter adapter(pModemData);
        int currentPreferredNetworkType = adapter.GetCurrentPrefNetworkMode();
        RilLogV("[%d] currentPreferredNetworkType in CP : %d", GetRilSocketId(), currentPreferredNetworkType);
        if (!ShouldSwitchPreferredNetworkType()) {
            RilLog("[%d] Skip Total OOS handling scenario.", GetRilSocketId());
            return 0;
        }

        int preferredNetworkType = GetPreferrecNetworkTypeForNextSerach(currentPreferredNetworkType, supportCdma);
        RilLogV("[%d] preferredNetworkType for the next search : %d", GetRilSocketId(), preferredNetworkType);
        if (currentPreferredNetworkType != preferredNetworkType) {
            RilLogV("[%d] set preferred network type by RIL from %d to %d", GetRilSocketId(), currentPreferredNetworkType, preferredNetworkType);
            ProtocolNetworkBuilder builder;
            ModemData *pModemData = builder.BuildSetPreferredNetworkType(preferredNetworkType);
            if (SendRequest(pModemData) < 0) {
                // TODO error
            }
            if (pModemData != NULL) {
                delete pModemData;
                pModemData = NULL;
            }
        }
        else {
            RilLogV("Do nothing");
        }
    }
    return 0;
}

enum { PLMN_UNKNOWN = -1, PLMN_HOME = 0, PLMN_ROAMING = 1, PLMN_IGNORE = 2 };

static bool IsTargetOperatorForChina(int targetOper) {
    switch (targetOper) {
    case TARGET_OPER_CHNOPEN:
    case TARGET_OPER_CMCC:
    case TARGET_OPER_CTC:
    case TARGET_OPER_CU:
        return true;
    }
    return false;
}

bool NetworkService::ShouldSwitchPreferredNetworkType()
{
    RilProperty *property = GetRilApplicationProperty();
    if (property != NULL) {
        int target_op = property->GetInt(RIL_APP_TARGET_OPER);
        RilLogV("[%s] Check target operator=%d", m_szSvcName, target_op);
        if (IsTargetOperatorForChina(target_op)) {
            RilLogI("[%s] Target operator is for China retail.", GetServiceName(), target_op);
            return true;
        }
    }
    return false;
}

int NetworkService::CheckChinaCdmaOperatorHomeRoaming(const char *mcc)
{
    if (TextUtils::IsEmpty(mcc)) {
        RilLogV("[%s] Invalid MCC", m_szSvcName);
        return PLMN_IGNORE;
    }

    RilProperty *property = GetRilApplicationProperty();
    if (property == NULL || !IsTargetOperatorForChina(property->GetInt(RIL_APP_TARGET_OPER))) {
        return PLMN_IGNORE;
    }

    property = GetRilContextProperty();
    // get IMSI in RilContext Property and check whether China CDMA operator
    if (property != NULL) {
        const string AllowMccMnc[] = {"46003", "46011", "20404", "45502", "45507", "00101"};
        string strImsi = property->GetString(RIL_CONTEXT_SIM_IMSI);
        bool bFound = false;
        for (unsigned int i = 0; i < sizeof(AllowMccMnc) / sizeof(AllowMccMnc[0]); i++ )
        {
            if ( strImsi.compare(0, 5, AllowMccMnc[i]) == 0 ) {
                RilLogI("[%s] Using China CDMA operator's SIM Card", GetServiceName(), GetRilSocketId());
                bFound = true;
                break;
            }
        }

        if (bFound) {
            const char* CHINA_HOME_MCC[] = {"460"/*China Mainland*/, "455"/*Macau*/};
            for (unsigned int i = 0; i < sizeof(CHINA_HOME_MCC) / sizeof(CHINA_HOME_MCC[0]); i++ ) {
                if (TextUtils::Equals(mcc, CHINA_HOME_MCC[i])) {
                    RilLogI("[%s] MCC(%S) is HOME for China CDMA operator", GetServiceName(), mcc);
                    return PLMN_HOME;
                }
            } // end for i ~
            RilLogI("[%s] MCC(%S) is ROAMING network for China CDMA operator", GetServiceName(), mcc);
            return PLMN_ROAMING;
        }
    }

    RilLogV("[%s] PLMN_UNKNOWN", GetServiceName());
    return PLMN_UNKNOWN;
}

int NetworkService::CheckGlobalOperatorHomeRoaming(const char *mcc)
{
    if (TextUtils::IsEmpty(mcc)) {
        return PLMN_UNKNOWN;
    }

    RilProperty *property = GetRilContextProperty();
    if (property != NULL) {
        string strImsi = property->GetString(RIL_CONTEXT_SIM_IMSI);
        if (strImsi.length() >= 3) {
            // compare with the first 3digits of IMSI.
            string mccFromImsi = strImsi.substr(0, 3);
            if (TextUtils::Equals(mccFromImsi.c_str(), mcc)) {
                return PLMN_HOME;
            }
            return PLMN_ROAMING;
        }
    }
    return PLMN_UNKNOWN;
}

int GetPreferredNetworkModeForNextHomeSearch(int currentNetworkType, bool supportCdma)
{
    // if current mode is PREF_NET_TYPE_LTE_GSM_WCDMA,
    //   CP can handle network mode automatically considering with CSIM.
    // NR type never be reported in MCC_IND or TOTAL_OOS_IND

    int networkType = -1;
    if (supportCdma) {
        // GSM -> CDMA
        // CDMA -> CDMA (no change)
        switch (currentNetworkType) {
        case PREF_NET_TYPE_GSM_WCDMA: /* GSM/WCDMA (WCDMA preferred) */
        case PREF_NET_TYPE_WCDMA: /* WCDMA  */
        case PREF_NET_TYPE_GSM_WCDMA_AUTO: /* GSM/WCDMA (auto mode, according to PRL) */
            networkType = PREF_NET_TYPE_CDMA_EVDO_AUTO;
            break;
        case PREF_NET_TYPE_GSM_ONLY: /* GSM only */
            networkType = PREF_NET_TYPE_CDMA_ONLY;
            break;
        case PREF_NET_TYPE_LTE_GSM_WCDMA: /* LTE, GSM/WCDMA */
        case PREF_NET_TYPE_LTE_ONLY: /* LTE only */
        case PREF_NET_TYPE_LTE_WCDMA: /* LTE/WCDMA */
            // no need to set
            // CP can handle automatically considering with CSIM
            break;
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO: /* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL) */
        case PREF_NET_TYPE_LTE_CDMA_EVDO: /* LTE, CDMA and EvDo */
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA: /* LTE, CDMA, EvDo, GSM/WCDMA */
        case PREF_NET_TYPE_CDMA_EVDO_AUTO: /* CDMA and EvDo (auto mode, according to PRL) */
        case PREF_NET_TYPE_EVDO_ONLY: /* EvDo only */
        case PREF_NET_TYPE_CDMA_ONLY: /* CDMA only */
        default:
            // no need to set
            break;
        }
    }
    else {
        // GSM -> GSM (no change)
        // CDMA -> GSM
        // Change network mode matched with current setting
        switch (currentNetworkType) {
        case PREF_NET_TYPE_LTE_CDMA_EVDO: /* LTE, CDMA and EvDo */
            networkType = PREF_NET_TYPE_LTE_GSM_WCDMA;
            break;
        case PREF_NET_TYPE_CDMA_EVDO_AUTO: /* CDMA and EvDo (auto mode, according to PRL) */
            networkType = PREF_NET_TYPE_GSM_WCDMA;
            break;
        case PREF_NET_TYPE_EVDO_ONLY: /* EvDo only */
            networkType = PREF_NET_TYPE_WCDMA;
            break;
        case PREF_NET_TYPE_CDMA_ONLY: /* CDMA only */
            networkType = PREF_NET_TYPE_GSM_ONLY;
            break;
        case PREF_NET_TYPE_GSM_WCDMA: /* GSM/WCDMA (WCDMA preferred) */
        case PREF_NET_TYPE_WCDMA: /* WCDMA  */
        case PREF_NET_TYPE_GSM_WCDMA_AUTO: /* GSM/WCDMA (auto mode, according to PRL) */
        case PREF_NET_TYPE_GSM_ONLY: /* GSM only */
        case PREF_NET_TYPE_LTE_GSM_WCDMA: /* LTE, GSM/WCDMA */
        case PREF_NET_TYPE_LTE_ONLY: /* LTE only */
        case PREF_NET_TYPE_LTE_WCDMA: /* LTE/WCDMA */
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO: /* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL) */
        case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA: /* LTE, CDMA, EvDo, GSM/WCDMA */
        default:
            // no need to set
            break;
        }
    }

    return networkType;
}

int GetPreferredNetworkModeForNextRoamingSearch(int currentNetworkType, bool supportCdma)
{
    // if current mode is PREF_NET_TYPE_LTE_GSM_WCDMA,
    //   CP can handle network mode automatically considering with CSIM.
    // NR type never be reported in MCC_IND or TOTAL_OOS_IND

    int networkType = -1;
    // Any mode -> GSM

    switch (currentNetworkType) {
    case PREF_NET_TYPE_CDMA_EVDO_AUTO: /* CDMA and EvDo (auto mode, according to PRL) */
        networkType = PREF_NET_TYPE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_CDMA_ONLY: /* CDMA only */
        networkType = PREF_NET_TYPE_GSM_ONLY;
        break;
    case PREF_NET_TYPE_EVDO_ONLY: /* EvDo only */
        networkType = PREF_NET_TYPE_WCDMA;
        break;
    case PREF_NET_TYPE_LTE_CDMA_EVDO: /* LTE, CDMA and EvDo */
        networkType = PREF_NET_TYPE_LTE_GSM_WCDMA;
        break;
    case PREF_NET_TYPE_GSM_WCDMA: /* GSM/WCDMA (WCDMA preferred) */
    case PREF_NET_TYPE_WCDMA: /* WCDMA  */
    case PREF_NET_TYPE_GSM_WCDMA_AUTO: /* GSM/WCDMA (auto mode, according to PRL) */
    case PREF_NET_TYPE_GSM_ONLY: /* GSM only */
    case PREF_NET_TYPE_LTE_GSM_WCDMA: /* LTE, GSM/WCDMA */
    case PREF_NET_TYPE_LTE_ONLY: /* LTE only */
    case PREF_NET_TYPE_LTE_WCDMA: /* LTE/WCDMA */
    case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO: /* GSM/WCDMA, CDMA, and EvDo (auto mode, according to PRL) */
    case PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA: /* LTE, CDMA, EvDo, GSM/WCDMA */
    default:
        // no need to set
        break;
    }
    return networkType;
}

int NetworkService::OnMccReceived(Message *pMsg)
{
    RilLog("[%s] %s()", GetServiceName(), __FUNCTION__);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolNetMccAdapter adapter(pModemData);
    // current network
    int currentNetworkType = adapter.GetCurrentPrefNetworkMode();
    const char *mcc = adapter.GetMcc();
    RilLog("[%s] currentNetworkType=%d MCC=%s", __FUNCTION__, currentNetworkType, mcc);
    property_set(RIL_UIM_REMOTE_MCC, mcc);

    if (!ShouldSwitchPreferredNetworkType()) {
        RilLog("[%d] Skip MCC handling scenario.", GetRilSocketId());
        return 0;
    }

    // Check China CDMA operator
    int result = CheckChinaCdmaOperatorHomeRoaming(mcc);
    // Check global operator if result is PLMN_UNKNOWN
    if (result == PLMN_UNKNOWN) {
        result = CheckGlobalOperatorHomeRoaming(mcc);
    }

    // CDMA capability
    int cdma_subscription_app_index = GetRilContextProperty()->GetInt("cdma_subscription_app_index", -1);
    bool supportCdma = (cdma_subscription_app_index >= 0);
    RilLogV("[%d] cdma_subscription_app_index=%d supportCdma=%d", GetRilSocketId(), cdma_subscription_app_index, supportCdma);

    int networkType = -1;
    if (result == PLMN_HOME) {
        RilLogV("[%s] HOME : mcc(%s) is found in home mcc list", __FUNCTION__, mcc);
        networkType = GetPreferredNetworkModeForNextHomeSearch(currentNetworkType, supportCdma);
    }
    else if (result == PLMN_ROAMING) {
        RilLogV("[%s] ROAMING : mcc(%s) is not found in home mcc list", __FUNCTION__, mcc);
        networkType = GetPreferredNetworkModeForNextRoamingSearch(currentNetworkType, supportCdma);
    }
    else {
        RilLogW("[%s] Not handled in case of MCC %s", __FUNCTION__, mcc);
    }

    RilLogV("[%s] requested networkType : %d", __FUNCTION__, networkType);
    if (networkType != -1 && networkType != currentNetworkType) {
        ProtocolNetworkBuilder builder;
        ModemData *pModemData = builder.BuildSetPreferredNetworkType(networkType);
        if (SendRequest(pModemData) < 0) {
            // TODO : error
        }
        if (pModemData != NULL) {
            delete pModemData;
            pModemData = NULL;
        }
    }
    else {
        RilLogV("[%s] Invalid or no need to set : %d", __FUNCTION__, currentNetworkType);
    }

    return 0;
}

// OEM
int NetworkService::DoOemSetPsService(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    OemHookRawRequestData *rildata = static_cast<OemHookRawRequestData *>(pMsg->GetRequestData());
    if (rildata == NULL || rildata->GetRawData() == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int state = *((int *)rildata->GetRawData() + 1);
    writeRilEvent(m_szSvcName, __FUNCTION__, "state(%s)", state == ALLOW_DATA_CALL ? "ALLOW_DATA_CALL" : "DISALLOW_DATA_CALL");
    RilLogV("%s", state == ALLOW_DATA_CALL ? "ALLOW_DATA_CALL" : "DISALLOW_DATA_CALL");

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildAllowData(state);
    if (SendRequest(pModemData, TIMEOUT_NET_ALLOW_DATA, MSG_NET_OEM_SET_PS_SERVICE_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnOemSetPsServiceDone(Message *pMsg)
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
    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%s)", errorCode == RIL_E_SUCCESS ? "SUCCESS" : "GENERIC_FAILURE");
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }
    return 0;
}

int NetworkService::DoOemGetPsService(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetPsService();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_OEM_GET_PS_SERVICE_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnOemGetPsServiceDone(Message *pMsg)
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

    ProtocolNetGetPsServiceAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int state = adapter.GetState();
        RilLogV("state=%s", state == ALLOW_DATA_CALL ? "ALLOW_DATA_CALL" : "DISALLOW_DATA_CALL");
        // set default data subscription PhoneID
        //update ACTIVE PS data SIM
        RilProperty *property = GetRilApplicationProperty();
        if (property != NULL && (state == ALLOW_DATA_CALL)) {
            property->Put(RIL_APP_PS_ACTIVE_SIM, GetRilSocketId());
            RilLogV("RilAppliationProperty: Put{%s=%d}", RIL_APP_PS_ACTIVE_SIM, GetRilSocketId());

            if (m_cardState == RIL_CARDSTATE_PRESENT) {
                RilLogV("[%s] SIM card present: update registration state", m_szSvcName);
                OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
            }
        }
        OnRequestComplete(RIL_E_SUCCESS, &state, sizeof(int));

        // update allowDataState in system
        SetAllowDataState(state == ALLOW_DATA_CALL, REASON_QUERY_PS_DOMAIN_STATE, GetRilSocketId());
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

int NetworkService::DoOemGetImsSupportService(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildNetworkRegistrationState(DOMAIN_DATA_NETWORK);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_OEM_GET_IMS_SUPPORT_SERVICE_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnOemGetImsSupportServiceDone(Message *pMsg)
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

    ProtocolNetDataRegStateAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int result[5];
        result[0] = (adapter.IsVolteServiceAvailabe() ? 1 : 0);
        result[1] = (adapter.IsEmergencyCallServiceAvailable() ? 1 : 0);
        result[2] = GetRilSocketId();
        result[3] = adapter.GetRadioTech();
        result[4] = adapter.GetRegState();
        OnRequestComplete(RIL_E_SUCCESS, result, sizeof(result));
    }
    else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

void NetworkService::WriteVolteEmcServiceStatus(bool isVopsSupported, bool isEmcBearerSupported, int rat, int regState, bool notify/* = false*/)
{
    // for legacy
    bool supportOemImsSupportInd = (RilApplication::RIL_VersionCode <= HAL_VERSION_CODE(1, 4));
    if (supportOemImsSupportInd) {
#pragma pack(1)
        struct {
            BYTE voPS;
            BYTE emc;
            BYTE rilsocketId;
            BYTE rat;
            BYTE regState;
        } ims_support_ind_data;
#pragma pack()
        ims_support_ind_data.voPS = isVopsSupported ? 1 : 0;
        ims_support_ind_data.emc = isEmcBearerSupported ? 1 : 0;
        ims_support_ind_data.rilsocketId = GetRilSocketId();
        ims_support_ind_data.rat = (BYTE) rat;
        ims_support_ind_data.regState = (BYTE) regState;
        int overallCardState = 0;
        int activeSim = -1;

        RilProperty *property = GetRilApplicationProperty();
        if (property != NULL) {
            activeSim = property->GetInt(RIL_APP_PS_ACTIVE_SIM, -1);
            overallCardState = property->GetInt(RIL_SIM_OVERALL_CARDSTATE, 0);
            RilLogV("WriteVolteEmcServiceStatus overallCardState=0x%08X", overallCardState);

            if (overallCardState == 0) {
                // 9 is invalid value
                ims_support_ind_data.voPS = 9;
            }
        }

        if (notify) {
            RilLogV("RIL_UNSOL_OEM_IMS_SUPPORT_SERVICE={RilSocketId=%d VoLTE Available=%d Emergency Call Available=%d RadioTech=%d}",
                    GetRilSocketId(), isVopsSupported, isEmcBearerSupported, rat);
            // notify status (Framework or RIL client)
            OnUnsolicitedResponse(RIL_UNSOL_OEM_IMS_SUPPORT_SERVICE, &ims_support_ind_data, sizeof(ims_support_ind_data));
        }
    }
}

void NetworkService::QueryEmergencyCallAvailableRadioTech()
{
    RilLog("[%s] %s", GetServiceName(), __FUNCTION__);
    // set correct status and rat info
    // Query
    // status : start
    // RAT : unspecified (0xFF)
    // Baseband will notify the current EMC available Radio Tech.
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetEmergencyCallStatus(EMERGENCY_CALL_STATUS_START, RADIO_TECH_UNSPECIFIED);
    if (pModemData != NULL) {
        if (SendRequest(pModemData) < 0) {
            RilLogW("Fail to send SetEmergencyCallStatus");
        }
        delete pModemData;
    }
}

String NetworkService::updateOperatorNameFromEonsResolver(String operatorNumeric, String simOperator, int lac, int useType)
{
    String ret("");
    if (useType == EONS_RESOLVER_REG_OPERATOR) {
        ret = m_EonsResolver.updateEons(operatorNumeric, lac, simOperator);
    }
    else if (useType == EONS_RESOLVER_AVAILABLE_NETWORK) {
        ret = m_EonsResolver.getEonsForAvailableNetworks(operatorNumeric);
    }
    else {
        // nothing to do
    }
    return ret;
}

int NetworkService::OnSimFileInfo(Message *pMsg)
{
    //RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolNetSimFileInfoAdapter adapter(pModemData);
    int simFileId = adapter.GetSimFileId();
    int recordLen = adapter.GetRecordLen();
    int numOfRecord = adapter.GetNumOfRecords();
    BYTE **simFileData = adapter.GetSimFileData();
    RilLogI("[%s] %s(): EF:0x%x, recLen = %d, numOfRec = %d", GetServiceName(), __FUNCTION__,
            simFileId, recordLen, numOfRecord);
    if (simFileId == NET_SIM_EF_PNN) {
        m_EonsResolver.resetPnnData();
        if(numOfRecord > 0) m_EonsResolver.setPnnData(simFileData, recordLen, numOfRecord);
    }
    else if (simFileId == NET_SIM_EF_OPL) {
        m_EonsResolver.resetOplData();
        if(numOfRecord > 0) m_EonsResolver.setOplData(simFileData, recordLen, numOfRecord);
    }
    else {
    }

    return 0;
}

BOOL NetworkService::IsPlmnSearching(void)
{
    BOOL ret;
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    ret = IsInTransaction(RIL_REQUEST_QUERY_AVAILABLE_NETWORKS) || IsInTransaction(RIL_REQUEST_QUERY_BPLMN_SEARCH);
    return ret;
}

void NetworkService::QueryCurrentPsDomainState()
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);
    RilContext *pRilContext = GetRilContext();
    if (pRilContext != NULL) {
        pRilContext->OnRequest(RIL_REQUEST_OEM_GET_PS_ATTACH_DETACH, NULL, 0, 0);
    }
}

int NetworkService::DoSetMicroCellSrch(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntRequestData* pReq = (IntRequestData*)pMsg->GetRequestData();
    if (pReq == NULL) {
        RilLogE("pReq is NULL");
        return -1;
    }

    int mode = pReq->GetInt();

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetMicroCellSearch((BYTE)mode);
    if (SendRequest(pModemData, TIMEOUT_NET_MC_SRCH, MSG_NET_SET_MC_SRCH_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnSetMicroCellSrchTimeout(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    // Cancel requested Micro cell searching
    ProtocolNetworkBuilder builder;

    ModemData *pModemData = builder.BuildSetMicroCellSearch((BYTE)RIL_MC_SRCH_MODE_CANCEL);
    if (pModemData != NULL) {
        if (SendRequest(pModemData) < 0) {
            RilLogW("Fail to send Cancel micro cell search request ");
        }
        delete pModemData;
    }

    // explicitly error
    OnRequestComplete(RIL_E_GENERIC_FAILURE);
    return 0;
}

int NetworkService::OnSetMicroCellSrchDone(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolNetMcSrchRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        const int srch_result = adapter.GetMcSrchResult();
        const char *plmn = adapter.GetMcSrchPlmn();

        RilLogV("Micor Cell search {result=%d, PLMN=%s}", srch_result, plmn);

        NetworkDataBuilder builder;
        const RilData *rildata = builder.BuildNetFemtoCellSrchResponse(srch_result, plmn);
        if (rildata != NULL && srch_result >= 0) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
        else {
            OnRequestComplete(RIL_E_GENERIC_FAILURE);
            delete rildata;
        }
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

bool NetworkService::IsPossibleToPassInRadioOffState(int request_id)
{
    switch (request_id) {
        case RIL_REQUEST_RADIO_POWER:
        case RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE:
        case RIL_REQUEST_OEM_GET_PS_ATTACH_DETACH:
        case RIL_REQUEST_GET_RADIO_CAPABILITY:
#if (RIL_VERSION >= 10)
        case RIL_REQUEST_SHUTDOWN:
        case RIL_REQUEST_ALLOW_DATA:
#endif
#ifdef RIL_EXTENSION
        case RIL_REQUEST_GET_ALLOW_DATA_STATE:
        case RIL_REQUEST_GET_DUPLEX_MODE:
        case RIL_REQUEST_SET_DUPLEX_MODE:
#endif
            break;
        // allow cs/ps reg query even in radio off
        // in order to update current not-reg state.
        // allow all 4 polling context command id
        case RIL_REQUEST_OPERATOR:
        case RIL_REQUEST_DATA_REGISTRATION_STATE:
        case RIL_REQUEST_VOICE_REGISTRATION_STATE:
        case RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE:

        case RIL_REQUEST_SET_DS_NETWORK_TYPE:
        case RIL_REQUEST_SET_DUAL_NETWORK_AND_ALLOW_DATA:
        case RIL_REQUEST_OEM_GET_IMS_SUPPORT_SERVICE:

        case RIL_REQUEST_OEM_GET_MANUAL_RAT_MODE:
        case RIL_REQUEST_OEM_SET_MANUAL_RAT_MODE:
        case RIL_REQUEST_OEM_GET_FREQUENCY_LOCK:
        case RIL_REQUEST_OEM_SET_FREQUENCY_LOCK:
        case RIL_REQUEST_SET_ACTIVATE_VSIM:
        case RIL_REQUEST_OEM_SET_ENDC_MODE:
        case RIL_REQUEST_OEM_GET_ENDC_MODE:
        case RIL_REQUEST_OEM_GET_FREQUENCY_INFO:
            break;
        default:
            return false;
    }
    return true;

}

bool NetworkService::IsPossibleToPassInRadioUnavailableState(int request_id)
{
    switch(request_id) {
        case RIL_REQUEST_RADIO_POWER:
        case RIL_REQUEST_OEM_GET_RADIO_STATE:
            break;
        default:
            return false;
    }
    return true;
}

#ifdef SUPPORT_CDMA
bool NetworkService::isCdmaVoice(int rat)
{
    switch(rat) {
        case RADIO_TECH_IS95A:
        case RADIO_TECH_IS95B:
        case RADIO_TECH_1xRTT:
        case RADIO_TECH_EVDO_0:
        case RADIO_TECH_EVDO_A:
        case RADIO_TECH_EVDO_B:
        case RADIO_TECH_EHRPD:
            break;
        default:
            return false;
    }
    return true;
}

int NetworkService::DoCdmaSetRoaming(Message *pMsg)
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

    int cdmaRoamingType = rildata->GetInt();
    RilLogV("CDMA Roaming Type: req=%d, cur=%d", cdmaRoamingType, m_cdmaRoamingType);
    if (cdmaRoamingType == m_cdmaRoamingType) {
        OnRequestComplete(RIL_E_SUCCESS);
        return 0;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetCdmaSetRoamingType(cdmaRoamingType);
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    } else if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_CDMA_SET_ROAMING_DONE) < 0) {
        return -1;
    }

    m_cdmaRoamingType = cdmaRoamingType;
    return 0;
}

int NetworkService::OnCdmaSetRoamingDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        m_cdmaRoamingType = -1;
        return -1;
    }
    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        m_cdmaRoamingType = -1;
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
        m_cdmaRoamingType = -1;
    }
    return 0;
}

int NetworkService::DoCdmaQueryRoaming(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildQueryCdmaRoamingType();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_CDMA_QUERY_ROAMING_DONE) < 0) {
        return -1;
    }
    return 0;
}

int NetworkService::OnCdmaQueryRoamingDone(Message *pMsg)
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

    ProtocolNetCdmaQueryRoamingTypeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int cdmaRoamingType = adapter.QueryRoamingType();
        OnRequestComplete(RIL_E_SUCCESS, &cdmaRoamingType, sizeof(int));
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int NetworkService::DoSetCdmaHybridMode(Message *pMsg)
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

    int hybridMode = rildata->GetInt();
    RilLog("Hybrid mode : %d", hybridMode);

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetCdmaHybridMode(hybridMode);
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    } else if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_SET_CDMA_HYBRID_MODE_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnSetCdmaHybridModeDone(Message *pMsg)
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
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

int NetworkService::DoGetCdmaHybridMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetCdmaHybridMode();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_GET_CDMA_HYBRID_MODE_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnGetCdmaHybridModeDone(Message *pMsg)
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

    ProtocolNetCdmaHybridModeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int hybridMode = adapter.GetCdmaHybridMode();
        OnRequestComplete(RIL_E_SUCCESS, &hybridMode, sizeof(int));
    }
    else {
        OnRequestComplete(errorCode);
    }

    return 0;
}
#endif //SUPPORT_CDMA

int NetworkService::DoSetDualNetworkTypeAndAllowData(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntsRequestData *rildata = (IntsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    UINT nRet = IsOppsiteStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during other stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }

#ifdef RIL_FEATURE_FUNCTION_CHECK_CURRENT_STACK_BUSY
    nRet = IsCurrentStackOccupyRF();
    if ( nRet != 0 )
    {
        RilLogE("[%s] %s() : Cannot change network setting during stack is busy", m_szSvcName, __FUNCTION__);
        return -1;
    }
#endif

    int typeForPrimary= rildata->GetInt(0);
    int typeForSecondary = rildata->GetInt(1);
    int allowedForPrimary = rildata->GetInt(2);
    int allowedForSecondary = rildata->GetInt(3);

    RilLog("Dual Network Type : Primary(%d,%d), Secondary(%d,%d)", typeForPrimary, allowedForPrimary, typeForSecondary, allowedForSecondary);
    writeRilEvent(m_szSvcName, __FUNCTION__, "Dual Network Type : Primary(%d,%d), Secondary(%d,%d)", typeForPrimary, allowedForPrimary, typeForSecondary, allowedForSecondary);

#ifdef RIL_FEATURE_BLOCK_PREFERRED_NETWORK_BEFORE_SIMREADY
    if (m_appState != RIL_APPSTATE_READY) {
        RilLogW("(U)SIM application state is not READY! Do not allow Set Preferred network type");
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
        return 0;
    }
#endif

    int reqTypePrimary = typeForPrimary;
    int reqTypeSecondary = typeForSecondary;
    if ( CustomProductFeature::SupportOnly5Mode() == true ) {
        reqTypePrimary = typeForPrimary;
        reqTypeSecondary = typeForSecondary;
        typeForPrimary = FilteroutCdmaNetworkType(typeForPrimary);
        typeForSecondary = FilteroutCdmaNetworkType(typeForSecondary);
        RilLogI("[%s] %s() requested types are changed (%d, %d) -> (%d, %d)", m_szSvcName, __FUNCTION__, reqTypePrimary, reqTypeSecondary, typeForPrimary, typeForSecondary);
    }
    if ( CustomProductFeature::SupportOnlyBasicPreferredNetworkType() == true ) {
        reqTypePrimary = typeForPrimary;
        reqTypeSecondary = typeForSecondary;
        typeForPrimary = FilteroutExtendedNetworkType(typeForPrimary);
        typeForSecondary = FilteroutExtendedNetworkType(typeForSecondary);
        RilLogI("[%s] %s() requested types are changed (%d, %d) -> (%d, %d)", m_szSvcName, __FUNCTION__, reqTypePrimary, reqTypeSecondary, typeForPrimary, typeForSecondary);
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetDualNetworkAndAllowData(typeForPrimary, typeForSecondary, allowedForPrimary, allowedForSecondary);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_SET_DUAL_NTW_AND_ALLOW_DATA_DONE) < 0) {
        return -1;
    }

    //update Main SIM
    RilProperty *property = GetRilApplicationProperty();
    if (property != NULL) {
        RilLogV("[%s] Update Main SIM slot{%s=%d}", __FUNCTION__, RIL_APP_MAIN_SIM, GetRilSocketId());
        property->Put(RIL_APP_PS_ACTIVE_SIM, GetRilSocketId());
        property->Put(RIL_APP_MAIN_SIM, GetRilSocketId());

        if (m_cardState == RIL_CARDSTATE_PRESENT) {
            RilLogV("[%s] SIM card present: update registration state", m_szSvcName);
            OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
        }
    }

    return 0;
}

int NetworkService::OnSetDualNetworkTypeAndAllowDataDone(Message *pMsg)
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
    writeRilEvent(m_szSvcName, __FUNCTION__, "errorCode(%s)", errorCode == RIL_E_SUCCESS ? "SUCCESS" : "GENERIC_FAILURE");
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
        stringstream ss;
        ss << RIL_APP_NET_PS_SERVICE_ALLOW << GetRilSocketId();;
        string strPropName = ss.str();
        GetRilApplicationProperty()->Put(strPropName, true);
        RilLogV("Put(%s, %d)", strPropName.c_str(), true);

        // set false to opposit sim.
        ss.str("");
        ss << RIL_APP_NET_PS_SERVICE_ALLOW << (GetRilSocketId() + 1) % 2;
        strPropName = ss.str();
        GetRilApplicationProperty()->Put(strPropName, false);

        // update allowDataState in system
        // Active PS status by SetDualNetworkTypeAndAllowData should be exclusive
        SetAllowDataState(true, REASON_SET_DUAL_NETWORK_TYPE_AND_ALLOW_DATA, GetRilSocketId());
        SetAllowDataState(false, REASON_SET_DUAL_NETWORK_TYPE_AND_ALLOW_DATA, (GetRilSocketId() + 1) % 2);
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }
    return 0;
}

int NetworkService::GetRegState() {
    return m_nRegState;
}

int NetworkService::DoStartNetworkScan(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    NetworkScanReqData *rildata = (NetworkScanReqData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    int scanType = rildata->GetScanType();
    int timeInterval = rildata->GetTimeInterval();
    int specifiersLength = rildata->GetSpecifiersLength();
    RIL_RadioAccessSpecifier *specifiers =  rildata->GetRadioAccessSpecifier();
    int maxSearchTime = rildata->GetMaxSearchTime();
    bool incrementalResults = rildata->GetIncrementalResults();
    int incrementalResultsPeriodicity = rildata->GetIncrementalResultsPeriodicity();
    int numOfMccMncs = rildata->GetNumOfMccMncs();
    char **mccMncs = rildata->GetMccMncs();

    RilLog("scanType=%d timeInterval=%d specifiersLength=%d",
            scanType, timeInterval, specifiersLength);
    RilLog("maxSearchTime=%d incrementalResults=%d incrementalResultsPeriodicity=%d numOfMccMncs=%d",
            maxSearchTime, incrementalResults, incrementalResultsPeriodicity, numOfMccMncs);
    ModemData *pModemData = builder.BuildStartNetworkScan(scanType, timeInterval, specifiersLength, specifiers,
            maxSearchTime, incrementalResults, incrementalResultsPeriodicity,
            numOfMccMncs, mccMncs);
    if (pModemData == NULL) {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        return 0;
    }

    // timeout : maxSearchTime + 5 sec
    if (SendRequest(pModemData, ((maxSearchTime + 5) * 1000), MSG_NET_START_NETWORK_SCAN_DONE) < 0) {
        OnRequestComplete(RIL_E_OPERATION_NOT_ALLOWED);
        return 0;
    }

    return 0;
}

int NetworkService::OnStartNetworkScanDone(Message *pMsg)
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
    UINT errorCode = adapter.GetErrorCode();
    if (errorCode != RIL_E_SUCCESS) {
        // VTS_1_2 allowed error code: RadioError::NONE, RadioError::REQUEST_NOT_SUPPORTED
        errorCode = RIL_E_REQUEST_NOT_SUPPORTED;
        if (GetCurrentReqeustData() != NULL) {
            NetworkScanReqData *nsr = (NetworkScanReqData *)GetCurrentReqeustData();
            int halVer = nsr->GetHalVersion();
            bool isLegacyRequest = nsr->IsLegacyRequest();
            RilLogV("HAL_VERSION_CODE=%X IsLegacyReques=%d", halVer, isLegacyRequest);
            if (halVer >= HAL_VERSION_CODE(1,4)) {
                if (isLegacyRequest) {
                    // Allowed error codes by VTS_1_4 RadioHidlTest_v1_4#startNetworkScan
                    //   RadioError::NONE, RadioError::OPERATION_NOT_ALLOWED
                    errorCode = RIL_E_OPERATION_NOT_ALLOWED;
                }
                else {
                    // Allowed error codes by VTS_1_4 RadioHidlTest_v1_4#startNetworkScan_GoodRequestX
                    //   RadioError::NONE, RadioError::INVALID_ARGUMENTS
                    errorCode = RIL_E_INVALID_ARGUMENTS;
                }
            }

            // For CTS test(CtsCarrierApiTestCases::testRequestNetworkScan)
            // If CP doesn't support a network scan feature,
            // RIL will send a network scan start response as SUCCESS
            // and a network scan result indication as COMPLETE.
            OnRequestComplete(RIL_E_SUCCESS, NULL, 0);

            RIL_NetworkScanResult_V1_4 result;
            memset(&result, 0, sizeof(result));
            result.status = (RIL_ScanStatus)COMPLETE;
            result.error = (RIL_Errno)RIL_E_REQUEST_NOT_SUPPORTED;
            int unsolResponse = ENCODE_REQUEST(RIL_UNSOL_NETWORK_SCAN_RESULT, halVer);
            OnUnsolicitedResponse(unsolResponse, &result, sizeof(result));
            return 0;
        }
    }
    OnRequestComplete(errorCode, NULL, 0);

    return 0;
}

int NetworkService::DoStopNetworkScan(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildStopNetworkScan();
    if ( SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_STOP_NETWORK_SCAN_DONE) < 0 )
    {
        return -1;
    }

    return 0;
}

int NetworkService::OnStopNetworkScanDone(Message *pMsg)
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
    UINT errorCode = adapter.GetErrorCode();
    if (errorCode != RIL_E_SUCCESS) errorCode = RIL_E_REQUEST_NOT_SUPPORTED;
    OnRequestComplete(errorCode, NULL, 0);

    return 0;
}

int NetworkService::OnNetworkScanResultReceived(Message *pMsg)
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

    ProtocolNetScanResultAdapter adapter(pModemData);
    int status = adapter.GetScanStatus();
    int errorCode = adapter.GetScanResult();

    int halVer = RilApplication::RIL_HalVersionCode;
    NetworkScanResultBuilder builder(halVer);
    const RilData *rildata = builder.Build(status, errorCode, adapter.GetCellInfoList());
    if (rildata != NULL) {
        int unsolResponse = ENCODE_REQUEST(RIL_UNSOL_NETWORK_SCAN_RESULT, halVer);
        OnUnsolicitedResponse(unsolResponse, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }

    return 0;
}

int NetworkService::OnCurrentPhysicalChannelConfigsReceived(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolNetPhysicalChannelConfigs adapter(pModemData);
    int size = adapter.GetSize();
    RIL_PhysicalChannelConfig_V1_4 *configs = adapter.GetConfigs();
    RIL_PhysicalChannelConfig_V1_4 *pccNr = NULL;
    for (int i = 0; configs != NULL && i < size; i++) {
        RIL_PhysicalChannelConfig_V1_4 *p_cur = configs + i;
        if (p_cur->rat == RADIO_TECH_NR) {
            pccNr = p_cur;
            break;
        }
    } // end for i ~

    int oldRat = mNrPhysicalChannelConfigs.rat;
    int oldStatus = mNrPhysicalChannelConfigs.status;

    if (pccNr != NULL) {
        RilLogV("[%d] NR PhysicalChannelConfig: status=%d rfRange=%d",
                GetRilSocketId(), pccNr->status, pccNr->rfInfo.range);
        mNrPhysicalChannelConfigs.rat = pccNr->rat;
        mNrPhysicalChannelConfigs.status = pccNr->status;
    }
    else {
        RilLogV("[%d] Not found NR PhysicalChannelConfig, NR may be released.");
        mNrPhysicalChannelConfigs.status = NONE;
        mNrPhysicalChannelConfigs.rat = RADIO_TECH_UNKNOWN;
    }

    int halVer = RilApplication::RIL_HalVersionCode;
    PhysicalChannelConfigsBuilder builder(halVer);
    const RilData *rildata = builder.Build(configs, size);
    if (rildata != NULL) {
        // explicitly indication for NR status
        OnUnsolicitedResponse(RIL_UNSOL_PHYSICAL_CHANNEL_CONFIG,
                rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }

    // Refresh NrIndicators
    // some basestation couldn't set EN-DC available capability.
    if (oldRat != mNrPhysicalChannelConfigs.rat || oldStatus != mNrPhysicalChannelConfigs.status) {
        RilLogV("[%d] Refresh NrIndicators. reason: NR SCG status changed", GetRilSocketId());
        OnUnsolicitedResponse(RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED);
    }

    return 0;
}

void NetworkService::SetAllowDataState(bool allowed, const char *reason, int phoneId)
{
    RilLogV("[%s] allowed=%d reason=%s phoneId=%d", __FUNCTION__, allowed, reason, phoneId);
    // vendor_rild_prop
    const char *propName = "vendor.ril.allow_data_";
    stringstream ss;
    ss << propName << phoneId;
    SystemProperty::Set(ss.str(), allowed ? 1 : 0);
}

TS25Record NetworkService::GetOperatorName(const char *numeric)
{
    TS25Record ret;
    if (!TextUtils::IsEmpty(numeric)) {
        int len = strlen(numeric);
        if ((len == 5 || len == 6)
                && TextUtils::IsDigitsOnly(numeric)) {
            char mccStr[5] = {0, };
            char mncStr[5] = {0, };
            memcpy(mccStr, numeric, 3);
            memcpy(mncStr, numeric + 3, (len - 3));
            int mcc = atoi(mccStr);
            int mnc = atoi(mncStr);
            RilLog("[%s] Find TS25Table for mcc=%d mnc=%d", __FUNCTION__, mcc, mnc);
            TS25Table *table = TS25Table::GetInstance();
            if (table != NULL) {
                ret = table->GetRecord(mcc, mnc);

                // w/a (SIM operator numeric not existed in TS.25 operator records)
                if (!ret.IsValid() && len == 6) {
                    memset(mncStr, 0, sizeof(mncStr));
                    memcpy(mncStr, numeric + 3, 2);
                    mnc = atoi(mncStr);
                    RilLogV("W/A plan. find for mcc=%d mnc=%d", mcc, mnc);
                    ret = table->GetRecord(mcc, mnc);
                }
            }
        }
    }

    return ret;
}

BOOL hasCondition(int conditions, int condition)
{
    return (conditions & condition) == condition;
}

void NetworkService::conditionsToString(int conditions)
{
    string builder = "[Condition:";
    if (hasCondition(conditions, SIT_EMERGENCY_CONDITION_ALWAYS)) {
        builder.append(" CONDITION_ALWAYS");
    }
    if (hasCondition(conditions, SIT_EMERGENCY_CONDITION_NO_SIM)) {
        builder.append(" CONDITION_NO_SIM");
    }
    if (hasCondition(conditions, SIT_EMERGENCY_CONDITION_TESTCARD)) {
        builder.append(" CONDITION_TESTCARD");
    }
    if (hasCondition(conditions, SIT_EMERGENCY_CONDITION_NOT_IMS_REGI)) {
        builder.append(" CONDITION_NOT_IMS_REGI");
    }
    if (hasCondition(conditions, SIT_EMERGENCY_CONDITION_UI_ONLY)) {
        builder.append(" CONDITION_UI_ONLY");
    }
    builder.append("]");
    RilLogI("[%s] %s() conditions=%s", m_szSvcName, __FUNCTION__, builder.c_str());
}

BOOL NetworkService::isEmergencyRouting(EmcInfo emcInfo)
{
    int conditions = emcInfo.m_conditions;

    bool isTestCard = false;
    // get IMSI in RilContext Property
    RilProperty* property = GetRilContextProperty();
    if (property != NULL) {
        string strImsi = property->GetString(RIL_CONTEXT_SIM_IMSI);
        RilLogI("[%s] %s() strImsi=%s", m_szSvcName, __FUNCTION__, strImsi.c_str());
        isTestCard = !strImsi.compare(0, 5, string(TEST_CARD_IMSI));
    }

    int imsRegiState = RIL_IMS_NOT_REGISTERED;
    // to do: get IMS regi state from property
    ImsService* pImsService = (ImsService*)GetCurrentService(RIL_SERVICE_IMS);
    if (pImsService != NULL) {
        imsRegiState = pImsService->GetImsRegState();
    }

    RilLogI("[%s] %s() found entry emergency number:%s, card state:%d,"
            " voice regi state:%d, isTestCard=%d, imsRegiState=%d",
             m_szSvcName, __FUNCTION__, emcInfo.m_number, m_cardState,
             m_nRegState, isTestCard, imsRegiState);
    conditionsToString(conditions);

    if (hasCondition(conditions, SIT_EMERGENCY_CONDITION_ALWAYS)) {
        return true;
    }
    if (hasCondition(conditions, SIT_EMERGENCY_CONDITION_NO_SIM)) {
        if (m_cardState == RIL_CARDSTATE_ABSENT) {
            return true;
        }
    }
    if (hasCondition(conditions, SIT_EMERGENCY_CONDITION_TESTCARD)) {
        if (isTestCard) {
            return true;
        }
    }
    if (hasCondition(conditions, SIT_EMERGENCY_CONDITION_NOT_IMS_REGI)) {
        if (imsRegiState != RIL_IMS_REGISTERED) {
            return true;
        }
    }
    if (hasCondition(conditions, SIT_EMERGENCY_CONDITION_UI_ONLY)) {
        return true;
    }
    return false;
}

void NetworkService::update(RIL_EmergencyNumber* emergencyNumber, EmcInfo *emcInfo) {
    emergencyNumber->mcc = emcInfo->m_mcc;
    emergencyNumber->mnc = emcInfo->m_mnc;
    emergencyNumber->number = emcInfo->m_number;
    emergencyNumber->categories = emcInfo->m_category;
    emergencyNumber->sources = emcInfo->m_source;
    emergencyNumber->urns = (char**)emcInfo->m_pUrn;
    emergencyNumber->len_urns = emcInfo->m_urn_count;
    for (int i = 0 ; i < emergencyNumber->len_urns ; i++) {
        emergencyNumber->urns[i] = emcInfo->m_urn[i];
    }
}

int NetworkService::mergeEmergencyNumberList(EmcInfoList *database,
                             EmcInfoList *radio,
                             RIL_EmergencyNumber* emergencyNumberList) {
    int count = 0;

    EmcInfo* emcInfo = database->getEmcInfoList();
    for(int i = 0 ; i < database->getCount() ; i++) {
        if (isEmergencyRouting(emcInfo[i])) {
            update(&emergencyNumberList[count++], &emcInfo[i]);
        }
    }

    emcInfo = radio->getEmcInfoList();
    for(int i = 0 ; i < radio->getCount() ; i++) {
        update(&emergencyNumberList[count++], &emcInfo[i]);
    }
    return count;
}

void NetworkService::mergeAndUpdateEmergencyNumberList()
{
    memset(m_emergencyNumberList, 0x00, sizeof(m_emergencyNumberList));
    // merge radio and database
    int len = 0;
    CscService* pCscService = (CscService*)GetCurrentService(RIL_SERVICE_CSC);
    if (pCscService != NULL) {
        len = mergeEmergencyNumberList(&m_emcInfoListFromDatabase,
                                       pCscService->getEmcInfoListFromRadio(),
                                       m_emergencyNumberList);
    }
    writeRilEvent(m_szSvcName, __FUNCTION__, "Send Emergency Number List size=%d", len * sizeof(RIL_EmergencyNumber));

    OnUnsolicitedResponse(RIL_UNSOL_EMERGENCY_NUMBER_LIST, m_emergencyNumberList, len * sizeof(RIL_EmergencyNumber));
}

void NetworkService::updateEmergencyNumberListFromDb() {
    const char *emergencyId = NULL;
    string lastValidPlmn = "";
    string netPlmn = "";
    string strSimPlmn = getValidSimOperatorNumeric();
    const char *simPlmn = strSimPlmn.c_str();
    int lenEmergencyId = 0;

    RilProperty *property = GetRilContextProperty();
    if (property != NULL) {
         netPlmn = property->GetString(RIL_CONTEXT_NET_CURRENT_PLMN);
    }

    // SIM based
    int simBasedEmergencyNumber = SystemProperty::GetInt(VENDOR_RIL_EMERGENCY_NUMBER_SIM, 0);
    RilLogV("networkservice.UpdateEmergencyNumberListFromDb_%d simBasedEmergencyNumber=%d",
            GetRilSocketId(), simBasedEmergencyNumber);
    if (simBasedEmergencyNumber == 1) {
        emergencyId = simPlmn;
        lastValidPlmn = getValidSimPlmn();
    } else {
        emergencyId = netPlmn.c_str();
        RilProperty *pRilAppProperty = GetRilApplicationProperty();
        if (pRilAppProperty != NULL) {
            lastValidPlmn = pRilAppProperty->GetString(RIL_APP_NET_LAST_VALID_PLMN);
        }
    }

    RilLogI("networkservice.UpdateEmergencyNumberListFromDb_%d emergencyId=%s, lastValidPlmn=%s, mEmergencyId=%s, simBasedEmergencyNumber=%d",
             GetRilSocketId(), emergencyId, lastValidPlmn.c_str(), mEmergencyId, simBasedEmergencyNumber);
    lenEmergencyId = strlen(emergencyId);
    if (!IsValidEmergencyId(emergencyId) || !strncmp(emergencyId, INVALID_PLMN, lenEmergencyId)) {
        if ((lastValidPlmn.length() == 0)) {
            // prevent duplicated update
            if (mEmergencyId[0] == 0) {
                RilLogI("networkservice.UpdateEmergencyNumberListFromDb_%d send default ecc list.",
                        GetRilSocketId());
                memset(mEmergencyId, 0, MAX_EMERGENCY_ID_LEN);
                strncpy(mEmergencyId, INVALID_PLMN, strlen(INVALID_PLMN));
                m_emcInfoListFromDatabase.clear();
                EccListLoader eccListLoader;
                eccListLoader.getEccList(mEmergencyId, &m_emcInfoListFromDatabase);
                for(int i = 0 ; i < m_emcInfoListFromDatabase.getCount() ; i++) {
                    EmcInfo* emcInfoArray = m_emcInfoListFromDatabase.getEmcInfoList();
                    RilLogV("networkservice.EccList[%d] number=%s, category=%d, conditions=%d\n",i,
                    emcInfoArray[i].m_number,
                    emcInfoArray[i].m_category,
                    emcInfoArray[i].m_conditions);
                }
                mergeAndUpdateEmergencyNumberList();
            }
        } else {
            RilLogI("networkservice.UpdateEmergencyNumberListFromDb_%d send empty ecc list.",
                    GetRilSocketId());
            memset(mEmergencyId, 0, MAX_EMERGENCY_ID_LEN);
            m_emcInfoListFromDatabase.clear();
            mergeAndUpdateEmergencyNumberList();
        }
    } else if (emergencyId != NULL && strncmp(emergencyId, mEmergencyId, lenEmergencyId)) {
        RilLogI("networkservice.UpdateEmergencyNumberListFromDb_%d use emergencyId %s ecc list.",
                GetRilSocketId(), emergencyId);
        memset(mEmergencyId, 0, MAX_EMERGENCY_ID_LEN);
        strncpy(mEmergencyId, emergencyId, (lenEmergencyId > MAX_EMERGENCY_ID_LEN ? MAX_EMERGENCY_ID_LEN : lenEmergencyId));
        m_emcInfoListFromDatabase.clear();
        EccListLoader eccListLoader;
        eccListLoader.getEccList(mEmergencyId, &m_emcInfoListFromDatabase);
        for(int i = 0 ; i < m_emcInfoListFromDatabase.getCount() ; i++) {
            EmcInfo* emcInfoArray = m_emcInfoListFromDatabase.getEmcInfoList();
            RilLogV("networkservice.EccList_%d [%d] number=%s, category=%d, conditions=%d\n", GetRilSocketId(), i,
            emcInfoArray[i].m_number,
            emcInfoArray[i].m_category,
            emcInfoArray[i].m_conditions);
        }
        mergeAndUpdateEmergencyNumberList();

        if (needToUpdateOppositeEmergencyNumberList()) {
            RilLogV("needToUpdateOppositeEmergencyNumberList");
            NetworkService* pOppNetworkService = (NetworkService*)GetOppositeService(RIL_SERVICE_NETWORK);
            if (pOppNetworkService != NULL) {
                pOppNetworkService->updateEmergencyNumberListFromDb();
            }
        }
    }
}

int NetworkService::needToUpdateOppositeEmergencyNumberList() {
    // overallCardState
    // bit 0 : SIM1 card state, bit 1 : SIM2 card state
    // 0 : No Sim, 1 : Only SIM1, 2 : Only SIM2, 3 = Dual SIM
    int overallCardState = 0;
    int isPresent = 0;
    RilProperty *pRilAppProperty = GetRilApplicationProperty();
    if (pRilAppProperty != NULL) {
        overallCardState = pRilAppProperty->GetInt(RIL_SIM_OVERALL_CARDSTATE, 0);
        isPresent = (overallCardState & (0x01<<GetRilSocketId())) >> GetRilSocketId();
    }
    bool temp = ((overallCardState == 0x01) || (overallCardState == 0x02));
    return isPresent & temp;
}

String NetworkService::getValidSimOperatorNumeric() {
    string strSimPlmn = GetSimOperatorNumeric();

    // overallCardState
    // bit 0 : SIM1 card state, bit 1 : SIM2 card state
    // 0 : No Sim, 1 : Only SIM1, 2 : Only SIM2, 3 = Dual SIM
    int overallCardState = 0;
    RilProperty *pRilAppProperty = GetRilApplicationProperty();
    if (pRilAppProperty != NULL) {
        overallCardState = pRilAppProperty->GetInt(RIL_SIM_OVERALL_CARDSTATE, 0);
        // RilLogV("getValidSimOperatorNumeric overallCardState=0x%02X", overallCardState);
        int isPresent = (overallCardState & (0x01<<GetRilSocketId())) >> GetRilSocketId();
        // ErrorCase 1: Current Socket is not SIM CARD PRESENT
        // RilLogV("SOCKET:%d is has card state:%d(overall:%d)", GetRilSocketId(), isPresent, overallCardState);

        if (isPresent) {
            if (m_appState == RIL_APPSTATE_PIN) {
                strSimPlmn.clear();
                strSimPlmn.append(PREFIX_ICCID);
                strSimPlmn.append(getIccIdPrefix());
                RilLogI("networkservice.getValidSimOperatorNumeric_%d RIL_APPSTATE_PIN strSimPlmn: %s",
                        GetRilSocketId(), strSimPlmn.c_str());
            }
            return strSimPlmn;
        }
    }
    return "";
}

String NetworkService::getIccIdPrefix() {
    //get ICCID
    string iccId = "";
    RilProperty *property = GetRilContextProperty();
    if (property != NULL) {
         iccId = property->GetString(RIL_CONTEXT_SIM_ICC_ID);
    }

    // need dynamic loading from db later
    const char *ICCID_LIST[] = {"891", "8951", "8952", "8954", "8955", "8956", "8957", "8958", "8982",
                                       "89502", "89503", "89504", "89505", "89506", "89507", "89509",};
    int size = sizeof(ICCID_LIST) / sizeof(ICCID_LIST[0]);
    for (int i = 0 ; i < size ; i++) {
        int lenIccid = strlen(ICCID_LIST[i]);
        if (strncmp(iccId.c_str(), ICCID_LIST[i], lenIccid) == 0) {
            return iccId.substr(0, lenIccid);
        }
    }

    return "";
}

bool NetworkService::IsValidEmergencyId(const char *emergencyId)
{
    if (TextUtils::IsEmpty(emergencyId)) {
        return false;
    }

    if (!(strlen(emergencyId) == 5 || strlen(emergencyId) == 6 ||
          strncmp(emergencyId, PREFIX_ICCID, strlen(PREFIX_ICCID)) == 0)) {
        return false;
    }

    return true;
}

// It returns VALID_PLMN except no SIM case.
String NetworkService::getValidSimPlmn() {
    // overallCardState
    // bit 0 : SIM1 card state, bit 1 : SIM2 card state
    // 0 : No Sim, 1 : Only SIM1, 2 : Only SIM2, 3 = Dual SIM
    int overallCardState = 0;
    RilProperty *pRilAppProperty = GetRilApplicationProperty();
    if (pRilAppProperty != NULL) {
        overallCardState = pRilAppProperty->GetInt(RIL_SIM_OVERALL_CARDSTATE, 0);
        RilLogV("getValidSimPlmn overallCardState=0x%02X", overallCardState);
    }

    if (overallCardState == 0) {
        return "";
    }

    return VALID_PLMN;
}

void NetworkService::resetEmergencyId() {
    // reset PLMN for ecc list
    memset(mEmergencyId, 0, MAX_EMERGENCY_ID_LEN);
    RilProperty *pRilAppProperty = GetRilApplicationProperty();
    if (pRilAppProperty != NULL) {
        pRilAppProperty->Put(RIL_APP_NET_LAST_VALID_PLMN, "");
    }
}
int NetworkService::DoGetManualRatMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetManualRatMode();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_OEM_GET_MANUAL_RAT_MODE_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnGetManualRatModeDone(Message *pMsg)
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

    ProtocolNetGetManualRatModeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int retValue[2] = {0, 0};
        adapter.GetManualRatMode(retValue);
        RilLogV("[%s] %s() mode=%d, rat=%d", m_szSvcName, __FUNCTION__, retValue[0], retValue[1]);
        OnRequestComplete(RIL_E_SUCCESS, retValue, sizeof(int)*2);
    }
    else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int NetworkService::DoSetManualRatMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntsRequestData *rildata = (IntsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int mode = rildata->GetInt(0);
    int rat = rildata->GetInt(1);
    RilLogV("[%s] %s() mode=%d, rat=%d", m_szSvcName, __FUNCTION__, mode, rat);

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetManualRatMode(mode, rat);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_OEM_SET_MANUAL_RAT_MODE_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnSetManualRatModeDone(Message *pMsg)
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

    ProtocolNetSetManualRatModeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int cause = adapter.GetCause();
        RilLogV("[%s] %s() cause=%d", m_szSvcName, __FUNCTION__, cause);
        OnRequestComplete(RIL_E_SUCCESS, &cause, sizeof(int));
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int NetworkService::DoGetFrequencyLock(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetFrequencyLock();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_OEM_GET_FREQUENCY_LOCK_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnGetFrequencyLockDone(Message *pMsg)
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

    ProtocolNetGetFreqLockAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int retValue[7] = {0, 0, 0, 0, 0, 0, 0};
        adapter.GetFrequencyLock(retValue);
        RilLogV("[%s] %s() mode=%d, rat=%d, ltePci=%d, lteEarFcn=%d, gsmArfcn=%d, wcdmaPsc=%d, wcdmaUarfcn=%d",
                m_szSvcName, __FUNCTION__, retValue[0], retValue[1], retValue[2], retValue[3], retValue[4], retValue[5], retValue[6]);
        OnRequestComplete(RIL_E_SUCCESS, retValue, sizeof(int)*7);
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int NetworkService::DoSetFrequencyLock(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntsRequestData *rildata = (IntsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int mode = rildata->GetInt(0);
    int rat = rildata->GetInt(1);
    int ltePci = rildata->GetInt(2);
    int lteEarfcn = rildata->GetInt(3);
    int gsmArfcn = rildata->GetInt(4);
    int wcdamPsc = rildata->GetInt(5);
    int wcdamUarfcn = rildata->GetInt(6);
    RilLogV("[%s] %s() mode=%d, rat=%d, ltePci=%d, lteEarfcn=%d, gsmArfcn=%d, wcdamPsc=%d, wcdamUarfcn=%d",
            m_szSvcName, __FUNCTION__, mode, rat, ltePci, lteEarfcn, gsmArfcn, wcdamPsc, wcdamUarfcn);

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetFrequencyLock(mode, rat, ltePci, lteEarfcn, gsmArfcn, wcdamPsc, wcdamUarfcn);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_OEM_SET_FREQUENCY_LOCK_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnSetFrequencyLockDone(Message *pMsg)
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

    ProtocolNetSetFreqLockAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int result = adapter.GetResult();
        RilLogV("[%s] %s() result=%d", m_szSvcName, __FUNCTION__, result);
        OnRequestComplete(RIL_E_SUCCESS, &result, sizeof(int));
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int NetworkService::DoSetEndcMode(Message *pMsg)
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

    int mode = rildata->GetInt();
    RilLogV("[%s] %s() mode=%d", m_szSvcName, __FUNCTION__, mode);

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildSetEndcMode(mode);
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_OEM_SET_ENDC_MODE_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnSetEndcModeDone(Message *pMsg)
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

    ProtocolNetSetFreqLockAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int NetworkService::DoGetEndcMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetEndcMode();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_OEM_GET_ENDC_MODE_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnGetEndcModeDone(Message *pMsg)
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

    ProtocolNetGetEndcModeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int mode = adapter.GetEndcMode();
        OnRequestComplete(RIL_E_SUCCESS, &mode, sizeof(int));
    }
    else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int NetworkService::OnFrequencyInfo(Message *pMsg)
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

    ProtocolNetworkFrequencyInfoIndAdapter adapter(pModemData);
    int rat = adapter.GetRat();
    int band = adapter.GetBand();
    int frequency = adapter.GetFrequency();

    int response[] = {rat, band, frequency};
    OnUnsolicitedResponse(RIL_UNSOL_OEM_FREQUENCY_INFO, response, sizeof(response));

    return 0;
}

int NetworkService::OnB2B1ConfigInfo(Message *pMsg)
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

    ProtocolIndAdapter adapter(pModemData);
    RilLogI("getParameter=(%s) getParameterLength= %d()", adapter.GetParameter(), adapter.GetParameterLength());
    OnUnsolicitedResponse(RIL_UNSOL_B2_B1_CONFIG_INFO,(void *)adapter.GetParameter(), (int)adapter.GetParameterLength());

    return 0;
}

// IOemSamsungslsi@1.1
bool NetworkService::IsNrScgAdded()
{
    return mNrPhysicalChannelConfigs.rat == RADIO_TECH_NR &&
            mNrPhysicalChannelConfigs.status == SECONDARY_SERVING;
}

int NetworkService::OnAcBarringInfo(Message *pMsg)
{
    RilLog("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolNetAcBarringInfo adapter(pModemData);
    char acBarringInfo[5] = {0, 100, 100, 100, 100};
    adapter.GetAcBarringInfo(acBarringInfo, sizeof(char)*5);
    RilLogV("[%s] %s() RIL_UNSOL_OEM_AIMS_AC_BARRING_INFO forEmc(%d), forMoSig(%d), forMoData(%d), forMmtelVoice(%d), forMmtelVide(%d)",
            m_szSvcName, __FUNCTION__, acBarringInfo[0], acBarringInfo[1], acBarringInfo[2], acBarringInfo[3], acBarringInfo[4]);
    OnUnsolicitedResponse(RIL_UNSOL_OEM_AIMS_AC_BARRING_INFO, acBarringInfo, sizeof(char)*5);
    return 0;
}

int NetworkService::DoGetFrequencyInfo(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildGetFrequencyInfo();
    if (SendRequest(pModemData, TIMEOUT_NET_DEFAULT, MSG_NET_GET_FREQUENCY_INFO_DONE) < 0) {
        return -1;
    }

    return 0;
}

int NetworkService::OnGetFrequencyInfoDone(Message *pMsg)
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

    ProtocolNetGetFrequencyInfoAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int rat = adapter.GetRat();
        int band = adapter.GetBand();
        int frequency = adapter.GetFrequency();

        int response[] = {rat, band, frequency};
        OnRequestComplete(RIL_E_SUCCESS, response, sizeof(response));
    }
    else {
        OnRequestComplete(errorCode);
    }

    return 0;
}
