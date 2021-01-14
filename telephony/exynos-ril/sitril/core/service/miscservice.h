 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __MISCSERVICE_H__
#define __MISCSERVICE_H__

#include "service.h"

#define TIMEOUT_MISC_DEFAULT        5000
#define SMS_DEFAULT_TIMEOUT        10000

class ServiceMgr;
class Message;
class RequestData;
class ModemData;
class DtmfInfo;
class OperatorContentValue;

class MiscService : public Service
{
public:
    MiscService(RilContext* pRilContext);
    virtual ~MiscService();

protected:
    static const int MISC_TIMEOUT = 5000;
    static const int MISC_DTMF_TIMEOUT = 15000;
    static const int IMS_DEFAULT_TIMEOUT = 5000;

    int m_nSubscription;
    bool mDontReportOemSignalStrength;
    RIL_SignalStrength_V1_4 *mCurrentSignalStrength;
    uint64_t mLastReceivedTimestamp;

    virtual int OnCreate(RilContext *pRilContext);
    virtual void OnStart();

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleInternalMessage(Message* pMsg);

    virtual void OnModemStateChanged(int state);
    virtual void OnRadioOffOrNotAvailable();
    virtual void OnRadioAvailable();
    virtual bool IsPossibleToPassInRadioOffState(int request_id);
    virtual bool IsPossibleToPassInRadioUnavailableState(int request_id);

protected:
    virtual int DoBaseBandVersion(Message *pMsg);
    virtual int OnBaseBandVersionDone(Message *pMsg);

    virtual int DoSignalStrength(Message *pMsg);
    virtual int OnSignalStrengthDone(Message *pMsg);

    virtual int OnSetTtyModeDone(Message *pMsg);
    virtual int DoSetTtyMode(Message *pMsg);
    virtual int DoGetTtyMode(Message *pMsg);
    virtual int OnGetTtyModeDone(Message *pMsg);

    virtual int DoScreenState(Message *pMsg);
    virtual int OnScreenStateDone(Message *pMsg);
    virtual int DoSendDeviceState(Message *pMsg);

    virtual int DoIMEI(Message *pMsg);
    virtual int OnIMEIDone(Message *pMsg);
    virtual int DoIMEISV(Message *pMsg);
    virtual int OnIMEISVDone(Message *pMsg);
    virtual int DoDevIdentity(Message *pMsg);
    virtual int OnDevIdentityDone(Message *pMsg);

    virtual int DoSetEngMode(Message *pMsg);
    virtual int OnOEMSetEngModeDone(Message *pMsg);
    virtual int DoSetScrLine(Message *pMsg);
    virtual int OnOEMSetScrLineDone(Message *pMsg);
    virtual int OnOEMSetDebugTraceDone(Message *pMsg);
    virtual int OnOEMSetEngStringInputDone(Message *pMsg);
    virtual int OnOEMSetPinControlDone(Message *pMsg);
    virtual int DoSetDebugTrace(Message *pMsg);
    virtual int DoSetCarrierConfig(Message *pMsg);
    virtual int DoSetEngStringInput(Message *pMsg);
    virtual int DoApnSettings(Message *pMsg);
    virtual int DoOemGetMslCode(Message *pMsg);
    virtual int OnOemGetMslCodeDone(Message *pMsg);
    virtual int DoSetPinControl(Message *pMsg);
    virtual int DoGetManualBandMode(Message *pMsg);
    virtual int OnGetManualBandModeDone(Message *pMsg);
    virtual int DoSetManualBandMode(Message *pMsg);
    virtual int OnSetManualBandModeDone(Message *pMsg);
    virtual int DoGetRfDesenseMode(Message *pMsg);
    virtual int OnGetRfDesenseModeDone(Message *pMsg);
    virtual int DoSetRfDesenseMode(Message *pMsg);
    virtual int OnSetRfDesenseModeDone(Message *pMsg);
    virtual int DoStoreAdbSerialNumber(Message *pMsg);
    virtual int OnStoreAdbSerialNumberDone(Message *pMsg);
    virtual int DoReadAdbSerialNumber(Message *pMsg);
    virtual int OnReadAdbSerialNumberDone(Message *pMsg);
    virtual int DoSetPreferredCallCapability(Message *pMsg);
    virtual int OnSetPreferredCallCapabilityDone(Message *pMsg);
    virtual int DoGetPreferredCallCapability(Message *pMsg);
    virtual int OnGetPreferredCallCapabilityDone(Message *pMsg);

