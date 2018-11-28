// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file AnyTaskGraphConf.hpp
 * @author Timothy Blattner
 * @date February 24, 2017
 *
 * @brief Implements the base class used by the TaskGraphConf, which removes the template arguments
 * and implements functions specific to any task graph configuration.
 */
#ifndef HTGS_ANYTASKGRAPHCONF_HPP
#define HTGS_ANYTASKGRAPHCONF_HPP

#include <list>
#include <string>
#include <cstddef>
#include <fstream>

#include <htgs/core/task/TaskManager.hpp>
#include <htgs/api/ITask.hpp>
#include <htgs/api/IRule.hpp>
#include <htgs/core/graph/edge/EdgeDescriptor.hpp>
#include <htgs/core/task/AnyITask.hpp>
#include <htgs/types/Types.hpp>
#include <htgs/core/graph/profile/TaskManagerProfile.hpp>
#ifdef WS_PROFILE
#include <htgs/core/graph/profile/ProfileData.hpp>
#endif

namespace htgs {

/**
 * @typedef ITaskMap
 * Creates a mapping between an ITask and a task manager.
 */
typedef std::map<AnyITask *, AnyTaskManager *> ITaskMap;

/**
 * @typedef ITaskPair
 * Defines a pair to be added into an ITaskMap
 */
typedef std::pair<AnyITask *, AnyTaskManager *> ITaskPair;

/**
 * @typedef TaskNameConnectorMap
 * Defines multiple mappings between the task name and its connector.
 */
typedef std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>> TaskNameConnectorMap;

/**
 * @typedef TaskNameConnectorPair
 * Defines a pair to be added into a TaskNameConnectorMap
 */
typedef std::pair<std::string, std::shared_ptr<AnyConnector>> TaskNameConnectorPair;

/**
 * @class AnyTaskGraphConf AnyTaskGraphConf.hpp <htgs/core/graph/AnyTaskGraphConf.hpp>
 * @brief Implements the base class for the TaskGraphConf class, removing the template arguments
 * and providing functionality that is applicable to any task graph configuration.
 *
 * For example, storing the base address, pipeline ID, etc.
 *
 */
class AnyTaskGraphConf {
 public:

  /**
   * Constructs the AnyTaskGraphConf
   * @param pipelineId the pipeline ID associated with this task graph
   * @param numPipelines the number of pipelines that exist for the task graph
   * @param baseAddress the base address for the graph, if it is an empty string then this
   * graph is the first/root graph.
   */
  AnyTaskGraphConf(size_t pipelineId, size_t numPipelines, std::string baseAddress) :
      pipelineId(pipelineId), numPipelines(numPipelines) {
    this->graphCreationTimestamp = std::chrono::high_resolution_clock::now();
    this->graphComputeTime = 0;
    this->graphCreationTime = 0;

    // TODO: Delete or Add #ifdef
//    if (baseAddress == "")
//      this->address = std::to_string(pipelineId);
//    else
//      this->address = baseAddress + ":" + std::to_string(this->pipelineId);
    this->taskManagers = new std::list<AnyTaskManager *>();
    this->taskCopyMap = new ITaskMap();
    this->taskConnectorNameMap = new TaskNameConnectorMap();
    this->numberOfSubGraphs = 0;
    this->iRuleMap = new IRuleMap();
    this->memAllocMap = new MemAllocMap();
  }

  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////// VIRTUAL FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Destructor
   */
  virtual ~AnyTaskGraphConf() {
    for (auto task : *taskManagers) {
      if (task != nullptr) {
        HTGS_DEBUG_VERBOSE("AnyTaskGraphConf: " << this << " Freeing memory for TaskManager: " << task);

        delete task;
        task = nullptr;
      }
    }
    delete taskManagers;

    delete taskCopyMap;
    taskCopyMap = nullptr;

    delete taskConnectorNameMap;
    taskConnectorNameMap = nullptr;

    delete iRuleMap;
    iRuleMap = nullptr;

    delete memAllocMap;
    memAllocMap = nullptr;
  }

  /**
   * Pure virtual function that gets the task manager that is consuming data from the graph's input
   * @return the task manager that is consuming data from the graph's input.
   */
  virtual AnyTaskManager *getGraphConsumerTaskManager() = 0;

