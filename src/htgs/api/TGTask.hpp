//
// Created by tjb3 on 10/30/18.
//

#ifndef HTGS_TGTASK_HPP
#define HTGS_TGTASK_HPP

#include <htgs/api/ITask.hpp>
#include <htgs/api/TaskGraphConf.hpp>
#include "TaskGraphRuntime.hpp"

namespace htgs {

  template<class T, class U>
  class TGTask : public ITask<T, U> {
    static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
    static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

   public:

    TGTask(TaskGraphConf<T, U> *taskGraphConf, bool waitForInitialization = true) : taskGraphConf(taskGraphConf), waitForInitialization(waitForInitialization) { }

    ~TGTask()
    {
      delete runtime;
      runtime = nullptr;
    }

    void initialize() override {
      HTGS_DEBUG("Initializing TGTask with graph " << taskGraphConf);

      // Get input and output connectors for the task
//
//      std::shared_ptr<Connector<T>>
//          inputConnector = std::static_pointer_cast<Connector<T>>();
//
//      std::shared_ptr<Connector<U>>
//          outputConnector = std::static_pointer_cast<Connector<U>>();


      // Increment output to account for the updated output connector
      this->getOwnerTaskManager()->getOutputConnector()->incrementInputTaskCount();

      HTGS_ASSERT(taskGraphConf->getInputConnector()->getQueueSize() == 0,
          "The TGTask " << this->getName()
          << " has " << taskGraphConf->getInputConnector()->getQueueSize() << " items in its queue, which are going to be "
          << "lost. Do not produce data into the task graph that the TGTask is wrapped.");

      // redirect input/output connectors and attach to the task graph
      taskGraphConf->setInputConnector(this->getOwnerTaskManager()->getInputConnector());
      taskGraphConf->setOutputConnector(this->getOwnerTaskManager()->getOutputConnector());


      // Launch the graph
      runtime = new TaskGraphRuntime(taskGraphConf);
      runtime->executeRuntime();

      if (waitForInitialization)
        taskGraphConf->waitForInitialization();
    }

    void executeTask(std::shared_ptr<T> data) override {
    }

    size_t getNumGraphsSpawned() override {
      return 1 + taskGraphConf->getNumberOfSubGraphs();
//        (graph->getNumberOfSubGraphs() == 0 ? 1 : graph->getNumberOfSubGraphs());
    }

    void shutdown() override {
      runtime->waitForRuntime();
    }

    std::string getName() override {
      return "TGTask";
    }

    ITask<T, U> *copy() override {
      return new TGTask<T,U>(this->taskGraphConf->copy(this->getPipelineId(), this->getNumPipelines()));
    }


    void gatherProfileData(std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles) override {
      taskGraphConf->gatherProfilingData(taskManagerProfiles);
    }

    void printProfile() override {
      taskGraphConf->printProfile();
    }

    bool canTerminate(std::shared_ptr<AnyConnector> inputConnector) override {
      return true;
    }


    std::string genDot(int flags,
                       std::string dotId,
                       std::shared_ptr<AnyConnector> input,
                       std::shared_ptr<AnyConnector> output) override {
      std::ostringstream oss;

      std::string computeTimeStr = taskGraphConf->getGraphComputeTime() == 0 ? "" : "Compute time: " + std::to_string((double)taskGraphConf->getGraphComputeTime() / 1000000.0) + " s\n";
      std::string createTimeStr = taskGraphConf->getGraphCreationTime() == 0 ? "" : "Creation time: " + std::to_string((double)taskGraphConf->getGraphCreationTime() / 1000000.0) + " s\n";

      oss << "subgraph cluster_" << dotId << " {" << std::endl;
      oss << "label=\"" << getName() << "\n" << computeTimeStr << createTimeStr << "\";" << std::endl;
      oss << "style=\"dashed\";" << std::endl;
      oss << "style =\"filled\";" << std::endl;
      oss << "fillcolor=cornsilk;" << std::endl;
      oss << "color=forestgreen;" << std::endl;

      taskGraphConf->setInputConnector(input);
      taskGraphConf->setOutputConnector(output);
      oss << taskGraphConf->genDotGraphContent(flags);
      oss << "}" << std::endl;


      return oss.str();
    }


   private:
    TaskGraphConf<T, U> *taskGraphConf;
    TaskGraphRuntime *runtime;
    bool waitForInitialization;

  };
}

#endif //HTGS_TGTASK_HPP
