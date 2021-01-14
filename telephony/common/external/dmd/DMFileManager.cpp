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
 * DMFileManager.cpp
 *
 *  Created on: 2018. 5. 18.
 */

#include "DMFileManager.h"
#include "dmd_main.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <set>
#include <cutils/properties.h>

static DMFileManager *sInstance = NULL;
#define DEFAULT_BASE_DIR   "/data/vendor/slog/"
#define DISK_USED_THRESHHOLD 85
#define MANAGED_FILE_COUNT_THRESHHOLD 1000

static const char *IGNORE_FILE_LIST[] = {
    "NNEXT_PROFILE.nprf",
    "sbuff_profile.sdm",
    ".sbuff_header.sdm",
};

static const char *MANAGED_EXT[] = {
    ".sdm", ".gz", ".zip"
};

static bool isIgnoreFile(string fileName) {
    unsigned int size = (unsigned int)(sizeof(IGNORE_FILE_LIST) / sizeof(IGNORE_FILE_LIST[0]));

    for (unsigned int i = 0; i < size; i++) {
        string filter = IGNORE_FILE_LIST[i];
        if (fileName.compare(filter) == 0) {
            ALOGD("matched in ignore file list. %s", filter.c_str());
            return true;
        }
    } // end for i ~
    return false;
}

static bool isManagedExt(string fileName) {
    unsigned int size = (unsigned int)(sizeof(MANAGED_EXT) / sizeof(MANAGED_EXT[0]));
    for (unsigned int i = 0; i < size; i++) {
        string ext = MANAGED_EXT[i];
        size_t pos = fileName.rfind(ext);
        if (pos != string::npos) {
            if (fileName.substr(pos).compare(ext) == 0) {
                //ALOGD("file ext is matched with %s", ext.c_str());
                return true;
            }
        }
    } // end for i ~
    return false;
}


DMFileManager::DMFileManager() : mCapacity(DISK_USED_THRESHHOLD), mManagedFileSize(0) {
    mBaseDir = DEFAULT_BASE_DIR;
    mTotalSize = 0;
    mMaxSize = 0;
}

DMFileManager::~DMFileManager() {

}

void DMFileManager::init() {
    createManagedFileList();
    shrink();
}

void DMFileManager::createManagedFileList() {
    DIR *dir;
    dir = opendir(mBaseDir.c_str());
    if (dir != NULL) {
        mTotalSize = 0;
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) {
                string fileName = entry->d_name;
                if (isManagedExt(fileName) && !isIgnoreFile(fileName)) {
                    //ALOGD("[%lu] %s", mFileList.size(), fileName.c_str());
                    struct DMFile dmFile;
                    dmFile.fileName = fileName;

                    string filePath = mBaseDir + fileName;
                    std::ifstream ifs(filePath, std::ios::in | std::ios::binary);
                    if (ifs.is_open() == 0) {
                        ALOGW("cannot open file(%s)", filePath.c_str());
                    }
                    ifs.seekg(0, std::ios::end);
                    dmFile.fileSize = ifs.tellg();
                    mTotalSize += dmFile.fileSize;
                    mFileList.push_back(dmFile);
                }
                else {
                    ALOGW("not a managed file. (%s)", fileName.c_str());
                }
            }
            else {
                //ALOGW("not a regular file (directory or others)");
            }
        } // end while ~
        closedir(dir);
    }
    else {
        //ALOGE("open directory(%s) error. errno=%d", mBaseDir.c_str(), errno);
    }
}

