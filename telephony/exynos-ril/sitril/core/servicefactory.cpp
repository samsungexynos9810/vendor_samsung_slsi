/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "servicefactory.h"
#include "cscservice.h"
#include "psservice.h"
#include "networkservice.h"
#include "simservice.h"
#include "miscservice.h"
#include "smsservice.h"
#include "audioservice.h"
#include "imsservice.h"
#include "wlanservice.h"
#include "vsimservice.h"
#include "gpsservice.h"
#include "stkservice.h"
#include "testservice.h"
#include "supplementaryservice.h"
#include "embmsservice.h"
#include "rillog.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

ServiceFactory::ServiceFactory()
{
    mInit = false;
    ClearAvailableService();
}

void ServiceFactory::AddAvailableService(UINT serviceId)
{
    mAvailableServiceList.push_back(serviceId);
}

void ServiceFactory::ClearAvailableService()
{
    mAvailableServiceList.clear();
}

void ServiceFactory::Init() {
    RilLogI("DefaultServiceFactory: Init");
    if (mInit) {
        RilLogI("already initialized.");
        return ;
    }

    OnInitialize();
    mInit = true;
}

Service *ServiceFactory::CreateService(RilContext *pRilContext, UINT nServiceId)
{
    Service *pService = NULL;

    switch (nServiceId) {
    case RIL_SERVICE_NETWORK:
        pService = new NetworkService(pRilContext);
        break;
    case RIL_SERVICE_CSC:
        pService = new CscService(pRilContext);
        break;
    case RIL_SERVICE_PS:
        pService = new PsService(pRilContext);
        break;
    case RIL_SERVICE_SIM:
        pService = new SimService(pRilContext);
        break;
    case RIL_SERVICE_MISC:
        pService = new MiscService(pRilContext);
        break;
    case RIL_SERVICE_SMS:
        pService = new SmsService(pRilContext);
        break;
    case RIL_SERVICE_AUDIO:
        pService = new AudioService(pRilContext);
        break;
    case RIL_SERVICE_IMS:
        pService = new ImsService(pRilContext);
        break;
    case RIL_SERVICE_WLAN:
        pService = new WLanService(pRilContext);
        break;
    case RIL_SERVICE_VSIM:
        pService = new VSimService(pRilContext);
        break;
    case RIL_SERVICE_GPS:
        pService = new GpsService(pRilContext);
        break;
    case RIL_SERVICE_STK:
        pService = new StkService(pRilContext);
        break;
    case RIL_SERVICE_TEST:
        pService = new TestService(pRilContext);
        break;
    case RIL_SERVICE_SUPPLEMENTARY:
        pService = new SupplementaryService(pRilContext);
        break;
    case RIL_SERVICE_EMBMS:
        pService = new EmbmsService(pRilContext);
        break;
    default:
        // TODO unsupported service creation
        return NULL;
    } // end switch ~

    if (pService != NULL && pService->GetServiceId() != nServiceId) {
        delete pService;
        pService = NULL;
    }

    return pService;
}

void ServiceFactory::CreateService(RilContext *pRilContext, ServiceMgr *pServiceMgr)
{
    RilLogI("[ServiceFactory] %s", __FUNCTION__);
    if (pRilContext == NULL || pServiceMgr == NULL) {
        RilLogE("Invalid parameter : pRilContext=%p pServiceMgr=%p", pRilContext, pServiceMgr);
        return ;
    }

    list<UINT>::iterator iter;
    for (iter = mAvailableServiceList.begin(); iter != mAvailableServiceList.end(); ++iter) {
        UINT serviceId = *iter;
        Service* pService = CreateService(pRilContext, serviceId);
        if (pServiceMgr->AddService(serviceId, pService) < 0) {
            RilLogW("Failed to add new service instance : service ID=%d", serviceId);
        }
    } // end for iter ~
}

/**
 * DefaultServiceFactory
 */
class DefaultServiceFactory : public ServiceFactory
{
public:
    DefaultServiceFactory() : ServiceFactory() {}
    virtual ~DefaultServiceFactory() {}
public:
    virtual void OnInitialize();
};

void DefaultServiceFactory::OnInitialize() {
    AddAvailableService(RIL_SERVICE_NETWORK);
    AddAvailableService(RIL_SERVICE_CSC);
    AddAvailableService(RIL_SERVICE_PS);
    AddAvailableService(RIL_SERVICE_SIM);
    AddAvailableService(RIL_SERVICE_MISC);
    AddAvailableService(RIL_SERVICE_SMS);
    AddAvailableService(RIL_SERVICE_AUDIO);
    AddAvailableService(RIL_SERVICE_IMS);
    AddAvailableService(RIL_SERVICE_GPS);
    AddAvailableService(RIL_SERVICE_STK);
    AddAvailableService(RIL_SERVICE_EMBMS);
    AddAvailableService(RIL_SERVICE_TEST);
    AddAvailableService(RIL_SERVICE_VSIM);
    AddAvailableService(RIL_SERVICE_SUPPLEMENTARY);
}

/**
 * static
 */
ServiceFactory *ServiceFactory::GetDefaultServiceFactory()
{
    // initialize
    DefaultServiceFactory *defaultServiceFactory = new DefaultServiceFactory();
    if (defaultServiceFactory != NULL) {
        defaultServiceFactory->Init();
    }
    return defaultServiceFactory;
}
