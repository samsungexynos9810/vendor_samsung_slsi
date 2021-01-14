#define LOG_TAG "ExynosDisplayPanel-Hal"
#include "ExynosDisplayFeature.h"
#include "ExynosDisplayUtils.h"
#include "ExynosDisplayPanel.h"

#include <utils/Log.h>
#include <cmath>

using std::string;
using std::vector;
using namespace vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0;

#define STRING_LEN 16

int panelSetValue(const char* path, int value)
{
    FILE* fp;
    char writeString[STRING_LEN] = "";

    /*ALOGD("%s: %s, (%d)", __func__, path, value);*/

    if (value < 0) {
        ALOGE("invalid value");
        return -1;
    }

    if (snprintf(writeString, sizeof(writeString), "%d", value) < 0) {
        ALOGE("error print");
        return -1;
    }

    fp = fopen(path, "w");
    if (fp == NULL) {
        ALOGE("open file error: %s", path);
        return -1;
    }

    if (fputs(writeString, fp) < 0) {
        ALOGE("write error");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int panelHandleFeature(int mode, int value)
{
    int ret = 0;

    ALOGD("%s: %d, %d", __func__, mode, value);

    switch (mode) {
        case (int)PanelFeature::CABC_MODE :
            ret = panelSetValue("/sys/devices/platform/panel_0/cabc_mode", value);
            break;

        case (int)PanelFeature::HBM_MODE :
            ret = panelSetValue("/sys/devices/platform/panel_0/hbm_mode", value);
            break;

        default :
            ret = -1;
            ALOGE("%s: not supported!!", __func__);
            break;
    }

    return ret;
}
