
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file MemoryData.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief Implements MemoryData used by a MemoryManager, which can be shared among multiple ITask
 * @details
 */
#ifndef HTGS_MEMORYDATA_HPP
#define HTGS_MEMORYDATA_HPP

#include <stddef.h>
#include <htgs/core/queue/PriorityBlockingQueue.hpp>
#include <htgs/types/MMType.hpp>
#include <htgs/api/IMemoryAllocator.hpp>
#include <htgs/api/IMemoryReleaseRule.hpp>
#include <htgs/api/IData.hpp>

namespace htgs {
/**
 * @class MemoryData MemoryData.hpp <htgs/api/MemoryData.hpp>
 *
 * @brief Describes memory allocated by a MemoryManager to manage shared memory across multiple ITask.
 * @details
 * Provides mechanisms for allocation, freeing, and memory release strategies associated
 * with memory. Using these mechanisms the MemoryManager can process the memory and recycle it only
 * when the IMemoryReleaseRule indicates it is ready to be released.
 *
 * In order to properly share this data, it should be forwarded along with other IData until the ITask
 * responsible for releasing the data back to its MemoryManager is called.
 *
 * To receive memory from the MemoryManager use ITask::getMemory.
 * To send memory to the MemoryManager use ITask::releaseMemory
 *
 * Use htgs::m_data_t<Type> from <htgs/types/Types.hpp> when handling MemoryData to reduce code size.
 *
 * Example Usage:
 * @code
 *   ITask::executeTask(std::shared_ptr<Data1> data)
 *   {
 *     ...
 *     // Shared memory getter
 *     // If you don't use m_data_t -> std::shared_ptr<htgs::MemoryData<int>>
 *     htgs::m_data_t<int> readBuffer = this->memGet<int>("readMemory", new ReleaseCountRule(1));
 *     readData(data->getFile(), readBuffer->get());
 *
 *     // Shared memory release example
 *     this->releaseMemory("otherMemory", data->getMemory());
 *
 *     addResult(new Data2(readBuffer));
 *     ...
 *   }
 * @endcode
 *
 * Example attaching MemoryManager:
 * @code
 * taskGraph->addMemoryManagerEdge("readMemory", readTask, new ReadMemoryAllocator(), readMemoryPoolSize);
 * @endcode
 * @tparam T the type of memory to be held by the MemoryData, this will automatically be converted to a pointer type
 */
template<class T>
class MemoryData : public IData {
 public:
  /**
   * Creates MemoryData with the specified IMemoryAllocator
   * @param allocator the memory allocator
   * @param address the address of the memory manager that allocates the memory data
   * @param memoryManagerName the name of the memory manager that allocated this memory
   * @param type the type of the memory manager that allocated this memory
   */
  MemoryData(std::shared_ptr<IMemoryAllocator<T>> allocator,
             std::string address,
             std::string memoryManagerName,
             MMType type) {
    this->type = type;
    this->address = address;
    this->memoryManagerName = memoryManagerName;
    this->allocator = allocator;
    if (allocator != nullptr)
      this->size = allocator->size();
    else
      this->size = 0;
    this->pipelineId = 0;
    this->memoryReleaseRule = nullptr;
    this->memory = nullptr;
  }

  /**
   * Destructor that releases IMemoryAllocator memory
   */
  ~MemoryData() {
    if (memoryReleaseRule) {
      delete memoryReleaseRule;
      memoryReleaseRule = nullptr;
    }
  }

  /**
   * Gets the address of the memory manager that allocated this memory data
   * @return the address of the memory manager that allocated the memory data
   */
  const std::string &getAddress() const {
    return address;
  }

  /**
   * Sets the pipelineId associated with the MemoryManager that allocated the memory
   * @param id the pipielineId
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  void setPipelineId(size_t id) { this->pipelineId = id; }

  /**
   * Gets the pipelineId associated with the MemoryManager that allocated the memory
   * @return the pipelineId
   */
  size_t getPipelineId() const { return this->pipelineId; }

