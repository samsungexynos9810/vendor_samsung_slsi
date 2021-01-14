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
 * DMAgent.cpp
 *
 *  Created on: 2018. 5. 18.
 */

#include "DMAgent.h"
#include "DMConstants.h"
#include "DMFileManager.h"
#include "OemServiceManager.h"
#include "dmd_main.h"
#include <unistd.h>
#include <stdio.h>
#include <string>

#define SERVICE_NAME    "dm0"
#define SERVICE_NAME_FM "dm1"

static void OEM_OnRequest(int type, int id, void *data, unsigned int datalen);
static void FDM_OnRequest(int type, int id, void *data, unsigned int datalen);

static OemServiceManager sOemServiceManager;
static OemServiceManager sOemServiceManagerFm;
static OEM_ServiceFunctions sOemServiceFunction = { OEM_OnRequest };
static OEM_ServiceFunctions sFactoryFunction = { FDM_OnRequest };


static int sActiveMode = MODE_NONE;
static void (*sActiveRespHandler)(void *data, unsigned int datalen);

static void ExternalDM_OnProcess(void *data, unsigned int datalen);
static void SilentLogging_OnProcess(void *data, unsigned int datalen);
static void ExtApp_OnProcess(void *data, unsigned int datalen);

int DMAgent_Init() {
    sActiveRespHandler = NULL;
    if (sOemServiceManager.init()) {
        sOemServiceManager.registerService(SERVICE_NAME, &sOemServiceFunction);
    }
    if (sOemServiceManagerFm.init()) {
        sOemServiceManagerFm.registerService(SERVICE_NAME_FM, &sFactoryFunction);
    }
    return 0;
}

void DMAgent_SetActiveMode(int activeMode) {
    sActiveMode = activeMode;
    switch (sActiveMode) {
    case MODE_EXTERNAL_DM:
        sActiveRespHandler = ExternalDM_OnProcess;
        break;
    case MODE_SILENT_LOGGING:
        sActiveRespHandler = SilentLogging_OnProcess;
        break;
    case MODE_EXT_APP_DM:
        sActiveRespHandler = ExtApp_OnProcess;
        break;
    default:
        ALOGE("Unsupported DMAgent. activeMode=%d", activeMode);
        sActiveMode = MODE_NONE;
        sActiveRespHandler = NULL;
        break;
    } // end switch ~
}

int DMAgent_GetActiveMode() {
    return sActiveMode;
}

int DMAgent_OnResponse(void *data, unsigned int datalen) {
    if (data != NULL && datalen > 0) {
        if (sActiveRespHandler != NULL) {
            sActiveRespHandler(data, datalen);
        }
        else {
            ALOGE("No active ResponseHandler. current mode is %d", sActiveMode);
        }
    }
    else {
        ALOGE("invalid response data");
    }

    return 0;
}

void ExternalDM_OnProcess(void *data, unsigned int datalen) {

}

void SilentLogging_OnProcess(void *data, unsigned int datalen) {

}

void ExtApp_OnProcess(void *data, unsigned int datalen) {
    ALOGE("%s", __FUNCTION__);
    sOemServiceManager.notifyCallback(TYPE_RAW, COMMAND_NOTIFY_DM_LOG, data, datalen);

}

/**************************************************************************************/
/**************************************************************************************/
// types of handler
static void OnCommandHandler(int id, void *data, unsigned int datalen);
static void OnRawHandler(int id, void *data, unsigned int datalen);
static void OnFactoryCommandHandler(int id, void *data, unsigned int datalen);

// command handler
static void SetCurrentMode(int mode);
static void SendProfile(void *data, unsigned int datalen);
static void StopDM();
static void SaveSnapshot();
static void RefreshFileList();
static void SaveAutoLog();
static void CheckMaxSize();
static void SetDmMaxFileSize(int newSize);


// factory command handler
static void TouchSdmHeader(bool remove);

void OnCommandHandler(int id, void *data, unsigned int datalen) {
    ALOGD("%s", __FUNCTION__);
    if (id == COMMAND_SET_DM_MODE) {
        int newMode = *(int *)data;
        SetCurrentMode(newMode);
    }
    else if (id == COMMAND_SEND_PROFILE) {
        SendProfile(data, datalen);
    }
    else if (id == COMMAND_STOP_DM) {
        StopDM();
    }
    else if (id == COMMAND_SAVE_SNAPSHOT) {
        SaveSnapshot();
    }
    else if (id == COMMAND_REFRESH_FILE_LIST) {
        RefreshFileList();
    }
    else if (id == COMMAND_SAVE_AUTOLOG) {
        SaveAutoLog();
    }
    else if (id == COMMAND_SET_DM_MAX_FILE_SIZE) {
        int size = *(int *)data;
        SetDmMaxFileSize(size);
    }
}

