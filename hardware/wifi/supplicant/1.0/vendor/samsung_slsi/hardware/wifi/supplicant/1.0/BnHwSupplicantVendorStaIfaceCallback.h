#ifndef HIDL_GENERATED_VENDOR_SAMSUNG_SLSI_HARDWARE_WIFI_SUPPLICANT_V1_0_BNHWSUPPLICANTVENDORSTAIFACECALLBACK_H
#define HIDL_GENERATED_VENDOR_SAMSUNG_SLSI_HARDWARE_WIFI_SUPPLICANT_V1_0_BNHWSUPPLICANTVENDORSTAIFACECALLBACK_H

#include <vendor/samsung_slsi/hardware/wifi/supplicant/1.0/IHwSupplicantVendorStaIfaceCallback.h>

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V1_0 {

struct BnHwSupplicantVendorStaIfaceCallback : public ::android::hidl::base::V1_0::BnHwBase {
    explicit BnHwSupplicantVendorStaIfaceCallback(const ::android::sp<ISupplicantVendorStaIfaceCallback> &_hidl_impl);
    explicit BnHwSupplicantVendorStaIfaceCallback(const ::android::sp<ISupplicantVendorStaIfaceCallback> &_hidl_impl, const std::string& HidlInstrumentor_package, const std::string& HidlInstrumentor_interface);

    virtual ~BnHwSupplicantVendorStaIfaceCallback();

    ::android::status_t onTransact(
            uint32_t _hidl_code,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            uint32_t _hidl_flags = 0,
            TransactCallback _hidl_cb = nullptr) override;


    typedef ISupplicantVendorStaIfaceCallback Pure;

    typedef android::hardware::details::bnhw_tag _hidl_tag;

    ::android::sp<ISupplicantVendorStaIfaceCallback> getImpl() { return _hidl_mImpl; }
    // Methods from ::vendor::samsung_slsi::hardware::wifi::supplicant::V1_0::ISupplicantVendorStaIfaceCallback follow.
    static ::android::status_t _hidl_onVendorDriverHang(
            ::android::hidl::base::V1_0::BnHwBase* _hidl_this,
            const ::android::hardware::Parcel &_hidl_data,
            ::android::hardware::Parcel *_hidl_reply,
            TransactCallback _hidl_cb);



private:
    // Methods from ::vendor::samsung_slsi::hardware::wifi::supplicant::V1_0::ISupplicantVendorStaIfaceCallback follow.

    // Methods from ::android::hidl::base::V1_0::IBase follow.
    ::android::hardware::Return<void> ping();
    using getDebugInfo_cb = ::android::hidl::base::V1_0::IBase::getDebugInfo_cb;
    ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb);

    ::android::sp<ISupplicantVendorStaIfaceCallback> _hidl_mImpl;
};

}  // namespace V1_0
}  // namespace supplicant
}  // namespace wifi
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_SAMSUNG_SLSI_HARDWARE_WIFI_SUPPLICANT_V1_0_BNHWSUPPLICANTVENDORSTAIFACECALLBACK_H
