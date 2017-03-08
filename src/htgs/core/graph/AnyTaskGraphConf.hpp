//
// Created by tjb3 on 2/24/17.
//

#ifndef HTGS_ANYTASKGRAPH_HPP
#define HTGS_ANYTASKGRAPH_HPP

#include <list>
#include <htgs/core/task/AnyTaskManager.hpp>
#include <string>
#include <htgs/core/task/TaskManager.hpp>
#include <htgs/api/ITask.hpp>
#include <cstddef>
#include <htgs/core/graph/edge/EdgeDescriptor.hpp>
#include <fstream>


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


class AnyTaskGraphConf {
 public:

  AnyTaskGraphConf (size_t pipelineId, size_t numPipelines, std::string baseAddress) : pipelineId(pipelineId), numPipelines(numPipelines) {
    if (baseAddress == "")
      this->address = std::to_string(pipelineId);
    else
      this->address = baseAddress + ":" + std::to_string(this->pipelineId);
    taskManagers = new std::list<AnyTaskManager *>();
    taskCopyMap = new ITaskMap();
    taskConnectorMap = new ConnectorMap();
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

    delete taskConnectorMap;
    taskConnectorMap = nullptr;
  }

  /**
  * Pure virtual function to get the vertices of the TaskGraph
  * @return the vertices of the TaskGraph
  */
  virtual std::list<AnyTaskManager *> *getTaskManagers() {
    return this->taskManagers;
  }

  virtual AnyTaskManager *getGraphConsumerTaskManager() = 0;
  virtual AnyTaskManager *getGraphProducerTaskManager() = 0;

  virtual std::shared_ptr<AnyConnector> getInputConnector() = 0;
  virtual std::shared_ptr<AnyConnector> getOutputConnector() = 0;

  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

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
  std::string genDotGraph(int flags) {
    std::ostringstream oss;

    oss << "digraph { rankdir=\"TB\"" << std::endl;
    oss << "forcelabels=true;" << std::endl;
    oss << "node[shape=record, fontsize=10, fontname=\"Verdana\"];" << std::endl;
    oss << "edge[fontsize=10, fontname=\"Verdana\"];" << std::endl;
    oss << "graph [compound=true];" << std::endl;

    for (AnyTaskManager *bTask : *taskManagers) {
      oss << bTask->getDot(flags);
    }

    if (getGraphConsumerTaskManager() != nullptr)
      oss << this->getInputConnector()->getDotId() << "[label=\"Graph Input\n" << this->getInputConnector()->getProducerCount() <<  (((DOTGEN_FLAG_SHOW_IN_OUT_TYPES & flags) != 0) ? ("\n"+this->getInputConnector()->typeName()) : "") << "\"];" << std::endl;

    if (getGraphProducerTaskManager() != nullptr)
      oss << "{ rank = sink; " << this->getOutputConnector()->getDotId() << "[label=\"Graph Output\n" << this->getOutputConnector()->getProducerCount() <<  (((DOTGEN_FLAG_SHOW_IN_OUT_TYPES & flags) != 0) ? ("\n"+this->getOutputConnector()->typeName()) : "") << "\"]; }" << std::endl;


    if ((flags & DOTGEN_FLAG_HIDE_MEM_EDGES) == 0) {
//      if (memReleaser->size() > 0) {
//        for (const auto &kv : *this->memReleaser) {
//          for (const auto &memConnector : *kv.second)
//            oss << std::string("mainThread") << " -> " << memConnector->getDotId() << ";" << std::endl;
//        }
//      }
    }

    if (oss.str().find("mainThread") != std::string::npos)
    {
      oss << "{ rank = sink; mainThread[label=\"Main Thread\", fillcolor = aquamarine4]; }\n";
    }


#ifdef PROFILE
    std::string desc = "";
    std::unordered_map<std::string, double> *timeMap;
    std::unordered_map<std::string, std::string> *colorMap;

    if ((flags & DOTGEN_FLAG_SHOW_PROFILE_COMP_TIME) != 0)
    {
      desc = "Compute Time (sec): ";
      timeMap = this->getComputeTimeAverages();

    }
    else if ((flags & DOTGEN_FLAG_SHOW_PROFILE_WAIT_TIME) != 0)
    {
      desc = "Wait Time (sec): ";
      timeMap = this->getWaitTimeAverages();
    }
    else if ((flags & DOTGEN_FLAG_SHOW_PROFILE_MAX_Q_SZ) != 0)
    {
      desc = "Max Q Size";
      timeMap = this->getMaxQSizeAverages();
    }

    if (desc != "") {
      colorMap = this->genColorMap(timeMap);
      oss << this->genProfileGraph(flags, timeMap, desc, colorMap);

      delete timeMap;
      delete colorMap;
    }
#endif

    oss << "}" << std::endl;

    return oss.str();
  }


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

  ConnectorMap *taskConnectorMap;


  size_t pipelineId; //!< The pipelineId for the task graph
  size_t numPipelines; //!< The number of pipelines from this graph

  std::string address; //!< The address for this task graph and its tasks

};

}

#endif //HTGS_ANYTASKGRAPH_HPP
