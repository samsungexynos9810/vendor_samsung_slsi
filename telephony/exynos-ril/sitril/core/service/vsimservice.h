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
 * vsimservice.h
 *
 *  Created on: 2016. 02. 26.
 */

#ifndef __VSIM_SERVICE_H__
#define __VSIM_SERVICE_H__

#include "service.h"

#define TIMEOUT_VSIM_DEFAULT_TIMEOUT    30000

class VSimService : public Service
{
public:
    VSimService(RilContext* pRilContext);
    virtual ~VSimService();

protected:
    virtual int OnCreate(RilContext *pRilContext);
    virtual void OnStart();

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);

    virtual bool IsPossibleToPassInRadioOffState(int request_id);
    virtual bool IsPossibleToPassInRadioUnavailableState(int request_id);

protected:
    virtual int DoVsimNotification(Message* pMsg);
    virtual int OnVsimNotificationDone(Message* pMsg);
    virtual int DoVsimOperation(Message* pMsg);
    virtual int OnVsimOperationDone(Message* pMsg);
    virtual int OnVsimOperationInd(Message* pMsg);
};


#endif /* __VSIM_SERVICE_H__ */
