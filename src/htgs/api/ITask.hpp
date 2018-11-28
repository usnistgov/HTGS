
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
#ifndef HTGS_ITASK_HPP
#define HTGS_ITASK_HPP

#include <functional>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <list>
#include <assert.h>
#include <sstream>
#include <htgs/core/graph/Connector.hpp>
#include <htgs/core/task/TaskManager.hpp>

#ifdef USE_NVTX
#include <nvtx3/nvToolsExt.h>
#endif

#if defined( __GLIBCXX__ ) || defined( __GLIBCPP__ )
#include <cxxabi.h>
#endif

namespace htgs {

template<class T, class U>
class TaskManager;

/**
 * @class ITask ITask.hpp <htgs/api/ITask.hpp>
 * @brief An interface to process input data and forward results within a TaskGraph
 * @details
 *
 * To use the ITask, a new class inherits ITask and defines the pure virtual functions. The ITask is
 * then connected into a TaskGraphConf, which is bound to a TaskGraphRuntime. The ITask contains metadata that describes
 * threading and scheduling rules. Using this metadata, the TaskGraphRuntime spawns threads into a pool for the ITask. Each thread
 * is bound to a separate instance of the ITask, which is generated through the copy function. 
 *
 * The purpose of this interface is to provide the functions necessary to represent computational and logical operations
 * for algorithms, which are added to a TaskGraphConf.
 * Custom behavior for an ITask can be implemented, as demonstrated with other classes that derive from ITask; i.e.,
 * Bookkeeper, ExecutionPipeline, and ICudaTask.
 *
 * An ITask should represent some component of an algorithm, such that multiple threads can
 * concurrently process and stream data through a TaskGraphConf. The main pieces that impact the performance
 * are: (1) Memory (reuse/capacity/locality), (2) Data dependencies, and (3) Computational complexity.
 *
 * There are two methods for handling memory.
 *
 * The first type of memory for an ITask is local memory. This
 * type should be allocated in the initialize() function and freed in the shutdown() function.
 * It is duplicated (one per thread) and should be local to that thread only.
 *
 * The second type of memory is shared memory, which can be used by other tasks in a TaskGraphConf. One ITask is responsible
 * for getting memory, while any other ITask or the main thread is responsible for releasing that memory.
 * The memory is managed by an external MemoryManager, which allocates the memory and frees the memory once the TaskGraphConf is finished.
 * Use the TaskGraph::addMemoryManagerEdge
 * to attach a task to get memory. The memory that is acquired should be incorporated into the
 * output data of the ITask and forwarded until it is released by some other task. The release memory operation should exist
 * within the same task graph.
 *
 * An ITask can get and release memory using the AnyITask::getMemory and AnyITask::releaseMemory routines, respectively. If there are cases where
 * the ITask getting memory may not have a memory edge, then that task can use the AnyITask::hasMemoryEdge routine to verify if the edge exists.
 *
 * If there are multiple computational ITasks within a TaskGraph, the number of threads processing each ITask
 * should be determined based on the workload of each ITask with the aim of reducing the wait period for every ITask (if possible).
 * By doing so, the overall compute time of a TaskGraph can be evenly distributed. The number of threads in use for computation
 * should not exceed the number of logical cores on a system.
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
 *     // get shared memory
 *     htgs::m_data_t<int> readBuffer = this->memGet<int>("readMemory", new ReleaseCountRule(1));
 *     readData(data->getFile(), readBuffer->get());
 *
 *     // Shared memory release example
 *     this->releaseMemory(data->getMemory());
 *
 *     // Add memory to output edge
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
 * htgs::TaskGraphConf<htgs::VoidData, htgs::VoidData> *taskGraph = new htgs::TaskGraphConf<htgs::VoidData, htgs::VoidData>();
 *
 * PreProcessTask *preProcTask = new PreProcessTask(numThreadsPreprocess);
 * ReadTask *readTask = new ReadTask(numThreadsRead);
 * MultiplyTask *mulTask = new MultiplyTask(numThreadsMultiply);
 *
 * // Add tasks to task graph (each task must be added before using addMemoryManagerEdge)
 * taskGraph->setGraphConsumerTask(preProcTask);
 * taskGraph->addEdge(preProcTask, readTask);
 * taskGraph->addEdge(readTask, mulTask);
 * taskGraph->addGraphProducerTask(mulTask);
 *
 * // Add memory edges. The types for the allocator must match the type specified when an ITask uses getMemory
 * // Memory pool size is specified based on algorithm scheduling and memory release rules.
 * int otherMemoryPoolSize = 100;
 * int readMemoryPoolSize = 200;
 *
 * // Creates the memory edge "otherMemory" with preProcTask as the getter, and readTask as the releaser
 * taskGraph->addMemoryManagerEdge("otherMemory", preProcTask, new OtherMemoryAllocator(), otherMemoryPoolSize);
 *
 * // Creates the memory edge "readMemory" with readTask as the getter, and mulTask as the releaser
 * taskGraph->addMemoryManagerEdge("readMemory", readTask, new ReadMemoryAllocator(), readMemoryPoolSize);
 *
 * htgs::TaskGraphRuntime *executeGraph = new htgs::TaskGraphRuntime(taskGraph);
 *
 * // Launches threads and binds them to ITasks
 * executeGraph->executeAndWaitForRuntime();
 * @endcode
 *
 * @tparam T the input data type for the ITask, T must derive from IData.
 * @tparam U the output data type for the ITask, U must derive from IData.
 */
template<class T, class U>
class ITask : public AnyITask {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:

