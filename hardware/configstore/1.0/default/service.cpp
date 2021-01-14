/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.1
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "vendor.samsung_slsi.hardware.configstore@1.0-service"

#include <vendor/samsung_slsi/hardware/configstore/1.0/IExynosHWCConfigs.h>
#include <hidl/HidlTransportSupport.h>

#include "ExynosHWCConfigs.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using vendor::samsung_slsi::hardware::configstore::V1_0::IExynosHWCConfigs;
using vendor::samsung_slsi::hardware::configstore::V1_0::implementation::ExynosHWCConfigs;
using android::sp;
using android::status_t;
using android::OK;

int main() {
    configureRpcThreadpool(10, true);

    sp<IExynosHWCConfigs> exynosHWCConfigs = new ExynosHWCConfigs;
    status_t status = exynosHWCConfigs->registerAsService();
    LOG_ALWAYS_FATAL_IF(status != OK, "Could not register IExynosHWCConfigs");

    // other interface registration comes here
    joinRpcThreadpool();
    return 0;
}
