#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/system_properties.h>
#include <ctime>

#define LOG_TAG "WLBTLOGDBG"
#include <utils/Log.h>

#include "common.h"
#include "filedir.h"
#include "livemxlog.h"
#include "wlbtlog.h"

#define MAX_FILE_SIZE_PROPOERTY "persist.vendor.wlbtlog.maxfilesize"
#define MAX_FILES_PROPOERTY "persist.vendor.wlbtlog.maxfiles"
#define LIVE_MXLOG_STATUS_PROPOERTY "persist.vendor.wlbtlog.livemxlog"
#define FILE_MXLOG_STATUS_PROPOERTY "persist.vendor.wlbtlog.filemxlog"
#define UDILOG_STATUS_PROPOERTY "persist.vendor.wlbtlog.udilog"

MxLogStatus  Common::live_mxlog_status;
MxLogStatus  Common::file_mxlog_status;
UdiLogStatus Common::udilog_status;

int Common::max_file_size = DEFAULT_MAX_FILE_SIZE;
int Common::max_files = DEFAULT_MAX_FILES;

void Common::ChangeLiveMxLogState(MxLogStatus s)
{
    live_mxlog_status = s;
    if(live_mxlog_status != MXLOG_OFF_T && file_mxlog_status != MXLOG_OFF_T)
    {
        ALOGE("Enable livemxlog and stop filemxlog");
        WlbtLog::stop_mxlog();
    }
    setPropertyValue(LIVE_MXLOG_STATUS_PROPOERTY, live_mxlog_status);
}

void Common::ChangeFileMxLogState(MxLogStatus s)
{
    file_mxlog_status = s;
    if(file_mxlog_status != MXLOG_OFF_T && live_mxlog_status != MXLOG_OFF_T)
    {
        ALOGE("Enable filemxlog and stop livemxlog");
        LiveMxLog::stop();
    }
    setPropertyValue(FILE_MXLOG_STATUS_PROPOERTY, file_mxlog_status);
}

void Common::ChangeUdiLogState(UdiLogStatus s)
{
    udilog_status = s;
    setPropertyValue(UDILOG_STATUS_PROPOERTY, udilog_status);
}

void Common::LoadConfiguration()
{
    char time_str_buffer[128];
    char base_dir_buffer[128];
    char output_path_buffer[256];
    char buffer[PROP_VALUE_MAX + 1];

    ALOGD("LoadConfiguration");

    memset(base_dir_buffer, 0, sizeof(base_dir_buffer));
    if(__system_property_get("exynos.wlbtlog.path", base_dir_buffer) == 0)
    {
        strcpy(base_dir_buffer, "/data/vendor/log/wlbt/");
        __system_property_set("exynos.wlbtlog.path", base_dir_buffer);
    }

    if(__system_property_get("exynos.sdcard.path.log", buffer) == 0)
    {
        __system_property_set("exynos.sdcard.path.log", "/sdcard/bbklog/");
    }

    get_time_string(time_str_buffer, sizeof(time_str_buffer));

    memset(output_path_buffer, 0, sizeof(output_path_buffer));
    sprintf(output_path_buffer, " %s%s", base_dir_buffer, time_str_buffer);

    create_dir(base_dir_buffer);
    int live_mxlog_property = getPropertyValue(LIVE_MXLOG_STATUS_PROPOERTY);
    int file_mxlog_property = getPropertyValue(FILE_MXLOG_STATUS_PROPOERTY);
    int udilog_property = getPropertyValue(UDILOG_STATUS_PROPOERTY);

    ALOGE("get value from conf files live_mxlog_property %d file_mxlog_property %d udilog_property %d",
		live_mxlog_property, file_mxlog_property, udilog_property);

    if (live_mxlog_property == -1 || file_mxlog_property == -1 || udilog_property == -1) { // init case
        ALOGD("This is init case");
        initDefaultValue();
        return;
    } else {
        live_mxlog_status = (MxLogStatus) live_mxlog_property;
        file_mxlog_status = (MxLogStatus) file_mxlog_property;
        udilog_status = (UdiLogStatus) udilog_property;
        update_max_file_size();
        update_max_files();
        ALOGD("get value from conf files live_mxlog_status %u file_mxlog_status %u udilog_status %u",
            live_mxlog_status, file_mxlog_status, udilog_status);
    }

    switch(live_mxlog_status)
    {
        case MXLOG_WIFI_NORMAL_T:
        ALOGD("setprop exynos.wlbtlog.livemxlog wifi_normal");
        system("setprop exynos.wlbtlog.livemxlog wifi_normal");
        break;
        case MXLOG_BT_NORMAL_T:
        ALOGD("setprop exynos.wlbtlog.livemxlog bt_normal");
        system("setprop exynos.wlbtlog.livemxlog bt_normal");
        break;
        case MXLOG_BT_AUDIO_T:
        ALOGD("setprop exynos.wlbtlog.livemxlog bt_audio");
        system("setprop exynos.wlbtlog.livemxlog bt_audio");
        break;
        default:
        ALOGD("setprop exynos.wlbtlog.livemxlog none");
        system("setprop exynos.wlbtlog.livemxlog none");
        break;
    }
    switch(file_mxlog_status)
    {
        case MXLOG_WIFI_NORMAL_T:
        {
            sprintf(output_path_buffer, "%s/mxlog.log", output_path_buffer);
            WlbtLog::start_mxlog(output_path_buffer, -1);
        }
        break;
        case MXLOG_BT_NORMAL_T:
        {
            sprintf(output_path_buffer, "%s/bt_normal.log", output_path_buffer);
            WlbtLog::start_bt_normal_log(output_path_buffer, -1);
        }
        break;
        case MXLOG_BT_AUDIO_T:
        {
            sprintf(output_path_buffer, "%s/bt_audio.log", output_path_buffer);
            WlbtLog::start_bt_audio_log(output_path_buffer, -1);
        }
        break;
        default:
        ALOGD("Stop filemxlog");
        break;
    }

    memset(output_path_buffer, 0, sizeof(output_path_buffer));
    sprintf(output_path_buffer, " %s%s", base_dir_buffer, time_str_buffer);
    switch(udilog_status)
    {
        case UDILOG_ON_T:
        {
            sprintf(output_path_buffer, "%s/udilog.log", output_path_buffer);
            WlbtLog::start_udilog(output_path_buffer, -1);
        }
        break;

        default:
        ALOGD("Stop udilog");
        break;
    }
}

