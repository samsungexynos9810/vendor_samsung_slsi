#include "ExynosHWCConfigs.h"

namespace vendor {
namespace samsung_slsi {
namespace hardware {
namespace configstore {
namespace V1_0 {
namespace implementation {

// Methods from ::vendor::samsung_slsi::hardware::configstore::V1_0::IExynosHWCConfigs follow.
Return<void> ExynosHWCConfigs::useDisplayport(useDisplayport_cb _hidl_cb) {
    bool value = false;
#ifdef USE_DISPLAYPORT
    value = true;
#endif
    _hidl_cb({true, value});
    return Void();
}

Return<void> ExynosHWCConfigs::useHWCService(useHWCService_cb _hidl_cb) {
    bool value = false;
#ifdef USES_HWC_SERVICES
    value = true;
#endif
    _hidl_cb({true, value});
    return Void();
}

Return<void> ExynosHWCConfigs::useVirtualDisplay(useVirtualDisplay_cb _hidl_cb) {
    bool value = false;
#ifdef USES_VIRTUAL_DISPLAY
    value = true;
#endif
    _hidl_cb({true, value});
    return Void();
}

Return<void> ExynosHWCConfigs::useDisableCompositionTypeGLES(useDisableCompositionTypeGLES_cb _hidl_cb) {
    bool value = false;
#ifdef USES_DISABLE_COMPOSITIONTYPE_GLES
    value = true;
#endif
    _hidl_cb({true, value});
    return Void();
}

Return<void> ExynosHWCConfigs::useSecureEncoderOnly(useSecureEncoderOnly_cb _hidl_cb) {
    bool value = false;
#ifdef USES_SECURE_ENCODER_ONLY
    value = true;
#endif
    _hidl_cb({true, value});
    return Void();
}

Return<void> ExynosHWCConfigs::useContigMemoryForScratchBuf(useContigMemoryForScratchBuf_cb _hidl_cb) {
    bool value = false;
#ifdef USES_CONTIG_MEMORY_FOR_SCRATCH_BUF
    value = true;
#endif
    _hidl_cb({true, value});
    return Void();
}

Return<void> ExynosHWCConfigs::useHdrGlesConversion(useHdrGlesConversion_cb _hidl_cb) {
    bool value = false;
#ifdef USES_HDR_GLES_CONVERSION
    value = true;
#endif
    _hidl_cb({true, value});
    return Void();
}

Return<void> ExynosHWCConfigs::getGrallocVersion(getGrallocVersion_cb _hidl_cb) {
    uint32_t value = 0;
#ifdef GRALLOC_VERSION
    value = GRALLOC_VERSION;
#endif
    _hidl_cb({true, value});
    return Void();
}

Return<void> ExynosHWCConfigs::useCPUPerfMode(useCPUPerfMode_cb _hidl_cb) {
    bool value = false;
#ifdef USES_CPU_PERF_MODE
    value = true;
#endif
    _hidl_cb({true, value});
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace configstore
}  // namespace hardware
}  // namespace samsung_slsi
}  // namespace vendor
