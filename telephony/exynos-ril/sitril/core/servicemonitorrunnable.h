/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef _SERVICE_MONITOR_RUNNABLE_H_
#define _SERVICE_MONITOR_RUNNABLE_H_

class Service;
#include "types.h"
#include "thread.h"

class ServiceMonitorRunnable : public Runnable
{
public:
    virtual void Run();
    explicit ServiceMonitorRunnable(Service *pService);
    virtual ~ServiceMonitorRunnable();
protected:
    Service *m_pSerV;
};

#endif /*_SERVICE_MONITOR_RUNNABLE_H_*/
