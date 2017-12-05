// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file AnyITask.hpp
 * @author Timothy Blattner
 * @date Nov 23, 2015
 *
 * @brief Implements parent ITask, removing template arguments.
 * @details
 */
#ifndef HTGS_ANYITASK_HPP
#define HTGS_ANYITASK_HPP

#include <memory>
#include <cassert>
#include <sstream>

#include <htgs/api/MemoryData.hpp>
#include <htgs/core/graph/Connector.hpp>
#include <htgs/core/graph/AnyConnector.hpp>
#include <htgs/types/Types.hpp>
#include <htgs/types/MMType.hpp>
#include <htgs/debug/debug_message.hpp>
#include <htgs/types/TaskGraphDotGenFlags.hpp>

#ifdef WS_PROFILE
#include <htgs/core/graph/profile/ProfileData.hpp>
#include <htgs/core/graph/profile/CustomProfile.hpp>
#endif

#include <htgs/core/task/AnyTaskManager.hpp>

namespace htgs {

/**
 * @class AnyITask AnyITask.hpp <htgs/core/task/AnyITask.hpp>
 * @brief Implements the parent ITask, which removes the template arguments of an ITask.
 * @details
 * Used anywhere the template arguments for an ITask are not needed.
 *
 */
class AnyITask {
 public:

  /**
 * Creates an ITask with number of threads equal to 1.
 */
  AnyITask() {
    this->numThreads = 1;
    this->startTask = false;
    this->poll = false;
    this->microTimeoutTime = 0;
    this->memoryWaitTime = 0;

    memoryEdges = std::shared_ptr<ConnectorMap>(new ConnectorMap());
    releaseMemoryEdges = std::shared_ptr<ConnectorMap>(new ConnectorMap());

    this->pipelineId = 0;
    this->numPipelines = 1;
  }

  /**
 * Constructs an ITask with a specified number of threads.
 * @param numThreads the number of threads associated with this ITask
 */
  AnyITask(size_t numThreads) {
    this->numThreads = numThreads;
    this->startTask = false;
    this->poll = false;
    this->microTimeoutTime = 0L;
    this->memoryWaitTime = 0;

    memoryEdges = std::shared_ptr<ConnectorMap>(new ConnectorMap());
    releaseMemoryEdges = std::shared_ptr<ConnectorMap>(new ConnectorMap());

    this->pipelineId = 0;
    this->numPipelines = 1;
  }

  /**
   * Constructs an ITask with a specified number of threads as well as additional scheduling options.
   * @param numThreads the number of threads associated with this ITask
   * @param isStartTask whether this ITask starts executing immediately and passes nullptr to executeTask()
   * @param poll whether this task will poll for data, if the timeout period expires, nullptr is passed to executeTask()
   * @param microTimeoutTime the timeout period for checking for data
   * @note If the ITask is declared as a start task or is polling, then executeTask() should properly handle nullptr data
   */
  AnyITask(size_t numThreads, bool isStartTask, bool poll, size_t microTimeoutTime) {
    this->numThreads = numThreads;
    this->startTask = isStartTask;
    this->poll = poll;
    this->microTimeoutTime = microTimeoutTime;
    this->memoryWaitTime = 0;

    memoryEdges = std::shared_ptr<ConnectorMap>(new ConnectorMap());
    releaseMemoryEdges = std::shared_ptr<ConnectorMap>(new ConnectorMap());

    this->pipelineId = 0;
    this->numPipelines = 1;
  }


  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////// VIRTUAL FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Destructor
   */
  virtual ~AnyITask() {}

  /**
   * Pure virtual function to copy an ITask
   * @return the copy of the ITask
   */
  virtual AnyITask *copy() = 0;

  /**
   * Virtual function to get the name of an ITask
   * @return the name of the ITask
   */
  virtual std::string getName() = 0;

  /**
   * Virtual function to get the label name used for dot graph viz.
   * @return the label name used for graphviz
   */
  virtual std::string getDotLabelName() = 0;

  /**
   * Gets the color of the shape for graphviz dot
   * @return the shape color
   */
  virtual std::string getDotShapeColor() = 0;

  /**
   * Gets the color for filling the shape for graphviz dot
   * @return the fill color
   */
  virtual std::string getDotFillColor() = 0;

  /**
   * Gets the shape for graphviz dot
   * @return the shape
   */
  virtual std::string getDotShape() = 0;

  /**
   * Virtual function that can be used to add custom output for dot visualizations
   * @return the values for dot visualization added to the label for the task's dot node,
   * use '\n' to create newlines to add additional profile data
   */
  virtual std::string getDotCustomProfile() = 0;

  /**
 * Virtual function that is called when an ITask is being shutdown by it's owner thread.
 */
  virtual void shutdown() = 0;

  /**
   * Virtual function that is called when an ITask is being initialized by it's owner thread.
  */
  virtual void initialize() = 0;

  /**
 * Virtual function that is called when an ITask is checking if it can be terminated
 * @param inputConnector the connector responsible for giving data to this Task
 * @return whether the ITask can be terminated or not
 * @retval TRUE if the ITask is ready to be terminated
 * @retval FALSE if the ITask is not ready to be terminated
 * @note By default this function checks if the input no longer sending data using inputConnector->isInputTerminated()
 */
  virtual bool canTerminate(std::shared_ptr<AnyConnector> inputConnector) = 0;

  /**
   * Virtual function that generates the input/output and per-task dot notation
   * @param flags the DOTGEN flags
   * @param dotId the id for this task
   * @param input the input connector for this task
   * @param output the output connector for this task
   * @return the dot that represents the interaction between the input/output and the internal custom dot notation
   */
  virtual std::string genDot(int flags,
                             std::string dotId,
                             std::shared_ptr<htgs::AnyConnector> input,
                             std::shared_ptr<htgs::AnyConnector> output) {
    std::ostringstream oss;

    if (input != nullptr) {
      oss << input->getDotId() << " -> " << dotId << ";" << std::endl;
      oss << input->genDot(flags);
    }

    if (output != nullptr) {
      oss << dotId << " -> " << output->getDotId() << ";" << std::endl;
      oss << output->genDot(flags);
    }

    oss << genDot(flags, dotId);

    return oss.str();
  }

  /**
  * Virtual function that is called to debug the ITask
  */
  virtual void debug() {}

  /**
   * Provides debug output for a node in the dot graph.
   * @return a string representation of the debug output that is added to the dot graph.
   */
  virtual std::string debugDotNode() { return ""; }

  /**
   * Virtual function that is called to provide profile output for the ITask
   * @note \#define PROFILE to enable profiling
   */
  virtual void profile() {}

  /**
   * Virtual function that is called after executionTask is called. This can be used to provide
   * detailed profile (or debug) data to be sent for visualization using the HTGS_Visualizer.
   * The format uses <key>:<value> pairs, separated by semi-colons.
   *
   * i.e.: gflops:<#>;bandwidth:<#>;...
   *
   * @return the string representation as key value pairs
   */
  virtual std::string profileStr() { return ""; }

  /**
  * Gets the demangled input type name of the connector
  * @return the demangled type name for the input
  */
  virtual std::string inTypeName() = 0;

  /**
  * Gets the demangled output type name of the connector
  * @return the demangled output type name for the input
  */
  virtual std::string outTypeName() = 0;

  /**
   * Gets the address from the owner task, which is the address of the task graph.
   * @return the address
   */
  virtual std::string getAddress() = 0;

  /**
  *
  * Copies the ITask including its list of memGetters and memReleasers
  * @return a deep copy of the ITask
  *
  * @note This function should only be called by the HTGS API
   * @internal
  */
  virtual AnyITask *copyITask(bool deep) = 0;

  /**
   * Prints the profile data to std::out.
   */
  virtual void printProfile() = 0;

  /**
 * Virtual function that adds additional dot attributes to this node.
 * @param flags the dot gen flags
 * @param dotId the for the node in dot
 * @return the additiona dota attributes for the dot graph representation
 */
  virtual std::string genDot(int flags, std::string dotId) {
    return dotId + ";\n";
//    std::string inOutLabel = (((flags & DOTGEN_FLAG_SHOW_IN_OUT_TYPES) != 0) ? ("\nin: "+ this->inTypeName() + "\nout: " + this->outTypeName()) : "");
//    std::string threadLabel = (((flags & DOTGEN_FLAG_SHOW_ALL_THREADING) != 0) ? "" : (" x" + std::to_string(this->getNumThreads())));
//    return dotId + "[label=\"" + this->getName() +
//        (this->debugDotNode() != "" ? ("\n"+this->debugDotNode()+"\n") : "") +
//        threadLabel + inOutLabel + "\",shape=box,color=black,width=.2,height=.2];\n";
  }

  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Virtual function that is called when an ITask is being initialized by it's owner thread.
   * @param pipelineId the pipelineId, only used if the ITask is inside of an ExecutionPipeline
   * @param numPipeline the number of pipelines, only used if the ITask is inside of an ExecutionPipeline
  */
  void initialize(size_t pipelineId, size_t numPipeline) {
    this->pipelineId = pipelineId;
    this->numPipelines = numPipeline;
    this->initialize();
  }

  /**
   * Sets the pipeline Id for this ITask.
   * @param pipelineId the pipelineId
   *
   * @note This function should only be used by the HTGS API
   * @internal
   */
  void setPipelineId(size_t pipelineId) {
    this->pipelineId = pipelineId;
  }