    virtual int DoOEMSysDump(Message *pMsg);
    virtual int DoOEMNotifyCPCrash(Message *pMsg);
    virtual int DoOEMNotifySilentReset(Message *pMsg);

    virtual INT32 DoDtmfStart();
    virtual INT32 DoDtmfStartDone(Message* pMsg);
    virtual INT32 DoDtmf();
    virtual INT32 DoDtmfDone(Message* pMsg);
    virtual INT32 DoDtmfStop();
    virtual INT32 DoDtmfStopDone(Message* pMsg);

    virtual int DoSmsAck(Message *pMsg);
    virtual int OnSmsAckDone(Message *pMsg);
    virtual int DoSmsAckWithPdu(Message *pMsg);
    virtual int OnSmsAckWithPduDone(Message *pMsg);

    virtual int DoAimsDefaultRequestHandler(Message *pMsg);
    virtual int OnAimsDefaultResponseHandler(Message *pMsg);

    virtual int DoNvReadItem(Message *pMsg);
    virtual int OnNvReadItemDone(Message *pMsg);
    virtual int DoNvWriteItem(Message *pMsg);
    virtual int OnNvWriteItemDone(Message *pMsg);
    virtual void ProcessNvReadItem(int nvItemId, const char *value);

    virtual INT32 DoSetOpenCarrierInfo(Message *pMsg);
    virtual INT32 OnSetOpenCarrierInfoDone(Message *pMsg);

    virtual int DoLceStart(Message *pMsg);
    virtual int OnLceStartDone(Message *pMsg);
    virtual int DoLceStop(Message *pMsg);
    virtual int OnLceStopDone(Message *pMsg);
    virtual int DoLcePullLceData(Message *pMsg);
    virtual int OnLcePullLceDataDone(Message *pMsg);

    BOOL IsNullRequest(Message *pMsg);
    BOOL IsNullResponse(Message *pMsg);

    virtual int DoGetActivityInfo(Message *pMsg);
    virtual int OnGetActivityInfoDone(Message *pMsg);

    //handle unsolicited response
    virtual int OnUnsolSignalStrength(Message *pMsg);
    virtual int OnUnsolNITZTime(Message *pMsg);
    virtual int OnUnsolOemDisplayEng(Message *pMsg);
    virtual int OnUnsolOemPinControl(Message *pMsg);
    virtual int OnUnsolPhoneReset(Message *pMsg);
    virtual int OnUnsolDataStateChange(Message *pMsg);
    virtual int OnUnsolLcedataRecv(Message *pMsg);

    virtual int DoOemCancelAvailableNetworks(Message *pMsg);
    virtual int OnOemCancelAvailableNetworksDone(Message *pMsg);
    virtual int DoOemIfExecuteAm(Message *pMsg);

    virtual int DoSendSGC(Message *pMsg);
    virtual int DoSendSGCDone(Message *pMsg);
    virtual int DoSetIndicationFilter(Message *pMsg);
    virtual int OnSetIndicationFilterDone(Message *pMsg);

    int ConvertRssiToAsu(INT32 rssi);
    int DoForceCpCrash(LogDumpCause crash_reason = LOG_DUMP_CAUSE_CP_CRASH_APP, int _info = 0);
    void GetLog(LogDumpCase type, LogDumpCause reason);

    void RequestDeviceReset(int reset_type);

