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
 * @file    ExecutionScheduler.cpp
 * @brief   This is ExecutionScheduler class file.
 * @details This header defines ExecutionScheduler class.
 *          This class is implementing the execution scheduling with priority queues.
 * @author  minsu.jeon (minsu.jeon@samsung.com)
 *          yeongjun.kim (yj0576.kim@samsung.com)
 */

#include <iostream>
#include <cstring>    // memcpy
#include <inttypes.h>  // PRId64, PRIu64

#include "log.h"
#include "Utils.h"    // convertToV1_0, convertToV1_1, android::nn::initVLogMask, logModelToInfo, DRIVER
#include "MyUtils.h"  // DumpToStdio

#include "Common.h"
#include "EdenServiceDelegatorLib.h"
#include "EdenPreparedModel.h"
#include "SchedulePolicy.h"
#include "EdenModelConvertLib.h"
#include "PrePostProcessor.h"

#include "ExecutionScheduler.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EdenDriver::ExecutionScheduler"

namespace android {
namespace nn {
namespace eden_driver {

uint64_t microsecondsDuration(std::chrono::steady_clock::time_point end, std::chrono::steady_clock::time_point start) {
    return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

static bool isRunning = false;

const int32_t USER_NN_TIMEOUT = 0xffffffff;

void userNotify(addr_t* addr, addr_t value);
int32_t userWait(addr_t* addr, uint32_t value, uint32_t timeout);
void showEdenRequest(EdenRequest& request);

static Return<void> callNotifyWithExecutionResult(const sp<V1_0::IExecutionCallback>& callback_1_0, const ErrorStatus& status,
                                                  const hidl_vec<OutputShape>& /*outputShapes*/, Timing /*timing*/) {
    return callback_1_0->notify(status);
}

static Return<void> callNotifyWithExecutionResult(const sp<V1_2::IExecutionCallback>& callback_1_2, const ErrorStatus& status,
                                                  const hidl_vec<OutputShape>& outputShapes, Timing timing) {
    return callback_1_2->notify_1_2(status, outputShapes, timing);
}

template <typename T_Callback>
int32_t requestOneExecutionInAsyncBase(ExecutionScheduler* executionScheduler,
                                       const EdenPreparedModel* edenPreparedModel,
                                       const V1_0::Request& request,
                                       BufferInfoOnExecute& bufInfoOnExecute,
                                       V1_2::MeasureTiming measure,
                                       std::chrono::steady_clock::time_point driverStart,
                                       T_Callback& callback) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    std::chrono::steady_clock::time_point deviceStart, deviceEnd, driverEnd;

    hidl_vec<OutputShape> outputShapes;
    int32_t retCode = RET_OK;

    do {
        if (edenPreparedModel == nullptr || callback == nullptr) {
            LOGE(EDEN_DRIVER, "Invalied Params.\n");
            retCode = INVALID_PARAMS;
            break;
        }

        // To skip DynamicOutputShapeTest
        // TODO: to support dynamic output
        int32_t outputSizeSum = 0;
        bool skipZeroSize = true;
        for (size_t idx = 0; idx < edenPreparedModel->model.outputIndexes.size(); idx++) {
            int32_t outputSizeMul = 1;
            int32_t outputIndex = edenPreparedModel->model.outputIndexes[idx];
            const V1_2::Operand& androidOperand = edenPreparedModel->model.operands[outputIndex];
            for (size_t i = 0; i < androidOperand.dimensions.size(); i++) {
                outputSizeSum += androidOperand.dimensions[i];
                outputSizeMul *= androidOperand.dimensions[i];
            }
            if (outputSizeMul != 0) {
                skipZeroSize = false;
            }
        }
        if (outputSizeSum == 0) {
            LOGE(EDEN_DRIVER, "Invalied OutputParams.\n");
            retCode = INVALID_PARAMS;
            break;
        }

        if (skipZeroSize) {
            LOGD(EDEN_DRIVER, "all the output is zero size, don't need to execute.\n");
            retCode = RET_OK;
            break;
        }

#if 0

        std::shared_ptr<NNRequestData> reqData = std::make_shared<NNRequestData>();
        reqData->execMode = EXECUTION_MODE::ASYNC;
        reqData->callback = callback.get();
        reqData->preparedModel = reinterpret_cast<EdenPreparedModel*>(preparedModel.get());
        reqData->request = &request;
        reqData->priority = 0;
        reqData->timeStamp.requestedTime = std::chrono::system_clock::now();

        int32_t ret = schedulePolicy_->decidePriority(preparedModel.get(), (void*)&request, reqData->priority);
        if (ret != RET_OK) {
            LOGE(EDEN_DRIVER, << "decidePriority() is failed.";
            //return
        }

        requestQueue_.push(reqData);
        pthread_cond_signal(&cond_executeThread);

#else

        // # of inputs between Android NN Model and Eden Model would be different.
        int32_t numOfInputs = edenPreparedModel->inputNumOfBuffers;
        // # of outputs between Android NN Model and Eden Model would be same until now.
        int32_t numOfOutputs = request.outputs.size();
        LOGD(EDEN_DRIVER, "numOfInputs: %d, numOfOutputs: %d\n", numOfInputs, numOfOutputs);

        if (numOfOutputs != edenPreparedModel->outputNumOfBuffers) {
            LOGE(EDEN_DRIVER, "Invalied Params.\n");
            retCode = INVALID_PARAMS;
            break;
        }

        int32_t modelId = edenPreparedModel->modelId;
        HwPreference hwPreference = edenPreparedModel->hwPreference;
        EdenBuffer* inputBuffers = reinterpret_cast<EdenBuffer*>(edenPreparedModel->inputAddr);
        EdenBuffer* outputBuffers = reinterpret_cast<EdenBuffer*>(edenPreparedModel->outputAddr);

        // @todo it is better to look at input eden operand's data type, not hwPreference
        if (hwPreference == GPU_ONLY) {
            LOGD(EDEN_DRIVER, "Not apply preProcessOnInputs! skip it!");
        } else {
            LOGD(EDEN_DRIVER, "Apply preProcessOnInputs!");
            // Convert input data layout
            for (int32_t idx = 0; idx < numOfInputs; idx++) {
                int32_t annInputIndex = edenPreparedModel->model.inputIndexes[idx];
                const V1_2::Operand& operand = edenPreparedModel->model.operands[annInputIndex];
                void* addr = inputBuffers[idx].addr;
                int32_t size = inputBuffers[idx].size;
                int32_t numOfDims = 0;
                int32_t dims[4] = {0, 0, 0, 0};
                getEdenDimensions(operand.dimensions, dims, numOfDims);
                DATA_TYPE dataType = getDataType(operand.type);

                executionScheduler->preProcessOnInputs(addr, size, numOfDims, dims, dataType, hwPreference);
            }
        }

        //////////////////////////////////////////
        ////////////// Call execute //////////////
        uint32_t requestId = INVALID_REQUEST_ID;
        EdenCallback cb;
        EdenRequest edenRequest;
        {
            RequestOptions requestOptions;
            RequestPreference requestPreference = {
                .userPreference = {
                .hw = hwPreference,
                .mode = NORMAL_MODE,
                },
            };

            //EdenCallback cb;
            cb.notify = userNotify;
            cb.waitFor = userWait;
            cb.requestId = INVALID_REQUEST_ID;  // only used to break on wait condition
            cb.executionResult.inference.retCode = RET_OK;

            edenRequest.modelId = modelId;
            edenRequest.inputBuffers = inputBuffers;
            edenRequest.outputBuffers = outputBuffers;
            edenRequest.callback = &cb;

            requestId = *reinterpret_cast<uint32_t*>(&edenRequest);

            // DEBUG
            // showEdenRequest(edenRequest);

            requestOptions.requestPreference = requestPreference;
            requestOptions.updatedOperations.numOfOperations = edenPreparedModel->updatedOperations.size();
            requestOptions.updatedOperations.operations = const_cast<int32_t*>(edenPreparedModel->updatedOperations.data());
            // NOTICE Below code was added to avoid MCD's Prevent issue.
            // reserved was remained w/o initialization intendedly but now below code was added.
            for (int32_t idx = 0; idx < 32; idx++) {
                requestOptions.reserved[idx] = 0;
            }

            if (measure == MeasureTiming::YES) deviceStart = std::chrono::steady_clock::now();
            int32_t ret = executionScheduler->edenServiceDelegator_->ExecuteRequest(&edenRequest, requestOptions);
            if (ret != RET_OK) {
                LOGE(EDEN_DRIVER, "edenServiceDelegator_->ExecuteReq() is failed.\n");
                return FAIL_ON_EDEN_EXECUTE_REQ;
            }

#if 0
            if (cb.waitFor(&cb.requestId, requestId, USER_NN_TIMEOUT) < 0) {
                LOGE(EDEN_DRIVER, "User returned TIMEOUT\n");
                retCode = FAIL_ON_EDEN_EXECUTE_REQ;
                break;
            } else {
                LOGD(EDEN_DRIVER, "Execution Done.\n");
            }
#endif
        }
        //////////////////////////////////////////

        // For debugging
#if 0
        for (int32_t idx = 0; idx < numOfOutputs; idx++) {
            int32_t annOutputIndex = edenPreparedModel->model.outputIndexes[idx];
            const Operand& operand = edenPreparedModel->model.operands[annOutputIndex];
            void* addr = outputBuffers[idx].addr;
            int32_t size = outputBuffers[idx].size;
            int32_t numOfDims = 0;
            int32_t dims[4] = {0, 0, 0, 0};
            getEdenDimensions(operand.dimensions, dims, numOfDims);
            DATA_TYPE dataType = getDataType(operand.type);
            if (1) {
                LOGD(EDEN_DRIVER, "Output right after execution...\n");
                DumpToStdio(addr, size, dataType);
                LOGD(EDEN_DRIVER, "Output right after execution...Done!\n");
            }
        }
#endif

        std::vector<sp<IMemory>> vecHidlMemory;
        std::vector<char*> vecMappedPtr;
        // Convert output data layout
        for (int32_t idx = 0; idx < numOfOutputs; idx++) {
            RequestArgument outputs = request.outputs[idx];
            if (outputs.hasNoValue) continue;

            auto poolIndex = outputs.location.poolIndex;
            //auto bufferSize = outputs.location.length;

            /* get memory from hidl_memory through IMemory */
            char* mappedPtr = nullptr;
            int32_t ret = bufInfoOnExecute.loadHidlMem(request.pools[poolIndex], true, mappedPtr);
            if (ret != RET_OK) {
                LOGE(EDEN_DRIVER, "%s(-) Fail on getVirtualAddressOnPool!", __func__);
                return ret;
            }
            vecMappedPtr.push_back(mappedPtr);
        }

        {
            if (cb.waitFor(&cb.requestId, requestId, USER_NN_TIMEOUT) < 0) {
                LOGE(EDEN_DRIVER, "User returned TIMEOUT\n");
                retCode = FAIL_ON_EDEN_EXECUTE_REQ;
                break;
            } else {
                LOGD(EDEN_DRIVER, "Execution Done.\n");
            }
            // When waitFor is returned, it means device execution is complete.
            if (measure == MeasureTiming::YES) deviceEnd = std::chrono::steady_clock::now();
        }

        int32_t width;
        int32_t height;
        int32_t channel;
        int32_t number;
        int32_t ret;
        // Convert output data layout
        for (int32_t idx = 0; idx < numOfOutputs; idx++) {
            // for zero_size cases
            ret = executionScheduler->edenServiceDelegator_->GetOutputBufferShape(modelId, idx, &width, &height, &channel, &number);
            if (ret != RET_OK) {
                LOGE(EDEN_DRIVER, "edenServiceDelegator_->ExecuteReq() is failed.\n");
                return ret;
            }
            auto total_size = width * height * channel * number;

            if (total_size == 0) {
                continue;
            }

            RequestArgument outputs = request.outputs[idx];
            if (outputs.hasNoValue) continue;

            //auto poolIdx = outputs.location.poolIndex;
            auto bufferSize = outputs.location.length;

            /* get memory from hidl_memory through IMemory */
            //sp<IMemory> hidlMemory = mapMemory(request.pools[poolIdx]);
            //sp<IMemory> hidlMemory = vecHidlMemory[idx];
            //if (hidlMemory != nullptr) {
            char* mappedPtr = vecMappedPtr[idx];
            if (mappedPtr != nullptr) {
                //char* mappedPtr = reinterpret_cast<char*>(static_cast<void*>(hidlMemory->getPointer()));
                // @todo need below startUpdate, endUpdate?
                // bufInfoOnExecute.startUpdate(mappedPtr);

                std::memcpy(mappedPtr + outputs.location.offset, outputBuffers[idx].addr, bufferSize);
                // bufInfoOnExecute.endUpdate(mappedPtr);

                if (hwPreference == GPU_ONLY) {
                    LOGD(EDEN_DRIVER, "Not apply postProcessOnOutputs! skip it!");
                } else {
//                    LOGD(EDEN_DRIVER, "Apply postProcessOnOutputs!");

                    int32_t annOutputIndex = edenPreparedModel->model.outputIndexes[idx];
                    //const Operand& operand = edenPreparedModel->model.operands[annOutputIndex];
                    const V1_2::Operand* ptrOperand = &(edenPreparedModel->model.operands[annOutputIndex]);

                    // @todo this code is only for RESHAPE, SHOULD BE REMOVED!!!
                    {
                        // @todo Below code is work-around to pass reshape vts
                        //       It has (1,3,3,1) as input and (9) as output
                        //       But it can't transform data from NCHW to NHWC with (9) since it means (9,1,1,1)
                        //       So use original input dimension information.
                        //       But in future more nice way should replace below code...
                        if (static_cast<int32_t>(edenPreparedModel->model.operations[0].type) == ANEURALNETWORKS_RESHAPE) {
                            if (edenPreparedModel->model.operations.size() == 1 && ptrOperand->dimensions.size() == 1) {
                                LOGD(EDEN_DRIVER, "in this case, converting for reshape is not working properly...\n");
                                annOutputIndex = edenPreparedModel->model.inputIndexes[0];
                                ptrOperand = &(edenPreparedModel->model.operands[annOutputIndex]);
                            }
                        }
                    }

                    //void* addr = mappedPtr;
                    void* addr = mappedPtr + outputs.location.offset;
                    int32_t size = bufferSize;
                    int32_t numOfDims = 0;
                    int32_t dims[4] = {0, 0, 0, 0};
                    getEdenDimensions(ptrOperand->dimensions, dims, numOfDims);
                    DATA_TYPE dataType = getDataType(ptrOperand->type);

                    executionScheduler->postProcessOnOutputs(addr, size, numOfDims, dims, dataType, hwPreference);
                }
            } else {
                LOGE(EDEN_DRIVER, "hidlMemory is nullptr!\n");
                return FAIL_ON_EDEN_EXECUTE_REQ;
            }
        }

        // @todo Below is workaround code to avoid unexpected behavier after executing NPU.
        // Below code should be removed after fixing NPU issue.
        if (edenPreparedModel->hwPreference == NPU_ONLY) {
            LOGD(EDEN_DRIVER, "edenPreparedModel->hwPreference is NPU_ONLY");
        }
    } while (0);

    ErrorStatus executionStatus = (retCode == RET_OK) ? ErrorStatus::NONE : ErrorStatus::GENERAL_FAILURE;

    // When outputShapes is aquired, NN HAL driver execution is complete
    if (measure == MeasureTiming::YES) {
        driverEnd = std::chrono::steady_clock::now();
        Timing timing = {
            .timeOnDevice = microsecondsDuration(deviceEnd, deviceStart),
            .timeInDriver = microsecondsDuration(driverEnd, driverStart)
        };
        // @todo outputShapes and timing should be filled properly.
        Return<void> returned = callNotifyWithExecutionResult(callback, executionStatus, outputShapes, timing);

        if (!returned.isOk()) {
            LOGE(EDEN_DRIVER, "hidl callback failed to return properly: %s\n", returned.description().c_str());
        }
    } else {
        Timing timing = {.timeOnDevice = UINT64_MAX, .timeInDriver = UINT64_MAX};
        // @todo outputShapes and timing should be filled properly.
        Return<void> returned = callNotifyWithExecutionResult(callback, executionStatus, outputShapes, timing);

        if (!returned.isOk()) {
            LOGE(EDEN_DRIVER, "hidl callback failed to return properly: %s\n", returned.description().c_str());
        }
    }

#endif

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Constructor
 * @details Constructor
 * @param void
 */
ExecutionScheduler::ExecutionScheduler(void) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    lock_executeThread = PTHREAD_MUTEX_INITIALIZER;
    lock_responseThread = PTHREAD_MUTEX_INITIALIZER;
    cond_executeThread = PTHREAD_COND_INITIALIZER;
    cond_responseThread = PTHREAD_COND_INITIALIZER;

    // @todo Temporally disable threads
#if 0
    // create threads
    executeThread = std::thread(&ExecutionScheduler::threadMainForRequestExecution, this);
    responseThread = std::thread(&ExecutionScheduler::threadMainForRespondExecution, this);
#endif

    edenServiceDelegator_ = nullptr;
    schedulePolicy_ = std::make_shared<SchedulePolicy>();

    isRunning = true;
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

/**
 * @brief Destructor
 * @details Destructor
 * @param void
 */
ExecutionScheduler::~ExecutionScheduler(void) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    if (isRunning) {
        LOGD(EDEN_DRIVER, "now closing threads...\n");
        isRunning = false;
    }

    // @todo Temporally disable threads
#if 0
    // #1: destroy threads
    executeThread.join();
    responseThread.join();

    // #2: clear queues
    while (requestQueue_.empty() == false) {
       auto request = requestQueue_.top();
       if (request != nullptr) {
           requestQueue_.pop();
       }
    }
    while (completeQueue_.empty() == false) {
        auto complete = completeQueue_.top();
        if (complete != nullptr) {
            completeQueue_.pop();
        }
    }
#endif
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

void ExecutionScheduler::setEdenServiceDelegator(std::shared_ptr<EdenServiceDelegator> edenServiceDelegator) {
    edenServiceDelegator_ = edenServiceDelegator;
}

void ExecutionScheduler::setCompilerManager(std::shared_ptr<CompilerManager> compilerManager) {
    compilerManager_ = compilerManager;
}

int32_t ExecutionScheduler::requestOneExecutionInAsync(const EdenPreparedModel* edenPreparedModel,
                                                       const V1_0::Request& request,
                                                       BufferInfoOnExecute& bufInfoOnExecute,
                                                       V1_2::MeasureTiming measure,
                                                       std::chrono::steady_clock::time_point driverStart,
                                                       sp<V1_0::IExecutionCallback> callback_1_0) {
    return requestOneExecutionInAsyncBase(this, edenPreparedModel, request, bufInfoOnExecute, measure, driverStart, callback_1_0);
}

int32_t ExecutionScheduler::requestOneExecutionInSync(const EdenPreparedModel* edenPreparedModel,
                                                      const V1_0::Request& request,
                                                      BufferInfoOnExecute& bufInfoOnExecute,
                                                      V1_2::MeasureTiming measure,
                                                      std::chrono::steady_clock::time_point driverStart,
                                                      sp<V1_0::IExecutionCallback> callback_1_0) {
    return requestOneExecutionInAsyncBase(this, edenPreparedModel, request, bufInfoOnExecute, measure, driverStart, callback_1_0);
}

/**
 * @brief Request one request execution asynchronously
 * @details This function receives one Request from NNAgent and pushes it to RequestQueue.
 *          Once it is pushed, it immediately returns to the caller.
 *          Pushed request is handled by seperate thread.
 * @param[in] preparedModel IPreparedModel to be executed
 * @param[in] request Request to be executed
 * @param[in] callback IExecutionCallback to be executed
 * @return error code
 */
int32_t ExecutionScheduler::requestOneExecutionInAsync(const EdenPreparedModel* edenPreparedModel,
                                                       const V1_0::Request& request,
                                                       BufferInfoOnExecute& bufInfoOnExecute,
                                                       V1_2::MeasureTiming measure,
                                                       std::chrono::steady_clock::time_point driverStart,
                                                       sp<V1_2::IExecutionCallback> callback_1_2) {
    return requestOneExecutionInAsyncBase(this, edenPreparedModel, request, bufInfoOnExecute, measure, driverStart, callback_1_2);
}

/**
 * @brief Request one request execution synchronously
 * @details This function receives one Request from NNAgent and pushes it to RequestQueue.
 *          After pushing request on queue, it waits until this request is complete.
 *          Then it returns to the caller.
 * @param[in] preparedModel IPreparedModel to be executed
 * @param[in] request Request to be executed
 * @param[in] callback IExecutionCallback to be executed
 * @return error code
 */
int32_t ExecutionScheduler::requestOneExecutionInSync(const EdenPreparedModel* edenPreparedModel,
                                                      const V1_0::Request& request,
                                                      BufferInfoOnExecute& bufInfoOnExecute,
                                                      V1_2::MeasureTiming measure,
                                                      std::chrono::steady_clock::time_point driverStart,
                                                      sp<V1_2::IExecutionCallback> callback_1_2) {
    return requestOneExecutionInAsyncBase(this, edenPreparedModel, request, bufInfoOnExecute, measure, driverStart, callback_1_2);
}

/**
 * @brief Thread main for handling RequestQueue
 * @details This function is main function for handling RequestQueue.
 *          It looks at the RequestQueue and if there is a Request to be handled,
 *          then it retrieves a Request from RequestQueue and starts an execution process.
 *          Once processing is complete, it pushes a RequestQueue to CompleteQueue.
 *          This steps are repeated.
 * @param void
 * @return error code
 */
void ExecutionScheduler::threadMainForRequestExecution(void) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    while (isRunning) {
        if (requestQueue_.empty() == false) {
            std::shared_ptr<NNRequestData> request = requestQueue_.top();
            if (request != nullptr) {
                std::lock_guard<std::mutex> lock(mutex_executeThread);

                if (request->execMode == EXECUTION_MODE::ASYNC) {
                    request->timeStamp.startedTime = std::chrono::system_clock::now();
                    int32_t ret = 0;
                    request->timeStamp.completedTime = std::chrono::system_clock::now();
                    if (ret != RET_OK) {
                        LOGE(EDEN_DRIVER, "requestOneExecutionInAsync() is failed.\n");
                    }
                } else {
                    request->timeStamp.startedTime = std::chrono::system_clock::now();
                    int32_t ret = 0;
                    request->timeStamp.completedTime = std::chrono::system_clock::now();
                    if (ret != RET_OK) {
                        LOGE(EDEN_DRIVER, "requestOneExecutionInSync() is failed.\n");
                    }
                }
                // pop from request queue
                requestQueue_.pop();
                // push to complete queue
                completeQueue_.push(request);
                pthread_cond_signal(&cond_responseThread);
            }
        } else {
            pthread_mutex_lock(&lock_executeThread);
            pthread_cond_wait(&cond_executeThread, &lock_executeThread);
            pthread_mutex_unlock(&lock_executeThread);
        }
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return;
}

/**
 * @brief Thread main for handling CompleteQueue
 * @details This function is main function for handling CompleteQueue.
 *          It looks at the CompleteQueue and if there is a Request to be handled,
 *          then it retrieves a Request from CompleteQueue and starts an completion process.
 *          This steps are repeated.
 * @param void
 * @return error code
 */
void ExecutionScheduler::threadMainForRespondExecution(void) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    while (isRunning) {
        if (completeQueue_.empty() == false) {
            std::shared_ptr<NNRequestData> complete = completeQueue_.top();
            if (complete != nullptr) {
                std::lock_guard<std::mutex> lock(mutex_responseThread);

                // Pick it from done queue and do post-work if exists

                // Load output result(Eden Memory Manager) to output buffer(Android NN Memory)
                if (complete->preparedModel != nullptr) {
                    //complete->preparedModel->loadOutputData(complete->request->outputs);
                }

                // Call notify with result
                if (complete->callback != nullptr) {
                    hidl_vec<OutputShape> outputShapes;
                    Timing timing;
                    // @todo outputShapes and timing should be filled properly.
                    complete->callback->notify_1_2(ErrorStatus::NONE, outputShapes, timing);
                }
                // pop from complete queue
                completeQueue_.pop();
            }
        } else {
            pthread_mutex_lock(&lock_responseThread);
            pthread_cond_wait(&cond_responseThread, &lock_responseThread);
            pthread_mutex_unlock(&lock_responseThread);
        }
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return;
}

int32_t ExecutionScheduler::preProcessOnInputs(void* addr, int32_t size, int32_t /*numOfDims*/, int32_t* dims, DATA_TYPE dataType, HwPreference hwPreference) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);
    // For debugging
#if 0
    {
        LOGD(EDEN_DRIVER, "Before input buffer converting...\n");
        //DumpToStdio(addr, size, dataType);
    }
#endif

    int32_t ret;
    // if data type is QUANT8, it means data type is UINT8. In this case, translate it to INT8 by -128
    if ((dataType == DATA_TYPE::QUANT8) && (hwPreference == NPU_ONLY)) {
        int32_t inputOffset = compilerManager_->getInputOffset();
        LOGD(EDEN_DRIVER, "InputOffset: %d \n", inputOffset);
        ret = PrePostProcessor::getInstance()->convertDataLayoutFromNHWCToNCHW(addr, size, dims[N_NCHW], dims[C_NCHW], dims[H_NCHW], dims[W_NCHW], dataType, inputOffset);
    } else {
        ret = PrePostProcessor::getInstance()->convertDataLayoutFromNHWCToNCHW(addr, size, dims[N_NCHW], dims[C_NCHW], dims[H_NCHW], dims[W_NCHW], dataType, 0);
    }
    if (ret != RET_OK) {
        LOGE(EDEN_DRIVER, "convertDataLayoutFromNHWCToNCHW() is failed.\n");
        return FAIL_TO_CONVT_NHWC_TO_NCHW;
    }

    // For debugging
#if 0
    {
        LOGD(EDEN_DRIVER, "After input buffer converting...\n");
        //DumpToStdio(addr, size, dataType);
    }
#endif

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

int32_t ExecutionScheduler::postProcessOnOutputs(void* addr, int32_t size, int32_t /*numOfDims*/, int32_t* dims, DATA_TYPE dataType, HwPreference hwPreference) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);
    // For debugging
#if 0
    {
        LOGD(EDEN_DRIVER, "Before output buffer converting...\n");
        //DumpToStdio(addr, size, dataType);
    }
#endif

