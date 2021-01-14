/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __MODEM_CONTROL_H__
#define __MODEM_CONTROL_H__

#include "types.h"
#include "rillog.h"

#define MODEM_BOOT                      "/dev/umts_boot0"
#define IOCTL_MODEM_STATUS              _IO('o', 0x27)
#define IOCTL_MODEM_FORCE_CRASH_EXIT    _IO('o', 0x34)
#define CP_RESET_INFO_SIZE              512

enum modem_state {
    STATE_OFFLINE,
    STATE_CRASH_RESET,          /* silent reset */
    STATE_CRASH_EXIT,           /* cp ramdump */
    STATE_BOOTING,
    STATE_ONLINE,
};

class ModemControl {
    public:
        ModemControl();
        virtual ~ModemControl();

        static INT32 CrashModem(char *_reason);
        static INT32 WaitingForBootDone();
        static INT32 CheckModemStatus(INT32 fd);
        static INT32 GetModemStatus(INT32 fd);
};

#endif
