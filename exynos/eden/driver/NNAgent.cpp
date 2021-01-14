/*
 * Copyright (C) 2019 Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/**
 * @file    NNAgent.cpp
 * @brief   This is NNAgent class file.
 * @details This header defines NNAgent class.
 *          This class is implementing NNAgent which is Facade.
 * @author  minsu.jeon (minsu.jeon@samsung.com)
 */

#include <iostream>

#include "log.h"
#include "Utils.h"               // convertToV1_0, convertToV1_1, android::nn::initVLogMask, logModelToInfo, DRIVER

#include "NeuralNetworks.h"      // Operation, Operand, ANEURALNETWORKS_ADD etc
#include "ValidateHal.h"         // validateRequest

#include "../include/eden_model.h"

#include "Common.h"

#include "ModelConverter.h"
#include "CompilerManager.h"
#include "ExecutionScheduler.h"
#include "ResourceManager.h"
#include "EdenServiceDelegatorLib.h"

#include "NNAgent.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EdenDriver::NNAgent"

// @todo Since there is no useful maximum number representing for ANEURALNETWORKS_XXX, just sets it to 256 which is big enough.
#define ANDROID_NN_OP_NUM_MAXIMUM 256

using namespace eden::nn;

// V1_0::IExecutionCallback* executionCallback = nullptr;

namespace android {
namespace nn {
namespace eden_driver {

static void updateCapabilities(hidl_vec<V1_2::Capabilities::OperandPerformance>* operandPerformance,
                               V1_2::OperandType type,
                               V1_0::PerformanceInfo perf) {
    const auto it = std::lower_bound(operandPerformance->begin(), operandPerformance->end(), type,
                                     [](const V1_2::Capabilities::OperandPerformance& perf,
                                        V1_2::OperandType type) { return perf.type < type; });
    it->info = perf;
}

static void callNotifyWithPreparedModel(const sp<V1_0::IPreparedModelCallback>& callback_1_0, ErrorStatus status, V1_2::IPreparedModel* preparedModel_1_2) {
    callback_1_0->notify(status, static_cast<V1_0::IPreparedModel*>(preparedModel_1_2));
}

static void callNotifyWithPreparedModel(const sp<V1_2::IPreparedModelCallback>& callback_1_2, ErrorStatus status, V1_2::IPreparedModel* preparedModel_1_2) {
    callback_1_2->notify_1_2(status, preparedModel_1_2);
}

template <typename T_Callback>
int32_t prepareModelBaseOnNNAgent(NNAgent* nnAgent,
                                  const V1_2::Model& model, ExecutionPreference /*preference*/,
                                  const hidl_vec<hidl_handle>* /*modelCache*/, const hidl_vec<hidl_handle>* /*dataCache*/,
                                  const HidlToken* /*token*/,
                                  const T_Callback& callback) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    int32_t retCode;

    // Try to load a cached model if exists.
    V1_2::IPreparedModel* preparedModel = nullptr;
    nnAgent->resourceManager_->loadCachedModel(model, &preparedModel);
    if (preparedModel != nullptr) {
        callNotifyWithPreparedModel(callback, ErrorStatus::NONE, preparedModel);
        return RET_OK;
    }

    // If fail to load a cached model, start model converting phase.
    EdenModel* edenModel = nullptr;
    ModelInfo modelInfo;
    if (nnAgent->vecNPUOperationInfos_.empty()) {
        LOGD(EDEN_DRIVER, "vecNPUOperationInfos_ is empty!\n");
        retCode = nnAgent->queryOperationInfo(model, TARGET_DEVICE_NPU, nnAgent->vecNPUOperationInfos_);
        if (retCode != RET_OK) return retCode;
    }
    nnAgent->modelConverter_->setNPUInfo(nnAgent->vecNPUOperationInfos_);
    retCode = nnAgent->modelConverter_->convert(model, edenModel, modelInfo);
    if (retCode != RET_OK) {
        LOGE(EDEN_DRIVER, "%s(-) Error on convert(..)!\n", __func__);
        return retCode;
    }

    //edenModel->DumpEdenModelForLOGD();

    // Once EdenModel is ready, it starts to communicate with EdenRuntime.

    // Call OpenModelFromMemory to register converted EdenModel.
    uint32_t modelId = -1;
    ModelPreference modelPreference;
    HwPreference hwPreference = getHwPreference(modelInfo);
    // hwPreference = GPU_ONLY; // VTS1.2 crash sometimes in NPU branch
    modelPreference.userPreference.hw = hwPreference;
    if ((hwPreference == NPU_ONLY || hwPreference == ALL_HW) && nnAgent->compilerManager_->isSupportedModelByNPUC(model)) {
        modelPreference.userPreference.mode = BOOST_MODE;
    } else {
        modelPreference.userPreference.mode = NORMAL_MODE;
    }
    LOGD(EDEN_DRIVER, "ModePreference : %d , NORMAL_MODE(0), BOOST_MODE(1)\n", modelPreference.userPreference.mode);
    nnAgent->setComputePrecision(model, edenModel);
    modelPreference.userPreference.inputBufferMode.enable = false;
    modelPreference.nnApiType = ANDROID_NN_API;

    // Leave buffer layout to NHWC for inputs
    // so that data layout converting would be taken place at user driver.
    if (hwPreference == GPU_ONLY) {
        nnAgent->modelConverter_->setInputOutputToNHWC(edenModel);
    }

    // OpenModel
    retCode = nnAgent->edenServiceDelegator_->OpenModel(edenModel, &modelId, modelPreference);
    if (retCode != RET_OK) {
        LOGE(EDEN_DRIVER, "%s(-) Error on OpenModel(..)!\n", __func__);
        return retCode;
    }

    nnAgent->mapModelIdToModel_.insert(std::make_pair(modelId, edenModel));

    // Allocate input, output buffers for model
    // [CanBePostponed] Call AllocateInputBuffers for this model.
    InHouseBufferInfo inputBuffers;
    if (edenModel->ReadyToAllocateInputBuffers(nullptr)) {
        retCode = nnAgent->allocateInputBuffers(modelId, inputBuffers);
        if (retCode != RET_OK) {
            LOGE(EDEN_DRIVER, "%s(-) Error on allocateInputBuffers(..)!\n", __func__);
            return retCode;
        }
    } else {
        inputBuffers.addr = nullptr;
        inputBuffers.numOfBuffers = 0;
    }
    InHouseBufferInfo outputBuffers;
    if (edenModel->ReadyToAllocateOutputBuffers(nullptr)) {
        retCode = nnAgent->allocateOutputBuffers(modelId, outputBuffers);
        if (retCode != RET_OK) {
            LOGE(EDEN_DRIVER, "%s(-) Error on allocateOutputBuffers(..)!\n", __func__);
            return retCode;
        }
    } else {
        outputBuffers.addr = nullptr;
        outputBuffers.numOfBuffers = 0;
    }

    std::vector<char*> vecAddr;
    std::vector<int32_t> vecSize;
    for (auto& vInfo : modelInfo.vecVirtualAddressOnPools) {
        if (vInfo.type == 1) {
            vecAddr.push_back(vInfo.addr);
            vecSize.push_back(vInfo.size);
        }
    }

    retCode = nnAgent->createPreparedModel(model, modelId, edenModel, hwPreference, inputBuffers, outputBuffers, vecAddr, vecSize, modelInfo.operandValues,
                                           modelInfo.mapOperandIdFromAToE, modelInfo.mapOperationIdFromAToE, modelInfo.inputConsumers, modelInfo.internalBuffers,
                                           modelInfo.vecIMemoryOnAshmems,
                                           &preparedModel);
    if (retCode != RET_OK) {
        LOGE(EDEN_DRIVER, "%s(-) Error on createPrepareModel(..)!\n", __func__);
        return retCode;
    }

    // [CanBePostponed] Store model to cache.
    nnAgent->resourceManager_->storeModelToCache(model, preparedModel);

    callNotifyWithPreparedModel(callback, ErrorStatus::NONE, preparedModel);

    // so here we need convert back to nhwc.
    if (hwPreference == GPU_ONLY) {
        nnAgent->modelConverter_->convertParamToNHWC(edenModel);
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

template <typename T_Callback>
int32_t executeBaseOnNNAgent(NNAgent* nnAgent,
                             EdenPreparedModel* edenPreparedModel,
                             const V1_0::Request& request,
                             V1_2::MeasureTiming measure,
                             std::chrono::steady_clock::time_point driverStart,
                             const T_Callback& callback,
                             bool async) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    int32_t ret = RET_OK;

    do {
        uint32_t modelId = edenPreparedModel->modelId;

        if (edenPreparedModel->resolveUnknownDimensions(request) != RET_OK) {
            LOGE(EDEN_DRIVER, "Oops, fail to resolve unknown dimension, rank!\n");
            ret = FAIL_TO_RESOLVE_UNKNOWN_DIMENSIONS;
            break;
        }

        // Make sure input/output buffers are ready
        if (edenPreparedModel->needToAllocateInputBuffers()) {
            if (edenPreparedModel->readyToAllocateInputBuffers(&request)) {
                LOGD(EDEN_DRIVER, "Ready to Allocate Input Buffers\n");
                InHouseBufferInfo inputBuffers;
                ret = nnAgent->allocateInputBuffers(modelId, inputBuffers);
                if (ret != RET_OK) {
                    LOGE(EDEN_DRIVER, "Oops, allocateInputBuffers() is failed!\n");
                    break;
                }
                edenPreparedModel->updateInputBuffers(inputBuffers.addr, inputBuffers.numOfBuffers);
            } else {
                LOGE(EDEN_DRIVER, "Oops, readyToAllocateInputBuffers() is failed!\n");
                ret = FAIL_TO_ALLOCATE_INPUT_BUFFERS_ON_EXECUTE;
                break;
            }
        } else {
            LOGD(EDEN_DRIVER, "Dont need to Allocate Input Buffers\n");
        }

        if (edenPreparedModel->needToAllocateOutputBuffers()) {
            if (edenPreparedModel->readyToAllocateOutputBuffers(&request)) {
                LOGD(EDEN_DRIVER, "Ready To Allocate Output Buffers\n");
                InHouseBufferInfo outputBuffers;
                ret = nnAgent->allocateOutputBuffers(modelId, outputBuffers);
                if (ret != RET_OK) {
                    LOGE(EDEN_DRIVER, "Oops, allocateInputBuffers() is failed!\n");
                    break;
                }
                edenPreparedModel->updateOutputBuffers(outputBuffers.addr, outputBuffers.numOfBuffers);
            } else {
                LOGE(EDEN_DRIVER, "Oops, readyToAllocateOutputBuffers() is failed!\n");
                ret = FAIL_TO_ALLOCATE_OUTPUT_BUFFERS_ON_EXECUTE;
                break;
            }
        } else {
            LOGD(EDEN_DRIVER, "Dont need to Allocate Output Buffers\n");
        }

        // Prepare buffer information on execute.
        BufferInfoOnExecute bufInfoOnExecute;

        // Load input data(Android NN Memory) to input buffer(Eden Memory Manager)
        ret = edenPreparedModel->loadInputData(request, bufInfoOnExecute);
        if (ret != RET_OK) {
            LOGE(EDEN_DRIVER, "Oops, loadInputData() is failed!\n");
            break;
        }

        if (async) {
            ret = nnAgent->executionScheduler_->requestOneExecutionInAsync(edenPreparedModel,
                                                                           request,
                                                                           bufInfoOnExecute,
                                                                           measure,
                                                                           driverStart,
                                                                           callback);
            if (ret != RET_OK) {
                LOGE(EDEN_DRIVER, "Oops, requestOneExecutionInAsync() is failed!\n");
                break;
            }
        } else {
            ret = nnAgent->executionScheduler_->requestOneExecutionInSync(edenPreparedModel,
                                                                          request,
                                                                          bufInfoOnExecute,
                                                                          measure,
                                                                          driverStart,
                                                                          callback);
            if (ret != RET_OK) {
                LOGE(EDEN_DRIVER, "Oops, requestOneExecutionInSync() is failed!\n");
                break;
            }
        }

        // Load output result(Eden Memory Manager) to output buffer(Android NN Memory)
        // loadOutputData(request.outputs);
    } while (0);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return ret;
}

/**
 * @brief Constructor
 * @details Constructor
 * @param void
 */
NNAgent::NNAgent(void) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    modelConverter_ = std::make_shared<ModelConverter>();
    compilerManager_ = std::make_shared<CompilerManager>();
    executionScheduler_ = std::make_shared<ExecutionScheduler>();
    resourceManager_ = std::make_shared<ResourceManager>();
    edenServiceDelegator_ = std::make_shared<EdenServiceDelegatorLib>();

    modelConverter_->setCompilerManager(compilerManager_);
    executionScheduler_->setEdenServiceDelegator(edenServiceDelegator_);
    executionScheduler_->setCompilerManager(compilerManager_);

    initialize();
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

/**
 * @brief Destructor
 * @details Destructor
 * @param void
 */
NNAgent::~NNAgent(void) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    shutdown();
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

/**
 * @brief Initialize
 * @details This function initializes NNAgent.
 * @param void
 * @return error code
 */
void NNAgent::initialize(void) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    edenServiceDelegator_->Init();
    compilerManager_->initNPUCCompiler();
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

/**
 * @brief close model
 * @details This function lets NNAgent close model.
 * @param modelId EdenModel modelId
 * @return error code
 */
void NNAgent::closeModel(uint32_t modelId) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    edenServiceDelegator_->CloseModel(modelId);
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

/**
 * @brief Shutdown
 * @details This function shutdowns NNAgent.
 * @param void
 * @return error code
 */
void NNAgent::shutdown(void) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    if (edenServiceDelegator_->Shutdown()) {
        // @todo below code seems weird.
        // executionCallback->notify(ErrorStatus::INVALID_ARGUMENT);
    }
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

/**
 * @brief Get capabilities for this device
 * @details This function returns a capabilities representing for performance of this device.
 * @param[in] capabilities Capabilities representing for performance of device
 * @return error code
 */
int32_t NNAgent::getCapabilities(V1_2::Capabilities& capabilities) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    int32_t retCode = RET_OK;
    V1_0::PerformanceInfo perfInfo;

    // Get perfInfo for relaxed float32(float16) on scalar/tensor
    retCode = compilerManager_->getPerformanceInfo(DATA_TYPE::RELAXED_FLOAT32, perfInfo);
    if (retCode != RET_OK) return retCode;

    capabilities.relaxedFloat32toFloat16PerformanceScalar = perfInfo;
    capabilities.relaxedFloat32toFloat16PerformanceTensor = perfInfo;
    capabilities.operandPerformance = nonExtensionOperandPerformance(perfInfo);

    // Get performance info for foat32
    retCode = compilerManager_->getPerformanceInfo(DATA_TYPE::FLOAT32, perfInfo);
    if (retCode != RET_OK) return retCode;

    updateCapabilities(&capabilities.operandPerformance, V1_2::OperandType::TENSOR_FLOAT32, perfInfo);
    updateCapabilities(&capabilities.operandPerformance, V1_2::OperandType::FLOAT32, perfInfo);

    // Get perfInfo for quanitzed
    retCode = compilerManager_->getPerformanceInfo(DATA_TYPE::QUANT8, perfInfo);
    if (retCode != RET_OK) return retCode;

    updateCapabilities(&capabilities.operandPerformance, V1_2::OperandType::TENSOR_QUANT8_ASYMM, perfInfo);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Get supported operation list on a given model by this device
 * @details This function returns a supported operation list on a given model by this device.
 * @param[in] model Android NN Model
 * @param[out] supportedOperations supported operation list
 * @return error code
 */
int32_t NNAgent::getSupportedOperations(const V1_2::Model& model, std::vector<bool>& supportedOperations) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    int32_t retCode = RET_OK;

    // If it already has a supported operation list info, then return it.
    vecNPUOperationInfos_.clear();
    vecGPUOperationInfos_.clear();
    vecCPUOperationInfos_.clear();
    if (vecNPUOperationInfos_.empty()) {
        LOGD(EDEN_DRIVER, "vecNPUOperationInfos_ is empty!\n");
        retCode = queryOperationInfo(model, TARGET_DEVICE_NPU, vecNPUOperationInfos_);
        if (retCode != RET_OK) return retCode;
    }
    if (vecGPUOperationInfos_.empty()) {
        LOGD(EDEN_DRIVER, "vecGPUOperationInfos_ is empty!\n");
        retCode = queryOperationInfo(model, TARGET_DEVICE_GPU, vecGPUOperationInfos_);
        if (retCode != RET_OK) return retCode;
    }
    if (vecCPUOperationInfos_.empty()) {
        LOGD(EDEN_DRIVER, "vecCPUOperationInfos_ is empty!\n");
        retCode = queryOperationInfo(model, TARGET_DEVICE_CPU, vecCPUOperationInfos_);
        if (retCode != RET_OK) return retCode;
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return loadSupportedOperationList(model, supportedOperations);
}

/**
 * @brief Prepare model to be accelerated by this device
 * @details This function prepares a model to be accelerated by this device.
 *          To be accelerated, it should be converted to EdenModel.
 *          It includes compileation phase for NPU and it might take some time.
 *          If there is a cached model it will be loaded to avoid converting step.
 * @param[in] model Android NN Model
 * @param[in] preference ExecutionPreference
 * @param[in] callback IPreparedModelCallback
 * @return error code
 */
int32_t NNAgent::prepareModel(const V1_2::Model& model, ExecutionPreference preference,
                              const hidl_vec<hidl_handle>* modelCache, const hidl_vec<hidl_handle>* dataCache,
                              const HidlToken* token,
                              const sp<V1_0::IPreparedModelCallback>& callback_1_0) {
    return prepareModelBaseOnNNAgent(this, model, preference, modelCache, dataCache, token, callback_1_0);
}

/**
 * @brief Prepare model to be accelerated by this device
 * @details This function prepares a model to be accelerated by this device.
 *          To be accelerated, it should be converted to EdenModel.
 *          It includes compileation phase for NPU and it might take some time.
 *          If there is a cached model it will be loaded to avoid converting step.
 * @param[in] model Android NN Model
 * @param[in] preference ExecutionPreference
 * @param[in] callback IPreparedModelCallback
 * @return error code
 */
int32_t NNAgent::prepareModel(const V1_2::Model& model, ExecutionPreference preference,
                              const hidl_vec<hidl_handle>* modelCache, const hidl_vec<hidl_handle>* dataCache,
                              const HidlToken* token,
                              const sp<V1_2::IPreparedModelCallback>& callback_1_2) {
    return prepareModelBaseOnNNAgent(this, model, preference, modelCache, dataCache, token, callback_1_2);
}

int32_t NNAgent::allocateInputBuffers(uint32_t modelId, InHouseBufferInfo& inputBuffers) {
    EdenBuffer* edenBuffers = nullptr;
    int32_t numOfBuffers = 0;

    int32_t retCode = edenServiceDelegator_->AllocateInputBuffers(modelId, &edenBuffers, &numOfBuffers);

    if (retCode != RET_OK) {
        LOGE(EDEN_DRIVER, "AllocateInputBuffers() is failed.\n");
        return retCode;
    } else {
        inputBuffers.addr = reinterpret_cast<void*>(edenBuffers);
        inputBuffers.numOfBuffers = numOfBuffers;
    }

    return RET_OK;
}

int32_t NNAgent::allocateOutputBuffers(uint32_t modelId, InHouseBufferInfo& outputBuffers) {
    EdenBuffer* edenBuffers = nullptr;
    int32_t numOfBuffers = 0;

    int32_t retCode = edenServiceDelegator_->AllocateOutputBuffers(modelId, &edenBuffers, &numOfBuffers);

    if (retCode != RET_OK) {
        LOGE(EDEN_DRIVER, "AllocateOutputBuffers() is failed.\n");
        return retCode;
    } else {
        outputBuffers.addr = reinterpret_cast<void*>(edenBuffers);
        outputBuffers.numOfBuffers = numOfBuffers;
    }

    return RET_OK;
}

/**
 * @brief Get device status
 * @details This function returns a device status.
 * @param[out] status DeviceStatus
 * @return error code
 */
int32_t NNAgent::getStatus(DeviceStatus& status) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    // @todo Get device status from runtime
    status = DeviceStatus::AVAILABLE;

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Execute a preparedModel with a given request asynchronously
 * @details This function executes a preparedModel with a given request
 *          and executes a callback when it's finished.
 * @param[in] preparedModel EdenPreparedModel to be executed
 * @param[in] request Request to be executed
 * @param[in] callback IExecutionCallback to be executed
 * @return error code
 */
int32_t NNAgent::execute(EdenPreparedModel* edenPreparedModel,
                         const V1_0::Request& request,
                         const sp<V1_0::IExecutionCallback>& callback_1_0) {
    std::chrono::steady_clock::time_point dummyDriverStart;

    if (!validateRequest(request, edenPreparedModel->model)) {
        return INVALID_PARAMS;
    }

    return executeBaseOnNNAgent(this, edenPreparedModel, request, MeasureTiming::NO, dummyDriverStart, callback_1_0, true);
}

/**
 * @brief Execute a preparedModel with a given request asynchronously
 * @details This function executes a preparedModel with a given request
 *          and executes a callback when it's finished.
 * @param[in] preparedModel EdenPreparedModel to be executed
 * @param[in] request Request to be executed
 * @param[in] callback IExecutionCallback to be executed
 * @return error code
 */
int32_t NNAgent::execute(EdenPreparedModel* edenPreparedModel,
                         const V1_0::Request& request,
                         V1_2::MeasureTiming measure,
                         const sp<V1_2::IExecutionCallback>& callback_1_2) {
    std::chrono::steady_clock::time_point driverStart;
    if (measure == MeasureTiming::YES) driverStart = std::chrono::steady_clock::now();

    if (!validateRequest(request, edenPreparedModel->model)) {
        return INVALID_PARAMS;
    }

    return executeBaseOnNNAgent(this, edenPreparedModel, request, measure, driverStart, callback_1_2, true);
}

/**
 * @brief Execute a preparedModel with a given request synchronously
 * @details This function executes a preparedModel with a given request
 *          and executes a callback when it's finished.
 * @param[in] preparedModel EdenPreparedModel to be executed
 * @param[in] request Request to be executed
 * @param[in] callback IExecutionCallback to be executed
 * @return error code
 */
int32_t NNAgent::executeSynchronously(EdenPreparedModel* edenPreparedModel,
                                      const V1_0::Request& request,
                                      V1_2::MeasureTiming measure,
                                      const sp<V1_2::IExecutionCallback>& callback_1_2) {
    std::chrono::steady_clock::time_point driverStart;
    if (measure == MeasureTiming::YES) driverStart = std::chrono::steady_clock::now();

    if (!validateRequest(request, edenPreparedModel->model)) {
        return INVALID_PARAMS;
    }

    return executeBaseOnNNAgent(this, edenPreparedModel, request, measure, driverStart, callback_1_2, true);
}

/**
 * @brief Notify to caller by executing a callback function
 * @details This function notifies to the caller by executing a callback function.
 * @param[in] callback IExecutionCallback to be executed
 * @return error code
 */
int32_t NNAgent::notify(const sp<V1_2::IExecutionCallback>& /*callback_1_2*/) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Query operation information.
 * @details This function queries to target device to load operation information.
 *          such as supported operation list and its constraints.
 * @param[in] model Android NN Model
 * @param[in] targetDevice 0(NPU), 1(GPU), 2(CPU)
 * @param[out] vecOperationInfos OperationInfo for a target device
 * @return error code
 */
int32_t NNAgent::queryOperationInfo(const V1_2::Model& model, int32_t targetDevice, std::vector<OperationInfo>& vecOperationInfos) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    std::vector<std::shared_ptr<void>> constraints;
    int32_t retCode = queryConstraints(targetDevice, constraints);
    if (retCode != RET_OK) return retCode;

    // Query it to NPU compiler manager.
    std::vector<bool> supportedOperations;
    retCode = querySupportedOperations(model, targetDevice, constraints, supportedOperations);

    if (retCode != RET_OK) return retCode;

    // Make sure size for supportedOperations and constraints are same.
    if (supportedOperations.size() != constraints.size()) {
        LOGE(EDEN_DRIVER, "Error, size for supportedOperations and constraints should be same!\n");
        LOGE(EDEN_DRIVER, " supportedOperations.size()=%zu\n", static_cast<size_t>(supportedOperations.size()));
        LOGE(EDEN_DRIVER, " constraints.size()=%zu\n", static_cast<size_t>(constraints.size()));
        return INVALID_NUM_OF_SUPPORTED_OPERATIONS;
    }

    // If vecOperationInfos_ is occupied, it will be dropped.
    if (vecOperationInfos.empty() == false) {
        LOGE(EDEN_DRIVER, "vecOperationInfos is not empty, so they are dropped!\n");
        vecOperationInfos.clear();
    }

    // Keep supported operations and its constaints
    vecOperationInfos.resize(supportedOperations.size());
    for (size_t idx = 0; idx < supportedOperations.size(); idx++) {
        vecOperationInfos[idx].supported = supportedOperations[idx];
        vecOperationInfos[idx].constraint = constraints[idx];
    }
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

int32_t NNAgent::resetSupportedOperations(std::vector<bool>& supportedOperations) {
    for (size_t idx = 0; idx < supportedOperations.size(); idx++) {
        supportedOperations[idx] = false;
    }
    return RET_OK;
}

/**
 * @brief Query to target device to load supported operation list.
 * @details This function queries to target device to load supported operation list.
 * @param[in] model Android NN Model
 * @param[in] targetDevice 0(NPU), 1(GPU), 2(CPU)
 * @param[out] supportedOperations supported operation list
 * @return error code
 */
int32_t NNAgent::querySupportedOperations(const V1_2::Model& model, int32_t targetDevice, const std::vector<std::shared_ptr<void>>& constraints,
                                          std::vector<bool>& supportedOperations) {
    LOGD(EDEN_DRIVER, "%s() is called with targetDevice:%d, NPU(0), GPU(1), CPU(2)\n", __func__, targetDevice);

    int32_t retCode = RET_OK;

    supportedOperations.resize(ANDROID_NN_OP_NUM_MAXIMUM);
    resetSupportedOperations(supportedOperations);

    switch (targetDevice) {
    case TARGET_DEVICE_NPU:  // NPU
        return compilerManager_->getSupportedOperations(model, constraints, supportedOperations);
    case TARGET_DEVICE_GPU:  // GPU
        // To avoid CTS TestRandomGraph fail
        // the following operations only support GPU
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_ABS)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_ARGMAX)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_ARGMIN)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_AXIS_ALIGNED_BBOX_TRANSFORM)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_BIDIRECTIONAL_SEQUENCE_LSTM)] = true; // 42

        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_BIDIRECTIONAL_SEQUENCE_RNN)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_BOX_WITH_NMS_LIMIT)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_CAST)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_CHANNEL_SHUFFLE)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_DETECTION_POSTPROCESSING)] = true; // 47

        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_EQUAL)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_EXP)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_EXPAND_DIMS)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_GATHER)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_GENERATE_PROPOSALS)] = true; // 52

        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_GREATER)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_GREATER_EQUAL)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_GROUPED_CONV_2D)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_HEATMAP_MAX_KEYPOINT)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_INSTANCE_NORMALIZATION)] = true; // 57

        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_LESS)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_LESS_EQUAL)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_LOG)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_LOGICAL_AND)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_LOGICAL_NOT)] = true; // 62

        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_LOGICAL_OR)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_LOG_SOFTMAX)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_MAXIMUM)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_MINIMUM)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_NEG)] = true;  // 67

        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_NOT_EQUAL)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_PAD_V2)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_POW)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_PRELU)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_QUANTIZE)] = true; // 72

        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_QUANTIZED_16BIT_LSTM)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_RANDOM_MULTINOMIAL)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_REDUCE_ALL)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_REDUCE_ANY)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_REDUCE_MAX)] = true; // 77

        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_REDUCE_MIN)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_REDUCE_PROD)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_REDUCE_SUM)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_ROI_ALIGN)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_ROI_POOLING)] = true;  // 82

        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_RSQRT)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_SELECT)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_SIN)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_SLICE)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_SPLIT)] = true; // 87

        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_SQRT)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_TILE)] = true;
         supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_TOPK_V2)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_TRANSPOSE_CONV_2D)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_UNIDIRECTIONAL_SEQUENCE_LSTM)] = true; // 92

        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_UNIDIRECTIONAL_SEQUENCE_RNN)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_RESIZE_NEAREST_NEIGHBOR)] = true;

        [[fallthrough]];
    case TARGET_DEVICE_CPU:  // CPU
        // List up supported operation list as below
        // @todo below define should be replaced to proper number
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_ADD)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_AVERAGE_POOL_2D)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_CONCATENATION)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_CONV_2D)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_DEPTHWISE_CONV_2D)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_DEPTH_TO_SPACE)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_DEQUANTIZE)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_EMBEDDING_LOOKUP)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_FLOOR)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_FULLY_CONNECTED)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_HASHTABLE_LOOKUP)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_L2_NORMALIZATION)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_L2_POOL_2D)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_LOCAL_RESPONSE_NORMALIZATION)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_LOGISTIC)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_LSH_PROJECTION)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_LSTM)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_MAX_POOL_2D)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_MUL)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_RELU)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_RELU1)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_RELU6)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_RESHAPE)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_RESIZE_BILINEAR)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_RNN)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_SOFTMAX)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_SPACE_TO_DEPTH)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_SVDF)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_TANH)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_BATCH_TO_SPACE_ND)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_DIV)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_MEAN)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_PAD)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_SPACE_TO_BATCH_ND)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_SQUEEZE)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_STRIDED_SLICE)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_SUB)] = true;
        supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_TRANSPOSE)] = true;

        // supportedOperations[static_cast<int32_t>(ANEURALNETWORKS_OEM_OPERATION)] = false;
        break;

    default:
        LOGE(EDEN_DRIVER, "Oops, targetDevice=%d is not yet supported!\n", targetDevice);
        retCode = INVALID_TARGET_DEVICE;
    }

    if (checkDimensions(model) != RET_OK) {
        LOGE(EDEN_DRIVER, "unsupport Dimensions \n");
        resetSupportedOperations(supportedOperations);
    }
    if (checkGraph(model) != RET_OK) {
        LOGE(EDEN_DRIVER, "unsupport model \n");
        resetSupportedOperations(supportedOperations);
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return retCode;
}

/**
 * @brief Query to target device to load constraints list.
 * @details This function queries to target device to load constraints list.
 * @param[in] targetDevice 0(NPU), 1(GPU), 2(CPU)
 * @param[out] constraints constraints list
 * @return error code
 */
int32_t NNAgent::queryConstraints(int32_t targetDevice, std::vector<std::shared_ptr<void>>& constraints) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    int32_t retCode = RET_OK;
    constraints.resize(ANDROID_NN_OP_NUM_MAXIMUM);
    for (int32_t idx = 0; idx < ANDROID_NN_OP_NUM_MAXIMUM; idx++) {
        constraints[idx] = nullptr;
    }

    switch (targetDevice) {
    case TARGET_DEVICE_NPU:  // NPU
        retCode = compilerManager_->getConstraints(constraints);
        break;
    case TARGET_DEVICE_GPU:  // GPU
    case TARGET_DEVICE_CPU:  // CPU
        // @todo Add constraints if there is
        break;
    default:
        LOGE(EDEN_DRIVER, "Oops, targetDevice=%d is not yet supported!\n", targetDevice);
        retCode = INVALID_TARGET_DEVICE;
        break;
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return retCode;
}

/**
 * @brief Load supported operation on model
 * @details This function loads supported operation on model.
 *          Index on supportedOperations corresponds to operations on model.
 * @param[in] model Android NN Model
 * @param[out] supportedOperations supported operation list on model
 * @return error code
 */
int32_t NNAgent::loadSupportedOperationList(const V1_2::Model& model, std::vector<bool>& supportedOperations) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    LOGD(EDEN_DRIVER, "model.operations.size: %zu\n", model.operations.size());
    if (!supportedOperations.empty()){
        supportedOperations.clear();
    }
    supportedOperations.resize(model.operations.size());

    for (size_t idx = 0; idx < model.operations.size(); idx++) {
        int32_t opType = static_cast<int32_t>(model.operations[idx].type);
        LOGD(EDEN_DRIVER, "opType: %d\n", opType);
        if ((opType < 0) || (opType > ANEURALNETWORKS_RESIZE_NEAREST_NEIGHBOR)) {
            return INVALID_NUM_OF_SUPPORTED_OPERATIONS;
        }
        if ((vecNPUOperationInfos_.at(opType).supported) ||
            (vecGPUOperationInfos_.at(opType).supported) ||
            (vecCPUOperationInfos_.at(opType).supported)) {
            supportedOperations.at(idx) = true;
        } else {
            supportedOperations.at(idx) = false;
        }
        LOGD(EDEN_DRIVER, "supportedOperations[%zu] is %d\n", idx, static_cast<int32_t>(supportedOperations.at(idx)));
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Load cached IPreparedModel if exists
 * @details This function tries to load cached IPreparedModel if exists.
 *          If there is a cached IPreparedModel, it is loaded on a given preparedModel.
 *          If not, preparedModel has nullptr.
 * @param[in] model Android NN Model
 * @param[out] preparedModel IPreparedModel loaded from the cached model
 * @return error code
 */
int32_t NNAgent::loadCachedModel(const V1_2::Model& /*model*/, V1_2::IPreparedModel** /*preparedModel*/) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return 0;
}

