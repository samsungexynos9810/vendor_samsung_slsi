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
 * rillogcapture.cpp
 *
 *  Created on: 2018. 4. 24.
 *      Author: sungwoo48.choi
 */

#include <utils/Log.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>

#include "rillog.h"
#include "reset_util.h"
#include "productfeature.h"
#include "rilproperty.h"
#include "rillogcapture.h"

#define NAME_SIZE 100
#define FILE_SIZE 20*1024*1024     //20MB
#define DUMP_BASE_DIR "/data/vendor/dump"
#define DUMP_FILE_NAME "rillogcapture"

IMPLEMENT_MODULE_TAG(RilLogCapture, RilLogCapture)

bool RilLogCapture::m_RilLogIsInited;
FILE *ril_log_capture = NULL;
FILE *logFileList = NULL;
struct tm *capT;
struct timeval val;
char first_boot[PROP_VALUE_MAX] = {0, };

RilLogCapture::RilLogCapture() {
    m_pRilLogCaptureThread = NULL;
    m_nRilLogPipeW = -1;
    m_nRilLogPipeR = -1;
    m_RilLogIsInited = false;
    m_pRilLogRMutex = NULL;
}

RilLogCapture::~RilLogCapture() {
    if (m_nRilLogPipeW != -1) {
        ALOGW("[%s::%s] Stop error: m_nRilLogPipeW is -1", TAG, __FUNCTION__);
        close(m_nRilLogPipeW);
        m_nRilLogPipeW = -1;
    }

    if (m_nRilLogPipeR != -1) {
        ALOGW("[%s::%s] Stop error: m_nRilLogPipeR is -1", TAG, __FUNCTION__);
        close(m_nRilLogPipeR);
        m_nRilLogPipeR = -1;
    }

    if (m_pRilLogRMutex) {
        delete m_pRilLogRMutex;
        m_pRilLogRMutex = NULL;
    }

    m_RilLogIsInited = false;
}

int RilLogCapture::Start() {
    ALOGV("[%s::%s] Start Ril Log capture", TAG, __FUNCTION__);
    m_pRilLogCaptureThread = new Thread(this);
    if (m_pRilLogCaptureThread == NULL || m_pRilLogCaptureThread->Start() < 0) {
        return -1;
    }
    return 0;
}

RilLogCapture *RilLogCapture::MakeInstance() {
    ALOGI("[%s::%s] ++", TAG, __FUNCTION__);

    if (m_RilLogIsInited) {
        ALOGE("[%s::%s] already initialized", TAG, __FUNCTION__);
        return NULL;
    }

    RilLogCapture *instance = NULL;
    instance = new RilLogCapture();
    if (instance != NULL) {
        if (instance->Init() < 0) {
            delete instance ;
            instance = NULL;
        }
    }

    m_RilLogIsInited = true;
    ALOGI("[%s::%s] --", TAG, __FUNCTION__);
    return instance;
}

int RilLogCapture::Init(void) {
    m_pRilLogRMutex = new CMutex();
    if(m_pRilLogRMutex == NULL) {
        ALOGE("[%s::%s] Fail to create RilLogRMutex instance", TAG, __FUNCTION__);
        return -1;
    }

    if (OpenMessagePipe() < 0) {
        ALOGE("[%s::%s] Fail to open message pipe", TAG, __FUNCTION__);
        return -1;
    }

    memset(first_boot, 0x00, sizeof(first_boot));
    property_get("radio.ril.reset_count", first_boot, "0");
    ALOGE("reset_count(%s)", first_boot);

    return 0;
}

// To make a caculation of next file number where log will be saved.
int RilLogCapture::setCurFileName(int num) {
    int newNum = (num+1)%5;

    logFileList = fopen(filelist, "w");
    if (logFileList != NULL) {
        fprintf(logFileList, "%d", newNum);
        fclose(logFileList);
        return newNum;
    }
    return num;
}

// To get the current file number where log is being saved.
int RilLogCapture::getCurFileName() {
    char m_fileName[NAME_SIZE] = {0, };
    int num = 0;

    logFileList = fopen(filelist, "r");
    if (logFileList != NULL) {
        int err;
        err = fscanf(logFileList, "%s", m_fileName);
        if (err != 1) ALOGE("[%s::%s]: File read error: %d", TAG, __FUNCTION__, err);
        fclose(logFileList);
        num = (int)(m_fileName[0] - '0');
        return num;
    }
    return 0;
}

int RilLogCapture::OpenMessagePipe() {
    ALOGE("[%s::%s]", TAG, __FUNCTION__);
    if (m_nRilLogPipeW != -1) {
        ALOGE("[%s::%s] Open command pipe again(m_nRilLogPipeW)", TAG, __FUNCTION__);
        close(m_nRilLogPipeW);
        m_nRilLogPipeW = -1;
    }

    if (m_nRilLogPipeR != -1) {
        ALOGE("[%s::%s] Open command pipe again(m_nRilLogPipeR)", TAG, __FUNCTION__);
        close(m_nRilLogPipeR);
        m_nRilLogPipeR = -1;
    }

    int fds[2];
    int n = pipe(fds);
    if (n < 0) {
        ALOGE("[%s::%s] Command pipe create fail", TAG, __FUNCTION__);
        return -1;
    }

    m_nRilLogPipeR = fds[0];
    m_nRilLogPipeW = fds[1];
    return 0;
}