  /**
   * Creates an ITask with number of threads equal to 1.
   */
  ITask() : super() {}

  /**
   * Constructs an ITask with a specified number of threads.
   * @param numThreads the number of threads associated with this ITask
   */
  ITask(size_t numThreads) : super(numThreads) {}

  /**
   * Constructs an ITask with a specified number of threads as well as additional scheduling options.
   * @param numThreads the number of threads associated with this ITask
   * @param isStartTask whether this ITask starts executing immediately and passes nullptr to executeTask()
   * @param poll whether this task will poll for data, if the timeout period expires, nullptr is passed to executeTask()
   * @param microTimeoutTime the timeout period for checking for data
   * @note If the ITask is declared as a start task or is polling, then executeTask() should properly handle nullptr data
   */
  ITask(size_t numThreads, bool isStartTask, bool poll, size_t microTimeoutTime) : super(numThreads,
                                                                                         isStartTask,
                                                                                         poll,
                                                                                         microTimeoutTime) {}


  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////// VIRTUAL FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  virtual ~ITask() override {}

  virtual void initialize() override {}

  /**
 * Pure virtual function that is called when an ITask's thread is to execute on data
 * @param data the data to be executed
 * @note To send output data use addResult()
 * @note If the ITask is a start task or is polling, data might be nullptr
 */
  virtual void executeTask(std::shared_ptr<T> data) = 0;

  /**
   * @copydoc AnyITask::canTerminate
   */
  virtual bool canTerminate(std::shared_ptr<AnyConnector> inputConnector) override {
    if (inputConnector == nullptr)
      return true;

    return inputConnector->isInputTerminated();
  }

  /**
   * @copydoc AnyITask::shutdown
   */
  virtual void shutdown() override {}

  /**
   * @copydoc AnyITask::executeTaskFinal
   */
  virtual void executeTaskFinal() override {}


  /**
   * @copydoc AnyITask::getName
   */
  virtual std::string getName() override {
    return "UnnamedITask";
  }

  /**
   * @copydoc AnyITask::getDotLabelName
   */
  virtual std::string getDotLabelName() override {
    return this->getName();
  }

  /**
   * @copydoc AnyITask::getDotShapeColor
   */
  virtual std::string getDotShapeColor() override {
    return "black";
  }

  /**
   * @copydoc AnyITask::getDotFillColor
   */
  virtual std::string getDotFillColor() override {
    return "white";
  }

  /**
   * @copydoc AnyITask::getDotShape
   */
  virtual std::string getDotShape() override {
    return "box";
  }

  /**
   * Adds the string text to the profiling of this task in the graphviz dot visualization.
   * @return the extra content to be added to the task during visualization.
   */
  virtual std::string getDotCustomProfile() override {
    return "";
  }

  /**
   * @copydoc AnyITask::printProfile
   */
  virtual void printProfile() override {}

  /**
   * @copydoc AnyITask::copy
   */
  virtual ITask<T, U> *copy() = 0;

