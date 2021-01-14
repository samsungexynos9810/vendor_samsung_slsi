/*
 * Copyright (C) 2017 The Android Open Source Project
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


#include <iostream>
#include <vector>
#include <cstdint>  // uint32_t
#include <memory>   // nullptr

#include "log.h"

#include <hidl/LegacySupport.h>  // configureRpcThreadpool, joinRpcThreadpool

#include "ValidateHal.h"         // validateModel, validateExecutionPreference
#include "Utils.h"               // convertToV1_0, convertToV1_1, android::nn::initVLogMask, logModelToInfo, DRIVER

#include "EdenDriver.h"          // EdenDriver
#include "NNAgent.h"             // NNAgent

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EdenDriver::EdenDriver"

namespace android {
namespace nn {
namespace eden_driver {

static void callNotifyWithError(const sp<V1_0::IPreparedModelCallback>& callback_1_0, ErrorStatus status) {
    callback_1_0->notify(status, nullptr);
}

static void callNotifyWithError(const sp<V1_2::IPreparedModelCallback>& callback_1_2, ErrorStatus status) {
    callback_1_2->notify_1_2(status, nullptr);
}

template <typename T_Model, typename T_Callback>
static Return<ErrorStatus> prepareModelBaseOnEdenDriver(NNAgent* nnAgent,
                                                        const T_Model& model, ExecutionPreference preference,
                                                        const hidl_vec<hidl_handle>* modelCache,
                                                        const hidl_vec<hidl_handle>* dataCache,
                                                        const HidlToken* token,
                                                        const T_Callback& callback) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);
#if 0
    if (VLOG_IS_ON(DRIVER)) {
        VLOG(DRIVER) << "prepareModelBaseOnEdenDriver";
        logModelToInfo(model);
    }
#endif
    if (callback.get() == nullptr) {
        LOGE(EDEN_DRIVER, "invalid callback passed to prepareModelBaseOnEdenDriver");
        return ErrorStatus::INVALID_ARGUMENT;
    }
    if (!validateModel(model) || !validateExecutionPreference(preference)) {
        callNotifyWithError(callback, ErrorStatus::INVALID_ARGUMENT);
        return ErrorStatus::INVALID_ARGUMENT;
    }

    int32_t retCode = nnAgent->prepareModel(convertToV1_2(model), preference, modelCache, dataCache, token, callback);
    if (retCode != RET_OK) {
        LOGE(EDEN_DRIVER, "Error on preparedModel(), (retCode=%d)\n", retCode);
        callNotifyWithError(callback, ErrorStatus::INVALID_ARGUMENT);
        return ErrorStatus::INVALID_ARGUMENT;
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return ErrorStatus::NONE;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// IDEVICE_1_0 /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Edendriver_1_0 constructor
 * @details This function creates a NNAgent.
 */
EdenDriver_1_0::EdenDriver_1_0(const char* name) : name(name), nnAgent(nullptr) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    nnAgent = new NNAgent();

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

/**
 * @brief Edendriver_1_0 destructor
 * @details This function delete a NNAgent.
 */
EdenDriver_1_0::~EdenDriver_1_0() {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    delete nnAgent;
    nnAgent = nullptr;

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

/**
 * @brief Get capabilities for 1.0
 * @details This function gets capabilities from NN HAL.
 * @param[in] cb callback to be executed.
 * @returns return code
 */
Return<void> EdenDriver_1_0::getCapabilities(getCapabilities_cb cb) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    return getCapabilities_1_1(
        [&](ErrorStatus error, const V1_1::Capabilities& capabilities) {
            // TODO(dgross): Do we need to check compliantWithV1_0(capabilities)?
            cb(error, convertToV1_0(capabilities));
        });
}

/**
 * @brief Get supported operations on a given model for 1.0
 * @details This function checks operations on a given model and returns which operations are supported.
 * @param[in] model Android NN Model.
 * @param[in] cb callback to be executed.
 * @returns return code
 */
Return<void> EdenDriver_1_0::getSupportedOperations(const V1_0::Model& model,
                                                getSupportedOperations_cb cb) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    if (!validateModel(model)) {
        std::vector<bool> supportedOperations;

        cb(ErrorStatus::INVALID_ARGUMENT, supportedOperations);
        return Void();
    }

    return getSupportedOperations_1_1(convertToV1_1(model), cb);
}

/**
 * @brief Prepare model matched to a given Android NN Model for 1.0
 * @details This function prepares a model matched to a given Android NN Model.
 * @param[in] model Android NN Model.
 * @param[in] cb callback to be executed.
 * @returns return code
 */
Return<ErrorStatus> EdenDriver_1_0::prepareModel(const V1_0::Model& model,
                                             const sp<V1_0::IPreparedModelCallback>& callback_1_0) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    if (callback_1_0.get() == nullptr) {
        LOGE(EDEN_DRIVER, "invalid callback passed to prepareModel\n");
        return ErrorStatus::INVALID_ARGUMENT;
    }
    if (!validateModel(model)) {
        callback_1_0->notify(ErrorStatus::INVALID_ARGUMENT, nullptr);
        return ErrorStatus::INVALID_ARGUMENT;
    }
    return prepareModel_1_1(convertToV1_1(model), ExecutionPreference::FAST_SINGLE_ANSWER, callback_1_0);
}

/**
 * @brief Get status of NN HAL
 * @details This function returns status of NN HAL.
 * @returns DeviceStatus
 */
Return<DeviceStatus> EdenDriver_1_0::getStatus() {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    DeviceStatus status;
    nnAgent->getStatus(status);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return status;
}

/**
 * @brief main function to start NN HAL as a service
 * @details This function is a main function to be launched as a service.
 * @returns return code
 */
int EdenDriver_1_0::run() {
    android::hardware::configureRpcThreadpool(4, true);
    if (registerAsService(name) != android::OK) {
        LOGE(EDEN_DRIVER, "Could not register service %s\n", name.c_str());
        return 1;
    } else {
        LOGI(EDEN_DRIVER, "Register service %s\n", name.c_str());
    }
    android::hardware::joinRpcThreadpool();
    LOGI(EDEN_DRIVER, "Service exited!\n");
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// IDEVICE_1_1 /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Edendriver_1_1 constructor
 * @details This function creates a NNAgent.
 */
EdenDriver_1_1::EdenDriver_1_1(const char* name) : EdenDriver_1_0(name) {
    LOGD(EDEN_DRIVER, "%s(+-)\n", __func__);
}

/**
 * @brief Edendriver_1_1 destructor
 * @details This function delete a NNAgent.
 */
EdenDriver_1_1::~EdenDriver_1_1() {
    LOGD(EDEN_DRIVER, "%s(+-)\n", __func__);
}

/**
 * @brief Get capabilities for 1.1
 * @details This function gets capabilities from NN HAL.
 * @param[in] cb callback to be executed.
 * @returns return code
 */
Return<void> EdenDriver_1_1::getCapabilities_1_1(getCapabilities_1_1_cb cb) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    return getCapabilities_1_2(
        [&](ErrorStatus error, const V1_2::Capabilities& capabilities) {
            // TODO(dgross): Do we need to check compliantWithV1_0(capabilities)?
            cb(error, convertToV1_1(capabilities));
        });
}

/**
 * @brief Get supported operations on a given model for 1.1
 * @details This function checks operations on a given model and returns which operations are supported.
 * @param[in] model Android NN Model.
 * @param[in] cb callback to be executed.
 * @returns return code
 */
Return<void> EdenDriver_1_1::getSupportedOperations_1_1(const V1_1::Model& model,
                                                    getSupportedOperations_1_1_cb cb) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    if (!validateModel(model)) {
        std::vector<bool> supportedOperations;
        cb(ErrorStatus::INVALID_ARGUMENT, supportedOperations);
        return Void();
    }

    return getSupportedOperations_1_2(convertToV1_2(model), cb);
}

/**
 * @brief Prepare model matched to a given Android NN Model for 1.1
 * @details This function prepares a model matched to a given Android NN Model.
 * @param[in] model Android NN Model.
 * @param[in] preference ExecutionPreference
 * @param[in] cb callback to be executed.
 * @returns return code
 */
Return<ErrorStatus> EdenDriver_1_1::prepareModel_1_1(const V1_1::Model& model, ExecutionPreference preference,
                                                   const sp<V1_0::IPreparedModelCallback>& callback_1_0) {
    return prepareModelBaseOnEdenDriver(this->nnAgent, model, preference, nullptr, nullptr, nullptr, callback_1_0);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// IDEVICE_1_2 /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Edendriver constructor
 * @details This function creates a NNAgent.
 */
EdenDriver::EdenDriver(const char* name) : EdenDriver_1_1(name) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);
}

