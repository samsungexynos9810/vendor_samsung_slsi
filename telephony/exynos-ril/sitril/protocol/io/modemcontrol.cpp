/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include "modemcontrol.h"

// add category to display selective logs
#undef RilLogI
#define RilLogI(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_INFO_LOG,  format, ##__VA_ARGS__)
#undef RilLogV
#define RilLogV(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_VERBOSE_LOG,  format, ##__VA_ARGS__)
#undef RilLogW
#define RilLogW(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_WARNING_LOG,  format, ##__VA_ARGS__)
#undef RilLogE
#define RilLogE(format, ...) CRilLog::Log(RIL_LOG_CAT_CORE, CRilLog::E_RIL_CRITICAL_LOG,  format, ##__VA_ARGS__)

ModemControl::ModemControl()
{
}

ModemControl::~ModemControl()
{
}

INT32 ModemControl::CrashModem(char *_reason)
{
    int fd = 0;
    RilLogI("%s()", __FUNCTION__);

    if ((fd = open("/dev/umts_ipc0", O_RDWR)) < 0)
    {
        RilLogE("/dev/umts_ipc0  open error: errno %d", errno);
        return -1;
    }

    if (ioctl(fd, IOCTL_MODEM_FORCE_CRASH_EXIT, _reason) == -1)
    {
        RilLogE("IOCTL_MODEM_FORCE_CRASH_EXIT Failed.");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;

}

INT32 ModemControl::WaitingForBootDone()
{
    int fd = -1;
    int status = 0;
    int spin = 100;
    int err = 0;

    RilLogV("[%s]Wait modem boot ready.",__FUNCTION__);

    fd = open(MODEM_BOOT, O_RDWR);
    if (fd < 0)
    {
        RilLogE("Open MODEM_BOOT error: %s", strerror(errno));
        return -1;
    }

    /**
     * If rild had been reset, cbd would be reset and modem status was changed while
     * ril init, so if cp had been reset by rild, wait until modem status changed
     */
    while (spin--)
    {
        status = ioctl(fd, IOCTL_MODEM_STATUS);
        if (status == STATE_ONLINE)
        {
            RilLogV("Modem  is ONLINE");
            break;
        }
        usleep(500000);
    }

    if (spin < 0)
    {
        RilLogE("Modem boot timeout");
        err = -1;
    }

    if (fd != -1)
    {
        close(fd);
    }

    return err;
}

INT32 ModemControl::CheckModemStatus(INT32 fd)
{
    INT32 status = 0;

    if ( fd < 0 )
    {
        RilLogE("cannot get modem status - due to fd is invalid(%d)", fd);
        return -1;
    }

    status = ioctl(fd, IOCTL_MODEM_STATUS);

    if ( status == STATE_ONLINE )
    {
        return 0;
    }

    switch(status)
    {
    case STATE_CRASH_RESET:
        RilLogE("STATE_CRASH_RESET");
        return -2;
    case STATE_CRASH_EXIT:
        RilLogE("STATE_CRASH_EXIT");
        return -3;
    case STATE_BOOTING:
        RilLogE("STATE_CRASH_BOOTING");
        break;
    case STATE_OFFLINE:
        RilLogE("STATE_CRASH_OFFLINE");
        break;
    default:
        break;
    }

    return status;
}

INT32 ModemControl::GetModemStatus(INT32 fd)
{
    INT32 status = 0;

    if ( fd < 0 )
    {
        RilLogE("cannot get modem status - due to fd is invalid(%d)", fd);
        return -1;
    }

    status = ioctl(fd, IOCTL_MODEM_STATUS);

    return status;
}


