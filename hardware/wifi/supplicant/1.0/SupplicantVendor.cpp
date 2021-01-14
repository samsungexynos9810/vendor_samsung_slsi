#include "SupplicantVendor.h"

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::samsung_slsi::hardware::wifi::supplicant::V1_0::ISupplicantVendor follow.
Return<void> SupplicantVendor::getVendorInterface(const ::android::hardware::wifi::supplicant::V1_0::ISupplicant::IfaceInfo& ifaceInfo, getVendorInterface_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<void> SupplicantVendor::listVendorInterfaces(listVendorInterfaces_cb _hidl_cb) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//ISupplicantVendor* HIDL_FETCH_ISupplicantVendor(const char* /* name */) {
    //return new SupplicantVendor();
//}
//
}  // namespace implementation
}  // namespace V1_0
}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor
