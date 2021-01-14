/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include "rilaudioapi.h"
#include <utils/Log.h>

#if 0
#define DLOG(format, ...) printf(format "\n", ##__VA_ARGS__)
#else
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "libsitril-audio"
#endif
#define DLOG(format, ...) RLOGD(format, ##__VA_ARGS__)
#endif

#define LIB_NAME "libsitril-audio.so"

int (*pfnRilAudioOpen)() = NULL;
int (*pfnRilAudioClose)() = NULL;
int (*pfnSetTtyMode)(int) = NULL;

int main()
{
    DLOG("###test_audio_api - start###");
    void *h = NULL;
    DLOG("1111");
    h = dlopen(LIB_NAME, RTLD_NOW);
    if (h == NULL) {
        DLOG("fail to dlopen %s", LIB_NAME);
        return 0;
    }

    pfnRilAudioOpen = (int (*)())dlsym(h, "RilAudioOpen");
    if (pfnRilAudioOpen == NULL) {
        DLOG("fail to load SetTtyMode");
        return 0;
    }

    pfnRilAudioClose = (int (*)())dlsym(h, "RilAudioClose");
    if (pfnRilAudioClose == NULL) {
        DLOG("fail to load SetTtyMode");
        return 0;
    }

    pfnSetTtyMode = (int (*)(int))dlsym(h, "SetTtyMode");
    if (pfnSetTtyMode == NULL) {
        DLOG("fail to load SetTtyMode");
        return 0;
    }

    DLOG("pfnRilAudioOpen=%p", pfnSetTtyMode);
    DLOG("pfnRilAudioClose=%p", pfnSetTtyMode);
    DLOG("pfnSetTtyMode=%p", pfnSetTtyMode);

    DLOG("RilAudioOpen");
    pfnRilAudioOpen();

    for (int i = 0; i < 4; i++) {
        int ret = pfnSetTtyMode(i);
        DLOG("SetTtyMode(%d) ret=%d", i, ret);
        DLOG("Wait 3 seconds ...");
        sleep(3);
    }

    DLOG("RilAudioClose");
    pfnRilAudioClose();
    sleep(1);

    dlclose(h);
    DLOG("###test_audio_api - end###");
    return 0;
}
