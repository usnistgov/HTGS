
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file MemoryManager.hpp
 * @author Timothy Blattner
 * @date Nov 27, 2015
 *
 * @brief Implements the MemoryManager class that processes MemoryData between two ITasks.
 */
#ifndef HTGS_MEMORYMANAGER_H
#define HTGS_MEMORYMANAGER_H

#include "MemoryPool.hpp"

#include "../../api/ITask.hpp"
#include "../../api/IMemoryAllocator.hpp"
#include "htgs/types/MMType.hpp"

namespace htgs {

/**
 * @class MemoryManager MemoryManager.hpp <htgs/core/memory/MemoryManager.hpp>
 * @brief Processes MemoryData between two ITasks using a memory pool.
 * @details
 * The memory pool is allocated using the IMemoryAllocator interface. As soon
 * as data is available in the pool, it is pushed to the ITask associated with the memory.
 *
 * The memoryPoolSize should be sufficient to process the algorithm based on the release
 * rules added to the MemoryData when an ITask gets the memory. These release rules should
 * be satisfied by the traversal of data specified based on algorithm and system memory capacity requirements.
 *
 * There are three types of memory managers:
 * (1) Static
 * (2) Dynamic
 * (3) User Managed
 *
 * Static memory managers allocate memory at initialization, and free memory during shutdown.
 *
 * Dynamic memory managers do not allocate memory. Memory allocation is moved to the ITask. Memory returned from an
 * ITask will be freed when rules are satisfied.
 *
 * User managed memory managers do not allocate or free memory. All memory allocation and freeing is left up to the
 * user to manage. The manager acts as a throttling mechanism that can be used to ensure memory limits are satisfied.
 *
 * @tparam T the input/output MemoryData type for the MemoryManager; i.e., double *
 */
template<class T>
class MemoryManager: public ITask<MemoryData<T>, MemoryData<T>> {


 public:
  /**
   * Creates the MemoryManager with the specified memory pool size and allocator.
   * Specifies 1 thread and that it should immediately start executing as soon as a thread is bound to it.
   * @param name the name of the memory manager edge
   * @param memoryPoolSize the size of the memory pool.
   * @param memoryAllocator the allocator for how the memory pool allocates the memory.
   * @param type the type of memory manager to create
   */
  MemoryManager(std::string name, size_t memoryPoolSize, std::shared_ptr<IMemoryAllocator<T>> memoryAllocator, MMType type) : ITask<
      MemoryData<T>,
      MemoryData<T>>(1, true, false, 0L) {
    this->allocator = memoryAllocator;
    this->memoryPoolSize = memoryPoolSize;
    this->pool = new MemoryPool<T>(memoryPoolSize);
    this->name = name;
    this->type = type;
  }

  /**
   * Destructor
   */
  ~MemoryManager() override {
    if (type == htgs::MMType::Static)
      pool->releaseAllMemory();

    delete pool;
    pool = nullptr;
  }

  /**
   * Initializes the MemoryManager, filling the memory pool with allocated data.
   * All the memory is allocated once a thread has been bound to ITask.
   */
  void initialize() override {
    this->pipelineId = this->getPipelineId();
    MemoryData<T> *memory = new MemoryData<T>(this->allocator, this->getName());

    bool allocate = false;
    if (type == MMType::Static)
      allocate = true;

    this->pool->fillPool(memory, pipelineId, allocate);
    this->pipelineConnectorList = this->getPipelineConnectorList();
    delete memory;
  }

  /**
   * Shuts down the MemoryManager releasing memory that is inside of the pool.
   */
  void shutdown() override{
    bool release = false;
    if (type == MMType::Static)
      release = true;
    this->pool->emptyPool(release);
  }

  /**
   * Processes memory data.
   * When data enters the MemoryManager, if the data is nullptr, then it will add
   * all data to its output edge. If the data is not nullptr, then the MemoryData::memoryUsed()
   * is called to update the state of the memory and checks if the memory can be recycled back into
   * the memory pool with MemoryData::canReleaseMemory().
   * @param data the MemoryData being processed
   */
  void executeTask(std::shared_ptr<MemoryData<T>> data) override {
    if (data != nullptr) {
      if (data->getPipelineId() == this->pipelineId) {

        if (type == MMType::UserManaged) {
          this->pool->addMemory(data);
        }
        else {
          data->memoryUsed();

          if (data->canReleaseMemory()) {
            if (type == MMType::Static)
              this->pool->addMemory(data);
            else if (type == MMType::Dynamic) {
              data->memFree();
              this->pool->addMemory(data);
            }
          }
        }
      }
      else {
        std::cerr << "Memory manager received data from another pipeline!" << std::endl;
        std::shared_ptr<Connector<MemoryData<T>>> connector = std::static_pointer_cast<Connector<MemoryData<T>>>(this->pipelineConnectorList->at(
            (unsigned long) data->getPipelineId()));
        connector->produceData(data);
      }
    }

    while (!this->pool->isPoolEmpty()) {
      this->addResult(this->pool->getMemory());
    }
  }

