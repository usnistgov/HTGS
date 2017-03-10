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
#include "AnyTaskManager.hpp"

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

    getMemoryEdges = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    releaseMemoryEdges = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    mmTypeMap = std::shared_ptr<std::unordered_map<std::string, MMType>>(new std::unordered_map<std::string, MMType>());
    releseMemoryEdgeOutsideGraphMap = std::shared_ptr<std::unordered_map<std::string, bool>>(new std::unordered_map<std::string, bool>());

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

    getMemoryEdges = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    releaseMemoryEdges = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    mmTypeMap = std::shared_ptr<std::unordered_map<std::string, MMType>>(new std::unordered_map<std::string, MMType>());
    releseMemoryEdgeOutsideGraphMap = std::shared_ptr<std::unordered_map<std::string, bool>>(new std::unordered_map<std::string, bool>());

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

    getMemoryEdges = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    releaseMemoryEdges = std::shared_ptr<ConnectorVectorMap> (new ConnectorVectorMap());
    mmTypeMap = std::shared_ptr<std::unordered_map<std::string, MMType>>(new std::unordered_map<std::string, MMType>());
    releseMemoryEdgeOutsideGraphMap = std::shared_ptr<std::unordered_map<std::string, bool>>(new std::unordered_map<std::string, bool>());

    this->pipelineId = 0;
    this->numPipelines = 1;
  }


  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////// VIRTUAL FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Destructor
   */
  virtual ~AnyITask() { }

  /**
   * Pure virtual function to copy an ITask
   * @return the copy of the ITask
   */
  virtual AnyITask *copy() = 0;

  /**
   * Pure virtual function to get the name of an ITask
   * @return the name of the ITask
   */
  virtual std::string getName() = 0;

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
   * @param dotId the id for this task
   * @param input the input connector for this task
   * @param output the output connector for this task
   * @return the dot that represents the interaction between the input/output and the internal custom dot notation
   */
  virtual std::string genDot(int flags, std::string dotId, std::shared_ptr<htgs::AnyConnector> input, std::shared_ptr<htgs::AnyConnector> output) {
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
  virtual void debug() { }

  /**
   * Provides debug output for a node in the dot graph.
   * @return a string representation of the debug output that is added to the dot graph.
   */
  virtual std::string debugDotNode() { return "";}

  /**
   * Virtual function that is called to provide profile output for the ITask
   * @note \#define PROFILE to enable profiling
   */
  virtual void profile() { }

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
  * @internal
  * Copies the ITask including its list of memGetters and memReleasers
  * @return a deep copy of the ITask
  *
  * @note This function should only be called by the HTGS API
  */
  virtual AnyITask *copyITask(bool deep) = 0;

  /**
 * Virtual function that adds additional dot attributes to this node.
 * @param flags the dot gen flags
 * @param dotId the for the node in dot
 * @return the additiona dota attributes for the dot graph representation
 */
  virtual std::string genDot(int flags, std::string dotId) {
    std::string inOutLabel = (((flags & DOTGEN_FLAG_SHOW_IN_OUT_TYPES) != 0) ? ("\nin: "+ this->inTypeName() + "\nout: " + this->outTypeName()) : "");
    std::string threadLabel = (((flags & DOTGEN_FLAG_SHOW_ALL_THREADING) != 0) ? "" : (" x" + std::to_string(this->getNumThreads())));
    return dotId + "[label=\"" + this->getName() +
        (this->debugDotNode() != "" ? ("\n"+this->debugDotNode()+"\n") : "") +
        threadLabel + inOutLabel + "\",shape=box,color=black,width=.2,height=.2];\n";
  }

#ifdef PROFILE
  virtual std::string getDotProfile(int flags,
                                    std::unordered_map<std::string, double> *mmap, double val,
                                    std::string desc, std::unordered_map<std::string, std::string> *colorMap)
  {
    std::string inOutLabel = (((flags & DOTGEN_FLAG_SHOW_IN_OUT_TYPES) != 0) ? ("\nin: "+ this->inTypeName() + "\nout: " + this->outTypeName()) : "");
    std::string threadLabel = (((flags & DOTGEN_FLAG_SHOW_ALL_THREADING) != 0) ? "" : (" x" + std::to_string(this->getNumThreads())));
    return this->getDotId() + "[label=\"" + this->getName() + threadLabel + inOutLabel + "\n" + desc + "\n" + std::to_string(val) + "\",shape=box,style=filled,penwidth=5,fillcolor=white,color=\""+colorMap->at(this->getNameWithPipID()) + "\",width=.2,height=.2];\n";
  }
  virtual void gatherComputeTime(std::unordered_multimap<std::string, long long int> *mmap)  {}
  virtual void gatherWaitTime(std::unordered_multimap<std::string, long long int> *mmap)  {}
  virtual void gatherMaxQSize(std::unordered_multimap<std::string, int> *mmap)  {}
#endif


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
   * @internal
   * Sets the pipeline Id for this ITask.
   * @param pipelineId the pipelineId
   *
   * @note This function should only be used by the HTGS API
   */
  void setPipelineId(size_t pipelineId) {
    this->pipelineId = pipelineId;
  }

  /**
   * Gets the pipeline ID
   * @return the pipeline id
   */
  size_t getPipelineId()
  {
    return this->pipelineId;
  }

  /**
   * @internal
   * Sets the number of pipelines that this ITask belongs too.
   * @param numPipelines the number of pipelines
   *
   * @note This function should only be used by the HTGS API
   */
  void setNumPipelines(size_t numPipelines) {
    this->numPipelines = numPipelines;
  }

  void setConnectorCommunicator(TaskGraphCommunicator *communicator)
  {
    this->connectorCommunicator = communicator;
  }

  TaskGraphCommunicator *getConnectorCommunicator() const {
    return connectorCommunicator;
  }

  /**
   * Gets the number of pipelines for the task's execution pipeline
   * @return the number of pipelines
   */
  size_t getNumPipelines()
  {
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
  void copyMemoryEdges(AnyITask * iTaskCopy)
  {
    iTaskCopy->setGetMemoryEdges(this->getMemoryEdges);
    iTaskCopy->setReleaseMemoryEdges(this->releaseMemoryEdges);
    iTaskCopy->setMMTypeMap(this->mmTypeMap);
    //TODO: This may not be needed . . .
    iTaskCopy->setReleaseMemoryEdgeOutsideGraph(this->releseMemoryEdgeOutsideGraphMap);
  }

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
  m_data_t<V> getMemory(std::string name, IMemoryReleaseRule *releaseRule) {
    return getMemory<V>(name, releaseRule, MMType::Static, 0);
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
  m_data_t<V> getDynamicMemory(std::string name, IMemoryReleaseRule *releaseRule, size_t numElems) {
    return getMemory<V>(name, releaseRule, MMType::Dynamic, numElems);
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
  void getUserManagedMemory(std::string name) {
    getMemory<void *>(name, nullptr, MMType::UserManaged, 0);
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
  void releaseMemory(std::string name, m_data_t<V> memory) {
    releaseMemory<V>(name, memory, MMType::Static);
  }

  /**
   * Releases memory onto a memory edge
   * @param name the name of the memory edge
   * @param memory the memory to be released
   * @tparam V the MemoryData type
   * @note The name specified must have been attached to this ITask as a memReleaser using
   * the TaskGraph::addMemoryManagerEdge routine, which can be verified using hasMemReleaser()
   * @note Memory edge must be defined as MMType::Dynamic
   */
  template<class V>
  void releaseDynamicMemory(std::string name, m_data_t<V> memory) {
    releaseMemory(name, memory, MMType::Dynamic);
  }


  /**
   * Releases memory onto a memory edge
   * @param name the name of the memory edge
   * @param pipelineId the pipelineId to add data to
   * @note The name specified must have been attached to this ITask as a memReleaser using
   * the TaskGraph::addUserManagedEdge routine, which can be verified using hasMemReleaser()
   * @note Memory edge must be defined as MMType::UserManaged by using the TaskGraph::addUserManagedMemoryManagerEdge
   */
  void releaseUserManagedMemory(std::string name, int pipelineId) {
    m_data_t<void *> memory(new MemoryData<void *>(nullptr, name));
    memory->setPipelineId(pipelineId);
    releaseMemory<void *>(name, memory, MMType::UserManaged);
  }



  /**
   * Checks whether this ITask contains a memGetter for a specified name
   * @param name the name of the memGetter edge
   * @return whether this ITask has a memGetter with the specified name
   * @retval TRUE if the ITask has a memGetter with the specified name
   * @retval FALSE if the ITask does not have a memGetter with the specified name
   * @note To add a memGetter to this ITask use TaskGraph::addMemoryManagerEdge
   */
  bool hasGetMemoryEdge(std::string name) {
    return getMemoryEdges->find(name) != getMemoryEdges->end();
  }

  /**
   * Checks whether this ITask contains a memReleaser for a specified name
   * @param name the name of the memReleaser edge
   * @return whether this ITask has a memReleaser with the specified name
   * @retval TRUE if the ITask has a memReleaser with the specified namfe
   * @retval FALSE if the ITask does not have a memReleaser with the specified name
   * @note To add a memReleaser to this ITask use TaskGraph::addMemoryManagerEdge
   */
  bool hasReleaseMemoryEdge(std::string name) {
    return releaseMemoryEdges->find(name) != releaseMemoryEdges->end();
  }

  /**
   *
   * Checks whether this ITask contains a memReleaser that exists outside of the graph that the memory
   * edge exists.
   * @param name the name of the memReleaser edge
   * @return whether the memReleaser's edge exists inside of another graph
   * @retval TRUE if the memReleaser edge is in a graph that this ITask does not belong too
   * @retval FALSE if the memReleaser edge is in the graph that the ITask belongs too
   */
  // TODO: This can be removed . . .
  bool isReleaseMemoryOutsideGraph(std::string name) {
    return this->releseMemoryEdgeOutsideGraphMap->at(name);
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
   // TODO: Rework for communication might simplify sending memory to neighbor pipelines . . .
  void attachGetMemoryEdge(std::string name, std::shared_ptr<AnyConnector> connector, MMType type) {
    std::shared_ptr<ConnectorVector> vector;
    if (hasGetMemoryEdge(name)) {
      vector = getMemoryEdges->find(name)->second;
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
    getMemoryEdges->insert(ConnectorVectorPair(name, vector));
    DEBUG("Num memory getters " << getMemoryEdges->size() << " with " << vector->size() << " connectors");
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
  // TODO: Rework for communication might simplify sending memory to neighbor pipelines . . .
  void attachReleaseMemoryEdge(std::string name, std::shared_ptr<AnyConnector> connector, MMType type, bool outsideMemManGraph) {
    std::shared_ptr<ConnectorVector> vector;
    if (hasReleaseMemoryEdge(name)) {
      vector = releaseMemoryEdges->find(name)->second;
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

    releseMemoryEdgeOutsideGraphMap->insert(std::pair<std::string, bool>(name, outsideMemManGraph));

    vector->push_back(connector);

    mmTypeMap->insert(std::pair<std::string, MMType>(name, type));
    releaseMemoryEdges->insert(ConnectorVectorPair(name, vector));
    DEBUG("Num memory releasers " << releaseMemoryEdges->size() << " with " << vector->size() << " connectors");
  }

  /**
   * Gets the memory manager type for a given name
   * @param name the name of the memory manager edge
   * @return the memory manager type for the specified name
   */
  MMType getMemoryManagerType(std::string name) {
    return this->mmTypeMap->at(name);
  }

  /**
   * Gets the memReleaser mapping
   * @return the map for memReleasers
   */
  std::shared_ptr<ConnectorVectorMap> getReleaseMemoryEdges() {
    return this->releaseMemoryEdges;
  };


  /**
   * Creates a dot notation representation for this task
   * @param input the input connector for this task
   * @param output the output connector for this task
   * @return the dot notation for the task.
   */
  std::string genDot(int flags, std::shared_ptr<AnyConnector> input, std::shared_ptr<AnyConnector> output) {
    std::string dotId = this->getDotId();
    std::ostringstream oss;
    oss << genDot(flags, dotId, input, output);

    if ((flags & DOTGEN_FLAG_HIDE_MEM_EDGES) == 0) {
      if (releaseMemoryEdges->size() > 0) {
        for (const auto &kv : *this->releaseMemoryEdges) {
          // TODO: Should be able to rework this . . . ?
          if (this->isReleaseMemoryOutsideGraph(kv.first)) {
            for (auto connector : *kv.second) {
              oss << dotId << " -> " << connector->getDotId() << "[label=\"release\", color=sienna];" << std::endl;
            }
          } else {
            // TODO: Should no longer have to get pipelineId
            oss << dotId << " -> " << kv.second->at(0)->getDotId() << "[label=\"release\", color=sienna];" << std::endl;
//            oss << dotId << " -> " << kv.second->at((unsigned long) this->pipelineId)->getDotId() << "[label=\"release\", color=sienna];" << std::endl;
          }
        }

      }

      if (getMemoryEdges->size() > 0) {
        for (const auto &kv : *this->getMemoryEdges) {
          // TODO: Should no longer have to get pipelineId
//          oss << kv.second->at((unsigned long) this->pipelineId)->getDotId() << " -> " << dotId << "[label=\"get\", color=sienna];" << std::endl;
          oss << kv.second->at(0)->getDotId() << " -> " << dotId << "[label=\"get\", color=sienna];" << std::endl;
        }
      }
    }

    return oss.str();
  }

  /**
   * @internal
   * Provides profile output for the ITask,
   *
   * @note this function should only be called by the HTGS API
   */
  void profileITask() {
    if (releaseMemoryEdges->size() > 0) {
      for (const auto &kv : *this->releaseMemoryEdges) {
        std::cout << "Mem releaser: " << kv.first << " profile; ";
        // produce
        // TODO: Should no longer have to get pipelineId
        kv.second->at((unsigned long) this->pipelineId)->profileProduce(this->numThreads);

      }

    }

    if (getMemoryEdges->size() > 0) {
      for (const auto &kv : *this->getMemoryEdges) {
        std::cout << "Mem getter: " << kv.first << " profile; ";
        // consume
        // TODO: Should no longer have to get pipelineId
        kv.second->at((unsigned long) this->pipelineId)->profileConsume(this->numThreads, false);

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
    id << this;
    std::string idStr = id.str();
    idStr.erase(0, 1);

    return idStr;
  }

  /**
   * Gets the name of the ITask with it's pipeline ID
   * @return  the name of the task with the pipeline ID
   */
  std::string getNameWithPipelineId() { return this->getName() + std::to_string(this->pipelineId); }

 private:

  //! @cond Doxygen_Suppress
  void setGetMemoryEdges(std::shared_ptr<ConnectorVectorMap> memGetter) { this->getMemoryEdges = memGetter; }
  void setMMTypeMap(std::shared_ptr<std::unordered_map<std::string, MMType>> pMap) { this->mmTypeMap = pMap; }
  void setReleaseMemoryEdges(std::shared_ptr<ConnectorVectorMap> memReleaser) {
    this->releaseMemoryEdges = memReleaser;
  }
  // TODO: Hopefully can get rid of this
  void setReleaseMemoryEdgeOutsideGraph(std::shared_ptr<std::unordered_map<std::string, bool>> memReleaserOutsideGraph) {
    this->releseMemoryEdgeOutsideGraphMap = memReleaserOutsideGraph;
  }

  // TODO: This may be reworked to no longer need the pipeline Id for getting memory
  template<class V>
  m_data_t<V> getMemory(std::string name, IMemoryReleaseRule *releaseRule, MMType type, size_t nElem)
  {
    assert(this->mmTypeMap->find(name) != this->mmTypeMap->end());
    assert(this->mmTypeMap->find(name)->second == type);

    auto conn = getMemoryEdges->find(name)->second->at(this->pipelineId);
    auto connector = std::dynamic_pointer_cast<Connector<MemoryData<V>>>(conn);

    m_data_t<V> memory = connector->consumeData();

    memory->setMemoryReleaseRule(releaseRule);

    if (type == MMType::Dynamic)
      memory->memAlloc(nElem);

    return memory;
  }

  // TODO: This may be reworked to no longer get the release memory with the pipeline Id
  // TODO: Could instead check the pipeline id of this task and the memory, if they are different then auto send to comm channel
  template<class V>
  void releaseMemory(std::string name, m_data_t<V> memory, MMType type)
  {
    assert(this->mmTypeMap->find(name) != this->mmTypeMap->end());
    assert(this->mmTypeMap->find(name)->second == type);

    auto conn = releaseMemoryEdges->find(name)->second->at(memory->getPipelineId());
    auto connector = std::dynamic_pointer_cast<Connector<MemoryData<V>>>(conn);

    connector->produceData(memory);

  }
  //! @endcond


  size_t numThreads; //!< The number of threads to be used with this ITask (forms a thread pool) used when creating a TaskManager
  bool startTask; //!< Whether the ITask will be a start task used when creating a TaskManager
  bool poll; //!< Whether the ITask should poll for data used when creating a TaskManager
  size_t microTimeoutTime; //!< The timeout time for polling in microseconds used when creating a TaskManager
  size_t pipelineId; //!< The execution pipeline id for the ITask
  size_t numPipelines;

  std::shared_ptr<std::unordered_map<std::string, bool>> releseMemoryEdgeOutsideGraphMap; //!< A mapping from a memory edge name to whether that memory edge is inside of another graph
  std::shared_ptr<ConnectorVectorMap> getMemoryEdges; //!< A mapping from memory edge name to memory manager connector for getting memory
  std::shared_ptr<ConnectorVectorMap> releaseMemoryEdges; //!< A mapping from memory edge name to memory manager connector for releasing memory
  std::shared_ptr<std::unordered_map<std::string, MMType>> mmTypeMap; //!< A mapping from memory edge name to memory manager type

  TaskGraphCommunicator *connectorCommunicator; //!< Task graph connector communicator

};
}

#endif //HTGS_ANYITASK_HPP