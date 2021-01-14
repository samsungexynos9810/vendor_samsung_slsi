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
 * simservice.cpp
 *
 *  Created on: 2018.03.09.
 *      Author: MOX
 */

#ifndef __STK_SERVICE_H__
#define __STK_SERVICE_H__

#include "service.h"
#include "stkmodule.h"
#include "stkdata.h"

#define    TIMEOUT_STK_DEFAULT        30000

class Message;
class RilContext;
class ModemData;

class StkService : public Service
{
public:
    StkService(RilContext* pRilContext);
    virtual ~StkService();

protected:
    virtual int OnCreate(RilContext *pRilContext);

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);

    virtual void OnSimStatusChanged(int cardState, int appState);
    virtual void OnVoiceRegistrationStateChanged(int regState);
    virtual void OnDataRegistrationStateChanged(int regState);
    virtual void OnDataCallStateChanged(int nCid, bool bActive);
    virtual bool IsDefaultApnConnecting();

protected:
    virtual int OnSimHotSwap(BOOL bRemoval);

    virtual int DoStkIsRunning(Message *pMsg);

    virtual int DoSendEvelopeCommand(Message *pMsg);
    virtual int OnSendEvelopeCommandDone(Message *pMsg);

    virtual int DoSendTerminalResponse(Message *pMsg);
    virtual int OnSendTerminalResponseDone(Message *pMsg);

    virtual int DoSendEnvelopeStatus(Message *pMsg);
    virtual int OnSendEnvelopeStatusDone(Message *pMsg);

    virtual int DoStkHandleCallSetupReqFromSim(Message *pMsg);
    virtual int OnStkHandleCallSetupReqFromSimDone(Message *pMsg);

    virtual int OnProactiveCommand(Message *pMsg);
    virtual int OnSimRefresh(Message *pMsg);

    virtual int OnSsReturnResult(Message *pMsg);
    virtual int OnSessionEnd(Message *pMsg);

    virtual int OnStkCcAlphaNtf(Message *pMsg);

    virtual bool IsPossibleToPassInRadioOffState(int request_id);

protected:
    BOOL m_bStkRunning;

    Message *m_pProactiveCmd;
    StkEfidList *m_pStkEfidList;

    int m_nCardState;

    // Internal Request
    public:
        virtual void OnRequestInternalComplete(RIL_Token t, int id, int result, void *data = NULL, int datalen = 0);

    // For STK Module
    protected:
        static const BOOL STK_MODULE_ENABLED = false;
        CStkModule *m_pStkModule;
        friend class CStkModule;

        virtual int SendStk(Message *pMsg);
        virtual int OnStkModule(Message *pMsg);
};

#endif /*__STK_SERVICE_H__*/