void Common::initDefaultValue() {
    ALOGD("initDefaultValue");
    live_mxlog_status = MXLOG_OFF_T;
    file_mxlog_status = MXLOG_OFF_T;
    udilog_status = UDILOG_OFF_T;
    max_file_size = getPropertyValue(MAX_FILE_SIZE_PROPOERTY);
    max_files = getPropertyValue(MAX_FILES_PROPOERTY);

    setPropertyValue(LIVE_MXLOG_STATUS_PROPOERTY, MXLOG_OFF_T);
    setPropertyValue(FILE_MXLOG_STATUS_PROPOERTY, MXLOG_OFF_T);
    setPropertyValue(UDILOG_STATUS_PROPOERTY, UDILOG_OFF_T);
    if(max_file_size == -1) {
        setPropertyValue(MAX_FILE_SIZE_PROPOERTY, DEFAULT_MAX_FILE_SIZE);
        max_file_size = DEFAULT_MAX_FILE_SIZE;
    }
    if(max_files == -1) {
        setPropertyValue(MAX_FILES_PROPOERTY, DEFAULT_MAX_FILES);
        max_files = DEFAULT_MAX_FILES;
    }
}

void Common::setPropertyValue(const char *propertyValue, int value) {
    ALOGD("setPropertyValue propertyValue %s value %d ", propertyValue, value);
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "setprop %s %u", propertyValue, value);
    system(buffer);
}

int Common::getPropertyValue(const char *propertyValue) {
    char buffer[PROP_VALUE_MAX + 1];
    int value;
    if(__system_property_get(propertyValue, buffer) != 0)
    {
        if(sscanf(buffer, "%d", &value) != 1)
        {
            value = -1;
        }
    } else {
        value = -1;
    }

    ALOGD("getPropertyValue %s : %d ", propertyValue, value);
    return value;
}

void Common::update_max_file_size()
{
    char buffer[PROP_VALUE_MAX + 1];
    if(__system_property_get(MAX_FILE_SIZE_PROPOERTY, buffer) != 0)
    {
        if(sscanf(buffer, "%u", &Common::max_file_size) != 1)
        {
            Common::max_file_size = DEFAULT_MAX_FILE_SIZE;
        }
        ALOGD("update_max_file_size / getproperty : %u ", max_file_size);
    }
}

void Common::update_max_files()
{
    char buffer[PROP_VALUE_MAX + 1];
    if(__system_property_get(MAX_FILES_PROPOERTY, buffer) != 0)
    {
        if(sscanf(buffer, "%u", &Common::max_files) != 1)
        {
            Common::max_files = DEFAULT_MAX_FILES;
        }
        ALOGD("update_max_files / getproperty : %u ", max_files);
    }
}

void Common::send_reply(int sock, int pid)
{
    char reply_buffer[16];
    if(sock == -1)
    {
        return;
    }
    memset(reply_buffer, 0x0, sizeof(reply_buffer));
    sprintf(reply_buffer, "%d", pid);
    send(sock, reply_buffer, strlen(reply_buffer), 0);
}

void Common::get_time_string(char *buffer, unsigned int size)
{
    time_t rawtime;
    struct tm *timeinfo;

    memset(buffer, 0, size);
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, size, "Log_%m%d%H%M%S", timeinfo);
}
