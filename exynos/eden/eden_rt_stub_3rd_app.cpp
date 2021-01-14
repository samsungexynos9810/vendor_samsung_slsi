/*
 * Copyright 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file eden_rt_stub.c
 * @brief implementation of eden_rt_stub api
 */

#include "EdenRuntime.h"
#include "log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EDEN_RT_STUB"

#include <cutils/native_handle.h>
#include <utils/misc.h>
#include <utils/Log.h>
#include <hidl/LegacySupport.h>
#include <hidl/Status.h>

#include <stdio.h>
#include <iostream>
#include <chrono>
#include <unordered_map>        // unordered_map
#include <sys/mman.h>           // mmap
#include <errno.h>              // errno

#include <sys/types.h>          // pid_t
#include <unistd.h>             // getpid
#include <stdlib.h>             // atexit
#include <shared_mutex>

#include <vendor/samsung_slsi/hardware/eden_runtime/1.0/IEdenruntime.h>

#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <hidlmemory/mapping.h>

#define CLRHIGH(u64)    ((u64) & 0x00000000FFFFFFFF)

namespace eden {
namespace rt {

typedef struct __EmaBuffer {
    uint32_t fd;
    void *addr;
    int32_t size;
    int32_t numOfBuffers;
} EmaBuffer;

using ::android::hardware::hidl_handle;

using ::android::hardware::hidl_memory;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::hidl::allocator::V1_0::IAllocator;

using ::android::hardware::hidl_string;
using ::android::hardware::Void;

using namespace std::chrono_literals;
using namespace vendor::samsung_slsi::hardware::eden_runtime;
using namespace vendor::samsung_slsi::hardware::eden_runtime::V1_0;
using android::sp;

using ::vendor::samsung_slsi::hardware::eden_runtime::V1_0::IEdenruntime;
using ::android::hidl::base::V1_0::IBase;

class Callback;
struct StubDeathRecipient;

sp<Callback> eCallback;
sp<IEdenruntime> service;
sp<StubDeathRecipient> stubDeathRecipient;
std::shared_timed_mutex mutex_service;

//          <client buf addr, <server buf addr, gtest EdenBuffer addr>>
std::unordered_map<uint64_t, std::pair<uint64_t, EdenBuffer **>> mMapClientbufServerbuf;

class Callback : public IEdenruntimeCallback {
    public:
    Callback(NotifyFuncPtr nfp) {
        LOGD(EDEN_RT_STUB, "Callback constructor is called!!\n");
        notify = nfp;
    }

    virtual ~Callback() {
        LOGD(EDEN_RT_STUB, "Callback desstructor is called!!\n");
        // Get current process id
        pid_t stub_pid = getpid();
        // Print out pid
        LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);
    }

