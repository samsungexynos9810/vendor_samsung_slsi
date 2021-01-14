#pragma once

#include <vendor/samsung_slsi/hardware/exynosdisplayfeature/1.0/IExynosDisplayFeature.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace exynosdisplayfeature {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct ExynosDisplayFeature : public IExynosDisplayFeature {
    // Methods from ::vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::IExynosDisplayFeature follow.
    Return<::vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::Status> setFeature(int32_t displayId, int32_t caseId, int32_t modeId, int32_t cookie) override;
    Return<::vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::Status> setRgbGain(double r, double g, double b) override;
    Return<::vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::Status> setPanelFeature(int32_t mode, int32_t value) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" IExynosDisplayFeature* HIDL_FETCH_IExynosDisplayFeature(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace exynosdisplayfeature
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor
