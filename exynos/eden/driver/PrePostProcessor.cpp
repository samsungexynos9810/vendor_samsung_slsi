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
 * @file    PrePostProcessor.cpp
 * @brief   This is PrePostProcessor class file.
 * @details This header defines PrePostProcessor class.
 *          This class is implementing pre/post processing such as data layout change.
 * @author  minsu.jeon (minsu.jeon@samsung.com)
 */

#include <iostream>
#include <cstring>  // memcpy
#include <memory>   // shared_ptr

#include "log.h"
#include "Utils.h"  // convertToV1_0, convertToV1_1, android::nn::initVLogMask, logModelToInfo, DRIVER

#include "PrePostProcessor.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EdenDriver::PrePostProcessor"

namespace android {
namespace nn {
namespace eden_driver {

/**
 * @brief Convert data layout from NCHW to NHWC
 * @details This function converts data layout from NCHW to NWHC on a given addr and size.
 * @param[in] addr buffer address to be converted
 * @param[in] size buffer size
 * @param[in] number
 * @param[in] channel
 * @param[in] height
 * @param[in] width
 * @returns return code
 */
template<typename T>
bool convertToNHWC(void* addr, int32_t size, int32_t number, int32_t channel, int32_t height, int32_t width) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    T* input = reinterpret_cast<T*>(addr);
    std::shared_ptr<T> spTemp(new T[size], std::default_delete<T[]>());
    T* ptr = spTemp.get();
    LOGD(EDEN_DRIVER, "addr=%p, size=%d, number=%d, channel=%d, height=%d, width=%d, tempPtr=%p\n",
                       addr, size, number, channel, height, width, reinterpret_cast<void*>(ptr));

    for (int n = 0; n < number; ++n) {
        for (int c = 0; c < channel; ++c) {
            for (int h = 0; h < height; ++h) {
                for (int w = 0; w < width; ++w) {
                    ptr[((n * height + h) * width + w) * channel + c] = *(input++);
                }
            }
        }
    }
    std::memcpy(addr, ptr, size);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return true;
}

/**
 * @brief Convert data layout from NHWC to NCHW
 * @details This function converts data layout from NCHW to NWHC on a given addr and size.
 * @param[in] addr buffer address to be converted
 * @param[in] size buffer size
 * @param[in] number
 * @param[in] channel
 * @param[in] height
 * @param[in] width
 * @returns return code
 */
template<typename T>
bool convertToNCHW(void* addr, int32_t size, int32_t number, int32_t channel, int32_t height, int32_t width) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    T* input = reinterpret_cast<T*>(addr);
    std::shared_ptr<T> spTemp(new T[size], std::default_delete<T[]>());
    T* ptr = spTemp.get();
    LOGD(EDEN_DRIVER, "addr=%p, size=%d, number=%d, channel=%d, height=%d, width=%d, tempPtr=%p\n",
                       addr, size, number, channel, height, width, reinterpret_cast<void*>(ptr));

    for (int n = 0; n < number; ++n) {
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                for (int c = 0; c < channel; ++c) {
                    ptr[((n * channel + c) * height + h) * width + w] = *(input++);
                }
            }
        }
    }
    std::memcpy(addr, ptr, size);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return true;
}

/**
 * @brief Convert data layout from NHWC to CNHW
 * @details This function converts data layout from NHWC to CNHW on a given addr and size.
 * @param[in] addr buffer address to be converted
 * @param[in] size buffer size
 * @param[in] number
 * @param[in] channel
 * @param[in] height
 * @param[in] width
 * @returns return code
 */
template<typename T>
bool convertToCNHW(void* addr, int32_t size, int32_t number, int32_t channel, int32_t height, int32_t width) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    T* input = reinterpret_cast<T*>(addr);
    std::shared_ptr<T> spTemp(new T[size], std::default_delete<T[]>());
    T* ptr = spTemp.get();
    LOGD(EDEN_DRIVER, "addr=%p, size=%d, number=%d, channel=%d, height=%d, width=%d, tempPtr=%p\n",
                       addr, size, number, channel, height, width, reinterpret_cast<void*>(ptr));

    for (int n = 0; n < number; ++n) {
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                for (int c = 0; c < channel; ++c) {
                    ptr[((c * number + n) * height + h) * width + w] = *(input++);
                }
            }
        }
    }
    std::memcpy(addr, ptr, size);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return true;
}

/**
 * @brief Convert data layout from NHWC to CNHW with offset
 * @details This function converts data layout from NHWC to CNHW on a given addr and size.
 * @param[in] addr buffer address to be converted
 * @param[in] size buffer size
 * @param[in] number
 * @param[in] channel
 * @param[in] height
 * @param[in] width
 * @param[in] offset
 * @returns return code
 */
template<typename T>
bool convertToCNHWWithOffset(void* addr, int32_t size, int32_t number, int32_t channel, int32_t height, int32_t width, int32_t offset) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    T* input = reinterpret_cast<T*>(addr);
    std::shared_ptr<T> spTemp(new T[size], std::default_delete<T[]>());
    T* ptr = spTemp.get();
    LOGD(EDEN_DRIVER, "addr=%p, size=%d, number=%d, channel=%d, height=%d, width=%d, tempPtr=%p\n",
                       addr, size, number, channel, height, width, reinterpret_cast<void*>(ptr));

    for (int n = 0; n < number; ++n) {
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                for (int c = 0; c < channel; ++c) {
                    ptr[((c * number + n) * height + h) * width + w] = (*(input++) + offset);
                }
            }
        }
    }
    std::memcpy(addr, ptr, size);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return true;
}

/**
 * @brief Convert data layout from NCHW to NHWC with offset
 * @details This function converts data layout from NCHW to NWHC on a given addr and size.
 * @param[in] addr buffer address to be converted
 * @param[in] size buffer size
 * @param[in] number
 * @param[in] channel
 * @param[in] height
 * @param[in] width
 * @param[in] offset
 * @returns return code
 */
template<typename T>
bool convertToNHWCWithOffset(void* addr, int32_t size, int32_t number, int32_t channel, int32_t height, int32_t width, int32_t offset) {
    LOGD(EDEN_DRIVER, "%s() is called.\n", __func__);

    T* input = reinterpret_cast<T*>(addr);
    std::shared_ptr<T> spTemp(new T[size], std::default_delete<T[]>());
    T* ptr = spTemp.get();
    LOGD(EDEN_DRIVER, "addr=%p, size=%d, number=%d, channel=%d, height=%d, width=%d, tempPtr=%p\n",
                       addr, size, number, channel, height, width, reinterpret_cast<void*>(ptr));

    for (int n = 0; n < number; ++n) {
        for (int c = 0; c < channel; ++c) {
            for (int h = 0; h < height; ++h) {
                for (int w = 0; w < width; ++w) {
                    ptr[((n * height + h) * width + w) * channel + c] = (*(input++) + offset);
                }
            }
        }
    }
    std::memcpy(addr, ptr, size);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return true;
}

/**
 * @brief Convert data layout from NHWC to NCHW with offset
 * @details This function converts data layout from NCHW to NWHC on a given addr and size.
 * @param[in] addr buffer address to be converted
 * @param[in] size buffer size
 * @param[in] number
 * @param[in] channel
 * @param[in] height
 * @param[in] width
 * @param[in] offset
 * @returns return code
 */
template<typename T>
bool convertToNCHWWithOffset(void* addr, int32_t size, int32_t number, int32_t channel, int32_t height, int32_t width, int32_t offset) {
    LOGD(EDEN_DRIVER, "%s() is called.\n", __func__);

    T* input = reinterpret_cast<T*>(addr);
    std::shared_ptr<T> spTemp(new T[size], std::default_delete<T[]>());
    T* ptr = spTemp.get();
    LOGD(EDEN_DRIVER, "addr=%p, size=%d, number=%d, channel=%d, height=%d, width=%d, tempPtr=%p\n",
                       addr, size, number, channel, height, width, reinterpret_cast<void*>(ptr));

    for (int n = 0; n < number; ++n) {
        for (int h = 0; h < height; ++h) {
            for (int w = 0; w < width; ++w) {
                for (int c = 0; c < channel; ++c) {
                    ptr[((n * channel + c) * height + h) * width + w] = (*(input++) + offset);
                }
            }
        }
    }
    std::memcpy(addr, ptr, size);

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return true;
}

/**
 * @brief Convert data layout from NCHW to NHWC
 * @details This function converts data layout from NCHW to NWHC on a given addr and size.
 * @param[in] addr buffer address to be converted
 * @param[in] size buffer size
 * @param[in] number
 * @param[in] channel
 * @param[in] height
 * @param[in] width
 * @param[in] dataType
 * @param[in] offset
 * @returns return code
 */
int32_t PrePostProcessor::convertDataLayoutFromNCHWToNHWC(void* addr, int32_t size,
                                                          int32_t number, int32_t channel,
                                                          int32_t height, int32_t width,
                                                          DATA_TYPE dataType,
                                                          int32_t offset) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    if (addr == nullptr || size == 0) {
        LOGD(EDEN_DRIVER, "Input has no value... Skip converting.\n");
        return RET_OK;
    }

    if (offset == 0) {
        switch (dataType) {
        case DATA_TYPE::QUANT8:
        case DATA_TYPE::BOOL8:
            convertToNHWC<int8_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::RELAXED_FLOAT32:
        case DATA_TYPE::FLOAT16:
            // Since there's no float16, use int16_t instead to match data size to 2byte
            convertToNHWC<int16_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::FLOAT32:
            convertToNHWC<float>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT8:
        case DATA_TYPE::UINT8:
            convertToNHWC<int8_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT16:
        case DATA_TYPE::UINT16:
            convertToNHWC<int16_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT32:
        case DATA_TYPE::UINT32:
            convertToNHWC<int32_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT64:
        case DATA_TYPE::UINT64:
            convertToNHWC<int64_t>(addr, size, number, channel, height, width);
            break;
        default:
            LOGE(EDEN_DRIVER, "Oops, dataType=%d is not yet supported!", static_cast<int32_t>(dataType));
            break;
        }
    } else {
        switch (dataType) {
        case DATA_TYPE::QUANT8:
        case DATA_TYPE::BOOL8:
            convertToNHWCWithOffset<int8_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::RELAXED_FLOAT32:
        case DATA_TYPE::FLOAT16:
            // Since there's no float16, use int16_t instead to match data size to 2byte
            convertToNHWCWithOffset<int16_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::FLOAT32:
            convertToNHWCWithOffset<float>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::INT8:
        case DATA_TYPE::UINT8:
            convertToNHWCWithOffset<int8_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::INT16:
        case DATA_TYPE::UINT16:
            convertToNHWC<int16_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT32:
        case DATA_TYPE::UINT32:
            convertToNHWCWithOffset<int32_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::INT64:
        case DATA_TYPE::UINT64:
            convertToNHWCWithOffset<int64_t>(addr, size, number, channel, height, width, offset);
            break;
        default:
            LOGE(EDEN_DRIVER, "Oops, dataType=%d is not yet supported!", static_cast<int32_t>(dataType));
            break;
        }
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}

/**
 * @brief Convert data layout from NWHC to NCHW
 * @details This function converts data layout from NHWC to NCHW on a given addr and size.
 * @param[in] addr buffer address to be converted
 * @param[in] size buffer size
 * @param[in] number
 * @param[in] channel
 * @param[in] height
 * @param[in] width
 * @param[in] dataType
 * @returns return code
 */
int32_t PrePostProcessor::convertDataLayoutFromNHWCToNCHW(void* addr, int32_t size,
                                                          int32_t number, int32_t channel,
                                                          int32_t height, int32_t width,
                                                          DATA_TYPE dataType,
                                                          int32_t offset) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    if (addr == nullptr || size == 0) {
        LOGD(EDEN_DRIVER, "Input has no value... Skip converting.\n");
        return RET_OK;
    }

    if (offset == 0) {
        switch (dataType) {
        case DATA_TYPE::QUANT8:
        case DATA_TYPE::BOOL8:
            convertToNCHW<int8_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::RELAXED_FLOAT32:
        case DATA_TYPE::FLOAT16:
            // Since there's no float16, use int16_t instead to match data size to 2byte
            convertToNCHW<int16_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::FLOAT32:
            convertToNCHW<float>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT8:
        case DATA_TYPE::UINT8:
            // Since there's no float16, use int16_t instead to match data size to 2byte
            convertToNCHW<int8_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT16:
        case DATA_TYPE::UINT16:
            // Since there's no float16, use int16_t instead to match data size to 2byte
            convertToNCHW<int16_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT32:
        case DATA_TYPE::UINT32:
            convertToNCHW<int32_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT64:
        case DATA_TYPE::UINT64:
            convertToNCHW<int64_t>(addr, size, number, channel, height, width);
            break;
        default:
            LOGE(EDEN_DRIVER, "Oops, dataType=%d is not yet supported!", static_cast<int32_t>(dataType));
            break;
        }
    } else {
        switch (dataType) {
        case DATA_TYPE::QUANT8:
        case DATA_TYPE::BOOL8:
            convertToNCHWWithOffset<int8_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::RELAXED_FLOAT32:
        case DATA_TYPE::FLOAT16:
            // Since there's no float16, use int16_t instead to match data size to 2byte
            convertToNCHWWithOffset<int16_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::FLOAT32:
            convertToNCHWWithOffset<float>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::INT8:
        case DATA_TYPE::UINT8:
            convertToNCHWWithOffset<int8_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::INT16:
        case DATA_TYPE::UINT16:
            convertToNCHWWithOffset<int16_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::INT32:
        case DATA_TYPE::UINT32:
            convertToNCHWWithOffset<int32_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::INT64:
        case DATA_TYPE::UINT64:
            convertToNCHWWithOffset<int64_t>(addr, size, number, channel, height, width, offset);
            break;
        default:
            LOGE(EDEN_DRIVER, "Oops, dataType=%d is not yet supported!", static_cast<int32_t>(dataType));
            break;
        }
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}
/**
 * @brief Convert data layout from NHWC to CNHW
 * @details This function converts data layout from NHWC to CNHW on a given addr and size.
 * @param[in] addr buffer address to be converted
 * @param[in] size buffer size
 * @param[in] number
 * @param[in] channel
 * @param[in] height
 * @param[in] width
 * @param[in] dataType
 * @returns return code
 */
int32_t PrePostProcessor::convertDataLayoutFromNHWCToCNHW(void* addr, int32_t size,
                                                          int32_t number, int32_t channel,
                                                          int32_t height, int32_t width,
                                                          DATA_TYPE dataType,
                                                          int32_t offset) {
    LOGD(EDEN_DRIVER, "%s(+)\n", __func__);

    if (addr == nullptr || size == 0) {
        LOGD(EDEN_DRIVER, "Input has no value... Skip converting.\n");
        return RET_OK;
    }

    if (offset == 0) {
        switch (dataType) {
        case DATA_TYPE::QUANT8:
        case DATA_TYPE::BOOL8:
            convertToCNHW<int8_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::RELAXED_FLOAT32:
        case DATA_TYPE::FLOAT16:
            // Since there's no float16, use int16_t instead to match data size to 2byte
            convertToCNHW<int16_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::FLOAT32:
            convertToCNHW<float>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT8:
        case DATA_TYPE::UINT8:
            // Since there's no float16, use int16_t instead to match data size to 2byte
            convertToCNHW<int8_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT16:
        case DATA_TYPE::UINT16:
            // Since there's no float16, use int16_t instead to match data size to 2byte
            convertToCNHW<int16_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT32:
        case DATA_TYPE::UINT32:
            convertToCNHW<int32_t>(addr, size, number, channel, height, width);
            break;
        case DATA_TYPE::INT64:
        case DATA_TYPE::UINT64:
            convertToCNHW<int64_t>(addr, size, number, channel, height, width);
            break;
        default:
            LOGE(EDEN_DRIVER, "Oops, dataType=%d is not yet supported!", static_cast<int32_t>(dataType));
            break;
        }
    } else {
        switch (dataType) {
        case DATA_TYPE::QUANT8:
        case DATA_TYPE::BOOL8:
            convertToCNHWWithOffset<int8_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::RELAXED_FLOAT32:
        case DATA_TYPE::FLOAT16:
            // Since there's no float16, use int16_t instead to match data size to 2byte
            convertToCNHWWithOffset<int16_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::FLOAT32:
            convertToCNHWWithOffset<float>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::INT8:
        case DATA_TYPE::UINT8:
            convertToCNHWWithOffset<int8_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::INT16:
        case DATA_TYPE::UINT16:
            convertToCNHWWithOffset<int16_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::INT32:
        case DATA_TYPE::UINT32:
            convertToCNHWWithOffset<int32_t>(addr, size, number, channel, height, width, offset);
            break;
        case DATA_TYPE::INT64:
        case DATA_TYPE::UINT64:
            convertToCNHWWithOffset<int64_t>(addr, size, number, channel, height, width, offset);
            break;
        default:
            LOGE(EDEN_DRIVER, "Oops, dataType=%d is not yet supported!", static_cast<int32_t>(dataType));
            break;
        }
    }

    LOGD(EDEN_DRIVER, "%s(-)\n", __func__);
    return RET_OK;
}
}  // namespace eden_driver
}  // namespace nn
}  // namespace android

