 /*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <stdio.h>
#include <string.h>
#include "iochannel.h"
#include "rillog.h"

IoChannel::IoChannel(const char *name)
{
    m_Fd = -1;
    memset(m_szIoChannelName, 0, sizeof(m_szIoChannelName));
    if (name != NULL) {
        strncpy(m_szIoChannelName, name, sizeof(m_szIoChannelName)-1);
    }
    Init();
}

IoChannel::~IoChannel()
{
    Close();
}

INT32 IoChannel:: Init()
{
    if (*m_szIoChannelName == 0) {
        return -1;
    }
    return 0;
}

INT32 IoChannel:: GetFd()
{
    return m_Fd;
}

INT32 IoChannel:: Open()
{
    m_Fd = open(m_szIoChannelName, O_RDWR);
    if (m_Fd < 0) {
        RilLogE("%s: failed to open [%s].", __FUNCTION__, m_szIoChannelName);
        return -1;
    }

    RilLogV("%s: opened [%d,%s].", __FUNCTION__, m_Fd, m_szIoChannelName);
    return 0;
}

INT32 IoChannel:: Close()
{
    RilLogV("%s: close fd=%d", __FUNCTION__, m_Fd);
    if (m_Fd >= 0) {
        close(m_Fd);
        m_Fd = -1;
    }

    return 0;
}
INT32 IoChannel:: Read(char *buf, INT32 size)
{
    INT32 n = -1;

    if (buf == NULL) {
        return -1;
    }

    if (m_Fd < 0) {
        RilLogE("%s: failed to read fd[%d].", __FUNCTION__, m_Fd);
        return -1;
    }

    n = -1;
    while (n < 0) {
        n = read(m_Fd, buf, size);
        if (n < 0 && (errno == EINTR || errno == EAGAIN)) {
            usleep(10000);
        }
        else {
            break;
        }
    }

    return n;
}
INT32 IoChannel::Write(char *buf, INT32 size)
{
    INT32 cur = 0;
    INT32 len = size;
    INT32 written = 0;

    if (m_Fd < 0) {
        RilLogE("[%s:%s] File Descriptor Error!", __FILE__, __FUNCTION__);
        return -1;
    }

    while (cur < len) {
        do {
            if (written < 0) {
                usleep(10000);
            }
            written = write(m_Fd, buf + cur, len - cur);
        } while (written < 0 && (errno == EINTR || errno == EAGAIN));

        if (written < 0) {
            RilLogE("[%s:%s] Write Error!", __FILE__, __FUNCTION__);
            return -1;
        }

        cur += written;
    }

    return 0;
}
