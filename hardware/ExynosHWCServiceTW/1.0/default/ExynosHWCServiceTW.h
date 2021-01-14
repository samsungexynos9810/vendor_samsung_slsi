#ifndef VENDOR_SAMSUNG_SLSI_HARDWARE_EXYNOSHWCSERVICETW_V1_0_EXYNOSHWCSERVICETW_H
#define VENDOR_SAMSUNG_SLSI_HARDWARE_EXYNOSHWCSERVICETW_V1_0_EXYNOSHWCSERVICETW_H

#include <vendor/samsung_slsi/hardware/ExynosHWCServiceTW/1.0/IExynosHWCServiceTW.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include "ExynosHWCService.h"
#include <binder/IServiceManager.h>

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace ExynosHWCServiceTW {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::samsung_slsi::hardware::ExynosHWCServiceTW::V1_0::IExynosHWCServiceTW;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct ExynosHWCServiceTW : public IExynosHWCServiceTW {
    // Methods from ::vendor::samsung_slsi::hardware::ExynosHWCServiceTW::V1_0::IExynosHWCServiceTW follow.

    ExynosHWCServiceTW();

    Return<int32_t> setWFDMode(uint32_t mode) override;
    Return<int32_t> getWFDMode() override;
    Return<void> getWFDInfo(getWFDInfo_cb _hidl_cb) override;
    Return<int32_t> sendWFDCommand(int32_t cmd, int32_t ext1, int32_t ext2) override;
    Return<void> setBootFinished() override;
    Return<void> enableMPP(uint32_t physicalType, uint32_t physicalIndex, uint32_t logcialIndex, uint32_t enable) override;
    Return<void> setHWCDebug(int32_t debug) override;
    Return<int32_t> setHWCCtl(uint32_t display, uint32_t ctrl, int32_t val) override;
    Return<int32_t> setWFDOutputResolution(uint32_t width, uint32_t height) override;
    Return<int32_t> setVDSGlesFormat(int32_t format) override;
    Return<uint32_t> getHWCDebug() override;
    Return<int32_t> setDDIScaler(uint32_t width, uint32_t height) override;
    Return<int32_t> setSecureVDSMode(uint32_t mode) override;
    Return<int32_t> setExternalVsyncEnabled(uint32_t index) override;
    Return<void> setPresentationMode(uint32_t multiple_layerStack) override;
    Return<int32_t> getExternalHdrCapabilities() override;
    Return<void> getCPUPerfInfo(int display, int config, getCPUPerfInfo_cb _hidl_cb) override;
    Return<bool> isNeedCompressedTargetBuffer(uint64_t displayId) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
    private:
    sp<android::IExynosHWCService> mHwcService = NULL;
    sp<android::IExynosHWCService> getHwcService();

};

extern "C" IExynosHWCServiceTW* HIDL_FETCH_IExynosHWCServiceTW(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace ExynosHWCServiceTW
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor

#endif  // VENDOR_SAMSUNG_SLSI_HARDWARE_EXYNOSHWCSERVICETW_V1_0_EXYNOSHWCSERVICETW_H
