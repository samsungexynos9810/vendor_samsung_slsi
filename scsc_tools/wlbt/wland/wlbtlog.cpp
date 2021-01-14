#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include "filedir.h"

#define LOG_TAG "WLBTLOGDBG"
#include <utils/Log.h>

#include "common.h"
#include "SingleShotTimer.h"
#include "mxlog.h"
#include "udilog.h"
#include "wlbtlog.h"

pid_t WlbtLog::bt_logging_pid = -1;
pid_t WlbtLog::mxlog_pid = -1;
pid_t WlbtLog::udilog_pid = -1;

void WlbtLog::setup_bt_normal_log_filter()
{
    system("echo 0x00824007 > /sys/module/scsc_bt/parameters/mxlog_filter");
}

void WlbtLog::setup_bt_audio_log_filter()
{
    system("echo 0x00824007 > /sys/module/scsc_bt/parameters/mxlog_filter");
}

void WlbtLog::setup_bt_custom_log_filter(const char* filter)
{
    char cmd[64];
    sprintf(cmd , "echo %s > /sys/module/scsc_bt/parameters/mxlog_filter", filter);
    system(cmd);
}

void WlbtLog::revert_bt_log_filter()
{
    system("echo 0x00000000 > /sys/module/scsc_bt/parameters/mxlog_filter");
}

bool WlbtLog::start_mxlog(const char* prefix, int res_sock)
{
    if(mxlog_pid != -1)
    {
        stop_mxlog();
    }
    if(bt_logging_pid != -1)
    {
        stop_bt_log();
    }

    mxlog_pid = MxLog::start(NULL, NULL, prefix);
    Common::send_reply(res_sock, mxlog_pid);
    if(mxlog_pid != -1)
    {
        Common::ChangeFileMxLogState(MXLOG_WIFI_NORMAL_T);
    }
    return mxlog_pid != -1;
}

bool WlbtLog::start_udilog(const char* prefix, int res_sock)
{
    if(udilog_pid != -1)
    {
        stop_udilog();
    }

    udilog_pid = UdiLog::start(NULL, NULL, prefix);
    Common::send_reply(res_sock, udilog_pid);
    if(udilog_pid != -1)
    {
        Common::ChangeUdiLogState(UDILOG_ON_T);
    }
    return udilog_pid != -1;
}

bool WlbtLog::start_bt_normal_log(const char* prefix, int res_sock)
{
    if(bt_logging_pid != -1)
    {
        stop_bt_log();
    }
    if(mxlog_pid != -1)
    {
        stop_mxlog();
    }

    bt_logging_pid = MxLog::start(setup_bt_normal_log_filter, revert_bt_log_filter, prefix);
    Common::send_reply(res_sock, bt_logging_pid);
    if(bt_logging_pid != -1)
    {
        Common::ChangeFileMxLogState(MXLOG_BT_NORMAL_T);
    }
    return bt_logging_pid != -1;
}

bool WlbtLog::start_bt_audio_log(const char* prefix, int res_sock)
{
    if(bt_logging_pid != -1)
    {
        stop_bt_log();
    }
    if(mxlog_pid != -1)
    {
        stop_mxlog();
    }

    bt_logging_pid = MxLog::start(setup_bt_audio_log_filter, revert_bt_log_filter, prefix);
    Common::send_reply(res_sock, bt_logging_pid);
    if(bt_logging_pid != -1)
    {
        Common::ChangeFileMxLogState(MXLOG_BT_AUDIO_T);
    }
    return bt_logging_pid != -1;
}

bool WlbtLog::start_bt_custom_log(const char* prefix, int res_sock, const char* data)
{
    if(bt_logging_pid != -1)
    {
        stop_bt_log();
    }
    if(mxlog_pid != -1)
    {
        stop_mxlog();
    }

    setup_bt_custom_log_filter(data);
    bt_logging_pid = MxLog::start(NULL, revert_bt_log_filter, prefix);
    Common::send_reply(res_sock, bt_logging_pid);
    if(bt_logging_pid != -1)
    {
        Common::ChangeFileMxLogState(MXLOG_BT_NORMAL_T);
    }
    return bt_logging_pid != -1;
}

void WlbtLog::stop_mxlog()
{
    if(mxlog_pid > 0)
    {
        mxlog_pid = -1;
        MxLog::stop();
        Common::ChangeFileMxLogState(MXLOG_OFF_T);
    }
}

void WlbtLog::stop_udilog()
{
    if(udilog_pid > 0)
    {
        udilog_pid = -1;
        UdiLog::stop();
        Common::ChangeUdiLogState(UDILOG_OFF_T);
    }
}

void WlbtLog::stop_bt_log()
{
    if(bt_logging_pid > 0)
    {
        bt_logging_pid = -1;
        MxLog::stop();
        Common::ChangeFileMxLogState(MXLOG_OFF_T);
    }
}
