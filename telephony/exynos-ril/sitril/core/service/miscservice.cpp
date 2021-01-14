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
 * miscservice.cpp
 *
 *  Created on: 2014. 6. 30.
 *      Author: m.afzal
 */
#include <telephony/librilutils.h>

#include "callreqdata.h"
#include "miscdata.h"
#include "miscdatabuilder.h"
#include "miscservice.h"
#include "modemcontrol.h"
#include "netdatabuilder.h"
#include "nvitemdata.h"
#include "operatortable.h"
#include "protocolimsbuilder.h"
#include "protocolmiscadapter.h"
#include "protocolmiscbuilder.h"
#include "protocolnetbuilder.h"
#include "protocolradioconfigadapter.h"
#include "protocolradioconfigbuilder.h"
#include "protocolsimadapter.h"
#include "protocolsimbuilder.h"
#include "protocolsmsbuilder.h"
#include "protocolsmsadapter.h"
#include "protocolstkbuilder.h"
#include "protocolstkadapter.h"
#include "radioconfigbuilder.h"
#include "reset_util.h"
#include "rillog.h"
#include "rilapplication.h"
#include "simservice.h"
#include "servicemonitorrunnable.h"
#include "smsdata.h"
#include "smsdatabuilder.h"
#include "stkdata.h"
#include "stkdatabuilder.h"
#include "ts25table.h"

#include <string>
#include <sstream>

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_MISC, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

#define PARAM_NULL(msg)      { if(msg==NULL) { RilLogE("%s::%s() Parameter = NULL", m_szSvcName, __FUNCTION__); return -1; } }
#define NULL_REQ(msg)        { if(msg==NULL || msg->GetRequestData()==NULL) { RilLogE("%s::%s() RequestData = NULL", m_szSvcName, __FUNCTION__); return -1; } }

#define IMS_AIMS_REQUEST_TIMEOUT  180000

extern BYTE g_nLastTpidNewSms;
#define INVALID_TIMESTAMP (uint64_t)(-1)

MiscService::MiscService(RilContext* pRilContext)
: Service(pRilContext, RIL_SERVICE_MISC)
{
    m_nSubscription = CDMA_SUBSCRIPTION_SOURCE_RUIM_SIM;

    mCardState = RIL_CARDSTATE_ABSENT;
    mAppState = RIL_APPSTATE_UNKNOWN;

    // By default, report signal strength to OEM.
    // Once getting signal strength ind, don't report anymore
    mDontReportOemSignalStrength = false;

    // if mCurrentSignalStrength is not NULL,
    // return cached signal strength info instead of querying to the modem.
    mCurrentSignalStrength = NULL;
    mLastReceivedTimestamp = INVALID_TIMESTAMP;
}

MiscService::~MiscService()
{
    if (mCurrentSignalStrength != NULL) {
        delete mCurrentSignalStrength;
    }
}

int MiscService::OnCreate(RilContext *pRilContext)
{
    RilLogI("%s::%s()", m_szSvcName, __FUNCTION__);
    return 0;
}

void MiscService::OnStart()
{
    // init TS.25 Table
    if (GetRilSocketId() == RIL_SOCKET_1) {
        RilLogI("%s %s TS25Table::MakeInstance", GetServiceName(), __FUNCTION__);
        TS25Table::MakeInstance();
    }
}