    android::hardware::Return<uint32_t> executeCallback(uint64_t cbAddr,
                                                        uint64_t value,
                                                        const hidl_handle& hd) override {
        LOGI(EDEN_RT_STUB, "(+)\n");
        if (!value) {
            LOGD(EDEN_RT_STUB, "%s: value is null\n", __func__);
            return RT_FAILED;
        }

        EdenRequest* req = (EdenRequest*)value;
        auto it = mMapClientbufServerbuf.find((uint64_t)req->outputBuffers);
        if (it == mMapClientbufServerbuf.end()) {
            ALOGE("failed to find ServerOutputBuffers according to buffers");
            return RT_FAILED;
        }

        const native_handle_t* handle = hd.getNativeHandle();
        int num = handle->numFds;
        EdenBuffer** buffers = it->second.second;

        for (int idx = 0; idx < num; idx++) {
            EdenBuffer* buffer = &((*buffers)[idx]);
            uint32_t fd = handle->data[idx];
            LOGD(EDEN_RT_STUB, "callback unmap before size: %d", buffer->size);
            munmap((void *)buffer->addr, buffer->size);
            buffer->size = handle->data[num + idx];
            buffer->addr = mmap(NULL, buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            LOGD(EDEN_RT_STUB, "callback fd: %d, size: %d\n", fd, buffer->size);
            LOGD(EDEN_RT_STUB, "callback mmap addr: %p\n", buffer->addr);
        }

        notify((addr_t*)cbAddr, value);
        LOGI(EDEN_RT_STUB, "(-)\n");
        return RT_SUCCESS;
    }

    android::hardware::Return<uint32_t> isAlive() {
        return RT_SUCCESS;
    }

    void setNotifyFunc(NotifyFuncPtr nfp) {
        LOGD(EDEN_RT_STUB, "setNotifyFunc is called!!\n");
        notify = nfp;
    }

    private:
        NotifyFuncPtr notify;
};

void ClearGlobalValue(void) {
    service = nullptr;
    eCallback = nullptr;
    stubDeathRecipient = nullptr;
}

struct StubDeathRecipient : public android::hardware::hidl_death_recipient {
    StubDeathRecipient() = default;
    virtual void serviceDied(uint64_t /*cookie*/, const android::wp<IBase>& who) {
        if (service != nullptr && android::hardware::interfacesEqual(service, who.promote())) {
            // Get current process id
            pid_t stub_pid = getpid();
            // Print out pid
            LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

            std::lock_guard<std::shared_timed_mutex> wlock(mutex_service);

            LOGE(EDEN_RT_STUB, "eden runtime service died");
            ClearGlobalValue();

            uint32_t count = 0;
            while (service == nullptr) {
                service = IEdenruntime::getService();
                count++;
                if (count > 0xffff) {
                    LOGE(EDEN_RT_STUB, "time-out to get a new eden service after death");
                    break;
                }
            }
            eCallback = new Callback(nullptr);

            // android::hardware::Return<uint32_t> ret;
            auto ret = service->init(stub_pid, eCallback);
            if (!ret.isOk()) {
                LOGE(EDEN_RT_STUB, "Error!! ret.isOk() is false (-)\n");
                ClearGlobalValue();
                return ;
            } else {
                uint32_t result = static_cast<uint32_t>(ret);
                if (!result) {
                    LOGD(EDEN_RT_STUB, "service->init success\n");
                } else {
                    LOGE(EDEN_RT_STUB, "service->init: %d\n", result);
                    ClearGlobalValue();
                    return ;
                }
            }

            stubDeathRecipient = new StubDeathRecipient();
            bool linkRet = service->linkToDeath(stubDeathRecipient, 0);
            if (!linkRet) {
                LOGE(EDEN_RT_STUB, "Could not link to death\n");
            }

            LOGI(EDEN_RT_STUB, "success to get a new eden service after death");
        } else {
            LOGE(EDEN_RT_STUB, "unknown service died");
        }
    }
};

/**
 *  @brief Init EDEN Runtime
 *  @details This API function initializes the CPU/GPU/NPU handler.
 *  @param void
 *  @returns return code
 */
RtRet Init(void) {
    LOGI(EDEN_RT_STUB, "(+)\n");

    // Get current process id
    pid_t stub_pid = getpid();
    // Print out pid
    LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

    {
        std::lock_guard<std::shared_timed_mutex> wlock(mutex_service);

        if (service != nullptr) {
            LOGI(EDEN_RT_STUB, "(-) already getService\n");
            return RT_SUCCESS;
        } else {
            service = IEdenruntime::getService();
            if (service == nullptr) {
                LOGE(EDEN_RT_STUB, "service is nullptr\n");
                return RT_SERVICE_NOT_AVAILABLE;
            }
        }

        eCallback = new Callback(nullptr);

        // android::hardware::Return<uint32_t> ret;
        auto ret = service->init(stub_pid, eCallback);
        if (!ret.isOk()) {
            LOGE(EDEN_RT_STUB, "Error!! ret.isOk() is false (-)\n");
            ClearGlobalValue();
            return RT_FAILED;
        } else {
            uint32_t result = static_cast<uint32_t>(ret);
            if (!result) {
                LOGD(EDEN_RT_STUB, "service->init success\n");
            } else {
                LOGE(EDEN_RT_STUB, "service->init: %d\n", result);
                ClearGlobalValue();
                return RT_INIT_FAILED;
            }
        }

        stubDeathRecipient = new StubDeathRecipient();
        bool linkRet = service->linkToDeath(stubDeathRecipient, 0);
        if (!linkRet) {
            LOGE(EDEN_RT_STUB, "Could not link to death\n");
        }
    }

    LOGI(EDEN_RT_STUB, "(-)\n");
    return RT_SUCCESS;
}

/**
 *  @brief Open a model file and generates an in-memory model structure
 *  @details This function reads a model file and construct an in-memory model structure.
 *           The model file should be one of the supported model file format.
 *           Once it successes to parse a given model file,
 *           unique model id is returned via modelId.
 *  @param[in] modelFile It is representing for EDEN model file such as file path.
 *  @param[out] modelId It is representing for constructed EdenModel with a unique id.
 *  @param[in] preference It is representing for a model preference.
 *  @returns return code
 */
RtRet OpenModelFromFile(EdenModelFile* modelFile, uint32_t* modelId, ModelPreference preference) {
    LOGI(EDEN_RT_STUB, "(+)\n");

    EdenModelHidl emh;
    int is_found_path = 0;

    emh.modelFileFormat = (EdenModelFileHidl)modelFile->modelFileFormat;

    if (modelFile->pathToModelFile != NULL) {
        emh.pathToModelFile =
            hidl_string((char*)modelFile->pathToModelFile, modelFile->lengthOfPath);
        emh.lengthOfPath = modelFile->lengthOfPath;
        is_found_path = 1;
    } else {
        emh.lengthOfPath = 0;
    }

    if (modelFile->pathToWeightBiasFile != NULL) {
        emh.pathToWeightBiasFile =
            hidl_string((char*)modelFile->pathToWeightBiasFile,
                                                modelFile->lengthOfWeightBias);
        emh.lengthOfWeightBias = modelFile->lengthOfWeightBias;
        is_found_path = 1;
    } else {
        emh.lengthOfWeightBias = 0;
    }

    if (is_found_path == 0) {
        LOGD(EDEN_RT_STUB, "not found path\n");
        return RT_PARAM_INVALID;
    }

    ModelPreferenceHidl mp;
    mp.userPreference.hw = (HwPreferenceHidl)preference.userPreference.hw;
    mp.userPreference.mode = (ModePreferenceHidl)preference.userPreference.mode;
    mp.userPreference.inputBufferMode.enable =
        (bool)preference.userPreference.inputBufferMode.enable;
    mp.userPreference.inputBufferMode.setInputAsFloat =
        (bool)preference.userPreference.inputBufferMode.setInputAsFloat;
    mp.nnApiType = (NnApiTypeHidl)preference.nnApiType;

    LOGD(EDEN_RT_STUB, "inputBufferMode.enable:%d, inputBufferMode.setInputAsFloat:%d\n",
        mp.userPreference.inputBufferMode.enable, mp.userPreference.inputBufferMode.setInputAsFloat);

    // Get current process id
    pid_t stub_pid = getpid();
    // Print out pid
    LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

    RtRet result = RT_SUCCESS;

    {
        std::shared_lock<std::shared_timed_mutex> rlock(mutex_service);

        if (service == nullptr) {
            LOGE(EDEN_RT_STUB, "service is null (-)\n");
            return RT_SERVICE_NOT_AVAILABLE;
        }

        // android::hardware::Return<uint32_t> ret;
        auto ret =
            service->openModel(stub_pid, emh, mp, [&modelId, &result](uint32_t rtRet, uint32_t mId) {
                if (rtRet == RT_SUCCESS) {
                    *modelId = mId;
                } else {
                    result = (RtRet)rtRet;
                }
            });

        if (!ret.isOk()) {
            LOGE(EDEN_RT_STUB, "ret.isOk() is false (-)\n");
            return RT_FAILED;
        }
    }

    LOGI(EDEN_RT_STUB, "(-) modelId: %d, result: %d (-)\n", *modelId, result);
    return result;
}

/**
 *  @brief Read a in-memory model on address and open it as a EdenModel
 *  @details This function reads a in-memory model on a given address and convert it to EdenModel.
 *           The in-memory model should be one of the supported model type in memory.
 *           Once it successes to parse a given in-memory model,
 *           unique model id is returned via modelId.
 *  @param[in] modelTypeInMemory it is representing for in-memory model such as Android NN Model.
 *  @param[in] addr address of in-memory model
 *  @param[in] size size of in-memory model
 *  @param[in] encrypted data on addr is encrypted
 *  @param[out] modelId It is representing for constructed EdenModel with a unique id.
 *  @param[in] preference It is representing for a model preference.
 *  @returns return code
 */
RtRet OpenModelFromMemory(ModelTypeInMemory modelTypeInMemory, int8_t* addr, int32_t size,
                          bool encrypted, uint32_t* modelId, ModelPreference preference) {
    LOGD(EDEN_RT_STUB, "(+)\n");

    EdenModelFromMemoryHidl emfm;
    emfm.modelTypeInMemory = (ModelTypeInMemoryHidl)modelTypeInMemory;
    emfm.size = size;
    emfm.encrypted = encrypted;

    {
        // Get service for ashmem
        sp<IAllocator> mAllocator = IAllocator::getService("ashmem");
        if (mAllocator == nullptr) {
            LOGE(EDEN_RT_STUB, "Fail to get allocator from ashmem service.\n");
            return RT_PARAM_INVALID;
        }

        // Allocator memory on ashmem
        bool success;
        hidl_memory hidlMem;
        auto hidlStatus = mAllocator->allocate(size,
            [&success, &hidlMem](bool s, const hidl_memory& m) {
                success = s;
                hidlMem = m;
            });
        if (!hidlStatus.isOk()) {
            LOGE(EDEN_RT_STUB, "Fail on hidl transaction!\n");
            return RT_PARAM_INVALID;
        }
        if (!success) {
            LOGE(EDEN_RT_STUB, "Fail on mAllocator->allocate!, size=[%d]\n", size);
            return RT_PARAM_INVALID;
        }

        // Copy data on addr to hidl_memory allocated by ashmem.
        sp<IMemory> memory = mapMemory(hidlMem);
        void* data = memory->getPointer();
        char* ptr = (char*)data;
        memory->update();
        std::memcpy((void*)ptr, (void*)addr, size);
        memory->commit();

        emfm.mem = hidlMem;
    }

    ModelPreferenceHidl mp;
    mp.userPreference.hw = (HwPreferenceHidl)preference.userPreference.hw;
    mp.userPreference.mode = (ModePreferenceHidl)preference.userPreference.mode;
    mp.userPreference.inputBufferMode.enable =
        (bool)preference.userPreference.inputBufferMode.enable;
    mp.userPreference.inputBufferMode.setInputAsFloat =
        (bool)preference.userPreference.inputBufferMode.setInputAsFloat;
    mp.nnApiType = (NnApiTypeHidl)preference.nnApiType;

    LOGD(EDEN_RT_STUB, "inputBufferMode.enable:%d, inputBufferMode.setInputAsFloat:%d\n",
        mp.userPreference.inputBufferMode.enable, mp.userPreference.inputBufferMode.setInputAsFloat);

    // Get current process id
    pid_t stub_pid = getpid();
    // Print out pid
    LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

    RtRet result = RT_SUCCESS;
    {
        std::shared_lock<std::shared_timed_mutex> rlock(mutex_service);

        if (service == nullptr) {
            LOGE(EDEN_RT_STUB, "service is null (-)\n");
            return RT_SERVICE_NOT_AVAILABLE;
        }

        // android::hardware::Return<uint32_t> ret;
        auto ret =
            service->openModelFromMemory(stub_pid, emfm, mp, [&modelId, &result](uint32_t rtRet, uint32_t mId) {
                if (rtRet == RT_SUCCESS) {
                    *modelId = mId;
                } else {
                   result = (RtRet)rtRet;
                }
            });

        if (!ret.isOk()) {
            LOGE(EDEN_RT_STUB, "ret.isOk() is false (-)\n");
            return RT_FAILED;
        }
    }

    LOGD(EDEN_RT_STUB, "modelId: %d, result: %d (-)\n", *modelId, result);
    return result;
}

/**
 *  @brief Allocate a buffer for input to execute a model
 *  @details This function allocates an efficient buffer to execute a model.
 *  @param[in] modelId The model id to be applied by.
 *  @param[out] buffers Array of EdenBuffers for input
 *  @param[out] numOfBuffers # of buffers
 *  @returns return code
 */
RtRet AllocateInputBuffers(uint32_t modelId,
                           EdenBuffer** buffers,
                           int32_t* numOfBuffers) {
    LOGI(EDEN_RT_STUB, "(+)\n");

    RtRet result = RT_SUCCESS;

    // Get current process id
    pid_t stub_pid = getpid();
    // Print out pid
    LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

    {
        std::shared_lock<std::shared_timed_mutex> rlock(mutex_service);

        if (service == nullptr) {
            LOGD(EDEN_RT_STUB, "service is null\n");
            return RT_SERVICE_NOT_AVAILABLE;
        }

        // android::hardware::Return<uint32_t> ret;
        auto ret =
            service->allocateInputBuffer(stub_pid, modelId, [&](uint32_t rtRet, const auto &hd) {
            if (rtRet == RT_SUCCESS) {
                const native_handle_t* handle = hd.getNativeHandle();
                int num = *numOfBuffers = handle->numFds;
                EdenBuffer* newBuffer = new EdenBuffer[num];

                for (int idx = 0; idx < num; idx++) {
                    EdenBuffer* buffer = &(newBuffer[idx]);
                    uint32_t fd = handle->data[idx];
                    buffer->size = handle->data[num + idx];
                    buffer->addr =
                    mmap(NULL, buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                    LOGD(EDEN_RT_STUB, "alloc inputbuf fd: %d, size: %d\n", fd, buffer->size);
                }
                uint64_t serverBuf = (uint64_t)((uint64_t)handle->data[num * 2] << 32 |
                        CLRHIGH((uint64_t)handle->data[num * 2 + 1]));
                mMapClientbufServerbuf.insert(std::make_pair((uint64_t)newBuffer,
                            std::make_pair(serverBuf, buffers)));
                *buffers = newBuffer;
            } else {
                result = (RtRet)rtRet;
            }
        });

        if (!ret.isOk()) {
            LOGE(EDEN_RT_STUB, "ret.isOk() is false (-)\n");
            return RT_FAILED;
        } else {
            if (!result) {
                LOGD(EDEN_RT_STUB, "service->allocateInputBuffer success\n");
            } else {
                LOGE(EDEN_RT_STUB, "service->allocateInputBuffer: %d\n", result);
            }
        }
    }

    LOGI(EDEN_RT_STUB, "(-)\n");
    return result;
}

/**
 *  @brief Allocate a buffer for output to execute a model
 *  @details This function allocates an efficient buffer to execute a model.
 *  @param[in] modelId The model id to be applied by.
 *  @param[out] buffers Array of EdenBuffers for input
 *  @param[out] numOfBuffers # of buffers
 *  @returns return code
 */
RtRet AllocateOutputBuffers(uint32_t modelId,
                            EdenBuffer** buffers,
                            int32_t* numOfBuffers) {
    LOGI(EDEN_RT_STUB, "(+)\n");

    // Get current process id
    pid_t stub_pid = getpid();
    // Print out pid
    LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

    RtRet result = RT_SUCCESS;
    {
        std::shared_lock<std::shared_timed_mutex> rlock(mutex_service);

        if (service == nullptr) {
            LOGD(EDEN_RT_STUB, "service is null\n");
            return RT_SERVICE_NOT_AVAILABLE;
        }

        // android::hardware::Return<uint32_t> ret;
        auto ret =
            service->allocateOutputBuffer(stub_pid, modelId, [&](uint32_t rtRet, const auto &hd) {
            if (rtRet == RT_SUCCESS) {
                const native_handle_t* handle = hd.getNativeHandle();
                int num = *numOfBuffers = handle->numFds;
                EdenBuffer* newBuffer = new EdenBuffer[num];

                for (int idx = 0; idx < num; idx++) {
                    EdenBuffer* buffer = &(newBuffer[idx]);
                    uint32_t fd = handle->data[idx];
                    buffer->size = handle->data[num + idx];
                    buffer->addr =
                        mmap(NULL, buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                    LOGD(EDEN_RT_STUB, "alloc outbuf fd: %d, size: %d\n", fd, buffer->size);
                    LOGD(EDEN_RT_STUB, "out allc mmap addr: %p\n", buffer->addr);
                }
                uint64_t serverBuf = (uint64_t)((uint64_t)handle->data[num * 2] << 32 |
                                            CLRHIGH((uint64_t)handle->data[num * 2 + 1]));
                mMapClientbufServerbuf.insert(std::make_pair((uint64_t)newBuffer,
                                            std::make_pair(serverBuf, buffers)));
                *buffers = newBuffer;
                LOGD(EDEN_RT_STUB, "client buf: %p, gtest buf: %p\n", newBuffer, buffers);
            } else {
                result = (RtRet)rtRet;
            }
        });

        if (!ret.isOk()) {
            LOGE(EDEN_RT_STUB, "ret.isOk() is false (-)\n");
            return RT_FAILED;
        } else {
            if (!result) {
                LOGD(EDEN_RT_STUB, "service->allocateOutputBuffer success\n");
            } else {
                LOGE(EDEN_RT_STUB, "service->allocateOutputBuffer: %d\n", result);
            }
        }
    }

    LOGI(EDEN_RT_STUB, "(-)\n");
    return (RtRet)result;
}

/**
 *  @brief Execute EDEN Req
 *  @details This API function executes EdenRequest with preference.
 *  @param[in] req It consists of EDEN Model ID, input/output buffers and callback.
 *  @param[in] evt Callback function defined by User.
 *  @param[in] preference it determines how to run EdenModel with preference.
 *  @returns return code
 */
RtRet ExecuteReq(EdenRequest* req, EdenEvent** /*evt*/, RequestPreference preference) {
    LOGI(EDEN_RT_STUB, "(+)\n");

    EdenRequestHidl erh;
    erh.modelId = req->modelId;

    auto it = mMapClientbufServerbuf.find((uint64_t)req->inputBuffers);
    if (it == mMapClientbufServerbuf.end()) {
        LOGE(EDEN_RT_STUB, "failed to find ServerInputBuffers according to buffers (-)");
        return RT_FAILED;
    }
    erh.inputAddr = it->second.first;

    it = mMapClientbufServerbuf.find((uint64_t)req->outputBuffers);
    if (it == mMapClientbufServerbuf.end()) {
        LOGE(EDEN_RT_STUB, "failed to find ServerOutputBuffers according to buffers (-)");
        return RT_FAILED;
    }
    erh.outputAddr = it->second.first;

    erh.cValue.pollingAddr = (uint64_t)&req->callback->requestId;
    erh.cValue.value = (uint64_t)req;

    RequestPreferenceHidl rp;
    rp.userPreference.hw = (HwPreferenceHidl)preference.userPreference.hw;
    rp.userPreference.mode = (ModePreferenceHidl)preference.userPreference.mode;
    rp.userPreference.inputBufferMode.enable =
        (bool)preference.userPreference.inputBufferMode.enable;
    rp.userPreference.inputBufferMode.setInputAsFloat =
        (bool)preference.userPreference.inputBufferMode.setInputAsFloat;

    LOGD(EDEN_RT_STUB, "inputBufferMode.enable:%d, inputBufferMode.setInputAsFloat:%d\n",
        rp.userPreference.inputBufferMode.enable, rp.userPreference.inputBufferMode.setInputAsFloat);

    sp<Callback> callback = new Callback(req->callback->notify);
    //eCallback->setNotifyFunc(req->callback->notify);

    // Get current process id
    pid_t stub_pid = getpid();
    // Print out pid
    LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

    uint32_t result;
    {
        std::shared_lock<std::shared_timed_mutex> rlock(mutex_service);

        if (service == nullptr) {
            LOGE(EDEN_RT_STUB, "service is nullptr (-)\n");
            return RT_SERVICE_NOT_AVAILABLE;
        }

        // android::hardware::Return<uint32_t> ret;
        auto ret = service->executeReq(stub_pid, erh, callback, rp);

        if (!ret.isOk()) {
            LOGE(EDEN_RT_STUB, "ret.isOk() is false (-)\n");
            return RT_FAILED;
        } else {
            result = static_cast<uint32_t>(ret);
            if (!result) {
                LOGD(EDEN_RT_STUB, "service->executeReq success\n");
            } else {
                LOGE(EDEN_RT_STUB, "service->executeReq: %d\n", result);
            }
        }
    }

    LOGI(EDEN_RT_STUB, "(-)\n");
    return (RtRet)result;
}

/**
 *  @brief Release a buffer allocated by Eden framework
 *  @details This function releases a buffer returned by AllocateXXXBuffers.
 *  @param[in] modelId The model id to be applied by.
 *  @param[in] buffers Buffer pointer allocated by AllocateXXXBuffers
 *  @returns return code
 */
RtRet FreeBuffers(uint32_t modelId, EdenBuffer* buffers) {
    LOGI(EDEN_RT_STUB, "(+) modelId : %d\n", modelId);

    auto it = mMapClientbufServerbuf.find((uint64_t)buffers);
    if (it == mMapClientbufServerbuf.end()) {
        LOGE(EDEN_RT_STUB, "failed to find handle according to buffers");
        return RT_FAILED;
    }

    // Get current process id
    pid_t stub_pid = getpid();
    // Print out pid
    LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

    int32_t num;
    RtRet result = RT_SUCCESS;
    {
        std::shared_lock<std::shared_timed_mutex> rlock(mutex_service);

        if (service == nullptr) {
            LOGD(EDEN_RT_STUB, "service is null\n");
            return RT_SERVICE_NOT_AVAILABLE;
        }

        // android::hardware::Return<uint32_t> ret;
        auto ret = service->freeBuffer(stub_pid, modelId, it->second.first,
                                [&result, &num](uint32_t rtRet, int32_t numOfBuffers) {
            if (rtRet == RT_SUCCESS) {
                num = numOfBuffers;
            } else {
                result = (RtRet)rtRet;
            }
        });

        if (!ret.isOk()) {
            LOGE(EDEN_RT_STUB, "ret.isOk() is false (-)\n");
            return RT_FAILED;
        } else {
            if (!result) {
                LOGD(EDEN_RT_STUB, "service->freeBuffer success\n");
            } else {
                LOGE(EDEN_RT_STUB, "service->freeBuffer: %d\n", result);
            }
        }
    }

    for (int idx = 0; idx < num; idx++) {
        EdenBuffer* buffer = &(buffers[idx]);
        LOGD(EDEN_RT_STUB, "free buffer unmap size: %d", buffer->size);
        munmap((void *)buffer->addr, buffer->size);
    }

    mMapClientbufServerbuf.erase((uint64_t)buffers);
    delete[] buffers;

    LOGI(EDEN_RT_STUB, "(-)\n");
    return (RtRet)result;
}

/**
 *  @brief Close EDEN Model
 *  @details This API function releases resources related with the EDEN Model.
 *  @param[in] modelId It is a unique id for EDEN Model.
 *  @returns return code
 */
RtRet CloseModel(uint32_t modelId) {
    LOGI(EDEN_RT_STUB, "(+)\n");

    // Get current process id
    pid_t stub_pid = getpid();
    // Print out pid
    LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

    uint32_t result;
    {
        std::shared_lock<std::shared_timed_mutex> rlock(mutex_service);

        if (service == nullptr) {
            LOGD(EDEN_RT_STUB, "service is null (-)\n");
            return RT_SERVICE_NOT_AVAILABLE;
        }

        // android::hardware::Return<uint32_t> ret;
        auto ret = service->closeModel(stub_pid, modelId);

        if (!ret.isOk()) {
            LOGE(EDEN_RT_STUB, "ret.isOk() is false (-)\n");
            return RT_FAILED;
        } else {
            result = static_cast<uint32_t>(ret);
            if (!result) {
                LOGD(EDEN_RT_STUB, "service->closeModel success\n");
            } else {
                LOGE(EDEN_RT_STUB, "service->closeModel: %d\n", result);
            }
        }
    }

    LOGI(EDEN_RT_STUB, "(-)\n");
    return (RtRet)result;
}

/**
 *  @brief Shutdown EDEN Runtime
 *  @details This API function close all EDEN Models with related resources for shutdown EDEN Framework.
 *  @param void
 *  @returns return code
 */
RtRet Shutdown(void) {
    LOGI(EDEN_RT_STUB, "(+)\n");

    // Get current process id
    pid_t stub_pid = getpid();
    // Print out pid
    LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

    uint32_t result;
    {
        std::shared_lock<std::shared_timed_mutex> rlock(mutex_service);

        if (service == nullptr) {
            LOGD(EDEN_RT_STUB, "service is null\n");
            return RT_SERVICE_NOT_AVAILABLE;
        }

        // android::hardware::Return<uint32_t> ret;
        auto ret = service->shutdown(stub_pid);
        if (!ret.isOk()) {
            LOGE(EDEN_RT_STUB, "ret.isOk() is false (-)\n");
            return RT_FAILED;
        } else {
            result = static_cast<uint32_t>(ret);
            if (!result) {
                LOGD(EDEN_RT_STUB, "service->shutdown success\n");
            } else {
                LOGE(EDEN_RT_STUB, "service->shutdown: %d\n", result);
            }
        }
    }

    LOGI(EDEN_RT_STUB, "(-)\n");
    return (RtRet)result;
}

/**
 *  @brief Get the input buffer information
 *  @details This function gets buffer shape for input buffer of a specified model.
 *  @param[in] modelId The model id to be applied by.
 *  @param[in] inputIndex Input index starting 0.
 *  @param[out] width Width
 *  @param[out] height Height
 *  @param[out] channel Channel
 *  @param[out] number Number
 *  @returns return code
 */
RtRet GetInputBufferShape(uint32_t modelId, int32_t inputIndex, int32_t* width, int32_t* height, int32_t* channel, int32_t* number) {
    LOGI(EDEN_RT_STUB, "(+)\n");

    // Get current process id
    pid_t stub_pid = getpid();
    // Print out pid
    LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

    RtRet result = RT_SUCCESS;
    {
        std::shared_lock<std::shared_timed_mutex> rlock(mutex_service);

        if (service == nullptr) {
            LOGD(EDEN_RT_STUB, "service is null (-)\n");
            return RT_FAILED;
        }

        // android::hardware::Return<uint32_t> ret;
        auto ret = service->getInputBufferShape(stub_pid, modelId, inputIndex,
            [&](uint32_t rtRet, int32_t inputWidth, int32_t inputHeight,
            int32_t inputChannel, int32_t inputNumber) {
                if (rtRet == RT_SUCCESS) {
                    *width = inputWidth;
                    *height = inputHeight;
                    *channel = inputChannel;
                    *number = inputNumber;
                } else {
                    *width = 0;
                    *height = 0;
                    *channel = 0;
                    *number = 0;
                    result = (RtRet)rtRet;
                }
            }
        );

        if (!ret.isOk()) {
            LOGE(EDEN_RT_STUB, "ret.isOk() is false (-)\n");
            return RT_FAILED;
        } else {
            if (result == RT_SUCCESS) {
                LOGD(EDEN_RT_STUB, "service->getInputBufferShape success\n");
            } else {
                LOGE(EDEN_RT_STUB, "service->getInputBufferShape: %d\n", result);
            }
        }
    }

    LOGI(EDEN_RT_STUB, "(-)\n");
    return (RtRet)result;
}

/**
 *  @brief Get the output buffer information
 *  @details This function gets buffer shape for output buffers of a specified model.
 *  @param[in] modelId The model id to be applied by.
 *  @param[in] outputIndex Output index starting 0.
 *  @param[out] width Width
 *  @param[out] height Height
 *  @param[out] channel Channel
 *  @param[out] number Number
 *  @returns return code
 */
RtRet GetOutputBufferShape(uint32_t modelId, int32_t outputIndex, int32_t* width, int32_t* height, int32_t* channel, int32_t* number) {
    LOGI(EDEN_RT_STUB, "(+)\n");

    // Get current process id
    pid_t stub_pid = getpid();
    // Print out pid
    LOGD(EDEN_RT_STUB, "stub_pid=[%d]\n", stub_pid);

    RtRet result = RT_SUCCESS;
    {
        std::shared_lock<std::shared_timed_mutex> rlock(mutex_service);

        if (service == nullptr) {
            LOGD(EDEN_RT_STUB, "service is null\n");
            return RT_FAILED;
        }

        // android::hardware::Return<uint32_t> ret;
        auto ret = service->getOutputBufferShape(stub_pid, modelId, outputIndex,
            [&](uint32_t rtRet, int32_t outputWidth, int32_t outputHeight,
            int32_t outputChannel, int32_t outputNumber) {
                if (rtRet == RT_SUCCESS) {
                    *width = outputWidth;
                    *height = outputHeight;
                    *channel = outputChannel;
                    *number = outputNumber;
                } else {
                    *width = 0;
                    *height = 0;
                    *channel = 0;
                    *number = 0;
                    result = (RtRet)rtRet;
                }
            }
        );

        if (!ret.isOk()) {
            LOGE(EDEN_RT_STUB, "ret.isOk() is false (-)\n");
            return RT_FAILED;
        } else {
            if (result == RT_SUCCESS) {
                LOGD(EDEN_RT_STUB, "service->getInputBufferShape success\n");
            } else {
                LOGE(EDEN_RT_STUB, "service->getInputBufferShape: %d\n", result);
            }
        }
    }

    LOGI(EDEN_RT_STUB, "(-)\n");
    return (RtRet)result;
}

#if 0
RtRet GetState(EdenState* state) {
    LOGD(EDEN_RT_STUB, "%s started\n", __func__);

    if (service == NULL) {
        LOGD(EDEN_RT_STUB, "service is null\n");
        return RT_FAILED;
    }
}
#endif

}
}