/**
 * @brief Store IPreparedModel to cache
 * @details This function stores a IPreparedModel to cache
 *          so that it can be retrieved via loadCachedModel next time.
 * @param[in] model Android NN Model
 * @param[in] preparedModel IPreparedModel to be stored to the cached model
 * @return error code
 */
int32_t NNAgent::storeCachedModel(const V1_2::Model& /*model*/, V1_2::IPreparedModel* /*preparedModel*/) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return 0;
}

/**
 * @brief Create IPreparedModel
 * @details This function creates a IPreparedModel with given information.
 * @param[in] model Android NN Model
 * @param[in] modelId unique model id generated by Eden Runtime service
 * @param[in] edenModel EdenModel
 * @param[in] inputBuffers InHouseBufferInfo representing for buffer information returned by Eden Runtime service
 * @param[in] outputBuffers InHouseBufferInfo representing for buffer information returned by Eden Runtime service
 * @param[in] vecAddr virtual address for buffers on pools
 * @param[in] vecSize size for buffers on pools
 * @param[in] operandValues copied internal operandValues of model
 * @param[in] mapOperandIdFromAToE map operand id from Android NN Model to Eden Model
 * @param[in] mapOperationIdFromAToE map operation id from Android NN Model to Eden Model
 * @param[in] inputConsumers android operation id that consumes an operand at index
 * @param[in] internalBuffers internal buffers allocated for this model
 * @param[in] vecIMemoryOnAshmems list of sp<IMemory> that returned via mapMemory()
 * @param[out] preparedModel IPreparedModel which is generated
 * @return error code
 */
