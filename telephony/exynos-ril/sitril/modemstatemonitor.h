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

#include "thread.h"

class RilApplication;

enum  {
    MS_OFFLINE,
    MS_CRASH_RESET,
    MS_CRASH_EXIT,
    MS_BOOTING,
    MS_ONLINE,
};

class ModemStateMonitor : public Runnable
{
    DECLARE_MODULE_TAG()
public:

private:
    RilApplication *m_pRilApp;
    Thread *m_pMonitorThread;
    int mState;
    int mFd;
    int mSpin;

public:
    ModemStateMonitor(RilApplication *pRilApp);
    virtual ~ModemStateMonitor();

public:
    int Start();
    int Init() { return 0; }
    void Run();
    void ResetModem(const char *reason);
protected:
    int PollModemState();
    int NextMonitorInterval(int state);

public:
    static ModemStateMonitor *MakeInstance(RilApplication *pRilApp);
};

#endif /* __MODEM_STATE_MONITOR_H__ */