void OnRawHandler(int id, void *data, unsigned int datalen) {
    ALOGD("%s Request to some DM command. for example stop req sending DM message", __FUNCTION__);
    if (EnableLogDump()){
        log_hexdump((const char *)data, datalen);
    }
    //write to modem, stop req, start/stop tcp packet req. etc
    dmdWriteUsbToModem(g_modem_fd, (char *)data, datalen);
}

void OnFactoryCommandHandler(int id, void *data, unsigned int datalen) {
    ALOGD("%s", __FUNCTION__);
    if (id == FACTORY_COMMAND_SET_DM_MODE) {
        if (data == NULL || datalen < sizeof(int)) {
            ALOGE("Invalid parameters");
            return ;
        }

        int mode = *((int *)data);
        char *profile = NULL;
        unsigned int profilelen = 0;
        if (datalen >= sizeof(int)) {
            profile = (char *)data + sizeof(int);
            profilelen = datalen - (unsigned int)sizeof(int);
        }

        if (DMAgent_GetActiveMode() != mode) {
            switch (mode) {
            case MODE_EXTERNAL_DM:
                ALOGI("MODE_EXTERNAL_DM");
                // 1. stop silent logging
                // 2. change mode
                // 3. remove sdm header
                StopDM();
                SetCurrentMode(MODE_EXTERNAL_DM);
                TouchSdmHeader(true);
                property_set("persist.vendor.sys.silentlog", "");
                property_set("persist.vendor.sys.silentlog.cp", "");
                break;
            case MODE_SILENT_LOGGING:
                ALOGI("MODE_SILENT_LOGGING");
                // 1. save header
                // 2. change mode
                // 3. send profile including start command
                TouchSdmHeader(false);
                SetCurrentMode(MODE_SILENT_LOGGING);
                if (profile != NULL && profilelen > 0) {
                    SendProfile(profile, profilelen);
                }
                property_set("persist.vendor.sys.silentlog", "On");
                property_set("persist.vendor.sys.silentlog.cp", "On");
                break;
            default:
                ALOGE("unsupported mode. mode=%d", mode);
                break;
            } // end switch ~
        }
        else {
            ALOGI("DM mode is not changed.");
        }
    }
    else {
        ALOGE("unsupported id. id=%d", id);
    }
}

void SetCurrentMode(int mode) {
    ALOGD("%s", __FUNCTION__);

    // init for further
    DMAgent_SetActiveMode(mode);

    if (mode == MODE_EXTERNAL_DM) {
        ALOGD("Set to ShannonDM mode");
        if(g_dmd_mode == MODE_SILENT) {
            g_silent_logging_started = 0;
            g_silent_logging_asked = 0;
            std::string profilePath(gSlogPath);
            profilePath += "sbuff_profile.sdm";
            if (remove(profilePath.c_str()) != 0)    // remove profile
            {
                ALOGE("%s : remove error(%s)", __FUNCTION__, strerror(errno));
            }
            OnSilentLogStop();    // reset buffers and silent log variables --ResetSilentLogValues
        }
        property_set("vendor.sys.silentlog.on", "");
        g_dmd_mode = MODE_NNEXT;
    }
    else if (mode == MODE_SILENT_LOGGING) {
        ALOGD("Set to Silent Logging mode");
        if(g_dmd_mode == MODE_NNEXT) {
            set_cp2usb_path(false);
            gTimeForFile = CalcTime();
            MergeHeader(gTimeForFile);     // insert header in first file for first time after boot
        }
        g_dmd_mode = MODE_SILENT;
        g_silent_logging_asked=1;
        set_cp2usb_path(false);
        CheckMaxSize();
    }
    else if (mode == MODE_EXT_APP_DM) {
        ALOGD("Set to External App mode");
        if(g_dmd_mode == MODE_SILENT) {
            //error return;
        }
        else
            g_dmd_mode = MODE_EXT_APP;
    }
    else {
        ALOGE("Unsupported mode. activeMode=%d", mode);
    }
}

void SendProfile(void *data, unsigned int datalen) {
    ALOGD("%s Send Profile data to CP", __FUNCTION__);
    if (EnableLogDump()){
        log_hexdump((const char *)data, datalen);
    }

    if (g_dmd_mode == MODE_SILENT) {
        DoSaveProfile((char *)data, datalen);
    }
    dmd_modem_write(g_modem_fd, (char *)data, datalen); //write to modem
}