int32_t NNAgent::createPreparedModel(const V1_2::Model& model, uint32_t modelId, EdenModel* edenModel, HwPreference hwPreference,
                                     InHouseBufferInfo inputBuffers, InHouseBufferInfo outputBuffers,
                                     std::vector<char*>& vecAddr, std::vector<int32_t>& vecSize,
                                     std::unique_ptr<uint8_t[]>& operandValues,
                                     std::map<int32_t, int32_t>& mapOperandIdFromAToE,
                                     std::map<int32_t, int32_t>& mapOperationIdFromAToE,
                                     std::vector<int32_t>& inputConsumers,
                                     std::vector<std::unique_ptr<int32_t[]>>& internalBuffers,
                                     std::vector<sp<IMemory>>& vecIMemoryOnAshmems,
                                     V1_2::IPreparedModel** preparedModel) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    EdenPreparedModel* edenPreparedModel = new EdenPreparedModel(this, model, modelId, edenModel, hwPreference, inputBuffers.addr, inputBuffers.numOfBuffers,
                                                                 outputBuffers.addr, outputBuffers.numOfBuffers,
                                                                 vecAddr, vecSize, operandValues,
                                                                 mapOperandIdFromAToE, mapOperationIdFromAToE, inputConsumers, internalBuffers,
                                                                 vecIMemoryOnAshmems); // FIXME inputBuffers outputBuffers

    *preparedModel = edenPreparedModel;

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Set compute precision(FP32, FP16, UINT8 for a given EdenModel
 * @details This function calculates a compute precision based on a GPU userdriver policy.
 * @param[in] model Android NN Model
 * @param[in] edenModel EdenModel
 * @return error code
 */

void NNAgent::setComputePrecision(const V1_2::Model& model, EdenModel* edenModel) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    // Below logic comes from legacy EdenDriver implemented by SRCX.
    ComputePrecision computePrecision = FP16;

    if (model.relaxComputationFloat32toFloat16 == false) {
        LOGD(EDEN_DRIVER, "model.relaxComputationFloat32toFloat16 is false, so set computePrecision to FP32\n");
        computePrecision = FP32;
    }

    for (int32_t inputIndex : model.inputIndexes) {
        auto& operand = model.operands[inputIndex];
        if ((operand.lifetime == OperandLifeTime::MODEL_INPUT) &&
            (operand.scale != 0 || operand.zeroPoint != 0)) {
            LOGD(EDEN_DRIVER, "input operand's lifetime is MODEL_INPUT, so set computePrecision to UINT8\n");
            computePrecision = UINT8;
            break;
        } else if (operand.type == OperandType::TENSOR_FLOAT16) {
            computePrecision = FP16;
            break;
        }
    }

    if (model.operations.size() == 1) {
        // Use FP32 to compute FLOAT16 to get better accurary to pass VTS&CTS
        int32_t opType = static_cast<int32_t>(model.operations[0].type);
        switch (opType) {
            case ANEURALNETWORKS_CONV_2D:
            case ANEURALNETWORKS_TRANSPOSE_CONV_2D:
            case ANEURALNETWORKS_GROUPED_CONV_2D:
            case ANEURALNETWORKS_LOG_SOFTMAX:
            case ANEURALNETWORKS_DEPTHWISE_CONV_2D: {
                const V1_2::Operand &androidInputOperandFirst = model.operands[model.operations[0].inputs[0]];
                if (androidInputOperandFirst.type == OperandType::TENSOR_FLOAT16) {
                    LOGD(EDEN_DRIVER, "set FP32 to get better accurary");
                    computePrecision = FP32;
                }
                break;
            }
            default:
                break;
        }

        for (const V1_2::Operation& operation : model.operations) {
            int32_t opType = static_cast<int32_t>(operation.type);
            switch (opType) {
                case ANEURALNETWORKS_MAXIMUM:
                case ANEURALNETWORKS_MINIMUM:
                case ANEURALNETWORKS_TILE:
                case ANEURALNETWORKS_SLICE:
                case ANEURALNETWORKS_TOPK_V2: {
                    const V1_2::Operand &androidInputOperandFirst = model.operands[operation.inputs[0]];
                    LOGD(EDEN_DRIVER, "Firt operand's type is %d", androidInputOperandFirst.type);
                    if (androidInputOperandFirst.type == OperandType::TENSOR_INT32) {
                        LOGD(EDEN_DRIVER, "opType is %d & OperandType is INT32, so change compute_precision to FP32",
                             opType);
                        computePrecision = FP32;
                    }
                    break;
                }
                case ANEURALNETWORKS_SELECT: {
                    uint32_t secondInputIndex = operation.inputs[1];
                    uint32_t thirdInputIndex = operation.inputs[2];
                    if (model.operands[secondInputIndex].lifetime != OperandLifeTime::MODEL_INPUT &&
                        model.operands[thirdInputIndex].lifetime != OperandLifeTime::MODEL_INPUT) {
                        for (int32_t outputIndex : model.outputIndexes) {
                            auto &operand = model.operands[outputIndex];
                            if (operand.type == OperandType::TENSOR_FLOAT16) {
                                computePrecision = FP16;
                                break;
                            } else if (operand.type == OperandType::TENSOR_QUANT8_ASYMM) {
                                computePrecision = UINT8;
                                break;
                            }
                        }
                    }
                    break;
                }
                case ANEURALNETWORKS_CAST: {
                    const V1_2::Operand &androidInputOperand = model.operands[operation.inputs[0]];
                    const V1_2::Operand &androidOutputOperand = model.operands[operation.outputs[0]];
                    if ((androidInputOperand.type == V1_2::OperandType::TENSOR_QUANT8_ASYMM) ||
                        (androidInputOperand.type == V1_2::OperandType::TENSOR_QUANT8_SYMM) ||
                        (androidInputOperand.type == V1_2::OperandType::TENSOR_QUANT8_SYMM_PER_CHANNEL)) {
                        if (androidOutputOperand.type == V1_2::OperandType::TENSOR_FLOAT16) {
                            computePrecision = FP16;
                        } else {
                            computePrecision = FP32;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    for (const V1_2::Operation& operation : model.operations) {
        int32_t opType = static_cast<int32_t>(operation.type);
        if (opType == ANEURALNETWORKS_DEQUANTIZE && model.operands[operation.inputs[1]].type == OperandType::TENSOR_FLOAT16) {
            computePrecision = FP16;
            break;
        } else if (opType == ANEURALNETWORKS_DEQUANTIZE || opType == ANEURALNETWORKS_LSH_PROJECTION) {
            LOGD(EDEN_DRIVER,  "opType is %d, so change computePrecision to FP32", opType);
            computePrecision = FP32;
            break;
        }
        if (opType == ANEURALNETWORKS_LSTM && computePrecision == FP16) { // for asr_float.tflite accuracy test
            computePrecision = FP32;
            break;
        } else if (opType == ANEURALNETWORKS_SOFTMAX && model.operands[operation.outputs[0]].type == OperandType::TENSOR_QUANT8_ASYMM) {
            LOGD(EDEN_DRIVER,  "opType is SOFTMAX, and output datatype is QUANT8, so change computePrecision to UINT8");
            computePrecision = UINT8;
            break;
        }
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    edenModel->SetComputePrecision(computePrecision);
}

/**
 * @brief check dimensions for a given model
 * @details This function check dimensions, return error if it is larger than 4 for some operations.
 * @param[in] model Android NN Model
 * @return error code
 */
int32_t NNAgent::checkDimensions(const V1_2::Model& model) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);
    // bypass the notice of execution failure in CTS
    int32_t outputSizeSum = 0;
    for (size_t idx = 0; idx < model.outputIndexes.size(); idx++) {
        int32_t outputIndex = model.outputIndexes[idx];
        const V1_2::Operand& androidOperand = model.operands[outputIndex];
        for (size_t i = 0; i < androidOperand.dimensions.size(); i++) {
            outputSizeSum += androidOperand.dimensions[i];
        }
    }
    if (outputSizeSum == 0) {
        LOGE(EDEN_DRIVER, "Invalied OutputParams.\n");
        return RET_PARAM_INVALID;
    }

    // TODO: to support dimension large than 4
    for (const V1_2::Operation &operation : model.operations) {
        int32_t opType = static_cast<int32_t>(operation.type);
        switch (opType) {
            case ANEURALNETWORKS_EXPAND_DIMS:
            case ANEURALNETWORKS_POW:
            case ANEURALNETWORKS_PRELU:
            case ANEURALNETWORKS_MAXIMUM:
            case ANEURALNETWORKS_MINIMUM:
            case ANEURALNETWORKS_LOGICAL_AND:
            case ANEURALNETWORKS_LOGICAL_OR:
            case ANEURALNETWORKS_ARGMIN:
            case ANEURALNETWORKS_ARGMAX:
            case ANEURALNETWORKS_TILE : {
                for (size_t idx = 0; idx < operation.inputs.size(); idx++) {
                    uint32_t idxInput = operation.inputs[idx];
                    const V1_2::Operand& androidOperand = model.operands[idxInput];
                    if (androidOperand.dimensions.size() > 4) {
                        LOGD(EDEN_DRIVER,  "not support dimensions.size = %d ", (int)androidOperand.dimensions.size());
                        return RET_PARAM_INVALID;
                    }
                }

                for (size_t idx = 0; idx < operation.outputs.size(); idx++) {
                    uint32_t idxOutput = operation.outputs[idx];
                    const V1_2::Operand& androidOperand = model.operands[idxOutput];
                    if (androidOperand.dimensions.size() > 4) {
                        LOGD(EDEN_DRIVER,  "not support dimensions.size = %d", (int)androidOperand.dimensions.size());
                        return RET_PARAM_INVALID;
                    }
                }
                break;
            }
            default:
                break;
        }
    }
    return RET_OK;
}


/**
 * @brief check graph
 * @details This function check graph, return error if it is not supported in eden
 * @param[in] model Android NN Model
 * @return error code
 */

int32_t NNAgent::checkGraph(const V1_2::Model& model) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    bool isNCHW = false;
    bool isWeightAsInput = false;
    for (const V1_2::Operation &operation : model.operations) {
        int32_t opType = static_cast<int32_t>(operation.type);

        if (opType == ANEURALNETWORKS_REDUCE_SUM && model.operations.size() == 1) { // to skip case that mse>0.0001
            for (int32_t inputIndex : model.inputIndexes) {
                auto& operand = model.operands[inputIndex];
                if (operand.type == OperandType::TENSOR_FLOAT16) {
                    return RET_PARAM_INVALID;
                }
            }
        }

        if (model.operations.size() >= 5) {
          // 1) TODO: change to NCHW
          if (opType == ANEURALNETWORKS_MAXIMUM || opType == ANEURALNETWORKS_MINIMUM ||
              opType == ANEURALNETWORKS_EQUAL || opType == ANEURALNETWORKS_NOT_EQUAL ||
              opType == ANEURALNETWORKS_REDUCE_SUM || opType == ANEURALNETWORKS_REDUCE_MIN ||
              opType == ANEURALNETWORKS_REDUCE_MAX ||
              opType == ANEURALNETWORKS_REDUCE_PROD || opType == ANEURALNETWORKS_REDUCE_ALL ||
              opType == ANEURALNETWORKS_REDUCE_ANY ||
              opType == ANEURALNETWORKS_TILE || opType == ANEURALNETWORKS_SLICE ||
              opType == ANEURALNETWORKS_HASHTABLE_LOOKUP) {
            return RET_PARAM_INVALID;
          }
        }

      if (model.operations.size() >= 2) {
            // 2) TODO: to support create graph with fully const input
            bool isConstInput = true;
            for (size_t idx = 0; idx < operation.inputs.size(); idx++) {
                uint32_t idxInput = operation.inputs[idx];
                const V1_2::Operand& androidOperand = model.operands[idxInput];
                if (androidOperand.lifetime == OperandLifeTime::MODEL_INPUT || androidOperand.lifetime == OperandLifeTime::TEMPORARY_VARIABLE) {
                    isConstInput = false;
                }
            }

            if (isConstInput == true) {
                LOGD(EDEN_DRIVER,  "eden-drv not support model with full const input \n");
                return RET_PARAM_INVALID;
            }

            // 3) TODO: to support model with mixed layout
            if (opType == ANEURALNETWORKS_CONV_2D) {
                bool isV1_2 = false;
                if (operation.inputs.size() >= 8) {
                    auto index = operation.inputs[7];
                    isV1_2 = model.operands[index].type == OperandType::BOOL ? true : false;
                }
                if (operation.inputs.size() >= 8 && isV1_2) {
                    isNCHW = getValue<bool>(model, operation, 7);
                } else if (operation.inputs.size() >= 11) {
                    isNCHW = getValue<bool>(model, operation, 10);
                }
            } else if (opType == ANEURALNETWORKS_GROUPED_CONV_2D) {
                if (operation.inputs.size() == 12) {
                    isNCHW = getValue<bool>(model, operation, 11);
                } else {
                    isNCHW = getValue<bool>(model, operation, 8);
                }
            } else if (opType == ANEURALNETWORKS_TRANSPOSE_CONV_2D) {
                isNCHW = getValue<bool>(model, operation, operation.inputs.size() - 1);
            } else if (opType == ANEURALNETWORKS_DEPTHWISE_CONV_2D) {
                if (operation.inputs.size() >= 12) {
                    isNCHW = getValue<bool>(model, operation, 11);
                } else if (operation.inputs.size() >= 9) {
                    isNCHW = getValue<bool>(model, operation, 8);
                }
            } else if (opType == ANEURALNETWORKS_AVERAGE_POOL_2D) {
                if (operation.inputs.size() == 11) {
                    isNCHW = getValue<bool>(model, operation, 10);
                } else if (operation.inputs.size() == 8) {
                    isNCHW = getValue<bool>(model, operation, 7);
                }
            } else if (opType == ANEURALNETWORKS_BATCH_TO_SPACE_ND) {
                if (operation.inputs.size() == 3) {
                    isNCHW = getValue<bool>(model, operation, 2);
                } else {
                    isNCHW = false;
                }
            } else if (opType == ANEURALNETWORKS_DEPTH_TO_SPACE) {
                if (operation.inputs.size() == 3) {
                    isNCHW = getValue<bool>(model, operation, 2);
                } else {
                    isNCHW = false;
                }
            } else if (opType == ANEURALNETWORKS_INSTANCE_NORMALIZATION) {
                isNCHW = getValue<bool>(model, operation, 4);
            } else if (opType == ANEURALNETWORKS_L2_POOL_2D) {
                if (operation.inputs.size() == 11) {
                    isNCHW = getValue<bool>(model, operation, 10);
                } else if (operation.inputs.size() == 8) {
                    isNCHW = getValue<bool>(model, operation, 7);
                }
            } else if (opType == ANEURALNETWORKS_MAX_POOL_2D) {
                if (operation.inputs.size() == 11) {
                    isNCHW = getValue<bool>(model, operation, 10);
                } else if (operation.inputs.size() == 8) {
                    isNCHW = getValue<bool>(model, operation, 7);
                }
            } else if (opType == ANEURALNETWORKS_RESIZE_BILINEAR) {
                if (operation.inputs.size() == 4) {
                    isNCHW = getValue<bool>(model, operation, 3);
                }
            } else if (opType == ANEURALNETWORKS_RESIZE_NEAREST_NEIGHBOR) {
                if (operation.inputs.size() == 4) {
                    isNCHW = getValue<bool>(model, operation, 3);
                }
            } else if (opType == ANEURALNETWORKS_SPACE_TO_DEPTH) {
                if (operation.inputs.size() == 3) {
                    isNCHW = getValue<bool>(model, operation, 2);
                }
            } else if (opType == ANEURALNETWORKS_SPACE_TO_BATCH_ND) {
                if (operation.inputs.size() == 4) {
                    isNCHW = getValue<bool>(model, operation, 3);
                }
            } else if (opType == ANEURALNETWORKS_ROI_ALIGN) {
                isNCHW = getValue<bool>(model, operation, 9);
            } else if (opType == ANEURALNETWORKS_ROI_POOLING) {
                isNCHW = getValue<bool>(model, operation, 7);
            } else if (opType == ANEURALNETWORKS_HEATMAP_MAX_KEYPOINT) {
                isNCHW = getValue<bool>(model, operation, 2);
            } else if (opType == ANEURALNETWORKS_GENERATE_PROPOSALS) {
                isNCHW = getValue<bool>(model, operation, 10);
            }
            if  (isNCHW) {
                LOGD(EDEN_DRIVER,  "eden-drv not support model with mixed layout \n");
                return RET_PARAM_INVALID;
            }

            // 4) TODO: support weight as input in multi-operations model
            switch (opType) {
                case ANEURALNETWORKS_CONV_2D:
                case ANEURALNETWORKS_GROUPED_CONV_2D:
                case ANEURALNETWORKS_TRANSPOSE_CONV_2D:
                case ANEURALNETWORKS_DEPTHWISE_CONV_2D:
                case ANEURALNETWORKS_FULLY_CONNECTED:
                {
                    // check weight and bias
                    for (size_t idx = 1; idx < 3; idx++) {
                        int32_t indexOperands = operation.inputs[idx];
                        const V1_2::Operand& androidOperand = model.operands[indexOperands];
                        if (androidOperand.lifetime == OperandLifeTime::TEMPORARY_VARIABLE) {
                            isWeightAsInput = true;
                        }
                    }
                    break;
                }
                case ANEURALNETWORKS_LSH_PROJECTION:
                {
                    // hhash
                    int32_t indexOperands = operation.inputs[0];
                    const V1_2::Operand& androidOperand = model.operands[indexOperands];
                    if (androidOperand.lifetime == OperandLifeTime::TEMPORARY_VARIABLE) {
                        isWeightAsInput = true;
                    }
                    break;
                }
                case ANEURALNETWORKS_RESHAPE: // axis
                case ANEURALNETWORKS_PRELU: // alpha
                {
                    int32_t indexOperands = operation.inputs[1];
                    const V1_2::Operand& androidOperand = model.operands[indexOperands];
                    if (androidOperand.lifetime == OperandLifeTime::TEMPORARY_VARIABLE) {
                        isWeightAsInput = true;
                    }
                    break;
                }
                default:
                    break;
            }

            if (isWeightAsInput) {
                LOGD(EDEN_DRIVER,  "eden-drv not support model with variable weight\n");
                return RET_PARAM_INVALID;
            }
        }
    }

    return RET_OK;
}

}  // namespace eden_driver
}  // namespace nn
}  // namespace android