  /**
   * Gets the list of task managers that are producing data for the graph's output
   * @return the list of task managers that are producing data for the graph's output.
   */
  virtual std::list<AnyTaskManager *> *getGraphProducerTaskManagers() = 0;

  /**
   * Virtual function that gets the connector used for graph input
   * @return the connector used for graph input
   */
  virtual std::shared_ptr<AnyConnector> getInputConnector() = 0;

  /**
   * Virtual function that gets the connector used for graph output
   * @return the connector used for graph output
   */
  virtual std::shared_ptr<AnyConnector> getOutputConnector() = 0;

  // TODO: Delete or Add #ifdef
//  /**
//   * Virtual function that initiates updating the task graph communicator.
//   */
//  virtual void updateCommunicator() = 0;

  // TODO: Delete or Add #ifdef
//  /**
//   * Virtual function that gets the task graph communicator.
//   * @return
//   */
//  virtual TaskGraphCommunicator *getTaskGraphCommunicator() const = 0;

#ifdef WS_PROFILE
  /**
   * Virtual function to send profile data directly to the WebSocketProfiler
   * @param profileData the profiling data
   * @note Must define WS_PROFILE directive
   */
  virtual void sendProfileData(std::shared_ptr<ProfileData> profileData) = 0;
#endif


  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
  * Get the vertices of the TaskGraph
  * @return the vertices of the TaskGraph
  */
  std::list<AnyTaskManager *> *getTaskManagers() {
    return this->taskManagers;
  }

  /**
   * Gathers profiling data for this task graph's task managers, which is added into the
   * task manager profiles map.
   * @param taskManagerProfiles the map that stores all the profile data for each task manager.
   */
  void gatherProfilingData(std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles) {
    for (auto tMan : *taskManagers) {
      tMan->gatherProfileData(taskManagerProfiles);
    }
  }

  /**
   * Gets the shared_ptr reference for a particular IRule
   * @tparam V the input type of the IRule
   * @tparam W the output type of the IRule
   * @param iRule the IRule
   * @return the shared_ptr reference to the IRule
   */
  template<class V, class W>
  std::shared_ptr<IRule<V, W>> getIRule(IRule<V, W> *iRule) {
    std::shared_ptr<IRule<V, W>> iRuleShr;
    if (this->iRuleMap->find(iRule) != this->iRuleMap->end()) {
      std::shared_ptr<AnyIRule> baseRulePtr = this->iRuleMap->find(iRule)->second;
      iRuleShr = std::static_pointer_cast<IRule<V, W>>(baseRulePtr);
    } else {
      iRuleShr = std::shared_ptr<IRule<V, W>>(iRule);
      this->iRuleMap->insert(IRulePair(iRule, iRuleShr));
    }
    return iRuleShr;
  }

  /**
   * Gets the shared_ptr reference for a particular IMemoryAllocator.
   * @tparam V the data type that is allocated
   * @param allocator the IMemoryAllocator
   * @return the shared_ptr reference to the IMemoryAllocator
   */
  template<class V>
  std::shared_ptr<IMemoryAllocator<V>> getMemoryAllocator(IMemoryAllocator<V> *allocator) {
    std::shared_ptr<IMemoryAllocator<V>> allocP;

    if (this->memAllocMap->find(allocator) == this->memAllocMap->end()) {
      allocP = std::shared_ptr<IMemoryAllocator<V>>(allocator);
      memAllocMap->insert(MemAllocPair(allocator, allocP));
    } else {
      allocP = std::static_pointer_cast<IMemoryAllocator<V>>(this->memAllocMap->at(allocator));
    }

    return allocP;
  }

  /**
   * Initializes the task graph just prior to spawning threads.
   */
  void initialize() {
    // TODO: Delete or Add #ifdef
//    this->getTaskGraphCommunicator()->setNumGraphsSpawned(this->getNumberOfSubGraphs());

    this->updateTaskManagersAddressingAndPipelines();
//    this->updateCommunicator();



    auto endTime = std::chrono::high_resolution_clock::now();
    this->graphCreationTime =
      static_cast<unsigned long long int>(std::chrono::duration_cast<std::chrono::microseconds>(endTime - graphCreationTimestamp).count());

    this->graphExecutingTimestamp = std::chrono::high_resolution_clock::now();
  }

  /**
   * Called when the task graph has finished setting up its tasks and launched all threads for the graph.
   */
  virtual void finishedSetup() { }

