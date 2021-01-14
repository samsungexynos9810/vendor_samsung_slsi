/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */
#ifndef __RESET_UTIL_H__
#define __RESET_UTIL_H__

#include "types.h"
#include "rildef.h"

int RilReset(const char* pReason);
int RilErrorReset(const char* pReason);

#define CRASH_MODE_SYS_PROP    "persist.vendor.ril.crash_handling_mode"

enum crash_handling_mode {
    CRASH_MODE_DUMP_PANIC = 0, /* kernel panic after dump */
    CRASH_MODE_DUMP_SILENT_RESET = 1, /* silent reset after dump */
    CRASH_MODE_SILENT_RESET = 2, /* only silent reset */
    CRASH_MODE_MAX,
    CRASH_MODE_DEFAULT = CRASH_MODE_DUMP_PANIC,
};

#endif /* __RESET_UTIL_H__ */
