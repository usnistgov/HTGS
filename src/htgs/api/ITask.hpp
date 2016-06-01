
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file ITask.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief An interface to process input data and forward results within a TaskGraph
 * @details
 */
#ifndef HTGS_ITASK_H
#define HTGS_ITASK_H

#include <functional>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <list>
#include <assert.h>


#include "../debug/debug_message.h"
#include "MemoryData.hpp"
#include "../core/task/BaseITask.hpp"
#include "../core/graph/BaseConnector.hpp"
#include "../core/graph/Connector.hpp"
#include "../core/memory/MMType.h"

namespace htgs {

/**
 * @typedef ConnectorVector
 * A vector of BaseConnectors.
 */
typedef std::vector<std::shared_ptr<BaseConnector>> ConnectorVector;

/**
 * @typedef ConnectorVectorMap
 * An unordered mapping of string names mapping to a pointer to ConnectorVectors.
 * This datastructure is used for execution pipelines and memory edges. Each ITask
 * can only have up to 1 ConnectorVectorMap with a given name. The vector of connectors represents
 * one per execution pipeline.
 */
typedef std::unordered_map<std::string, std::shared_ptr<ConnectorVector>> ConnectorVectorMap;

/**
 * @typedef ConnectorVectorPair
 * Defines a pair to be added to a ConnectorVectorMap.
 */
typedef std::pair<std::string, std::shared_ptr<ConnectorVector>> ConnectorVectorPair;

class BaseConnector;

class IData;

template<class V, class W>
class TaskScheduler;

template<class V>
class Connector;

template<class V>
class MemoryData;

/**
 * @class ITask ITask.hpp <htgs/api/ITask.hpp>
 * @brief An interface to process input data and forward results within a TaskGraph
 * @details
 *
 * To use the ITask, a new class inherits ITask and defines the pure virtual functions. The ITask is
 * then connected into a TaskGraph, which is bound to a Runtime. The ITask contains metadata that describes
 * threading and scheduling rules. Using this metadata, the Runtime spawns threads as a pool. Each thread
 * is bound to a separate instance of the ITask, which is generated through the copy function. 
 *
 * The purpose of this interface is to provide the functions necessary to represent computational and logical operations
 * for algorithms, which are added to a TaskGraph.
 * Custom behavior for an ITask can be implemented, as demonstrated with other classes that derive from ITask; i.e.,
 * Bookkeeper, ExecutionPipeline, and ICudaTask.
 *
 * An ITask should represent some component of an algorithm, such that multiple threads can
 * concurrently process and stream data through a TaskGraph. The main pieces that impact the performance
 * are: (1) Memory requirements, (2) Data dependencies, and (3) Computational complexity.
 *
 * There are two methods for handling memory.
 *
 * The first type of memory for an ITask is local memory. This
 * type should be allocated in the initialize() function and freed in the shutdown() function.
 * It is duplicated (one per thread) and should be local to that thread only.
 *
 * The second type of memory is shared memory, which can be used by other tasks in a TaskGraph. One ITask is responsible
 * for getting memory, while another ITask is responsible for releasing memory.
 * This memory is managed by an external MemoryManager, which allocates the memory, connects the getter
 * and the releaser, and frees the memory once the TaskGraph is finished. Use the TaskGraph::addMemoryManagerEdge
 * to attach shared memory between two ITasks. The memory that is acquired should be incorporated into the
 * output data of the ITask and forwarded until it is released.
 *
 * An ITask can get and release memory using the memGet() and memRelease() routines, respectively. If there are cases where
 * the ITask getting or releasing memory may not a memory edge, then that task can use the hasMemGetter and
 * has memReleaser routines to verify the edge exists.
 *
 * If there are multiple computational ITasks within a TaskGraph, the number of threads processing each ITask
 * should be determined based on the workload of each ITask with the aim of reducing the wait period for every ITask (if possible).
 * By doing so, the overall compute time of a TaskGraph can be evenly distributed. The number of threads in use for computation
 * should not exceed the number of logical cores on a system.
 *
 * There are two types of initialize functions. The basic ITask::initialize(int pipelineId, int numPipelines) is
 * the most commonly used variant. The ITask::initialize(int pipelineId, int numPipeline, TaskScheduler<T, U> *ownerTask,
 * std::shared_ptr<ConnectorVector> pipelineConnectorList) can be used for more advanced operations such as processing data from
 * other execution pipelines using the pipelineConnectorList.
 *
 * Example Implementation:
 * @code
 * class ReadTask : public htgs::ITask<Data1, Data2>
 * {
 *  public:
 *   ReadTask(int numThreads, long memorySize) : ITask(numThreads), memorySize(memorySize) {}
 *
 *   virtual void initialize(int pipelineId, int numPipelines)
 *   {
 *     // This memory will be allocated multiple times (one for each thread that is bound to the ReadTask ITask)
 *     reusableMemory = new double[memorySize];
 *   }
 *
 *   virtual void shutdown() { delete [] reusableMemory; }
 *
 *   virtual void executeTask(std::shared_ptr<Data1> data)
 *   {
 *     ...
 *     // Shared memory getter
 *     std::shared_ptr<htgs::MemoryData<int *>> readBuffer = this->memGet<int *>("readMemory", new ReleaseCountRule(1));
 *     readData(data->getFile(), readBuffer->get());
 *
 *     // Shared memory release example
 *     if (this->hasMemReleaser("otherMemory")
 *     	this->memRelease("otherMemory", data->getMemory());
 *
 *     addResult(new Data2(readBuffer));
 *     ...
 *   }
 *
 *   virtual std::string getName() { return "ReadTask"; }
 *
 *   virtual htgs::ITask<Data1, Data2> *copy() { return new ReadTask(this->getNumThreads(), memorySize); }
 *
 *   // Optional. Default will check inputConnector->isInputTerminated();
 *   virtual bool isTerminated(std::shared_ptr<htgs::BaseConnector> inputConnector) { return inputConnector->isInputTerminated(); }
 *
 *  private:
 *   // reusableMemory is not used in computation, but shows an example of local memory allocation
 *   double *reusableMemory;
 *   long memorySize;
 * }
 * @endcode
 *
 * Usage Example:
 * @code
 * int numThreadsPreprocess = 1;
 * int numThreadsRead = 2;
 * int numThreadsMultiply = 10;
 *
 * htgs::TaskGraph<htgs::VoidData, htgs::VoidData> *taskGraph = new htgs::TaskGraph<htgs::VoidData, htgs::VoidData>();
 *
 * PreProcessTask *preProcTask = new PreProcessTask(numThreadsPreprocess);
 * ReadTask *readTask = new ReadTask(numThreadsRead);
 * MultiplyTask *mulTask = new MultiplyTask(numThreadsMultiply);
 *
 * // Add tasks to task graph (each task must be added before using addMemoryManagerEdge)
 * taskGraph->addEdge(preProcTask, readTask);
 * taskGraph->addEdge(readTask, mulTask);
 *
 * // Add memory edges. The types for the allocator must match the type specified when an ITask uses memGet
 * // Memory pool size is specified based on algorithm scheduling and memory release rules.
 * int otherMemoryPoolSize = 100;
 * int readMemoryPoolSize = 200;
 *
 * // Creates the memory edge "otherMemory" with preProcTask as the getter, and readTask as the releaser
 * taskGraph->addMemoryManagerEdge("otherMemory", preProcTask, readTask, new OtherMemoryAllocator(), otherMemoryPoolSize);
 *
 * // Creates the memory edge "readMemory" with readTask as the getter, and mulTask as the releaser
 * taskGraph->addMemoryManagerEdge("readMemory", readTask, mulTask, new ReadMemoryAllocator(), readMemoryPoolSize);
 *
 * htgs::Runtime *executeGraph = new htgs::Runtime(taskGraph);
 *
 * // Launches threads and binds them to ITasks
 * executeGraph->executeAndWaitForRuntime();
 * @endcode
 *
 * @tparam T the input data type for the ITask, T must derive from IData.
 * @tparam U the output data type for the ITask, U must derive from IData.
 */
template<class T, class U>
class ITask: public BaseITask {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:

  /**
   * Creates an ITask with number of threads equal to 1.
   */
  ITask() {
    this->numThreads = 1;
    this->isStartTask = false;
    this->poll = false;
    this->microTimeoutTime = 0L;

    memGetter = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    memReleaser = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    mmTypeMap = std::shared_ptr<std::unordered_map<std::string, MMType>>(new std::unordered_map<std::string, MMType>());
    memReleaserOutsideGraph = std::shared_ptr<std::unordered_map<std::string, bool>>(new std::unordered_map<std::string, bool>());

    this->pipelineId = 0;
  }

  /**
   * Constructs an ITask with a specified number of threads.
   * @param numThreads the number of threads associated with this ITask
   */
  ITask(int numThreads) {
    this->numThreads = numThreads;
    this->isStartTask = false;
    this->poll = false;
    this->microTimeoutTime = 0L;

    memGetter = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    memReleaser = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    mmTypeMap = std::shared_ptr<std::unordered_map<std::string, MMType>>(new std::unordered_map<std::string, MMType>());
    memReleaserOutsideGraph = std::shared_ptr<std::unordered_map<std::string, bool>>(new std::unordered_map<std::string, bool>());
    this->pipelineId = 0;
  }

  /**
   * Constructs an ITask with a specified number of threads as well as additional scheduling options.
   * @param numThreads the number of threads associated with this ITask
   * @param isStartTask whether this ITask starts executing immediately and passes nullptr to executeTask()
   * @param poll whether this task will poll for data, if the timeout period expires, nullptr is passed to executeTask()
   * @param microTimeoutTime the timeout period for checking for data
   * @note If the ITask is declared as a start task or is polling, then executeTask() should properly handle nullptr data
   */
  ITask(int numThreads, bool isStartTask, bool poll, long microTimeoutTime) {
    this->numThreads = numThreads;
    this->isStartTask = isStartTask;
    this->poll = poll;
    this->microTimeoutTime = microTimeoutTime;

    memGetter = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    memReleaser = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    mmTypeMap = std::shared_ptr<std::unordered_map<std::string, MMType>>(new std::unordered_map<std::string, MMType>());
    memReleaserOutsideGraph = std::shared_ptr<std::unordered_map<std::string, bool>>(new std::unordered_map<std::string, bool>());

    this->pipelineId = 0;
  }

