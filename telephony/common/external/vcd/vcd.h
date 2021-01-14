/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef __VCD_H__
#define __VCD_H__

#define LOG_TAG "VCD"
#include <log/log.h>
#include "utils.h"

enum {
    DEVICE_TYPE_MODEM,
    DEVICE_TYPE_CONNECTIVITY,
    DEVICE_TYPE_APP,
    DEVICE_TYPE_IMSCONNECTIVITY,
    DEVICE_TYPE_RIL,
};

enum clientSock_type {
    TYPE_NORMAL,
    TYPE_APP,
};

enum clientSock_id {
    ID_NV,
    ID_IMS,
    ID_APP,
};
typedef int (*CommandsRouteHandler)(const char *data, size_t datalen);
typedef int (*CommandResponseHandler)(int device, char *data, size_t datalen);
typedef int (*CommandRequestHandler)(int device, char *data, size_t datalen);

void PrintATCommands(const char *tag, const char *cmd, int cmdlen);

#endif // __VCD_h__
