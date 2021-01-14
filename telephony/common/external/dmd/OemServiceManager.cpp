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
 * OemServiceManager.cpp
 *
 *  Created on: 2018. 5. 14.
 */
#include "OemServiceManager.h"
#include <dlfcn.h>
#include <string.h>

#define LOG_TAG     "DMD"
#include <utils/Log.h>

#define LIB_PATH_OEMSERVICE    "liboemservice.so"

#define dlog(x...) ALOGD( x )


OemServiceManager::OemServiceManager() : mHandle(NULL), mLib(NULL) {
    memset(&mFunc, 0, sizeof(OEM_ServiceFunctions));
    m_pfnRegisterService = NULL;
    m_pFnReleaseService = NULL;
    m_pFnNotifyCallback = NULL;
}

OemServiceManager::~OemServiceManager() {
    release();
}

bool OemServiceManager::init() {
    mLib = dlopen(LIB_PATH_OEMSERVICE, RTLD_NOW);
    if (mLib == NULL) {
        dlog("failed to dlopen for %s. error %s", LIB_PATH_OEMSERVICE, dlerror());
        return false;
    }

    m_pfnRegisterService = (OEM_RegisterService)dlsym(mLib, "registerService");
    m_pFnReleaseService = (OEM_ReleaseService)dlsym(mLib, "releaseService");
    m_pFnNotifyCallback = (OEM_NotifyCallback)dlsym(mLib, "notifyCallback");

    if (m_pfnRegisterService == NULL || m_pFnReleaseService == NULL || m_pFnNotifyCallback == NULL) {
        dlog("failed to load functions");
        release();
        return false;
    }

    dlog("m_pfnRegisterService=0x%p m_pFnReleaseService=0x%p m_pFnNotifyCallback=0x%p",
            m_pfnRegisterService, m_pFnReleaseService, m_pFnNotifyCallback);
    return true;
}

void OemServiceManager::release() {
    dlog("%s", __FUNCTION__);
    if (mLib != NULL) {
        dlclose(mLib);
        mLib = NULL;
    }

    m_pfnRegisterService = NULL;
    m_pFnReleaseService = NULL;
    m_pFnNotifyCallback = NULL;

    memset(&mFunc, 0, sizeof(OEM_ServiceFunctions));
}

int OemServiceManager::registerService(const char *serviceName, OEM_ServiceFunctions *func) {
    if (mLib == NULL) {
        dlog("lib(%s) is not opened yet.", LIB_PATH_OEMSERVICE);
        return -1;
    }

    if (m_pfnRegisterService == NULL) {
        dlog("function is not loaded yet.");
        return -1;
    }

    if (serviceName == NULL || *serviceName == 0) {
        dlog("invalid serviceName");
        return -1;
    }

    if (func == NULL) {
        dlog("invalid OEM_SerivceFunctions");
        return -1;
    }

    mHandle = m_pfnRegisterService(serviceName, func);
    if (mHandle == NULL) {
        dlog("failed to register service as %s", serviceName);
        return -1;
    }

    memcpy(&mFunc, func, sizeof(OEM_ServiceFunctions));

    return 0;
}

void OemServiceManager::releaseService() {
    if (mLib == NULL) {
        dlog("lib(%s) is not opened yet.", LIB_PATH_OEMSERVICE);
        return ;
    }

    if (m_pFnReleaseService == NULL) {
        dlog("function is not loaded yet.");
        return ;
    }

    m_pFnReleaseService(mHandle);
}

int OemServiceManager::notifyCallback(int type, int id, void *data, unsigned int datalen) {
    if (mLib == NULL) {
        dlog("lib(%s) is not opened yet.", LIB_PATH_OEMSERVICE);
        return -1;
    }

    if (m_pFnNotifyCallback == NULL) {
        dlog("function is not loaded yet.");
        return -1;
    }

    m_pFnNotifyCallback(mHandle, type, id, data, datalen);
    return 0;
}