  /**
   * Destructor
   */
  virtual ~ITask() { }

  /**
   * @internal
   * Sets the pipeline Id for this ITask.
   * @param pipelineId the pipelineId
   * @note This function should only be used by the HTGS API
   */
  void setPipelineId(int pipelineId) {
    this->pipelineId = pipelineId;
  }

  /**
   * @internal
   * Sets the number of threads associated with this ITask.
   * @param numThreads the number of  threads associated with the ITask
   * @note This function will only be applied if the TaskGraph holding this ITask has not been copied or executed using a RunTime
   */
  void setNumThreads(int numThreads) {
    this->numThreads = numThreads;
  }

  /**
   * Sets whether this task is a start task or not.
   * If the task is a start task, then it will begin executing as soon as the ITask is bound to a thread.
   * It will be passed the value nullptr to executeTask()
   * @param isStartTask whether the task should be a start task
   */
  void setIsStartTask(bool isStartTask) {
    this->isStartTask = isStartTask;
  }

  /**
   * @internal
   * Sets whether this task is polling for data or not.
   * If the task is polling and the timeout period expires, then the ITask will be passed
   * the value nullptr to executeTask()
   * @param poll whether the task should poll
   * @note This function only works if a thread has not been bound to this ITask.
   */
  void setPoll(bool poll) {
    this->poll = poll;
  }

  /**
   * @internal
   * Sets the timeout time for polling in microseconds.
   * Polling should be set to true if this is to be used ( setPoll() )
   * @param microTimeoutTime the timeout time for polling
   * @note This function only works if a thread has not been bound to this ITask.
   */
  void setMicroTimeoutTime(long microTimeoutTime) {
    this->microTimeoutTime = microTimeoutTime;
  }

  /**
   * Gets the number of threads associated with this ITask
   * @return the number of threads
   */
  int getNumThreads() const {
    return this->numThreads;
  }

  /**
   * Gets whether this ITask is a starting task
   * @return whether the ITask is a starting task
   * @retval TRUE if the task will begin execution immediately when it is bound to a thread
   * @retval FALSE if the task must wait for data before it can begin exection
   */
  bool getIsStartTask() const {
    return this->isStartTask;
  }

