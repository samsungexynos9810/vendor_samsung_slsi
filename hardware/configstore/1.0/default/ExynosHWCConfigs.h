#ifndef VENDOR_SAMSUNG_SLSI_HARDWARE_CONFIGSTORE_V1_0_EXYNOSHWCCONFIGS_H
#define VENDOR_SAMSUNG_SLSI_HARDWARE_CONFIGSTORE_V1_0_EXYNOSHWCCONFIGS_H

#include <vendor/samsung_slsi/hardware/configstore/1.0/IExynosHWCConfigs.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace configstore {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct ExynosHWCConfigs : public IExynosHWCConfigs {
    // Methods from ::vendor::samsung_slsi::hardware::configstore::V1_0::IExynosHWCConfigs follow.
    Return<void> useDisplayport(useDisplayport_cb _hidl_cb) override;
    Return<void> useHWCService(useHWCService_cb _hidl_cb) override;
    Return<void> useVirtualDisplay(useVirtualDisplay_cb _hidl_cb) override;
    Return<void> useDisableCompositionTypeGLES(useDisableCompositionTypeGLES_cb _hidl_cb) override;
    Return<void> useSecureEncoderOnly(useSecureEncoderOnly_cb _hidl_cb) override;
    Return<void> useContigMemoryForScratchBuf(useContigMemoryForScratchBuf_cb _hidl_cb) override;
    Return<void> useHdrGlesConversion(useHdrGlesConversion_cb _hidl_cb) override;
    Return<void> getGrallocVersion(getGrallocVersion_cb _hidl_cb) override;
    Return<void> useCPUPerfMode(useCPUPerfMode_cb _hidl_cb) override;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace configstore
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor

#endif  // VENDOR_SAMSUNG_SLSI_HARDWARE_CONFIGSTORE_V1_0_EXYNOSHWCCONFIGS_H
