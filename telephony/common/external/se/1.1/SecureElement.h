/*
 * Copyright Samsung Electronics Co., LTD.
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

#ifndef ANDROID_HARDWARE_SECURE_ELEMENT_V1_1_SECUREELEMENT_H
#define ANDROID_HARDWARE_SECURE_ELEMENT_V1_1_SECUREELEMENT_H

#include <android/hardware/secure_element/1.0/types.h>
#include <android/hardware/secure_element/1.1/ISecureElement.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include "Types.h"
#include "UiccTerminal.h"

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_1 {
namespace implementation {

using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::secure_element::V1_0::SecureElementStatus;
using ::android::hardware::secure_element::V1_0::LogicalChannelResponse;
using ::android::hardware::secure_element::V1_1::ISecureElementHalCallback;
using ::android::hardware::Void;
using ::android::sp;

struct SecureElement : public ISecureElement, public hidl_death_recipient {
    SecureElement();
    ~SecureElement();
    Return<void> init(
        const sp<V1_0::ISecureElementHalCallback>& clientCallback) override;
    Return<void> init_1_1(
        const sp<V1_1::ISecureElementHalCallback>& clientCallback) override;
    Return<void> getAtr(getAtr_cb _hidl_cb) override;
    Return<bool> isCardPresent() override;
    Return<void> transmit(const hidl_vec<uint8_t>& data,
                        transmit_cb _hidl_cb) override;
    Return<void> openLogicalChannel(const hidl_vec<uint8_t>& aid, uint8_t p2,
                                  openLogicalChannel_cb _hidl_cb) override;
    Return<void> openBasicChannel(const hidl_vec<uint8_t>& aid, uint8_t p2,
                                openBasicChannel_cb _hidl_cb) override;
    Return<SecureElementStatus> closeChannel(uint8_t channelNumber) override;
    void serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) override;
    const char *statusToString(SecureElementStatus status);

private:
    uint8_t mOpenedchannelCount = 0;
    bool mOpenedChannels[MAX_CHANNEL_NUM];
    uint8_t *mApduRequestBuffer = NULL;
    uint8_t *mApduResponseBuffer = NULL;
    UiccTerminal* mUiccTerminal;
    uint64_t mExternalProxyCookie;
    static sp<V1_0::ISecureElementHalCallback> mCallbackV1_0;
    static sp<V1_1::ISecureElementHalCallback> mCallbackV1_1;
    bool mIsServiceDied;
    pthread_mutex_t mMutex;
};

}  // namespace implementation
}  // namespace V1_1
}  // namespace secure_element
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_SECURE_ELEMENT_V1_1_SECUREELEMENT_H
