#ifndef SLSI_LIVEMXLOG_H
#define SLSI_LIVEMXLOG_H

#include "common.h"

class LiveMxLog
{
private:
    static int pid;
    static MxLogStatus mode;
public:
    static void start_wifi_normal();
    static void start_bt_normal();
    static void start_bt_audio();
    static void stop();
};

#endif