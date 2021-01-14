#include "ExynosHWCServiceTW.h"
#include "ExynosHWCService.h"
#include <log/log.h>
#include <binder/IServiceManager.h>

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace ExynosHWCServiceTW {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::samsung_slsi::hardware::ExynosHWCServiceTW::V1_0::IExynosHWCServiceTW follow.

ExynosHWCServiceTW::ExynosHWCServiceTW() {
    sp<android::IServiceManager> sm = android::defaultServiceManager();
    if (sm != NULL) {
        if (sm->checkService(android::String16("Exynos.HWCService"))) {
            mHwcService = android::interface_cast<android::IExynosHWCService>
                (sm->getService(android::String16("Exynos.HWCService")));
        } else {
            mHwcService = NULL;
        }

        if (mHwcService == NULL)
            ALOGE("Can not get ExynosHWCService");
    }
}

sp<android::IExynosHWCService> ExynosHWCServiceTW::getHwcService() {
    if (mHwcService != NULL)
        return mHwcService;
    else {
        sp<android::IServiceManager> sm = android::defaultServiceManager();
        if (sm != NULL) {
            mHwcService = android::interface_cast<android::IExynosHWCService>
                (sm->getService(android::String16("Exynos.HWCService")));

            return mHwcService;
        }
    }
    return NULL;
}

Return<int32_t> ExynosHWCServiceTW::setWFDOutputResolution(uint32_t width, uint32_t height) {
    int32_t ret = -1;
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = (int32_t)(mHwcService->setWFDOutputResolution(width, height));
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
    return ret;
}

Return<int32_t> ExynosHWCServiceTW::setVDSGlesFormat(int32_t format) {
    int32_t ret = -1;
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = (int32_t)(mHwcService->setVDSGlesFormat(format));
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
    return ret;
}

Return<int32_t> ExynosHWCServiceTW::setWFDMode(uint32_t mode) {
    int32_t ret = -1;
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = (int32_t)(mHwcService->setWFDMode(mode));
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
    return ret;
}

Return<int32_t> ExynosHWCServiceTW::getWFDMode() {
    int32_t ret = -1;
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = (int32_t)(mHwcService->getWFDMode());
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
    return ret;
}

Return<void> ExynosHWCServiceTW::getWFDInfo(getWFDInfo_cb _hidl_cb) {
    int32_t ret = -1;
    WFDInfo info;

    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = (int32_t)(mHwcService->getWFDInfo(&info.state, &info.compositionType, &info.format, &info.usage, &info.width, &info.height));
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
    _hidl_cb(ret, info);
    return Void();
}

Return<int32_t> ExynosHWCServiceTW::sendWFDCommand(int32_t cmd, int32_t ext1, int32_t ext2) {
    int32_t ret = -1;
#ifdef SUPPORT_WFD_COMMAND
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = (int32_t)(mHwcService->sendWFDCommand(cmd, ext1, ext2));
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
#endif
    return ret;
}

Return<void> ExynosHWCServiceTW::setBootFinished() {
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        mHwcService->setBootFinished();
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);

    return Void();
}

Return<void> ExynosHWCServiceTW::setHWCDebug(int32_t debug) {
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        mHwcService->setHWCDebug(debug);
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);

    return Void();
}

Return<int32_t> ExynosHWCServiceTW::setHWCCtl(uint32_t display, uint32_t ctrl, int32_t val) {
    int32_t ret = 0;
    if (mHwcService == NULL)
        getHwcService();

#ifdef USE_LIBHWC
    if (mHwcService != NULL)
        mHwcService->setHWCCtl(ctrl, val);
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
#else
    if (mHwcService != NULL)
        ret = mHwcService->setHWCCtl(display, ctrl, val);
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
#endif
    return ret;
}

Return<uint32_t> ExynosHWCServiceTW::getHWCDebug() {
    uint32_t ret = 0;

#ifndef USE_LIBHWC
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = mHwcService->getHWCDebug();
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
#endif
    return ret;
}

Return<int32_t> ExynosHWCServiceTW::setSecureVDSMode(uint32_t mode) {
    int32_t ret = -1;

#ifndef USE_LIBHWC
    if (mHwcService != NULL)
        ret = (int32_t)(mHwcService->setSecureVDSMode(mode));
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
#endif
    return ret;
}

Return<void> ExynosHWCServiceTW::enableMPP(uint32_t physicalType, uint32_t physicalIndex, uint32_t logicalIndex, uint32_t enable) {
#ifndef USE_LIBHWC
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        mHwcService->enableMPP(physicalType, physicalIndex, logicalIndex, enable);
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
#endif
    return Void();
}

Return<int32_t> ExynosHWCServiceTW::setExternalVsyncEnabled(uint32_t index) {
    int32_t ret = -1;

#ifndef USE_LIBHWC
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = (int32_t)(mHwcService->setExternalVsyncEnabled(index));
   else
       ALOGE("%s :: Can not get ExynosHWCService", __func__);
#endif
    return ret;
}

Return<int32_t> ExynosHWCServiceTW::setDDIScaler(uint32_t __unused width, uint32_t __unused height) {
    int32_t ret = 0;

#ifdef ENABLE_DDI_SCALER
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = mHwcService->setDDIScaler(width, height);
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
#endif
    return ret;
}

Return<void> ExynosHWCServiceTW::setPresentationMode(uint32_t multiple_layerStack) {
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        mHwcService->setPresentationMode(multiple_layerStack);
    else
       ALOGE("%s :: Can not get ExynosHWCService", __func__);

    return Void();
}

Return<int32_t> ExynosHWCServiceTW::getExternalHdrCapabilities() {
    int32_t ret = -1;
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = mHwcService->getExternalHdrCapabilities();
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);

    return ret;
}

Return<void> ExynosHWCServiceTW::getCPUPerfInfo(int __unused display, int __unused config, getCPUPerfInfo_cb _hidl_cb) {
    int32_t ret = -1;
    CPUPerfInfo info;

#ifdef USE_CPU_PERF_MODE
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = (int32_t)mHwcService->getCPUPerfInfo(display, config, &info.cpuIDs, &info.min_clock);
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);

    _hidl_cb(ret, info);
#else
    info.cpuIDs = 0;
    info.min_clock = 0;
    _hidl_cb(ret, info);
#endif
    return Void();
}

Return<bool> ExynosHWCServiceTW::isNeedCompressedTargetBuffer(uint64_t displayId) {
    //default is true
    bool ret = true;
#ifdef USE_LIBHWC21
    if (mHwcService == NULL)
        getHwcService();

    if (mHwcService != NULL)
        ret = mHwcService->isNeedCompressedTargetBuffer(displayId);
    else
        ALOGE("%s :: Can not get ExynosHWCService", __func__);
#endif
    return ret;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

IExynosHWCServiceTW* HIDL_FETCH_IExynosHWCServiceTW(const char* /* name */) {
    return new ExynosHWCServiceTW();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace ExynosHWCServiceTW
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor
