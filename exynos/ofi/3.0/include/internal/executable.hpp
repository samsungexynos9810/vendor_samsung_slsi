#ifndef __SEVA_EXECUTABLE_HPP
#define __SEVA_EXECUTABLE_HPP

#include "types.hpp"
#include "memory.hpp"

namespace seva
{
namespace graph
{
class Graph;
class Executable {
public:
    enum class Direction {
        IN = 0,
        OUT,
        INTER,
    };

public:
    virtual ~Executable() {}
    /**
     * @brief Initialize Executable Engine
     */
    virtual bool Init() = 0;
    /**
     * @brief De-initialize Executable Engine
     */
    virtual bool Deinit() = 0;
    /**
     * @brief Available Executable Engine
     */
    virtual bool IsAvailable() = 0;
    /**
     * @brief Load cgo file.
     * @param [in] path the cgo file path
     * @param [out] modelId model identifier
     * @return on Success, true is returned. On failure false is returned.
     */
    virtual bool Load(const char* path, int* modelId) = 0;
    /**
     * @brief Load cgo file.
     * @param [in] path the merged cgos file path
     * @param [in] the name of compiled graph
     * @param [out] modelId model identifier
     * @return on Success, true is returned. On failure false is returned.
     */
    virtual bool Load(const char* path, const char* graphName, int* modelId) = 0;
    /**
     * @brief Load cgo from memory.
     * @param [in] cgoBuffer pointer to the memory contains cgo.
     * @param [in] cgoSize size of cgo stored in memory.
     * @param [out] model identifier
     * @return on Success, true is returned. On failure false is returned.
     */
    virtual bool Load(char *cgoBuffer, int cgoSize, int* modelId) = 0;
    /**
     * @brief Unload graph.
     * @param [in] modelId model identifier which returns at Load function
     * @return on Success, true is returned. On failure false is returned.
     */
    virtual bool Unload(int modelId) = 0;
    /**
     * @brief Do sanity check before running graph.
     * @param [in] modelId model identifier which returns at Load function
     * @return on Success, true is returned. On failure false is returned.
     */
    virtual bool Verify(int modelId) = 0;
    /**
     * @brief Do sanity check before running graph.
     * @param [in] modelId model identifier which returns at Load function
     * @return on Success, true is returned. On failure false is returned.
     */
    virtual bool Verify(Graph& graph, int* modelId) = 0;
    /**
     * @brief Execute graph with given modeId
     * @param [in] modelId model identifier which returns at Load function
     * @return on Success, true is returned. On failure false is returned.
     */
    virtual bool Execute(int modelId) = 0;
    virtual bool Execute(Graph& graph, int modelId) = 0;
    virtual int RegisterCallback(int modelId, int nodeId, void* cb, void* data) = 0;
    virtual MemoryInfo CreateMemory(int size, Memory::Type type) = 0;
    virtual MemoryInfo CreateMemoryExtFd(int size, int fd, void* vaddr) = 0;
    virtual MemoryInfo CreateMemoryExtFdOffset(int size, int fd, int offset, void* vaddr) = 0;
    virtual void SetBufferToEngine(int modelId, MemoryInfo memInfo, Direction direction, int id, int c, int w, int h) = 0;
    /**
     * @brief Set input/output buffer
     * @param [in] modelId model identifier which returns at Load function
     * @param [in] buffer buffer pointer
     * @param [in] direction buffer director Executable::Direction::IN or Executable::Direction::OUT
     * @param [in] id buffer index (start from 0 for each input/output)
     */
    virtual void SetBufferToEngine(int modelId, BufferPtr& buffer, Direction direction, int id) = 0;
    virtual void SetUserParamToEngine(int modelId, MemoryInfo memInfo, int id) = 0;
    /**
     * @brief Set user parameter
     * @param [in] modelId model identifier which returns at Load function
     * @param [in] param UserParam pointer
     * @param [in] id user param index (start from 0)
     */
    virtual void SetUserParamToEngine(int modelId, UserParamPtr& param, int id) = 0;
    virtual bool ReleaseMemory(MemoryInfo info) = 0;
    virtual bool ReleaseMemoryExtFd(MemoryInfo info) = 0;
};
}
}
#endif