  /**
   * Gets whether this ITask is polling for data or not
   * @return whether the ITask is polling
   * @retval TRUE if the task is polling for data, it will wait until the timeout period expires.
   * @retval FALSE if the task is not polling for data
   */
  bool isPoll() const {
    return this->poll;
  }

  /**
   * Gets the timeout time for polling
   * @return the timeout time
   */
  long getMicroTimeoutTime() const {
    return this->microTimeoutTime;
  }

  /**
   * Adds results to the output list to be sent to the next connected ITask in a TaskGraph
   * @param result the result data to be passed
   */
  void addResult(std::shared_ptr<U> result) {
    this->ownerTask->addResult(result);
  }

  /**
   * Adds results to the output list to be sent to the next connected ITask in a TaskGraph.
   * The result pointer will be wrapped into a shared smart pointer and then placed in the output list.
   * @param result the result data to be passed
   */
  void addResult(U *result) {
   this->ownerTask->addResult(std::shared_ptr<U>(result));
  }

  /**
   * @internal
   * Copies the ITask including its list of memGetters and memReleasers
   * @return a deep copy of the ITask
   *
   * @note This function should only be called by the HTGS API
   */
  ITask<T, U> *copyITask() {
    ITask<T, U> *iTaskCopy = copy();

    iTaskCopy->setMemGetter(this->memGetter);
    iTaskCopy->setMemReleaser(this->memReleaser);
    iTaskCopy->setMMTypeMap(this->mmTypeMap);
    iTaskCopy->setMemReleaserOutsideGraph(this->memReleaserOutsideGraph);
    return iTaskCopy;
  };


  /**
   * @internal
   * Retrieves memory from a memory edge
   * @param name the name of the memory edge
   * @param releaseRule the release rule to be associated with the newly acquired memory
   * @return the MemoryData
   * @tparam V the MemoryData type
   * @note The name specified must have been attached to this ITask as a memGetter using
   * the TaskGraph::addMemoryManagerEdge routine, which can be verified using hasMemGetter()
   *
   * @note This function will block if no memory is available, ensure the
   * memory pool size is sufficient based on memory release rules and data flow.
   * @note Memory edge must be defined as MMType::Static
   */
  template<class V>
  std::shared_ptr<MemoryData<V>> memGet(std::string name, IMemoryReleaseRule *releaseRule) {
    assert(this->mmTypeMap->find(name) != this->mmTypeMap->end());
    assert(this->mmTypeMap->find(name)->second == MMType::Static);
    std::shared_ptr<BaseConnector> conn = memGetter->find(name)->second->at((unsigned long) this->pipelineId);
    std::shared_ptr<Connector<MemoryData<V>>> connector = std::dynamic_pointer_cast<Connector<MemoryData<V>>>(conn);
    std::shared_ptr<MemoryData<V>> memory = connector->consumeData();
    memory->setMemoryReleaseRule(releaseRule);
    return memory;
  }

  /**
   * @internal
   * Retrieves memory from a memory edge
   * @param name the name of the memory edge
   * @param releaseRule the release rule to be associated with the newly acquired memory
   * @param numElems the number of elements to allocate (uses internal allocator defined from the memory edge)
   * @return the MemoryData
   * @tparam V the MemoryData type
   * @note The name specified must have been attached to this ITask as a memGetter using
   * the TaskGraph::addMemoryManagerEdge routine, which can be verified using hasMemGetter()
   *
   * @note This function will block if no memory is available, ensure the
   * memory pool size is sufficient based on memory release rules and data flow.
   * @note Memory edge must be defined as MMType::Dynamic
   */
  template<class V>
  std::shared_ptr<MemoryData<V>> memGet(std::string name, IMemoryReleaseRule *releaseRule, size_t numElems) {
    assert(this->mmTypeMap->find(name) != this->mmTypeMap->end());
    assert(this->mmTypeMap->find(name)->second == MMType::Dynamic);
    std::shared_ptr<BaseConnector> conn = memGetter->find(name)->second->at((unsigned long) this->pipelineId);
    std::shared_ptr<Connector<MemoryData<V>>> connector = std::dynamic_pointer_cast<Connector<MemoryData<V>>>(conn);
    std::shared_ptr<MemoryData<V>> memory = connector->consumeData();
    memory->setMemoryReleaseRule(releaseRule);
    memory->memAlloc(numElems);
    return memory;
  }

