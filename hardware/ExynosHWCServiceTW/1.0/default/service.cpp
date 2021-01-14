/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "vendor.samsung_slsi.hardware.ExynosHWCServiceTW@1.0-service"

#include <android/log.h>
#include <hidl/HidlTransportSupport.h>
#include <binder/ProcessState.h>

#include "ExynosHWCServiceTW.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::sp;

using vendor::samsung_slsi::hardware::ExynosHWCServiceTW::V1_0::IExynosHWCServiceTW;
using vendor::samsung_slsi::hardware::ExynosHWCServiceTW::V1_0::implementation::ExynosHWCServiceTW;

int main() {
    ALOGD("ExynosHWC HIDL start");
    android::ProcessState::initWithDriver("/dev/vndbinder");

    android::sp<IExynosHWCServiceTW> service = new ExynosHWCServiceTW();
    configureRpcThreadpool(1, true /*callerWillJoin*/);
    status_t err = service->registerAsService();

    if (err != OK) {
        ALOGE("Could not register IExynosHWCServiceTW service.");
    }

    joinRpcThreadpool();
}
