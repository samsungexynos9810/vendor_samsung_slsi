#define LOG_TAG "ExynosDisplayFeature-Hal"
#include "ExynosDisplayFeature.h"
#include "ExynosDisplayUtils.h"
#include "ExynosDisplayColor.h"
#include "ExynosDisplayHDR.h"
#include "ExynosDisplayPanel.h"
#include <utils/Log.h>

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace exynosdisplayfeature {
namespace V1_0 {
namespace implementation {

static int handleFeature(int displayId, int caseId, int modeId, int cookie)
{
    int ret = 0;
    ALOGD("handleFeature (%d %d %d %d)", displayId, caseId, modeId, cookie);

    switch (caseId) {
        default :
            break;
    }

    return ret;
}

// Methods from ::vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::IExynosDisplayFeature follow.
Return<::vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::Status> ExynosDisplayFeature::setFeature(int32_t displayId, int32_t caseId, int32_t modeId, int32_t cookie) {
    // TODO implement
    ALOGD("setFeature (%d %d %d %d)", displayId, caseId, modeId, cookie);
    handleFeature(displayId, caseId, modeId, cookie);
    return ::vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::Status {};
}

Return<::vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::Status> ExynosDisplayFeature::setRgbGain(double r, double g, double b) {
    // TODO implement
    ALOGD("setRgbGain (%f %f %f)", r, g, b);
    colorSetRgbGain(r, g, b);
    return ::vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::Status {};
}

Return<::vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::Status> ExynosDisplayFeature::setPanelFeature(int32_t mode, int32_t value) {
    // TODO implement
    ALOGD("setPanelFeature (%d %d)", mode, value);
    panelHandleFeature(mode, value);
    return ::vendor::samsung_slsi::hardware::exynosdisplayfeature::V1_0::Status {};
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IExynosDisplayFeature* HIDL_FETCH_IExynosDisplayFeature(const char* /* name */) {
    return new ExynosDisplayFeature();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace exynosdisplayfeature
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor
