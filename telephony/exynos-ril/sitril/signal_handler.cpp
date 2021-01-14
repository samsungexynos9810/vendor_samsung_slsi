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
 * signal_handler.cpp
 *
 *  Created on: 2015. 01. 21.
 *      Author: jhdaniel.kim
 */

#include <unistd.h>
 #include <signal.h>
#include "signal_handler.h"
#include "rilapplication.h"
 #include "rillog.h"
#ifdef HAVE_ANDROID_OS
#include <cutils/properties.h>
#define LOG_RIL_SIGNAL_ACTION   "vendor.ril.signal.action"
#endif
#include "reset_util.h"

#ifndef PROP_VALUE_MAX
#define PROP_VALUE_MAX 92
#endif


// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

 IMPLEMENT_MODULE_TAG(SignalMonitor, SignalHandler)

SignalMonitor* gpThis;
void signal_handler(int signo);

int register_signal_handler()
{
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);

    //signal(SIGPIPE, SIG_IGN);
    signal(SIGPIPE, signal_handler);
    return 0;
}

void unregister_signal_handler()
{
    signal(SIGUSR1, SIG_DFL);
    signal(SIGUSR2, SIG_DFL);

    signal(SIGPIPE, SIG_DFL);
}

void signal_handler(int signo)
{
    gpThis->HandleSignal(signo);
}

SignalMonitor::SignalMonitor(RilApplication *pApp)
{
    m_pRilApp = pApp;
 }

SignalMonitor::~SignalMonitor()
{

}

int SignalMonitor::Start()
{
    gpThis = this;
    register_signal_handler();

    return 0;
}

int SignalMonitor::Stop()
{
    unregister_signal_handler();
    return 0;
}

int SignalMonitor::HandleSignal(int signo)
{
    //char szActNo[PROP_VALUE_MAX] = {0,};
    int nActNo = 0;
    RilLogI("%s() : signo(%d)", __FUNCTION__, signo);

    switch(signo)
    {
    case SIGUSR1:
        RilLogV("[SIGNAL_HANDLER] SIGUSR1 reserved for dedicated usage");
        HandleSignalReserved();
        break;
    case SIGUSR2:
        RilLogV("[SIGNAL_HANDLER] SIGUSR2 used for dev");
#ifdef HAVE_ANDROID_OS
        property_get((char *)LOG_RIL_SIGNAL_ACTION, szActNo, "0");
        nActNo = atoi(szActNo);
#endif
        HandleSignalDevAction(nActNo);
        break;
    case SIGPIPE:
        RilLogE("[SIGNAL_HANDLER] SIGPIPE is caught");
        RilLogE("Restart RIL process for sync status between CP and RIL");
        RilErrorReset("SIGPIPE");
        break;
    default:
        RilLogE("[SIGNAL_HANDLER] Signal(%d) is caught", signo);
        break;
    }

    return 0;
}

int SignalMonitor::HandleSignalReserved()
{
    return 0;
}

int SignalMonitor::HandleSignalDevAction(int dev_id)
{
    const int ID_RILLOG_CAT_CHANGED = 1;
    switch(dev_id)
    {
    case ID_RILLOG_CAT_CHANGED:
        // re-load the current log policy
        CRilLog::InitRilLog();
        break;
    }

    return 0;
}