    virtual int DoCdmaGetSubscriptSource(Message *pMsg);
    virtual int DoCdmaSetSubscriptSource(Message *pMsg);
    virtual int DoGetCdmaSubscription(Message *pMsg);
    virtual int OnGetCdmaSubscriptionDone(Message *pMsg);
    virtual int DoGetHardwareConfig(Message *pMsg);
    virtual int OnGetHardwareConfigDone(Message *pMsg);
    virtual int DoCdmaSmsAck(Message *pMsg);
    virtual int OnCdmaSmsAckDone(Message *pMsg);

    virtual int DoSetVoiceOperation(Message *pMsg);
    virtual int OnSetVoiceOperationDone(Message *pMsg);

    virtual int DoDeviceInfo(Message *pMsg);
    virtual int DoGetNeighboringCellIds(Message *pMsg);
    virtual int DoSetLocationUpdates(Message *pMsg);
    virtual int OnSetLocationUpdateDone(Message *pMsg);

    virtual int DoSetSuppSvcNotification(Message *pMsg);
    virtual int OnSetSuppSvcNotificationDone(Message *pMsg);

    virtual void OnSimStatusChanged(int cardState, int appState);

    virtual int DoSetCarrierInfoImsiEncryption(Message *pMsg);
    virtual int OnSetCarrierInfoImsiEncryptionDone(Message *pMsg);
    virtual int DoSetSignalStrengthReportingCriteria(Message *pMsg);
    virtual int OnSetSignalStrengthReportingCriteriaDone(Message *pMsg);
    virtual int DoSetLinkCapacityReportingCriteria(Message *pMsg);
    virtual int OnSetLinkCapacityReportingCriteriaDone(Message *pMsg);
    virtual int OnLceDataRecv(Message *pMsg);

    virtual int OnUnsolCarrierInfoImsiEncryption(Message *pMsg);
    virtual int OnUnsolHardwareConfigChanged(Message *pMsg);
    virtual int OnUnsolCdmaPrlChanged(Message *pMsg);
    virtual int OnUnsolModemRestart(Message *pMsg);
    virtual int DoSetPSensorStatus(Message *pMsg);
    virtual int OnSetPSensorStatusDone(Message *pMsg);
    virtual int OnUnsolSarContolState(Message *pMsg);

    virtual int DoSetSarState(Message *pMsg);
    virtual int OnSetSarStateDone(Message *pMsg);
    virtual int DoGetSarState(Message *pMsg);
    virtual int OnGetSarStateDone(Message *pMsg);
    virtual int OnUnsolSarRfConnection(Message *pMsg);

    virtual int DoScanRssi(Message *pMsg);
    virtual int OnScanRssiDone(Message *pMsg);
    virtual int OnScanRssiResultRecived(Message *pMsg);

    virtual int DoSendATCommand(Message *pMsg);
    virtual int OnSendATCommandDone(Message *pMsg);
    virtual int OnUnsolATCommand(Message *pMsg);

    virtual int DoGetPlmnNameFromSE13Table(Message *pMsg);
    string GetNetworkName(int mcc, int mnc);

    virtual int DoTs25TableDump(Message *pMsg);

    virtual int DoGetRadioNode(Message *pMsg);
    virtual int OnGetRadioNodeDone(Message *pMsg);
    virtual int DoSetRadioNode(Message *pMsg);
    virtual int OnSetRadioNodeDone(Message *pMsg);
    virtual int DoGetProvisionUpdateRequest(Message *pMsg);
    virtual int OnGetProvisionUpdateRequestDone(Message *pMsg);
    virtual int DoSetProvisionUpdateDoneRequest(Message *pMsg);
    virtual int OnSetProvisionUpdateDoneRequestDone(Message *pMsg);
    virtual int DoRadioConfigReset(Message *pMsg);
    virtual int OnRadioConfigResetDone(Message *pMsg);

    virtual int DoEnableModem(Message *pMsg);
    virtual int OnEnableModemDone(Message *pMsg);
    virtual int DoGetModemStackStatus(Message *pMsg);
    virtual int OnGetModemStackStatusDone(Message *pMsg);
    virtual int DoSetSystemSelectionChannels(Message *pMsg);

    void SetDebugTraceOffOnBoot();
    void SetModemsConfig();
    void SendDeviceInfo();
    void SendSGCValue();
    void SendSvnInfo();

