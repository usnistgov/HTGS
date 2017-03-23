//
// Created by tjb3 on 2/24/17.
//

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

typedef std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>> TaskNameConnectorMap;

typedef std::pair<std::string, std::shared_ptr<AnyConnector>>TaskNameConnectorPair;


class AnyTaskGraphConf {
 public:

  AnyTaskGraphConf (size_t pipelineId, size_t numPipelines, std::string baseAddress) :
      pipelineId(pipelineId), numPipelines(numPipelines) {
    if (baseAddress == "")
      this->address = std::to_string(pipelineId);
    else
      this->address = baseAddress + ":" + std::to_string(this->pipelineId);
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

  virtual ~AnyTaskGraphConf() {
    for (auto task : *taskManagers)
    {
      if (task != nullptr)
      {
        delete task;
        task = nullptr;
      }
    }
    delete taskManagers;

    delete taskCopyMap;
    taskCopyMap = nullptr;

    delete taskConnectorNameMap;
    taskConnectorNameMap = nullptr;
  }

  /**
  * Pure virtual function to get the vertices of the TaskGraph
  * @return the vertices of the TaskGraph
  */
  virtual std::list<AnyTaskManager *> *getTaskManagers() {
    return this->taskManagers;
  }

  virtual AnyTaskManager *getGraphConsumerTaskManager() = 0;
  virtual std::list<AnyTaskManager *> *getGraphProducerTaskManagers() = 0;

  virtual std::shared_ptr<AnyConnector> getInputConnector() = 0;
  virtual std::shared_ptr<AnyConnector> getOutputConnector() = 0;

  virtual void updateCommunicator() = 0;

  virtual TaskGraphCommunicator *getTaskGraphCommunicator() const = 0;

  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  void gatherProfilingData(std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles)
  {
      for (auto tMan : *taskManagers)
      {
        tMan->gatherProfileData(taskManagerProfiles);
      }
  }

  template <class V, class W>
  std::shared_ptr<IRule<V, W>> getIRule(IRule<V, W> *iRule)
  {
    std::shared_ptr<IRule<V, W>> iRuleShr;
    if (this->iRuleMap->find(iRule) != this->iRuleMap->end()) {
      std::shared_ptr<AnyIRule> baseRulePtr = this->iRuleMap->find(iRule)->second;
      iRuleShr = std::static_pointer_cast<IRule<V, W>>(baseRulePtr);
    }
    else{
      iRuleShr = std::shared_ptr<IRule<V, W>>(iRule);
      this->iRuleMap->insert(IRulePair(iRule, iRuleShr));
    }
    return iRuleShr;
  };

  template <class V>
  std::shared_ptr<IMemoryAllocator<V>> getMemoryAllocator(IMemoryAllocator<V> *allocator)
  {
    std::shared_ptr<IMemoryAllocator<V>> allocP;

    if (this->memAllocMap->find(allocator) == this->memAllocMap->end())
    {
      allocP = std::shared_ptr<IMemoryAllocator<V>>(allocator);
      memAllocMap->insert(MemAllocPair(allocator, allocP));
    }
    else
    {
      allocP = std::static_pointer_cast<IMemoryAllocator<V>>(this->memAllocMap->at(allocator));
    }

    return allocP;
  }

  /**
   * Initializes the task graph just prior to spawning threads
   */
  void initialize()
  {
    this->getTaskGraphCommunicator()->setNumGraphsSpawned(this->getNumberOfSubGraphs());

    this->updateTaskManagersAddressingAndPipelines();
    this->updateCommunicator();
  }

  TaskNameConnectorMap *getTaskConnectorNameMap() const {
    return taskConnectorNameMap;
  }

  template <class T, class U>
  ITask<T, U> *getCopy(ITask<T, U> *orig)
  {
    for (auto tCopy : *taskCopyMap) {
      if (tCopy.first == orig) {
        return (ITask<T, U> *)tCopy.second->getTaskFunction();
      }
    }

    return nullptr;
  }

  AnyITask *getCopy(AnyITask *orig)
  {
    for (auto tCopy : *taskCopyMap) {
      if (tCopy.first == orig) {
        return tCopy.second->getTaskFunction();
      }
    }

    return nullptr;
  }

  template <class T, class U>
  TaskManager<T, U> *getTaskManager(ITask <T, U> *task) {

    TaskManager<T, U> *taskManager = nullptr;

    for (auto tSched : *taskManagers)
    {
      if (tSched->getTaskFunction() == task)
      {
        taskManager = (TaskManager<T, U> *)tSched;
        break;
      }
    }

    if (taskManager == nullptr)
    {
      taskManager = new TaskManager<T, U>(task, task->getNumThreads(), task->isStartTask(), task->isPoll(), task->getMicroTimeoutTime(), pipelineId, numPipelines, address);
      this->taskManagers->push_back(taskManager);

      // Increment number of graphs spawned from the task
      this->numberOfSubGraphs += task->getNumGraphsSpawned();
    }

    return taskManager;

  }

  void addTaskManager(AnyTaskManager *taskManager)
  {
    for (auto tMan : *taskManagers)
    {
      if (tMan == taskManager)
        return;
    }

    this->taskManagers->push_back(taskManager);
  }


  size_t getPipelineId() { return this->pipelineId; }

  size_t getNumPipelines() { return this->numPipelines; }


  /**
   * Writes the dot representation of the task graph to disk
   * @param file the file path (will not create directories)
   */
  void writeDotToFile(std::string file) {
    writeDotToFile(file, 0);
  }

  void writeDotToFile(std::string file, int flags) {
    std::ofstream f(file);
    f << genDotGraph(flags);
    f.flush();

    std::cout << "Writing dot file for task graph to " << file << std::endl;
  }

  /**
   * @internal
   * Updates the task managers addresses, pipelineIds and the number of pipelines for all tasks in the TaskGraph
   *
   * @note This function should only be called by the HTGS API
   */
  void updateTaskManagersAddressingAndPipelines() {
    for (auto t : *this->taskManagers) {
      t->updateAddressAndPipelines(address, this->pipelineId, this->numPipelines);

      std::string taskAddressName = this->address + ":" + t->getName();
      this->taskConnectorNameMap->insert(TaskNameConnectorPair(taskAddressName, t->getInputConnector()));

//      t->setPipelineId(pipelineId);
//      t->setNumPipelines(numPipelines);
//      // TODO: May be able to get rid of this
//      t->addPipelineConnector(pipelineId);
    }
  }

  std::string getAddress()
  {
    return this->address;
  }

  size_t getNumberOfSubGraphs() const {
    return numberOfSubGraphs;
  }


  /**
   * Generate the content only of the graph (excludes all graph definitions and attributes)
   */
  std::string genDotGraphContent(int flags) {
    std::ostringstream oss;

    for (AnyTaskManager *bTask : *taskManagers) {
      oss << bTask->getDot(flags);
    }

    if ((flags & DOTGEN_FLAG_HIDE_MEM_EDGES) != 0) {

//      if (memReleaser->size() > 0) {
//        for (const auto &kv : *this->memReleaser) {
//          auto connector = kv.second->at(this->pipelineId);
//          oss << std::string("mainThread") << " -> " << connector->getDotId() << ";" << std::endl;
//        }
//
//        oss << "mainThread[label=\"Main Thread\"];\n";
//      }
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
  virtual std::string genDotGraph(int flags) = 0;


  void copyTasks(std::list<AnyTaskManager *> *tasks)
  {
    for (auto task : *tasks)
    {
      this->createCopy(task);
    }
  }

  AnyTaskManager *getTaskManagerCopy(AnyITask *iTask)
  {
    for (auto tCopy : *taskCopyMap) {
      if (tCopy.first == iTask) {
        return tCopy.second;
      }
    }

    return nullptr;
  }

  bool hasTask(AnyITask *task)
  {
    for (auto taskSched : *taskManagers)
    {
      if (taskSched->getTaskFunction() == task)
        return true;
    }

    return false;
  }

 private:


  void createCopy(AnyTaskManager *taskManager)
  {
    AnyITask *origITask = taskManager->getTaskFunction();

    // If the original ITask is not in the taskCopyMap, then add a new copy and map it to the original
    if (this->taskCopyMap->find(origITask) == this->taskCopyMap->end())
    {
      AnyTaskManager *taskManagerCopy = taskManager->copy(false);
      taskCopyMap->insert(ITaskPair(origITask, taskManagerCopy));
      taskManagers->push_back(taskManagerCopy);
    }
  }


  ITaskMap *taskCopyMap;
  std::list<AnyTaskManager *> *taskManagers;

  size_t pipelineId; //!< The pipelineId for the task graph
  size_t numPipelines; //!< The number of pipelines from this graph

  std::string address; //!< The address for this task graph and its tasks
  TaskNameConnectorMap *taskConnectorNameMap;

  size_t numberOfSubGraphs; //!< The number of sub-graphs that will be spawned

  IRuleMap *iRuleMap; //!< A mapping for each IRule pointer to the shared pointer for that IRule
  MemAllocMap *memAllocMap; //!< A mapping for each IMemoryAllocator to its associated shared_ptr
};

}

#endif //HTGS_ANYTASKGRAPHCONF_HPP
