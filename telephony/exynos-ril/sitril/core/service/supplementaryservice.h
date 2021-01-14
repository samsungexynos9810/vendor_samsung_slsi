/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __SUPPLEMENTARYSERVICE_H__
#define __SUPPLEMENTARYSERVICE_H__

#include "service.h"
#include "calldata.h"

#define SUPPLEMENTARY_DEFAULT_TIMEOUT        60000

class SupplementaryService :public Service
{
public:
    SupplementaryService(RilContext* pRilContext);
    virtual ~SupplementaryService();

protected:
    virtual int OnCreate(RilContext *pRilContext);

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleInternalMessage(Message* pMsg);
    virtual void OnSimStatusChanged(int cardState, int appState);

protected:
    virtual INT32 DoChangeCallBarringPwd();
    virtual INT32 DoChangeCallBarringPwdDone(Message* pMsg);
    virtual INT32 DoSetCallWaiting();
    virtual INT32 DoSetCallWaitingDone(Message* pMsg);
    virtual INT32 DoQueryCallWaiting();
    virtual INT32 DoQueryCallWaitingDone(Message* pMsg);
    virtual INT32 DoSetCallForwarding();
    virtual INT32 DoSetCallForwardingDone(Message* pMsg);
    virtual INT32 DoQueryCallForwarding();
    virtual INT32 DoQueryCallForwardingDone(Message* pMsg);
    virtual INT32 DoSetClir();
    virtual INT32 DoGetClir();
    virtual INT32 DoGetClirDone(Message* pMsg);
    virtual INT32 DoGetClip();
    virtual INT32 DoGetClipDone(Message* pMsg);
    virtual INT32 DoCancelUssd();
    virtual INT32 DoCancelUssdDone(Message* pMsg);
    virtual INT32 DoSendUssd();
    virtual INT32 DoSendUssdDone(Message* pMsg);

    virtual INT32 OnUssd(Message *pMsg);
    virtual INT32 OnSsSvc(Message *pMsg);

    virtual INT32 DoQueryColp();
    virtual INT32 DoQueryColpDone(Message* pMsg);
    virtual INT32 DoQueryColr();
    virtual INT32 DoQueryColrDone(Message* pMsg);
    virtual INT32 DoSendEncodedUssd();
    virtual INT32 OnUnsolOnSS(Message *pMsg);

    BOOL IsNullRequest(Message *pMsg);
    BOOL IsNullResponse(Message *pMsg);
    virtual bool IsPossibleToPassInRadioOffState(int request_id);
    int GetValidErrors(int errorCode);
    bool IsOperatorUsingUnknownServiceClass();

protected:
    int mCardState;
    ClirInfo m_clirInfo;
    RIL_SuppSvcNotification mRespSuppSvcNoti;

private:
    static const int MAX_CALL_FORWARD_LIST_COUNT = 8;
    static const int MAX_USSD_COUNT = 3;

    BOOL m_ussdUserInitiated;

    RIL_CallForwardInfo *mRespCallForward[MAX_CALL_FORWARD_LIST_COUNT];
    RIL_CallForwardInfo mRespCallForwardData[MAX_CALL_FORWARD_LIST_COUNT];
    char *mRespUssd[MAX_USSD_COUNT];
    int mResp2Ints[2];
    int mRespInt;
};
#endif