  /**
   * @internal
   * Retrieves memory from a memory edge that is managed by the user.
   * Should use this function to throttle the allocator to ensure not too many elements are allocated.
   * The number of elements is associated with the MemoryManager edge pool size.
   * @param name the name of the memory edge
   * @note The name specified must have been attached to this ITask as a memGetter using the
   * TaskGraph::addUserManagedMemoryManagerEdge routine, which can be verified using hasMemGetter()
   *
   * @note This function will block if no memory is available, ensure the memory pool size
   * is sufficient based on how you handle memory. Should be used in conjunction with memRelease
   * @note Memory edge must be defined as MMType::UserManaged by using the TaskGraph::addUserManagedMemoryManagerEdge
   */
  void allocUserManagedMemory(std::string name) {
    assert(this->mmTypeMap->find(name)->second == MMType::UserManaged);
    std::shared_ptr<BaseConnector> conn = memGetter->find(name)->second->at((unsigned long) this->pipelineId);
    std::shared_ptr<Connector<MemoryData<void *>>> connector = std::dynamic_pointer_cast<Connector<MemoryData<void *>>>(conn);
    connector->consumeData();
  }

  /**
   * Releases memory onto a memory edge
   * @param name the name of the memory edge
   * @param memory the memory to be released
   * @tparam V the MemoryData type
   * @note The name specified must have been attached to this ITask as a memReleaser using
   * the TaskGraph::addMemoryManagerEdge routine, which can be verified using hasMemReleaser()
   * @note Memory edge must be defined as MMType::Static OR MMType::Dynamic
   */
  template<class V>
  void memRelease(std::string name, std::shared_ptr<MemoryData<V>> memory) {
    assert(this->mmTypeMap->find(name) != this->mmTypeMap->end());
    assert(this->mmTypeMap->find(name)->second == MMType::Static
               || this->mmTypeMap->find(name)->second == MMType::Dynamic);
    std::shared_ptr<BaseConnector> conn = memReleaser->find(name)->second->at((unsigned long) memory->getPipelineId());
    std::shared_ptr<Connector<MemoryData<V>>> connector = std::dynamic_pointer_cast<Connector<MemoryData<V>>>(conn);
    connector->produceData(memory);
  }

  /**
   * Releases memory onto a memory edge
   * @param name the name of the memory edge
   * @param pipelineId the pipelineId to add data to
   * @note The name specified must have been attached to this ITask as a memReleaser using
   * the TaskGraph::addUserManagedEdge routine, which can be verified using hasMemReleaser()
   * @note Memory edge must be defined as MMType::UserManaged by using the TaskGraph::addUserManagedMemoryManagerEdge
   */
  void memRelease(std::string name, int pipelineId) {
    assert(this->mmTypeMap->find(name) != this->mmTypeMap->end());
    assert(this->mmTypeMap->find(name)->second == MMType::UserManaged);

    std::shared_ptr<MemoryData<void *>> memory(new MemoryData<void *>(nullptr));
    memory->setPipelineId(pipelineId);

    std::shared_ptr<BaseConnector> conn = memReleaser->find(name)->second->at((unsigned long) memory->getPipelineId());
    std::shared_ptr<Connector<MemoryData<void *>>> connector = std::dynamic_pointer_cast<Connector<MemoryData<void *>>>(conn);
    connector->produceData(memory);
  }

  /**
   * Checks whether this ITask contains a memGetter for a specified name
   * @param name the name of the memGetter edge
   * @return whether this ITask has a memGetter with the specified name
   * @retval TRUE if the ITask has a memGetter with the specified name
   * @retval FALSE if the ITask does not have a memGetter with the specified name
   * @note To add a memGetter to this ITask use TaskGraph::addMemoryManagerEdge
   */
  bool hasMemGetter(std::string name) {
    return memGetter->find(name) != memGetter->end();
  }

  /**
   * Checks whether this ITask contains a memReleaser for a specified name
   * @param name the name of the memReleaser edge
   * @return whether this ITask has a memReleaser with the specified name
   * @retval TRUE if the ITask has a memReleaser with the specified name
   * @retval FALSE if the ITask does not have a memReleaser with the specified name
   * @note To add a memReleaser to this ITask use TaskGraph::addMemoryManagerEdge
   */
  bool hasMemReleaser(std::string name) {
    return memReleaser->find(name) != memReleaser->end();
  }

  /**
   * Checks whether this ITask contains a memReleaser that exists outside of the graph that the memory
   * edge exists.
   * @param name the name of the memReleaser edge
   * @return whether the memReleaser's edge exists inside of another graph
   * @retval TRUE if the memReleaser edge is in a graph that this ITask does not belong too
   * @retval FALSE if the memReleaser edge is in the graph that the ITask belongs too
   */
  bool isMemReleaserOutsideGraph(std::string name) {
    return this->memReleaserOutsideGraph->at(name);
  }

  /**
   * @internal
   * Attaches a memGetter to this ITask
   * @param name the name of the memory edge
   * @param connector the connector for the MemoryManager
   * @param type the memory manager type
   *
   * @note This function should only be called by the HTGS API, use TaskGraph::addMemoryManagerEdge instead.
   */
  void attachMemGetter(std::string name, std::shared_ptr<BaseConnector> connector, MMType type) {
    std::shared_ptr<ConnectorVector> vector;
    if (hasMemGetter(name)) {
      vector = memGetter->find(name)->second;
    }
    else {
      vector = std::shared_ptr<ConnectorVector>(new ConnectorVector());
    }

    // If the pipeline id is not the same as the vector size, then the connection has already been made
    // Each pipeline memory edge is added in the correct order; ie pipeline 0 added first ... pipeline 1 second, etc.
    // We allow the memory getter to reuse the same memory edge when we have multiple memory releasers for the same memory edge.
    if (this->pipelineId != vector->size())
    {
      return;
    }

    vector->push_back(connector);

    mmTypeMap->insert(std::pair<std::string, MMType>(name, type));
    memGetter->insert(ConnectorVectorPair(name, vector));
    DEBUG("Num memory getters " << memGetter->size() << " with " << vector->size() << " connectors");
  }

  /**
   * @internal
   * Attaches a memReleaser to this ITask
   * @param name the name of the memory edge
   * @param connector the conector for the MemoryManager
   * @param type the memory manager type
   * @param outsideMemManGraph indicates if this ITask exists outside of the graph where the memory manager is
   *
   * @note This function should only be called by the HTGS API, use TaskGraph::addMemoryManagerEdge instead.
   */
  void attachMemReleaser(std::string name, std::shared_ptr<BaseConnector> connector, MMType type, bool outsideMemManGraph) {
    std::shared_ptr<ConnectorVector> vector;
    if (hasMemReleaser(name)) {
      vector = memReleaser->find(name)->second;
    }
    else {
      vector = std::shared_ptr<ConnectorVector>(new ConnectorVector());
    }

    // Errors if you try to add the same memory releaser to the same memory edge.
    // Each pipeline memory edge is added in the correct order; ie pipeline 0 added first ... pipeline 1 second, etc.
    // If the same pipieline is added at the incorrect time then the pipeline id will not equal the size of the vector
    // If the memory edge originates in a graph that this ITask does not belong too, then ignore this check
    if (this->pipelineId != vector->size() && !outsideMemManGraph)
    {
      std::cerr << "Error attempting to add mem releaser to the same named memory edge!" << std::endl;
      exit(-1);
    }

    memReleaserOutsideGraph->insert(std::pair<std::string, bool>(name, outsideMemManGraph));

    vector->push_back(connector);

    mmTypeMap->insert(std::pair<std::string, MMType>(name, type));
    memReleaser->insert(ConnectorVectorPair(name, vector));
    DEBUG("Num memory releasers " << memReleaser->size() << " with " << vector->size() << " connectors");
  }

  /**
   * Gets the memReleaser mapping
   * @return the map for memReleasers
   */
  std::shared_ptr<ConnectorVectorMap> getMemReleasers() {
    return this->memReleaser;
  };

