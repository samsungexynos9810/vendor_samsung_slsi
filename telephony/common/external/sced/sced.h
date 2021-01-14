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
 * sced.h
 *
 * Created on: 2018. 05. 23
 */
#define LOG_TAG "SCED"
#include <utils/Log.h>

#ifndef __SCED_H__
#define __SCED_H__

enum {
    COMMAND_LOGCAT = 0,
    COMMAND_LOGCAT_SNAPSHOT = 1,
    COMMAND_TCP_DUMP = 2,
    COMMAND_TCP_DUMP_SNAPSHOT = 3,
    COMMAND_KILL = 4,
};

#endif /* __SCED_H__ */