  /**
   * Called when all the threads in this graph have finished executing.
   */
  void shutdown() {
    auto endTime = std::chrono::high_resolution_clock::now();

    graphComputeTime =
      static_cast<unsigned long long int>(std::chrono::duration_cast<std::chrono::microseconds>(endTime - graphExecutingTimestamp).count());

  }

  /**
   *  Waits for all task managers to finish initializing. When this returns it is safe to assume that
   *  all tasks have been initialized from the graph.
   *  @note Only call this function after htgs::TaskGraphRuntime::executeRuntime has been called for the graph.
   */
    void waitForInitialization() {
      std::unique_lock<std::mutex> lock(this->initializeMutex);
      this->initializeCondition.wait(lock, [=]
      {
        bool ret = true;
        for (AnyTaskManager *tm : *taskManagers)
        {
          if(!tm->isInitialized()) {
            ret = false;
            break;
          }
        }

        return ret;
      });

    }

    /**
     * Notifies the task graph to check if all task managers have been initialized or not.
     * @note
     */
    std::condition_variable *getInitializationCondition() {
      return &this->initializeCondition;
    }

    /**
     * Gets the initialization mutex, used for signaling when initialization is done.
     * @return the initialization mutex
     */
    std::mutex *getInitializationMutex() {
      return &this->initializeMutex;
    }

  /**
   * Gets the task name connector map that maps the task name to its input connector.
   * @return the task name connector map that namps the task name to its input connector.
   */
  TaskNameConnectorMap *getTaskConnectorNameMap() const {
    return taskConnectorNameMap;
  }

  /**
   * Gets the copy for an ITask based on some original ITask reference.
   * This function is used to find the associated ITask reference to ensure connections are
   * maintained when copying the TaskGraphConf.
   * @tparam T the input type of the ITask
   * @tparam U the output type of the ITask
   * @param orig the pointer to the original ITask that may have been copied before
   * @return the ITask's copy or nullptr if the copy is not found.
   */
  template<class T, class U>
  ITask<T, U> *getCopy(ITask<T, U> *orig) {
    for (auto tCopy : *taskCopyMap) {
      if (tCopy.first == orig) {
        return (ITask<T, U> *) tCopy.second->getTaskFunction();
      }
    }

    return nullptr;
  }

  /**
   * Gets the copy for an AnyITask based on some original AnyITask reference.
   * This function is used to find the associated ITask reference to ensure connections are
   * maintained when copying the TaskGraphConf. This version does not use the template arguments.
   * @param orig the pointer to the original AnyITask that may have been copied before
   * @return the AnyITask's copy or nullptr if the copy is not found.
   */
  AnyITask *getCopy(AnyITask *orig) {
    for (auto tCopy : *taskCopyMap) {
      if (tCopy.first == orig) {
        return tCopy.second->getTaskFunction();
      }
    }

    return nullptr;
  }

  /**
   * Gets the task manager that is responsible for a particular ITask
   * @tparam T the input type of the TaskManager/ITask
   * @tparam U the output type of the TaskManager/ITask
   * @param task the ITask
   * @return the task manager responsible for the ITask
   */
  template<class T, class U>
  TaskManager<T, U> *getTaskManager(ITask<T, U> *task) {

    TaskManager<T, U> *taskManager = nullptr;

    for (auto tSched : *taskManagers) {
      if (tSched->getTaskFunction() == task) {
        taskManager = (TaskManager<T, U> *) tSched;
        break;
      }
    }

    if (taskManager == nullptr) {
      taskManager = new TaskManager<T, U>(task,
                                          task->getNumThreads(),
                                          task->isStartTask(),
                                          task->isPoll(),
                                          task->getMicroTimeoutTime(),
                                          pipelineId,
                                          numPipelines,
                                          address);
      this->taskManagers->push_back(taskManager);

      // Increment number of graphs spawned from the task
      this->numberOfSubGraphs += task->getNumGraphsSpawned();
    }

    return taskManager;

  }

  /**
   * Adds a task manager to the task graph
   * @param taskManager the task manager
   */
  void addTaskManager(AnyTaskManager *taskManager) {
    for (auto tMan : *taskManagers) {
      if (tMan == taskManager)
        return;
    }

    this->taskManagers->push_back(taskManager);
  }

