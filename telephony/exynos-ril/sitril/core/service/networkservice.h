 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __NETWORK_SERVICE_H__
#define __NETWORK_SERVICE_H__

#include "service.h"
#include "uplmnselector.h"
#include "ts25table.h"
#include "eonsResolver.h"
#include "callreqdata.h"
#include <slsi/radio_v1_4.h>

#define TIMEOUT_NET_DEFAULT                     30000
#define TIMEOUT_NET_RADIO_POWER                 30000
#define TIMEOUT_NET_QUERY_AVAILABLE_NETWORK     300000 // changed 300sec. from 2min 20150721 <- CP timeout was 20xsec
#define TIMEOUT_NET_SET_NETWORK_MODE            120000 // 2 min.
#define TIMEOUT_NET_ALLOW_DATA                  20000  // 20 sec.
#define TIMEOUT_NET_MC_SRCH                     50000
#define TIMEOUT_NET_EMERGENCY_CALL              30000
#define MAX_UUID_LENGTH                         64

enum {
    DOMAIN_UNKNOWN_NETWORK = 0x00,
    DOMAIN_VOICE_NETWORK = 0x01,
    DOMAIN_DATA_NETWORK = 0x02,
    DOMAIN_COMBINE_NETWORK = 0x03,
};

enum {
    RADIO_NOT_READY,
    RADIO_READY,
    RADIO_WAITING,
};


//class Service;
class Message;
class RilContext;
class ModemData;

class NetworkService : public Service
{
protected:
    int                m_nRadioReady;
    RIL_RadioState    m_radioState;
    RIL_RadioState    m_desiredRadioState;
    bool              mDelayedRadioPower;
    RIL_RadioTechnology    m_nVoiceRat;
    RIL_RadioTechnology    m_nDataRat;
    int m_nRegState;
    int m_nDataRegState;

    int m_rcVersion;
    int m_rcSession;
    int m_rcPhase;
    int m_rcRaf;
    int m_rcStatus;
    char m_rcModemuuid[MAX_UUID_LENGTH];

    int m_cardState;
    int m_appState;

    UplmnSelector m_UplmnSelector;

    bool m_bShutdown;

    int m_totalOosCnt = 0;
    int m_cdmaRoamingType;
    bool mIsWfcEnabled;
    RIL_PhysicalChannelConfig_V1_4 mNrPhysicalChannelConfigs;
    bool mIsNrTestMode;

    EonsResolver m_EonsResolver;

    char mEmergencyId[MAX_EMERGENCY_ID_LEN + 1] = {0,};
    EmcInfoList m_emcInfoListFromDatabase;
    RIL_EmergencyNumber m_emergencyNumberList[MAX_EMERGENCY_NUMBER_LIST_COUNT * 2];

public:
    NetworkService(RilContext* pRilContext);
    virtual ~NetworkService();

public:
    BOOL IsPlmnSearching(void);
    int GetRegState(void);
    void mergeAndUpdateEmergencyNumberList();
private:
    int mCurrentSim;
    int FilteroutExtendedNetworkType(int reqType);
    int FilteroutCdmaNetworkType(int reqType);
    void conditionsToString(int conditions);
    BOOL isEmergencyRouting(EmcInfo emcInfo);
    void update(RIL_EmergencyNumber *emergencyNumber, EmcInfo *emcInfo);
    int mergeEmergencyNumberList(EmcInfoList *database, EmcInfoList *radio, RIL_EmergencyNumber* emergencyNumberList);
    void updateEmergencyNumberListFromDb();
    int needToUpdateOppositeEmergencyNumberList();
    String getValidSimOperatorNumeric();
    String getValidSimPlmn();
    String getIccIdPrefix();
    bool IsValidEmergencyId(const char *emergencyId);
    void resetEmergencyId();

protected:
    virtual int OnCreate(RilContext *pRilContext);
    virtual void OnStart();

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleInternalMessage(Message* pMsg);
    virtual BOOL OnHandleRequestTimeout(Message* pMsg);

    virtual void OnReset();
    virtual void OnModemOnline();
    virtual void OnRadioStateChanged(int radioState);
    virtual void OnRadioNotAvailable();
    virtual void OnRadioOffOrNotAvailable();
    virtual void OnRadioAvailable();
    virtual void OnSimStatusChanged(int cardState, int appState);
    virtual void OnServiceStateChanged(const ServiceState& state);

