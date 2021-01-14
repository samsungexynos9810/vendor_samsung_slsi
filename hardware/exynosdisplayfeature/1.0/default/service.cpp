#define LOG_TAG "vendor.exynosdisplayfeature@1.0-service"

#include <vendor/samsung_slsi/hardware/exynosdisplayfeature/1.0/IExynosDisplayFeature.h>
#include <android/log.h>
#include <binder/ProcessState.h>
#include <hidl/LegacySupport.h>
#include "ExynosDisplayFeature.h"

using vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::IExynosDisplayFeature;
using android::hardware::defaultPassthroughServiceImplementation;

int main() {
    // the conventional HAL might start binder services
    ALOGD("exynosdisplayfeature HIDL start\n");
    android::ProcessState::initWithDriver("/dev/vndbinder");
    android::ProcessState::self()->setThreadPoolMaxThreadCount(4);
    android::ProcessState::self()->startThreadPool();
    return defaultPassthroughServiceImplementation<IExynosDisplayFeature>();
}


