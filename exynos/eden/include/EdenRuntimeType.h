/*
 * Copyright (C) 2018 Samsung Electronics Co. LTD
 *
 * This software is proprietary of Samsung Electronics.
 * No part of this software, either material or conceptual may be copied or distributed, transmitted,
 * transcribed, stored in a retrieval system or translated into any human or computer language in any form by any means,
 * electronic, mechanical, manual or otherwise, or disclosed
 * to third parties without the express written permission of Samsung Electronics.
 */

/**
 * @file    EdenRuntimeType.h
 * @brief   This describs EDEN Runtime Types
 */

#ifndef RUNTIME_INCLUDE_EDENRUNTIMETYPE_H_
#define RUNTIME_INCLUDE_EDENRUNTIMETYPE_H_

#include <stdint.h>      // uint32_t, uint8_t

static const char* const NPU_NCP = "NCP";
static const char* const NORMALIZATION = "Normalization";
static const char* const QUANTIZATION = "Quantization";
static const char* const DEQUANTIZATION = "Dequantization";
static const char* const PAD = "PAD";
static const char* const SOFTMAX = "SOFTMAX";
static const char* const INVERSE_CFU = "InverseCFU";

static const char* const NCP_BINARY = "NCP_BINARY";
static const char* const NCP_NAME = "NCP_NAME";
static const char* const IFM = "IFM";
static const char* const TENSOR = "Tensor";
static const char* const MEAN = "MEAN";
static const char* const SCALE = "SCALE";
static const char* const FRAC_LEN = "FRAC_LEN";
static const char* const PADDING_SHAPE = "PADDING_SHAPE";
static const char* const PADDING_VALUE = "PADDING_VALUE";
static const char* const SOFTMAX_OPTIONS = "Softmax";
static const char* const COLS_IN_CELL = "COLS_IN_CELL";
static const char* const LINES_IN_CELL = "LINES_IN_CELL";
static const char* const INTERLEAVED_SLICES = "INTERLEAVED_SLICES";
static const char* const IDPS = "IDPS";
static const char* const UNITSIZE = "UNITSIZE";

typedef enum {
    RT_SUCCESS = 0,
    RT_FAILED = 1,

    RT_INIT_FAILED = 2,
    RT_NPU_INIT_FAILED = 3,
    RT_GPU_INIT_FAILED = 4,
    RT_CPU_INIT_FAILED = 5,

    RT_OPEN_FAILED = 6,
    RT_NPU_OPEN_FAILED = 7,
    RT_GPU_OPEN_FAILED = 8,
    RT_CPU_OPEN_FAILED = 9,

    RT_EXECUTE_FAILED = 10,
    RT_NPU_EXECUTE_FAILED = 11,
    RT_GPU_EXECUTE_FAILED = 12,
    RT_CPU_EXECUTE_FAILED = 13,

    RT_CLOSE_FAILED = 14,
    RT_NPU_CLOSE_FAILED = 15,
    RT_GPU_CLOSE_FAILED = 16,
    RT_CPU_CLOSE_FAILED = 17,

    RT_SHUTDOWN_FAILED = 18,
    RT_NPU_SHUTDOWN_FAILED = 19,
    RT_GPU_SHUTDOWN_FAILED = 20,
    RT_CPU_SHUTDOWN_FAILED = 21,

    RT_PREPARE_CALLBACK_FAILED = 22,
    RT_EXECUTE_CALLBACK_FAILED = 23,

    RT_MODEL_INVALID = 24,
    RT_PARAM_INVALID = 25,

    RT_SKIP_TO_CONVERT_BUFFER,
    RT_SKIP_TO_CONVERT_INFO,
    RT_FAIL_TO_CONVERT_BUFFER,
    RT_FAIL_TO_CONVERT_INFO,

    RT_FAIL_TO_ALLOCATE_MEM,
    RT_FAIL_TO_COPY_MEM,
    RT_FAIL_TO_FREE_MEM,

    RT_MODEL_ID_IS_INVALID,
    RT_EDEN_OP_IS_INVALID,
    RT_NOT_SUPPORTED_CUSTOM_OP_LIST,
    RT_TIMEOUT_ON_REGISTERING_MODEL_TO_EDEN,
    RT_FAIL_TO_GET_EDEN_MODEL,
    RT_FAIL_TO_GET_TARGET_BUFFER_ADDR,
    RT_FAIL_TO_GET_BUFFER_DIMENTION,
    RT_FAIL_TO_ALLOCATE_EDEN_MEMORY,
    RT_FAIL_TO_BUILD_PRE_TASK_FOR_NPU,
    RT_FAIL_TO_BUILD_POST_TASK_FOR_NPU,
    RT_INVALID_INDEX_ON_OPERAND,
    RT_INVALID_BUFFER_LAYOUT,
    RT_INVALID_OPERAND_FOR_OPERATION,
    RT_FAIL_TO_NPU_PREPARE_REQ,

    RT_INVALID_NN_API_TYPE,
    RT_NO_AVAILABLE_DEVICE,
    RT_NO_MORE_MODEL_OPEN,
    RT_SERVICE_NOT_AVAILABLE,

    RT_ERROR_ON_CREATE_MODEL_FROM_TFLITE_MODEL,
    RT_ERROR_ON_CREATE_MODEL_FROM_MEMORY,
    RT_ERROR_ON_EDEN_MODEL_CLOSE_MODEL,

    RT_INVALID_PREFERENCEHW,
    RT_ERROR_ON_REGISTER_MODEL,
    RT_ERROR_ON_UNREGISTER_MODEL,
    RT_ERROR_ON_ALLOCATE_BUFFERS_FOR_INPUT_OPERANDS,
    RT_ERROR_ON_ALLOCATE_BUFFERS_FOR_OUTPUT_OPERANDS,
    RT_ERROR_ON_ALLOCATE_BUFFERS_FOR_TEMP_OPERANDS,
    RT_ERROR_ON_ALLOCATE_BUFFERS_FOR_OPERANDS,
    RT_ERROR_ON_FREE_BUFFERS_FOR_OPERANDS,
    RT_ERROR_ON_GET_MATCHED_EMA_BUFFERS,
    RT_ERROR_ON_GET_DEVICE_HEALTH_STATE,

    RT_UNSUPPORTED_FEATURES,

    RT_FAILED_DATA_TAILOR,

    // RT STUB specific errors
    RT_ERROR_ON_INIT_HELPER,
    RT_ERROR_ON_DEINIT_HELPER,
    RT_ERROR_ON_PREPARE_OPEN_MODEL_FROM_MEMORY,
    RT_ERROR_ON_CALL_OPEN_MODEL_FROM_MEMORY,
    RT_ERROR_ON_UNPREPARE_OPEN_MODEL_FROM_MEMORY,
    RT_ERROR_ON_ALLOC_ION_BUFFER,
    RT_ERROR_ON_RELEASE_ION_BUFFER,

    RT_ERROR_ON_GET_SERVICE_ASHMEM,
    RT_ERROR_ON_ALLOCATE_ASHMEM,

    RT_BUSY,
    RT_INVALID_MODEL_ID,
    RT_SET_MODEL_ID_FAILED,
} RtRet;

/** @todo
 * consider struct name can show more information
 */
typedef struct __ParsedData {
    uint8_t inputCount;
    uint8_t outputCount;
    double* mean;
    double* scale;
    uint32_t channel;
    uint32_t height;
    uint32_t width;
    int32_t* quantFracLen;
    int32_t* dequantFracLen;
    uint32_t outputSize;
} ParsedData;

#endif  // RUNTIME_INCLUDE_EDENRUNTIMETYPE_H_