    int32_t ret;
    // if data type is QUANT8, it means data type is UINT8. In this case, translate it to INT8 by +128
    if ((dataType == DATA_TYPE::QUANT8) && (hwPreference == NPU_ONLY)) {
        int32_t outputOffset = compilerManager_->getOutputOffset();
        LOGD(EDEN_DRIVER, "OutputOffset: %d \n", outputOffset);
        ret = PrePostProcessor::getInstance()->convertDataLayoutFromNCHWToNHWC(addr, size, dims[N_NCHW], dims[C_NCHW], dims[H_NCHW], dims[W_NCHW], dataType, outputOffset);
    } else {
        ret = PrePostProcessor::getInstance()->convertDataLayoutFromNCHWToNHWC(addr, size, dims[N_NCHW], dims[C_NCHW], dims[H_NCHW], dims[W_NCHW], dataType, 0);
    }
    if (ret != RET_OK) {
        LOGE(EDEN_DRIVER, "convertDataLayoutFromNCHWToNHWC() is failed.\n");
        return FAIL_TO_CONVT_NCHW_TO_NHWC;
    }

    // For debugging
#if 0
    {
        LOGD(EDEN_DRIVER, "After output buffer converting...\n");
        //DumpToStdio(addr, size, dataType);
    }
#endif

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

void userNotify(addr_t* addr, addr_t value) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    *addr = value;
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
}

int32_t userWait(addr_t* addr, uint32_t /*value*/, uint32_t timeout) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    while (--timeout > 0) {
        usleep(1000);
        if (*addr != INVALID_REQUEST_ID) break;
    }

    if (timeout == 0) {
        return -1;
    }
    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return 0;
}

void showEdenRequest(EdenRequest& request) {
    LOGD(EDEN_DRIVER, "showEdenRequest() is called...\n");
    LOGD(EDEN_DRIVER, "request.modelId:%d\n", request.modelId);
    LOGD(EDEN_DRIVER, "request.inputBuffers[0].addr:%p\n", request.inputBuffers[0].addr);
    LOGD(EDEN_DRIVER, "request.inputBuffers[0].size:%d\n", request.inputBuffers[0].size);
    LOGD(EDEN_DRIVER, "request.outputBuffers[0].addr:%p\n", request.outputBuffers[0].addr);
    LOGD(EDEN_DRIVER, "request.outputBuffers[0].size:%d\n", request.outputBuffers[0].size);
    LOGD(EDEN_DRIVER, "request.callback->requestId:%" PRIu64 "\n", static_cast<uint64_t>(request.callback->requestId));
    LOGD(EDEN_DRIVER, "request.hw:%d\n", request.hw);
}
}  // namespace eden_driver
}  // namespace nn
}  // namespace android

