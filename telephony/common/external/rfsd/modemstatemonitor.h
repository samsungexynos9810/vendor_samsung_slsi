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
 * modemstatemonitor.h
 *
 *  Created on: 2014. 11. 13.
 *      Author: sungwoo48.choi
 */

#ifndef __MODEM_STATE_MONITOR_H__
#define __MODEM_STATE_MONITOR_H__

#include "main.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <log/log.h>

#include "thread.h"

#define IOCTL_MODEM_STATUS          _IO('o', 0x27)
#define MODEM_BOOT                "/dev/umts_boot0"

enum  {
    MS_OFFLINE,
    MS_CRASH_RESET,
    MS_CRASH_EXIT,
    MS_BOOTING,
    MS_ONLINE,
};

enum modem_state {
    STATE_OFFLINE,
    STATE_CRASH_RESET,          /* silent reset */
    STATE_CRASH_EXIT,           /* cp ramdump */
    STATE_BOOTING,
    STATE_ONLINE,
};

class ModemStateMonitor : public Runnable
{
//    DECLARE_MODULE_TAG()
public:

private:
//    RilApplication *m_pRilApp;
    Thread *m_pMonitorThread;
    CRfsService *m_pRfsSrv;

public:
//    ModemStateMonitor(RilApplication *pRilApp);
    ModemStateMonitor(CRfsService *pRfsSrv);
    virtual ~ModemStateMonitor();

public:
    int Start();
    int Init() { return 0; }
    void Run();
    INT32 GetModemStatus(INT32 fd);

protected:
    virtual void OnClose();

//public:
//    static ModemStateMonitor *MakeInstance(RilApplication *pRilApp);
};

#endif /* __MODEM_STATE_MONITOR_H__ */
