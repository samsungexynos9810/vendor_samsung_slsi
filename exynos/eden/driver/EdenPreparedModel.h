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
 * @file    EdenPreparedModel.h
 * @brief   This is EdenPreparedModel class file.
 * @details This header implements IPreparedModel interface.
 * @author  minsu.jeon (minsu.jeon@samsung.com)
 */

#ifndef DRIVER_EDENPREPAREDMODEL_H_
#define DRIVER_EDENPREPAREDMODEL_H_

#include <map>
#include <vector>
#include <cstdint>  // int32_t

#include "HalInterfaces.h"  // IDevice, Return, ErrorStatus, IPreparedModelCallback, getCapabilities_cb etc
#include <ui/GraphicBuffer.h>

#include "../include/eden_types.h"  // HwPreference
#include "BufferInfoOnExecute.h"

namespace eden {
namespace nn {
class EdenModel;
}
}

namespace android {
namespace nn {
namespace eden_driver {

using ::android::hardware::MQDescriptorSync;

class NNAgent;

class EdenPreparedModel : public IPreparedModel {
 public:
    explicit EdenPreparedModel(NNAgent* nnAgent,
                               const V1_2::Model& model, uint32_t modelId, eden::nn::EdenModel* edenModel,
                               HwPreference hwPreference,
                               void* inputAddr, int32_t inputNumOfBuffers,
                               void* outputAddr, int32_t outputNumOfBuffers,
                               std::vector<char*>& vecMappedAddr, std::vector<int32_t>& vecMappedSize,
                               std::unique_ptr<uint8_t[]>& operandValues,
                               std::map<int32_t, int32_t>& mapOperandIdFromAToE,
                               std::map<int32_t, int32_t>& mapOperationIdFromAToE,
                               std::vector<int32_t>& inputConsumers,
                               std::vector<std::unique_ptr<int32_t[]>>& internalBuffers,
                               std::vector<sp<IMemory>>& vecIMemoryOnAshmems);
    ~EdenPreparedModel() override;

    // IPreparedModel@1.0
    Return<ErrorStatus> execute(const V1_0::Request& request, const sp<V1_0::IExecutionCallback>& callback) override;

    // IPreparedModel@1.2
    Return<ErrorStatus> execute_1_2(const V1_0::Request& request,
                                    V1_2::MeasureTiming measure,
                                    const sp<V1_2::IExecutionCallback>& callback) override;
    Return<void> executeSynchronously(const V1_0::Request& request,
                                      V1_2::MeasureTiming measure,
                                      executeSynchronously_cb cb) override;
    Return<void> configureExecutionBurst(const sp<V1_2::IBurstCallback>& callback,
                                         const MQDescriptorSync<V1_2::FmqRequestDatum>& requestChannel,
                                         const MQDescriptorSync<V1_2::FmqResultDatum>& resultChannel,
                                         configureExecutionBurst_cb _hidl_cb) override;

    // Internal public functions
    int32_t loadInputData(const V1_0::Request& request, BufferInfoOnExecute& bufInfoOnExecute);
    int32_t loadOutputData(const V1_0::Request& request, BufferInfoOnExecute& bufInfoOnExecute);

    int32_t resolveUnknownDimensions(const V1_0::Request& request);

    // Setter
    void updateInputBuffers(void* addr, int32_t numOfBuffers);
    void updateOutputBuffers(void* addr, int32_t numOfBuffers);

    // Getter
    bool readyToAllocateInputBuffers(const V1_0::Request* request);
    bool readyToAllocateOutputBuffers(const V1_0::Request* request);

    bool needToAllocateInputBuffers() { return needInputBuffers_; }
    bool needToAllocateOutputBuffers() { return needOutputBuffers_; }

    V1_2::Model model;

    uint32_t modelId;
    HwPreference hwPreference;
    void* inputAddr;  // EdenBuffer*
    int32_t inputNumOfBuffers;
    void* outputAddr;  // EdenBuffer*
    int32_t outputNumOfBuffers;

    std::vector<int32_t> updatedOperations;

 private:
    int32_t loadDefaultDataOnTargetBuffer(int32_t inputIdx);
    int32_t loadDataOnTargetBuffer(char* srcAddr, int32_t length, int32_t inputIdx);

    void show();

    NNAgent* nnAgent_;
    eden::nn::EdenModel* edenModel_;  // @todo This might be removed in future.

    std::unique_ptr<uint8_t[]> operandValues_;

    std::vector<char*> vecMappedAddr_;
    std::vector<int32_t> vecMappedSize_;

    // Mapping information
    std::map<int32_t, int32_t> mapOperandIdFromAToE_;
    std::map<int32_t, int32_t> mapOperationIdFromAToE_;
    std::vector<int32_t> inputConsumers_;
    std::vector<std::unique_ptr<int32_t[]>> internalBuffers_;
    std::vector<sp<IMemory>> vecIMemoryOnAshmems_;

    bool needInputBuffers_;
    bool needOutputBuffers_;
};

}  // namespace eden_driver
}  // namespace nn
}  // namespace android

#endif  // DRIVER_EDENPREPAREDMODEL_H_