  /**
   * Provides debug output for MemoryManager
   */
  void debug() override {
    DEBUG(this->getName() << " max pool size: " << this->memoryPoolSize << " isEmpty? " << this->pool->isPoolEmpty());
  }

  /**
   * Gets the name of the MemoryManager
   * @return
   */
  virtual std::string getName() override {
    std::string typeStr;
    switch (this->type) {
      case MMType::Static:
        typeStr = "static";
        break;
      case MMType::Dynamic:
        typeStr = "dynamic";
        break;
      case MMType::UserManaged:
        typeStr = "user managed";
        break;
    }
    return std::string("MM(" + typeStr + "): " + this->name);
  }

  /**
   * Creates a shallow copy of the MemoryManager.
   * Does not copy the contents of the MemoryPool.
   * @return the shallow copy of the MemoryManager
   */
  virtual MemoryManager<T> *copy() override {
    return new MemoryManager<T>(this->name, this->memoryPoolSize, this->allocator, this->type);
  }

  /**
   * Gets the size of the MemoryPool
   * @return
   */
  size_t getMemoryPoolSize() {
    return this->memoryPoolSize;
  }

  /**
   * Gets the allocator that is responsible for allocating and freeing memory for the MemoryPool.
   * @return the allocator
   */
  std::shared_ptr<IMemoryAllocator<T>> getAllocator() {
    return this->allocator;
  }

  /**
   * Gets the name of the memory manager.
   * Will match the name of the memory edge that it is managing.
   * @return the name of the memory manager
   */
  std::string getMemoryManagerName() { return name; }

  /**
    * Gets the demangled type name of the connector
    * @return the demangled type name
    */
  std::string typeName()
  {
    int status;
    char *realName = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    std::string ret(realName);

    free(realName);

    return ret;

  }


  /**
   * Gets the memory manager type.
   * @return the memory manager type.
   */
  MMType getType() const { return type; }

  virtual std::string genDot(int flags,
                             std::string dotId,
                             std::shared_ptr<AnyConnector> input,
                             std::shared_ptr<AnyConnector> output) override {

    if ((flags & DOTGEN_FLAG_HIDE_MEM_EDGES) != 0) {
      return "";
    }

    std::ostringstream oss;

    oss << input->getDotId() << " -> " << dotId << "[color=sienna];" << std::endl;
    oss << input->getDotId() + "[label=\"" + this->typeName() + "\",shape=box,style=filled,shape=oval,width=.2,height=.2, fillcolor=sienna, color=sienna];\n";

    if (output != nullptr) {
      oss << dotId << " -> " << output->getDotId() << "[color=sienna];" << std::endl;
      oss << output->getDotId() + "[label=\""+ this->typeName() +"\",shape=box,style=filled,shape=oval,width=.2,height=.2, fillcolor=sienna, color=sienna];\n";
    }

    oss << dotId + "[label=\"" + this->getName() + "\",color=sienna];\n";

    return oss.str();

  }

#ifdef PROFILE
  std::string getDotProfile(int flags,
                            std::unordered_map<std::string, double> *mmap, double val,
                            std::string desc, std::unordered_map<std::string, std::string> *colorMap) override
  {
    return "";
  }
#endif

 private:
  std::shared_ptr<ConnectorVector> pipelineConnectorList; //!< The list of execution pipeline connectors one for each MemoryManager of the same type
  std::shared_ptr<IMemoryAllocator<T>> allocator; //!< The allocator used for allocating and freeing memory
  size_t memoryPoolSize; //!< The size of the memory pool
  MemoryPool<T> *pool; //!< The memory pool
  size_t pipelineId; //!< The execution pipeline id
  std::string name; //!< The name of the memory manager
  MMType type; //!< The memory manager type

};
}

#endif //HTGS_MEMORYMANAGER_H

