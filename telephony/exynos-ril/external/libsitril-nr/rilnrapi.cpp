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
 * rilnrapi.cpp
 */
#include "base.h"
#include "rilclienthelper.h"
#include <unistd.h>

class RilNrApiImpl {
    DECLARE_MODULE_TAG()
private:
    RilClientHelper *mClientHelper;
    HANDLE mClient;
public:
    RilNrApiImpl();
    ~RilNrApiImpl();

public:
    int enableNr(bool enable);

public:
    static RilNrApiImpl instance;
    static RilNrApiImpl *GetInstance();
};
RilNrApiImpl RilNrApiImpl::instance;

IMPLEMENT_MODULE_TAG(RilNrApiImpl, RilNrApiImpl)

// RilNrApiImpl impl
RilNrApiImpl::RilNrApiImpl() : mClientHelper(NULL), mClient(NULL)
{
    mClientHelper = RilClientHelper::GetInstance();
    if (mClientHelper != NULL) {
        mClient = mClientHelper->Open();
    }
}

RilNrApiImpl::~RilNrApiImpl()
{
    if (mClientHelper != NULL) {
        mClientHelper->Close(mClient);
        usleep(100000);
        delete mClientHelper;
    }
}

int RilNrApiImpl::enableNr(bool enable)
{
    if (mClient == NULL) {
        RLOGE("libsitril-client not opened");
        return -1;
    }

    // change mode both stacks
    int mode = enable ? 1 : 0;
    // RILC_REQ_SET_ENDC_MODE(27)
    // mode(int)
    for (int i = 0; i < 2; i++) {
        int ret = mClientHelper->Send(mClient, RILC_REQ_SET_ENDC_MODE, &mode, sizeof(int), NULL, i);
        if (ret != RILC_STATUS_SUCCESS) {
            RLOGE("[%d] RILC_REQ_SET_ENDC_MODE error. mode=%d", i, mode);
        }
    } // end for i ~

    return 0;
}

RilNrApiImpl *RilNrApiImpl::GetInstance()
{
    return &instance;
}

// API exported
RIL_API_EXPORT int enableNr(int enable)
{
    return RilNrApiImpl::GetInstance()->enableNr(enable);
}