  /**
   * Prints profile data to console for all task managers
   */
  void printProfile() {
    for (auto tMan : *taskManagers) {
      tMan->printProfile();
    }
  }

  /**
   * Gets the pipeline ID for the task graph configuration.
   * @return the pipeline ID
   */
  size_t getPipelineId() { return this->pipelineId; }

  /**
   * Gets the number of pipelines that exist for this task graph
   * @return the number of pipelines
   */
  size_t getNumPipelines() { return this->numPipelines; }

  /**
   * Writes the dot representation of the task graph to disk with additional options such as
   * profiling.
   *
   * Example:
   * taskGraph->writeDotToFile("example.dot", DOTGEN_FLAG_HIDE_MEM_EDGES | DOTGEN_FLAG_SHOW_IN_OUT_TYPES);
   *
   * The bit flags are aggregated using bit-wise OR operator.
   *
   * @param file the filename (will not create directories)
   * @param flags the flags for DOTGEN
   * @param graphTitle the title of the graph that is inserted into a graph title section in the visualization
   * @param customTitleText custom text that can be inserted into the graph title section in the visualization
   * @note Use the directive PROFILE to enable profiling output and call after execution.
   * @note See TaskGraphDotGenFlags.hpp for list of bit flags
   * @note Calling this function prior to execution show the graph structure.
   */
  void writeDotToFile(std::string file, int flags = 0, std::string graphTitle = "", std::string customTitleText = "") {

    if ((flags & DOTGEN_FLAG_SHOW_CONNECTORS) == 0 && (flags & DOTGEN_FLAG_SHOW_CONNECTOR_VERBOSE) == 0) {

      if ((flags & DOTGEN_FLAG_SHOW_ALL_THREADING) != 0) {
        std::cerr
            << "DOT visualization without connectors does not support showing all threading. Adding DOTGEN_FLAG_SHOW_CONNECTORS flag."
            << std::endl;
        flags = flags | DOTGEN_FLAG_SHOW_CONNECTORS;

      }
    }

    bool graphColored = false;
#ifdef PROFILE
    if ((flags & DOTGEN_COLOR_COMP_TIME) != 0) {
      std::string name = "color-compute-time-" + file;
      std::ofstream f(name);
      f << genDotGraph(flags, DOTGEN_COLOR_COMP_TIME, graphTitle, customTitleText);
      f.flush();

      std::cout << "Writing dot file for task graph with compute time coloring to " << name << std::endl;

      graphColored = true;
    }

    if ((flags & DOTGEN_COLOR_WAIT_TIME) != 0) {
      std::string name = "color-wait-time-" + file;

      std::ofstream f(name);
      f << genDotGraph(flags, DOTGEN_COLOR_WAIT_TIME, graphTitle, customTitleText);
      f.flush();

      std::cout << "Writing dot file for task graph with wait time coloring to " << name << std::endl;

      graphColored = true;
    }

    if ((flags & DOTGEN_COLOR_MAX_Q_SZ) != 0) {
      std::string name = "color-max-q-sz-" + file;

      std::ofstream f(name);
      f << genDotGraph(flags, DOTGEN_COLOR_MAX_Q_SZ, graphTitle, customTitleText);
      f.flush();

      std::cout << "Writing dot file for task graph with max Q size coloring to " << name << std::endl;

      graphColored = true;
    }
#endif

    if (!graphColored) {
      std::ofstream f(file);
      f << genDotGraph(flags, 0, graphTitle, customTitleText);
      f.flush();

      std::cout << "Writing dot file for task graph to " << file << std::endl;
    }
  }

  /**
   * Updates the task managers addresses, pipelineIds and the number of pipelines for all tasks in the TaskGraph
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  virtual void updateTaskManagersAddressingAndPipelines() = 0;

  /**
   * Gets the address for the task graph.
   * All tasks within this graph share the same address as the graph.
   * @return the address
   */
  std::string getAddress() {
    return this->address;
  }

  /**
   * Gets the number of sub graphs within this task graph.
   *
   * This number represents the number of subgraphs spawned by all execution pipelines in the
   * graph.
   *
   * @return the number of sub graphs.
   */
  size_t getNumberOfSubGraphs() const {
    return numberOfSubGraphs;
  }

