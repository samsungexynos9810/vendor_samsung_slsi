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
 * @file    ModelConverter.cpp
 * @brief   This is ModelConverter class file.
 * @details This header defines ModelConverter class.
 *          This class is implementing model converting from Android NN Model to EdenModel.
 * @author  minsu.jeon (minsu.jeon@samsung.com)
 */

#include <iostream>
#include "log.h"
#include "Utils.h"               // convertToV1_0, convertToV1_1, android::nn::initVLogMask, logModelToInfo, DRIVER

#include "NeuralNetworks.h"  // Operation, Operand, ANEURALNETWORKS_ADD etc
#include "Common.h"   // DATA_TYPE, OperationInfo

#include "ModelConverter.h"
#include "CompilerManager.h"
#include "PrePostProcessor.h"
#include "MyUtils.h"    // DumpToStdio

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EdenDriver::ModelConverter"

using namespace eden::nn;

namespace android {
namespace nn {
namespace eden_driver {

void showModel(const V1_2::Model& model) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    int numOfOperand = model.operands.size();
    LOGD(EDEN_DRIVER, "model.operands.size()=%d\n", numOfOperand);

    int numOfOperations = model.operations.size();
    LOGD(EDEN_DRIVER, "model.operations.size()=%d\n", numOfOperations);

    int numOfInputIndexes = model.inputIndexes.size();
    LOGD(EDEN_DRIVER, "model.inputIndexes.size()=%d\n", numOfInputIndexes);
    for (int i = 0; i < numOfInputIndexes; i++) {
        LOGD(EDEN_DRIVER, "    %d\n", model.inputIndexes[i]);
    }

    int numOfOutputIndexes = model.outputIndexes.size();
    LOGD(EDEN_DRIVER, "model.outputIndexes.size()=%d\n", numOfOutputIndexes);
    for (int i = 0; i < numOfOutputIndexes; i++) {
        LOGD(EDEN_DRIVER, "    %d\n", model.outputIndexes[i]);
    }

    for (int i = 0; i < numOfOperations; i++) {
        const V1_2::Operation& operation = model.operations[i];
        LOGD(EDEN_DRIVER, "[%d] type=%d\n", i, operation.type);
        LOGD(EDEN_DRIVER, "    inputs...\n");
        for (size_t j = 0; j < operation.inputs.size(); j++) {
            LOGD(EDEN_DRIVER, "        %d\n", operation.inputs[j]);
        }
        LOGD(EDEN_DRIVER, "    outputs...\n");
        for (size_t j = 0; j < operation.outputs.size(); j++) {
            LOGD(EDEN_DRIVER, "        %d\n", operation.outputs[j]);
        }
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

/**
 * @brief Set CompilerManager
 * @details This function sets an association to CompilerManager
 * @param[in] compilerManager an instance of CompilerManager.
 * @returns void
 */
void ModelConverter::setCompilerManager(std::shared_ptr<CompilerManager> compilerManager) {
    compilerManager_ = compilerManager;
}

/**
 * @brief Set NpuInfo
 * @details This function sets a NPU information
 * @param[in] vecNPUOperationInfos
 * @returns void
 */
void ModelConverter::setNPUInfo(std::vector<OperationInfo>& vecNPUOperationInfos) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    vecNPUOperationInfos_ = vecNPUOperationInfos;
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

/**
 * @brief Set ConstantCopy
 * @details This function sets a constant copy
 * @param[in] constantCopyAddr
 * @returns void
 */
void ModelConverter::setConstantCopy(std::shared_ptr<int8_t>& constantCopyAddr) {
    userConstantCopyAddr_ = constantCopyAddr;
}

/**
 * @brief Convert Android NN Model to EdenModel
 * @details This function converts an Android NN Model to an EdenModel.
 *          This workload requires several jobs.
 *          1) Data layout change from NHWC to NCHW.
 *          2) Divide model to several submodels by NPU supported and not-supported.
 *          3) Compile submodels for NPU and generate NCP.
 *          4) Add pre/post additional operations on model if needed.
 * @param[in] model Android NN Model to be converted
 * @param[out] edenModel EdenModel converted from Android NN Model
 * @param[out] modelInfo model specific information such as virtual address map, supported operation group
 * @returns return code
 */
int32_t ModelConverter::convert(const V1_2::Model& model, EdenModel*& edenModel, ModelInfo& modelInfo) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    // showModel(model);

    if (model.inputIndexes.size() == 0) {
        return RET_NO_INPUT_TENSOR_FOR_MODEL;
    }
    if (model.outputIndexes.size() == 0) {
        return RET_NO_OUTPUT_TENSOR_FOR_MODEL;
    }

    prepareModelInfo(model, modelInfo);
    divideOperationList(model, modelInfo.vecOpGroup);

    std::set<int32_t> allOperandsExceptNPU;
    std::vector<int32_t> allOperationsExceptNPU;
    getOperandsExceptNPU(modelInfo, allOperandsExceptNPU);
    getOperationsExceptNPU(modelInfo, allOperationsExceptNPU);

    int32_t numOfEdenOperands = allOperandsExceptNPU.size();

    int32_t numOfEdenOperationForNPU = getNumOfOperationForNPU(modelInfo);
    int32_t numOfEdenOperations = allOperationsExceptNPU.size() + numOfEdenOperationForNPU;

    LOGD(EDEN_DRIVER, "numOfEdenOperands:%d, numOfEdenOperations:%d, numOfEdenOperationForNPU:%d\n",
                       numOfEdenOperands, numOfEdenOperations, numOfEdenOperationForNPU);

    EdenModel* tempEdenModel = nullptr;
    EdenOperand* edenOperands = nullptr;
    EdenOperation* edenOperations = nullptr;
    createEmptyEdenModel(tempEdenModel, edenOperands, numOfEdenOperands, edenOperations, numOfEdenOperations);

    EdenOperation* edenOperation = nullptr;

    int32_t numOfInputs = model.inputIndexes.size();
    LOGD(EDEN_DRIVER, "numOfInputs: %d\n", numOfInputs);
    // Converting Android Operation to Eden Operation
    LOGD(EDEN_DRIVER, "Start converting Android Operation to Eden Operation...\n");
    int32_t edenOperationIdx = 0;
    for (NNSubOperationList& subOperationList : modelInfo.vecOpGroup) {
        if (subOperationList.targetDevice == NN_TARGET_DEVICE::NPU) {
            LOGD(EDEN_DRIVER, "Try to compile for NPU...\n");

            edenOperation = &edenOperations[edenOperationIdx];
            keepCustomOperationId(modelInfo, edenOperationIdx);

            int32_t zp = 0;
            for (int32_t inputIdx = 0; inputIdx < numOfInputs; inputIdx++) {
                // FIXME check if the number of inputs is greater than 1
                const V1_2::Operand& inputOperand = model.operands[model.operations[0].inputs[inputIdx]];
                zp = inputOperand.zeroPoint;
            }

            // TODO currently, the firmware does not support converting raster to cell format with zeropoint>0.
            // The following condition will be removed after the firmware is ready.
            if (zp == 0) {
                // disable runCFU
                // The firmware takes over the converting task.
                tempEdenModel->ShouldRunCFU() = false;
                compilerManager_->getNpuCompiler()->getCompilerOptions()->setInputFormat("GRAY8_NORMAL");
            } else {
                tempEdenModel->ShouldRunCFU() = true;
                compilerManager_->getNpuCompiler()->getCompilerOptions()->setInputFormat("NO_FORMAT");
            }

            // Create EdenOperands for NCP, this includes compilation
            createEdenCustomOpForNCP(model, modelInfo, subOperationList, edenOperation, tempEdenModel);

            // Move to next operand index
            edenOperationIdx++;
        } else {
            for (int32_t androidOperationIdx : subOperationList.operationList) {
                const V1_2::Operation& androidOperation = model.operations[androidOperationIdx];
                keepOperationToOperationIdMap(modelInfo, (void*)(&androidOperation), androidOperationIdx);
                LOGD(EDEN_DRIVER, "Try to convert androidOperationIdx=%d...\n", androidOperationIdx);

                edenOperation = &edenOperations[edenOperationIdx];
                keepOperationIdMap(modelInfo, androidOperationIdx, edenOperationIdx);

                int32_t status = createEdenOperation(model, modelInfo, androidOperation, edenOperation, tempEdenModel);
                if (RET_OK != status) {
                    LOGE(EDEN_DRIVER, "createEdenOperation failed: %d!\n", status);
                    delete[] edenOperands;
                    return status;
                }

                // Move to next operand index
                edenOperationIdx++;
            }
            // Convert operands to NCHW for NPU
            // After Operand is parsed on createEdenOperation,
            // we can know which operands are NCHW/NHWC by referring to modelInfo.vecIsNCHW
            convertToNCHW(model, modelInfo, subOperationList.operationList);
        }
    }
    LOGD(EDEN_DRIVER, "Start converting Android Operation to Eden Operation...Done!\n");

    // Set all operand name as Tensor if it has nullptr
    setEdenOperandNamesForNull(tempEdenModel);

    // Identify input and output indexes
    // set input indexes

    int32_t numOfEdenModelInputs = 0;

    int32_t* edenModelInputOperandIndexes = nullptr;
    edenModelInputOperandIndexes = new int32_t[numOfInputs];

    for (int32_t inputIdx = 0; inputIdx < numOfInputs; inputIdx++) {
        LOGD(EDEN_DRIVER, "inputIdx: %d, model.inputIndexes[inputIdx]: %d\n", inputIdx, model.inputIndexes[inputIdx]);
        if (modelInfo.mapOperandIdFromAToE.find(model.inputIndexes[inputIdx]) != modelInfo.mapOperandIdFromAToE.end()) {
            edenModelInputOperandIndexes[inputIdx] = modelInfo.mapOperandIdFromAToE.at(model.inputIndexes[inputIdx]);
            numOfEdenModelInputs++;
        } else {
            LOGD(EDEN_DRIVER, "inputIdx: %d, model.inputIndexes[inputIdx]: %d is tranformed to XXXOptions...skip it.\n",
                               inputIdx, model.inputIndexes[inputIdx]);
        }
    }

    // set output indexes
    int32_t numOfOutputs = model.outputIndexes.size();
    int32_t numOfEdenModelOutputs = 0;

    int32_t* edenModelOutputOperandIndexes = nullptr;
    edenModelOutputOperandIndexes = new int32_t[numOfOutputs];
    LOGD(EDEN_DRIVER, "numOfOutputs: %d\n", numOfOutputs);
    for (int32_t outputIdx = 0; outputIdx < numOfOutputs; outputIdx++) {
        LOGD(EDEN_DRIVER, "outputIdx: %d, model.outputIndexes[outputIdx]: %d\n", outputIdx, model.outputIndexes[outputIdx]);
        if (modelInfo.mapOperandIdFromAToE.find(model.outputIndexes[outputIdx]) != modelInfo.mapOperandIdFromAToE.end()) {
            edenModelOutputOperandIndexes[outputIdx] = modelInfo.mapOperandIdFromAToE.at(model.outputIndexes[outputIdx]);
            numOfEdenModelOutputs++;
        } else {
            LOGD(EDEN_DRIVER, "outputIdex: %d, model.outputIndexes[outputIdx]: %d is tranformed to XXXOptions...skip it.\n",
                               outputIdx, model.outputIndexes[outputIdx]);
        }
    }

    tempEdenModel->IdentifyInputsAndOutputs(numOfEdenModelInputs, edenModelInputOperandIndexes, numOfEdenModelOutputs, edenModelOutputOperandIndexes);

    delete[] edenModelInputOperandIndexes;
    delete[] edenModelOutputOperandIndexes;

    edenModel = tempEdenModel;

    delete[] edenOperands;
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

int32_t ModelConverter::setInputOutputToNHWC(EdenModel*& edenModel) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    auto& inputIndexes = edenModel->GetInputIndexes();
    auto& outputIndexes = edenModel->GetOutputIndexes();
    auto& mapToOperand = edenModel->GetMapToOperand();

    for (int32_t inputIdx : inputIndexes) {
        const EdenOperand* edenOperand = mapToOperand.at(inputIdx);

        if ((edenOperand->opType == EDEN_OP_CONV_2D || edenOperand->opType == EDEN_OP_DEPTHWISE_CONV_2D) &&
            inputIdx == 2 && edenOperand->isNCHW == true) {
            // weight need to convert from {Cout,K,K,Cin} To {Cout,Cin,K,K}
            edenOperand->shapeInfo->bufferLayout = BUFFER_LAYOUT_NHWC;
            int32_t n = edenOperand->shapeInfo->dims[N_NCHW];
            int32_t c = edenOperand->shapeInfo->dims[C_NCHW];
            int32_t h = edenOperand->shapeInfo->dims[H_NCHW];
            int32_t w = edenOperand->shapeInfo->dims[W_NCHW];

            edenOperand->shapeInfo->dims[N_NHWC] = n;
            edenOperand->shapeInfo->dims[H_NHWC] = h;
            edenOperand->shapeInfo->dims[W_NHWC] = w;
            edenOperand->shapeInfo->dims[C_NHWC] = c;
        }
        if (edenOperand->opType != EDEN_OP_BIDIRECTIONAL_SEQUENCE_LSTM && edenOperand->opType != EDEN_OP_EXPAND_DIMS &&
            edenOperand->opType != EDEN_OP_GATHER && edenOperand->opType != EDEN_OP_BIDIRECTIONAl_RNN &&
            edenOperand->isNCHW == false) {
                edenOperand->shapeInfo->bufferLayout = BUFFER_LAYOUT_NHWC;
                int32_t n = edenOperand->shapeInfo->dims[N_NCHW];
                int32_t c = edenOperand->shapeInfo->dims[C_NCHW];
                int32_t h = edenOperand->shapeInfo->dims[H_NCHW];
                int32_t w = edenOperand->shapeInfo->dims[W_NCHW];

                edenOperand->shapeInfo->dims[N_NHWC] = n;
                edenOperand->shapeInfo->dims[H_NHWC] = h;
                edenOperand->shapeInfo->dims[W_NHWC] = w;
                edenOperand->shapeInfo->dims[C_NHWC] = c;
        }
        if (edenOperand->opType == EDEN_OP_DECONV_2D && inputIdx == 2 &&
            edenOperand->isNCHW == false) {
            // keep weight layout {Cout,K,K,Cin}, and keep the dim info To {Cout,Cin,K,K}
            edenOperand->shapeInfo->bufferLayout = BUFFER_LAYOUT_NCHW;
            int32_t n = edenOperand->shapeInfo->dims[N_NHWC];
            int32_t h = edenOperand->shapeInfo->dims[H_NHWC];
            int32_t w = edenOperand->shapeInfo->dims[W_NHWC];
            int32_t c = edenOperand->shapeInfo->dims[C_NHWC];

            edenOperand->shapeInfo->dims[N_NCHW] = n;
            edenOperand->shapeInfo->dims[C_NCHW] = c;
            edenOperand->shapeInfo->dims[H_NCHW] = h;
            edenOperand->shapeInfo->dims[W_NCHW] = w;
        }
    }

    for (int32_t outputIdx : outputIndexes) {
        const EdenOperand* edenOperand = mapToOperand.at(outputIdx);
        if (edenOperand->opType != EDEN_OP_BIDIRECTIONAL_SEQUENCE_LSTM && edenOperand->opType != EDEN_OP_EXPAND_DIMS &&
            edenOperand->opType != EDEN_OP_GATHER && edenOperand->opType != EDEN_OP_BIDIRECTIONAl_RNN &&
            edenOperand->isNCHW == false) {
            edenOperand->shapeInfo->bufferLayout = BUFFER_LAYOUT_NHWC;
            int32_t n = edenOperand->shapeInfo->dims[N_NCHW];
            int32_t c = edenOperand->shapeInfo->dims[C_NCHW];
            int32_t h = edenOperand->shapeInfo->dims[H_NCHW];
            int32_t w = edenOperand->shapeInfo->dims[W_NCHW];

            edenOperand->shapeInfo->dims[N_NHWC] = n;
            edenOperand->shapeInfo->dims[H_NHWC] = h;
            edenOperand->shapeInfo->dims[W_NHWC] = w;
            edenOperand->shapeInfo->dims[C_NHWC] = c;
        }
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

void ModelConverter::convertParamToNHWC(EdenModel*& edenModel) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    auto eden_operands = edenModel->GetOperands();
    auto eden_operations = edenModel->GetOperations();

    for (auto &operation : eden_operations) {
        switch (operation->opType) {
            case EDEN_OP_CONV_2D:
            case EDEN_OP_DEPTHWISE_CONV_2D:
            {
                int32_t kernel_index = operation->inputOperandIndexes[1];
                EdenOperand *kernel_operand = eden_operands[kernel_index];
                if (kernel_operand->buffer->size != 0 ) {
                    PrePostProcessor::getInstance()->convertDataLayoutFromNCHWToNHWC(kernel_operand->buffer->addr,
                                                                                     kernel_operand->buffer->size,
                                                                                     kernel_operand->shapeInfo->dims[N_NCHW],
                                                                                     kernel_operand->shapeInfo->dims[C_NCHW],
                                                                                     kernel_operand->shapeInfo->dims[H_NCHW],
                                                                                     kernel_operand->shapeInfo->dims[W_NCHW],
                                                                                     getDataTypeFromEden(kernel_operand->shapeInfo->dataType),
                                                                                     0);
                }
                break;
            }
            default:
                LOGD(EDEN_DRIVER, "Op type %d don't need to convertParamToNHWC\n", operation->opType);
                break;
        }
    }
}

void ModelConverter::convertOperandToCNHW(ModelInfo& modelInfo, const V1_2::Operand& operand) {
    void* addr = nullptr;
    int32_t size = 0;
    if (operand.lifetime == OperandLifeTime::CONSTANT_REFERENCE) {
        char* virtAddr = modelInfo.vecVirtualAddressOnPools[operand.location.poolIndex].addr;
        addr = virtAddr + operand.location.offset;
        size = operand.location.length;
        LOGD(EDEN_DRIVER, "CONSTANT_REFERENCE: addr=%p\n", addr);
        LOGD(EDEN_DRIVER, "CONSTANT_REFERENCE: size=%d\n", size);
    } else if (operand.lifetime == OperandLifeTime::CONSTANT_COPY) {
        addr = reinterpret_cast<void*>(modelInfo.operandValues.get() + operand.location.offset);
        size = operand.location.length;
        LOGD(EDEN_DRIVER, "CONSTANT_COPY: modelInfo.operandValues.get()=%p\n", modelInfo.operandValues.get());
        LOGD(EDEN_DRIVER, "CONSTANT_COPY: operand.location.offset=%d\n", operand.location.offset);
        LOGD(EDEN_DRIVER, "CONSTANT_COPY: operand.location.length=%d\n", operand.location.length);
        LOGD(EDEN_DRIVER, "CONSTANT_COPY: addr=%p\n", addr);
        LOGD(EDEN_DRIVER, "CONSTANT_COPY: size=%d\n", size);
    } else {
        LOGD(EDEN_DRIVER, "operand.lieftime is %d... skip it.\n", static_cast<int32_t>(operand.lifetime));
        return;
    }

    if (operand.dimensions.size() > 1) {
        LOGD(EDEN_DRIVER, "Converting at addr=%p with size=%d operand.dimensions.size(): %zu\n", addr, size, operand.dimensions.size());

        int32_t numOfDims;
        int32_t dims[4];
        getEdenDimensions(operand.dimensions, dims, numOfDims, false);
        DATA_TYPE dataType = getDataType(operand.type);

        LOGD(EDEN_DRIVER, "Before converting data...\n");
        //DumpToStdio(addr, size, dataType);
        PrePostProcessor::getInstance()->convertDataLayoutFromNHWCToCNHW(addr, size, dims[N_NCHW], dims[C_NCHW], dims[H_NCHW], dims[W_NCHW], dataType);
        LOGD(EDEN_DRIVER, "After converting data...\n");
        //DumpToStdio(addr, size, dataType);
    } else {
        LOGD(EDEN_DRIVER, "operand.dimensions.size() is 0, don't need to dataLayoutConvert...\n");
    }
}

void ModelConverter::convertOperandToNCHW(ModelInfo& modelInfo, const V1_2::Operand& operand) {
    void* addr = nullptr;
    int32_t size = 0;
    if (operand.lifetime == OperandLifeTime::CONSTANT_REFERENCE) {
        char* virtAddr = modelInfo.vecVirtualAddressOnPools[operand.location.poolIndex].addr;
        addr = virtAddr + operand.location.offset;
        size = operand.location.length;
        LOGD(EDEN_DRIVER, "CONSTANT_REFERENCE: addr=%p\n", addr);
        LOGD(EDEN_DRIVER, "CONSTANT_REFERENCE: size=%d\n", size);
    } else if (operand.lifetime == OperandLifeTime::CONSTANT_COPY) {
        addr = reinterpret_cast<void*>(modelInfo.operandValues.get() + operand.location.offset);
        size = operand.location.length;
        LOGD(EDEN_DRIVER, "CONSTANT_COPY: modelInfo.operandValues.get()=%p\n", modelInfo.operandValues.get());
        LOGD(EDEN_DRIVER, "CONSTANT_COPY: operand.location.offset=%d\n", operand.location.offset);
        LOGD(EDEN_DRIVER, "CONSTANT_COPY: operand.location.length=%d\n", operand.location.length);
        LOGD(EDEN_DRIVER, "CONSTANT_COPY: addr=%p\n", addr);
        LOGD(EDEN_DRIVER, "CONSTANT_COPY: size=%d\n", size);
    } else {
        LOGD(EDEN_DRIVER, "operand.lieftime is %d... skip it.\n", static_cast<int32_t>(operand.lifetime));
        return;
    }

    if (operand.dimensions.size() > 1) {
        LOGD(EDEN_DRIVER, "Converting at addr=%p with size=%d operand.dimensions.size(): %zu\n", addr, size, operand.dimensions.size());

        int32_t numOfDims;
        int32_t dims[4];
        getEdenDimensions(operand.dimensions, dims, numOfDims, false);
        DATA_TYPE dataType = getDataType(operand.type);

        LOGD(EDEN_DRIVER, "Before converting data...\n");
        //DumpToStdio(addr, size, dataType);
        PrePostProcessor::getInstance()->convertDataLayoutFromNHWCToNCHW(addr, size, dims[N_NCHW], dims[C_NCHW], dims[H_NCHW], dims[W_NCHW], dataType);
        LOGD(EDEN_DRIVER, "After converting data...\n");
        //DumpToStdio(addr, size, dataType);
    } else {
        LOGD(EDEN_DRIVER, "operand.dimensions.size() is 0, don't need to dataLayoutConvert...\n");
    }
}

/**
 * @brief Convert data layout on model to NCHW
 * @details This function converts data layout for data of Operand on model to NCHW.
 *          It iterates all Operand and processes data layout change on data.
 *          e.g. weight, bias, kernel on Operand will be changed to NCHW
 *          This function assumes model is constructed by NHWC data layout.
 * @param[in] model Android NN Model to be converted in terms of data layout from NHWC to NCHW
 * @param[in] modelInfo model specific information such as virtual address map, supported operation group
 * @param[in] operationList operation list
 * @returns return code
 */
int32_t ModelConverter::convertToNCHW(const V1_2::Model& model, ModelInfo& modelInfo, const std::vector<int32_t>& operationList) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    for (int32_t opIdx : operationList) {
        const V1_2::Operation& androidOperation = model.operations[opIdx];
        uint32_t op_type = static_cast<int32_t>(androidOperation.type);
        LOGD(EDEN_DRIVER, "op_type=%d\n", op_type);
        switch (op_type)
        {
            case ANEURALNETWORKS_CONV_2D:
            case ANEURALNETWORKS_DEPTHWISE_CONV_2D:
            case ANEURALNETWORKS_GROUPED_CONV_2D:
            case ANEURALNETWORKS_TRANSPOSE_CONV_2D:
            {
                bool isNCHW = false;
                if (op_type == ANEURALNETWORKS_CONV_2D) {
                    bool isV1_2 = false;
                    if (androidOperation.inputs.size() >= 8 ) {
                        auto index = androidOperation.inputs[7];
                        isV1_2 = model.operands[index].type == OperandType::BOOL ? true : false;
                    }

                    if (androidOperation.inputs.size() >= 8 && isV1_2) {
                        isNCHW = getValue<bool>(model, androidOperation, 7);
                    } else if (androidOperation.inputs.size() >= 11) {
                        isNCHW = getValue<bool>(model, androidOperation, 10);
                    }
                } else if (op_type == ANEURALNETWORKS_GROUPED_CONV_2D) {
                    if (androidOperation.inputs.size() == 12) {
                        isNCHW = getValue<bool>(model, androidOperation, 11);
                    } else {
                        isNCHW = getValue<bool>(model, androidOperation, 8);
                    }
                } else if (op_type == ANEURALNETWORKS_TRANSPOSE_CONV_2D) {
                    isNCHW = getValue<bool>(model, androidOperation, androidOperation.inputs.size() - 1);
                } else if (op_type == ANEURALNETWORKS_DEPTHWISE_CONV_2D) {
                    bool isV1_2 = false;
                    if (androidOperation.inputs.size() >= 9 ) {
                      auto index = androidOperation.inputs[8];
                      isV1_2 = model.operands[index].type == OperandType::BOOL ? true : false;
                    }

                    if (androidOperation.inputs.size() >= 9 && isV1_2) {
                      isNCHW = getValue<bool>(model, androidOperation, 8);
                    } else if (androidOperation.inputs.size() >= 12) {
                      isNCHW = getValue<bool>(model, androidOperation, 11);
                    }
                }

                LOGD(EDEN_DRIVER, "isNCHW=%d\n", isNCHW);
                if (!isNCHW) {
                    LOGD(EDEN_DRIVER, "convert inputs[0] of op_type=%d\n", op_type);
                    const V1_2::Operand &inputOperand = model.operands[androidOperation.inputs[0]];
                    convertOperandToNCHW(modelInfo, inputOperand);
                    modelInfo.vecConverted[0] = true;
                    modelInfo.vecIsNCHW[0] = true;

                    for (int32_t outputIdx : androidOperation.outputs) {
                        LOGD(EDEN_DRIVER, "convert outputs[%d] of op_type=%d\n", outputIdx, op_type);
                        const V1_2::Operand &outputOperand = model.operands[outputIdx];
                        convertOperandToNCHW(modelInfo, outputOperand);
                        modelInfo.vecConverted[outputIdx] = true;
                        modelInfo.vecIsNCHW[outputIdx] = true;
                    }
                }

                for (size_t idx = 1; idx < androidOperation.inputs.size(); idx++) {
                    int32_t inputIdx = androidOperation.inputs[idx];
                    const V1_2::Operand &inputOperand = model.operands[inputIdx];
                    if (idx == 1 && op_type == ANEURALNETWORKS_TRANSPOSE_CONV_2D) {
                        // in EDEN, the filter layout of deconv is CNHW
                        LOGD(EDEN_DRIVER, "convert inputs[%zu]=%d of op_type=%d\n", idx, inputIdx, op_type);
                        convertOperandToCNHW(modelInfo, inputOperand);
                        modelInfo.vecConverted[inputIdx] = true;
                        modelInfo.vecIsNCHW[inputIdx] = true;
                    } else {
                        LOGD(EDEN_DRIVER, "convert inputs[%zu]=%d of op_type=%d\n", idx, inputIdx, op_type);
                        convertOperandToNCHW(modelInfo, inputOperand);
                        modelInfo.vecConverted[inputIdx] = true;
                        modelInfo.vecIsNCHW[inputIdx] = true;
                    }
                }
                break;
            }
            case ANEURALNETWORKS_FULLY_CONNECTED:
            {
                for (size_t idx = 1; idx < androidOperation.inputs.size(); idx++) {
                    int32_t inputIdx = androidOperation.inputs[idx];
                    LOGD(EDEN_DRIVER, "convert inputs[%zu]=%d of op_type=%d\n", idx, inputIdx, op_type);
                    const V1_2::Operand &inputOperand = model.operands[inputIdx];
                    convertOperandToNCHW(modelInfo, inputOperand);
                    modelInfo.vecConverted[inputIdx] = true;
                    modelInfo.vecIsNCHW[inputIdx] = true;
                }

                for (int32_t outputIdx : androidOperation.outputs) {
                    const V1_2::Operand &outputOperand = model.operands[outputIdx];
                    LOGD(EDEN_DRIVER, "convert outputs[]=%d of op_type=%d\n", outputIdx, op_type);
                    convertOperandToNCHW(modelInfo, outputOperand);
                    modelInfo.vecConverted[outputIdx] = true;
                    modelInfo.vecIsNCHW[outputIdx] = true;
                }
                break;
            }
            case ANEURALNETWORKS_GATHER:
                for (int32_t inputIdx : androidOperation.inputs) {
                    modelInfo.vecConverted[inputIdx] = true;
                    modelInfo.vecIsNCHW[inputIdx] = true;
                }
                for (int32_t outputIdx : androidOperation.outputs) {
                    modelInfo.vecConverted[outputIdx] = true;
                    modelInfo.vecIsNCHW[outputIdx] = true;
                }
                break;
            default: {
                for (int32_t inputIdx : androidOperation.inputs) {
                    const V1_2::Operand &inputOperand = model.operands[inputIdx];
                    LOGD(EDEN_DRIVER, "convert inputs[]=%d of op_type=%d\n", inputIdx, op_type);
                    convertOperandToNCHW(modelInfo, inputOperand);
                    modelInfo.vecConverted[inputIdx] = true;
                    modelInfo.vecIsNCHW[inputIdx] = true;
                }
                for (int32_t outputIdx : androidOperation.outputs) {
                    LOGD(EDEN_DRIVER, "convert outputs[]=%d of op_type=%d\n", outputIdx, op_type);
                    const V1_2::Operand &outputOperand = model.operands[outputIdx];
                    convertOperandToNCHW(modelInfo, outputOperand);
                    modelInfo.vecConverted[outputIdx] = true;
                    modelInfo.vecIsNCHW[outputIdx] = true;
                }
                break;
            }
        }
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Get all operands except ones that used by NPU
 * @details This function finds all operands except ones tha used by NPU.
 *          If operands are used by NPU and they are also used by non-NPU,
 *          they are included on result. Model inputs and outputs are those cases.
 * @param[in] modelInfo model specific information such as virtual address map, supported operation group
 * @param[in] allOperandsExceptNPU all operands except ones that used by NPU
 * @returns void
 */
void ModelConverter::getOperandsExceptNPU(ModelInfo& modelInfo, std::set<int32_t>& allOperandsExceptNPU) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    // Sum up operands except ones used by NPU internally.
    // They will be compiled and represented as one CustomOp.
    allOperandsExceptNPU.clear();
    for (NNSubOperationList subOperationList : modelInfo.vecOpGroup) {
        if (subOperationList.targetDevice != NN_TARGET_DEVICE::NPU) {
            allOperandsExceptNPU.insert(subOperationList.inputOperandIndexes.begin(),
                subOperationList.inputOperandIndexes.end());
            allOperandsExceptNPU.insert(subOperationList.outputOperandIndexes.begin(),
                subOperationList.outputOperandIndexes.end());
        }
    }

    // Add model input and output operands
    for (int32_t idx : modelInfo.modelInputIndexes) {
        allOperandsExceptNPU.insert(idx);
    }
    for (int32_t idx : modelInfo.modelOutputIndexes) {
        allOperandsExceptNPU.insert(idx);
    }

    // show allOperandsExceptNPU
    LOGD(EDEN_DRIVER, "allOperandsExceptNPU...\n");
    for (auto it = allOperandsExceptNPU.begin(); it != allOperandsExceptNPU.end(); ++it) {
        LOGD(EDEN_DRIVER, "%d \n", *it);
    }
    LOGD(EDEN_DRIVER, "allOperandsExceptNPU...Done!\n");
    LOGD(EDEN_DRIVER, "ModelConverter::getOperandsExceptNPU() is Done!.\n");
}

/**
 * @brief Get all operations except ones that used by NPU
 * @details This function finds all operations except ones tha used by NPU.
 * @param[in] modelInfo model specific information such as virtual address map, supported operation group
 * @param[in] allOperandsExceptNPU all operations except ones that used by NPU
 * @returns void
 */
void ModelConverter::getOperationsExceptNPU(ModelInfo& modelInfo, std::vector<int32_t>& allOperationsExceptNPU) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    // Sum up operations except ones used by NPU.
    // They will be compiled and represented as one CustomOp.
    allOperationsExceptNPU.clear();
    for (NNSubOperationList subOperationList : modelInfo.vecOpGroup) {
        if (subOperationList.targetDevice != NN_TARGET_DEVICE::NPU) {
            allOperationsExceptNPU.insert(allOperationsExceptNPU.end(),
                subOperationList.operationList.begin(), subOperationList.operationList.end());
        }
    }

    // show allOperandsExceptNPU
    LOGD(EDEN_DRIVER, "allOperationsExceptNPU...\n");
    for (auto opIdx : allOperationsExceptNPU) {
        LOGD(EDEN_DRIVER, "%d \n", opIdx);
    }
    LOGD(EDEN_DRIVER, "allOperationsExceptNPU...Done!\n");
    LOGD(EDEN_DRIVER, "ModelConverter::getOperationsExceptNPU() is done!\n");
}

/**
 * @brief Get # of operations for NPU
 * @details This function finds # of operations assigned to NPU.
 * @param[in] modelInfo model specific information such as virtual address map, supported operation group
 * @returns # of operations assigned to NPU
 */
int32_t ModelConverter::getNumOfOperationForNPU(ModelInfo& modelInfo) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    // Sum up operations except ones used by NPU.
    // They will be compiled and represented as one CustomOp.
    int32_t numOfEdenOperationsForNPU = 0;
    for (NNSubOperationList subOperationList : modelInfo.vecOpGroup) {
        if (subOperationList.targetDevice == NN_TARGET_DEVICE::NPU) {
            numOfEdenOperationsForNPU += subOperationList.operationList.size();
        }
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return numOfEdenOperationsForNPU;
}

/**
 * @brief Get input indexes for a given OpGroup
 * @details This function finds input indexes for a given OpGroup.
 * @param[in] subOperationList list for operations to be executed for OpGroup
 * @param[out] inputIndexes list for input indexes
 * @returns # of operations assigned to NPU
 */
void ModelConverter::getInputOperandIndexes(NNSubOperationList& subOperationList, std::vector<int32_t>& inputIndexes) {
    LOGD(EDEN_DRIVER, "%s() is called.\n", __func__);

    std::set<int32_t>& first = subOperationList.inputOperandIndexes;
    for (auto iter = first.begin(); iter != first.end(); ++iter) {
        LOGI(EDEN_DRIVER, "%d \n", *iter);
        inputIndexes.push_back(*iter);
    }
    LOGD(EDEN_DRIVER, "%s() is done!\n", __func__);
}

/**
 * @brief Get output indexes for a given OpGroup
 * @details This function finds output indexes for a given OpGroup.
 * @param[in] subOperationList list for operations to be executed for OpGroup
 * @param[out] outputIndexes list for output indexes
 * @returns # of operations assigned to NPU
 */
void ModelConverter::getOutputOperandIndexes(NNSubOperationList& subOperationList, std::vector<int32_t>& outputIndexes) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    std::set<int32_t> diff;
    std::set<int32_t>& first = subOperationList.outputOperandIndexes;
    std::set<int32_t>& second = subOperationList.inputOperandIndexes;
    std::set_difference(first.begin(), first.end(), second.begin(), second.end(),
        std::inserter(diff, diff.end()));