/**
 * @brief Edendriver destructor
 * @details This function delete a NNAgent.
 */
EdenDriver::~EdenDriver() {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);
}

Return<void> EdenDriver::getVersionString(getVersionString_cb cb) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    cb(ErrorStatus::NONE, "EdenDriver_1_2");

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return Void();
}

Return<void> EdenDriver::getType(getType_cb cb) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    cb(ErrorStatus::NONE, V1_2::DeviceType::ACCELERATOR);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return Void();
}

/**
 * @brief Get capabilities for 1.2
 * @details This function gets capabilities from NN HAL.
 * @param[in] cb callback to be executed.
 * @returns return code
 */
Return<void> EdenDriver::getCapabilities_1_2(getCapabilities_1_2_cb cb) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);
    android::nn::initVLogMask();

    V1_2::Capabilities capabilities;
    int32_t retCode = nnAgent->getCapabilities(capabilities);
    if (retCode != RET_OK) {
        LOGE(EDEN_DRIVER, "Error on getCapabilities(), (retCode=%d)\n", retCode);
        cb(ErrorStatus::INVALID_ARGUMENT, capabilities);
    } else {
        cb(ErrorStatus::NONE, capabilities);
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return Void();
}

Return<void> EdenDriver::getSupportedExtensions(getSupportedExtensions_cb cb) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    cb(ErrorStatus::NONE, {/* No extensions. */});

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return Void();
}

/**
 * @brief Get supported operations on a given model for 1.2
 * @details This function checks operations on a given model and returns which operations are supported.
 * @param[in] model Android NN Model.
 * @param[in] cb callback to be executed.
 * @returns return code
 */
Return<void> EdenDriver::getSupportedOperations_1_2(const V1_2::Model& model,
                                                    getSupportedOperations_1_2_cb cb) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    std::vector<bool> supportedOperations;
    ErrorStatus status = ErrorStatus::NONE;
    if (validateModel(model)) {
        int32_t retCode = nnAgent->getSupportedOperations(model, supportedOperations);
        if (retCode != RET_OK) {
            LOGE(EDEN_DRIVER, "Error on getSupportedOperations(), (retCode=%d)\n", retCode);
            status = ErrorStatus::INVALID_ARGUMENT;
        }
    } else {
        status = ErrorStatus::INVALID_ARGUMENT;
    }

    cb(status, supportedOperations);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return Void();
}

Return<void> EdenDriver::getNumberOfCacheFilesNeeded(getNumberOfCacheFilesNeeded_cb cb) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    cb(ErrorStatus::NONE, /*numModelCache=*/0, /*numDataCache=*/0);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return Void();
}

/**
 * @brief Prepare model matched to a given Android NN Model for 1.2
 * @details This function prepares a model matched to a given Android NN Model.
 * @param[in] model Android NN Model.
 * @param[in] preference ExecutionPreference
 * @param[in] modelCache
 * @param[in] dataCache
 * @param[in] token
 * @param[in] cb callback to be executed.
 * @returns return code
 */
Return<ErrorStatus> EdenDriver::prepareModel_1_2(const V1_2::Model& model, ExecutionPreference preference,
                                                 const hidl_vec<hidl_handle>& modelCache,
                                                 const hidl_vec<hidl_handle>& dataCache,
                                                 const HidlToken& token,
                                                 const sp<V1_2::IPreparedModelCallback>& callback_1_2) {
    return prepareModelBaseOnEdenDriver(this->nnAgent, model, preference, &modelCache, &dataCache, &token, callback_1_2);
}

Return<ErrorStatus> EdenDriver::prepareModelFromCache(const hidl_vec<hidl_handle>& /*modelCache*/,
                                                      const hidl_vec<hidl_handle>& /*dataCache*/,
                                                      const HidlToken& /*token*/,
                                                      const sp<V1_2::IPreparedModelCallback>& callback_1_2) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    callback_1_2->notify_1_2(ErrorStatus::GENERAL_FAILURE, nullptr);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return ErrorStatus::NONE;
}

}  // namespace eden_driver
}  // namespace nn
}  // namespace android

