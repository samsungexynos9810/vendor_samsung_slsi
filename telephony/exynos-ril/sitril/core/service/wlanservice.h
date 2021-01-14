/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#ifndef __WLANSERVICE_H__
#define __WLANSERVICE_H__

#include "service.h"

#define    TIMEOUT_WLAN_DEFAULT        5000

class ServiceMgr;
class Message;
class RequestData;
class ModemData;

class WLanService :public Service
{
public:
    WLanService(RilContext* pRilContext);
    virtual ~WLanService();

protected:
    static const int GPS_TIMEOUT = 5000;

    virtual int OnCreate(RilContext *pRilContext);

    virtual BOOL OnHandleRequest(Message* pMsg);
    virtual BOOL OnHandleSolicitedResponse(Message* pMsg);
    virtual BOOL OnHandleUnsolicitedResponse(Message* pMsg);

    virtual bool IsPossibleToPassInRadioOffState(int request_id);

private:
    virtual int DoSimAuthenticate(Message *pMsg);
    virtual int OnSimAuthenticateDone(Message *pMsg);
};

#endif