  /**
   * Gets the number of graphs spawned by this ITask
   * @return the number of graphs spawned
   */
  virtual size_t getNumGraphsSpawned() { return 0; }

  virtual std::string genDotProducerEdgeToTask(std::map<std::shared_ptr<AnyConnector>, AnyITask *> &inputConnectorDotMap, int dotFlags) override
  {
    auto connectorPair = inputConnectorDotMap.find(this->ownerTask->getOutputConnector());
    if (connectorPair != inputConnectorDotMap.end())
    {
      auto consumerIds = connectorPair->second->getConsumerDotIds();
      if (consumerIds != "")
        return this->getDotId() + " -> " + connectorPair->second->getConsumerDotIds() + ";\n";
    }

    return "";
  }


  virtual std::string genDotConsumerEdgeFromConnector(std::shared_ptr<AnyConnector> connector, int flags) override
  {
    if (this->ownerTask->getInputConnector() != nullptr &&
        connector != nullptr && this->ownerTask->getInputConnector() == connector)
    {
      auto consumerIds = this->getConsumerDotIds();
      if (consumerIds != "")
        return connector->getDotId() + " -> " + this->getConsumerDotIds() + ";\n";
    }
    return "";
  }

  virtual std::string genDotProducerEdgeFromConnector(std::shared_ptr<AnyConnector> connector, int flags)
  {
    if (this->ownerTask->getOutputConnector() != nullptr &&
      connector != nullptr && this->ownerTask->getOutputConnector() == connector)
    {
      // TODO: Use a new getProducerDotIds ?
      return this->getDotId() + " -> " + connector->getDotId() + ";\n";
    }

    return "";
  }



  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Copies the ITask (including a copy of all memory edges)
   * @param deep whether to do a deep copy and copy the memory managers as well
   * @return the copy of the ITask
   */
  ITask<T, U> *copyITask(bool deep) override {
    ITask<T, U> *iTaskCopy = copy();

    HTGS_ASSERT(iTaskCopy != nullptr, "Copying Task '" << this->getName() << "' resulted in nullptr. Make sure you have the 'copy' function implemented (see https://pages.nist.gov/HTGS/doxygen/classhtgs_1_1_i_task.html#acaedf1466b238036d880efcbf1feafe6)");


    if (deep)
      copyMemoryEdges(iTaskCopy);

    return iTaskCopy;
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
   * Function that is called when an ITask is being initialized by it's owner thread.
   * This initialize function contains the TaskManager associated with the ITask.
   * @param pipelineId the pipelineId, only used if the ITask is inside of an ExecutionPipeline
   * @param numPipeline the number of pipelines, only used if the ITask is inside of an ExecutionPipeline
   * @param ownerTask the owner Task for this ITask
   * ICudaTask's in an execution pipeline
   */
  void initialize(size_t pipelineId, size_t numPipeline, TaskManager<T, U> *ownerTask) {
    this->ownerTask = ownerTask;
    super::initialize(pipelineId, numPipeline);
  }


  /**
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
   * @internal
   */
  template<class V>
  m_data_t<V> getMemory(std::string name, IMemoryReleaseRule *releaseRule) {
    return getMemory<V>(name, releaseRule, MMType::Static, 0);
  }

  /**
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
   * @internal
   */
  template<class V>
  m_data_t<V> getDynamicMemory(std::string name, IMemoryReleaseRule *releaseRule, size_t numElems) {
    return getMemory<V>(name, releaseRule, MMType::Dynamic, numElems);
  }

  /**
   * Releases memory onto a memory edge, which is transferred by the graph communicator
   * @param memory the memory to be released
   * @tparam V the MemoryData type
   * @note the m_data_t should be acquired from a task using the getMemory function. A reference to this data can be passed along within IData.
   */
  template<class V>
  [[gnu::deprecated("Replaced by calling 'releaseMemory' directory with htgs::MemoryData (or m_data_t)")]]
  void releaseMemory(m_data_t<V> memory) {
    memory->releaseMemory();
    // TODO: Delete or Add #ifdef
//    std::shared_ptr<DataPacket> dataPacket = std::shared_ptr<DataPacket>(new DataPacket(this->getName(),
//                                                                                        this->getAddress(),
//                                                                                        memory->getMemoryManagerName(),
//                                                                                        memory->getAddress(),
//                                                                                        memory));
#ifdef USE_NVTX
    this->getOwnerTaskManager()->getProfiler()->addReleaseMarker();
#endif
//    this->getTaskGraphCommunicator()->produceDataPacket(dataPacket);
  }


  /**
   * Resets profile data
   */
  void resetProfile()
  {
    this->ownerTask->resetProfile();
  }

  /**
   * Gets the thread ID associated with this task
   * @return the thread ID
   */
  size_t getThreadID()
  {
    return this->ownerTask->getThreadId();
  }

  /**
   * Gets the task's compute time.
   * @return the compute time in microseconds.
   */
  unsigned long long int getTaskComputeTime() const {
    return this->ownerTask->getTaskComputeTime();
  }


  /**
   * @copydoc AnyITask::inTypeName
   */
  std::string inTypeName() override final {
#if defined( __GLIBCXX__ ) || defined( __GLIBCPP__ )
    int status;
    char *realName = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    std::string ret(realName);

    free(realName);

    return ret;
#else
    return typeid(T).name();
#endif

  }

  /**
   * @copydoc AnyITask::outTypeName
   */
  std::string outTypeName() override final {
#if defined( __GLIBCXX__ ) || defined( __GLIBCPP__ )
    int status;
    char *realName = abi::__cxa_demangle(typeid(U).name(), 0, 0, &status);
    std::string ret(realName);

    free(realName);

    return ret;
#else
    return typeid(U).name();
#endif

  }

  /**
   * @copydoc AnyITask::getAddress
   */
  std::string getAddress() override final {
    return ownerTask->getAddress();
  }

  /**
   * Sets the owner task manager for this ITask
   * @param ownerTask the task manager that owns this ITask
   */
  void setTaskManager(TaskManager<T, U> *ownerTask) {
    this->ownerTask = ownerTask;
  }

  /**
   * Gets the owner task manager for this ITask
   * @return the owner task manager
   */
  TaskManager<T, U> *getOwnerTaskManager() {
    return this->ownerTask;
  }

  /**
   * Gathers profile data.
   * @param taskManagerProfiles the mapping between the task manager and the profile data for the task manager.
   */
  virtual void gatherProfileData(std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles) {}

 private:
  //! @cond Doxygen_Suppress
  typedef AnyITask super;


  template<class V>
  m_data_t<V> getMemory(std::string name, IMemoryReleaseRule *releaseRule, MMType type, size_t nElem) {
    HTGS_ASSERT(this->getMemoryEdges()->find(name) != this->getMemoryEdges()->end(), "Task '" << this->getName() << "' cannot getMemory as it does not have the memory edge '" << name << "'"  );

    auto conn = getMemoryEdges()->find(name)->second;
    auto connector = std::dynamic_pointer_cast<Connector<MemoryData<V>>>(conn);

#ifdef WS_PROFILE
    sendWSProfileUpdate(StatusCode::WAITING_FOR_MEM);
#endif

#ifdef USE_NVTX
    nvtxRangeId_t rangeId = this->getOwnerTaskManager()->getProfiler()->startRangeWaitingForMemory();
#endif

#ifdef PROFILE
    auto start = std::chrono::high_resolution_clock::now();
#endif
    m_data_t<V> memory = connector->consumeData();

#ifdef USE_NVTX
    this->getOwnerTaskManager()->getProfiler()->endRangeWaitingForMem(rangeId);
#endif

#ifdef PROFILE
    auto finish = std::chrono::high_resolution_clock::now();
    this->incMemoryWaitTime(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());
#endif

#ifdef WS_PROFILE
    sendWSProfileUpdate(StatusCode::EXECUTE);
#endif



    memory->setMemoryReleaseRule(releaseRule);

    if (memory->getType() != type) {
      std::cerr
        << "Error: Incorrect usage of getMemory. Dynamic memory managers use 'getDynamicMemory', Static memory managers use 'getMemory' for task '"
        << this->getName() << "' on memory edge " << name << std::endl;
      exit(-1);
    }

    if (type == MMType::Dynamic)
      memory->memAlloc(nElem);

    return memory;
  }

  //! @endcond

  TaskManager<T, U> *ownerTask; //!< The owner task for this ITask


};

}

#endif //HTGS_ITASK_HPP