  /**
   * Gets the total time the graph was computing.
   * @return the total time the graph was computing.
   */
  unsigned long long int getGraphComputeTime() const {
    return graphComputeTime;
  }
  /**
   * Gets the total time the graph was getting created.
   * @return the total time the graph was getting created.
   */
  unsigned long long int getGraphCreationTime() const {
    return graphCreationTime;
  }

  /**
   * Generate the content only of the graph (excludes all graph definitions and attributes)
   */
  std::string genDotGraphContent(int flags) {
    std::ostringstream oss;

    for (AnyTaskManager *bTask : *taskManagers) {
      oss << bTask->getDot(flags);
    }

    return oss.str();
  }

  /**
   * Creates an exact copy of this task graph.
   * @return a copy of the task graph.
   */
  virtual AnyTaskGraphConf *copy() = 0;

  /**
   * Generates the dot graph as a string
   */
  virtual std::string genDotGraph(int flags, int colorFlag, std::string graphTitle = "", std::string customTitleText = "") = 0;

  /**
   * Creates a copy of each task from the list of AnyTaskManagers passed as a parameter.
   * Each copy is added into this graph and a mapping between the original and the copy is made.
   * @param tasks the tasks to make copies of.
   */
  void copyTasks(std::list<AnyTaskManager *> *tasks) {
    for (auto task : *tasks) {
      this->createCopy(task);
    }
  }

  /**
   * Gets the task manager copy for a given ITask.
   * @param iTask the ITask to lookup.
   * @return the task manager used to manage the ITask
   */
  AnyTaskManager *getTaskManagerCopy(AnyITask *iTask) {
    for (auto tCopy : *taskCopyMap) {
      if (tCopy.first == iTask) {
        return tCopy.second;
      }
    }

    return nullptr;
  }

  /**
   * Checks whether an ITask is in the graph or not
   * @param task the ITask to check
   * @return true if the task is in this graph, otherwise false
   * @retval TRUE if the task is in the graph
   * @retval FALSE if the task is not in the graph
   */
  bool hasTask(AnyITask *task) {
    for (auto taskSched : *taskManagers) {
      if (taskSched->getTaskFunction() == task)
        return true;
    }

    return false;
  }

 private:

  /**
   * Creates a copy of a task manager and adds the copy and a mapping between the task manager
   * copy and the original ITask that the manager is responsible for.
   * @param taskManager the task manager to create a copy for.
   */
  void createCopy(AnyTaskManager *taskManager) {
    AnyITask *origITask = taskManager->getTaskFunction();

    // If the original ITask is not in the taskCopyMap, then add a new copy and map it to the original
    if (this->taskCopyMap->find(origITask) == this->taskCopyMap->end()) {
      AnyTaskManager *taskManagerCopy = taskManager->copy(false);
      taskCopyMap->insert(ITaskPair(origITask, taskManagerCopy));
      taskManagers->push_back(taskManagerCopy);
    }
  }

  ITaskMap *taskCopyMap; //!< The ITask copy map that maps an original ITask to a task manager copy
  std::list<AnyTaskManager *> *taskManagers; //!< The list of task managers for the task graph

  size_t pipelineId; //!< The pipelineId for the task graph
  size_t numPipelines; //!< The number of pipelines from this graph

  std::string address; //!< The address for this task graph and its tasks
  TaskNameConnectorMap *taskConnectorNameMap; //!< Maps the tsak name to the task's connector

  size_t numberOfSubGraphs; //!< The number of sub-graphs that will be spawned

  IRuleMap *iRuleMap; //!< A mapping for each IRule pointer to the shared pointer for that IRule
  MemAllocMap *memAllocMap; //!< A mapping for each IMemoryAllocator to its associated shared_ptr


  std::chrono::time_point<std::chrono::high_resolution_clock> graphCreationTimestamp; //!< Timestamp when graph constructor was called
  std::chrono::time_point<std::chrono::high_resolution_clock> graphExecutingTimestamp; //!< Timestamp for how long the graph executed
  unsigned long long int graphComputeTime; //!< The total time to execute the graph
  unsigned long long int graphCreationTime; //!< The total time to create the graph

    std::condition_variable initializeCondition; //!< The condition variable to signal to check if initialization has finished.
    std::mutex initializeMutex; //!< Mutex used to signal initializational.

  };

}

#endif //HTGS_ANYTASKGRAPHCONF_HPP
