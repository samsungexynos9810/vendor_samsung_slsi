//
// Copyright (C) 2017 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#define LOG_TAG "ExynosConfigStore"

#include <android-base/logging.h>
#include <exynos_configstore/exynosUtils.h>

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace details {

bool wouldLogInfo() {
    return WOULD_LOG(INFO);
}

void logAlwaysInfo(const std::string& message) {
    LOG(INFO) << message;
}

void logAlwaysError(const std::string& message) {
    LOG(ERROR) << message;
}

}  // namespace details
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor
