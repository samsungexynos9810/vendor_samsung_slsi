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
 * @file    NNAgent.h
 * @brief   This is NNAgent class file.
 * @details This header defines NNAgent class.
 *          This class is implementing NNAgent which is Facade.
 * @author  minsu.jeon (minsu.jeon@samsung.com)
 */

#ifndef DRIVER_NNAGENT_H_
#define DRIVER_NNAGENT_H_

#include <iostream>
#include <vector>
#include <map>
#include <cstdint>  // int32_t
#include <memory>   // shared_ptr

#include "HalInterfaces.h"  // IDevice, Return, ErrorStatus, IPreparedModelCallback, getCapabilities_cb etc

#include "EdenPreparedModel.h"
#include "Common.h"  // OperationInfo

class ModelConverter;
class CompilerManager;
class ExecutionScheduler;
class ResourceManager;
class EdenServiceDelegator;

namespace eden {
namespace nn {
class EdenModel;
}
}

namespace android {
namespace nn {
namespace eden_driver {

using HidlToken = hidl_array<uint8_t, ANEURALNETWORKS_BYTE_SIZE_OF_CACHE_TOKEN>;
using executeSynchronously_cb = android::hardware::neuralnetworks::V1_2::IPreparedModel::executeSynchronously_cb;

typedef struct __InHouseBufferInfo {
    void* addr;
    int32_t numOfBuffers;
} InHouseBufferInfo;

class NNAgent {
 public:
    explicit NNAgent();
    ~NNAgent();

    void initialize();
    void closeModel(uint32_t modelId);
    void shutdown();

    int32_t getCapabilities(V1_2::Capabilities& capabilities);
    int32_t getSupportedOperations(const V1_2::Model& model, std::vector<bool>& supportedOperations);

    int32_t prepareModel(const V1_2::Model& model, ExecutionPreference preference,
                         const hidl_vec<hidl_handle>* modelCache, const hidl_vec<hidl_handle>* dataCache,
                         const HidlToken* token,
                         const sp<V1_0::IPreparedModelCallback>& callback_1_0);
    int32_t prepareModel(const V1_2::Model& model, ExecutionPreference preference,
                         const hidl_vec<hidl_handle>* modelCache, const hidl_vec<hidl_handle>* dataCache,
                         const HidlToken* token,
                         const sp<V1_2::IPreparedModelCallback>& callback_1_2);

    int32_t allocateInputBuffers(uint32_t modelId, InHouseBufferInfo& inputBuffers);
    int32_t allocateOutputBuffers(uint32_t modelId, InHouseBufferInfo& outputBuffers);

    int32_t getStatus(DeviceStatus& status);

    int32_t execute(EdenPreparedModel* edenPreparedModel,
                    const V1_0::Request& request,
                    const sp<V1_0::IExecutionCallback>& callback_1_0);
    int32_t execute(EdenPreparedModel* edenPreparedModel,
                    const V1_0::Request& request,
                    V1_2::MeasureTiming measure,
                    const sp<V1_2::IExecutionCallback>& callback_1_2);
    int32_t executeSynchronously(EdenPreparedModel* edenPreparedModel,
                                 const V1_0::Request& request,
                                 V1_2::MeasureTiming measure,
                                 const sp<V1_2::IExecutionCallback>& callback_1_2);

    int32_t notify(const sp<V1_2::IExecutionCallback>& callback_1_2);

    // Getter
    eden::nn::EdenModel* getEdenModel(uint32_t modelId);

 private:
    int32_t queryOperationInfo(const V1_2::Model& model, int32_t targetDevice,
                               std::vector<OperationInfo>& vecOperationInfos);
    int32_t resetSupportedOperations(std::vector<bool>& supportedOperations);
    int32_t querySupportedOperations(const V1_2::Model& model, int32_t targetDevice,
                                     const std::vector<std::shared_ptr<void>>& constraints,
                                     std::vector<bool>& supportedOperations);
    int32_t queryConstraints(int32_t targetDevice, std::vector<std::shared_ptr<void>>& constraints);

    int32_t loadSupportedOperationList(const V1_2::Model& model, std::vector<bool>& supportedOperations);

    int32_t loadCachedModel(const V1_2::Model& model, V1_2::IPreparedModel** preparedModel);
    int32_t storeCachedModel(const V1_2::Model& model, V1_2::IPreparedModel* preparedModel);
    int32_t createPreparedModel(const V1_2::Model& model, uint32_t modelId, eden::nn::EdenModel* edenModel,
                                HwPreference hwPreference,
                                InHouseBufferInfo inputBuffers, InHouseBufferInfo outputBuffers,
                                std::vector<char*>& vecAddr, std::vector<int32_t>& vecSize,
                                std::unique_ptr<uint8_t[]>& operandValues,
                                std::map<int32_t, int32_t>& mapOperandIdFromAToE,
                                std::map<int32_t, int32_t>& mapOperationIdFromAToE,
                                std::vector<int32_t>& inputConsumers,
                                std::vector<std::unique_ptr<int32_t[]>>& internalBuffers,
                                std::vector<sp<IMemory>>& vecIMemoryOnAshmems,
                                V1_2::IPreparedModel** preparedModel);

    void setComputePrecision(const V1_2::Model& model, eden::nn::EdenModel* edenModel);

    int32_t checkDimensions(const V1_2::Model& model);
    int32_t checkGraph(const V1_2::Model& model);

    std::shared_ptr<ModelConverter> modelConverter_;
    std::shared_ptr<CompilerManager> compilerManager_;
    std::shared_ptr<ExecutionScheduler> executionScheduler_;
    std::shared_ptr<ResourceManager> resourceManager_;
    std::shared_ptr<EdenServiceDelegator> edenServiceDelegator_;

    std::vector<OperationInfo> vecNPUOperationInfos_;
    std::vector<OperationInfo> vecGPUOperationInfos_;
    std::vector<OperationInfo> vecCPUOperationInfos_;

    std::map<uint32_t, eden::nn::EdenModel*> mapModelIdToModel_;

    template <typename T_Callback>
    friend int32_t prepareModelBaseOnNNAgent(NNAgent* nnAgent,
                                             const V1_2::Model& model, ExecutionPreference preference,
                                             const hidl_vec<hidl_handle>* modelCache, const hidl_vec<hidl_handle>* dataCache,
                                             const HidlToken* token,
                                             const T_Callback& callback_1_2);

    template <typename T_Callback>
    friend int32_t executeBaseOnNNAgent(NNAgent* nnAgent,
                                        EdenPreparedModel* edenPreparedModel,
                                        const V1_0::Request& request,
                                        V1_2::MeasureTiming measure,
                                        std::chrono::steady_clock::time_point driverStart,
                                        const T_Callback& callback,
                                        bool async);
};

}  // namespace eden_driver
}  // namespace nn
}  // namespace android

#endif  // DRIVER_NNAGENT_H_

