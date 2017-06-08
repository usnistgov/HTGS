
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file MemoryPool.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief Implements the MemoryPool class
 */
#ifndef HTGS_MEMORYPOOL_HPP
#define HTGS_MEMORYPOOL_HPP

#include <list>
#include <iostream>
#include <memory>
#include "../../types/Types.hpp"
#include "htgs/debug/debug_message.hpp"
#include "../queue/BlockingQueue.hpp"

namespace htgs {

template<class V>
class MemoryData;

/**
 * @class MemoryPool MemoryPool.hpp <htgs/core/memory/MemoryPool.hpp>
 * @brief Creates a pool of memory that allocates/frees MemoryData.
 * @details
 * Currently the memory pool is static and cannot grow dynamically, although dynamic memory
 * allocation can be added in the future, for the GPU it is better to preallocate the memory
 * prior to execution to avoid unnecessary GPU synchronization.
 *
 * @note This class should only be called by the HTGS API
 */
template<class T>
class MemoryPool {
 public:
  /**
   * Creates a memory pool with the specified size number of elements.
   * @param queueSize the number of elements in the memory pool.
   */
  MemoryPool(size_t queueSize) {
    this->memoryQueue = new BlockingQueue<m_data_t<T>>(queueSize);
    this->allMemory = new std::list<m_data_t<T>>();
    this->queueSize = queueSize;
  }

  /**
   * Destructor, deallocates all memory allocated by the MemoryPool.
   */
  ~MemoryPool() {
    delete memoryQueue;
    memoryQueue = nullptr;

    delete allMemory;
    allMemory = nullptr;

  }

  /**
   * Releases all memory associated with this memory pool.
   */
  void releaseAllMemory() {
    for (m_data_t<T> mem : *allMemory) {
      if (mem) {
        mem->memFree();
      }
    }
  }

  /**
   * Fills the pool with memory and specifies the pipelineId to be associated with the MemoryData.
   * @param memory the memory that is allocated.
   * @param pipelineId the pipelineId associated with the memory.
   * @param allocate whether to allocate the memory before adding
   */
  void fillPool(MemoryData<T> *memory, size_t pipelineId, bool allocate) const {
    size_t remainingSize = this->memoryQueue->remainingCapacity();

    DEBUG("Inserting " << remainingSize << " elements to memory pool");

    for (size_t i = 0; i < remainingSize; i++) {

      MemoryData<T> *newMemory = memory->copy();
      DEBUG_VERBOSE("Adding memory " << newMemory);

      newMemory->setPipelineId(pipelineId);

      // Allocates only if asked, used for dynamic and user mananaged memory
      if (allocate)
        newMemory->memAlloc();

      std::shared_ptr<MemoryData<T>> shrMem = std::shared_ptr<MemoryData<T>>(newMemory);
      this->memoryQueue->Enqueue(shrMem);
      this->allMemory->push_back(shrMem);
    }
  }

  /**
   * Creates a shallow copy of the MemoryPool
   * @return the copy of the MemoryPool
   */
  MemoryPool<T> *copy() { return new MemoryPool<T>(this->queueSize); }

  /**
   * Gets whether the pool is empty or not
   * @return whether the pool is empty
   * @retval TRUE if the pool is empty
   * @retval FALSE if the pool is not empty
   */
  bool isPoolEmpty() const { return this->memoryQueue->isEmpty(); }

  /**
   * Empties the memory pool releasing memory that had been allocated.
   */
  void emptyPool(bool free) const {
    size_t poolSize = this->memoryQueue->size();
    for (size_t i = 0; i < poolSize; i++) {
      m_data_t<T> memory = this->memoryQueue->remove();
      if (free)
        memory->memFree();

    }
  }

  /**
   * Gets the next piece of memory from the MemoryPool
   * @return the next memory in the MemoryPool
   */
  m_data_t <T> getMemory() const {
    return this->memoryQueue->Dequeue();
  }

  /**
   * Adds memory back into the MemoryPool
   * @param o the memory to be added in.
   */
  void addMemory(m_data_t <T> o) const {
    this->memoryQueue->Enqueue(o);
  }

 private:
  std::list<m_data_t <T>> *allMemory; //!< The list of all memory that has been allocated by the memory pool
  BlockingQueue <m_data_t<T>> *memoryQueue; //!< A blocking queue for getting/recycling memory
  size_t queueSize; //!< The size of the memory queue

};
}

#endif //HTGS_MEMORYPOOL_HPP
