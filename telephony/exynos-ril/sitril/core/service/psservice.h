/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __PsService_H__
#define __PsService_H__

#include "service.h"
#include "pdpcontext.h"
#include "apnsetting.h"
#include "netlink.h"
#include "datacallreqdata.h"
#include <sys/time.h>
#include <map>

#define    TIMEOUT_DEFAULT_PS             5000
#define    TIMEOUT_SETUP_DATA_CALL        150000
#define    TIMEOUT_DEACT_DATA_CALL        60000

class PsService :public Service
{
protected:
    bool m_bIsAttachDone;
    PdpContext *m_PdpContext[MAX_DATA_CALL_SIZE];
    int m_nPdpContextSize;
    int m_nAttachCid;
    ApnSetting *m_pAttachApn;
    ApnSetting *m_pPreferredApn;
    PdpContext *m_pActivatingPdpContext;
    PdpContext *m_pDeactivatingPdpContext;
    //map<int, RIL_DataProfileInfo_v15> map_dpi; // key is profileId, for future use

    char m_imsi[MAX_IMSI_LEN+1];

    NetLinkMonitor *m_pNetLinkMonitor;

private:
    // For Test
    int m_bForceFailAfterSuccess;

public:
    PsService(RilContext* pRilContext);
    virtual ~PsService();

protected:
    virtual int OnCreate(RilContext *pRilContext);
    virtual void OnDestroy();

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleInternalMessage(Message* pMsg);
    virtual BOOL OnHandleRequestTimeout(Message* pMsg);

    virtual void OnRadioStateChanged(int radioState);
    virtual void OnSimStatusChanged(int cardState, int appState);
    virtual void OnImsiUpdated(const char *imsi);
    virtual bool IsPossibleToPassInRadioOffState(int request_id);

protected:
    BOOL InitPdpContext();
    BOOL ResetPdpContext(bool bKeepAttachApn = false);
    PdpContext *GetPdpContext(int cid);
    PdpContext *GetPdpContextByIndex(int index);
    PdpContext *GetAvailablePdpContext(ApnSetting *pApnSetting, int dataProfile);
    PdpContext *GetAttachPdpContext();
    PdpContext *GetAttachPdpContext(ApnSetting *pApnSetting);

    virtual const char *GetAttachApnType();
    virtual int GetAttachCid() const;

    virtual int DoSetupDataCall(Message *pMsg);
    virtual int OnSetupDataCallDone(Message *pMsg);
    virtual int OnSetupDataCallTimeout(Message *pMsg);
    virtual int OnSetupDataCallIPv6Configured(Message *pMsg);
    virtual void OnSetupDataCallComplete(int errorCode, PdpContext *pPdpContext);
    virtual void OnSetupDataCallCompletePdpFail(int pdp_fail_status);
    virtual void OnNotifyDataCallList();

    virtual int DoDeactDataCall(Message *pMsg);
    virtual int OnDeactDataCallDone(Message *pMsg);
    virtual int OnDeactDataCallTimeout(Message *pMsg);
    virtual void OnDeactDataCallComplete(PdpContext *pPdpContext);
    virtual int OnRequestDeactDataCall(PdpContext *pPdpContext, int reason = DEACT_REASON_NORMAL);
    virtual int OnRequestDeactDataCall(int cid, int reason = DEACT_REASON_NORMAL);

    virtual int DoGetDataCallList(Message *pMsg);
    virtual int OnGetDataCallListDone(Message *pMsg);
    virtual int OnDataCallListChanged(Message *pMsg);

    virtual int DoSetInitialAttachApn(Message *pMsg);
    virtual int OnSetInitialAttachApnDone(Message *pMsg);
    virtual int DoSetDataProfile(Message *pMsg);
    virtual int DoSetDataProfileInternal(SetupDataCallRequestData *datacall);
    virtual int DoSetDataProfileDirect(SetupDataCallRequestData *datacall);
    virtual int OnSetDataProfileDone(Message *pMsg);

    virtual int DoStartKeepAlive(Message *pMsg);
    virtual int OnStartKeepAliveDone(Message *pMsg);
    virtual int DoStopKeepAlive(Message *pMsg);
    virtual int OnStopKeepAliveDone(Message *pMsg);
    virtual int OnUnsolKeepaliveStatus(Message *pMsg);
    virtual int OnUnsolPcoData(Message *pMsg);

    virtual int DoRefreshInitialAttachApn(Message *pMsg);
    int RequestSetInitialAttachApn(const ApnSetting *pApnSetting);
    virtual bool IsInternalAttachEnabled();
    int RequestAttachApnInternal(const char *carrier);

    virtual int OnDedicatedBearerInfoUpdated(Message *pMsg);
    virtual int OnNasTimerStatusChanged(Message *pMsg);

    virtual int DoSetFastDormancy(void);

    virtual int OnSimOperatorInfoUpdated(Message *pMsg);

    /* Radio Config 1.1 */
    virtual int DoSetPreferredDataModem(Message *pMsg);
    virtual int OnSetPreferredDataModemDone(Message *pMsg);
protected:

    void UpdatePcscf(PdpContext *pdpContext, bool clear = false) const;
    void PrintAddressInfo(const DataCall *pDc);

    void SetRSRATimeoutTimer(int cid, const char *ifname);
    void RSRATimeoutCallback(int sigNo, siginfo_t *evp, int cid);
    static void TimerHandler_wrapper(int sigNo, siginfo_t *evp, void *uc);
    map<int, timer_t> map_timer_id; // cid to timer_id

private:
    // type 0 = SetupDataCallDone, 1 = GetDataCallListDone, 2 = DataCallListChangedInd
    ModemData *replaceModemDataForTest(ModemData *oldData, int type);
    void DumpBuf(char *data, size_t tlen);
    void DumpPcscfExtPayload(char *data);
    int isSameTargetOperator(int targetOperator);
    bool isRoamState();
    bool IsAttSim(const char *);
    bool IsNullRequest(Message *pMsg);
    bool IsNullResponse(Message *pMsg);
    void notifyAMBR(const DataCall *);
    void decodeAMBR(const DataCall *);
    bool isRejectRatForIMS(int rat);
};
#endif