  /**
   * Gets the pipeline ID
   * @return the pipeline id
   */
  size_t getPipelineId() {
    return this->pipelineId;
  }

  /**
   * Sets the number of pipelines that this ITask belongs too.
   * @param numPipelines the number of pipelines
   *
   * @note This function should only be used by the HTGS API
   * @internal
   */
  void setNumPipelines(size_t numPipelines) {
    this->numPipelines = numPipelines;
  }

  /**
   * Sets the task graph communicator.
   * @param communicator
   */
  void setTaskGraphCommunicator(TaskGraphCommunicator *communicator) {
    this->taskGraphCommunicator = communicator;
  }

  /**
   * Gets the task graph communicator
   * @return the task graph communicator
   */
  TaskGraphCommunicator *getTaskGraphCommunicator() const {
    return taskGraphCommunicator;
  }

  /**
   * Gets the number of pipelines for the task's execution pipeline
   * @return the number of pipelines
   */
  size_t getNumPipelines() const {
    return this->numPipelines;
  }

  /**
   * Gets the number of threads associated with this ITask
   * @return the number of threads
   */
  size_t getNumThreads() const {
    return this->numThreads;
  }

  /**
   * Gets whether this ITask is a starting task
   * @return whether the ITask is a starting task
   * @retval TRUE if the task will begin execution immediately when it is bound to a thread
   * @retval FALSE if the task must wait for data before it can begin exection
   */
  bool isStartTask() const {
    return this->startTask;
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
  size_t getMicroTimeoutTime() const {
    return this->microTimeoutTime;
  }

  /**
   * Copies the memory edges from this AnyITask to another AnyITask
   * @param iTaskCopy the other AnyITask to copy the memory edges too
   */
  void copyMemoryEdges(AnyITask *iTaskCopy) {
    iTaskCopy->setMemoryEdges(this->memoryEdges);
    iTaskCopy->setReleaseMemoryEdges(this->releaseMemoryEdges);
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
  void releaseMemory(m_data_t<V> memory) {
    std::shared_ptr<DataPacket> dataPacket = std::shared_ptr<DataPacket>(new DataPacket(this->getName(),
                                                                                        this->getAddress(),
                                                                                        memory->getMemoryManagerName(),
                                                                                        memory->getAddress(),
                                                                                        memory));
    this->taskGraphCommunicator->produceDataPacket(dataPacket);
  }

  /**
   * Checks whether this ITask contains a memory edge for a specified name
   * @param name the name of the memGetter edge
   * @return whether this ITask has a memGetter with the specified name
   * @retval TRUE if the ITask has a memGetter with the specified name
   * @retval FALSE if the ITask does not have a memGetter with the specified name
   * @note To add a memGetter to this ITask use TaskGraph::addMemoryManagerEdge
   */
  bool hasMemoryEdge(std::string name) {
    return memoryEdges->find(name) != memoryEdges->end();
  }

  /**
   * Attaches a memory edge to this ITask to get memory
   * @param name the name of the memory edge
   * @param getMemoryConnector the connector for getting memory for the MemoryManager
   * @param releaseMemoryConnector the connector for releasing memory for the MemoryManager
   * @param type the memory manager type
   *
   * @note This function should only be called by the HTGS API, use TaskGraph::addMemoryManagerEdge instead.
   * @internal
   */
  void attachMemoryEdge(std::string name, std::shared_ptr<AnyConnector> getMemoryConnector,
                        std::shared_ptr<AnyConnector> releaseMemoryConnector, MMType type) {
    if (hasMemoryEdge(name)) {
      std::cerr << "ERROR: " << this->getName() << " already has a memory edge named " << name << std::endl;
    } else {
      memoryEdges->insert(ConnectorPair(name, getMemoryConnector));
      releaseMemoryEdges->insert(ConnectorPair(name, releaseMemoryConnector));
    }

    DEBUG("Num memory getters " << memoryEdges->size());
  }

  /**
   * Creates a dot notation representation for this task
   * @param flags the DOTGEN flags
   * @param input the input connector for this task
   * @param output the output connector for this task
   * @return the dot notation for the task.
   */
  std::string genDot(int flags, std::shared_ptr<AnyConnector> input, std::shared_ptr<AnyConnector> output) {
    std::string dotId = this->getDotId();
    std::ostringstream oss;
    oss << genDot(flags, dotId, input, output);

    if ((flags & DOTGEN_FLAG_HIDE_MEM_EDGES) == 0) {
      if (memoryEdges->size() > 0) {
        for (const auto &kv : *this->memoryEdges) {
          oss << kv.second->getDotId() << " -> " << dotId << "[label=\"get\", color=sienna];" << std::endl;
        }
      }
    }

    return oss.str();
  }

  /**
   * Provides profile output for the ITask,
   *
   * @note this function should only be called by the HTGS API
   * @internal
   */
  void profileITask() {
    if (memoryEdges->size() > 0) {
      for (const auto &kv : *this->memoryEdges) {
        std::cout << "Mem getter: " << kv.first << " profile; ";
        // consume
        kv.second->profileConsume(this->numThreads, false);
      }
    }
    this->profile();
  }

  /**
   * Gets the id used for dot nodes.
   * @return the unique dot id (uses the memory address of this AnyITask)
   */
  std::string getDotId() {
    std::ostringstream id;
    id << "x" << this;
    std::string idStr = id.str();

    return idStr;
  }

  /**
   * Gets the name of the ITask with it's pipeline ID
   * @return  the name of the task with the pipeline ID
   */
  std::string getNameWithPipelineId() { return this->getName() + std::to_string(this->pipelineId); }

  /**
   * Gets the memory edges for the task
   * @return the memory edges
   */
  const std::shared_ptr<ConnectorMap> &getMemoryEdges() const {
    return memoryEdges;
  }

  /**
   * Gets the memory edges for releasing memory for the memory manager, used to shutdown the memory manager.
   * @return the mapping between the memory edge name and the memory release connectors
   */
  const std::shared_ptr<ConnectorMap> &getReleaseMemoryEdges() const {
    return releaseMemoryEdges;
  }


  /**
   * Gets the amount of time the task was waiting for memory
   * @return the amount of time the task waited for memory
   */
  unsigned long long int getMemoryWaitTime() const {
    return memoryWaitTime;
  }

#ifdef WS_PROFILE
  void sendWSProfileUpdate(StatusCode code)
  {
    if (this->getName() == "WebSocketProfiler")
      return;
    std::shared_ptr<ProfileData> updateStatus(new ChangeStatusProfile(this, code));
    std::shared_ptr<DataPacket> dataPacket(new DataPacket(this->getName(), this->getAddress(), "WebSocketProfiler", "0", updateStatus));
    this->taskGraphCommunicator->produceDataPacket(dataPacket);
  }
#endif

 private:
  //! @cond Doxygen_Suppress
  void setMemoryEdges(std::shared_ptr<ConnectorMap> memGetter) { this->memoryEdges = memGetter; }

  void setReleaseMemoryEdges(const std::shared_ptr<ConnectorMap> &releaseMemoryEdges) {
    AnyITask::releaseMemoryEdges = releaseMemoryEdges;
  }

  template<class V>
  m_data_t<V> getMemory(std::string name, IMemoryReleaseRule *releaseRule, MMType type, size_t nElem) {
    assert(("Unable to find memory edge 'name' for task", this->memoryEdges->find(name) != this->memoryEdges->end()));

    auto conn = memoryEdges->find(name)->second;
    auto connector = std::dynamic_pointer_cast<Connector<MemoryData<V>>>(conn);

#ifdef WS_PROFILE
    sendWSProfileUpdate(StatusCode::WAITING_FOR_MEM);
#endif

#ifdef PROFILE
    auto start = std::chrono::high_resolution_clock::now();
#endif
    m_data_t<V> memory = connector->consumeData();
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
          << "Error: Incorrect usage of getMemory. Dynamic memory managers use 'getDynamicMemory', Static memory managers use 'getMemory' for task "
          << this->getName() << " on memory edge " << name << std::endl;
      exit(-1);
    }

    if (type == MMType::Dynamic)
      memory->memAlloc(nElem);

    return memory;
  }
  //! @endcond


  /**
   * Increments memory wait time
   * @param val the amount of time passed waiting for memory
   */
  void incMemoryWaitTime(unsigned long long int val) { this->memoryWaitTime += val; }

  size_t
      numThreads; //!< The number of threads to be used with this ITask (forms a thread pool) used when creating a TaskManager
  bool startTask; //!< Whether the ITask will be a start task used when creating a TaskManager
  bool poll; //!< Whether the ITask should poll for data used when creating a TaskManager
  size_t microTimeoutTime; //!< The timeout time for polling in microseconds used when creating a TaskManager
  size_t pipelineId; //!< The execution pipeline id for the ITask
  size_t numPipelines; //!< The number of pipelines that exist for this task

  std::shared_ptr<ConnectorMap>
      memoryEdges; //!< A mapping from memory edge name to memory manager connector for getting memory
  std::shared_ptr<ConnectorMap>
      releaseMemoryEdges; //!< A mapping from the memory edge name to the memory manager's input connector to shutdown the memory manager
  TaskGraphCommunicator *taskGraphCommunicator; //!< Task graph connector communicator

  unsigned long long int memoryWaitTime; //!< The amount of time this task waited for memory

};
}

#endif //HTGS_ANYITASK_HPP