    void UpdateRadioState(RIL_RadioState radioState, bool notifyResult = false);

    virtual int DoVoiceRegistrationState(Message *pMsg);
    virtual int OnVoiceRegistrationStateDone(Message *pMsg);
    virtual void OnVoiceRegistrationCustomNotification(int regState, int rejectCause);
    virtual int OnNetworkStateChanged(Message *pMsg);

    virtual int DoDataRegistrationState(Message *pMsg);
    virtual int OnDataRegistrationStateDone(Message *pMsg);

    virtual int DoOperator(Message *pMsg);
    virtual int OnOperatorDone(Message *pMsg);

    virtual int DoRadioPower(Message *pMsg);
    virtual int OnRadioPowerDone(Message *pMsg);
    virtual int DoShutdown(Message *pMsg);
    virtual int OnShutdownDone(Message *pMsg);
    virtual int OnRadioStateChanged(Message *pMsg);
    virtual int DoGetRadioState(Message *pMsg);
    virtual int OnGetRadioStateDone(Message *pMsg);
    virtual int OnGetRadioStateTimeout(Message *pMsg);

    virtual int DoQueryNetworkSelectionMode(Message *pMsg);
    virtual int OnQueryNetworkSelectionModeDone(Message *pMsg);

    virtual int DoSetNetworkSelectionAuto(Message *pMsg);
    virtual int OnSetNetworkSelectionAutoDone(Message *pMsg);

    virtual int DoSetNetworkSelectionManual(Message *pMsg);
    virtual int OnSetNetworkSelectionManualDone(Message *pMsg);
    virtual int DoSetNetworkSelectionManualWithRat(Message *pMsg);

    virtual int DoQueryAvailableNetwork(Message *pMsg);
    virtual int OnQueryAvailableNetworkDone(Message *pMsg);
    virtual int OnQueryAvailableNetworkTimeout(Message *pMsg);
    virtual int DoQueryBplmnSearch(Message *pMsg);
    virtual int OnQueryBplmnSearchDone(Message *pMsg);

    virtual int DoSetBandMode(Message *pMsg);
    virtual int OnSetBandModeDone(Message *pMsg);

    virtual int DoQueryAvailableBandMode(Message *pMsg);
    virtual int OnQueryAvailableBandModeDone(Message *pMsg);

    virtual int DoSetPreferredNetworkType(Message *pMsg);
    virtual int OnSetPreferredNetworkTypeDone(Message *pMsg);

    virtual int DoGetPreferredNetworkType(Message *pMsg);
    virtual int OnGetPreferredNetworkTypeDone(Message *pMsg);

    virtual int DoVoiceRadioTech(Message *pMsg);

    virtual int DoGetCellInfoList(Message *pMsg);
    virtual int OnGetCellInfoListDone(Message *pMsg);
    virtual int OnCellInfoListReceived(Message *pMsg);

    virtual int DoSetUnsolCellInfoListRate(Message *pMsg);
    virtual int OnSetUnsolCellInfoListRateDone(Message *pMsg);

    virtual int OnRadioReady(Message *pMsg);

    virtual int DoAllowData(Message *pMsg);
    virtual int OnAllowDataDone(Message *pMsg);

    virtual int DoSetUplmn(Message *pMsg);
    virtual int OnSetUplmnDone(Message *pMsg);

    virtual int DoGetUplmn(Message *pMsg);
    virtual int OnGetUplmnDone(Message *pMsg);

    virtual int DoSetDSNetworkType(Message *pMsg);
    virtual int OnSetDSNetworkTypeDone(Message *pMsg);

    virtual int DoGetRCNetworkType(Message *pMsg);
    virtual int OnGetRCNetworkTypeDone(Message *pMsg);
    virtual int OnRCInfoReceived(Message *pMsg);

    virtual int DoSetRCNetworkType(Message *pMsg);
    virtual int OnSetRCNetworkTypeDone(Message *pMsg);

    virtual int DoGetDuplexMode(Message *pMsg);
    virtual int OnGetDuplexModeDone(Message *pMsg);

    virtual int DoSetMicroCellSrch(Message *pMsg);
    virtual int OnSetMicroCellSrchDone(Message *pMsg);
    virtual int OnSetMicroCellSrchTimeout(Message *pMsg);

    virtual int DoSetDuplexMode(Message *pMsg);
    virtual int OnSetDuplexModeDone(Message *pMsg);