  /**
   * Gets the size of the memory that was allocated
   * @return the memory size
   */
  size_t getSize() const { return this->size; }

  /**
   * Sets the memory release rule
   * @param rule the new rule that manages the memory
   */
  void setMemoryReleaseRule(IMemoryReleaseRule *rule) {
    if (memoryReleaseRule) {
      delete memoryReleaseRule;
      memoryReleaseRule = nullptr;
    }
    this->memoryReleaseRule = rule;
  }

  /**
   * Gets the memory release rule associated with the memory data
   * @return the memory release rule
   */
  IMemoryReleaseRule *getMemoryReleaseRule() const {
    return this->memoryReleaseRule;
  }

  /**
   * Checks whether the memory can be recycled/released by the MemoryManager
   * @return whether the memory is ready to be recycled/released by the MemoryManager
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  bool canReleaseMemory() {
    return this->memoryReleaseRule->canReleaseMemory();
  }

  /**
   * Updates the state of the memory when it is received by the MemoryManager
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  void memoryUsed() {
    this->memoryReleaseRule->memoryUsed();
  }

  /**
   * Allocates the memory that this MemoryData is using
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  void memAlloc() { this->memory = this->allocator->memAlloc(); }

  /**
   * Gets the memory that this MemoryData is managing
   * @return the memory attached to the MemoryData
   */
  T *get() { return this->memory; }

  /**
   * Gets the data that is held by this memory data at the specified index
   * @param idx the index
   * @return the data
   */
  const T &get(size_t idx) const {
    return *(this->memory + idx);
  }

  /**
   * Gets the data that is held by this memory data at the specified index
   * @param idx the index
   * @return the data
   */
  T &get(size_t idx) {
    return *(this->memory + idx);
  }

  /**
   * Gets the value for memory at index.
   * @param idx the index
   * @return the value at index
   * @note T must be a pointer to memory to use this functionality.
   */
  const T &operator[](size_t idx) const {
    return *(this->memory + idx);
  }

  /**
   * Gets the value for memory at index.
   * @param idx the index
   * @return the value at index
   * @note T must be a pointer to memory to use this functionality.
   */
  T &operator[](size_t idx) {
    return *(this->memory + idx);
  }

  /**
   * Frees the memory that this MemoryData is managing
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  void memFree() {
    if (this->memory) {
      this->allocator->memFree(this->memory);
      this->memory = nullptr;
    }
  }

  /**
   * Gets the type of memory that is associated with the memory manager
   * @return the type of memory (either Dynamic or Static)
   */
  MMType getType() const {
    return type;
  }

  /**
   * Creates a copy of this MemoryData
   * @return the copy
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  MemoryData<T> *copy() {
    return new MemoryData<T>(this->allocator,
                             this->address,
                             this->memoryManagerName,
                             this->type);
  }

  /**
   * Allocates the memory that this memory data is managed with the specified size
   * @param size the number of elements to allocate
   */
  void memAlloc(size_t size) {
    this->memory = this->allocator->memAlloc(size);
    this->size = size;
  }

  /**
   * Gets the memory manager name
   * @return the memory manager name
   */
  const std::string &getMemoryManagerName() const {
    return memoryManagerName;
  }

 private:
  MMType type; //!< The type of memory manager
  std::string memoryManagerName; //!< The name of the memory manager that allocated the memory
  std::string address; //!< The address of the memory manager, used to release back
  size_t pipelineId; //!< The pipelineId associated with where this memory was managed
  T *memory; //!< The memory
  size_t size; //!< The size of the memory (in elements)
  IMemoryReleaseRule *memoryReleaseRule; //!< The memory release rule associated with the memory
  std::shared_ptr<IMemoryAllocator<T>> allocator; //!< The allocator associated with the memory
};
}

#endif //HTGS_MEMORY_HPP
