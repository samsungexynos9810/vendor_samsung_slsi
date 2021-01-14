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
 * modemstatemonitor.cpp
 *
 *  Created on: 2014. 11. 17.
 *      Author: sungwoo48.choi
 */


#include "modemstatemonitor.h"
#include "rfsservice.h"
#include "main.h"
#include <sys/poll.h>

ModemStateMonitor::ModemStateMonitor(CRfsService *pRfsSrv)
{
    m_pRfsSrv = pRfsSrv;
    m_pMonitorThread = NULL;
}

ModemStateMonitor::~ModemStateMonitor()
{

}

int ModemStateMonitor::Start()
{
    m_pMonitorThread = new Thread(this);
    if (m_pMonitorThread->Start() < 0) {
        ALOGE("[ModemStateMonitor::%s] thread errors", __FUNCTION__);
        return -1;
    }
    ALOGD("[ModemStateMonitor::%s] thread ok", __FUNCTION__);

    return 0;
}

INT32 ModemStateMonitor::GetModemStatus(INT32 fd)
{
    INT32 status = 0;
    ALOGE("[ModemStateMonitor::%s]", __FUNCTION__);

    if ( fd < 0 )
    {
        ALOGE("cannot get modem status - due to fd is invalid(%d)", fd);
        return -1;
    }

    status = ioctl(fd, IOCTL_MODEM_STATUS);

    return status;
}


void ModemStateMonitor::OnClose()
{
}

void ModemStateMonitor::Run()
{
    int32_t fd;
    struct pollfd pollfd;
    int ret = 0;
    int state = -1;
    char checkNVProp[RFS_PATH_LEN];

    fd = open("/dev/umts_boot0", O_RDWR);
    pollfd.fd = fd;
    pollfd.events = POLLHUP | POLLIN | POLLRDNORM;
    pollfd.revents = 0;

    ALOGD("[ModemStateMonitor::%s] Start modem state monitor!", __FUNCTION__);
    while(1)
    {
        pollfd.revents = 0;
        ret = poll(&pollfd, 1, -1);

        if((pollfd.revents & POLLHUP) || (pollfd.revents & POLLIN) || (pollfd.revents & POLLRDNORM))
        {
            ALOGD("[ModemStateMonitor::%s] receive poll event!!!", __FUNCTION__);
            const unsigned int interval = 2;
            bool reCheckNV = false;
            while(true) {
                state = ioctl(fd, IOCTL_MODEM_STATUS);
                ALOGD("[ModemStateMonitor::%s] modem status [%d]", __FUNCTION__, state);
                if(state == STATE_OFFLINE || state == STATE_BOOTING) {
                    reCheckNV = true;
                    int spin = 100;
                    while(spin--) {
                        state = ioctl(fd, IOCTL_MODEM_STATUS);
                        if(state == STATE_ONLINE) {
                            break;
                        }
                        usleep(100 * 1000);
                    }
                    if(spin < 0) {
                        ALOGE("[ModemStateMonitor::%s] Modem boot timeout", __FUNCTION__);
                        reCheckNV = false;
                    }
                }
                else if(state == STATE_CRASH_EXIT || state == STATE_CRASH_RESET) {
                    ALOGD("[ModemStateMonitor::%s] Modem  is STATE_CRASH_EXIT or STATE_CRASH_RESET", __FUNCTION__);
                    ALOGD("[ModemStateMonitor::%s] Check status after %d seconds again.", __FUNCTION__, interval);
                    memset(checkNVProp, 0, sizeof(checkNVProp));
                    property_get(RFS_PROP_NV_CHECK_DONE, checkNVProp , "-1");
                    ALOGI("[ModemStateMonitor::%s] RFS_PROP_NV_CHECK_DONE = %s", __FUNCTION__, checkNVProp);
                    if(checkNVProp[0] != '0') {
                        ALOGI("[ModemStateMonitor::%s] set RFS_PROP_NV_CHECK_DONE to 0.", __FUNCTION__);
                        property_set(RFS_PROP_NV_CHECK_DONE, "0");
                    }
                    reCheckNV = true;
                    sleep(interval);
                    continue;
                }

                if(state == STATE_ONLINE) {
                    ALOGD("[ModemStateMonitor::%s] Modem is ONLINE", __FUNCTION__);
                    property_set(RFS_PROP_NV_CHECK_DONE, "0");
                    if(firstBoot) {
                        firstBoot = false;
                        break;
                    }
                    if(reCheckNV) {
                        m_pRfsSrv->CheckNvFiles();
                    }
                }
                break;
            }
        }
        else {
            ALOGE("[ModemStateMonitor::%s] unknown poll event!", __FUNCTION__);
            usleep(200000);
        }
    }
    close(fd);
    ALOGV("[ModemStateMonitor::%s] Stop monitoring Modem Status ...", __FUNCTION__);
    return;
}