    LOGD(EDEN_DRIVER, "outputIndexes=(");
    for (auto iter = diff.begin(); iter != diff.end(); ++iter) {
        LOGD(EDEN_DRIVER, "%d ", *iter);
        outputIndexes.push_back(*iter);
    }
    LOGD(EDEN_DRIVER, ")\n");
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

/**
 * @brief Divide operation list by NPU supported and not-supported
 * @details This function divides an operation on a given by NPU supported and not-supported.
 *          Then it generates one or more NNSubOperationList
 *          which represents for a bunch of operation list and its target device.
 * @param[in] model Android NN Model
 * @param[out] vecOpGroup Divided operation list for target devices represented as NNSubOperationList
 * @returns return code
 */
int32_t ModelConverter::divideOperationList(const V1_2::Model& model, std::vector<NNSubOperationList>& vecOpGroup) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    bool isSupportedByNPUModel = compilerManager_->isSupportedModelByNPUC(model);

    std::vector<bool> vecSupportedByNPU(model.operations.size(), false);
    LOGD(EDEN_DRIVER, "vecSupportedByNPU model.operations.size(): %zu(+)\n", model.operations.size());
    for (size_t idx = 0; idx < model.operations.size(); idx++) {
        if (isSupportedByNPUModel && supportedByNPU(model, model.operations[idx])) {
            LOGD(EDEN_DRIVER, "vecSupportedByNPU idx: %zu(+)\n", idx);
            vecSupportedByNPU[idx] = true;
        } else {
            LOGD(EDEN_DRIVER, "opType=(%d) at idx=%zu is not supported by NPU!\n", model.operations[idx].type, idx);
            // Default value is false
            // vecSupportedByNPU[idx] = false;
        }
    }

