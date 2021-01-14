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
 * liboemservice.cpp
 *
 *  Created on: 2018. 5. 3.
 */

#include "oemservice.h"
#include "OemServiceImpl.h"
#include <string>
#include <map>

using namespace std;
static map<string, OemService *> sOemServiceList;

HANDLE registerService(const char *serviceName, OEM_ServiceFunctions *func) {
    dlog("registerService");

    if (serviceName == NULL || *serviceName == 0) {
        dlog("invalid service name");
        return NULL;
    }

    if (func == NULL) {
        dlog("invalid param. OEM_SerivceFunctions is NULL.");
        return NULL;
    }

    string key = serviceName;
    map<string, OemService *>::iterator iter = sOemServiceList.find(key);
    if (iter != sOemServiceList.end()) {
        dlog("serviceName %s already existed.", serviceName);
        return iter->second;
    }

    OemService *newOemService = OemService::makeInstance(serviceName, func);
    if (newOemService != NULL) {
        if (newOemService->registerService() == android::OK) {
            sOemServiceList[key] = newOemService;
            dlog("registerService as \"%s\", OEM_SerivceFunctions=0x%p", serviceName, func);
        }
        else {
            dlog("failed to registerService as \"%s\"", serviceName);
            delete newOemService;
        }
    }
    else {
        dlog("memory allocation fail or invalid parameter");
    }

    return (HANDLE)newOemService;
}

void releaseService(HANDLE h) {
    dlog("releaseService");
    OemService *oemService = (OemService *)h;
    if (oemService != NULL) {
        const char *serviceName = oemService->getServiceName();
        dlog("HANDLE=0x%p serviceName=%s", oemService, serviceName);
        map<string, OemService *>::iterator iter = sOemServiceList.find(string(serviceName));
        if (iter != sOemServiceList.end()) {
            sOemServiceList.erase(iter);
        }
        else {
            dlog("unregistered service instance, serviceName=%s", serviceName);
        }

        delete oemService;
        oemService = NULL;
    }
    else {
        dlog("invalid HANDLE");
    }
}

void notifyCallback(HANDLE h, int type, int id, void *data, unsigned int datalen) {
    dlog("notifyCallback");
    OemService *oemService = (OemService *)h;
    if (oemService != NULL) {
        const char *serviceName = oemService->getServiceName();
        map<string, OemService *>::iterator iter = sOemServiceList.find(string(serviceName));
        if (iter != sOemServiceList.end()) {
            dlog("(%s)notifyCallback", oemService->getServiceName());
            oemService->onCallback(type, id, data, datalen);
        }
        else {
            dlog("unregistered service instance, serviceName=%s", serviceName);
        }
    }
}
