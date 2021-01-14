#ifndef HIDL_GENERATED_VENDOR_SAMSUNG_SLSI_HARDWARE_WIFI_SUPPLICANT_V1_0_BNHWSUPPLICANTVENDORNETWORK_H
#define HIDL_GENERATED_VENDOR_SAMSUNG_SLSI_HARDWARE_WIFI_SUPPLICANT_V1_0_BNHWSUPPLICANTVENDORNETWORK_H

#include <vendor/samsung_slsi/hardware/wifi/supplicant/1.0/IHwSupplicantVendorNetwork.h>

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V1_0 {

struct BnHwSupplicantVendorNetwork : public ::android::hidl::base::V1_0::BnHwBase {
    explicit BnHwSupplicantVendorNetwork(const ::android::sp<ISupplicantVendorNetwork> &_hidl_impl);
    explicit BnHwSupplicantVendorNetwork(const ::android::sp<ISupplicantVendorNetwork> &_hidl_impl, const std::string& HidlInstrumentor_package, const std::string& HidlInstrumentor_interface);

    virtual ~BnHwSupplicantVendorNetwork();

    ::android::status_t onTransact(
            uint32_t _hidl_code,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            uint32_t _hidl_flags = 0,
            TransactCallback _hidl_cb = nullptr) override;


    typedef ISupplicantVendorNetwork Pure;

    typedef android::hardware::details::bnhw_tag _hidl_tag;

    ::android::sp<ISupplicantVendorNetwork> getImpl() { return _hidl_mImpl; }

private:
    // Methods from ::android::hidl::base::V1_0::IBase follow.
    ::android::hardware::Return<void> ping();
    using getDebugInfo_cb = ::android::hidl::base::V1_0::IBase::getDebugInfo_cb;
    ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb);

    ::android::sp<ISupplicantVendorNetwork> _hidl_mImpl;
};

}  // namespace V1_0
}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_SAMSUNG_SLSI_HARDWARE_WIFI_SUPPLICANT_V1_0_BNHWSUPPLICANTVENDORNETWORK_H