  /**
   * @internal
   * Initializes an ITask
   * @param pipelineId the pipelineId
   * @param numPipeline the number of pipelines
   * @param ownerTask the owner task
   * @param pipelineConnectorList the pipelineConnectorList
   *
   * @note This function should only be called by the HTGS API
   */
  void initializeITask(int pipelineId, int numPipeline, TaskScheduler<T, U> *ownerTask,
                       std::shared_ptr<ConnectorVector> pipelineConnectorList) {
    this->pipelineId = pipelineId;
    this->ownerTask = ownerTask;
    this->initialize(pipelineId, numPipeline);
    this->initialize(pipelineId, numPipeline, ownerTask, pipelineConnectorList);
  }

  /**
   * @internal
   * Provides profile output for the ITask,
   *
   * @note this function should only be called by the HTGS API
   */
  void profileITask() {
    if (memReleaser->size() > 0) {
      for (const auto &kv : *this->memReleaser) {
        std::cout << "Mem releaser: " << kv.first << " profile; ";
        // produce
        kv.second->at((unsigned long) this->pipelineId)->profileProduce(this->numThreads);

      }

    }

    if (memGetter->size() > 0) {
      for (const auto &kv : *this->memGetter) {
        std::cout << "Mem getter: " << kv.first << " profile; ";
        // consume
        kv.second->at((unsigned long) this->pipelineId)->profileConsume(this->numThreads, false);

      }
    }
    profile();
  }

  /**
   * Virtual function that is called when an ITask is being initialized by it's owner thread.
   * This initialize function contains some advanced parameters such as the TaskScheduler associated
   * with the ITask and the list of pipeline connectors. These parameters can be used for features such
   * as work stealing. If they are not needed, then override the initialize(int pipelineId, int numPipeline) function
   * instead.
   * @param pipelineId the pipelineId, only used if the ITask is inside of an ExecutionPipeline
   * @param numPipeline the number of pipelines, only used if the ITask is inside of an ExecutionPipeline
   * @param ownerTask the owner Task for this ITask
   * @param pipelineConnectorList the list of connectors that connect to other duplicate
   * ICudaTask's in an execution pipeline
   */
  virtual void initialize(int pipelineId, int numPipeline, TaskScheduler<T, U> *ownerTask,
                          std::shared_ptr<ConnectorVector> pipelineConnectorList) {}

  /**
   * Virtual function that is called when an ITask is being initialized by it's owner thread.
   * @param pipelineId the pipelineId, only used if the ITask is inside of an ExecutionPipeline
   * @param numPipeline the number of pipelines, only used if the ITask is inside of an ExecutionPipeline
  */
 virtual void initialize(int pipelineId, int numPipeline) {}

 /**
  * Virtual function that is called when an ITask is being shutdown by it's owner thread.
  */
  virtual void shutdown() {}

  /**
   * Pure virtual function that is called when an ITask's thread is to execute on data
   * @param data the data to be executed
   * @note To send output data use addResult()
   * @note If the ITask is a start task or is polling, data might be nullptr
   */
  virtual void executeTask(std::shared_ptr<T> data) = 0;

  /**
   * Virtual function that gets the name of the ITask
   * @return the name of the ITask
   */
  virtual std::string getName() {
    return "UnnamedITask";
  }

  /**
   * Pure virtual function that creates a copy of the ITask
   * @return the copy of the ITask
   */
  virtual ITask<T, U> *copy() = 0;

  /**
   * Virtual function that is called when an ITask is checking if it can be terminated
   * @param inputConnector the connector responsible for giving data to this Task
   * @return whether the ITask can be terminated or not
   * @retval TRUE if the ITask is ready to be terminated
   * @retval FALSE if the ITask is not ready to be terminated
   * @note By default this function checks if the input no longer sending data using inputConnector->isInputTerminated()
   */
  virtual bool isTerminated(std::shared_ptr<BaseConnector> inputConnector) {
    return inputConnector->isInputTerminated();  
  }

  /**
   * Virtual function that is called to debug the ITask   
   */
  virtual void debug() { }

  /**
   * Virtual function that is called to provide profile output for the ITask
   * @note \#define PROFILE to enable profiling
   */
  virtual void profile() { }

