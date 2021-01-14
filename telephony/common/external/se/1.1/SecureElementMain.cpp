/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#define LOG_TAG "secure_element@1.1-service"
#include <android/hardware/secure_element/1.1/ISecureElement.h>
#include <hidl/LegacySupport.h>
#include <log/log.h>

#include "SecureElement.h"

using android::hardware::secure_element::V1_1::ISecureElement;
using android::hardware::secure_element::V1_1::implementation::SecureElement;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::OK;
using android::sp;
using android::status_t;

int main() {
    ALOGD("SE HAL Service 1.1 is starting.");

    sp<ISecureElement> seService = new SecureElement();
    configureRpcThreadpool(2, true);
    status_t status = seService->registerAsService("SIM1");
    if (status != OK) {
        ALOGE("Not registerAsService for SE HAL Iface (%d).", status);
        return -1;
    }

    ALOGD("Secure Element Service is ready");
    joinRpcThreadpool();

    while (true) {
        sleep(UINT32_MAX);
    }

    return 1;
}
