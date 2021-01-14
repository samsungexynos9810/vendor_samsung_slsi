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
 * oem_internal.h
 *
 *  Created on: 2018. 5. 11.
 */

#ifndef __OEM_INTERNAL_H__
#define __OEM_INTERNAL_H__

#define MAX_SERVICE_COUNT   4
#define MAX_SERVICE_NAMX    128

// Enable verbose logging
#define VDBG 0

#define LOG_TAG "liboemservice"
#include <utils/Log.h>

#define dlog(x...) ALOGD( x )

#endif // __OEM_INTERNAL_H__
