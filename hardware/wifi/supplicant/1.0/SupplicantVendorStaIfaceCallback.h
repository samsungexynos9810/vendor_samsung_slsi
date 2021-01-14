#ifndef VENDOR_SAMSUNG_SLSI_HARDWARE_WIFI_SUPPLICANT_V1_0_SUPPLICANTVENDORSTAIFACECALLBACK_H
#define VENDOR_SAMSUNG_SLSI_HARDWARE_WIFI_SUPPLICANT_V1_0_SUPPLICANTVENDORSTAIFACECALLBACK_H

#include <vendor/samsung_slsi/hardware/wifi/supplicant/1.0/ISupplicantVendorStaIfaceCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct SupplicantVendorStaIfaceCallback : public ISupplicantVendorStaIfaceCallback {
    // Methods from ::vendor::samsung_slsi::hardware::wifi::supplicant::V1_0::ISupplicantVendorStaIfaceCallback follow.
    Return<void> onVendorDriverHang(const hidl_string& state, const hidl_string& msg) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" ISupplicantVendorStaIfaceCallback* HIDL_FETCH_ISupplicantVendorStaIfaceCallback(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor

#endif  // VENDOR_SAMSUNG_SLSI_HARDWARE_WIFI_SUPPLICANT_V1_0_SUPPLICANTVENDORSTAIFACECALLBACK_H
