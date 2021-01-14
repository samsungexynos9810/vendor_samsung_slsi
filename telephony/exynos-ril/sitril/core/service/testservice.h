/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __TestService_H__
#define __TestService_H__

#include "service.h"
#include "pdpcontext.h"
#include "apnsetting.h"
#include "netlink.h"
#include <sys/time.h>
#include <map>

class TestService :public Service
{
protected:
    NetLinkMonitor *m_pNetLinkMonitor;

public:
    TestService(RilContext* pRilContext);
    virtual ~TestService();
    static void TriggerTest(ServiceMgr *);

protected:
    virtual int OnCreate(RilContext *pRilContext);
    virtual void OnDestroy();

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleInternalMessage(Message* pMsg);
    virtual BOOL OnHandleRequestTimeout(Message* pMsg);

protected:
    virtual int DoTriggerInd(Message *pMsg);
    virtual int isSupportedInd(int ind);
};
#endif

