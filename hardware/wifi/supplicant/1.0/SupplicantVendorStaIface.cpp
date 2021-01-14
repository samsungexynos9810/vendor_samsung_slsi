#include "SupplicantVendorStaIface.h"

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::samsung_slsi::hardware::wifi::supplicant::V1_0::ISupplicantVendorStaIface follow.
Return<void> SupplicantVendorStaIface::registerVendorCallback(const sp<::vendor::samsung_slsi::hardware::wifi::supplicant::V1_0::ISupplicantVendorStaIfaceCallback>& callback, registerVendorCallback_cb _hidl_cb) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//ISupplicantVendorStaIface* HIDL_FETCH_ISupplicantVendorStaIface(const char* /* name */) {
    //return new SupplicantVendorStaIface();
//}
//
}  // namespace implementation
}  // namespace V1_0
}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor
