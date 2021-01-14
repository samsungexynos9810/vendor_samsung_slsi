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
 * sced.cpp
 *
 * Created on: 2018. 05. 23
 */
#include "sced.h"
#include "OemServiceManager.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define SERVICE_NAME "sced0"
#define BUFFER_SIZE 512

static void OEM_OnRequest(int type, int id, void *data, unsigned int datalen);
static void DoShellExecution(int type, int id, char *data, unsigned int datalen);

static OemServiceManager sOemServiceManager;
static OEM_ServiceFunctions sOemServiceFunction = { OEM_OnRequest };

/**************************************************************************************/
// Entry function from HIDL server
static void OEM_OnRequest(int type, int id, void *data, unsigned int datalen) {
    char buf[BUFFER_SIZE +1] = {0, };
    if (data != NULL) {
        strncpy(buf, (char *)data, datalen);
    }

    ALOGD("%s, data: %s, size: %u", __FUNCTION__, buf, datalen);
    switch(id) {
        case COMMAND_LOGCAT:
        case COMMAND_LOGCAT_SNAPSHOT:
        case COMMAND_TCP_DUMP:
        case COMMAND_TCP_DUMP_SNAPSHOT:
            DoShellExecution(type, id, buf, datalen);
            break;
        case COMMAND_KILL:
            system(buf);
            break;
        default:
            ALOGE("unsupported message id. id = %d", id);
            break;
    }
}
/**************************************************************************************/

void DoShellExecution(int type, int id, char *data, unsigned int datalen) {
    ALOGD("%s", __FUNCTION__);

    char logfile[BUFFER_SIZE];
    char *bufcheck;
    char filemode[] = "0660";
    int fmode = strtol(filemode, 0, 8);
    unsigned int i = 0;

    // find file path
    for (i = 0; i < datalen; ++i) {
        bufcheck = &data[i];
        if (strncmp(bufcheck, "/data/", 6) == 0) {
            break;
        }
    }

    sprintf(logfile, bufcheck, datalen-i);
    ALOGD("file path is = %s", logfile);

    int filefd = open(logfile, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    if (filefd == -1) {
        ALOGD("%s: open %s failed. errno = %d", __FUNCTION__, logfile, errno);
    }else{
        close(filefd);
    }

    //change file permissions
    chmod (logfile, fmode);

    int pid = fork();
    if (pid == 0) {
        ALOGD("child buf : %s\n", data);
        if (execl("system/bin/sh", "system/bin/sh", "-c", data, NULL) < 0) {
            ALOGE("execl() fails. errno = %d", errno);
        }
    } else {
        ALOGD("Child PID is %d", pid);
        sOemServiceManager.notifyCallback(type, id, &pid, sizeof(int));
    }
}

int32_t main(void) {
    ALOGD("%s: Init OemServiceManager", __FUNCTION__);

    if (sOemServiceManager.init()) {
        sOemServiceManager.registerService(SERVICE_NAME, &sOemServiceFunction);
    }

    while(1) {
        sleep((unsigned int)-1);
    }
    return 0;
}
