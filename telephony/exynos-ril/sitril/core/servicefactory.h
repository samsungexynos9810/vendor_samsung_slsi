/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef    __SERVICE_FACTORY_H__
#define    __SERVICE_FACTORY_H__

#include "rildef.h"
#include "servicemgr.h"
#include <list>
using namespace std;

class RilContext;
class Service;
class ServiceMgr;

class ServiceFactory {

    /**
     * member
     */
private:
    list<UINT> mAvailableServiceList;
    bool mInit;

    /**
     * constructor
     */
public:
    ServiceFactory();
    virtual ~ServiceFactory() {}

protected:
    void AddAvailableService(UINT serviceId);
    void ClearAvailableService();

    /**
     * interface
     */
public:
    void Init();
    virtual void OnInitialize()=0;
    virtual Service *CreateService(RilContext *pRilContext, UINT nServiceId);
    virtual void CreateService(RilContext *pRilContext, ServiceMgr *pServiceMgr);

    /**
     * static
     */
public:
    // get default service factory instance
    static ServiceFactory *GetDefaultServiceFactory();
};

#endif    // __SERVICE_FACTORY_H__
