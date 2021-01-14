#include "SupplicantVendorStaIfaceCallback.h"

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::samsung_slsi::hardware::wifi::supplicant::V1_0::ISupplicantVendorStaIfaceCallback follow.
Return<void> SupplicantVendorStaIfaceCallback::onVendorDriverHang(const hidl_string& state, const hidl_string& msg) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//ISupplicantVendorStaIfaceCallback* HIDL_FETCH_ISupplicantVendorStaIfaceCallback(const char* /* name */) {
    //return new SupplicantVendorStaIfaceCallback();
//}
//
}  // namespace implementation
}  // namespace V1_0
}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor
