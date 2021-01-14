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
 * @file    eden_memory_manager.h
 * @brief   This is EDEN Memory Manager class
 * @details This header defines EDEN Memory Manager class.
 *          This class is implementing the Eden Memory Manager
 * @author  minsu.jeon (minsu.jeon@samsung.com)
 */

#ifndef EDENMODEL_INCLUDE_EDEN_MEMORY_MANAGER_H_
#define EDENMODEL_INCLUDE_EDEN_MEMORY_MANAGER_H_

#include <cstdint>        // int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t
#include <map>  // map
#include <vector>         // vector

#include "eden_types.h"        // EdenBuffer, EdenMemType, ModelPreference, NnApiType
#include "eden_nn_types.h"     // NnRet
#include "eden_model_types.h"  // EdenDataType

#include "eden_memory.h"       // eden_memory_t

namespace eden {
namespace nn {

typedef struct __EdenMemoryInfo {
  int32_t numOfBuffers;
  EdenBuffer* buffers;
  int32_t* sizeOfBuffers;
  int32_t* operandIds;
  int32_t* allocSizeOfBuffers;
  eden_memory_t* emaBuffers;
} EdenMemoryInfo;

class EdenModel;

/**
 *  EdenMemoryManager class. This class implements EDEN Memory Manager.
 */
class EdenMemoryManager {
 public:
  /**
   * @brief EdenMemoryManager constructor
   * @details Initialize internal variables and resources
   * @param void
   */
  EdenMemoryManager(void);

  /**
   * @brief EdenMemoryManager destructor
   * @details Release internal resourses
   * @param void
   */
  virtual ~EdenMemoryManager(void);

  /**
   * @brief Register new model
   * @details This function allocates resources for a given model instance.
   * @param[in] modelId EdenModel instance id.
   * @returns return code
   */
  NnRet RegisterModel(EdenModel* model, ModelPreference preference);

  /**
   * @brief Allocate buffers for input operands
   * @details This function allocates memory for input operands.
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of EdenBuffer refering to
   *          input operands on model.
   * @param[in] modelId EdenModel instance id.
   * @param[in] memType Memory type for input operands.
   * @param[out] buffers Array of EdenBuffer*.
   * @param[out] numOfBuffers Number of item on buffers.
   * @returns return code
   */
  NnRet AllocateBuffersForInputOperands(uint32_t modelId, EdenMemType memType, EdenBuffer** buffers, int32_t* numOfBuffers);

  /**
   * @brief Allocate buffers for output operands
   * @details This function allocates memory for output operands.
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of EdenBuffer refering to
   *          output operands on model.
   * @param[in] modelId EdenModel instance id.
   * @param[in] memType Memory type for output operands.
   * @param[out] buffers Array of EdenBuffer*.
   * @param[out] numOfBuffers Number of item on buffers.
   * @returns return code
   */
  NnRet AllocateBuffersForOutputOperands(uint32_t modelId, EdenMemType memType, EdenBuffer** buffers, int32_t* numOfBuffers);

  /**
   * @brief Allocate buffers for operands by referring to given operandIds
   * @details This function allocates memory for operands matched to a given operandIds.
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of EdenBuffer refering to operandIds.
   * @param[in] modelId EdenModel instance id.
   * @param[in] memType Memory type for output operands.
   * @param[in] numOfOperand Number of operand id on operandIds.
   * @param[in] operandIds Array of operand id.
   * @param[out] buffers Array of EdenBuffer*.
   * @returns return code
   */
  NnRet AllocateBuffersForOperands(uint32_t modelId, EdenMemType memType, int32_t numOfOperand, int32_t* operandIds, EdenBuffer** buffers, bool cellAlign = false);

  /**
   * @brief Allocate buffers for an operand by referring to given operandId
   * @details This function allocates memory for an operand matched to a given operandId.
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of EdenBuffer refering to an operandId.
   * @param[in] modelId EdenModel instance id.
   * @param[in] memType Memory type for output operands.
   * @param[in] operandId Operand id on model.
   * @param[out] buffers Array of EdenBuffer*.
   * @returns return code
   */
  NnRet AllocateBuffersForOperand(uint32_t modelId, EdenMemType memType, int32_t operandId, EdenBuffer** buffers, bool cellAlign = false);

  NnRet AllocateBuffersForBridgeOperand(uint32_t modelId, addr_t refer, EdenMemType memType, int32_t operandId, EdenBuffer** buffers, bool cellAlign = false);
  NnRet GetBuffersForBridgeOperand(uint32_t modelId, addr_t refer, int32_t operandId, EdenBuffer** buffers, bool check = false);

  /**
   * @brief Reallocate buffers for a given oldBuffers
   * @details This function finds a matched EdenBuffers allocated before,
   *          and releases previously allocated one.
   *          Then it allocates new EdenBuffers with a given parameters,
   *          and replace it to old one.
   * @param[in] modelId EdenModel instance id.
   * @param[in] oldBuffers Array of EdenBuffer*.
   * @param[in] numOfBuffers Number of items on buffers.
   * @param[in] sizeOfBuffers Array of EdenBuffer*.
   * @param[out] newBuffers Array of EdenBuffer* allocated.
   * @returns return code
   */
  NnRet ReallocateBuffersForOperands(uint32_t modelId, addr_t requestId, EdenBuffer* oldBuffers, int32_t newNumOfBuffers, int32_t* newSizeOfBuffers);

  /**
   * @brief Get rellocated buffer information for a given modelId
   * @details This function gets reallocated buffer information for a given modelId,
   *          which is called via ReallocateBuffersForOperands().
   * @param[in] modelId EdenModel instance id
   * @param[out] bufferInfo Map between operandId to EdenBuffer*
   * @returns return code
   */
  NnRet GetReallocatedBufferInfo(uint32_t modelId, addr_t requestId, std::map<int32_t, EdenBuffer*>& bufferInfo);

  /**
   * @brief Clear rellocated buffer information for a given modelId
   * @details This function clear reallocated buffer information for a given modelId,
   *          which is called via ReallocateBuffersForOperands().
   * @param[in] modelId EdenModel instance id
   * @returns return code
   */
  NnRet ClearReallocatedBufferInfo(uint32_t modelId, addr_t requestId);

  /**
   * @brief Free resources allocated for buffers of a given modelId
   * @details This function releases a resources allocated for buffers of a given modelId.
   *          It first searches it on input resources, then output resources.
   * @param[in] modelId EdenModel instance id
   * @param[in] buffers Array of EdenBuffer*.
   * @returns return code
   */
  NnRet FreeBuffersForOperands(uint32_t modelId, EdenBuffer* buffers);

  /**
   * @brief Free temporal resources allocated for operandIds of a given modelId
   * @details This function releases a temporal resources allocated for operandIds of a given modelId.
   *          It only searches on temporal resources.
   * @param[in] modelId EdenModel instance id
   * @param[in] numOfOperand Number of operand id on operandIds.
   * @param[in] operandIds Array of operand id.
   * @returns return code
   */
  NnRet FreeTempBuffersForOperands(uint32_t modelId, int32_t numOfOperand, int32_t* operandIds);

  /**
   * @brief Free temporal resources allocated for an operandId of a given modelId
   * @details This function releases a temporal resources allocated for an operandId of a given modelId.
   *          It only searches on temporal resources.
   * @param[in] modelId EdenModel instance id
   * @param[in] operandId Operand id on model.
   * @returns return code
   */
  NnRet FreeTempBuffersForOperand(uint32_t modelId, int32_t operandId);

  /**
   * @brief Unregister new model id
   * @details This function releases resources for a given model id.
   * @param[in] modelId EdenModel instance id.
   * @returns return code
   */
  NnRet UnregisterModel(uint32_t modelId);

  /**
   * @brief Return EMABuffer pointer match to a given EdenBuffer
   * @details This function returns a pointer of EMABuffer which is pair of a given EdenBuffer.
   * @param[in] modelId EdenModel instance id.
   * @param[in] buffers pointer of EdenBuffer
   * @param[out] emaBuffers pointer of eden_memory_t
   * @returns pointer of eden_memory_t pair of a given EdenBuffer*
   */
  NnRet GetMatchedEMABuffers(uint32_t modelId, EdenBuffer* buffers, eden_memory_t** emaBuffers);

  /**
   * @brief Find a matched EMABuffer in terms of operandId and update a given emaBuffers
   * @details This function updates a given emaBuffers's values matched to a given operandId.
   *          Therefore, caller should make sure emaBuffers is not null.
   * @param[in] modelId EdenModel instance id.
   * @param[in] operandId Operand id on model.
   * @param[out] emaBuffers pointer of eden_memory_t
   * @returns return code
   */
  NnRet GetMatchedEMABuffers(uint32_t modelId, int32_t operandId, eden_memory_t* emaBuffers);

  /**
   * @brief Find a matched EMABuffer in terms of addr/size and update a given emaBuffers
   * @details This function updates a given emaBuffers's values matched to a given addr/size.
   *          Therefore, caller should make sure emaBuffers is not null.
   * @param[in] modelId EdenModel instance id.
   * @param[in] addr Virtual address of buffer, returned by EMM.
   * @param[in] size size of buffer, returned by EMM.
   * @param[out] emaBuffers pointer of eden_memory_t
   * @returns return code
   */
  NnRet GetMatchedEMABuffers(uint32_t modelId, void* addr, int32_t size, eden_memory_t* emaBuffers);

  /**
   * @brief Calculate required buffer size to store input based on Z-order
   * @details This function calculates required size for storing contents on buffer based on Z-order.
   * @param[in] width buffer width
   * @param[in] height buffer height
   * @param[in] channel number of channel
   * @param[in] numOfBuffers number of buffers
   * @returns Size for Z-order data
   */
  int32_t SizeForZOrder(int32_t width, int32_t height, int32_t channel, int32_t numOfBuffers);

  /**
   * @brief Dump internal memory info mapping
   * @details This function shows internal memory info mappings for a given modelId.
   * @param[in] modelId Model ID for EdenModel.
   * @returns void
   */
  void DumpInternalMemoryInfos(uint32_t modelId);

 private:
  /**
   * @brief Create EdenMemoryInfo for input operand
   * @details This function creates EdenMemoryInfo reflecting input operands.
   * @param[in] modelId EdenModel instance id.
   * @param[out] memInfo Created instance of EdenMemoryInfo
   * @returns return code
   */
  NnRet CreateMemoryInfoForInput(uint32_t modelId, EdenMemoryInfo** memInfo);

  /**
   * @brief Create EdenMemoryInfo for output operand
   * @details This function creates EdenMemoryInfo reflecting output operands.
   * @param[in] modelId EdenModel instance id.
   * @param[out] memInfo Created instance of EdenMemoryInfo
   * @returns return code
   */
  NnRet CreateMemoryInfoForOutput(uint32_t modelId, EdenMemoryInfo** memInfo);

  /**
   * @brief Create a new EdenMemoryInfo with given parameters.
   * @details This function creates a EdenMemoryInfo with a given parameters.
   * @param[in] numOfBuffers EdenMemoryInfo to be released
   * @param[in] sizeOfBuffers EdenMemoryInfo to be released
   * @param[out] memInfo EdenMemoryInfo to be released
   * @returns return code
   */
  NnRet CreateMemoryInfo(int32_t numOfBuffers, int32_t* sizeOfBuffers, int32_t* operandIds, EdenMemoryInfo** memInfo);

  /**
   * @brief Allocate buffers using given information
   * @details This function allocates memory using given informaion
   *          Allocated memory can be different based on a given memType.
   *          Allocated memory is returned via an array of EdenBuffer/eden_memory_t
   * @todo Proper error handling should be added in future to avoid memory leak
   * @param[in] numOfBuffers number of buffers to be allocated
   * @param[in] sizeOfBuffers array for each buffer size
   * @param[in] memType Memory type for input operands.
   * @param[out] buffers Array of EdenBuffer*.
   * @param[out] emaBuffers Array of eden_memory_t*
   * @returns return code
   */
  NnRet AllocateBuffers(int32_t numOfBuffers, int32_t* sizeOfBuffers, EdenMemType memType, EdenBuffer** buffers, eden_memory_t** emaBuffers);

  /**
   * @brief Release EdenMemoryInfo matched to a given buffers
   * @details This function releases a EdenMemoryInfo from a matched resource.
   * @param[in] vecMemInfo vector for EdenMemoryInfo of a specific model
   * @param[in] buffers Array of EdenBuffer*.
   * @returns return code
   */
  NnRet FindAndReleaseMemoryInfo(std::vector<EdenMemoryInfo*>& vecMemInfo, EdenBuffer* buffers);

  /**
   * @brief Release EdenMemoryInfo matched to a given operandIds
   * @details This function releases a EdenMemoryInfo from a matched resource.
   * @param[in] vecMemInfo vector for EdenMemoryInfo of a specific model
   * @param[in] numOfOperand Number of operand id on operandIds.
   * @param[in] operandIds Array of operand id.
   * @returns return code
   */
  NnRet FindAndReleaseMemoryInfo(std::vector<EdenMemoryInfo*>& vecMemInfo, int32_t numOfOperand, int32_t* operandIds);

  /**
   * @brief Release a given EdenMemoryInfo
   * @details This function releases a resources allocated for a given EdenMemoryInfo
   *          It does not release memInfo itself.
   * @param[in] memInfo EdenMemoryInfo to be released
   * @returns return code
   */
  NnRet ReleaseMemoryInfo(EdenMemoryInfo* memInfo);

  /**
   * @brief Get size of a given data type
   * @details This function returns size of data type in byte.
   * @todo Below utility function should be moved to other proper place, not in EdenModel class.
   * @param[in] dataType EdenDataType
   * @returns Size for Z-order data
   */
  int32_t GetSizeOfDataTypeInByte(EdenDataType dataType);

  NnRet GetSizeOfBufferForOperandId(uint32_t modelId, int32_t operandId, bool cellAlign, int32_t& sizeOfBuffer);

  /**
   * @brief Return EdenModel* matched to a given modelId
   * @details This function returns a EdenModel* matched to a given modelId.
   * @param[in] modelId Model ID for EdenModel.
   * @param[out] model Matched EdenModel*
   * @returns return code
   */
  NnRet GetEdenModel(uint32_t modelId, EdenModel*& model);

  /**
   * @brief Return NnApiType matched to a given modelId
   * @details This function returns a NnApiType matched to a given modelId.
   * @param[in] modelId Model ID for NnApiType.
   * @param[out] nnApiType Matched NnApiType.
   * @returns return code
   */
  NnRet GetNnApiType(uint32_t modelId, NnApiType& nnApiType);

  /**
   * @brief Check if modelId is registered on EMM
   * @details This function checks modelId if it is registered.
   * @param[in] modelId Model ID for EdenModel.
   * @returns true(Registered), false(Not registered)
   */
  bool IsRegisteredModel(uint32_t modelId);

  std::map<uint32_t, EdenModel*> mapModelIdToModel_;                               //*< Map for (modelId, EdenModel*)
  std::map<uint32_t, std::vector<EdenMemoryInfo*>> mapModelIdToInputMemoryInfo_;   //*< Map for (modelId, input buffers)
  std::map<uint32_t, std::vector<EdenMemoryInfo*>> mapModelIdToOutputMemoryInfo_;  //*< Map for (modelId, output buffers)

  // For Reallocate Buffer
  // Map for (requestId, map<operandId, EdenBuffer*>)
  std::map<addr_t, std::map<int32_t, EdenBuffer*>> mapRequestIdToReallocatedBufferInfo_;;
  // Map for (modelId, vector<requestId>)
  std::map<uint32_t, std::vector<addr_t>> mapModelIdToRellocatedRequest_;

  // For Bridge(Temp) Buffer
  // Map for (inputBuffer addr, map<operandId, EdenMemoryInfo*>)
  std::map<addr_t, std::map<int32_t, EdenMemoryInfo*>> mapBufferAddrToBridgeBufferInfo_;
  // Map for (modelId, vector<inputBuffer addr>)
  std::map<uint32_t, std::vector<addr_t>> mapModelIdToBridgeBufferAddr_;
  // Map for (modelId, vector<EdenMemoryInfo>)
  std::map<uint32_t, std::vector<EdenMemoryInfo*>> mapModelIdToTempMemoryInfo_;

  std::map<uint32_t, ModelPreference> mapModelIdToModelPreference_;                //*< Map for (modelId, ModelPreference)
};

}  // namespace nn
}  // namespace eden

#endif  // EDENMODEL_INCLUDE_EDEN_MEMORY_MANAGER_H_
