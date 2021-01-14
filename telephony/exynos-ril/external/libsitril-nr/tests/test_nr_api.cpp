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
#include "rilnrapi.h"
#include <utils/Log.h>

#if 0
#define DLOG(format, ...) printf(format "\n", ##__VA_ARGS__)
#else
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "libsitril-nr"
#endif
#define DLOG(format, ...) RLOGD(format, ##__VA_ARGS__)
#endif

#define LIB_NAME "libsitril-nr.so"

static int test_vec[] = {0,1,0,1};
static void do_test(size_t trial)
{
    void *h = NULL;
    DLOG("##### %s trial=%zu #####", __FUNCTION__, trial);
    h = dlopen(LIB_NAME, RTLD_NOW);
    if (h == NULL) {
        DLOG("fail to dlopen %s", LIB_NAME);
    }

    int (*pfnEnableNr)(int) = NULL;
    pfnEnableNr = (int (*)(int))dlsym(h, "enableNr");
    if (pfnEnableNr == NULL) {
        DLOG("fail to load enableNr");
        return ;
    }

    int status = test_vec[trial];
    DLOG("[%zu] set status to %d", trial, status);

    DLOG("pfnEnableNr=%p", pfnEnableNr);
    int ret = pfnEnableNr(status);
    DLOG("enableNr(true) ret=%d", ret);
    sleep(3);
    dlclose(h);
    DLOG("##### %s end #####", __FUNCTION__);
}

int main()
{
    DLOG("##### test_nr_api - start #####");
    size_t vec_size = sizeof(test_vec) / sizeof(test_vec[0]);
    for (size_t i = 0; i < vec_size; i++) {
        do_test(i);
    } // end for i ~

    DLOG("##### test_nr_api - end #####");
    return 0;
}