    /* Radio Config 1.1 */
    virtual int DoGetPhoneCapability(Message *pMsg);
    virtual int OnGetPhoneCapabilityDone(Message *pMsg);

    virtual int DoSetModemsConfig(Message *pMsg);
    virtual int OnSetModemsConfigDone(Message *pMsg);

    virtual int DoOemModemInfo(Message *pMsg);
    virtual int OnOemModemInfoDone(Message *pMsg);
    virtual int OnOemModemInfoReceived(Message *pMsg);
    virtual int DoOemModemReset(Message *pMsg);
    virtual int DoOemModemResetDone(Message *pMsg);
    virtual int DoOemSetRtpPktlossThreshold(Message *pMsg);
    virtual int OnOemSetRtpPktlossThresholdDone(Message *pMsg);
    virtual int OnOemRtpPktlossThresholdReceived(Message *pMsg);
    virtual int DoOemSwitchModemFunction(Message *pMsg);
    virtual int DoOemSwitchModemFunctionDone(Message *pMsg);
    virtual int DoOemSetPdcpDiscardTimer(Message *pMsg);
    virtual int DoOemSetPdcpDiscardTimerDone(Message *pMsg);
    virtual int DoSetSelflog(Message *pMsg);
    virtual int OnSetSelflogDone(Message *pMsg);
    virtual int DoGetSelfLogStatus(Message *pMsg);
    virtual int OnGetSelfLogStatusDone(Message *pMsg);
    virtual int OnUnsolSelflogStatus(Message *pMsg);
    virtual int DoSetActivateVsim(Message *pMsg);
    virtual int OnSetActivateVsimDone(Message *pMsg);
    virtual int DoOemGetCqiInfo(Message *pMsg);
    virtual int OnOemGetCqiInfoDone(Message *pMsg);
    virtual int DoOemSetSarSetting(Message *pMsg);
    virtual int OnOemSetSarSettingDone(Message *pMsg);
    virtual int DoSetImsTestMode(Message *pMsg);
    virtual int OnSetImsTestModeDone(Message *pMsg);
    virtual int DoOemSetGmoSwitch(Message *pMsg);
    virtual int OnOemSetGmoSwitchDone(Message *pMsg);
    virtual int DoOemSetTcsFci(Message *pMsg);
    virtual int OnOemSetTcsFciDone(Message *pMsg);
    virtual int DoOemGetTcsFci(Message *pMsg);
    virtual int OnOemGetTcsFciDone(Message *pMsg);
    virtual int DoOemSetCABandwidthFilter(Message *pMsg);
    virtual int OnOemSetCABandwidthFilterDone(Message *pMsg);
    virtual int OnOemCABandwidthFilterInd(Message *pMsg);
    virtual int DoSetModemLogDump(Message *pMsg);
    virtual int OnSetModemLogDumpDone(Message *pMsg);
    virtual int DoSetElevatortSensor(Message *pMsg);
    virtual int DoSetElevatortSensorDone(Message *pMsg);
    virtual int DoSetUicc(Message *pMsg);
    virtual int OnSetUiccDone(Message *pMsg);
    virtual int OnEndcCapabilityReceived(Message *pMsg);
    virtual int DoSetSelfLogProfile(Message *pMsg);
    virtual int OnSetSelfLogProfileDone(Message *pMsg);
    virtual int DoSetForbidLteCell(Message *pMsg);
    virtual int OnSetForbidLteCellDone(Message *pMsg);

    void UpdateCurrentSignalStrength(RIL_SignalStrength_V1_4 *signalStrength);
    void PrintCurrentSignalStrength();
    bool IsQueryingSignalStrengthNeed();

private:
    int mCardState;
    int mAppState;

    typedef enum {
        SIM_CARDSTATE_UNKNOWN,
        SIM_CARDSTATE_ABSENT,
        SIM_CARDSTATE_PRESENT,
        SIM_CARDSTATE_ERROR,
    } SIM_CARD_STATE;
};

#endif
