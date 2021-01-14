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
    RFS I/O Reader implementation
*/

#include "rfschannel.h"

CRfsIoChannel::CRfsIoChannel()
{
    mFd = -1;
}

CRfsIoChannel::~CRfsIoChannel()
{
    Close();
}

int CRfsIoChannel::Open(const char *name)
{
    ALOGI("[RfsService::IoChannel] %s", __FUNCTION__);

    if (mFd > -1) {
        ALOGW("[RfsService::IoChannel] channel already opened");
        return 0;
    }

    mFd = open(name, O_RDWR);
    if (mFd < 0) {
        ALOGE("[RfsService::IoChannel] failed to open channel %s", name);
        return -1;
    }

    ALOGV("[RfsService::IoChannel] channel opened [%d, %s]", mFd, name);
    return 0;
}

int CRfsIoChannel::Close()
{
    ALOGI("[RfsService::IoChannel] %s", __FUNCTION__);

    if (mFd > -1) {
        close(mFd);
        mFd = -1;
    }

    return 0;
}

int CRfsIoChannel::Read(char *buf, int size)
{
    int n = -1;

    ALOGV("[RfsService::IoChannel] %s", __FUNCTION__);

    if (buf == NULL) {
        return -1;
    }

    if (mFd < 0) {
        ALOGE("[RfsService::IoChannel] failed to read channel");
        return -1;
    }

    n = -1;
    while (n < 0) {
        n = read(mFd, buf, size);
        if (n < 0 && (errno == EINTR || errno == EAGAIN)) {
            usleep(10000);
        } else {
            break;
        }
    }

    ALOGV("[RfsService::IoChannel] read %d bytes", n);
    return n;
}

int CRfsIoChannel::Write(char *buf, int size)
{
    size_t cur = 0;
    size_t len = size;
    ssize_t written = 0;

    ALOGV("[RfsService::IoChannel] %s", __FUNCTION__);

    if (mFd < 0) {
        return -1;
    }

        ALOGV("[RfsService::IoChannel] write %d bytes", (int)len);

    while (cur < len) {
        do {
            if (written < 0) {
                usleep(10000);
            }
            written = write(mFd, buf + cur, len - cur);
        } while (written < 0 && (errno == EINTR || errno == EAGAIN));

        if (written < 0) {
            return -1;
        }

        cur += written;
    }

    return 0;
}