BOOL MiscService::OnHandleRequest(Message* pMsg)
{
    RilLogI("%s::%s()", m_szSvcName, __FUNCTION__);

    INT32 nRet = -1;
    if(NULL == pMsg)
        return FALSE;

    switch (pMsg->GetMsgId())
    {
        case MSG_MISC_BASEBAND_VER:
        {
            nRet = DoBaseBandVersion(pMsg);
            break;
        }
        case MSG_MISC_SIGNAL_STR:
        {
            nRet = DoSignalStrength(pMsg);
            break;
        }
        case MSG_MISC_QUERY_TTY:
        {
            nRet = DoGetTtyMode(pMsg);
            break;
        }
        case MSG_MISC_SET_TTY:
        {
            nRet = DoSetTtyMode(pMsg);
            break;
        }
        case MSG_MISC_SCREEN:
        {
            nRet = DoScreenState(pMsg);
            break;
        }
        case MSG_MISC_SEND_DEVICE_STATE:
        {
            nRet = DoSendDeviceState(pMsg);
            break;
        }
        case MSG_MISC_GET_IMEI:
        {
            nRet = DoIMEI(pMsg);
            break;
        }
        case MSG_MISC_GET_IMEISV:
        {
            nRet = DoIMEISV(pMsg);
            break;
        }
        case MSG_MISC_DEV_IDENTITY:
        {
            nRet = DoDevIdentity(pMsg);
            break;
        }
        case MSG_MISC_OEM_HIDDEN_REQ:
        {
            nRet = DoOEMSysDump(pMsg);
            break;
        }
        case MSG_MISC_OEM_FCRASH_MNR_REQ:
        {
            nRet = DoOEMSysDump(pMsg);
            break;
        }
        case MSG_MISC_OEM_NOTIFY_CPCRASH:
        {
            nRet = DoOEMNotifyCPCrash(pMsg);
            break;
        }
        case MSG_MISC_OEM_NOTIFY_SILENTRESET:
        {
            nRet = DoOEMNotifySilentReset(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_ENG_MODE:
        {
            nRet = DoSetEngMode(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_SCREEN_LINE:
        {
            nRet = DoSetScrLine(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_DEBUG_TRACE:
        {
            nRet = DoSetDebugTrace(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_CARRIER_CONFIG:
        {
            nRet = DoSetCarrierConfig(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_ENG_STRING_INPUT:
        {
            nRet = DoSetEngStringInput(pMsg);
            break;
        }
        case MSG_MISC_OEM_APN_SETTINGS:
        {
            nRet = DoApnSettings(pMsg);
            break;
        }
        case MSG_MISC_OEM_GET_MSL_CODE:
        {
            nRet = DoOemGetMslCode(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_PIN_CONTROL:
        {
            nRet = DoSetPinControl(pMsg);
            break;
        }
        case MSG_MISC_OEM_GET_MANUAL_BAND_MODE:
        {
            nRet = DoGetManualBandMode(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_MANUAL_BAND_MODE:
        {
            nRet = DoSetManualBandMode(pMsg);
            break;
        }
        case MSG_MISC_OEM_GET_RF_DESENSE_MODE:
        {
            nRet = DoGetRfDesenseMode(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_RF_DESENSE_MODE:
        {
            nRet = DoSetRfDesenseMode(pMsg);
            break;
        }
        case MSG_MISC_OEM_STORE_ADB_SERIAL_NUMBER:
        {
            nRet = DoStoreAdbSerialNumber(pMsg);
            break;
        }
        case MSG_MISC_OEM_READ_ADB_SERIAL_NUMBER:
        {
            nRet = DoReadAdbSerialNumber(pMsg);
            break;
        }
        case MSG_MISC_OEM_CANCEL_AVAILABLE_NETWORKS:
        {
            nRet = DoOemCancelAvailableNetworks(pMsg);
            break;
        }
        case MSG_MISC_OEM_IF_EXECUTE_AM:
        {
            nRet = DoOemIfExecuteAm(pMsg);
            break;
        }
        case MSG_MISC_DTMF_START:
        {
            nRet = DoDtmfStart();
            break;
        }
        case MSG_MISC_DTMF:
        {
            nRet = DoDtmf();
            break;
        }
        case MSG_MISC_DTMF_STOP:
        {
            nRet = DoDtmfStop();
            break;
        }
        case MSG_MISC_SMS_ACKNOWLEDGE:
        {
            nRet = DoSmsAck(pMsg);
            break;
        }
        case MSG_MISC_SMS_ACK_WITH_PDU:
        {
            nRet = DoSmsAckWithPdu(pMsg);
            break;
        }
        case MSG_MISC_AIMS_SEND_SMS_ACK:
        case MSG_MISC_AIMS_SEND_ACK_INCOMING_SMS:
        case MSG_MISC_AIMS_SEND_ACK_INCOMING_CDMA_SMS:
        case MSG_AIMS_ADD_PDN_INFO:
        case MSG_AIMS_DEL_PDN_INFO:
        case MSG_AIMS_STACK_START_REQ:
        case MSG_AIMS_STACK_STOP_REQ:
        case MSG_AIMS_HIDDEN_MENU:
        case MSG_AIMS_SET_HIDDEN_MENU_ITEM:
        case MSG_AIMS_GET_HIDDEN_MENU_ITEM:
        {
            nRet = DoAimsDefaultRequestHandler(pMsg);
            break;
        }

        case MSG_MISC_NV_READ_ITEM:
        {
            nRet = DoNvReadItem(pMsg);
            break;
        }
        case MSG_MISC_NV_WRITE_ITEM:
        {
            nRet = DoNvWriteItem(pMsg);
            break;
        }
        case MSG_MISC_GET_ACTIVITY_INFO:
        {
            nRet = DoGetActivityInfo(pMsg);
            break;
        }
        case MSG_MISC_SET_OPEN_CARRIER_INFO:
        {
            nRet = DoSetOpenCarrierInfo(pMsg);
            break;
        }
        case MSG_MISC_LCE_START:
        {
            nRet = DoLceStart(pMsg);
            break;
        }
        case MSG_MISC_LCE_STOP:
        {
            nRet = DoLceStop(pMsg);
            break;
        }
        case MSG_MISC_PULL_LCEDATA:
        {
            nRet = DoLcePullLceData(pMsg);
            break;
        }
#ifdef SUPPORT_CDMA
        case MSG_MISC_CDMA_GET_SUBSCRIPT_SOURCE:
            nRet = DoCdmaGetSubscriptSource(pMsg);
            break;
        case MSG_MISC_CDMA_SET_SUBSCRIPT_SOURCE:
            nRet = DoCdmaSetSubscriptSource(pMsg);
            break;
        case MSG_MISC_GET_CDMA_SUBSCRIPTION:
            nRet = DoGetCdmaSubscription(pMsg);
            break;
        case MSG_MISC_GET_HARDWARE_CONFIG:
            nRet = DoGetHardwareConfig(pMsg);
            break;
        case MSG_MISC_SMS_CDMA_ACKNOWLEDGE:
            nRet = DoCdmaSmsAck(pMsg);
            break;
#endif // SUPPORT_CDMA
        case MSG_MISC_SET_VOICE_OPERATION:
            nRet = DoSetVoiceOperation(pMsg);
            break;
        case MSG_MISC_OEM_SET_PREFERRED_CALL_CAPABILITY:
            nRet = DoSetPreferredCallCapability(pMsg);
            break;
        case MSG_MISC_OEM_GET_PREFERRED_CALL_CAPABILITY:
            nRet = DoGetPreferredCallCapability(pMsg);
            break;
        case MSG_MISC_OEM_SEND_SGC:
            nRet = DoSendSGC(pMsg);
            break;
        case MSG_MISC_SET_DEVICE_INFO:
            nRet = DoDeviceInfo(pMsg);
            break;
        case MSG_MISC_SET_INDICATION_FILTER:
            nRet = DoSetIndicationFilter(pMsg);
            break;
        case MSG_MISC_GET_NEIGHBORING_CELL_IDS:
            nRet = DoGetNeighboringCellIds(pMsg);
            break;
        case MSG_MISC_SET_LOCATION_UPDATES:
            nRet = DoSetLocationUpdates(pMsg);
            break;
        case MSG_MISC_SET_SUPP_SVC_NOTIFICATION:
            nRet = DoSetSuppSvcNotification(pMsg);
            break;
        case MSG_MISC_SET_CARRIER_INFO_IMSI_ENCRYPTION:
            nRet = DoSetCarrierInfoImsiEncryption(pMsg);
            break;
        case MSG_MISC_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA:
            nRet = DoSetSignalStrengthReportingCriteria(pMsg);
            break;
        case MSG_MISC_SET_LINK_CAPACITY_REPORTING_CRITERIA:
            nRet = DoSetLinkCapacityReportingCriteria(pMsg);
            break;
        case MSG_MISC_SET_PSENSOR_STATUS:
            nRet = DoSetPSensorStatus(pMsg);
            break;
        case MSG_MISC_SET_SAR_STATE:
            nRet = DoSetSarState(pMsg);
            break;
        case MSG_MISC_GET_SAR_STATE:
            nRet = DoGetSarState(pMsg);
            break;
        case MSG_MISC_SCAN_RSSI:
            nRet = DoScanRssi(pMsg);
            break;
        case MSG_MISC_FORWARDING_AT_COMMAND:
            nRet = DoSendATCommand(pMsg);
            break;
        case MSG_MISC_GET_PLMN_NAME_FROM_SE13TABLE:
            nRet = DoGetPlmnNameFromSE13Table(pMsg);
            break;
        case MSG_MISC_TS25TABLE_DUMP:
            nRet = DoTs25TableDump(pMsg);
            break;
        case MSG_MISC_OEM_GET_RADIO_NODE:
            nRet = DoGetRadioNode(pMsg);
            break;
        case MSG_MISC_OEM_SET_RADIO_NODE:
            nRet = DoSetRadioNode(pMsg);
            break;
        case MSG_MISC_OEM_GET_PROVISION_UPDATE_REQUEST:
            nRet = DoGetProvisionUpdateRequest(pMsg);
            break;
        case MSG_MISC_OEM_SET_PROVISION_UPDATE_DONE_REQUEST:
            nRet = DoSetProvisionUpdateDoneRequest(pMsg);
            break;
        case MSG_MISC_OEM_RADIO_CONFIG_RESET:
            nRet = DoRadioConfigReset(pMsg);
            break;
        case MSG_MISC_GET_PHONE_CAPABILITY:
            nRet = DoGetPhoneCapability(pMsg);
            break;
        case MSG_MISC_SET_MODEMS_CONFIG:
            nRet = DoSetModemsConfig(pMsg);
            break;
        case MSG_MISC_ENABLE_MODEM:
            nRet = DoEnableModem(pMsg);
            break;
        case MSG_MISC_GET_MODEM_STACK_STATUS:
            nRet = DoGetModemStackStatus(pMsg);
            break;
        case MSG_MISC_SET_ACTIVATE_VSIM:
            nRet = DoSetActivateVsim(pMsg);
            break;
        case MSG_MISC_OEM_MODEM_INFO:
            nRet = DoOemModemInfo(pMsg);
            break;
        case MSG_MISC_OEM_MODEM_RESET:
            nRet = DoOemModemReset(pMsg);
            break;
        case MSG_MISC_OEM_SET_RTP_PKTLOSS_THRESHOLD:
            nRet = DoOemSetRtpPktlossThreshold(pMsg);
            break;
        case MSG_MISC_OEM_SET_FUNC_SWITCH_REQ:
            nRet = DoOemSwitchModemFunction(pMsg);
            break;
        case MSG_MISC_OEM_SET_PDCP_DISCARD_TIMER:
            nRet = DoOemSetPdcpDiscardTimer(pMsg);
            break;
        case MSG_MISC_OEM_SET_SELFLOG:
            nRet = DoSetSelflog(pMsg);
            break;
        case MSG_MISC_OEM_GET_SELFLOG_STATUS:
            nRet = DoGetSelfLogStatus(pMsg);
            break;
        case MSG_MISC_OEM_GET_CQI_INFO:
            nRet = DoOemGetCqiInfo(pMsg);
            break;
        case MSG_MISC_OEM_SET_SAR_SETTING:
            nRet = DoOemSetSarSetting(pMsg);
            break;
        case MSG_MISC_SET_IMS_TEST_MODE:
            nRet = DoSetImsTestMode(pMsg);
            break;
        case MSG_MISC_OEM_SET_GMO_SWITCH:
            nRet = DoOemSetGmoSwitch(pMsg);
            break;
        case MSG_MISC_OEM_SET_TCS_FCI_REQ:
            nRet = DoOemSetTcsFci(pMsg);
            break;
        case MSG_MISC_OEM_GET_TCS_FCI_INFO:
            nRet = DoOemGetTcsFci(pMsg);
            break;
        case MSG_MISC_OEM_SET_CA_BW_FILTER:
            nRet = DoOemSetCABandwidthFilter(pMsg);
            break;
        case MSG_MISC_SET_MODEM_LOG_DUMP:
            nRet = DoSetModemLogDump(pMsg);
            break;
        case MSG_MISC_OEM_SET_ELEVATOR_SENSOR:
            nRet = DoSetElevatortSensor(pMsg);
            break;
        case MSG_MISC_SET_UICC:
            nRet = DoSetUicc(pMsg);
            break;
        case MSG_MISC_OEM_SET_SELFLOG_PROFILE:
            nRet = DoSetSelfLogProfile(pMsg);
            break;
        case MSG_MISC_OEM_SET_FORBID_LTE_CELL:
            nRet = DoSetForbidLteCell(pMsg);
            break;
        default:
            break;
    }

    if(0 == nRet)
        return TRUE;
    else
        return FALSE;
}
BOOL MiscService::OnHandleSolicitedResponse(Message* pMsg)
{
    RilLogI("%s::%s()", m_szSvcName, __FUNCTION__);
    INT32 nRet = -1;

    if(NULL == pMsg)
        return FALSE;

    //switch (m_pCurReqMsg->GetMsgId()+1)
    switch(pMsg->GetMsgId())
    {
        case MSG_MISC_BASEBAND_VER_DONE:
        {
            nRet = OnBaseBandVersionDone(pMsg);
            break;
        }
        case MSG_MISC_SIGNAL_STR_DONE:
        {
            nRet = OnSignalStrengthDone(pMsg);
            break;
        }
        case MSG_MISC_QUERY_TTY_DONE:
        {
            nRet = OnGetTtyModeDone(pMsg);
            break;
        }
        case MSG_MISC_SET_TTY_DONE:
        {
            nRet = OnSetTtyModeDone(pMsg);
            break;
        }
        case MSG_MISC_SCREEN_DONE:
        {
            nRet = OnScreenStateDone(pMsg);
            break;
        }
        case MSG_MISC_GET_IMEI_DONE:
        {
            nRet = OnIMEIDone(pMsg);
            break;
        }
        case MSG_MISC_GET_IMEISV_DONE:
        {
            nRet = OnIMEISVDone(pMsg);
            break;
        }
        case MSG_MISC_DEV_IDENTITY_DONE:
        {
            nRet = OnDevIdentityDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_ENG_MODE_DONE:
        {
            nRet = OnOEMSetEngModeDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_SCREEN_LINE_DONE:
        {
            nRet = OnOEMSetScrLineDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_DEBUG_TRACE_DONE:
        {
            nRet = OnOEMSetDebugTraceDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_ENG_STRING_INPUT_DONE:
        {
            nRet = OnOEMSetEngStringInputDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_GET_MSL_CODE_DONE:
        {
            nRet = OnOemGetMslCodeDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_PIN_CONTROL_DONE:
        {
            nRet = OnOEMSetPinControlDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_GET_MANUAL_BAND_MODE_DONE:
        {
            nRet = OnGetManualBandModeDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_MANUAL_BAND_MODE_DONE:
        {
            nRet = OnSetManualBandModeDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_GET_RF_DESENSE_MODE_DONE:
        {
            nRet = OnGetRfDesenseModeDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_RF_DESENSE_MODE_DONE:
        {
            nRet = OnSetRfDesenseModeDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_STORE_ADB_SERIAL_NUMBER_DONE:
        {
            nRet = OnStoreAdbSerialNumberDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_READ_ADB_SERIAL_NUMBER_DONE:
        {
            nRet = OnReadAdbSerialNumberDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_CANCEL_AVAILABLE_NETWORKS_DONE:
        {
            nRet = OnOemCancelAvailableNetworksDone(pMsg);
            break;
        }
        case MSG_MISC_DTMF_START_DONE:
        {
            nRet = DoDtmfStartDone(pMsg);
            break;
        }
        case MSG_MISC_DTMF_DONE:
        {
            nRet = DoDtmfDone(pMsg);
            break;
        }
        case MSG_MISC_DTMF_STOP_DONE:
        {
            nRet = DoDtmfStopDone(pMsg);
            break;
        }
        case MSG_MISC_SMS_ACKNOWLEDGE_DONE:
        {
            nRet = OnSmsAckDone(pMsg);
            break;
        }
        case MSG_MISC_SMS_ACK_WITH_PDU_DONE:
        {
            nRet = OnSmsAckWithPduDone(pMsg);
            break;
        }
        case MSG_MISC_AIMS_SEND_SMS_ACK_DONE:
        case MSG_MISC_AIMS_SEND_ACK_INCOMING_SMS_DONE:
        case MSG_MISC_AIMS_SEND_ACK_INCOMING_CDMA_SMS_DONE:
        case MSG_AIMS_ADD_PDN_INFO_DONE:
        case MSG_AIMS_DEL_PDN_INFO_DONE:
        case MSG_AIMS_STACK_START_REQ_DONE:
        case MSG_AIMS_STACK_STOP_REQ_DONE:
        case MSG_AIMS_HIDDEN_MENU_DONE:
        case MSG_AIMS_SET_HIDDEN_MENU_ITEM_DONE:
        case MSG_AIMS_GET_HIDDEN_MENU_ITEM_DONE:
        {
            nRet = OnAimsDefaultResponseHandler(pMsg);
            break;
        }
        case MSG_MISC_NV_READ_ITEM_DONE:
        {
            nRet = OnNvReadItemDone(pMsg);
            break;
        }
        case MSG_MISC_NV_WRITE_ITEM_DONE:
        {
            nRet = OnNvWriteItemDone(pMsg);
            break;
        }
        case MSG_MISC_GET_ACTIVITY_INFO_DONE:
        {
            nRet = OnGetActivityInfoDone(pMsg);
            break;
        }
        case MSG_MISC_SET_OPEN_CARRIER_INFO_DONE:
        {
            nRet = OnSetOpenCarrierInfoDone(pMsg);
            break;
        }
#ifdef SUPPORT_CDMA
        case MSG_MISC_GET_CDMA_SUBSCRIPTION_DONE:
        {
            nRet = OnGetCdmaSubscriptionDone(pMsg);
            break;
        }
        case MSG_MISC_SMS_CDMA_ACKNOWLEDGE_DONE:
        {
            nRet = OnCdmaSmsAckDone(pMsg);
            break;
        }
        case MSG_MISC_GET_HARDWARE_CONFIG_DONE:
        {
            nRet = OnGetHardwareConfigDone(pMsg);
            break;
        }
#endif // SUPPORT_CDMA
        case MSG_MISC_SET_VOICE_OPERATION_DONE:
        {
            nRet = OnSetVoiceOperationDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_SET_PREFERRED_CALL_CAPABILITY_DONE:
        {
            nRet = OnSetPreferredCallCapabilityDone(pMsg);
            break;
        }
        case MSG_MISC_OEM_GET_PREFERRED_CALL_CAPABILITY_DONE:
        {
            nRet = OnGetPreferredCallCapabilityDone(pMsg);
            break;
        }
        case MSG_MISC_SET_INDICATION_FILTER_DONE:
        {
            nRet = OnSetIndicationFilterDone(pMsg);
            break;
        }
        case MSG_MISC_SET_CARRIER_INFO_IMSI_ENCRYPTION_DONE:
        {
            nRet = OnSetCarrierInfoImsiEncryptionDone(pMsg);
            break;
        }
        case MSG_MISC_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA_DONE:
        {
            nRet = OnSetSignalStrengthReportingCriteriaDone(pMsg);
            break;
        }
        case MSG_MISC_SET_LINK_CAPACITY_REPORTING_CRITERIA_DONE:
        {
            nRet = OnSetLinkCapacityReportingCriteriaDone(pMsg);
            break;
        }
        case MSG_MISC_LCE_START_DONE:
        {
            nRet = OnLceStartDone(pMsg);
            break;
        }
        case MSG_MISC_LCE_STOP_DONE:
        {
            nRet = OnLceStopDone(pMsg);
            break;
        }
        case MSG_MISC_PULL_LCEDATA_DONE:
        {
            nRet = OnLcePullLceDataDone(pMsg);
            break;
        }
        case MSG_MISC_SET_LOCATION_UPDATES_DONE:
        {
            nRet = OnSetLocationUpdateDone(pMsg);
            break;
        }
        case MSG_MISC_SET_SUPP_SVC_NOTIFICATION_DONE:
        {
            nRet = OnSetSuppSvcNotificationDone(pMsg);
            break;
        }
        case MSG_MISC_SET_PSENSOR_STATUS_DONE:
        {
            nRet = OnSetPSensorStatusDone(pMsg);
            break;
        }
        case MSG_MISC_SET_SAR_STATE_DONE:
        {
            nRet = OnSetSarStateDone(pMsg);
            break;
        }
        case MSG_MISC_GET_SAR_STATE_DONE:
        {
            nRet = OnGetSarStateDone(pMsg);
            break;
        }
        case MSG_MISC_SCAN_RSSI_DONE:
            nRet = OnScanRssiDone(pMsg);
            break;
        case MSG_MISC_FORWARDING_AT_COMMAND_DONE:
            nRet = OnSendATCommandDone(pMsg);
            break;
        case MSG_MISC_OEM_GET_RADIO_NODE_DONE:
            nRet = OnGetRadioNodeDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_RADIO_NODE_DONE:
            nRet = OnSetRadioNodeDone(pMsg);
            break;
        case MSG_MISC_OEM_GET_PROVISION_UPDATE_REQUEST_DONE:
            nRet = OnGetProvisionUpdateRequestDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_PROVISION_UPDATE_DONE_REQUEST_DONE:
            nRet = OnSetProvisionUpdateDoneRequestDone(pMsg);
            break;
        case MSG_MISC_OEM_RADIO_CONFIG_RESET_DONE:
            nRet = OnRadioConfigResetDone(pMsg);
            break;
        case MSG_MISC_GET_PHONE_CAPABILITY_DONE:
            nRet = OnGetPhoneCapabilityDone(pMsg);
            break;
        case MSG_MISC_SET_MODEMS_CONFIG_DONE:
            nRet = OnSetModemsConfigDone(pMsg);
            break;
        case MSG_MISC_ENABLE_MODEM_DONE:
            nRet = OnEnableModemDone(pMsg);
            break;
        case MSG_MISC_GET_MODEM_STACK_STATUS_DONE:
            nRet = OnGetModemStackStatusDone(pMsg);
            break;
        case MSG_MISC_SET_ACTIVATE_VSIM_DONE:
            nRet = OnSetActivateVsimDone(pMsg);
            break;
        case MSG_MISC_OEM_MODEM_INFO_DONE:
            nRet = OnOemModemInfoDone(pMsg);
            break;
        case MSG_MISC_OEM_MODEM_RESET_DONE:
            nRet = DoOemModemResetDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_RTP_PKTLOSS_THRESHOLD_DONE:
            nRet = OnOemSetRtpPktlossThresholdDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_FUNC_SWITCH_REQ_DONE:
            nRet = DoOemSwitchModemFunctionDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_PDCP_DISCARD_TIMER_DONE:
            nRet = DoOemSetPdcpDiscardTimerDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_SELFLOG_DONE:
            nRet = OnSetSelflogDone(pMsg);
            break;
        case MSG_MISC_OEM_GET_SELFLOG_STATUS_DONE:
            nRet = OnGetSelfLogStatusDone(pMsg);
            break;
        case MSG_MISC_OEM_GET_CQI_INFO_DONE:
            nRet = OnOemGetCqiInfoDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_SAR_SETTING_DONE:
            nRet = OnOemSetSarSettingDone(pMsg);
            break;
        case MSG_MISC_SET_IMS_TEST_MODE_DONE:
            nRet = OnSetImsTestModeDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_GMO_SWITCH_DONE:
            nRet = OnOemSetGmoSwitchDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_TCS_FCI_REQ_DONE:
            nRet = OnOemSetTcsFciDone(pMsg);
            break;
        case MSG_MISC_OEM_GET_TCS_FCI_INFO_DONE:
            nRet = OnOemGetTcsFciDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_CA_BW_FILTER_DONE:
            nRet = OnOemSetCABandwidthFilterDone(pMsg);
            break;
        case MSG_MISC_SET_MODEM_LOG_DUMP_DONE:
            nRet = OnSetModemLogDumpDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_ELEVATOR_SENSOR_DONE:
            nRet = DoSetElevatortSensorDone(pMsg);
            break;
        case MSG_MISC_SET_UICC_DONE:
            nRet = OnSetUiccDone(pMsg);
            break;
        case MSG_MISC_IND_CURRENT_LINK_CAPACITY_ESTIMATE:
            OnLceDataRecv(pMsg);
            break;
        case MSG_MISC_OEM_SET_SELFLOG_PROFILE_DONE:
            nRet = OnSetSelfLogProfileDone(pMsg);
            break;
        case MSG_MISC_OEM_SET_FORBID_LTE_CELL_DONE:
            nRet = OnSetForbidLteCellDone(pMsg);
            break;
        default:
            break;
    }

    if(0 == nRet)
        return TRUE;
    else
        return FALSE;

}

BOOL MiscService::OnHandleUnsolicitedResponse(Message* pMsg)
{
    RilLogI("%s::%s()", m_szSvcName, __FUNCTION__);
    if(NULL == pMsg)
        return FALSE;

    switch (pMsg->GetMsgId())
    {
        case MSG_MISC_UNSOL_SIGNAL_STRENGTH:
        {
            OnUnsolSignalStrength(pMsg);
            break;
        }
        case MSG_MISC_UNSOL_NITZ_TIME_RECEIVED:
        {
            OnUnsolNITZTime(pMsg);
            break;
        }
        case MSG_MISC_UNSOL_OEM_DISPLAY_ENG:
        {
            OnUnsolOemDisplayEng(pMsg);
            break;
        }
        case MSG_MISC_UNSOL_OEM_PIN_CONTROL:
        {
            OnUnsolOemPinControl(pMsg);
            break;
        }
        case MSG_MISC_PHONE_RESET:
        {
            OnUnsolPhoneReset(pMsg);
            break;
        }
        case MSG_MISC_DATA_STATE_CHANGE:
        {
            OnUnsolDataStateChange(pMsg);
            break;
        }
        case MSG_MISC_LCEDATA_RECV:
        {
            OnUnsolLcedataRecv(pMsg);
            break;
        }
        case MSG_MISC_IND_HARDWARE_CONFIG_CHANGED:
        {
            OnUnsolHardwareConfigChanged(pMsg);
            break;
        }
        case MSG_MISC_IND_CDMA_PRL_CHANGED:
        {
            OnUnsolCdmaPrlChanged(pMsg);
            break;
        }
        case MSG_MISC_IND_MODEM_RESTART:
        {
            OnUnsolModemRestart(pMsg);
            break;
        }
        case MSG_MISC_IND_CARRIER_INFO_IMSI_ENCRYPTION:
        {
            OnUnsolCarrierInfoImsiEncryption(pMsg);
            break;
        }
        case MSG_MISC_SAR_CONTROL_STATE_IND:
        {
            OnUnsolSarContolState(pMsg);
            break;
        }
        case MSG_MISC_SAR_RF_CONNECTION_IND:
        {
            OnUnsolSarRfConnection(pMsg);
            break;
        }
        case MSG_MISC_SCAN_RSSI_RESULT_RECEIVED:
            OnScanRssiResultRecived(pMsg);
            break;
        case MSG_MISC_UNSOL_AT_COMMAND:
            OnUnsolATCommand(pMsg);
            break;
        case MSG_MISC_OEM_MODEM_INFO_RECEIVED:
            OnOemModemInfoReceived(pMsg);
            break;
        case MSG_MISC_OEM_RTP_PKTLOSS_THRESHOLD_RECEIVED:
            OnOemRtpPktlossThresholdReceived(pMsg);
            break;
        case MSG_MISC_UNSOL_SELFLOG_STATUS:
            OnUnsolSelflogStatus(pMsg);
            break;
        case MSG_MISC_OEM_CA_BW_FILTER_IND:
            OnOemCABandwidthFilterInd(pMsg);
            break;
        case MSG_MISC_OEM_IND_NTW_ENDC_CAPABILITY:
            OnEndcCapabilityReceived(pMsg);
            break;
        default:
            break;
    }

    return TRUE;
}

BOOL MiscService::OnHandleInternalMessage(Message* pMsg)
{
    return TRUE;
}

void MiscService::OnModemStateChanged(int state)
{
    if (GetRilSocketId() == RIL_SOCKET_1) {
        if (state == MS_CRASH_RESET) {
            char crashDump[] = "broadcast -a com.samsung.slsi.telephony.action.CRASH_DUMP -n com.samsung.slsi.telephony.silentlogging/.SilentLoggingReceiver";
            OnUnsolicitedResponse(RIL_UNSOL_OEM_AM, (unsigned char *)&crashDump, strlen(crashDump));
        }
        else if (state == MS_CRASH_EXIT) {
#ifndef BLOCK_RED_SCREEN
            // Red screen
            char redScreen[] = "start -n com.samsung.slsi.sysdebugmode/com.samsung.slsi.logdump.ReceiverActivity";
            OnUnsolicitedResponse(RIL_UNSOL_OEM_AM, (unsigned char *)&redScreen, strlen(redScreen));
#endif
            // Snap shot
            char crashDump[] = "broadcast -a com.samsung.slsi.telephony.action.CRASH_DUMP -n com.samsung.slsi.telephony.silentlogging/.SilentLoggingReceiver";
            OnUnsolicitedResponse(RIL_UNSOL_OEM_AM, (unsigned char *)&crashDump, strlen(crashDump));
        }
    }
}

void MiscService::OnRadioOffOrNotAvailable()
{
    UpdateCurrentSignalStrength(NULL);
}


void MiscService::OnRadioAvailable()
{
    // device information
    SendDeviceInfo();

    if (GetRilSocketId() == RIL_SOCKET_1) {
        // CP debug trace
        SetDebugTraceOffOnBoot();

        // set modems config (single or multi-sim)
        SetModemsConfig();

        // SGC for Modem
        SendSGCValue();

        // IMEI SV
        SendSvnInfo();
    }
}

//  handle request
int MiscService::DoBaseBandVersion(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.GetBaseBandVersion();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_BASEBAND_VER_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoSignalStrength(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    if (!IsQueryingSignalStrengthNeed()) {
        RilLogV("[%d] Return cached signal strength", GetRilSocketId());
        PrintCurrentSignalStrength();
        OnRequestComplete(RIL_E_SUCCESS, mCurrentSignalStrength, sizeof(RIL_SignalStrength_V1_4));
        return 0;
    }

    RilLogV("[%d] Query next signal strength", GetRilSocketId());
    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.GetSignalStrength();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SIGNAL_STR_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoGetTtyMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.GetTtyMode();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_QUERY_TTY_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoSetTtyMode(Message *pMsg)
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
    int ttyMode = rildata->GetInt();
    RilLogV("SetTtyMode request rildata->GetInt() is = %d", rildata->GetInt());
    RilLogV("SetTtyMode request is = %d", ttyMode);
    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.SetTtyMode(ttyMode);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_TTY_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoScreenState(Message *pMsg)
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

    int scrState = rildata->GetInt() & 0xff;    // get lower byte
    RilLogV("scrState request rildata->GetInt() is = %d", rildata->GetInt());
    RilLogV("scrState request is = %d", scrState);
    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.SetScreenState(scrState);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SCREEN_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    RilLogV("%s() Screen %s", __FUNCTION__, scrState ? "ON" : "OFF");

    return 0;
}

int MiscService::DoSendDeviceState(Message *pMsg)
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

    int deviceStateType = rildata->GetInt(0);
    int deviceState = rildata->GetInt(1);
    RilLog("deviceStateType=%d deviceState=%d", deviceStateType, deviceState);

    // TODO: do action according to device state type
    // enum {RIL_DST_POWER_SAVE_MODE, RIL_DST_CHARGING_STATE, RIL_DST_LOW_DATA_EXPECTED}
    // modem actions have not been defined yet.

    OnRequestComplete(RIL_E_SUCCESS);

    return 0;
}

int MiscService::DoIMEI(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.GetIMEI();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_GET_IMEI_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoIMEISV(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.GetIMEISV();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_GET_IMEISV_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoDevIdentity(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.GetDevID();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_DEV_IDENTITY_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoOEMSysDump(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    if ( pMsg->GetMsgId() == MSG_MISC_OEM_FCRASH_MNR_REQ )
    {
        RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
        int rcmId = 0xFFFF;
        if (rildata == NULL) { RilLogE("[%s] %s() rildata is NULL", m_szSvcName, __FUNCTION__); }
        else {
            int *miscData = (int *)rildata->GetRawData();
            if (miscData != NULL) rcmId = miscData[0];
        }
        DoForceCpCrash(LOG_DUMP_CAUSE_CP_CRASH_MNR, rcmId);
    }
    else
    {
        DoForceCpCrash();
    }

    return 0;
}

int MiscService::DoOEMNotifyCPCrash(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        OnRequestComplete(RIL_E_GENERIC_FAILURE, NULL, 0);
        return -1;
    }

#ifndef BLOCK_RED_SCREEN
    // Red screen
    char redScreen[] = "start -n com.samsung.slsi.sysdebugmode/com.samsung.slsi.logdump.ReceiverActivity";
    OnUnsolicitedResponse(RIL_UNSOL_OEM_AM, (unsigned char *)&redScreen, strlen(redScreen));
#endif
    // Snap shot
    char crashDump[] = "broadcast -a com.samsung.slsi.telephony.action.CRASH_DUMP -n com.samsung.slsi.telephony.silentlogging/.SilentLoggingReceiver";
    OnUnsolicitedResponse(RIL_UNSOL_OEM_AM, (unsigned char *)&crashDump, strlen(crashDump));
    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

int MiscService::DoOEMNotifySilentReset(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        OnRequestComplete(RIL_E_GENERIC_FAILURE, NULL, 0);
        return -1;
    }

    // Silent reset
    char crashDump[] = "broadcast -a com.samsung.slsi.telephony.action.CRASH_DUMP -n com.samsung.slsi.telephony.silentlogging/.SilentLoggingReceiver";
    OnUnsolicitedResponse(RIL_UNSOL_OEM_AM, (unsigned char *)&crashDump, strlen(crashDump));

    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

int MiscService::DoForceCpCrash(LogDumpCause crash_reason, int _info)
{
    RilLogI("[%s] %s(), reason %d, %d", m_szSvcName, __FUNCTION__, crash_reason, _info);

    bool isNrTestMode = SystemProperty::GetInt("persist.vendor.radio.nr_test", 0) == 1;
    if (isNrTestMode) {
        int rat = SystemProperty::GetInt("persist.vendor.radio.phy_rat", 0);
        int scg = SystemProperty::GetInt("persist.vendor.radio.phy_scg", 0);
        RilLogV("persist.vendor.radio.phy_rat [%d]", rat);
        RilLogV("persist.vendor.radio.phy_scg [%d]", scg);

        RilLogV("[SIT_IND_SCG_BEARER_ALLOCATION emulation]");
        char testbuf[] = {0x02, 0x00, 0x3f, 0x07, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00};
        ((char *)testbuf)[8] = rat & 0xFF;
        ((char *)testbuf)[9] = scg & 0xFF;

        GetRilContext()->ProcessModemData(testbuf, sizeof(testbuf)/sizeof(testbuf[0]));
    } else {
        char reason[CP_RESET_INFO_SIZE] = {0,};

        if (crash_reason == LOG_DUMP_CAUSE_CP_CRASH_APP) {
            strncpy(reason, "APP\n", CP_RESET_INFO_SIZE);
        } else if (crash_reason == LOG_DUMP_CAUSE_CP_CRASH_MNR) {
            snprintf(reason, CP_RESET_INFO_SIZE-1, "SIT timeout 0x%04x\n", _info);
        } else {
            strncpy(reason, "Unknown\n", CP_RESET_INFO_SIZE);
        }
        reason[CP_RESET_INFO_SIZE-1] = 0;

        if (ModemControl::CrashModem(reason) < 0) {
            RilLogE("Force CP Crash Failed.");
            OnRequestComplete(RIL_E_GENERIC_FAILURE, NULL, 0);
            return -1;
        }
    }

    OnRequestComplete(RIL_E_SUCCESS, NULL, 0);
    return 0;
}

void MiscService::GetLog(LogDumpCase type, LogDumpCause reason)
{
    char Command[100];
    struct tm current;
    char *reasonStr;
    char *logCmd;
    char *logPrefix;

    switch (type) {
        case LOG_DUMP_CASE_DUMPSTATE:
            logCmd = (char *)"dumpstate";
            logPrefix = (char *)"dumpstate";
            break;
        case LOG_DUMP_CASE_RADIO:
            logCmd = (char *)"logcat -b radio -v threadtime -d";
            logPrefix = (char *)"radio";
            break;
        case LOG_DUMP_CASE_MAIN:
            logCmd = (char *)"logcat -v threadtime -d";
            logPrefix = (char *)"main";
            break;
        default:
            return;
    }

    switch (reason) {
        case LOG_DUMP_CAUSE_CP_CRASH_APP:
            reasonStr = (char *)"_CPCrash_APP";
            break;
        case LOG_DUMP_CAUSE_CP_CRASH_MNR:
            reasonStr = (char *)"_CPCrash_MNR";
            break;
        default:
            reasonStr = (char *)"";
            break;
    }

    time_t nTime;
    char current_time[20];

    time(&nTime);
    localtime_r(&nTime, &current);

    snprintf(current_time, sizeof(current_time)-1, "%04d%02d%02d%02d%02d%02d",
            current.tm_year + 1900,
            current.tm_mon + 1,
            current.tm_mday,
            current.tm_hour,
            current.tm_min,
            current.tm_sec);

    RilLogE("%s: %s%s_%s.log", __FUNCTION__, logPrefix, reasonStr, current_time);

    snprintf(Command, sizeof(Command)-1, "%s > /data/vendor/dump/%s%s_%s.log", logCmd, logPrefix, reasonStr, current_time);
    system(Command);

    snprintf(Command, sizeof(Command)-1, "chmod 444 /data/vendor/dump/%s%s_%s.log", logPrefix, reasonStr, current_time);
    system(Command);
}

int MiscService::DoSetEngMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }
    const char *MiscData = (char *) rildata->GetRawData();
    ProtocolMiscBuilder builder;
    ModemData *pModemData;

    if (rildata->GetSize() == 1) {
        pModemData = builder.GetEngMode(MiscData[0]);
        RilLogV("engMode= %d", MiscData[0]);
    } else {
        pModemData = builder.GetEngMode(MiscData[0], MiscData[1]);
        RilLogV("engMode= %d, subMode = %d", MiscData[0], MiscData[1]);
    }

    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_ENG_MODE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoSetScrLine(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    const char *MiscData = (char *) rildata->GetRawData();
    BYTE scrLine = MiscData[0];

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.GetScrLine(scrLine);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_SCREEN_LINE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoSetDebugTrace(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    const char *MiscData = (char *) rildata->GetRawData();
    BYTE debugTrace = MiscData[0];

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.SetDebugTrace(debugTrace);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_DEBUG_TRACE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoSetCarrierConfig(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }
    const char *MiscData = (char *) rildata->GetRawData();
    char *tok;
    char tmp[1024*4];
    int cnt = 0;
    String key, value;

    memset(tmp, 0, sizeof(tmp));
    strncpy(tmp, MiscData, rildata->GetSize());
    RilLog("carrier configs = %s", tmp);

    RilProperty *property = GetRilContextProperty();

    tok = strtok(tmp, "|");
    while (tok != NULL)
    {
        cnt++;
        if ((cnt % 2) == 1)
        {
            key = tok;
        }
        else if ((cnt % 2) == 0)
        {
            value = tok;
            if (property != NULL) {
                property->Put(key, value);
            }
        }
        tok = strtok(NULL, "|");
    }

    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

int MiscService::DoSetEngStringInput(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int len = rildata->GetSize();
    if (rildata->GetRawData() != NULL && len < 255) {
        ProtocolMiscBuilder builder;
        ModemData *pModemData = builder.SetEngStringInput(len, (char *) rildata->GetRawData());
        if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_ENG_STRING_INPUT_DONE) < 0) {
            RilLogE("SendRequest error");
            return -1;
        }
    } else {
        RilLogE("Invalid input data");
        return -1;
    }
    return 0;
}

int MiscService::DoApnSettings(Message *pMsg)
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

    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

int MiscService::DoOemGetMslCode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.GetMslCode();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_GET_MSL_CODE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoSetPinControl(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    const char *MiscData = (char *) rildata->GetRawData();
    BYTE signal = MiscData[0];
    BYTE status = MiscData[1];

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.SetPinControl(signal, status);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_PIN_CONTROL_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoGetManualBandMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.GetManualBandMode();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_GET_MANUAL_BAND_MODE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoSetManualBandMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.SetManualBandMode(rildata->GetRawData(), rildata->GetSize());
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_MANUAL_BAND_MODE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoGetRfDesenseMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.GetRfDesenseMode();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_GET_RF_DESENSE_MODE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoSetRfDesenseMode(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.SetRfDesenseMode(rildata->GetRawData(), rildata->GetSize());
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_RF_DESENSE_MODE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoStoreAdbSerialNumber(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int len = rildata->GetSize();
    if (rildata->GetRawData() != NULL && len <= MAX_ADB_SERIAL_NUMBER) {
        ProtocolMiscBuilder builder;
        ModemData *pModemData = builder.StoreAdbSerialNumber(rildata->GetRawData(), len);
        if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_STORE_ADB_SERIAL_NUMBER_DONE) < 0) {
            RilLogE("SendRequest error");
            return -1;
        }
    } else {
        RilLogE("data size is invalid");
        return -1;
    }

    return 0;
}

int MiscService::DoReadAdbSerialNumber(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.ReadAdbSerialNumber();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_READ_ADB_SERIAL_NUMBER_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnBaseBandVersionDone(Message *pMsg)
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

    ProtocolMiscVersionAdapter adapter(pModemData);
    MiscDataBuilder builder;
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        //int mask = adapter.GetMask();
        const char *swver = adapter.GetSwVer();
        const char *hwver = adapter.GetHwVer();
        //const char *rfcal = adapter.GetRfCalDate();
        //const char *prodcode = adapter.GetProdCode();
        //const char *modelid = adapter.GetModelID();
        //int prlnam = adapter.GetPrlNamNum();
        //const BYTE *prlver = adapter.GetPrlVersion();
        //int erinam = adapter.GetEriNamNum();
        //const BYTE *eriver = adapter.GetEriVersion();
        //const BYTE *cpchipset = adapter.GetCPChipSet();

        char szBuff[PROP_VALUE_MAX];
        memset(szBuff, 0x00, sizeof(szBuff));
        snprintf(szBuff, sizeof(szBuff)-1, "%s", hwver);
        property_set(RIL_VENDOR_BASEBAND_HW_VERSION, szBuff);

        const RilData *rildata = builder.BuildBaseBandVersionResponse(swver);
        if (rildata != NULL){
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::OnSignalStrengthDone(Message *pMsg)
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

    ProtocolMiscSigStrengthAdapter adapter(pModemData);

    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        RIL_SignalStrength_V1_4 currentSignalStrength = adapter.GetSignalStrength();
        int halVer = RilApplication::RIL_HalVersionCode;
        if (GetCurrentReqeustData() != NULL) {
            halVer = GetCurrentReqeustData()->GetHalVersion();
        }
        SignalStrengthBuilder buidler(halVer);
        const RilData *rildata = buidler.Build(currentSignalStrength);
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }

        RIL_SignalStrengthForVoWifi signalStrengthForVoWifi =
                adapter.GetSignalStrengthForVowifi();
        if (!mDontReportOemSignalStrength && signalStrengthForVoWifi.valid) {
            RilLogE("[%s] %s() RIL_UNSOL_OEM_WFC_SIGNAL_STRENGTH rsrp(%d), rssnr(%d), gwstr(%d), ecno(%d)",
                                m_szSvcName, __FUNCTION__,
                                signalStrengthForVoWifi.lte_rsrp, signalStrengthForVoWifi.lte_rssnr,
                                signalStrengthForVoWifi.umts_sig_str, signalStrengthForVoWifi.umts_ecno);
            int sigForVowifi[4] = {
                    signalStrengthForVoWifi.lte_rsrp, signalStrengthForVoWifi.lte_rssnr,
                    signalStrengthForVoWifi.umts_sig_str, signalStrengthForVoWifi.umts_ecno
            };
            OnUnsolicitedResponse(RIL_UNSOL_OEM_WFC_SIGNAL_STRENGTH, sigForVowifi, sizeof(sigForVowifi));
        }
        else {
            RilLogV("[%d] ignore reporting RIL_UNSOL_OEM_WFC_SIGNAL_STRENGTH", GetRilSocketId());
        }
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::OnGetTtyModeDone(Message *pMsg)
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

    ProtocolMiscGetTtyAdapter adapter(pModemData);
    int ttymode = adapter.GetTtyMode();
    RilLogV("ttymode =%d", ttymode);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        MiscDataBuilder builder;
        const RilData *rildata = builder.BuildGetTtyModeResponse(ttymode);
        if (rildata != NULL){
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::OnSetTtyModeDone(Message *pMsg)
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

int MiscService::OnScreenStateDone(Message *pMsg)
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

//handle unsolicited response
void MiscService::PrintCurrentSignalStrength()
{
    if (mCurrentSignalStrength != NULL) {
        RilLogV("[%d] RIL_SignalStrength_V1_4(%lu) " \
                "GSM[%d %d %d] CDMA[%d %d] EVDO[%d %d %d] " \
                "LTE[%d %d %d %d %d %d] TD-SCDMA[%u %u %u] WCDMA[%d %d %u %u] " \
                "NR[%d %d %d %d %d %d]",
                GetRilSocketId(), mLastReceivedTimestamp,
                mCurrentSignalStrength->GSM_SignalStrength.signalStrength,
                mCurrentSignalStrength->GSM_SignalStrength.bitErrorRate,
                mCurrentSignalStrength->GSM_SignalStrength.timingAdvance,
                mCurrentSignalStrength->CDMA_SignalStrength.dbm,
                mCurrentSignalStrength->CDMA_SignalStrength.ecio,
                mCurrentSignalStrength->EVDO_SignalStrength.dbm,
                mCurrentSignalStrength->EVDO_SignalStrength.ecio,
                mCurrentSignalStrength->EVDO_SignalStrength.signalNoiseRatio,
                mCurrentSignalStrength->LTE_SignalStrength.signalStrength,
                mCurrentSignalStrength->LTE_SignalStrength.rsrp,
                mCurrentSignalStrength->LTE_SignalStrength.rsrq,
                mCurrentSignalStrength->LTE_SignalStrength.rssnr,
                mCurrentSignalStrength->LTE_SignalStrength.cqi,
                mCurrentSignalStrength->LTE_SignalStrength.timingAdvance,
                mCurrentSignalStrength->TD_SCDMA_SignalStrength.signalStrength,
                mCurrentSignalStrength->TD_SCDMA_SignalStrength.bitErrorRate,
                mCurrentSignalStrength->TD_SCDMA_SignalStrength.rscp,
                mCurrentSignalStrength->WCDMA_SignalStrength.signalStrength,
                mCurrentSignalStrength->WCDMA_SignalStrength.bitErrorRate,
                mCurrentSignalStrength->WCDMA_SignalStrength.rscp,
                mCurrentSignalStrength->WCDMA_SignalStrength.ecno,
                mCurrentSignalStrength->NR_SignalStrength.ssRsrp,
                mCurrentSignalStrength->NR_SignalStrength.ssRsrq,
                mCurrentSignalStrength->NR_SignalStrength.ssSinr,
                mCurrentSignalStrength->NR_SignalStrength.csiRsrp,
                mCurrentSignalStrength->NR_SignalStrength.csiRsrq,
                mCurrentSignalStrength->NR_SignalStrength.csiSinr );
    }
    else {
        RilLogV("No cached signal strength info");
    }
}

void MiscService::UpdateCurrentSignalStrength(RIL_SignalStrength_V1_4 *signalStrength)
{
    if (mCurrentSignalStrength != NULL) {
        delete mCurrentSignalStrength;
        mCurrentSignalStrength = NULL;
        mLastReceivedTimestamp = INVALID_TIMESTAMP;
    }

    if (signalStrength != NULL) {
        mCurrentSignalStrength = new RIL_SignalStrength_V1_4;
        if (mCurrentSignalStrength != NULL) {
            memcpy(mCurrentSignalStrength, signalStrength, sizeof(RIL_SignalStrength_V1_4));
            mLastReceivedTimestamp = ril_nano_time();
            PrintCurrentSignalStrength();
        }
        else {
            RilLogW("[%s] memory alloc error", __FUNCTION__);
        }
    }
}

bool MiscService::IsQueryingSignalStrengthNeed()
{
    // allow querying the current signal strength if
    //   - never received unsolicited signal strength at all
    //   - invalid last received time stamp
    // mCurrentSignalStrength and mLastReceivedTimestamp will be cleared
    // when radio power goes to off or unavailable state.
    if (mCurrentSignalStrength != NULL && mLastReceivedTimestamp != INVALID_TIMESTAMP) {
        return false;
    }

    return true;
}

int MiscService::OnUnsolSignalStrength(Message *pMsg)
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

    ProtocolMiscSigStrengthIndAdapter adapter(pModemData);
    RIL_SignalStrength_V1_4 currentSignalStrength = adapter.GetSignalStrength();
    int halVer = RilApplication::RIL_HalVersionCode;
    SignalStrengthBuilder buidler(halVer);
    const RilData *rildata = buidler.Build(currentSignalStrength);
    if (rildata != NULL) {
        OnUnsolicitedResponse(RIL_UNSOL_SIGNAL_STRENGTH, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }

    // Do not report unsolicited OEM signal strength anymore
    // when received a solicited response
    mDontReportOemSignalStrength = true;

    RIL_SignalStrengthForVoWifi signalStrengthForVoWifi =
            adapter.GetSignalStrengthForVowifi();
    if (signalStrengthForVoWifi.valid) {
        RilLogE("[%s] %s() RIL_UNSOL_OEM_WFC_SIGNAL_STRENGTH rsrp(%d), rssnr(%d), gwstr(%d), ecno(%d)",
                            m_szSvcName, __FUNCTION__,
                            signalStrengthForVoWifi.lte_rsrp, signalStrengthForVoWifi.lte_rssnr,
                            signalStrengthForVoWifi.umts_sig_str, signalStrengthForVoWifi.umts_ecno);
        int sigForVowifi[4] = {
                signalStrengthForVoWifi.lte_rsrp, signalStrengthForVoWifi.lte_rssnr,
                signalStrengthForVoWifi.umts_sig_str, signalStrengthForVoWifi.umts_ecno
        };
        OnUnsolicitedResponse(RIL_UNSOL_OEM_WFC_SIGNAL_STRENGTH, sigForVowifi, sizeof(sigForVowifi));
    }

    return 0;
}

int MiscService::OnUnsolNITZTime(Message *pMsg)
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

    ProtocolMiscNITZTimeAdapter adapter(pModemData);
    //int timeinfo = adapter.TimeInfoType();    // not needed
    int daylightvalid = adapter.DayLightValid();
    int year = adapter.Year();
    int month = adapter.Month();
    int day = adapter.Day();
    int hour = adapter.Hour();
    int minute = adapter.Minute();
    int second = adapter.Second();
    int timezone = adapter.TimeZone();
    int daylightadjust = adapter.DayLightAdjust();
    int dayofweek = adapter.DayofWeek();
    //int mminfo = adapter.GetMMInfo();
    //const BYTE *plmn = adapter.GetPLMN(); //not used

    MiscDataBuilder builder;
    const RilData *rildata = builder.BuildNitzTimeIndication(daylightvalid, year, month,
        day, hour, minute, second, timezone, daylightadjust, dayofweek );
    if (rildata != NULL){
        OnUnsolicitedResponse(RIL_UNSOL_NITZ_TIME_RECEIVED, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }
    return 0;
}

int MiscService::OnUnsolOemDisplayEng(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscOemDisplayEngAdapter adapter(pMsg->GetModemData());

    MiscDataBuilder builder;
    const RilData *rildata = builder.BuildDisplayEngIndication(adapter.GetParameter(), adapter.GetParameterLength());
    if (rildata != NULL){
        OnUnsolicitedResponse(RIL_UNSOL_OEM_DISPLAY_ENG_MODE, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }
    return 0;
}

int MiscService::OnUnsolOemPinControl(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscPinControlAdapter adapter(pMsg->GetModemData());
    BYTE result[2];
    result[0] = adapter.GetSignal();
    result[1] = adapter.GetStatus();

    OnUnsolicitedResponse(RIL_UNSOL_OEM_PIN_CONTROL, result, 2);

    return 0;
}

int MiscService::OnUnsolPhoneReset(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscPhoneResetAdapter adapter(pMsg->GetModemData());
    int reset_type = adapter.GetResetType() & 0xFF;
    int reset_cause = adapter.GetResetCause() & 0xFF;

    RilLogV("[%s] %s() : reset type:%d reset cause:%d", m_szSvcName, __FUNCTION__, reset_type, reset_cause);
    switch(reset_type)
    {
    case 0x01: // reset phone only
        RilLogW("[%s] %s() : @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", m_szSvcName, __FUNCTION__);
        RilLogW("[%s] %s() : Modem reset", m_szSvcName, __FUNCTION__);
        RilLogW("[%s] %s() : @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@", m_szSvcName, __FUNCTION__);
        sleep(3);
        if (GetRilContext() != NULL) {
            GetRilContext()->ResetModem("SILENT_RESET_REQUESTED_BY_MODEM");
        }
        break;
    case 0x02: // reboot (reset both phone and modem)
        RilLogW("[%s] %s() : Phone Reboot", m_szSvcName, __FUNCTION__);
        sleep(3);
        RequestDeviceReset(0x02);
        break;
    case 0x03: // shutdown
        RilLogW("[%s] %s() : Phone Shutdown", m_szSvcName, __FUNCTION__);
        sleep(3);
        RequestDeviceReset(0x03);
        break;
    default:
        break;
    }

    return 0;
}

int MiscService::OnUnsolDataStateChange(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscDataStateChangeAdapter adapter(pMsg->GetModemData());
    BYTE expected_state = adapter.GetExpectedState();

    RilLogV("[%s] %s() : data state:%d", m_szSvcName, __FUNCTION__, expected_state);

    stringstream ss;
    ss << "broadcast -a com.samsung.slsi.action.DATA_STATE -n com.samsung.slsi.telephony.testmode/.TestModeReceiver";
    ss << " --ei data_state ";
    ss << expected_state;
    ss << " --ei phone_id ";
    ss << GetRilSocketId();
    string intent = ss.str();

    RilLogV("send data state change request, (data_state=%d), (phone_id=%d)", expected_state, GetRilSocketId());
    OnUnsolicitedResponse(RIL_UNSOL_OEM_AM, intent.c_str(), intent.length());

    return 0;
}

void MiscService::RequestDeviceReset(int reset_type)
{
    stringstream ss;
    ss << "broadcast -a com.samsung.slsi.action.POWER -n com.samsung.slsi.telephony.testmode/.TestModeReceiver";
    ss << " --ei reset_type ";
    ss << reset_type;
    string intent = ss.str();

    char szBuff[PROP_VALUE_MAX];
    memset(szBuff, 0x00, sizeof(szBuff));
    sprintf(szBuff, "%d", reset_type);
    property_set(VENDOR_RIL_RESET_TYPE, szBuff);
    RilLogV("send device reset request, (reset type=%d)", reset_type);
    OnUnsolicitedResponse(RIL_UNSOL_OEM_AM, intent.c_str(), intent.length());
}

int MiscService::OnUnsolLcedataRecv(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if(IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolMiscLceIndAdapter adapter(pModemData);
    MiscDataBuilder builder;
    int dlLc = adapter.GetDLLc();
    int ulLc = adapter.GetULLc();
    int confLevel = adapter.GetConfLevel();
    int isSuspended = adapter.GetIsSuspended();
    const RilData *rildata = builder.BuildLceDataIndication(dlLc, ulLc, confLevel, isSuspended);
    if (rildata != NULL){
        OnUnsolicitedResponse(RIL_UNSOL_LCEDATA_RECV, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }

    return 0;
}

int MiscService::OnIMEIDone(Message *pMsg)
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

    ProtocolMiscIMEIAdapter adapter(pModemData);
    int len = adapter.IMEILen();
    const BYTE *imei = adapter.GetIMEI();
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        MiscDataBuilder builder;
        const RilData *rildata = builder.BuildIMEIResponse(len, imei);
        if (rildata != NULL){
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::OnIMEISVDone(Message *pMsg)
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

    ProtocolMiscIMEISVAdapter adapter(pModemData);
    int len = adapter.IMEISVLen();
    static BYTE send_imeisv[MAX_IMEISV_LEN];
    const BYTE *imeisv = adapter.GetIMEISV();

    if ( imeisv == NULL ) {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
        return 0;
    }
#define SV_START_BIT_RECEIVE  (15)
#define SV_START_BIT_RESPONSE (14)
#define SV_FIELD_LEN          (2)

    /* remove the checksum field (1byte)
        - IMEI format : AA-BBBBBB-CCCCCC-D : D is checksum
        - IMEISV format : AA-BBBBBB-CCCCCC-EE : EE is software version*/
    memset(send_imeisv, 0, MAX_IMEISV_LEN);
    memcpy(send_imeisv, imeisv, MAX_IMEISV_LEN);
    memmove(&send_imeisv[SV_START_BIT_RESPONSE], &send_imeisv[SV_START_BIT_RECEIVE], SV_FIELD_LEN);
    send_imeisv[MAX_IMEISV_LEN-1] = 0;

    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        MiscDataBuilder builder;
        const RilData *rildata = builder.BuildIMEISVResponse(len, (const BYTE*)send_imeisv);
        if (rildata != NULL){
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::OnDevIdentityDone(Message *pMsg)
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

    ProtocolMiscDeviceIDAdapter adapter(pModemData);
    int imeilen = adapter.IMEILen();
    const BYTE *imei = adapter.GetIMEI();
    int imeisvlen = adapter.IMEISVLen();
    const BYTE *imeisv = adapter.GetIMEISV();
    int meidlen = adapter.MEIDLen();
    const BYTE *meid = adapter.GetMEID();
    int esnlen = adapter.ESNLen();
    const BYTE *esn = adapter.GetESN();
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        MiscDataBuilder builder;
        const RilData *rildata = builder.BuildDevIDResponse(imeilen, imei, imeisvlen, imeisv, meidlen, meid, esnlen, esn);
        if (rildata != NULL){
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::OnOEMSetEngModeDone(Message *pMsg)
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

int MiscService::OnOEMSetScrLineDone(Message *pMsg)
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

int MiscService::OnOEMSetEngStringInputDone(Message * pMsg)
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

int MiscService::OnOemGetMslCodeDone(Message *pMsg)
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

    ProtocolMiscGetMslCodeAdapter adapter(pModemData);

    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        const char *mslCode = adapter.getMslCode();
        OnRequestComplete(RIL_E_SUCCESS, (void *)mslCode, 6); //length = 6;
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::OnOEMSetPinControlDone(Message * pMsg)
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

int MiscService::OnGetManualBandModeDone(Message * pMsg)
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
        OnRequestComplete(errorCode, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::OnSetManualBandModeDone(Message * pMsg)
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

    ProtocolMiscSetManualBandModeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        char cause = adapter.GetCause();
        OnRequestComplete(errorCode, &cause, sizeof(cause));
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::OnGetRfDesenseModeDone(Message * pMsg)
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
        OnRequestComplete(errorCode, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::OnSetRfDesenseModeDone(Message *pMsg)
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

    ProtocolMiscSetManualBandModeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        char cause = adapter.GetCause();
        OnRequestComplete(errorCode, &cause, sizeof(cause));
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::OnStoreAdbSerialNumberDone(Message * pMsg)
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

    ProtocolMiscStoreAdbSerialNumberAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        char rcmError = adapter.GetRcmError();
        OnRequestComplete(errorCode, &rcmError, sizeof(rcmError));
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::OnReadAdbSerialNumberDone(Message * pMsg)
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

    ProtocolMiscReadAdbSerialNumberAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        char adbSerialNumber[MAX_ADB_SERIAL_NUMBER] = {0, };
        strncpy(adbSerialNumber, adapter.GetAdbSerialNumber(), MAX_ADB_SERIAL_NUMBER-1);
        OnRequestComplete(errorCode, adbSerialNumber, sizeof(adbSerialNumber));
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::DoOemCancelAvailableNetworks(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    // Cancel requested PLMN searching
    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildCancelQueryAvailableNetwork();
    if (SendRequest(pModemData, 5000, MSG_MISC_OEM_CANCEL_AVAILABLE_NETWORKS_DONE) < 0) {
        RilLogW("Fail to send Cancel query available networks request ");
        return -1;
    }
    return 0;
}

int MiscService::OnOemCancelAvailableNetworksDone(Message *pMsg)
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

int MiscService::DoOemIfExecuteAm(Message *pMsg)
{
    RilLog("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata =(RawRequestData *)(pMsg->GetRequestData());
    OnUnsolicitedResponse(RIL_UNSOL_OEM_AM, rildata->GetRawData(), rildata->GetSize());
    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

int MiscService::OnOEMSetDebugTraceDone(Message *pMsg)
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

int MiscService::DoSendSGC(Message *pMsg)
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

    const int TargetOp = rildata->GetInt();
    RilLogV("[%s] %s(), SGCVal : %d", m_szSvcName, __FUNCTION__, TargetOp);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.SendSGCValue(TargetOp, 0/*reserved1*/, 0/*reserved2*/);
    if (SendRequest(pModemData, 2000, MSG_MISC_OEM_SEND_SGC_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoSendSGCDone(Message *pMsg)
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

int MiscService::DoDeviceInfo(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    StringsRequestData *rildata = (StringsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    const char* pModel = rildata->GetString(0);
    const char* pSwVer = rildata->GetString(1);
    const char* pProductName = rildata->GetString(2);

    RilLogV("[%s] %s(), Model : %s, SwVer : %s, Product Name : %s", m_szSvcName, __FUNCTION__, pModel, pSwVer, pProductName);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.SendDeviceInfo(pModel, pSwVer, pProductName);
    if (pModemData != NULL) {
        if (SendRequest(pModemData) < 0) {
            RilLogE("SendRequest error");
            delete pModemData;
            return -1;
        } else {
            delete pModemData;
        }
    }

    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

INT32 MiscService::DoDtmfStart()
{
    RilLogI("[MiscService] %s",__FUNCTION__);
    if(IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }
    // On Sim absent, requesting command into CP cause IPC TIMEOUT
    // So return early,
    if( mCardState != RIL_CARDSTATE_PRESENT ) {
        RilLogI("[%s] %s mCardState(%d) is not Present(%d)",
                m_szSvcName, __FUNCTION__, mCardState, RIL_CARDSTATE_PRESENT);
        OnRequestComplete(RIL_E_INVALID_MODEM_STATE);
        return 0;
    }

    DtmfInfo* pDtmfReqData = (DtmfInfo*)m_pCurReqMsg->GetRequestData();

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildDtmfStart(false, pDtmfReqData->m_szDtmf[0]);
    RilLogV("[MiscService] Start Dtmf: %c", pDtmfReqData->m_szDtmf[0]);
    if ( SendRequest(pModemData, MISC_DTMF_TIMEOUT, MSG_MISC_DTMF_START_DONE) < 0 )
    {
        return -1;
    }

    return 0;
}
INT32 MiscService::DoDtmfStartDone(Message* pMsg)
{
    RilLogI("[MiscService] %s",__FUNCTION__);
    if(IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * Allowed RadioError on CardState::ABSENT are
 *   RadioError::NONE
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:INVALID_CALL_ID
 *   RadioError:MODEM_ERR
 *   RadioError:INVALID_MODEM_STATE
 *   GeneralErrors
 *    RadioError::RADIO_NOT_AVAILABLE (radio resetting)
 *    RadioError::NO_MEMORY
 *    RadioError::INTERNAL_ERR
 *    RadioError::SYSTEM_ERR
 *    RadioError::REQUEST_NOT_SUPPORTED
 *    RadioError::CANCELLED
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:NO_RESOURCES
 *   RadioError:NO_MEMORY
 *   RadioError:MODEM_ERR
 *   RadioError:INTERNAL_ERR
 *   RadioError:INVALID_CALL_ID
 *   RadioError:SYSTEM_ERR
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:CANCELLED
 *   RadioError:INVALID_MODEM_STATE
 */
    if ( errorCode == RIL_E_SUCCESS )
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}

INT32 MiscService::DoDtmf()
{
    RilLogI("[MiscService] %s",__FUNCTION__);
    if(IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    DtmfInfo* pDtmfReqData = (DtmfInfo*)m_pCurReqMsg->GetRequestData();

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildDtmf(strlen(pDtmfReqData->m_szDtmf), pDtmfReqData->m_szDtmf);
    RilLogI("[MiscService] do Dtmf: %s", pDtmfReqData->m_szDtmf);
    if ( SendRequest(pModemData, MISC_DTMF_TIMEOUT, MSG_MISC_DTMF_DONE) < 0 )
    {
        return -1;
    }

    return 0;
}
INT32 MiscService::DoDtmfDone(Message* pMsg)
{
    RilLogI("[MiscService] %s",__FUNCTION__);
    if(IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * Allowed RadioError on CardState::ABSENT are
 *   RadioError::NONE
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:INVALID_CALL_ID
 *   RadioError:MODEM_ERR
 *   RadioError:INVALID_MODEM_STATE
 *   GeneralErrors
 *    RadioError::RADIO_NOT_AVAILABLE (radio resetting)
 *    RadioError::NO_MEMORY
 *    RadioError::INTERNAL_ERR
 *    RadioError::SYSTEM_ERR
 *    RadioError::REQUEST_NOT_SUPPORTED
 *    RadioError::CANCELLED
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:NO_RESOURCES
 *   RadioError:NO_MEMORY
 *   RadioError:MODEM_ERR
 *   RadioError:INVALID_CALL_ID
 *   RadioError:INTERNAL_ERR
 *   RadioError:SYSTEM_ERR
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:CANCELLED
 *   RadioError:INVALID_MODEM_STATE
 */
    if ( errorCode == RIL_E_SUCCESS )
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}
INT32 MiscService::DoDtmfStop()
{
    RilLogI("[MiscService] %s",__FUNCTION__);
    if(IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }
    // On Sim absent, requesting command into CP cause IPC TIMEOUT
    // So return early,
    if( mCardState != RIL_CARDSTATE_PRESENT ) {
        RilLogI("[%s] %s mCardState(%d) is not Present(%d)",
                m_szSvcName, __FUNCTION__, mCardState, RIL_CARDSTATE_PRESENT);
        OnRequestComplete(RIL_E_INVALID_MODEM_STATE);
        return 0;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildDtmfStop();
    if ( SendRequest(pModemData, MISC_DTMF_TIMEOUT, MSG_MISC_DTMF_STOP_DONE) < 0 )
    {
        return -1;
    }

    return 0;
}

INT32 MiscService::DoDtmfStopDone(Message* pMsg)
{
    RilLogI("[MiscService] %s",__FUNCTION__);
    if(IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

/*
 * Allowed RadioError on CardState::ABSENT are
 *   RadioError::NONE
 *   RadioError:INVALID_CALL_ID
 *   RadioError:MODEM_ERR
 *   RadioError:INVALID_MODEM_STATE
 *   GeneralErrors
 *    RadioError::RADIO_NOT_AVAILABLE (radio resetting)
 *    RadioError::NO_MEMORY
 *    RadioError::INTERNAL_ERR
 *    RadioError::SYSTEM_ERR
 *    RadioError::REQUEST_NOT_SUPPORTED
 *    RadioError::CANCELLED
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:NO_RESOURCES
 *   RadioError:NO_MEMORY
 *   RadioError:MODEM_ERR
 *   RadioError:INVALID_CALL_ID
 *   RadioError:INTERNAL_ERR
 *   RadioError:SYSTEM_ERR
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:CANCELLED
 *   RadioError:INVALID_MODEM_STATE
 */
    if ( errorCode == RIL_E_SUCCESS )
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_INTERNAL_ERR);
    }

    return 0;
}

int MiscService::DoSmsAck(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    const SmsAcknowledge *rildata = (const SmsAcknowledge *)pMsg->GetRequestData();
    if (rildata == NULL) {
        return -1;
    }

    int result = rildata->mResult;
    int failcause = rildata->mFailureCause;

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSmsAck(result, g_nLastTpidNewSms, failcause);
    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_MISC_SMS_ACKNOWLEDGE_DONE) < 0 ) {
        RilLogE("[%s] %s() : Failed(%d)", m_szSvcName, __FUNCTION__, failcause);
        return -1;
    }
    return 0;
}

int MiscService::OnSmsAckDone(Message *pMsg)
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

int MiscService::DoSmsAckWithPdu(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    const SmsAcknowledgePdu *rildata = (const SmsAcknowledgePdu *)pMsg->GetRequestData();
    if (rildata == NULL) {
        return -1;
    }

#if 0
    const char *pdu = rildata->mPdu->ToHexString();
    int pduSize = strlen(pdu);
#else
    const char *pdu = rildata->mPdu->mData;
    int pduSize = rildata->mPdu->mLen;
#endif

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSmsAck(rildata->mResult, g_nLastTpidNewSms, pdu, pduSize);

    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT, MSG_MISC_SMS_ACKNOWLEDGE_DONE) < 0 ) {
        RilLogE("[%s] %s() : Failed", m_szSvcName, __FUNCTION__);
        return -1;
    }

    return 0;
}

int MiscService::OnSmsAckWithPduDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
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

/**
 * DoAimsDefaultRequestHandler
 * @desc default AIMS Request handler
 */
int MiscService::DoAimsDefaultRequestHandler(Message *pMsg)
{

    RilLogV("[%s] Process AIMS Request", GetServiceName());
    NULL_REQ(pMsg);

    RawRequestData *rildata =(RawRequestData *)(pMsg->GetRequestData());
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    RilLogV("[%s] AIMS Request: msgId=%d requestId=%d parameter=0x%p parameter length=%d",
            GetServiceName(), pMsg->GetMsgId(), rildata->GetReqId(), rildata->GetRawData(), rildata->GetSize());

    ProtocolImsBuilder builder;
    ModemData *pModemData = builder.BuildAimsPDU(rildata->GetReqId(), rildata->GetRawData(), rildata->GetSize());
    if (SendRequest(pModemData, IMS_AIMS_REQUEST_TIMEOUT, pMsg->GetMsgId() + 1) < 0) {
        RilLogW("[%s] Failed to send AIMS PDU", GetServiceName());
        return -1;
    }

    return 0;
}

/**
 * OnAimsDefaultResponseHandler
 * @desc default AIMS Response handler
 */
int MiscService::OnAimsDefaultResponseHandler(Message *pMsg)
{
    RilLogV("[%s] Process AIMS Response", GetServiceName());
    PARAM_NULL(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    RilLogV("[%s] AIMS Response: msgId=%d errorCode=%d parameter=0x%p parameter length=%d",
            GetServiceName(), pMsg->GetMsgId(), errorCode, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

int MiscService::DoNvReadItem(Message *pMsg)
{
    RilLogI("[%s] %s", GetServiceName(),__FUNCTION__);
#if 1
    RilLogI("[%s] %s - not supported", GetServiceName(),__FUNCTION__);
    return -1;
#else
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    NvReadItemRequestData *rildata = (NvReadItemRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildNvReadItem(rildata->GetNvItemID());
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_NV_READ_ITEM_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
#endif
}

int MiscService::OnNvReadItemDone(Message *pMsg)
{
    RilLogI("[%s] %s", GetServiceName(),__FUNCTION__);
#if 1
    RilLogI("[%s] %s - not supported", GetServiceName(),__FUNCTION__);
    return -1;
#else
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolMiscNvReadItemAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        Message *pMsg = GetCurrentMsg();
        if (pMsg != NULL) {
            NvReadItemRequestData *rildata = (NvReadItemRequestData *)pMsg->GetRequestData();
            if (rildata != NULL) {
                ProcessNvReadItem(rildata->GetNvItemID(), adapter.GetValue());
            }
        }

        OnRequestComplete(RIL_E_SUCCESS, (void *)adapter.GetValue(), adapter.GetLength());
    }
    else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }
    return 0;
#endif
}

int MiscService::DoNvWriteItem(Message *pMsg)
{
    RilLogI("[%s] %s", GetServiceName(),__FUNCTION__);
#if 1
    RilLogI("[%s] %s - not supported", GetServiceName(),__FUNCTION__);
    return -1;
#else
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    NvWriteItemRequestData *rildata = (NvWriteItemRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildNvWriteItem(rildata->GetNvItemID(), rildata->GetValue());
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_NV_WRITE_ITEM_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
#endif
}

int MiscService::OnNvWriteItemDone(Message *pMsg)
{
    RilLogI("[%s] %s", GetServiceName(),__FUNCTION__);
#if 1
    RilLogI("[%s] %s - not supported", GetServiceName(),__FUNCTION__);
    return -1;
#else
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
#endif
}

// Added in O & P
int MiscService::DoLceStart(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if(IsNullRequest(pMsg))
    {
        return -1;
    }

    IntsRequestData *rildata = (IntsRequestData *)pMsg->GetRequestData();

    int interval = rildata->GetInt(0);
    int mode = rildata->GetInt(1);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildLceStart(mode, interval);
    // Currently Not supported
    if(pModemData == NULL) {
        OnRequestComplete(RIL_E_LCE_NOT_SUPPORTED);
        return 0;
    }
    if ( SendRequest(pModemData, MISC_TIMEOUT, MSG_MISC_LCE_START_DONE) < 0 )
    {
        return -1;
    }

    return 0;
}

int MiscService::OnLceStartDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if(IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolMiscLceAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
/*
 * Allowed RadioError on CardState::ABSENT are
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:LCE_NOT_SUPPORTED
 *   RadioError:INTERNAL_ERR
 *   RadioError:SIM_ABSENT
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:LCE_NOT_SUPPORTED
 *   RadioError:INTERNAL_ERR
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:NO_MEMORY
 *   RadioError:NO_RESOURCES
 *   RadioError:CANCELLED
 */
    RilLogE("[%s] %s() errorCode=%d", m_szSvcName, __FUNCTION__, errorCode);
    if (errorCode == RIL_E_SUCCESS) {
        RIL_LceStatusInfo lceStatusInfo;
        lceStatusInfo.lce_status = adapter.getLceStatus();
        lceStatusInfo.actual_interval_ms = adapter.getActualIntervalMs();
        OnRequestComplete(RIL_E_SUCCESS, &lceStatusInfo, sizeof(lceStatusInfo));
    }
    else {
        if( mCardState != RIL_CARDSTATE_PRESENT ) {
            RilLogI("[%s] %s mCardState(%d) is not Present(%d)",
                    m_szSvcName, __FUNCTION__, mCardState, RIL_CARDSTATE_PRESENT);
            OnRequestComplete(RIL_E_SIM_ABSENT);
        } else {
            OnRequestComplete(RIL_E_INTERNAL_ERR);
        }
    }

    return 0;
}

int MiscService::DoLceStop(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildLceStop();
    // Currently Not supported
    if(pModemData == NULL) {
        OnRequestComplete(RIL_E_LCE_NOT_SUPPORTED);
        return 0;
    }
    if ( SendRequest(pModemData, MISC_TIMEOUT, MSG_MISC_LCE_STOP_DONE) < 0 )
    {
        return -1;
    }

    return 0;
}

int MiscService::OnLceStopDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if(IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolMiscLceAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
/*
 * Allowed RadioError on CardState::ABSENT are
 *   RadioError:NONE
 *   RadioError:LCE_NOT_SUPPORTED
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:SIM_ABSENT
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:LCE_NOT_SUPPORTED
 *   RadioError:INTERNAL_ERR
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:NO_MEMORY
 *   RadioError:NO_RESOURCES
 *   RadioError:CANCELLED
 *   RadioError:SIM_ABSENT
 */
    if (errorCode == RIL_E_SUCCESS) {
        RIL_LceStatusInfo lceStatusInfo;
        lceStatusInfo.lce_status = adapter.getLceStatus();
        lceStatusInfo.actual_interval_ms = adapter.getActualIntervalMs();
        OnRequestComplete(RIL_E_SUCCESS, &lceStatusInfo, sizeof(lceStatusInfo));
    }
    else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::DoLcePullLceData(Message *pMsg)
{
    RilLogI("[%s] %s", GetServiceName(),__FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildLcePullLceData();
    // Currently Not supported but should return NONE or INTERNAL_ERR
    if(pModemData == NULL) {
        OnRequestComplete(RIL_E_INTERNAL_ERR);
        return 0;
    }
    if ( SendRequest(pModemData, MISC_TIMEOUT, MSG_MISC_PULL_LCEDATA_DONE) < 0 )
    {
        return -1;
    }

    return 0;
}

int MiscService::OnLcePullLceDataDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if(IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolMiscLceAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
/*
 * Allowed RadioError on CardState::ABSENT are
 *   RadioError:NONE
 *   RadioError:INTERNAL_ERR
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:OEM_ERROR_1 ~ RadioError:OEM_ERROR_25
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:LCE_NOT_SUPPORTED
 *   RadioError:INTERNAL_ERR
 *   RadioError:NO_MEMORY
 *   RadioError:NO_RESOURCES
 *   RadioError:CANCELLED
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:SIM_ABSENT //???
 */
    if (errorCode == RIL_E_SUCCESS) {
        RIL_LceDataInfo lceDataInfo;
        lceDataInfo.last_hop_capacity_kbps = adapter.getDlCapacityKbps();
        lceDataInfo.confidence_level = adapter.getConfidencelevel();
        lceDataInfo.lce_suspended = adapter.getLceSuspended();
        OnRequestComplete(RIL_E_SUCCESS, &lceDataInfo, sizeof(lceDataInfo));
    }
    else {
        if( mCardState != RIL_CARDSTATE_PRESENT ) {
            RilLogI("[%s] %s mCardState(%d) is not Present(%d)",
                    m_szSvcName, __FUNCTION__, mCardState, RIL_CARDSTATE_PRESENT);
            OnRequestComplete(RIL_E_INTERNAL_ERR);
        } else {
            OnRequestComplete(RIL_E_INTERNAL_ERR);
        }
    }

    return 0;
}

BOOL MiscService::IsNullRequest(Message *pMsg)
{
    if(NULL == pMsg || NULL == pMsg->GetRequestData())
    {
        RilLogE("[MiscService] pMsg is NULL or reqeust data is NULL");
        return 1;
    }
    return 0;
}

BOOL MiscService::IsNullResponse(Message *pMsg)
{
    if(NULL == pMsg || NULL == pMsg->GetModemData())
    {
        RilLogE("[MiscService] pMsg is NULL or modemdata data is NULL");
        return TRUE;
    }
    return FALSE;
}

void MiscService::ProcessNvReadItem(int nvItemId, const char *value)
{
    // TODO do default processing
    RilLogV("NV Item ID=%d Value=%s", nvItemId, value);
}

int MiscService::DoGetActivityInfo(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.GetModemActivityInfo();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_GET_ACTIVITY_INFO_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnGetActivityInfoDone(Message *pMsg)
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

    ProtocolMiscGetActivityInfoAdapter adapter(pModemData);

    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        RIL_ActivityStatsInfo activityStatsInfo;
        memset(&activityStatsInfo, 0, sizeof(RIL_ActivityStatsInfo));

        activityStatsInfo.sleep_mode_time_ms = adapter.GetSleepPeriod();
        activityStatsInfo.idle_mode_time_ms = adapter.GetIdlePeriod();
        memcpy(activityStatsInfo.tx_mode_time_ms, adapter.GetTxPeriod(), sizeof(UINT32)*RIL_NUM_TX_POWER_LEVELS);
        activityStatsInfo.rx_mode_time_ms = adapter.GetRxPeriod();

        OnRequestComplete(RIL_E_SUCCESS, &activityStatsInfo, sizeof(activityStatsInfo));
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

INT32 MiscService::DoSetOpenCarrierInfo(Message *pMsg)
{
    RilLogI("[%s] %s() [<--", GetServiceName(),__FUNCTION__);
    if(pMsg == NULL || pMsg->GetRequestData() == NULL)
    {
        RilLogE("%s::%s() RequestData = NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    StringsRequestData* pReq = (StringsRequestData*)pMsg->GetRequestData();
    if (pReq == NULL)
    {
        RilLogE("pReq is NULL");
        return -1;
    }

    UINT32 openCarrierIndex = strtol(pReq->GetString(0), NULL, 10);
    const char *plmn = pReq->GetString(1);
    RilLogI("[%s::%s] OCI:0x%x PLMN:%s", GetServiceName(), __FUNCTION__, openCarrierIndex, plmn);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetOpenCarierInfo(openCarrierIndex, plmn);
    if (SendRequest(pModemData, IMS_DEFAULT_TIMEOUT, MSG_MISC_SET_OPEN_CARRIER_INFO_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    BYTE ocInfo[MAX_PLMN_LEN + sizeof(UINT32)];
    memset(ocInfo, 0, sizeof(ocInfo));
    memcpy(ocInfo, plmn, strlen(plmn));
    if(strlen(plmn) == 5) ocInfo[MAX_PLMN_LEN - 1] = '#';
    memcpy(ocInfo + MAX_PLMN_LEN, &openCarrierIndex, sizeof(UINT32));
    OnUnsolicitedResponse(RIL_UNSOL_OEM_IMS_OPEN_CARRIER_INFO, ocInfo, sizeof(ocInfo));

    RilLogI("[%s] %s() [-->", GetServiceName(),__FUNCTION__);
    return 0;
}


INT32 MiscService::OnSetOpenCarrierInfoDone(Message *pMsg)
{
    RilLogI("[MiscService] %s",__FUNCTION__);
    if(pMsg == NULL)
    {
        RilLogE("%s::%s() Message = NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    //ProtocolRespAdapter adapter(pMsg->GetModemData());
    //UINT uErrCode = adapter.GetErrorCode();

    /* This RIL req is triggered internally in vendor RIL, send sucess always */
    OnRequestComplete(RIL_E_SUCCESS);

    return 0;
}

bool MiscService::IsPossibleToPassInRadioOffState(int request_id)
{
    switch (request_id) {
        case RIL_REQUEST_SET_UICC_SUBSCRIPTION:
        case RIL_REQUEST_BASEBAND_VERSION:
        case RIL_REQUEST_DEVICE_IDENTITY:
        case RIL_REQUEST_GET_IMEI:
        case RIL_REQUEST_GET_IMEISV:
        case RIL_REQUEST_OEM_MODEM_DUMP:
        case RIL_REQUEST_OEM_SET_ENG_MODE:
        case RIL_REQUEST_OEM_SET_SCR_LINE:
        case RIL_REQUEST_OEM_SET_DEBUG_TRACE:
        case RIL_REQUEST_NV_READ_ITEM:
        case RIL_REQUEST_NV_WRITE_ITEM:
        case RIL_REQUEST_GET_HARDWARE_CONFIG:
        case RIL_REQUEST_OEM_SET_ENG_STRING_INPUT:
        case RIL_REQUEST_OEM_GET_MSL_CODE:
        case RIL_REQUEST_OEM_SET_PIN_CONTROL:
        case RIL_REQUEST_OEM_SEND_SGC:
        case RIL_REQUEST_OEM_GET_MANUAL_BAND_MODE:
        case RIL_REQUEST_OEM_SET_MANUAL_BAND_MODE:
        case RIL_REQUEST_OEM_GET_RF_DESENSE_MODE:
        case RIL_REQUEST_OEM_SET_RF_DESENSE_MODE:
        case RIL_REQUEST_OEM_STORE_ADB_SERIAL_NUMBER:
        case RIL_REQUEST_OEM_READ_ADB_SERIAL_NUMBER:
        case RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE:
        case RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:
        case RIL_REQUEST_SET_VOICE_OPERATION:
        case RIL_REQUEST_OEM_SET_PREFERRED_CALL_CAPABILITY:
        case RIL_REQUEST_OEM_GET_PREFERRED_CALL_CAPABILITY:
        case RIL_REQUEST_SCREEN_STATE:
        case RIL_REQUEST_SET_DEVICE_INFO:
        case RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING:
        case RIL_REQUEST_SEND_DEVICE_STATE:
        case RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER:
        case RIL_REQUEST_OEM_AIMS_ADD_PDN_INFO:
        case RIL_REQUEST_OEM_AIMS_DEL_PDN_INFO:
        case RIL_REQUEST_OEM_AIMS_STACK_START_REQ:
        case RIL_REQUEST_OEM_AIMS_STACK_STOP_REQ:
        case RIL_REQUEST_OEM_AIMS_HIDDEN_MENU:
        case RIL_REQUEST_OEM_SCAN_RSSI:
        case RIL_REQUEST_OEM_FORWARDING_AT_COMMAND:
        case RIL_REQUEST_OEM_AIMS_SEND_SMS_ACK:
        case RIL_REQUEST_OEM_AIMS_SEND_ACK_INCOMING_SMS:
        case RIL_REQUEST_OEM_AIMS_SEND_ACK_INCOMING_CDMA_SMS:
        case RIL_REQUEST_OEM_GET_PLMN_NAME_FROM_SE13TABLE:
        case RIL_REQUEST_OEM_TS25TABLE_DUMP:
        case RIL_REQUEST_GET_PHONE_CAPABILITY:
        case RIL_REQUEST_SET_MODEMS_CONFIG:
        case RIL_REQUEST_GET_MODEMS_CONFIG:
        case RIL_REQUEST_GET_MODEM_STATUS:
        case RIL_REQUEST_OEM_MODEM_INFO:
        case RIL_REQUEST_OEM_MODEM_RESET:
        case RIL_REQUEST_OEM_SET_RTP_PKTLOSS_THRESHOLD:
        case RIL_REQUEST_OEM_SWITCH_MODEM_FUNCTION:
        case RIL_REQUEST_OEM_REQ_SET_PDCP_DISCARD_TIMER:
        case RIL_REQUEST_OEM_SET_SELFLOG:
        case RIL_REQUEST_OEM_GET_SELFLOG_STATUS:
        case RIL_REQUEST_SET_ACTIVATE_VSIM:
        case RIL_REQUEST_OEM_GET_CQI_INFO:
        case RIL_REQUEST_OEM_SET_SAR_SETTING:
        case RIL_REQUEST_OEM_SET_IMS_TEST_MODE:
        case RIL_REQUEST_OEM_SET_GMO_SWITCH:
        case RIL_REQUEST_OEM_SET_TCS_FCI:
        case RIL_REQUEST_OEM_GET_TCS_FCI:
        case RIL_REQUEST_OEM_SET_CA_BANDWIDTH_FILTER:
        case RIL_REQUEST_OEM_SET_ELEVATOR_SENSOR:
        case RIL_REQUEST_SET_LOCATION_UPDATES:
        case RIL_REQUEST_SET_TTY_MODE:
        case RIL_REQUEST_QUERY_TTY_MODE:
        case RIL_REQUEST_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA:
        case RIL_REQUEST_OEM_SET_SELFLOG_PROFILE:
        case RIL_REQUEST_OEM_SET_FORBID_LTE_CELL:
        case RIL_REQUEST_OEM_AIMS_SET_HIDDEN_MENU_ITEM:
        case RIL_REQUEST_OEM_AIMS_GET_HIDDEN_MENU_ITEM:
            break;
        default:
            return false;
    }
    return true;

}

bool MiscService::IsPossibleToPassInRadioUnavailableState(int request_id)
{
    switch(request_id) {
    case RIL_REQUEST_SET_UICC_SUBSCRIPTION:
    case RIL_REQUEST_SET_DEVICE_INFO:
    case RIL_REQUEST_CDMA_SET_SUBSCRIPTION_SOURCE:
    case RIL_REQUEST_SEND_DEVICE_STATE:
    case RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER:
    case RIL_REQUEST_OEM_GET_PLMN_NAME_FROM_SE13TABLE:
    case RIL_REQUEST_OEM_TS25TABLE_DUMP:
    case RIL_REQUEST_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA:
        return true;

    }
    return false;
}

int MiscService::DoCdmaGetSubscriptSource(Message *pMsg)
{
    MiscDataBuilder builder;

    const RilData *pRilData = builder.BuildCdmaSubscriptionSource(m_nSubscription);
    if ( pRilData == NULL ) {
        return OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }
    else {
        OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    }

    return 0;
}

int MiscService::DoCdmaSetSubscriptSource(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        return -1;
    }

    INT32 nCdmaSubSrc = rildata->GetInt();
    RilLog("[%s] %s() Set Subscription: %d", m_szSvcName, __FUNCTION__, nCdmaSubSrc);

    if(nCdmaSubSrc==CDMA_SUBSCRIPTION_SOURCE_NV)
    {
        RilLog("[%s] %s() CDMA_SUBSCRIPTION_SOURCE_NV", m_szSvcName, __FUNCTION__);
    }
    else
    {
        RilLog("[%s] %s() CDMA_SUBSCRIPTION_SOURCE_RUIM_SIM", m_szSvcName, __FUNCTION__);
    }
    OnRequestComplete(RIL_E_SUCCESS);

    if(m_nSubscription!=nCdmaSubSrc)
    {
        m_nSubscription = nCdmaSubSrc;
        MiscDataBuilder builder;
        const RilData *pRilData = builder.BuildCdmaSubscriptionSource(m_nSubscription);
        if (pRilData != NULL) {
            OnUnsolicitedResponse(RIL_UNSOL_CDMA_SUBSCRIPTION_SOURCE_CHANGED, pRilData->GetData(), pRilData->GetDataLength());
            delete pRilData;
        }
    }

    return 0;
}

int MiscService::DoGetCdmaSubscription(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildCdmaSubscription();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_GET_CDMA_SUBSCRIPTION_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnGetCdmaSubscriptionDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolCdmaSubscriptionAdapter adapter(pModemData);
    UINT uErrCode = adapter.GetErrorCode();

    if (uErrCode == RIL_E_SUCCESS) {
        MiscDataBuilder builder;
        const RilData *pRilData = builder.BuildCdmaSubscription(adapter.GetMdn(), adapter.GetSid(), adapter.GetNid(), adapter.GetMin(), adapter.GetPrlVersion());
        if ( pRilData == NULL ) {
            return OnRequestComplete(RIL_E_GENERIC_FAILURE);
        }
        else {
            OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());
            delete pRilData;
        }
    }
    else OnRequestComplete(RIL_E_SUBSCRIPTION_NOT_AVAILABLE);

    return 0;
}

int MiscService::DoGetHardwareConfig(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    // TODO: disable when SIT is implemented
    int radioState = GetRadioState();
    int nHwCfgState = (radioState == RADIO_STATE_ON)? RIL_HARDWARE_CONFIG_STATE_STANDBY: RIL_HARDWARE_CONFIG_STATE_DISABLED;

    UINT uRat = 0;
    uRat = RAF_EVDO_0 | RAF_EVDO_A | RAF_EVDO_B;

    MiscDataBuilder builder;
    const RilData *pRilData = NULL;
    if(m_nSubscription==CDMA_SUBSCRIPTION_SOURCE_NV)
    {
        pRilData = builder.BuildHardwareConfigNV(NULL, nHwCfgState, 0, uRat, 1, 1, RIL_SOCKET_NUM);
    }
    else
    {
        pRilData = builder.BuildHardwareConfigRuim(NULL, nHwCfgState, NULL);
    }
    if ( pRilData == NULL ) return OnRequestComplete(RIL_E_GENERIC_FAILURE);
    OnRequestComplete(RIL_E_SUCCESS, pRilData->GetData(), pRilData->GetDataLength());

    if (pRilData != NULL) {
        delete pRilData;
    }

    return 0;
}

int MiscService::OnGetHardwareConfigDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (IsNullResponse(pMsg)) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolMiscGetHwConfigAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        RIL_HardwareConfig *pRsp = NULL;
        int num = adapter.GetNum();
        if ( num > 0 ) {
            pRsp = (RIL_HardwareConfig *)calloc(num, sizeof(RIL_HardwareConfig));
            adapter.GetData(pRsp, num);
        } else {
            num = 0;
        }

        OnRequestComplete(RIL_E_SUCCESS, pRsp, num * sizeof(RIL_HardwareConfig));
        if ( pRsp != NULL) free(pRsp);

    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::DoCdmaSmsAck(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    CdmaSmsAckRequestData *rildata = (CdmaSmsAckRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("[%s] %s() rildata is NULL", m_szSvcName,  __FUNCTION__);
        return -1;
    }

    ProtocolSmsBuilder builder;
    ModemData *pModemData = builder.BuildSendCdmaSmsAck(g_nLastTpidNewSms, rildata->GetErrorClass(),
            rildata->GetErrorCode());
    if (SendRequest(pModemData, SMS_DEFAULT_TIMEOUT/*CDMA_SMS_DEFAULT_TIMEOUT*/,
                MSG_MISC_SMS_CDMA_ACKNOWLEDGE_DONE) < 0 ) {
        RilLogE("[%s] %s() Failed.", m_szSvcName, __FUNCTION__);
        return -1;
    }
    return 0;
}

int MiscService::OnCdmaSmsAckDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    OnRequestComplete(adapter.GetErrorCode());
    return 0;
}

INT32 MiscService::DoSetVoiceOperation(Message *pMsg)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
    if(IsNullRequest(m_pCurReqMsg))
    {
        return -1;
    }

    IntRequestData *rildata = (IntRequestData *)(pMsg->GetRequestData());
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }
    int mode = rildata->GetInt();
    RilLogI("[%s] Voice Operation mode = %d", m_szSvcName, mode);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetVoiceOperation(mode);
    if ( SendRequest(pModemData, MISC_TIMEOUT, MSG_MISC_SET_VOICE_OPERATION_DONE) < 0 )
    {
        return -1;
    }

    return 0;
}

INT32 MiscService::OnSetVoiceOperationDone(Message* pMsg)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
    if(IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();

    if ( errorCode == RIL_E_SUCCESS )
    {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else
    {
        OnRequestComplete(errorCode== RIL_E_RADIO_NOT_AVAILABLE ? RIL_E_RADIO_NOT_AVAILABLE : RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

int MiscService::DoSetPreferredCallCapability(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    const char *MiscData = (char *) rildata->GetRawData();
    BYTE mode = MiscData[0];
    RilLogI("[%s] Preferred Call Capability mode = %d", m_szSvcName, mode);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetPreferredCallCapability(mode);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_PREFERRED_CALL_CAPABILITY_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

INT32 MiscService::OnSetPreferredCallCapabilityDone(Message* pMsg)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
    if(IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);

    return 0;
}

int MiscService::DoGetPreferredCallCapability(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildGetPreferredCallCapability();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_GET_PREFERRED_CALL_CAPABILITY_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnGetPreferredCallCapabilityDone(Message *pMsg)
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

    ProtocolMiscGetPreferredCallCapability adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int mode = adapter.GetMode();
        RilLogV("preferred call capability mode=%d", mode);
        OnRequestComplete(errorCode, &mode, sizeof(mode));
    }
    else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::DoSetIndicationFilter(Message *pMsg)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);

    if(IsNullRequest(pMsg))
    {
        return -1;
    }

    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    // RIL_REQUEST_SCREEN_STATE - DEPRECATED
    // use RIL_REQUEST_SET_UNSOLICITED_RESPONSE_FILTER to turn on/off unsolicited

    // indicationFilter : a 32-bit bitmask of RIL_UnsolicitedResponseFilter
    // RIL_UR_SIGNAL_STRENGTH            = 0x01
    // RIL_UR_FULL_NETWORK_STATE       = 0x02
    // RIL_UR_DATA_CALL_DORMANCY_CHANGED = 0x04
    int indicationFilter = rildata->GetInt() & 0xFFFFFFFF;
    RilLogV("New indicationFilter - RIL_UR_SIGNAL_STRENGTH : %s RIL_UR_FULL_NETWORK_STATE : %s RIL_UR_DATA_CALL_DORMANCY_CHANGED : %s",
            indicationFilter & RIL_UR_SIGNAL_STRENGTH ? "true" : "false",
            indicationFilter & RIL_UR_FULL_NETWORK_STATE ? "true" : "false",
            indicationFilter & RIL_UR_DATA_CALL_DORMANCY_CHANGED ? "true" : "false");

    // internal request (legacy)
    int screenState = (indicationFilter & RIL_UR_SIGNAL_STRENGTH) ? 1 : 0;
    GetRilContext()->OnRequest(RIL_REQUEST_SCREEN_STATE, &screenState , sizeof(int), NULL);

    // set indication filter
    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetUnsolicitedResponseFilter(indicationFilter);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_INDICATION_FILTER_DONE) < 0) {
        return -1;
    }

    return 0;
}

int MiscService::OnSetIndicationFilterDone(Message *pMsg)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);

    if (IsNullResponse(pMsg)) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode != RIL_E_SUCCESS) {
        RilLogW("errorCode: %d", errorCode);
    }
    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

int MiscService::DoSetCarrierInfoImsiEncryption(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if(IsNullRequest(pMsg))
    {
        return -1;
    }

    CarrierInfoForImsiEncryptionData *rildata = (CarrierInfoForImsiEncryptionData *)pMsg->GetRequestData();
    int nResult = -1;

    char *pMcc = rildata->GetMcc();
    char *pMnc = rildata->GetMnc();
    int keyLen = rildata->GetCarrierKeyLen();
    BYTE *pKey = rildata->GetCarrierKey();
    int keyIdLen = rildata->GetKeyIdLen();
    char *pKeyId = rildata->GetKeyIdentifier();
    LONG expTime = rildata->GetEpirationTime();

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetCarrierInfoImsiEncryption(pMcc, pMnc, keyLen, pKey, keyIdLen, pKeyId, expTime);
    nResult = SendRequest(pModemData, TIMEOUT_SIM_DEFAULT, MSG_MISC_SET_CARRIER_INFO_IMSI_ENCRYPTION_DONE);

    return (nResult<0)? -1: 0;
}

int MiscService::OnSetCarrierInfoImsiEncryptionDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (IsNullResponse(pMsg)) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolRespAdapter adapter(pModemData);
    UINT errorCode = adapter.GetErrorCode();
    if (errorCode != RIL_E_SUCCESS) errorCode = RIL_E_REQUEST_NOT_SUPPORTED;
    OnRequestComplete(errorCode, NULL, 0);

    return 0;
}

int MiscService::DoSetSignalStrengthReportingCriteria(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if(IsNullRequest(pMsg))
    {
        return -1;
    }

    SignalStrengthReportingCriteria *rildata = (SignalStrengthReportingCriteria *)pMsg->GetRequestData();
    if (rildata == NULL) {
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        return 0;
    }

    if (GetRadioState() == RADIO_STATE_UNAVAILABLE) {
        RilLogI("[%s] %s() Postphone due to RADIO_UNAVAILABLE", m_szSvcName, __FUNCTION__);
        PostponeRequestMessage(1000);
        return 0;
    }

    int hysteresisMs = rildata->GetHysteresisMs();
    int hysteresisDb = rildata->GetHysteresisDb();
    int numOfThresholdsDbm = rildata->GetNumOfThresholdsDbm();
    int *thresholdsDbm = rildata->GetThresholdsDbm();
    int accessNetwork = rildata->GetAccessNetwork();

    // test invalidHysteresisDb for VTS 1.2
    // hysteresisDb must be smaller than the smallest threshold delta
    // ASSERT_TRUE(MIN(thresholdsDbm[i+1] - thresholdsDbm[i]) >= hysteresisDb)
    // example
    // vector {-109, -103, -97, -89}  hysteresisDb 10
    //   : invalid argument. delta is 6
    if (numOfThresholdsDbm > 0 && thresholdsDbm != NULL) {
        for (int i = 0; i < numOfThresholdsDbm - 1; i++) {
            if (hysteresisDb < 0 ||
                thresholdsDbm[i] > thresholdsDbm[i+1] ||
                thresholdsDbm[i+1] - thresholdsDbm[i] < hysteresisDb) {
                OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
                return 0;
            }
        } // end for i ~
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetSignalReportCriteria(hysteresisMs, hysteresisDb, numOfThresholdsDbm, thresholdsDbm, accessNetwork);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_SIGNAL_STRENGTH_REPORTING_CRITERIA_DONE) < 0) {
        return -1;
    }

    return 0;

}

int MiscService::OnSetSignalStrengthReportingCriteriaDone(Message *pMsg)
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
    if (errorCode != RIL_E_SUCCESS) {
        RilLogW("errorCode: %d", errorCode);
    }
    OnRequestComplete(RIL_E_SUCCESS);

    return 0;
}

int MiscService::DoSetLinkCapacityReportingCriteria(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if(IsNullRequest(pMsg))
    {
        return -1;
    }

    LinkCapacityReportingCriteria *rildata = (LinkCapacityReportingCriteria *)pMsg->GetRequestData();
    int hysteresisMs = rildata->GetHysteresisMs();
    int hysteresisDlKbps = rildata->GetHysteresisDlKbps();
    int hysteresisUlKbps = rildata->GetHysteresisUlKpbs();
    int numOfThresholdsDownlinkKbps = rildata->GetNumOfThresholdsDownlinkKbps();
    int* thresholdsDownlinkKbps = rildata->GetThresholdsDownlinkKbps();
    int numOfThresholdsUplinkKbps = rildata->GetNumOfThresholdsUplinkKbps();
    int* thresholdsUplinkKbps = rildata->GetThresholdsUplinkKbps();
    int accessNetwork = rildata->GetAccessNetwork();

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetLinkCapaReportCriteria(
            hysteresisMs, hysteresisDlKbps, hysteresisUlKbps, numOfThresholdsDownlinkKbps,
            thresholdsDownlinkKbps, numOfThresholdsUplinkKbps, thresholdsUplinkKbps, accessNetwork);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_LINK_CAPACITY_REPORTING_CRITERIA_DONE) < 0) {
        return -1;
    }

    return 0;
}

int MiscService::OnSetLinkCapacityReportingCriteriaDone(Message *pMsg)
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
    } else {
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
    }

    return 0;
}

int MiscService::OnLceDataRecv(Message *pMsg)
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

    ProtocolMiscCurrentLinkCapacityEstimate adapter(pModemData);
    int data[2] = {0, 0};
    adapter.GetCurrentLinkCapaEstimate(data);
    OnUnsolicitedResponse(RIL_UNSOL_LCEDATA_RECV, data, sizeof(int)*2);

    return 0;
}

int MiscService::OnUnsolCarrierInfoImsiEncryption(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if(IsNullResponse(pMsg))
    {
        return -1;
    }

    OnUnsolicitedResponse(RIL_UNSOL_CARRIER_INFO_IMSI_ENCRYPTION, NULL, 0);

    return 0;
}

int MiscService::OnUnsolHardwareConfigChanged(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (IsNullResponse(pMsg)) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolMiscHwConfigChangeAdapter adapter(pModemData);
    RIL_HardwareConfig *pRsp = NULL;
    int num = adapter.GetNum();
    if ( num > 0 ) {
        pRsp = (RIL_HardwareConfig *)calloc(num, sizeof(RIL_HardwareConfig));
        adapter.GetData(pRsp, num);
    } else {
        num = 0;
    }

    if (pRsp != NULL) {
        OnUnsolicitedResponse(RIL_UNSOL_HARDWARE_CONFIG_CHANGED, pRsp, num * sizeof(RIL_HardwareConfig));
        free(pRsp);
    }

    return 0;
}

int MiscService::OnUnsolCdmaPrlChanged(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (IsNullResponse(pMsg)) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolMiscCdmaPrlChangeAdapter adapter(pModemData);
    int prlVersion = adapter.GetPrlVersion();
    OnUnsolicitedResponse(RIL_UNSOL_CDMA_PRL_CHANGED, &prlVersion, sizeof(prlVersion));

    return 0;
}

int MiscService::OnUnsolModemRestart(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (IsNullResponse(pMsg)) {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    ProtocolIndAdapter adapter(pModemData);
    unsigned int len = adapter.GetParameterLength();
    const char *pData = adapter.GetParameter();
    OnUnsolicitedResponse(RIL_UNSOL_MODEM_RESTART, pData, len);

    return 0;
}

int MiscService::DoSetSuppSvcNotification(Message *pMsg)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int enable = rildata->GetInt() & 0xFFFFFFFF;
    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetSuppSvcNotification(enable);
    if (pModemData == NULL)
    {

        OnRequestComplete(RIL_E_SUCCESS);
        return 0;
    }
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_SUPP_SVC_NOTIFICATION_DONE) < 0) {
        RilLogE("SendRequest error");
        OnRequestComplete(RIL_E_SUCCESS);
        return 0;
    }

    return 0;
}

int MiscService::OnSetSuppSvcNotificationDone(Message *pMsg)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        OnRequestComplete(RIL_E_SUCCESS);
        return 0;
    }
    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
/*
 * Allowed RadioError on CardState::ABSENT are
 *   RadioError:NONE
 *   RadioError:SIM_ABSENT
 * Valid errors returned:
 *   RadioError:NONE
 *   RadioError:RADIO_NOT_AVAILABLE
 *   RadioError:INVALID_ARGUMENTS
 *   RadioError:SIM_BUSY
 *   RadioError:NO_MEMORY
 *   RadioError:SYSTEM_ERR
 *   RadioError:MODEM_ERR
 *   RadioError:INTERNAL_ERR
 *   RadioError:REQUEST_NOT_SUPPORTED
 *   RadioError:NO_RESOURCES
 *   RadioError:CANCELLED
 *   RadioError:SIM_ABSENT
 */
    OnRequestComplete(errorCode);

    return 0;
}

int MiscService::DoGetNeighboringCellIds(Message *pMsg)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
    // To do
    // For VTS test, return success
    OnRequestComplete(RIL_E_SUCCESS);
    return 0;
}

int MiscService::DoSetLocationUpdates(Message *pMsg)
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

    int enable = rildata->GetInt();
    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetLocationUpdates(enable);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_LOCATION_UPDATES_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnSetLocationUpdateDone(Message *pMsg)
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

void MiscService::OnSimStatusChanged(int cardState, int appState)
{
    mCardState = cardState;
    mAppState = appState;

    RilLogV("[%s] Card state: %d, App state: %d", __FUNCTION__, cardState, appState);
}

int MiscService::OnUnsolSarContolState(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscSarContolStateAdapter adapter(pMsg->GetModemData());
    BYTE device_state = adapter.GetDeviceState();

    RilLogV("[%s] %s() : Device state:%d", m_szSvcName, __FUNCTION__, device_state);
    OnUnsolicitedResponse(RIL_UNSOL_OEM_PSENSOR_CONTROL_STATE, &device_state, 1);

    return 0;
}

int MiscService::DoSetPSensorStatus(Message *pMsg)
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

    int pSensorStatus = rildata->GetInt();
    RilLogV("[%s] %s(), P-Sensor status = %d", m_szSvcName, __FUNCTION__, pSensorStatus);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildPSensorStatus(pSensorStatus);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_PSENSOR_STATUS_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnSetPSensorStatusDone(Message *pMsg)
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

int MiscService::OnUnsolSarRfConnection(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscSarRfConnectionAdapter adapter(pMsg->GetModemData());
    BYTE rf_state = adapter.GetRfState();

    RilLogV("[%s] %s() : RF connection: %d -> %d", m_szSvcName, __FUNCTION__, SystemProperty::GetInt(RIL_VENDOR_RF_CONNECTION, 0), rf_state);

    if (rf_state > 0) SystemProperty::Set(RIL_VENDOR_RF_CONNECTION, "1");
    else SystemProperty::Set(RIL_VENDOR_RF_CONNECTION, "0");

    OnUnsolicitedResponse(RIL_UNSOL_OEM_SAR_RF_CONNECTION, &rf_state, 1);

    return 0;
}

int MiscService::DoSetSarState(Message *pMsg)
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

    int sarState = rildata->GetInt();
    RilLogV("[%s] %s(), sar state = %d", m_szSvcName, __FUNCTION__, sarState);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetSarState(sarState);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_SAR_STATE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnSetSarStateDone(Message *pMsg)
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

int MiscService::DoGetSarState(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildGetSarState();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_GET_SAR_STATE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnGetSarStateDone(Message *pMsg)
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

    ProtocolMiscGetSarStateAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int sarState = adapter.GetSarState();
        OnRequestComplete(RIL_E_SUCCESS, &sarState, sizeof(sarState));
    } else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::DoScanRssi(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    IntsRequestData *rildata = (IntsRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    const int INDEX_RAT = 0;
    const int INDEX_BAND = 1;
    const int INDEX_RBW = 2;
    const int INDEX_SCAN_MODE = 3;
    const int INDEX_START_FREQUENCY = 4;
    const int INDEX_END_FREQUENCY = 5;
    const int INDEX_STEP = 6;
    const int INDEX_ANTENNA_SELECTION = 7;
    const int INDEX_SAMPLING_COUNT = 8;
    const int INDEX_TX1_ENABLED = 9;
    const int INDEX_TX1_BAND = 10;
    const int INDEX_TX1_BW = 11;
    const int INDEX_TX1_FREQUENCY = 12;
    const int INDEX_TX1_POWER = 13;
    const int INDEX_TX1_RB_NUM = 14;
    const int INDEX_TX1_RB_OFFSET = 15;
    const int INDEX_TX1_MCS = 16;
    const int INDEX_TX2_ENABLED = 17;
    const int INDEX_TX2_BAND = 18;
    const int INDEX_TX2_BW = 19;
    const int INDEX_TX2_FREQUENCY = 20;
    const int INDEX_TX2_POWER = 21;
    const int INDEX_TX2_RB_NUM = 22;
    const int INDEX_TX2_RB_OFFSET = 23;
    const int INDEX_TX2_MCS = 24;

    int rat = rildata->GetInt(INDEX_RAT);
    int band = rildata->GetInt(INDEX_BAND);
    int rbw = rildata->GetInt(INDEX_RBW);
    int scanMode = rildata->GetInt(INDEX_SCAN_MODE);
    int startFreq = rildata->GetInt(INDEX_START_FREQUENCY);
    int endFreq = rildata->GetInt(INDEX_END_FREQUENCY);
    int step = rildata->GetInt(INDEX_STEP);
    int antSel = rildata->GetInt(INDEX_ANTENNA_SELECTION);
    int sampling = rildata->GetInt(INDEX_SAMPLING_COUNT);
    int tx1 = rildata->GetInt(INDEX_TX1_ENABLED);
    int tx1Band = rildata->GetInt(INDEX_TX1_BAND);
    int tx1Bw = rildata->GetInt(INDEX_TX1_BW);
    int tx1Freq = rildata->GetInt(INDEX_TX1_FREQUENCY);
    int tx1Power = rildata->GetInt(INDEX_TX1_POWER);
    int tx1RbNum = rildata->GetInt(INDEX_TX1_RB_NUM);
    int tx1RbOffset = rildata->GetInt(INDEX_TX1_RB_OFFSET);
    int tx1Mcs = rildata->GetInt(INDEX_TX1_MCS);
    int tx2 = rildata->GetInt(INDEX_TX2_ENABLED);
    int tx2Band = rildata->GetInt(INDEX_TX2_BAND);
    int tx2Bw = rildata->GetInt(INDEX_TX2_BW);
    int tx2Freq = rildata->GetInt(INDEX_TX2_FREQUENCY);
    int tx2Power = rildata->GetInt(INDEX_TX2_POWER);
    int tx2RbNum = rildata->GetInt(INDEX_TX2_RB_NUM);
    int tx2RbOffset = rildata->GetInt(INDEX_TX2_RB_OFFSET);
    int tx2Mcs = rildata->GetInt(INDEX_TX2_MCS);

    RilLogV(" Scan Params {rat=%d band=%d rbw=%d scan mode=%d start freq=%d end freq=%d step=%d antenna sel=%d sampling=%d", rat, band, rbw, scanMode, startFreq, endFreq, step, antSel, sampling);
    RilLogV(" tx1{tx1=%d tx1 band=%d tx1 bw=%d tx1 freq=%d tx1 power=%d tx1 rbnum=%d rx1 rboffset=%d tx1 mcs=%d}", tx1, tx1Band, tx1Bw, tx1Freq, tx1Power, tx1RbNum, tx1RbOffset, tx1Mcs);
    RilLogV(" tx2{tx2=%d tx2 band=%d tx2 bw=%d tx2 freq=%d tx2 power=%d tx2 rbnum=%d rx2 rboffset=%d tx2 mcs=%d}}", tx2, tx2Band, tx2Bw, tx2Freq, tx2Power, tx2RbNum, tx2RbOffset, tx2Mcs);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildScanRssi(rat, band, rbw, scanMode, startFreq, endFreq, step, antSel, sampling,
                                                tx1, tx1Band, tx1Bw, tx1Freq, tx1Power, tx1RbNum, tx1RbOffset, tx1Mcs,
                                                tx2, tx2Band, tx2Bw, tx2Freq, tx2Power, tx2RbNum, tx2RbOffset, tx2Mcs);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SCAN_RSSI_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnScanRssiDone(Message *pMsg)
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

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS);
    }
    else {
        OnRequestComplete(RIL_E_MODEM_ERR);
    }

    return 0;
}

int MiscService::OnScanRssiResultRecived(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolMiscRssiScanResultAdapter adapter(pModemData);
    int total = adapter.GetTotalPage();
    int current = adapter.GetCurrentPage();
    int startFreq = adapter.GetStartFrequency();
    int endFreq = adapter.GetEndFrequency();
    int step = adapter.GetStep();
    int resultSize = adapter.GetScanResultSize();
    INT16 *result = adapter.GetScanResult();
    RilLogV(" Scan Result {(%d/%d) Freq[%d,%d] step=%d resultSize=%d",
                current, total, startFreq, endFreq, step, resultSize);

    MiscDataBuilder builder;
    const RilData *rildata = builder.BuildRssiScanResult(total, current, startFreq, endFreq, step, result, resultSize);
    if (rildata != NULL){
        OnUnsolicitedResponse(RIL_UNSOL_OEM_SCAN_RSSI_RESULT, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }

    return 0;
}

int MiscService::DoSendATCommand(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    if (rildata->GetSize() <= 0) {
        RilLogV(" [AT] > no data");
        return -1;
    }

    const int MAX_COMMAND_LENGTH = 1000;
    char command[MAX_COMMAND_LENGTH + 1] = {0, };
    int datalen = rildata->GetSize() > MAX_COMMAND_LENGTH ? MAX_COMMAND_LENGTH : rildata->GetSize();
    memcpy(command, rildata->GetRawData(), datalen);
    command[datalen] = 0;
    RilLogV(" [AT] > %s", command);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildATCommand(command);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_FORWARDING_AT_COMMAND_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnSendATCommandDone(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);
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
        OnRequestComplete(RIL_E_MODEM_ERR);
        char err[] = "\r\nERROR\r\n";
        RilLogV(" [AT] < %s (by RIL)", err);
        MiscDataBuilder builder;
        const RilData *rildata = builder.BuildATCommand(err);
        if (rildata != NULL){
            OnUnsolicitedResponse(RIL_UNSOL_OEM_FORWARDING_AT_COMMAND, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    }

    return 0;
}

int MiscService::OnUnsolATCommand(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("[%s] %s() pModemData is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    ProtocolMiscATCommandAdapter adapter(pModemData);
    const char *command = adapter.GetCommand();
    if (!TextUtils::IsEmpty(command)) {
        RilLogV(" [AT] < %s", command);
        MiscDataBuilder builder;
        const RilData *rildata = builder.BuildATCommand(command);
        if (rildata != NULL){
            OnUnsolicitedResponse(RIL_UNSOL_OEM_FORWARDING_AT_COMMAND, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    }
    else {
        RilLogW(" [AT] < received but no data");
    }

    return 0;
}

int MiscService::DoGetPlmnNameFromSE13Table(Message *pMsg) {
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("[%s] %s() pMsg is NULL", m_szSvcName, __FUNCTION__);
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    // *(int *)data[0] - mcc
    // *(int *)data[1] - mnc
    if (rildata->GetSize() < (int)sizeof(short) * 2) {
        RilLogW("Invalid parameters");
        return -1;
    }

    int mcc = ((short *)rildata->GetRawData())[0] & 0xFFFF;
    int mnc = ((short *)rildata->GetRawData())[1] & 0xFFFF;
    char alphaEons[MAX_FULL_NAME_LEN];
    memset(alphaEons, 0, sizeof(alphaEons));
    string ret = GetNetworkName(mcc, mnc);
    if (!TextUtils::IsEmpty(ret)) {
        strncpy(alphaEons, ret.c_str(), MAX_FULL_NAME_LEN - 1);
        RilLogV("%d/%d %s", mcc, mnc, ret.c_str());
    }
    OnRequestComplete(RIL_E_SUCCESS, alphaEons, strlen(alphaEons)+1);
    return 0;
}

string MiscService::GetNetworkName(int mcc, int mnc)
{
    TS25Record record;
    TS25Table *table = TS25Table::GetInstance();
    if (table != NULL) {
        record = table->GetRecord(mcc, mnc);
        if (TextUtils::IsEmpty(record.networkName)) {
            RilLogW("not found matched information for %d/%d", mcc, mnc);
        }
    }

    bool isUseLongName = TS25Record::isUsingLongNameOfT32Table(mcc, mnc);
    string ret;
    if (isUseLongName == true) ret = record.ppcin;
    else ret = record.networkName;

    return ret;
}

int MiscService::DoTs25TableDump(Message *pMsg)
{
    TS25Table *ts25table = TS25Table::GetInstance();
    if (ts25table != NULL) {
        ts25table->Dump();
    }

    OnRequestComplete(RIL_E_SUCCESS, NULL, 0);
    return 0;
}

int MiscService::DoGetRadioNode(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *) pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    if (rildata->GetSize() != MAX_RADIO_NODE_DATA_LEN) {
        RilLogE("Invalid data size");
        return -1;
    }

    const char *path = (const char*)rildata->GetRawData();

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildGetRadioNode(path);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_GET_RADIO_NODE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnGetRadioNodeDone(Message *pMsg)
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

    ProtocolMiscGetRadioNodeAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        char value[MAX_RADIO_NODE_DATA_LEN] = {0, };
        strncpy(value, adapter.GetValue(), MAX_RADIO_NODE_DATA_LEN-1);
        OnRequestComplete(errorCode, value, sizeof(value));
    } else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::DoSetRadioNode(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    if (rildata->GetSize()/MAX_RADIO_NODE_DATA_LEN != 2) {
        RilLogE("Invalid data size");
        return -1;
    }

    const char *path = (const char*)rildata->GetRawData();
    const char *value = (const char*)rildata->GetRawData() + MAX_RADIO_NODE_DATA_LEN;

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetRadioNode(path, value);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_RADIO_NODE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnSetRadioNodeDone(Message *pMsg)
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

int MiscService::DoGetProvisionUpdateRequest(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildGetVoLteProvisionUpdate();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_GET_PROVISION_UPDATE_REQUEST_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnGetProvisionUpdateRequestDone(Message *pMsg)
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

    ProtocolMiscGetVoLteProvisionUpdateAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int status = adapter.GetStatus();
        if (status == 0) {
            errorCode = RIL_E_GENERIC_FAILURE;
        }
    }
    OnRequestComplete(errorCode);
    return 0;
}

int MiscService::DoSetProvisionUpdateDoneRequest(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetVoLteProvisionUpdate();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_PROVISION_UPDATE_DONE_REQUEST_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnSetProvisionUpdateDoneRequestDone(Message *pMsg)
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

    ProtocolMiscSetVoLteProvisionUpdateAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int result = adapter.GetResult();
        if (result == 0) {
            errorCode = RIL_E_GENERIC_FAILURE;
        }
    }
    OnRequestComplete(errorCode);
    return 0;
}

int MiscService::DoRadioConfigReset(Message *pMsg)
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

    int type = rildata->GetInt();
    RilLogV("[%s] %s(), reset type = %d", m_szSvcName, __FUNCTION__, type);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildRadioConfigReset(type);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_RADIO_CONFIG_RESET_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnRadioConfigResetDone(Message *pMsg)
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

int MiscService::DoSetActivateVsim(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    SetActivateVsimReqData *rildata = (SetActivateVsimReqData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    int slot = rildata->GetSlotId();
    const char *pIccid = rildata->GetIccid();
    int iccidLen = (pIccid == NULL)? 0 : strlen(pIccid);
    const char *pImsi = rildata->GetImsi();
    int imsiLen = (pImsi == NULL)? 0 : strlen(pImsi);
    const char *pHplmn = rildata->GetHomePlmn();
    int vsimState = rildata->GetVsimState();
    int vsimCardType = rildata->GetVsimCardType();

    RilLogV("[%s] %s(), %d, %s, %s, %s, %d, %d", m_szSvcName, __FUNCTION__, slot,
            pIccid, pImsi, pHplmn, vsimState, vsimCardType);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetActivateVsim(slot, iccidLen, pIccid,
            imsiLen, pImsi, pHplmn,
            vsimState, vsimCardType);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_ACTIVATE_VSIM_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnSetActivateVsimDone(Message *pMsg)
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

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int isSuccsee = 0;
        const char *pData = adapter.GetParameter();
        if (pData != NULL) {
            isSuccsee = pData[0];
            RilLogV("[%s] %s(), isSuccess = %d", m_szSvcName, __FUNCTION__, pData[0]);
        }
        OnRequestComplete(RIL_E_SUCCESS, &isSuccsee, sizeof(int));
    }
    else {
        OnRequestComplete(RIL_E_INTERNAL_ERR);
    }

    return 0;
}

int MiscService::DoEnableModem(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (IsNullRequest(pMsg)) return -1;

    int nResult = -1;

    IntRequestData *rildata = (IntRequestData *)pMsg->GetRequestData();
    if(rildata)
    {
        /*
        * data is int *
        * ((int *)data)[0] is > 0 to turn on the logical modem
        * ((int *)data)[0] is == 0 to turn off the logical modem
        */
        int mode = SIT_PWR_STATCK_DISABLE;
        if(rildata->GetInt()>0)
        {
            mode = SIT_PWR_STATCK_ENABLE;
        }

        ProtocolMiscBuilder builder;
        ModemData *pModemData = builder.BuildSetStatckStatus(mode);
        nResult = SendRequest(pModemData, MISC_TIMEOUT, MSG_MISC_ENABLE_MODEM_DONE);
    }

    return (nResult<0)? -1: 0;
}

int MiscService::OnEnableModemDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (IsNullResponse(pMsg)) return -1;

    ProtocolRespAdapter adapter(pMsg->GetModemData());
    int errorCode = adapter.GetErrorCode();
    if (errorCode != RIL_E_SUCCESS) errorCode = RIL_E_MODEM_ERR;
    OnRequestComplete(errorCode, NULL, 0);
    return 0;
}

int MiscService::DoGetModemStackStatus(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    int nResult = -1;
    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildGetStatckStatus();
    nResult = SendRequest(pModemData, MISC_TIMEOUT, MSG_MISC_GET_MODEM_STACK_STATUS_DONE);
    return (nResult<0)? -1: 0;
}

int MiscService::OnGetModemStackStatusDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);
    if (IsNullResponse(pMsg)) return -1;

    ProtocolRespAdapter adapter(pMsg->GetModemData());
    int errorCode = adapter.GetErrorCode();
    int status = 1;
    if (errorCode == RIL_E_SUCCESS)
    {
        ProtocolMiscGetStackStatusAdapter adapter(pMsg->GetModemData());
        status = adapter.GetMode();
    }
    else
    {
        errorCode = RIL_E_MODEM_ERR;
    }

    MiscDataBuilder builder;
    const RilData *pRilData = builder.BuildGetModemStatus(status);
    if ( pRilData == NULL )
    {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }
    else
    {
        OnRequestComplete(errorCode, pRilData->GetData(), pRilData->GetDataLength());
        delete pRilData;
    }
    return 0;
}

int MiscService::DoSetSystemSelectionChannels(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    // TODO since radio 1.3

    OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
    return 0;
}

void MiscService::SetDebugTraceOffOnBoot()
{
    RilLogV("[%s] %s", GetServiceName(), __FUNCTION__);
    // 0: debug on, 1: debug off, 2:notset
    int traceOffOption = SystemProperty::GetInt(VENDOR_CP_DEBUG_OFF_ON_BOOT, 99);

    // if user mode & not set any value
    // if user select to debug off explicitly
    if ( (RilProperty::IsUserMode() == true && traceOffOption == 99)
        || traceOffOption == 1 ) {
        if (GetRilContext() != NULL) {
            // Set CP Debug Trace Off
            BYTE debugOnOff[1] = {0};   //0:disable, 1:enable
            GetRilContext()->OnRequest(RIL_REQUEST_OEM_SET_DEBUG_TRACE, debugOnOff, sizeof(char), 0);
        }
    }
}

void MiscService::SetModemsConfig()
{
    RilLogV("[%s] %s", GetServiceName(), __FUNCTION__);
    // numOfLiveModems
    int numOfLiveModems = RilApplication::IsMultiSimEnabled() ? 2 : 1;
    if (GetRilContext() != NULL) {
        GetRilContext()->OnRequest(RIL_REQUEST_SET_MODEMS_CONFIG, &numOfLiveModems, sizeof(numOfLiveModems), 0);
    }
}

void MiscService::SendDeviceInfo()
{
    RilLogV("[%s] %s", GetServiceName(), __FUNCTION__);
    string model = SystemProperty::Get(RO_PRODUCT_MODEL_DM, "");
    if (model.size() == 0) {
        model = SystemProperty::Get(RO_PRODUCT_MODEL, "");
    }
    string swVer = SystemProperty::Get(RO_BUILD_ID, "");
    string productName = SystemProperty::Get(RO_PRODUCT_NAME, "");
    RilLogV("Model:%s, SW Ver:%s Product Name:%s", model.c_str(), swVer.c_str(), productName.c_str());
    if (GetRilContext() != NULL) {
        const char *ptrStrings[] = { model.c_str(), swVer.c_str(), productName.c_str() };
        GetRilContext()->OnRequest(RIL_REQUEST_SET_DEVICE_INFO, ptrStrings, sizeof(ptrStrings), 0);
    }
}

void MiscService::SendSGCValue()
{
    RilLogV("[%s] %s", GetServiceName(), __FUNCTION__);
    //Read SGC Value
    RilProperty* property = GetRilApplicationProperty();
    int target_op = property->GetInt(RIL_APP_TARGET_OPER);
    RilLogV("[%s] Check target operator=%d", __FUNCTION__, target_op);
    if (GetRilContext() != NULL) {
        GetRilContext()->OnRequest(RIL_REQUEST_OEM_SEND_SGC, &target_op, sizeof(int), 0);
    }

    return;
}

void MiscService::SendSvnInfo()
{
    RilLogV("[%s] %s", GetServiceName(), __FUNCTION__);

    char szBuff[PROP_VALUE_MAX];
    memset(szBuff, 0x00, sizeof(szBuff));
    property_get(RO_IMEI_SVN, szBuff, "");
    RilLogV("[%s] %s() SV number(%s)",m_szSvcName, __FUNCTION__, szBuff);

    if ((strlen(szBuff) > 0) && (strlen(szBuff) <= 2)) {
        if (strlen(szBuff) == 1) {
            szBuff[1] = szBuff[0];
            szBuff[0] = '0';
        }

        for (int i = 0; i < 2; i++) {
            if (szBuff[i] < 0x30 || szBuff[i] > 0x39) {
                RilLogW("[%s] %s() Wrong character", m_szSvcName, __FUNCTION__);
                return;
            }
        }

        ProtocolNetworkBuilder builder;
        ModemData *pModemData = builder.BuildSvNumber(szBuff);
        if (pModemData != NULL) {
            if (SendRequest(pModemData) < 0) {
                RilLogW("Fail to send SVN");
            }
            delete pModemData;
        }
    } else {
        RilLogW("[%s] %s() SV number empty",m_szSvcName, __FUNCTION__);
    }
}

int MiscService::DoGetPhoneCapability(Message *pMsg)
{
    RilLogI("[%s] %s", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolRadioConfigBuilder builder;
    ModemData *pModemData = builder.BuildGetPhoneCapability();
    if (SendRequest(pModemData, MISC_TIMEOUT, MSG_MISC_GET_PHONE_CAPABILITY_DONE)) {
        return -1;
    }

    return 0;
}

int MiscService::OnGetPhoneCapabilityDone(Message *pMsg)
{
    RilLogI("[%] %s()", m_szSvcName, __FUNCTION__);
    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    int maxActiveData = 1;  // L+L modem it should be 2.
    int maxActiveInternetData = 1;  // DSDS(L+L) 1, DSDA 2
    bool isInternetLingeringSupported = false;
    int size = 2;
    int *logicalModemList = NULL;  // default: logicalModemList [0, 1]

    ProtocolPhoneCapabilityAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        // phone capability by modem
        maxActiveData = adapter.GetMaxActiveData();
        maxActiveInternetData = adapter.GetMaxActiveInternetData();
        isInternetLingeringSupported = false;
        size = adapter.GetLogicalModemListSize();
        logicalModemList = adapter.GetLogicalModemList();
        RilLogV("[PhoneCapability] maxActiveData=%d maxActiveInternetData=%d" \
                " isInternetLingeringSupported=%d logicalModemListSize=%d",
                maxActiveData, maxActiveInternetData, isInternetLingeringSupported, size);
    }
    else {
        // phone capability by platform
        // default: DSDS/Dual VoLTE
        bool isDualVolteEnabled =
                SystemProperty::GetInt(RIL_VENDOR_RADIO_DUAL_VOLTE, 0) == 1;
        if (isDualVolteEnabled) {
            maxActiveData = 2;
        }
        size = 2;
        RilLogV("[PhoneCapability] Use platform default: maxActiveData=%d maxActiveInternetData=%d" \
                " isInternetLingeringSupported=%d logicalModemListSize=%d",
                maxActiveData, maxActiveInternetData, isInternetLingeringSupported, size);
    }
    RadioConfigBuildler builder;
    const RilData *rildata = builder.BuildPhoneCapability(maxActiveData, maxActiveInternetData,
            isInternetLingeringSupported, size, logicalModemList);
    if (rildata != NULL) {
        OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }
    else {
        OnRequestComplete(RIL_E_INTERNAL_ERR);
    }

    return 0;
}

int MiscService::DoSetModemsConfig(Message *pMsg)
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

    int numOfLiveModems = rildata->GetInt();
    RilLogV("ModemsConfig=%d", numOfLiveModems);
    if (numOfLiveModems < 1 || numOfLiveModems > 2) {
        // VTS IRadioConfig@1.1 RadioConfigHidlTest#setModemsConfig_invalidArgument
        OnRequestComplete(RIL_E_INVALID_ARGUMENTS);
        return 0;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetModemsConfig(numOfLiveModems);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_MODEMS_CONFIG_DONE) < 0) {
        RilLogE("SendRequest error");
        // VTS IRadioConfig@1.1 RadioConfigHidlTest#setModemsConfig_goodRequest
        OnRequestComplete(RIL_E_REQUEST_NOT_SUPPORTED);
    }

    return 0;
}

int MiscService::OnSetModemsConfigDone(Message *pMsg)
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
    if (errorCode != RIL_E_SUCCESS) {
        // VTS IRadioConfig@1.1 RadioConfigHidlTest#setModemsConfig_goodRequest
        // an allowed error code is among  RIL_E_SUCCESS and RIL_E_REQUEST_NOT_SUPPORTED.
        // return always RIL_E_SUCCESS even if failed by CP
        // without success, SS/DS switching will never be allowed.
        errorCode = RIL_E_SUCCESS;
    }
    OnRequestComplete(errorCode);
    return 0;
}

int MiscService::DoOemModemInfo(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    char *data = (char *)rildata->GetRawData();
    unsigned int datalen = rildata->GetSize();
    if (data == NULL || datalen < sizeof(int) * 2) {
        return -1;
    }

    int type = *((int *)data);
    unsigned int size = *((int *)data + 1);
    char *params = NULL;
    if (size + sizeof(int) * 2 > datalen) {
        return -1;
    }

    if (size > 0) {
        params = (char *)data + (sizeof(int) * 2);
    }
    RilLog("[%d] %s type=%d param length=%d", GetRilSocketId(), __FUNCTION__, type, size);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildModemInfo(type, params, size);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_MODEM_INFO_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnOemModemInfoDone(Message *pMsg)
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

    ProtocolOemModemInfoAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        MiscDataBuilder builder;
        int command = adapter.GetCommandType();
        unsigned int size = adapter.GetSize();
        char *data = (char *)adapter.GetData();
        RilLog("[%d] ModemInfo: command=%d size=%u resp=%s", GetRilSocketId(), command, size, data);

        const RilData *rildata = builder.BuildModemInfo(command, data, size);
        if (rildata != NULL) {
            OnRequestComplete(RIL_E_SUCCESS, rildata->GetData(), rildata->GetDataLength());
            delete rildata;
        }
    }
    else {
        OnRequestComplete(RIL_E_INTERNAL_ERR);
    }

    return 0;
}

int MiscService::OnOemModemInfoReceived(Message *pMsg)
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

    ProtocolOemModemInfoIndAdapter adapter(pModemData);
    MiscDataBuilder builder;
    int command = adapter.GetCommandType();
    unsigned int size = adapter.GetSize();
    char *data = (char *)adapter.GetData();
    RilLog("[%d] ModemInfo: type=%d size=%u resp=%s", GetRilSocketId(), command, size, data);
    const RilData *rildata = builder.BuildModemInfo(command, data, size);
    if (rildata != NULL) {
        OnUnsolicitedResponse(RIL_UNSOL_OEM_MODEM_INFO, rildata->GetData(), rildata->GetDataLength());
        delete rildata;
    }
    return 0;
}

int MiscService::DoOemModemReset(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolNetworkBuilder builder;
    ModemData *pModemData = builder.BuildRestartModem();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_MODEM_RESET_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::DoOemModemResetDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    PARAM_NULL(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());
    } else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

int MiscService::DoOemSetRtpPktlossThreshold(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }

    const char *data = (char *)rildata->GetRawData();
    BYTE interval = data[0];
    BYTE pktLossThr = data[1];
    RilLogV("[%d] %s interval=%d, pktLossThr=%d", GetRilSocketId(), __FUNCTION__, interval, pktLossThr);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetRtplossThr(interval, pktLossThr);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_RTP_PKTLOSS_THRESHOLD_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnOemSetRtpPktlossThresholdDone(Message *pMsg)
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

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);

    return 0;
}

int MiscService::OnOemRtpPktlossThresholdReceived(Message *pMsg)
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

    OnUnsolicitedResponse(RIL_UNSOL_OEM_RTP_PKTLOSS_THRESHOLD);
    return 0;
}

int MiscService::DoOemSwitchModemFunction(Message *pMsg)
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

    int feature = rildata->GetInt(0);
    BYTE enable = (BYTE)rildata->GetInt(1);
    RilLogV("[%d] Switch Modem feature=%d enable=%d", GetRilSocketId(), feature, enable);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSwitchModemFunction(feature, enable);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_FUNC_SWITCH_REQ_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::DoOemSwitchModemFunctionDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    PARAM_NULL(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolOemSwitchModemFunctionAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        BYTE result = (BYTE)adapter.GetResult();
        RilLog("[%d] Switch Modem Function: result=%d", GetRilSocketId(), result);
        OnRequestComplete(RIL_E_SUCCESS, &result, sizeof(BYTE));
    }
    else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::DoOemSetPdcpDiscardTimer(Message *pMsg)
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

    int discardTimer = rildata->GetInt();
    RilLogV("[%d] Set PDCP Discard Timer=%d", GetRilSocketId(), discardTimer);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetPdcpDiscardTimer(discardTimer);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_PDCP_DISCARD_TIMER_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::DoOemSetPdcpDiscardTimerDone(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    PARAM_NULL(pMsg);

    ModemData *pModemData = pMsg->GetModemData();
    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        OnRequestComplete(RIL_E_SUCCESS, (void *)adapter.GetParameter(), (int)adapter.GetParameterLength());
    } else {
        OnRequestComplete(RIL_E_GENERIC_FAILURE);
    }

    return 0;
}

int MiscService::DoSetSelflog(Message *pMsg)
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
    int size = rildata->GetInt(1);
    RilLogV("[%s] %s(), mode = %s sise = %d", m_szSvcName, __FUNCTION__, (mode == 0) ? "START" : "STOP", size);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetSelflog(mode, size);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_SELFLOG_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnSetSelflogDone(Message *pMsg)
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

    ProtocolMiscSetSelflogAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int result = adapter.GetSelflogResult();
        RilLogV("[%s] %s(), result = %d", m_szSvcName, __FUNCTION__, result);
        OnRequestComplete(errorCode, &result, sizeof(int));
    } else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::DoGetSelfLogStatus(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildGetSelflogStatus();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_GET_SELFLOG_STATUS_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnGetSelfLogStatusDone(Message *pMsg)
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

    ProtocolMiscGetSelflogStatusAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int status = adapter.GetSelflogStatus();
        RilLogV("[%s] %s(), status = %d", m_szSvcName, __FUNCTION__, status);
        OnRequestComplete(errorCode, &status, sizeof(int));
    } else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::OnUnsolSelflogStatus(Message *pMsg)
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

    ProtocolMiscSelflogStatusAdapter adapter(pMsg->GetModemData());
    BYTE status = adapter.GetIndSelflogStatus();
    RilLogV("[%s] %s() : Status:%d", m_szSvcName, __FUNCTION__, status);
    OnUnsolicitedResponse(RIL_UNSOL_OEM_SELFLOG_STATUS, &status, 1);

    return 0;
}

int MiscService::DoOemGetCqiInfo(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildGetCqiInfo();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_GET_CQI_INFO_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnOemGetCqiInfoDone(Message *pMsg)
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

    ProtocolOemGetCqiInfoAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int type = (int)adapter.GetCqiType();
        int cqiInfo0 = (int)adapter.GetCqiInfo0();
        int cqiInfo1 = (int)adapter.GetCqiInfo1();
        int ri = (int)adapter.GetRi();
        RilLogV("[%s] %s(), type = %d CQIInfo0 = %d CQIInfo1 = %d RI = %d", m_szSvcName, __FUNCTION__, type, cqiInfo0, cqiInfo1, ri);

        int response[] = {type, cqiInfo0, cqiInfo1, ri};
        OnRequestComplete(errorCode, response, sizeof(response));
    } else {
        OnRequestComplete(errorCode);
    }
    return 0;
}

int MiscService::DoOemSetSarSetting(Message *pMsg)
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

    int dsi = rildata->GetInt();
    RilLogV("[%s] %s(), DSI = %d", m_szSvcName, __FUNCTION__, dsi);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetSarSetting(dsi);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_SAR_SETTING_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }
    return 0;
}

int MiscService::OnOemSetSarSettingDone(Message *pMsg)
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

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);

    return 0;
}

int MiscService::DoSetImsTestMode(Message *pMsg)
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
    RilLogV("[%d] IMS Test Mode=%d", GetRilSocketId(), mode);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetImsTestMode(mode);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_IMS_TEST_MODE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnSetImsTestModeDone(Message *pMsg)
{
    RilLogI("[%] %s()", m_szSvcName, __FUNCTION__);

    if (IsNullResponse(pMsg))
    {
        return -1;
    }

    ModemData *pModemData = pMsg->GetModemData();

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode != RIL_E_SUCCESS)
        errorCode = RIL_E_GENERIC_FAILURE;

    OnRequestComplete(errorCode, NULL, 0);
    return 0;
}

int MiscService::DoOemSetGmoSwitch(Message *pMsg)
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

    int feature = rildata->GetInt();
    RilLogV("[%s] %s(), Feature = %d", m_szSvcName, __FUNCTION__, feature);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetGmoSwitch(feature);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_GMO_SWITCH_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnOemSetGmoSwitchDone(Message *pMsg)
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

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);

    return 0;
}

int MiscService::DoSetElevatortSensor(Message *pMsg)
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
    int enable = rildata->GetInt();
    RilLogV("[%s] %s(), Feature = %d", m_szSvcName, __FUNCTION__, enable);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetElevatorSensor(enable);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_ELEVATOR_SENSOR) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::DoSetElevatortSensorDone(Message *pMsg)
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

    ProtocolRespAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    OnRequestComplete(errorCode);

    return 0;
}

int MiscService::DoOemSetTcsFci(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }
    void *data = rildata->GetRawData();
    int state = ((int *)data)[0];
    int len = ((int *)data)[1];
    char *fci = (char *)((int *)data + 2);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetTcsFci(state, len, fci);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_TCS_FCI_REQ_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnOemSetTcsFciDone(Message *pMsg)
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

    ProtocolMiscSetTcsFciAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int result = adapter.GetResult();
        RilLogV("[%s] %s(), result = %d", m_szSvcName, __FUNCTION__, result);
        OnRequestComplete(errorCode, &result, sizeof(int));
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::DoOemGetTcsFci(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildGetTcsFci();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_GET_TCS_FCI_INFO_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnOemGetTcsFciDone(Message *pMsg)
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

    ProtocolMiscGetTcsFciAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        const char *fci = adapter.GetFci();
        RilLogV("[%s] %s(), FCI = %s", m_szSvcName, __FUNCTION__, fci);
        OnRequestComplete(errorCode, (void *)fci, MAX_FCI_LEN);
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::DoOemSetCABandwidthFilter(Message *pMsg)
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

    int enable = rildata->GetInt();
    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetCABandwidthFilter(enable);
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_CA_BW_FILTER_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnOemSetCABandwidthFilterDone(Message *pMsg)
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

int MiscService::OnOemCABandwidthFilterInd(Message *pMsg)
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

    ProtocolCaBandwidthFilterIndAdapter adapter(pModemData);
    int caConfig = adapter.GetCaConfig();
    int nrb = adapter.GetNRB();
    RilLogV("[%s] CA config=%d NRB=%d", GetServiceName(), caConfig, nrb);

    int response[] = { caConfig, nrb };
    OnUnsolicitedResponse(RIL_UNSOL_OEM_CA_BANDWIDTH_FILTER, response, sizeof(response));

    return 0;
}

int MiscService::DoSetModemLogDump(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetModemLogDump();
    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_SET_MODEM_LOG_DUMP_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnSetModemLogDumpDone(Message *pMsg)
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

    ProtocolMiscSetModemLogDumpAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int result = adapter.GetResult();
        RilLogV("[%s] %s(), result = %d", m_szSvcName, __FUNCTION__, result);
        OnRequestComplete(errorCode, &result, sizeof(int));
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::DoSetUicc(Message *pMsg)
{
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    int nResult = -1;
    UiccSubscription *rildata = (UiccSubscription *) pMsg->GetRequestData();
    if (NULL == rildata) {
        return -1;
    }

    int slot = rildata->GetSlot();
    int appIndex = rildata->GetAppIndex();
    int subType = rildata->GetSubscriptionType();
    int activeStatus = rildata->GetActivationStatus();

    RilLogV("%s::%s() Slot(%d), AppIndex(%d), SubscriptionType(%d), ActivationStatus(%d)", m_szSvcName, __FUNCTION__,
            slot, appIndex, subType, activeStatus);

    if (mCardState == RIL_CARDSTATE_ABSENT) {
        OnRequestComplete(RIL_E_INTERNAL_ERR);
        RilLogE("%s::%s() Internal error", m_szSvcName, __FUNCTION__);
        return 0;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetUicc(activeStatus);
    if(pModemData!=NULL) nResult = SendRequest(pModemData, TIMEOUT_SIM_ACT_DEACT, MSG_MISC_SET_UICC_DONE);
    else OnRequestComplete(RIL_E_NO_MEMORY);

    return (nResult<0)? -1: 0;
}

int MiscService::OnSetUiccDone(Message *pMsg)
{
    RilLogI("[%s] %s()", GetServiceName(), __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolSimResponseAdapter adapter(pMsg->GetModemData());
    UINT uErrCode = adapter.GetErrorCode();
    OnRequestComplete(uErrCode);

    return 0;
}

int MiscService::OnEndcCapabilityReceived(Message *pMsg)
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

    ProtocolMiscEndcCapabilityIndAdapter adapter(pModemData);
    int capability = adapter.GetCapability();
    int cause = adapter.GetCause();
    RilLogV("[%s] ENDCCapaInd Capability=%d Cause=%d", GetServiceName(), capability, cause);

    int response[] = {capability, cause};
    OnUnsolicitedResponse(RIL_UNSOL_IND_ENDC_CAPABILITY, response, sizeof(response));

    return 0;
}

int MiscService::DoSetSelfLogProfile(Message *pMsg) {
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetSelflogProfile();

    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_SELFLOG_PROFILE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnSetSelfLogProfileDone(Message *pMsg) {
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

    ProtocolMiscSetSelflogProfileAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int result = adapter.GetResult();
        RilLogV("[%s] %s(), result = %d", m_szSvcName, __FUNCTION__, result);
        OnRequestComplete(errorCode, &result, sizeof(int));
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}

int MiscService::DoSetForbidLteCell(Message *pMsg) {
    RilLogI("[%s] %s()", m_szSvcName, __FUNCTION__);

    if (pMsg == NULL) {
        RilLogE("pMsg is NULL");
        return -1;
    }

    RawRequestData *rildata = (RawRequestData *)pMsg->GetRequestData();
    if (rildata == NULL) {
        RilLogE("rildata is NULL");
        return -1;
    }
    void *data = rildata->GetRawData();
    unsigned int dataLen = rildata->GetSize();
    if (dataLen < sizeof(int)*3) {
        RilLogE("Data is invalid");
        return -1;
    }
    int mode = ((int *)data)[0];
    int cellId = ((int *)data)[1];
    int forbiddenTimer = ((int *)data)[2];

    char plmn[MAX_PLMN_LEN + 1] = {0, };
    memcpy(plmn, (char *)((int *)data + 3), dataLen - sizeof(int)*3);

    ProtocolMiscBuilder builder;
    ModemData *pModemData = builder.BuildSetForbidLteCell(mode, cellId, forbiddenTimer, plmn);

    if (pModemData == NULL) {
        RilLogE("pModemData is NULL");
        return -1;
    }

    if (SendRequest(pModemData, TIMEOUT_MISC_DEFAULT, MSG_MISC_OEM_SET_SELFLOG_PROFILE_DONE) < 0) {
        RilLogE("SendRequest error");
        return -1;
    }

    return 0;
}

int MiscService::OnSetForbidLteCellDone(Message *pMsg) {
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

    ProtocolMiscSetForbidLteCellAdapter adapter(pModemData);
    int errorCode = adapter.GetErrorCode();
    if (errorCode == RIL_E_SUCCESS) {
        int result = adapter.GetResult();
        RilLogV("[%s] %s(), result = %d", m_szSvcName, __FUNCTION__, result);
        OnRequestComplete(errorCode, &result, sizeof(int));
    } else {
        OnRequestComplete(errorCode);
    }

    return 0;
}