void StopDM() {
    ALOGD("%s Stop DM", __FUNCTION__);
    char REQUEST_STOP[] = {0x7F, 0x0a, 0x00, 0x00, 0x07, 0x00, 0x00, 0xFF, 0xa0, 0x00, 0x02, 0x7E};
    unsigned int size = (unsigned int)(sizeof(REQUEST_STOP) / sizeof(REQUEST_STOP[0]));
    dmd_modem_write(g_modem_fd, (char *)REQUEST_STOP, size); //write to modem
    int ret = DoZipOperation(gTimeForFile);
    sOemServiceManager.notifyCallback(TYPE_COMMAND, COMMAND_STOP_DM, &ret, sizeof(int));
}

void SaveSnapshot() {
    ALOGD("%s Save CP DM message into files", __FUNCTION__);
    trigger_save_dump();
    // TODO need to check to return result to the caller
    /*
    if ((send(client_sock, socket_buf , 1, MSG_EOR)) < 0) {
        ALOGD("send failed for trigger_save_dump = %d\n  %s",serv_sock, strerror(errno));
    } else {
        ALOGD("----------Sent trigger_save_dump complete----------\n");
    }
    */
}

void RefreshFileList() {
    ALOGD("%s", __FUNCTION__);
    DMFileManager *fileManager = DMFileManager::getInstance();
    if (fileManager != NULL) {
        fileManager->refreshFileList();
    }
}

void SaveAutoLog() {
    ALOGD("%s() Save Auto Log into files", __FUNCTION__);
    int ret = saveAutoLog();
    sOemServiceManager.notifyCallback(TYPE_COMMAND, COMMAND_SAVE_AUTOLOG, &ret, sizeof(int));
}

void CheckMaxSize() {
    ALOGD("%s", __FUNCTION__);
    DMFileManager *fileManager = DMFileManager::getInstance();
    if (fileManager != NULL) {
        fileManager->shrink();
    }
}

void SetDmMaxFileSize(int newSize) {
    ALOGD("%s() Set DmMaxFileSize=%d", __FUNCTION__, newSize);
    gDmFileMaxSize = newSize * MEGABYTE;
}

static void TouchSdmHeader(bool remove) {
    char buf[256] = { 0, };
    property_get(PROP_SLOG_PATH, buf, DEF_SLOG_PATH);
    std::string path = buf;
    path += ".sbuff_header.sdm";

    if (remove) {
        if (unlink(path.c_str()) < 0) {
            ALOGE("Failed to remove header. errno=%d", errno);
        }
    }
    else {
        char header[] = {
            0x32, 0x00, 0x39, 0xFD, 0x04, 0x00, 0x02, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x53, 0x69, 0x6c, 0x65,
            0x6e, 0x74, 0x20, 0x4c, 0x6f, 0x67, 0x67, 0x69, 0x6e, 0x67, 0x28, 0x46, 0x61, 0x63, 0x74, 0x6f,
            0x72, 0x79, 0x20, 0x4d, 0x6f, 0x64, 0x65, 0x29, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00
        };

        size_t size = sizeof(header) / sizeof(header[0]);
        FILE *out = fopen(path.c_str(), "w");
        if (out != NULL) {
            size_t ret = fwrite(header, 1, size, out);
            fclose(out);
            if (ret != size) {
                ALOGW("header may not be valid. Remove.");
                if (unlink(path.c_str()) < 0) {
                    ALOGE("Failed to remove header. errno=%d", errno);
                }
            }
        }
    }
}

// Entry function from HIDL server
static void OEM_OnRequest(int type, int id, void *data, unsigned int datalen) {
    ALOGD("OEM_OnRequest type=%d id=%d, datalen=%u", type, id, datalen);

    if (type == TYPE_COMMAND) {
        OnCommandHandler(id, data, datalen);
    }
    else if (type == TYPE_RAW) {
        OnRawHandler(id, data, datalen);
    }
    else {
        ALOGE("unsupported type. type=%d", type);
    }
}

static void FDM_OnRequest(int type, int id, void *data, unsigned int datalen) {
    ALOGD("OEM_OnRequest type=%d id=%d, datalen=%u", type, id, datalen);
    if (type == TYPE_FACTORY_COMMAND) {
        OnFactoryCommandHandler(id, data, datalen);
    }
    else {
        ALOGE("unsupported type. type=%d", type);
    }
}
