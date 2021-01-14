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
 * OemServiceManager.h
 *
 *  Created on: 2018. 5. 14.
 */
#ifndef __OEM_SERVICE_MANAGER_H__
#define __OEM_SERVICE_MANAGER_H__

#include "OemService.h"

class OemServiceManager {
private:
    HANDLE mHandle;
    void *mLib;
    OEM_ServiceFunctions mFunc;
    OEM_RegisterService m_pfnRegisterService;
    OEM_ReleaseService m_pFnReleaseService;
    OEM_NotifyCallback m_pFnNotifyCallback;

public:
    OemServiceManager();
    virtual ~OemServiceManager();
public:
    bool init();
    void release();
    int registerService(const char *serviceName, OEM_ServiceFunctions *func);
    void releaseService();
    int notifyCallback(int type, int id, void *data, unsigned int datalen);
};

#endif // __OEM_SERVICE_MANAGER_H__