    // Build up vecOpGroup based on vecSupportedByNPU result
    for (size_t idx = 0; idx < vecSupportedByNPU.size(); ) {
        idx = buildOpGroup(model, idx, vecSupportedByNPU, vecOpGroup);
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Build NNSubOperationList based on a given vecSupportedByNPU starting from offset
 * @details This function builds a NNSubOperationList.
 * @param[in] model Android NN Model
 * @param[in] offset offset on vecSupportedByNPU
 * @param[in] vecSupportedByNPU list whether each operation is supported by NPU or not
 * @param[out] vecOpGroup Divided operation list for target devices represented as NNSubOperationList
 * @returns return code
 */
int32_t ModelConverter::buildOpGroup(const V1_2::Model& model, int32_t offset, std::vector<bool>& vecSupportedByNPU,
                                     std::vector<NNSubOperationList>& vecOpGroup) {
    LOGD(EDEN_DRIVER,  "%s() is called with offset: %d\n", __func__, offset);
    NNSubOperationList subOperationList;
    subOperationList.targetDevice = (vecSupportedByNPU[offset] == true ? NN_TARGET_DEVICE::NPU : NN_TARGET_DEVICE::GPU);
    bool targetFlag = vecSupportedByNPU[offset];

    // Iterate vecSupportedByNPU until targetFlag is failed
    size_t idx = offset;
    for (; idx < vecSupportedByNPU.size(); idx++) {
        if (vecSupportedByNPU[idx] == targetFlag) {
            subOperationList.operationList.push_back(idx);
            for (int32_t inputOperandIndex : model.operations[idx].inputs) {
                subOperationList.inputOperandIndexes.insert(inputOperandIndex);
            }
            for (int32_t outputOperandIndex : model.operations[idx].outputs) {
                subOperationList.outputOperandIndexes.insert(outputOperandIndex);
            }
        } else {
            break;
        }
    }
    // Now all supported operations are added
    vecOpGroup.push_back(subOperationList);

    // idx has a first index which does not support it or last index + 1
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return idx;
}

/**
 * @brief Check if a given operation is supported by NPU or not
 * @details This function checks if a given operation is supported by NPU or not.
 * @param[in] model Android NN Model
 * @param[in] androidOperation Android Operation
 * @returns return code
 */
bool ModelConverter::supportedByNPU(const V1_2::Model& model, const V1_2::Operation& androidOperation) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    OperationInfo& operationInfo = vecNPUOperationInfos_[static_cast<int32_t>(androidOperation.type)];
    if (operationInfo.supported == false) {
        return false;
    }

    bool supported = true;

    int32_t opType = static_cast<int32_t>(androidOperation.type);
    switch (opType) {
    case ANEURALNETWORKS_CONV_2D:
    {
        // Get input width, height
        const V1_2::Operand& inputOperand = model.operands[androidOperation.inputs[0]];  // 0 is input
        const V1_2::Operand& kernelOperand = model.operands[androidOperation.inputs[1]];  // 1 is kernel

        // Check unknown dimension, unknown rank
        if ((inputOperand.dimensions.size() == 0) ||   // unknown rank
            (inputOperand.dimensions[W_NHWC] == 0) ||  // unknown dimension
            (inputOperand.dimensions[H_NHWC] == 0)) {  // unknown dimension
            LOGD(EDEN_DRIVER, "Unknown dimension, unknown rank is detected... skip it\n");
            break;
        }
        if ((kernelOperand.dimensions.size() == 0) ||   // unknown rank
            (kernelOperand.dimensions[W_NHWC] == 0) ||  // unknown dimension
            (kernelOperand.dimensions[H_NHWC] == 0)) {  // unknown dimension
            LOGD(EDEN_DRIVER, "Unknown dimension, unknown rank is detected... skip it\n");
            break;
        }

        if (inputOperand.type != V1_2::OperandType::TENSOR_QUANT8_ASYMM) {
            supported = false;
            LOGD(EDEN_DRIVER, "OperandType is not TENSOR_QUANT8_ASYMM... skip it\n");
            break;
        }
        int32_t inputWidth = inputOperand.dimensions[1];   // 1 is input width
        int32_t inputHeight = inputOperand.dimensions[2];  // 2 is input height

        // Check kernel size
        // int32_t depthOut = kernelOperand.dimensions[0];
        int32_t filterHeight = kernelOperand.dimensions[1];
        int32_t filterWidth = kernelOperand.dimensions[2];
        // int32_t filterChannel = kernelOperand.dimensions[3];

        std::shared_ptr<void> constraint = operationInfo.constraint;
        Conv2DConstraint* ptrConstraint = reinterpret_cast<Conv2DConstraint*>(constraint.get());
        if ((ptrConstraint->maxKernelSize < filterWidth) ||
            (ptrConstraint->maxKernelSize < filterHeight)) {
            supported = false;
            LOGD(EDEN_DRIVER, "Too big kernel size... skip it\n");
            break;
        }

        if (androidOperation.inputs.size() == 10) {  // explicit
            // Check stride size
            int32_t strideWidth = getValue<int32_t>(model, androidOperation, 7);   // 7 is stride for width dimension
            int32_t strideHeight = getValue<int32_t>(model, androidOperation, 8);  // 8 is stride for height dimension
            if ((ptrConstraint->maxStrideSize < strideWidth) ||
                (ptrConstraint->maxStrideSize < strideHeight)) {
                supported = false;
                LOGD(EDEN_DRIVER, "Too big stride size... skip it\n");
                break;
            }

            // Check padding size
            int32_t padLeft = getValue<int32_t>(model, androidOperation, 3);    // 3 is padding on left
            int32_t padRight = getValue<int32_t>(model, androidOperation, 4);   // 4 is padding on right
            int32_t padTop = getValue<int32_t>(model, androidOperation, 5);     // 5 is padding on top
            int32_t padBottom = getValue<int32_t>(model, androidOperation, 6);  // 6 is padding on bottom
            if ((ptrConstraint->maxPaddingSize < padLeft) ||
                (ptrConstraint->maxPaddingSize < padRight) ||
                (ptrConstraint->maxPaddingSize < padTop) ||
                (ptrConstraint->maxPaddingSize < padBottom)) {
                supported = false;
                LOGD(EDEN_DRIVER, "Too big padding size... skip it\n");
                break;
            }
        } else {
            // Check stride size
            int32_t strideWidth = getValue<int32_t>(model, androidOperation, 4);   // 4 is stride for width dimension
            int32_t strideHeight = getValue<int32_t>(model, androidOperation, 5);  // 5 is stride for height dimension
            if ((ptrConstraint->maxStrideSize < strideWidth) ||
                (ptrConstraint->maxStrideSize < strideHeight)) {
                supported = false;
                LOGD(EDEN_DRIVER, "Too big stride size... skip it\n");
                break;
            }

            // Check padding size
            int32_t padLeft = 0;
            int32_t padRight = 0;
            int32_t padTop = 0;
            int32_t padBottom = 0;
            int32_t paddingScheme = getValue<int32_t>(model, androidOperation, 3);    // 3 is padding scheme
            if (paddingScheme == 1) {  // ANEURALNETWORKS_PADDING_SAME=1
                int32_t outSize = (inputWidth + strideWidth - 1) / strideWidth;
                int32_t neededInput = (outSize - 1) * strideWidth + filterWidth;
                int32_t totalPadding = std::max(0, neededInput - inputWidth);
                padLeft = totalPadding / 2;
                padRight = (totalPadding + 1) / 2;

                outSize = (inputHeight + strideHeight - 1) / strideHeight;
                neededInput = (outSize - 1) * strideHeight + filterHeight;
                totalPadding = std::max(0, neededInput - inputHeight);
                padTop = totalPadding / 2;
                padBottom = (totalPadding + 1) / 2;
            }

            if ((ptrConstraint->maxPaddingSize < padLeft) ||
                (ptrConstraint->maxPaddingSize < padRight) ||
                (ptrConstraint->maxPaddingSize < padTop) ||
                (ptrConstraint->maxPaddingSize < padBottom)) {
                supported = false;
                LOGD(EDEN_DRIVER, "Too big padding size... skip it\n");
                break;
            }
        }
    }
    break;
    case ANEURALNETWORKS_MAX_POOL_2D:
    // @todo Add more constraints if exists
    break;
    case ANEURALNETWORKS_AVERAGE_POOL_2D:
    // @todo Add more constraints if exists
    break;
    case ANEURALNETWORKS_RESHAPE:
    // @todo Add more constraints if exists
    break;
    case ANEURALNETWORKS_CONCATENATION:
    // @todo Add more constraints if exists
    break;
    default:
        LOGD(EDEN_DRIVER, "opType=%d is not supported by NPU!\n", opType);
        supported = false;
        break;
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return supported;
}

/**
 * @brief Create CustomOp for NCP represents for a given operation list
 * @details This function creates a CustomOp for NCP represents for a given operation list.
 *          This function cooperates with CompilerManager to generate NCP and embeds it on CustomOp.
 * @param[in] model Android NN Model
 * @param[in] operationList operation index list corresponding to Model's Operand[]
 * @param[out] edenOperand Created EdenOperand representing for a given operationList on model
 * @returns return code
 */
int32_t ModelConverter::createEdenCustomOpForNCP(const V1_2::Model& model, ModelInfo& modelInfo, NNSubOperationList& subOperationList,
                                                 EdenOperation* edenOperation, EdenModel* edenModel) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    // Convert operands to NCHW for NPU
    // Since weight/bias are embedded on compiled binary,
    // they should be converted before compile(..)
    convertToNCHW(model, modelInfo, subOperationList.operationList);

    void* bufferForNCP = nullptr;
    int32_t bufferSizeForNCP = 0;
    compilerManager_->compile(model, modelInfo, subOperationList.operationList, &bufferForNCP, &bufferSizeForNCP);

    // Now create EdenOperand for NCP_BINARY and NCP_NAME

    // Create EdenOperand for NCP_BINARY
    EdenOperand* edenOperandForBinary = nullptr;
    createEdenCustomOperandForNcpBinary(bufferForNCP, bufferSizeForNCP, &edenOperandForBinary);

    // Create EdenOperand for NCP_NAME
    EdenOperand* edenOperandForName = nullptr;
    createEdenCustomOperandForNcpName(&edenOperandForName);

    // Add EdenOperands for NCP to get unique ids on edenOperandIdForBinary and edenOperandIdForName
    int32_t edenOperandIdForBinary = -1;
    int32_t edenOperandIdForName = -1;
    edenModel->AddOperandForOptions(edenOperandForBinary, &edenOperandIdForBinary);
    edenModel->AddOperandForOptions(edenOperandForName, &edenOperandIdForName);

    // Now build up edenOperation
    edenOperation->opType = EDEN_OP_CUSTOM;
    getEdenCustomOperationNameForNCP(edenOperation->opName.name, edenOperation->opName.length);

    // @todo what if ncp receives more than one input tensors?
    //std::vector<int32_t> inputIndexes;
    //getInputOperandIndexes(subOperationList, inputIndexes);
    //int32_t inputIndex = inputIndexes[0];
    // @todo in fact, below code assumes first operand is only one to be delivered as an input.
    edenOperation->numOfInputs = 3;  // 1 for input tensor, +2 for NCP_BINARY and NCP_NAME
    edenOperation->inputOperandIndexes = new int32_t[edenOperation->numOfInputs];
    int32_t inputIndex = modelInfo.modelInputIndexes[0];

    edenOperation->inputOperandIndexes[0] = getEdenOperandIdx(model, modelInfo, edenModel, inputIndex,
                                                              edenOperation->opType, edenOperation->isNCHW);
    edenOperation->inputOperandIndexes[1] = edenOperandIdForBinary;
    edenOperation->inputOperandIndexes[2] = edenOperandIdForName;

    std::vector<int32_t> outputIndexes;
    getOutputOperandIndexes(subOperationList, outputIndexes);
    edenOperation->numOfOutputs = outputIndexes.size();
    edenOperation->outputOperandIndexes = new int32_t[edenOperation->numOfOutputs];
    for (int32_t outIdx = 0; outIdx < edenOperation->numOfOutputs; outIdx++) {
        edenOperation->outputOperandIndexes[outIdx] = getEdenOperandIdx(model, modelInfo, edenModel, outputIndexes[outIdx],
                                                                        edenOperation->opType, edenOperation->isNCHW);
    }

    // ShowArray("edenOperation->inputOperandIndexes=", edenOperation->inputOperandIndexes, edenOperation->numOfInputs);
    // ShowArray("edenOperation->outputOperandIndexes=", edenOperation->outputOperandIndexes, edenOperation->numOfOutputs);

    // Now add generated edenOperation to model
    int32_t edenOperationId = -1;
    edenModel->AddOperation(edenOperation, &edenOperationId);
    LOGD(EDEN_DRIVER, "Operation converting is complete! opType=%d, edenOperationId=%d\n", edenOperation->opType, edenOperationId);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Create CustomOp for Normalization with given means and scales
 * @details This function creates a CustomOp for Normalization represents for a given means and scales.
 * @param[in] vecMeans list of mean values
 * @param[in] vecScales list of scale values
 * @param[out] edenOperand Created EdenOperand for normalization as custom op.
 * @returns return code
 */
int32_t ModelConverter::createEdenCustomOpForNormalization(const std::vector<int32_t>& /*vecMeans*/, const std::vector<int32_t>& /*vecScales*/,
                                                           EdenOperand*& /*edenOperand*/) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Create CustomOp for Quantization with a given fractional length
 * @details This function creates a CustomOp for Quantization with a given fractional length.
 * @param[in] vecFracLens list of fractional length
 * @param[out] edenOperand Created EdenOperand for quantization as custom op.
 * @returns return code
 */
int32_t ModelConverter::createEdenCustomOpForQuantization(const std::vector<int32_t>& /*vecFracLens*/, EdenOperand*& /*edenOperand*/) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Create CustomOp for Dequantization with a given fractional length
 * @details This function creates a CustomOp for Dequantization with a given fractional length.
 * @param[in] vecFracLens list of fractional length
 * @param[out] edenOperand Created EdenOperand for quantization as custom op.
 * @returns return code
 */
int32_t ModelConverter::createEdenCustomOpForDequantization(const std::vector<int32_t>& /*vecFracLens*/, EdenOperand*& /*edenOperand*/) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Clear internal map
 * @details This function clears internal map used on converting.
 * @param void
 * @returns return code
 */
int32_t ModelConverter::clearMaps() {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

}  // namespace eden_driver
}  // namespace nn
}  // namespace android

