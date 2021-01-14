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
 * modemstatemonitor.cpp
 *
 *  Created on: 2014. 11. 17.
 *      Author: sungwoo48.choi
 */


#include "modemstatemonitor.h"
#include "rilapplication.h"
#include "modemcontrol.h"
#include <sys/poll.h>
#include "reset_util.h"
#include "rillog.h"
#include "productfeature.h"
#include "rilproperty.h"
#include "systemproperty.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

IMPLEMENT_MODULE_TAG(ModemStateMonitor, ModemStateMonitor)

ModemStateMonitor::ModemStateMonitor(RilApplication *pRilApp) : m_pRilApp(pRilApp)
{
    m_pMonitorThread = NULL;
    mState = -1;
    mFd = -1;
    mSpin = 0;
}

ModemStateMonitor::~ModemStateMonitor()
{

}

int ModemStateMonitor::Start()
{
    if (RilProperty::IsUserMode()) {
        string mode = SystemProperty::Get(CRASH_MODE_SYS_PROP, "");
        RilLog("[%s]%s user mode and crash_hanlding_mode=%s", TAG, __FUNCTION__, mode.c_str());
        if (mode.empty()) {
            SystemProperty::Set(CRASH_MODE_SYS_PROP, "2");
            RilLogV("[%s]%s set to CRASH_MODE_SILENT_RESET", TAG, __FUNCTION__);
        }
    }

    m_pMonitorThread = new Thread(this);
    if (m_pMonitorThread == NULL || m_pMonitorThread->Start() < 0) {
        return -1;
    }

    return 0;
}

ModemStateMonitor *ModemStateMonitor::MakeInstance(RilApplication *pRilApp)
{
    ModemStateMonitor *instance = NULL;
    if (pRilApp != NULL) {
        instance = new ModemStateMonitor(pRilApp);
        if (instance != NULL) {
            if (instance->Init() < 0) {
                delete instance ;
                instance = NULL;
            }
        }
    }
    return instance;
}

int ModemStateMonitor::PollModemState()
{
    int oldState = mState;
    mState = ModemControl::GetModemStatus(mFd);

    RilLog("[%s] oldState=%d newState=%d", __FUNCTION__, oldState, mState);
    if (mState != oldState) {
        int mode = SystemProperty::GetInt(CRASH_MODE_SYS_PROP, CRASH_MODE_DEFAULT);
        RilLogE("MODEM STATUS crash handling mode = %d", mode);

        switch(mState)
        {
        case STATE_CRASH_RESET: {
            RilLogE("****************************************************");
            RilLogE("@@@@@@ MODEM STATUS : STATE_CRASH_RESET @@@@@@");
            RilLogE("****************************************************");
            m_pRilApp->OnModemStateChanged(MS_CRASH_RESET);
            break;
        }
        case STATE_CRASH_EXIT: {
            RilLogE("***************************************************");
            RilLogE("@@@@@@ MODEM STATUS : STATE_CRASH_EXIT @@@@@@");
            RilLogE("***************************************************");
            if (mode == CRASH_MODE_DUMP_SILENT_RESET || mode == CRASH_MODE_SILENT_RESET) {
                m_pRilApp->OnModemStateChanged(MS_CRASH_RESET);
            }
            else {
                m_pRilApp->OnModemStateChanged(MS_CRASH_EXIT);
            }
            break;
        }
        case STATE_OFFLINE:
        case STATE_BOOTING: {
            RilLog("MODEM STATUS : %s", STATE_OFFLINE ? "STATE_OFFLINE" : "STATE_BOOTING");
            m_pRilApp->OnModemStateChanged(mState);
                int spin = 300;
                while (spin--) {
                    mState = ModemControl::GetModemStatus(mFd);
                    if (mState == STATE_ONLINE) {
                        RilLog("MODEM STATUS : STATE_ONLINE");
                        m_pRilApp->OnModemStateChanged(MS_ONLINE);
                        break;
                    }
                    usleep(100 * 1000);
                } // end while ~

                if (spin < 0) {
                    RilLogW("@@@ CP booting is not done yet during %d sec @@@", spin / 100);
                }
            break;
        }
        case STATE_ONLINE:
            RilLog("MODEM STATUS : STATE_ONLINE");
            m_pRilApp->OnModemStateChanged(MS_ONLINE);
            break;
        default:
            RilLogW("MODEM STATUS : unknown Modem Status (%d)", mState);
            break;
        } // end switch ~
    }

    // wait for 200ms to avoid busy state
    usleep(200000);
    return 0;
}

int ModemStateMonitor::NextMonitorInterval(int state)
{
    const int NORMAL_INTERVAL = 3000;   // 3sec
    const int INFINATE = -1;
    const int IMMEDIATE = 0;

    if (state < 0) {
        return IMMEDIATE;
    }

    // poll timeout. milliseconds
    switch (state) {
    case STATE_ONLINE:
        // return infinite
        return INFINATE;
    case STATE_CRASH_EXIT:
    case STATE_CRASH_RESET:
        // return infinite
        return NORMAL_INTERVAL;
    case STATE_OFFLINE:
    case STATE_BOOTING:
        // return normal interval time
        return NORMAL_INTERVAL;
    default:
        return NORMAL_INTERVAL;
    } // end switch ~
}

void ModemStateMonitor::Run()
{
    RilLogV("[%s::%s] Start Modem State monitoring", TAG, __FUNCTION__);
    mFd = open(MODEM_BOOT, O_RDWR);
    if (mFd < 0)
    {
        RilLogE("Open MODEM_BOOT error: %s", strerror(errno));
        RilReset("FAIL_TO_OPEN_BOOT_DEVICE");
        return;
    }

    bool runnable = true;
    struct pollfd pfd;
    pfd.fd = mFd;
    pfd.events = POLLHUP | POLLIN | POLLRDNORM;
    while (runnable) {
        int timeout = NextMonitorInterval(mState);
        pfd.revents = 0;
        int ret = poll(&pfd, 1, timeout);
        if (mState < 0 || ret == 0
             || (ret > 0 && ((pfd.revents & POLLHUP) || (pfd.revents & POLLIN) || (pfd.revents & POLLRDNORM)))) {

            if (ret == 0)
                RilLogV("poll timeout. interval=%d", timeout);

            PollModemState();
            if (mState == STATE_CRASH_RESET || mState == STATE_CRASH_EXIT) {
                // poll even immediately during CP crash state
                // sleep to avoid busy waiting
                sleep(3);
            }
        } else {
            RilLogE("MODEM STATUS : unknown poll event (%d)", pfd.revents);
            sleep(3);
        }
    } // end while runnable ~

    close(mFd);
    RilLogV("Stop monitoring Modem Status ...");
}

void ModemStateMonitor::ResetModem(const char *reason)
{
    RilLogW("######### Reset Modem by RIL(%s) #########", reason);
    // reset modem wil be done by vendor_init
    SystemProperty::Set("vendor.sys.modem_reset", 1);
    usleep(200000);
    RilLogW("# Modem state become STATE_CRASH_RESET but not a CP crash!!!");
    m_pRilApp->OnModemStateChanged(MS_CRASH_RESET);
}
