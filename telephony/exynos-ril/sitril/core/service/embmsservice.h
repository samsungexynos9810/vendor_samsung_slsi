/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __EMBMSSERVICE_H__
#define __EMBMSSERVICE_H__

#include "service.h"
#include "pdpcontext.h"

#define    TIMEOUT_EMBMS_DEFAULT        5000
#define    TIMEOUT_EMBMS_UNSOL_IND_AP_TO_NETWORK        10

class Message;
class RilContext;
class ModemData;
class EmbmbsService;

class ServiceMgr;
class Message;
class RequestData;
class ModemData;

typedef enum
{
    MODEM_STATUS_READY,
    MODEM_STATUS_EMBMS_ENABLE,
    MODEM_STATUS_EMBMS_DISABLE,
    MODEM_STATUS_DEVICE_OFF,
} ModemStatus_t;

#define NO_NETWORK_TIME_AVAILABLE ((uint64_t)(-1))


class EmbmsService :public Service
{
private:
    PdpContext* m_PdpContext;
    int mCardState;
public:
    EmbmsService(RilContext* pRilContext);
    virtual ~EmbmsService();

protected:
    static const int EMBMS_TIMEOUT = 5000;

    virtual int OnCreate(RilContext *pRilContext);

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);

    virtual int DoEnableService(Message *pMsg);
    virtual int OnEnableServiceDone(Message *pMsg);
    virtual int DoDisableService(Message *pMsg);
    virtual int OnDisableServiceDone(Message *pMsg);
    virtual int DoSetSession(Message *pMsg);
    virtual int OnSetSessionDone(Message *pMsg);
    virtual int DoGetSessionList(Message *pMsg);
    virtual int OnSessionListUpdate(Message *pMsg);
    virtual int OnGetSessionListDone(Message *pMsg);
    virtual int DoGetSignalStrength(Message *pMsg);
    virtual int OnGetSignalStrengthDone(Message *pMsg);
    virtual int OnSignalStrength(Message *pMsg);
    virtual int DoGetNetworkTime(Message *pMsg);
    virtual int OnGetNetworkTimeDone(Message *pMsg);
    virtual int OnNetworkTime(Message *pMsg);
    virtual int OnCoverage(Message *pMsg);
    virtual int OnSaiList(Message *pMsg);
    virtual int OnGlobalCellId(Message *pMsg);
    virtual int DoCheckAvaiableEmbms(Message *pMsg);
    virtual int OnCheckAvaiableEmbmsDone(Message *pMsg);

    virtual void OnRadioStateChanged(int radioState);
    virtual void OnSimStatusChanged(int cardState, int appState);
};

#endif //__EMBMSSERVICE_H__