void DMFileManager::shrink() {

    while (mFileList.size() > 1) {
        struct statvfs sb;
        if (statvfs("/data", &sb) == 0) {
            char buf[128] = {0, };
            // MegaByte
            property_get("persist.vendor.sys.diag.log.maxsize", buf, "0");
            mMaxSize = atoi(buf);
            if (mMaxSize == 0) {
                // default max size : 85% of Total storage(/data)
                mMaxSize = (unsigned int)(sb.f_blocks * sb.f_bsize * mCapacity / 100 / MEGABYTE);
            }
            unsigned int size = mTotalSize / MEGABYTE;
            ALOGD("mMaxSize=%dMB TotalSize=%dMB", mMaxSize, size);
            if (size < mMaxSize) {
                break;
            }

            struct DMFile dmFile = mFileList.front();
            mFileList.pop_front();
            string path = mBaseDir;
            path += dmFile.fileName;

            if (unlink(path.c_str()) < 0) {
                ALOGE("failed to delete file %s. errno=%d", path.c_str(), errno);
                return ;
            }
            mTotalSize -= dmFile.fileSize;

            ALOGD("Remove old file %s", dmFile.fileName.c_str());
        }
    } // end while ~

    while (mManagedFileSize > 0 && mFileList.size() > mManagedFileSize) {
        struct DMFile dmFile = mFileList.front();
        mFileList.pop_front();
        string path = mBaseDir;
        path += dmFile.fileName;

        if (unlink(path.c_str()) < 0) {
            ALOGE("failed to delete file %s. errno=%d", path.c_str(), errno);
            return ;
        }
        mTotalSize -= dmFile.fileSize;

        ALOGD("Remove old file %s", dmFile.fileName.c_str());
    }

    ALOGD("Remained managed file size: %lu", mFileList.size());
}

void DMFileManager::add(const char *fileName) {
    if (fileName != NULL && *fileName != 0) {
        string str(fileName);
        add(str);
    }
    else {
        ALOGW("Invalid fileName.");
    }
}

void DMFileManager::add(const string &fileName) {
    struct DMFile dmFile;
    dmFile.fileName = fileName;
    string filePath = mBaseDir + fileName;
    std::ifstream ifs(filePath, std::ios::in | std::ios::binary);
    if (ifs.is_open() == 0) {
        ALOGW("cannot open file(%s)", filePath.c_str());
    }
    ifs.seekg(0, std::ios::end);
    dmFile.fileSize = ifs.tellg();
    mTotalSize += dmFile.fileSize;

    ALOGD("DMFileManager::%s fileNmae=%s fileSize=%d", __FUNCTION__, dmFile.fileName.c_str(), dmFile.fileSize);
    mFileList.push_back(dmFile);
    shrink();
}

void DMFileManager::removeAll() {
    ALOGD("%s", __FUNCTION__);
    while (mFileList.size() > 0) {
        for (list<DMFile>::iterator iter = mFileList.begin();
                iter != mFileList.end();
                iter++) {
            struct DMFile dmFile;
            string path = mBaseDir;
            path += dmFile.fileName;
            if (unlink(path.c_str()) < 0) {
                ALOGW("Failed to delete file %s.", dmFile.fileName.c_str());
                continue;
            }
            mTotalSize += dmFile.fileSize;
            ALOGD("Remove file %s", dmFile.fileName.c_str());
        } // end for ~
        mFileList.clear();
    }
    mTotalSize = 0;
}

void DMFileManager::refreshFileList() {
    ALOGD("%s", __FUNCTION__);
    mTotalSize = 0;
    mFileList.clear();
    createManagedFileList();
}

void DMFileManager::setLimit(unsigned int capacity, unsigned int managedFileSize) {
    mCapacity = capacity;
    if (mCapacity == 0 || mCapacity > DISK_USED_THRESHHOLD)
        mCapacity = DISK_USED_THRESHHOLD;

    mManagedFileSize = managedFileSize;
    if (mManagedFileSize == 0 || mManagedFileSize > MANAGED_FILE_COUNT_THRESHHOLD)
        mManagedFileSize = MANAGED_FILE_COUNT_THRESHHOLD;

    ALOGD("setLimit; capacity:%d managedFileSize:%d", mCapacity, mManagedFileSize);
}

void DMFileManager::setBaseDir(const char *baseDir) {
    if (baseDir != NULL && *baseDir != 0) {
        mBaseDir = baseDir;
        mBaseDir.at(mBaseDir.length() - 1);
    }
}

DMFileManager *DMFileManager::getInstance() {
    if (sInstance == NULL) {
        sInstance = new DMFileManager();
        if (sInstance != NULL) {
            char buf[128] = {0, };
            int capacity = -1;
            int managedFileSize = -1;

            property_get("vendor.sys.diag.log.capacity", buf, "0");
            capacity = atoi(buf);
            property_get("vendor.sys.diag.log.managed_size", buf, "0");
            managedFileSize = atoi(buf);
            sInstance->setLimit((unsigned int)capacity, (unsigned int)managedFileSize);
            property_get("vendor.sys.exynos.slog.path", buf, "");
            sInstance->setBaseDir(buf);
        }
    }
    return sInstance;
}
