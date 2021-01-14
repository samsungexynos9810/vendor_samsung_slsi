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
 * smsservice.h
 *
 *  Created on: 2014. 7. 21.
 *      Author: sungwoo48.choi
 */
#ifndef __SMS_SERVICE_H__
#define __SMS_SERVICE_H__

#include "service.h"

#define SMS_DEFAULT_TIMEOUT        10000
#define SMS_SEND_TIMEOUT        200000

class RilContext;
class Message;

class SmsService : public Service
{
public:
    char m_sca[MAX_GSM_SMS_SERVICE_CENTER_ADDR];
    int  m_scaLen;
    int  m_index[1];
    int  m_smsclass;
    int  m_simsms_full;

    SmsService(RilContext* pRilContext);
    virtual ~SmsService();

    BYTE m_nLastTpid;

protected:
    virtual int OnCreate(RilContext *pRilContext);

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);
    int OnErrorRequestComplete(int errorCode);

    void OnSimStatusChanged(int cardState, int appState);

    virtual bool IsPossibleToPassInRadioOffState(int request_id);
protected:
    virtual int DoSendSms(Message *pMsg, bool bExpectMore);
    virtual int OnSendSmsDone(Message *pMsg);
    virtual int DoSmsAck(Message *pMsg);
    virtual int OnSmsAckDone(Message *pMsg);
    virtual int DoWriteSmsToSim(Message *pMsg);
    virtual int OnWriteSmsToSimDone(Message *pMsg);
    virtual int DoDeleteSmsOnSim(Message *pMsg);
    virtual int OnDeleteSmsOnSimDone(Message *pMsg);
    virtual int DoGetBroadcastSmsConfig(Message *pMsg);
    virtual int OnGetBroadcastSmsConfigDone(Message *pMsg);
    virtual int DoSetBroadcastSmsConfig(Message *pMsg);
    virtual int OnSetBroadcastSmsConfigDone(Message *pMsg);
    virtual int DoSmsBroadcastActivation(Message *pMsg);
    virtual int OnSmsBroadcastActivationDone(Message *pMsg);
    virtual int DoGetSmscAddress(Message *pMsg);
    virtual int OnGetSmscAddressDone(Message *pMsg);
    virtual int DoSetSmscAddress(Message *pMsg);
    virtual int OnSetSmscAddressDone(Message *pMsg);
    virtual int DoReportSmsMemoryStatus(Message *pMsg);
    virtual int OnReportSmsMemoryStatusDone(Message *pMsg);
    virtual int DoSmsAckWithPdu(Message *pMsg);
    virtual int OnSmsAckWithPduDone(Message *pMsg);

    virtual int OnIncomingNewSms(Message *pMsg);
    virtual int OnIncomingNewSmsStatusReport(Message *pMsg);
    virtual int OnIncomingNewSmsOnSim(Message *pMsg);
    virtual int OnSimSmsStorageFull(Message *pMsg);
    virtual int OnIncomingNewBroadcastSms(Message *pMsg);

#ifdef SUPPORT_CDMA
    virtual int DoSendCdmaSms(Message *pMsg);
    virtual int OnSendCdmaSmsDone(Message *pMsg);
    virtual int DoWriteCdmaSmsToRuim(Message *pMsg);
    virtual int OnWriteCdmaSmsToRuimDone(Message *pMsg);
    virtual int DoDeleteCdmaSmsOnRuim(Message *pMsg);
    virtual int OnDeleteCdmaSmsOnRuimDone(Message *pMsg);
    virtual int DoGetCdmaBroadcastSmsConfig(Message *pMsg);
    virtual int OnGetCdmaBroadcastSmsConfigDone(Message *pMsg);
    virtual int DoSetCdmaBroadcastSmsConfig(Message *pMsg);
    virtual int OnSetCdmaBroadcastSmsConfigDone(Message *pMsg);
    virtual int DoSmsCdmaBroadcastActivation(Message *pMsg);
    virtual int OnSmsCdmaBroadcastActivationDone(Message *pMsg);

    virtual int OnIncomingNewCdmaSms(Message *pMsg);
    virtual int OnRuimSmsStorageFull(Message *pMsg);
    virtual int OnVoiceMsgWaitingInfo(Message *pMsg);
#endif // SUPPORT_CDMA
    virtual int DoSendAimsSms(Message *pMsg);
    virtual int OnSendAimsSmsDone(Message *pMsg);
    virtual int DoGetSmsCapacityOnSim(Message *pMsg);
    virtual int OnGetSmsCapacityOnSimDone(Message *pMsg);
};
#endif /* __SMS_SERVICE_H__ */
