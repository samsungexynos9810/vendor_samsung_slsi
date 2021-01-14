/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __SERVICEMGR_H__
#define __SERVICEMGR_H__

#include <map>
#include <stddef.h>
#include "message.h"
#include "servicefactory.h"

using namespace std;

class RilContext;
class Service;
class Protocol;
class ServiceFactory;
class RequestData;
class ModemData;

typedef std::map<UINT, Service*> ServicesMap;
typedef std::map<UINT, std::pair<UINT, UINT>> RouteMap;

class ServiceMgr
{
public:
    ServiceMgr(RilContext* pCRilContext);
    virtual ~ServiceMgr();
    virtual int OnInitialize();
public:
    int Init(ServiceFactory *pServiceFactory = NULL);
    int Finalize();
    int SendMessage(Message* msg);
    void BroadcastSystemMessage(Message* msg);
    void BroadcastSystemMessage(int messageId, RilData *data = NULL);
    int RouteRequest(int requestId, int &nServiceId, int &nMessageId);
    int RouteRequest(RequestData *pRequestData, int &nServiceId, int &nMessageId);
    int RouteOemRequest(RequestData *pRequestData, int &nServiceId, int &nMessageId);
    int RouteProtocolInd(int nId, int &nServiceId, int &nMessageId);
    int RouteResponse(ModemData *pModemData, int &nServiceId, int &nMessageId);

    int SetRequestMap(int nRequestId, int nServiceId, int nMessageId);
    int SetOemRequestMap(int nOemRequestId, int nServiceId, int nMessageId);
    int SetProtocolIndMap(int nIndId, int nServiceId, int nMessageId);

    int AddService(int nServiceId, Service *pService);
    int StartServices();
    Service* FindService(int nServiceId);
    bool IsAsyncMessage(int nServiceId, int nMessageId);

private:
    RilContext* m_pRilContext;
    ServicesMap m_mapServiceMap;
};
#endif