    virtual int DoQueryEmergencyCallAvailableRadioTech(Message *pMsg);
    virtual int DoSetEmergencyCallStatus(Message *pMsg);
    virtual int OnSetEmergencyCallStatusDone(Message *pMsg);
    virtual int OnEmergencyActInfoReceived(Message *pMsg);

    // OEM
    virtual int DoOemSetPsService(Message *pMsg);
    virtual int OnOemSetPsServiceDone(Message *pMsg);

    virtual int DoOemGetImsSupportService(Message *pMsg);
    virtual int OnOemGetImsSupportServiceDone(Message *pMsg);

    virtual int DoOemGetPsService(Message *pMsg);
    virtual int OnOemGetPsServiceDone(Message *pMsg);

    virtual bool IsPossibleToPassInRadioOffState(int request_id);
    virtual bool IsPossibleToPassInRadioUnavailableState(int request_id);

    virtual bool isCdmaVoice(int rat);
    virtual int DoCdmaSetRoaming(Message *pMsg);
    virtual int OnCdmaSetRoamingDone(Message *pMsg);
    virtual int DoCdmaQueryRoaming(Message *pMsg);
    virtual int OnCdmaQueryRoamingDone(Message *pMsg);
    virtual int DoSetCdmaHybridMode(Message *pMsg);
    virtual int OnSetCdmaHybridModeDone(Message *pMsg);
    virtual int DoGetCdmaHybridMode(Message *pMsg);
    virtual int OnGetCdmaHybridModeDone(Message *pMsg);

    virtual int OnSimFileInfo(Message *pMsg);
    virtual int DoSetDualNetworkTypeAndAllowData(Message *pMsg);
    virtual int OnSetDualNetworkTypeAndAllowDataDone(Message *pMsg);
    virtual int DoStartNetworkScan(Message *pMsg);
    virtual int OnStartNetworkScanDone(Message *pMsg);
    virtual int DoStopNetworkScan(Message *pMsg);
    virtual int OnStopNetworkScanDone(Message *pMsg);
    virtual int OnNetworkScanResultReceived(Message *pMsg);
    virtual int OnCurrentPhysicalChannelConfigsReceived(Message *pMsg);

    virtual int DoGetManualRatMode(Message *pMsg);
    virtual int OnGetManualRatModeDone(Message *pMsg);
    virtual int DoSetManualRatMode(Message *pMsg);
    virtual int OnSetManualRatModeDone(Message *pMsg);
    virtual int DoGetFrequencyLock(Message *pMsg);
    virtual int OnGetFrequencyLockDone(Message *pMsg);
    virtual int DoSetFrequencyLock(Message *pMsg);
    virtual int OnSetFrequencyLockDone(Message *pMsg);
    virtual int DoSetEndcMode(Message *pMsg);
    virtual int OnSetEndcModeDone(Message *pMsg);
    virtual int DoGetEndcMode(Message *pMsg);
    virtual int OnGetEndcModeDone(Message *pMsg);
    virtual int OnFrequencyInfo(Message *pMsg);
    virtual int OnAcBarringInfo(Message *pMsg);
    virtual int OnB2B1ConfigInfo(Message *pMsg);
    virtual int DoGetFrequencyInfo(Message *pMsg);
    virtual int OnGetFrequencyInfoDone(Message *pMsg);

protected:
    void OnRadioReady();
    void WriteVolteEmcServiceStatus(bool volteAvilable, bool emcAvailable, int rat, int regState, bool notify = false);
    void QueryCurrentPsDomainState();
    void QueryEmergencyCallAvailableRadioTech();
    String updateOperatorNameFromEonsResolver(String operatorNumeric, String simOperator, int lac, int useType);

    virtual int OnTotalOosHappened(Message *pMsg);
    virtual int OnMccReceived(Message *pMsg);

    void SavePreferredNetworkMode(int netMode, bool bOpposite = false);
    int GetPreferredNetworkMode();
    int CheckChinaCdmaOperatorHomeRoaming(const char *mcc);
    int CheckGlobalOperatorHomeRoaming(const char *mcc);
    bool ShouldSwitchPreferredNetworkType();
    void SetAllowDataState(bool allowed, const char *reason, int phoneId);
    TS25Record GetOperatorName(const char *numeric);

    bool IsNrScgAdded();
};

#endif /*__NETWORK_SERVICE_H__*/
