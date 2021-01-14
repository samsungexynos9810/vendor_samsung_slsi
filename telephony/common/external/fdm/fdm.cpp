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
 * fdm.cpp
 *
 *  Created on: 2018. 7. 3.
 */

#include <vendor/samsung_slsi/telephony/hardware/oemservice/1.0/IOemService.h>
#include <vendor/samsung_slsi/telephony/hardware/oemservice/1.0/types.h>
#include <stdio.h>
#include <stdlib.h>

using namespace vendor::samsung_slsi::telephony::hardware::oemservice::V1_0;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using android::sp;

#define LOG_TAG "FactoryDM"
#include <utils/Log.h>

#ifdef __FACTORY_DM_SLIB__
#define dlog(x...) ALOGD( x )
#else
#define dlog(x...) do { printf( x ); printf("\n"); } while(0)
#endif

#define SERVICE_NAME   "dm1"

extern char DEFAULT_PROFILE[];
extern unsigned int DEFAULT_PROFILE_SIZE;

enum {
    TYPE_COMMAND,
    TYPE_RAW,
    TYPE_FACTORY_COMMAND,
};

enum {
    MODE_NONE,
    MODE_EXTERNAL_DM,
    MODE_SILENT_LOGGING,
    MODE_ON_BOARD_DM,
};

enum {
    COMMAND_SET_DM_MODE,
    COMMAND_SEND_PROFILE,
    COMMAND_STOP_DM,
    COMMAND_SAVE_SNAPSHOT,
    FACTORY_COMMAND_SET_DM_MODE = 100,
};

static sp<IOemService> getServiceProxy() {
    sp<IOemService> serviceProxy = IOemService::getService(SERVICE_NAME);
    return serviceProxy;
}

static int setFactoryDMMode(int mode) {
    dlog("%s %d", __FUNCTION__, mode);
    sp<IOemService> serviceProxy = getServiceProxy();
    if (serviceProxy == NULL) {
        dlog("failed to get service proxy");
        return -1;
    }

    if (!(mode == MODE_EXTERNAL_DM || mode ==MODE_SILENT_LOGGING)) {
        dlog("unsupported Factory DM mode(%d)", mode);
        return -1;
    }

    char *buf = NULL;
    size_t datalen = sizeof(int);
    if (mode == MODE_SILENT_LOGGING) {
        datalen += DEFAULT_PROFILE_SIZE;
    }
    buf = new char[datalen];
    if (buf == NULL) {
        dlog("memory allocation error");
        return -1;
    }

    memcpy(buf, &mode, sizeof(int));
    memcpy(buf + sizeof(int), DEFAULT_PROFILE, DEFAULT_PROFILE_SIZE);

    hidl_vec<uint8_t> data;
    data.setToExternal((uint8_t *)buf, datalen);
    serviceProxy->sendRequestRaw(TYPE_FACTORY_COMMAND, FACTORY_COMMAND_SET_DM_MODE, data);
    sleep(1);

    delete[] buf;

    return 0;
}

#ifdef __FACTORY_DM_SLIB__
#ifdef __cplusplus
extern "C" {
#endif

int FDM_setMode(int mode) {
    return setFactoryDMMode(mode);
}

#ifdef __cplusplus
}
#endif

#else
int main(int argc, char **argv)
{
    if (argc == 2) {
        dlog("argv[1]=%s", argv[1]);
        int mode = atoi(((char **)argv)[1]);
        setFactoryDMMode(mode);
    }
    else {
        dlog("argc=%d", argc);
    }

    return 0;
}

#endif // __FACTORY_DM_SLIB__