int RilLogCapture::notifyNewRilLog(const char* rilLogMsg) {
    int ret = 0;

    if (m_nRilLogPipeW != -1) {
        m_pRilLogRMutex->lock();
        int buffSize = 1024;
        char buff[buffSize];

        gettimeofday(&val, NULL);
        capT = localtime(&val.tv_sec);

        snprintf(buff, sizeof(buff)-1, "%04d%02d%02d-%02d:%02d:%02d.%06ld\t%d\t%d\t",
                capT->tm_year+1900, capT->tm_mon+1, capT->tm_mday, capT->tm_hour, capT->tm_min, capT->tm_sec, val.tv_usec, getpid(), gettid());
        if (write(m_nRilLogPipeW, buff, strlen(buff)) <= 0)
            ret = -1;

        snprintf(buff, sizeof(buff)-1, "%s", rilLogMsg);
        if (write(m_nRilLogPipeW, buff, strlen(buff)) <= 0)
            ret = -1;

        snprintf(buff, sizeof(buff)-1, "\n");
        if (write(m_nRilLogPipeW, buff, strlen(buff)) <= 0)
            ret = -1;

        m_pRilLogRMutex->unlock();
    } else {
        ret = -1;
    }

    if (ret == -1)
        ALOGE("[%s::%s] Notify new message fail: m_nRilLogPipeW is -1", TAG, __FUNCTION__);

    return ret;
}

int rmdirs(const char *path, int is_error_stop) {
    DIR *dir_ptr = NULL;
    struct dirent *file = NULL;
    struct stat buf;
    char filename[1024];

    if ((dir_ptr = opendir(path)) == NULL)
        return unlink(path);

    while ((file = readdir(dir_ptr)) != NULL) {
        ALOGD("find %s/%s", path, file->d_name);
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0) {
            continue;
        }

        snprintf(filename, sizeof(filename)-1, "%s/%s", path, file->d_name);
        if (lstat(filename, &buf) == -1) {
            continue;
        }

        if (S_ISDIR(buf.st_mode)) {
            if(rmdirs(filename, is_error_stop) == -1 && is_error_stop) {
                return -1;
            }
        } else if (S_ISREG(buf.st_mode) || S_ISLNK(buf.st_mode)) {
            if (unlink(filename) == -1 && is_error_stop) {
                return -1;
            }
        }
    }
    closedir(dir_ptr);

    return rmdir(path);
}

void RilLogCapture::Run() {
    ALOGV("[%s::%s] Start Ril Log capture, pipe(%d)", TAG, __FUNCTION__, m_nRilLogPipeR);

    snprintf(filelist, sizeof(filelist)-1, "%s/cur/filelist", DUMP_BASE_DIR);
    if (strcmp(first_boot, "0") == 0) {
        // When first boot,
        // previous logs are removed and current logs are changed to previous logs.
        char comBuffer[NAME_SIZE] = {0, };
        char preComBuffer[NAME_SIZE] = {0, };

        snprintf(comBuffer, sizeof(comBuffer)-1, "%s/pre", DUMP_BASE_DIR);
        if (rmdirs(comBuffer, 0) != 0)
            ALOGE("fail to remove the previous log directory: %s", strerror(errno));

        snprintf(preComBuffer, sizeof(preComBuffer)-1, "%s/pre", DUMP_BASE_DIR);
        snprintf(comBuffer, sizeof(comBuffer)-1, "%s/cur", DUMP_BASE_DIR);
        if (rename(comBuffer, preComBuffer) != 0)
            ALOGE("fail to change the directory name: %s", strerror(errno));

        snprintf(comBuffer, sizeof(comBuffer)-1, "%s/cur", DUMP_BASE_DIR);
        if (mkdir(comBuffer, 0700) != 0)
            ALOGE("fail to make new directory: %s", strerror(errno));

        logFileList = fopen(filelist, "w");
        if (logFileList != NULL) {
            fprintf(logFileList, "0");
            fclose(logFileList);
        }
        snprintf(filestr, sizeof(filestr)-1, "%s/cur/%s0", DUMP_BASE_DIR, DUMP_FILE_NAME);
    } else {
        // Since second initialization,
        // it's continue to save the log to the previous file.
        int fileNum = getCurFileName();
        snprintf(filestr, sizeof(filestr)-1, "%s/cur/%s%d", DUMP_BASE_DIR, DUMP_FILE_NAME, fileNum);
    }

    unsigned int fileSize = getFileSize();
    int readSize = 0;
    ril_log_capture = fopen(filestr, "a+");

    while(true) {
        int buffSize = 1024;
        char buff[buffSize];
        memset(buff, 0, buffSize);
        if (m_nRilLogPipeR != -1) {
            if (fileSize >= FILE_SIZE) {
                int fileNum = 0;
                //char removeFile[NAME_SIZE] = {0, };

                fclose(ril_log_capture);

                fileNum = setCurFileName(getCurFileName());
                snprintf(filestr, sizeof(filestr)-1, "%s/cur/%s%d", DUMP_BASE_DIR, DUMP_FILE_NAME, fileNum);
                fileSize = 0;

                ril_log_capture = fopen(filestr, "w");
            }

            if ((readSize = read(m_nRilLogPipeR, buff, buffSize)) > 0) {
                buff[buffSize-1] = 0;
                if(ril_log_capture != NULL) {
                    fprintf(ril_log_capture, "%s", buff);
                    fflush(ril_log_capture);
                    fileSize = fileSize + readSize;
                }
            }
        }
    }
    return;
}

int RilLogCapture::getFileSize() {
    int fileLength = 0;
    ril_log_capture = fopen(filestr, "r");
    if (ril_log_capture != NULL) {
        fseek(ril_log_capture, 0, SEEK_END);
        fileLength = ftell(ril_log_capture);
        fclose(ril_log_capture);
        return fileLength;
    }
    return 0;
}

void RilLogCapture::OnClose() {
    ALOGV("[%s::%s] Close Ril Log capture", TAG, __FUNCTION__);
}