  /**
   * Creates a dot notation representation for this task
   * @param input the input connector for this task
   * @param output the output connector for this task
   * @return the dot notation for the task.
   */
  std::string getDot(std::shared_ptr<BaseConnector> input, std::shared_ptr<BaseConnector> output) {
    std::string dotId = this->getDotId();
    std::ostringstream oss;

    if (memReleaser->size() > 0) {
      for (const auto &kv : *this->memReleaser) {
        if (this->isMemReleaserOutsideGraph(kv.first))
        {
          for (auto connector : *kv.second)
          {
            oss << dotId << " -> " << connector->getDotId() << ";" << std::endl;
          }
        }
        else {
          oss << dotId << " -> " << kv.second->at((unsigned long) this->pipelineId)->getDotId() << ";" << std::endl;
        }
      }

    }

    if (memGetter->size() > 0) {
      for (const auto &kv : *this->memGetter) {
        oss << kv.second->at((unsigned long) this->pipelineId)->getDotId() << " -> " << dotId << ";" << std::endl;
      }
    }

    oss << genDot(dotId, input, output);
    return oss.str();
  }

  /**
   * Virtual function that generates the input/output and per-task dot notation
   * @param dotId the id for this task
   * @param input the input connector for this task
   * @param output the output connector for this task
   * @return the dot that represents the interaction between the input/output and the internal custom dot notation
   */
  virtual std::string genDot(std::string dotId, std::shared_ptr<BaseConnector> input, std::shared_ptr<BaseConnector> output) {
    std::ostringstream oss;
    oss << input->getDotId() << " -> " << dotId << ";" << std::endl;
    oss << input->genDot();

    if (output != nullptr) {
      oss << dotId << " -> " << output->getDotId() << ";" << std::endl;
      oss << output->genDot();
    }

    oss << genDot(dotId);

    return oss.str();
  }
  /**
   * Virtual function that adds additional dot attributes to this node.
   * @return the additiona dota attributes for the dot graph representation
   */
  virtual std::string genDot(std::string dotId) {
    return dotId + "[label=\"" + this->getName() + "\"];\n";
  }


  /**
   * Gets the id used for dot nodes.
   */
  std::string getDotId() {
    std::ostringstream id;
    id << this;
    std::string idStr = id.str();
    idStr.erase(0, 1);

    return idStr;
  }

  /**
   * Gets the memory manager type for the specified memory manager edge name
   * @param name the name of the memory manager edge
   * @return the memory manager type
   */
  MMType getMemoryManagerType(std::string name) {
    return this->mmTypeMap->at(name);
  }
 private:
  //! @cond Doxygen_Suppress
  void setMemGetter(std::shared_ptr<ConnectorVectorMap> memGetter) { this->memGetter = memGetter; }
  void setMMTypeMap(std::shared_ptr<std::unordered_map<std::string, MMType>> pMap) { this->mmTypeMap = pMap; }
  void setMemReleaser(std::shared_ptr<ConnectorVectorMap> memReleaser) {
    this->memReleaser = memReleaser;
  }
  void setMemReleaserOutsideGraph(std::shared_ptr<std::unordered_map<std::string, bool>>memReleaserOutsideGraph){
    this->memReleaserOutsideGraph = memReleaserOutsideGraph;
  }

  //! @endcond


 private:
  int
      numThreads; //!< The number of threads to be used with this ITask (forms a thread pool) used when creating a TaskScheduler
  bool isStartTask; //!< Whether the ITask will be a start task used when creating a TaskScheduler
  bool poll; //!< Whether the ITask should poll for data used when creating a TaskScheduler
  long microTimeoutTime; //!< The timeout time for polling in microseconds used when creating a TaskScheduler


  std::shared_ptr<std::unordered_map<std::string, bool>> memReleaserOutsideGraph; //!< A mapping from a memory edge name to whether that memory edge is inside of another graph
  std::shared_ptr<ConnectorVectorMap> memGetter; //!< A mapping from memory edge name to memory manager connector for getting memory
  std::shared_ptr<ConnectorVectorMap> memReleaser; //!< A mapping from memory edge name to memory manager connector for releasing memory
  std::shared_ptr<std::unordered_map<std::string, MMType>> mmTypeMap; //!< A mapping from memory edge name to memory manager type

  TaskScheduler<T, U> *ownerTask; //!< The owner task for this ITask

  int pipelineId; //!< The execution pipeline id for the ITask
};

}


#endif //HTGS_ITASK_H
