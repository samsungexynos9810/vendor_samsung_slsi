#include "common.h"
#include "mxlog.h"
#include "wlbtlog.h"
#include "FaultIds.h"

#define LOG_TAG "MXLOG"
#include <utils/Log.h>

#include "livemxlog.h"

int LiveMxLog::pid = -1;
MxLogStatus LiveMxLog::mode = MXLOG_OFF_T;
void LiveMxLog::start_wifi_normal()
{
    if(pid != -1)
    {
        stop();
    }
    pid = MxLog::start(NULL, NULL, NULL, [](char* logBuffer){
        char* start;
        if ( logBuffer[1] > '4' || logBuffer[2] != '>')
        {
            return;
        }
        if ((start = strstr(logBuffer, "[wlbt]")) != NULL)
        {
            ALOGD("%s", start);
            if ((start = strstr(logBuffer, "FAULT_IND")) != NULL)
            {
                char tmp[128] = {0};
                char* token = NULL;
                int id = 0;
                uint32_t arg = 0;
                uint32_t count = 0;
                uint8_t index = 0;
                strncpy(tmp, start, (strlen(start) < sizeof(tmp)-1)? strlen(start) : sizeof(tmp)-1);
                strtok(tmp, " |"); // Get rid of FAULT_IND
                for (index = 0 ; (token = strtok(NULL, " |")) ; index++)
                {
                    if(index == 3)
                    {
                        id = (int)strtol(token, NULL, 16);
                    }
                    else if(index == 5)
                    {
                        arg = (uint32_t)strtol(token, NULL, 16);
                    }
                    else if(index == 7)
                    {
                        count = (uint32_t)strtol(token, NULL, 10);
                    }
                }
                if(index < 7)
                {
                    ALOGD("%s", start);
                    return;
                }
                if( (id & 0xF000) != 0x2000 )
                {
                    ALOGD("%s", start);
                    return;
                }
                const char* const fault_message = FaultInd::getInstance()->getFaultMessage(id);
                ALOGD("Fault Message : %s(%d) , Arg : 0x%x , count : %d", fault_message, id, arg, count);
            }
        }
        else
        {
            ALOGD("%s", logBuffer);
        }
    });
    if(pid != -1)
    {
        mode = MXLOG_WIFI_NORMAL_T;
        Common::ChangeLiveMxLogState(mode);
    }
}

void LiveMxLog::start_bt_normal()
{
    if(pid != -1)
    {
        stop();
    }
    pid = MxLog::start(WlbtLog::setup_bt_normal_log_filter, WlbtLog::revert_bt_log_filter, NULL, [](char* logBuffer){
        ALOGD("%s", logBuffer);
    });
    if(pid != -1)
    {
        mode = MXLOG_BT_NORMAL_T;
        Common::ChangeLiveMxLogState(mode);
    }
}

void LiveMxLog::start_bt_audio()
{
    if(pid != -1)
    {
        stop();
    }
    pid = MxLog::start(WlbtLog::setup_bt_audio_log_filter, WlbtLog::revert_bt_log_filter, NULL, [](char* logBuffer){
        ALOGD("%s", logBuffer);
    });
    if(pid != -1)
    {
        mode = MXLOG_BT_AUDIO_T;
        Common::ChangeLiveMxLogState(mode);
    }
}

void LiveMxLog::stop()
{
    if(pid != -1)
    {
        pid = -1;
        MxLog::stop();
        mode = MXLOG_OFF_T;
        Common::ChangeLiveMxLogState(mode);
    }
}
