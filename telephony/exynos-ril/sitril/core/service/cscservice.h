/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __CSCSERVICE_H__
#define __CSCSERVICE_H__

#include "service.h"
#include "calldata.h"
#include "callreqdata.h"

#define NET_PROP_VALUE_MAX 256
#define CALL_DEFAULT_TIMEOUT                 5000
#define SUPPLEMENTARY_DEFAULT_TIMEOUT        60000

class ServiceMgr;
class Message;
class RequestData;
class CallDialReqData;
class CallEmergencyDialReqData;
class CallInfo;
class CallList;
class DtmfInfo;
class ModemData;

typedef enum {CALL_ID_STATE_INACTIVE, CALL_ID_STATE_ACTIVE}CS_CALL_ID_STATE;

typedef struct
{
    int call_id;
    int state;
}call_id_state;

class CallId
{
public:
    CallId();
private:
    static const int MAX_CALL_ID_COUNT = 9;
    call_id_state m_CallId[MAX_CALL_ID_COUNT];
public:
    void Init();
    int GetCpIndex(int ApId);
    int GetApIndex(int CpId);

    void SyncReady();
    int AddCallId(int CpId);
    void SyncDone();
};

class CscService :public Service
{
public:
    CscService(RilContext* pRilContext);
    virtual ~CscService();

protected:
    virtual int OnCreate(RilContext *pRilContext);

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleInternalMessage(Message* pMsg);
    virtual void OnImsiUpdated(const char *imsi);
    virtual void OnSimStatusChanged(int cardState, int appState);

protected:
    /* CS : Call */
    virtual INT32 DoGetCallList();
    virtual INT32 DoGetCallListDone(Message* pMsg);
    char * BuildCallListResponse(CallList *data, int *len);
    //char * BuildCallListResponseTest(CallList *data, int *len);

    virtual INT32 DoDial();
    virtual INT32 DoDialDone(Message* pMsg);
    virtual INT32 DoEmergencyDial();
    virtual INT32 DoEmergencyDialDone(Message* pMsg);
    UINT ReturnEmergencyCategory(const char* pNumber);
    virtual INT32 DoAnswer();
    virtual INT32 DoAnswerDone(Message* pMsg);
    virtual INT32 DoExplicitCallTransfer();
    virtual INT32 DoExplicitCallTransferDone(Message* pMsg);
    virtual INT32 DoHangup(Message *pMsg);
    virtual INT32 DoReleaseCall(int callId, Message *pMsg);
    virtual INT32 DoReleaseCallMulti(int callId, Message *pMsg);
    virtual INT32 DoHangupDone(Message* pMsg);
    virtual INT32 DoLastCallFailCause();
    virtual INT32 DoLastCallFailCauseDone(Message* pMsg);

#ifdef SUPPORT_CDMA
    virtual INT32 DoCdmaBurstDtmf();
    virtual INT32 DoCdmaBurstDtmfDone(Message* pMsg);
#endif

    virtual INT32 OnRingbackTone(Message *pMsg);
    virtual INT32 OnCallStateChanged(Message* pMsg);
    virtual INT32 OnCallRinging(Message* pMsg);
    virtual INT32 OnNetEmergencyCallListReceived(Message *pMsg);
    virtual INT32 OnEmergencySupportRatModeNtf(Message *pMsg);

#ifdef SUPPORT_CDMA
    virtual INT32 DoCdmaSetPreferredVoicePrivacyMode();
    virtual INT32 DoCdmaSetPreferredVoicePrivacyModeDone(Message* pMsg);
    virtual INT32 DoCdmaQueryPreferredVoicePrivacyMode();
    virtual INT32 DoCdmaQueryPreferredVoicePrivacyModeDone(Message* pMsg);

    virtual INT32 OnCdmaInfoRec(Message* pMsg);
#endif

    /* CS : Supplementary Service */
    virtual INT32 DoUdub();
    virtual INT32 DoUdubDone(Message* pMsg);
    virtual INT32 DoHangupFgResumeBg();
    virtual INT32 DoHangupFgResumeBgDone(Message* pMsg);
    virtual INT32 DoHangupWaitOrBg();
    virtual INT32 DoHangupWaitOrBgDone(Message* pMsg);
    virtual INT32 DoSwitchWaitOrHoldAndActive();
    virtual INT32 DoSwitchWaitOrHoldAndActiveDone(Message* pMsg);
    virtual INT32 DoConference();
    virtual INT32 DoConferenceDone(Message* pMsg);
#ifdef SUPPORT_CDMA
    virtual INT32 DoCdmaFlash();
    virtual INT32 DoCdmaFlashDone(Message* pMsg);
#endif
    virtual INT32 DoSeparateConnection();
    virtual INT32 DoSeparateConnectionDone(Message* pMsg);

    virtual INT32 DoSwitchVoiceCallAudio();
    virtual INT32 DoSwitchVoiceCallAudioDone(Message* pMsg);
    virtual INT32 DoSetCallConfirm();
    virtual INT32 DoSetCallConfirmDone(Message* pMsg);
    virtual INT32 DoSendCallConfirm();
    virtual INT32 DoSendCallConfirmDone(Message* pMsg);
    virtual INT32 OnCallPresent(Message* pMsg);
#ifdef SUPPORT_CDMA
    virtual INT32 OnCdmaCallWaitingNtf(Message *pMsg);
    virtual INT32 OnUnsolCdmaOtaProvisionStatus(Message *pMsg);
#endif

    virtual int OnEnterEmergencyCallbackModeChanged(Message *pMsg);
    virtual int OnExitEmergencyCallbackModeChanged(Message *pMsg);
    virtual int DoExitEmergencyCbMode(Message *pMsg);
    virtual int DoExitEmergencyCbModeDone(Message *pMsg);
    virtual int OnUnsolResendInCallMute(Message *pMsg);

    /* CS : Sound */
    virtual INT32 DoSetMute();
    virtual INT32 DoSetMuteDone(Message* pMsg);
    virtual INT32 DoGetMute();
    virtual INT32 DoGetMuteDone(Message* pMsg);

    const char* GetCallTypeString(CallType callType);

    BOOL IsNullRequest(Message *pMsg);
    BOOL IsNullResponse(Message *pMsg);
    BOOL isCdmaVoice(int rat);

    virtual bool IsPossibleToPassInRadioOffState(int request_id);

protected:
    int mCardState;
    int mAppState;
    ClirInfo m_clirInfo;
    char m_carrier[MAX_PLMN_LEN+1];
#ifdef SUPPORT_CDMA
    RIL_CDMA_CallWaiting_v6 mRespCdmaCallWaitingNoti;
    RIL_CDMA_InformationRecords mRespCdmaInfoRecsNoti;
#endif

private:
    static const int MAX_CALL_LIST_COUNT = 9;

    CallId m_CallId;

    CallList *m_currCallList;

    RIL_Call *mRespCalls[MAX_CALL_LIST_COUNT];
    RIL_Call mRespCallsData[MAX_CALL_LIST_COUNT];
    RIL_Call_V1_2 *mRespCalls_V1_2[MAX_CALL_LIST_COUNT];
    RIL_Call_V1_2 mRespCallsData_V1_2[MAX_CALL_LIST_COUNT];
    EmcInfoList m_emcInfoListFromRadio;
    int mRespInt;
    bool m_bCallConfirm;

public:
    BOOL IsInCallState();
    EmcInfoList* getEmcInfoListFromRadio() { return &m_emcInfoListFromRadio; }
};
#endif
