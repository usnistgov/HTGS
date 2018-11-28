// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file TGTask.hpp
 * @author Timothy Blattner
 * @date Oct 30, 2018
 *
 * @brief Holds the TGTask class implementation.
 */

#ifndef HTGS_TGTASK_HPP
#define HTGS_TGTASK_HPP

#include <htgs/api/ITask.hpp>
#include <htgs/api/TaskGraphConf.hpp>
#include "TaskGraphRuntime.hpp"

namespace htgs {

  class TaskGraphRuntime;

  /**
   * @class TGTask TGTask.hpp <htgs/api/TGTask.hpp>
   * @brief TGTask is the task graph task, which is used to bundle a graph as a task, which can then be connected to other graphs.
   * @details The primary function of this task is to help provide more generalization of custom graphs, which can then
   * be incorporated easily into other projects. An added bonus is the ability to visualize graphs and sub-graphs from
   * within a single graph.
   *
   * Common usage:
   * @code
   * htgs::TaskGraphConf<htgs::VoidData, htgs::VoidData> *taskGraph = new htgs::TaskGraphConf<htgs::VoidData, htgs::VoidData>();
   * // Add edges and other components within taskGraph
   * ...
   *
   * bool waitForInit = true;
   * htgs::TGTask<htgs::VoidData, htgs::VoidData> *tgTask = taskGraph->createTaskGraphTask("CustomName", waitForInit);
   *
   * // Add tgTask into another graph as needed.
   *
   * @endcode
   *
   *
   * @tparam T the input data type for the TGTask, T must derive from IData and match the input type of the htgs::TaskGraphConf.
   * @tparam U the output data type for the TGTask, U must derive from IData and match the output type of the htgs::TaskGraphConf.
   */
  template<class T, class U>
  class TGTask : public ITask<T, U> {
    static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
    static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

   public:

    /**
     * Constructs a TGTask that will redirect the input and output of the TGTask to its underlying task graph.
     * @param taskGraphConf the task graph to redirect input and output too.
     * @param name the name of the TGTask, default = "TGTask"
     * @param waitForInitialization whether to wait for initialization or not, default = true
     */
    TGTask(TaskGraphConf<T, U> *taskGraphConf, std::string name = "TGTask", bool waitForInitialization = true) :
        taskGraphConf(taskGraphConf), runtime(nullptr), waitForInitialization(waitForInitialization), name(name) { }

    /**
     *  Deconstructs the TGTask
     */
    ~TGTask()
    {
      if (runtime) {
        delete runtime;
        runtime = nullptr;
      } else{
        if(taskGraphConf)
        {
          delete taskGraphConf;
          taskGraphConf = nullptr;
        }
      }
    }

    /**
     * Initializes the TGTask, which will redirect the input/output connectors that are attached to its underlying htgs::TaskGraphConf.
     * The task graph will have all of its spawned. If waitForInitialization is set to true, then this function
     * will only return once all threads have spawned for its task graph.
     *
     */
    void initialize() override {
      HTGS_DEBUG("Initializing TGTask with graph " << taskGraphConf);

      if (this->getOwnerTaskManager()->getOutputConnector() != nullptr) {

        // Increment output to account for the updated output connector
        this->getOwnerTaskManager()->getOutputConnector()->incrementInputTaskCount();
        taskGraphConf->setOutputConnector(this->getOwnerTaskManager()->getOutputConnector());

      }

      if (this->getOwnerTaskManager()->getInputConnector() != nullptr) {
        HTGS_ASSERT(taskGraphConf->getInputConnector()->getQueueSize() == 0,
                    "The TGTask " << this->getName()
                                  << " has " << taskGraphConf->getInputConnector()->getQueueSize()
                                  << " items in its queue, which are going to be "
                                  << "lost. Do not produce data into the task graph that the TGTask is wrapped.");

        // redirect input connectors and attach to the task graph
        taskGraphConf->setInputConnector(this->getOwnerTaskManager()->getInputConnector());
      }

      // Launch the graph
      runtime = new TaskGraphRuntime(taskGraphConf);
      runtime->executeRuntime();

      if (waitForInitialization)
        taskGraphConf->waitForInitialization();
    }

    /**
     * Not called, as the thread will terminate after initialization is done.
     * @param data
     */
    void executeTask(std::shared_ptr<T> data) override {
    }

    /**
     * Gets the number of graphs spawned by the TGTask
     * @return the number of graphs spawned
     */
    size_t getNumGraphsSpawned() override {
      return 1 + taskGraphConf->getNumberOfSubGraphs();
    }

    /**
     * Shutsdown the TGTask by waiting for the underlying htgs::TaskGraphConfg to finish running
     */
    void shutdown() override {
      runtime->waitForRuntime();
    }

    /**
     * Gets the name of the TGTask
     * @return the name
     */
    std::string getName() override {
      return name;
    }

    /**
     * Creates a copy of the TGTask. This will also create a new copy of the htgs::TaskGraphConf
     * @return the copy of the TGTask
     */
    ITask<T, U> *copy() override {
      return new TGTask<T,U>(this->taskGraphConf->copy(this->getPipelineId(), this->getNumPipelines()), name, waitForInitialization);
    }

    std::string genCustomDot(ProfileUtils *profileUtils, int colorFlag) override {
      if (profileUtils == nullptr)
        return "";

      std::ostringstream oss;

      double time = 0.0;
      if (colorFlag == DOTGEN_COLOR_COMP_TIME)
        time = (double) taskGraphConf->getGraphComputeTime();
      else if (colorFlag == DOTGEN_COLOR_WAIT_TIME)
        time = (double) this->getOwnerTaskManager()->getWaitTime();
      else if (colorFlag == DOTGEN_COLOR_MAX_Q_SZ)
        time = (double) this->getOwnerTaskManager()->getMaxQueueSize();

      oss << "subgraph cluster_" << this->getDotId() << " {" << std::endl;

      std::string color = profileUtils->getColorForTime(time);

      oss << (colorFlag != 0 ? "penwidth=5\ncolor=\"" + color + "\"" : "color=forestgreen");
      oss << std::endl;

      oss << taskGraphConf->genCustomDotForTasks(profileUtils, colorFlag);

      oss << "}" << std::endl;

      return oss.str();
    }


    /**
     * Gathers profiling data for the underlying htgs::TaskGraphConf
     * @param taskManagerProfiles the profiling data
     */
    void gatherProfileData(std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles) override {
      taskGraphConf->gatherProfilingData(taskManagerProfiles);
    }

    /**
     * Prints profile information to console
     */
    void printProfile() override {
      taskGraphConf->printProfile();
    }

    /**
     * The TGTask is set to terminate immediately, as the primary functionality is done at initialization.
     * @param inputConnector the input connector
     * @return true
     */
    bool canTerminate(std::shared_ptr<AnyConnector> inputConnector) override {
      return true;
    }

    virtual std::string genDotProducerEdgeToTask(std::map<std::shared_ptr<AnyConnector>, AnyITask *> &inputConnectorDotMap, int dotFlags) override
    {
      return "";
    }

    virtual std::string genDotProducerEdgeFromConnector(std::shared_ptr<AnyConnector> connector, int flags)
    {
      return "";
    }

    virtual std::string genDotConsumerEdgeFromConnector(std::shared_ptr<AnyConnector> connector, int flags) override
    {
      return "";
    }


    std::string getConsumerDotIds() override {
      // Who is consuming the input data...
      if (this->getOwnerTaskManager()->getInputConnector() != nullptr)
      {
        return taskGraphConf->getGraphConsumerTaskManager()->getTaskFunction()->getConsumerDotIds();
      }

      return "";
    }

    std::string getProducerDotIds() override {
      std::ostringstream oss;

      if (this->getOwnerTaskManager()->getOutputConnector() != nullptr)
      {
        oss << "{";
        for (auto producer : *taskGraphConf->getGraphProducerTaskManagers())
        {
          oss << producer->getTaskFunction()->getProducerDotIds() << ";";
        }
        oss << "}";
      }

      return oss.str();
    }

//    std::string getDotCustomProfile() override {
//      std::ostringstream oss;
//      oss << "subgraph cluster_" << this->getDotId() << " {" << std::endl;
//
//      oss << useColorMap ? ",penwidth=5,color=\"" + colorMap->at(dotId) + "\"" : ", color=" + tFun->getDotShapeColor());
//      ret += ",width=.2,height=.2];\n"
//
//
//      oss << "}" << std::endl;
//
//    }



//
//    std::string genDotConsumerEdgeFromConnector(std::shared_ptr<AnyConnector> connector, int flags) override
//    {
//      return "";
//    }
//
//
//    std::string genDotProducerEdgeToTask(std::map<std::shared_ptr<AnyConnector>, AnyITask *> &inputConnectorDotMap, int dotFlags) override
//    {
//      std::ostringstream oss;
////
////      if (this->getOwnerTaskManager()->getInputConnector() != nullptr)
////      {
////        taskGraphConf->setInputConnector(this->getOwnerTaskManager()->getInputConnector());
////      }
////
////      if (this->getOwnerTaskManager()->getOutputConnector() != nullptr)
////      {
////        taskGraphConf->setOutputConnector(this->getOwnerTaskManager()->getOutputConnector());
////      }
//
//      auto graphConsumerManager = taskGraphConf->getGraphConsumerTaskManager();
//      if (this->getOwnerTaskManager()->getInputConnector() != nullptr)
//      {
//        auto connectorPair = inputConnectorDotMap.find(this->getOwnerTaskManager()->getInputConnector());
//
//        if (connectorPair != inputConnectorDotMap.end())
//        {
//          oss << graphConsumerManager->getTaskFunction()->getDotId() << " -> " << connectorPair->second << ";\n";
//        }
//      }
////
////      if (this->getOwnerTaskManager()->getOutputConnector() != nullptr)
////      {
////        auto connectorPair = inputConnectorDotMap
////      }
//
//
////      auto connectorDot = inputConnectorDotMap.find()
//
//
//      return oss.str();
//    }

    /**
     * Generates the sub graph dot file representation of the TGTask
     * @param flags dot flags
     * @param dotId the id for the TGTask
     * @param input the input connector for the TGTask
     * @param output the output connector for the TGTask
     * @return the string representation of the TGTask sub graph.
     */
    std::string genDot(int flags,
                       std::string dotId,
                       std::shared_ptr<AnyConnector> input,
                       std::shared_ptr<AnyConnector> output) override {
      std::ostringstream oss;

      std::string computeTimeStr = taskGraphConf->getGraphComputeTime() == 0 ? "" : "Compute time: " + std::to_string((double)taskGraphConf->getGraphComputeTime() / 1000000.0) + " s\\n";
      std::string createTimeStr = taskGraphConf->getGraphCreationTime() == 0 ? "" : "Creation time: " + std::to_string((double)taskGraphConf->getGraphCreationTime() / 1000000.0) + " s\\n";

      oss << "subgraph cluster_" << dotId << " {" << std::endl;
      oss << "label=\"" << getName() << "\\n" << computeTimeStr << createTimeStr << "\";" << std::endl;
      oss << "style=\"dashed\";" << std::endl;
      oss << "style =\"filled\";" << std::endl;
      oss << "fillcolor=cornsilk;" << std::endl;
      oss << "color=forestgreen;" << std::endl;

      if (input != nullptr)
        taskGraphConf->setInputConnector(input);

      if (output != nullptr)
        taskGraphConf->setOutputConnector(output);

      oss << taskGraphConf->genDotGraphContent(flags);
      oss << "}" << std::endl;

      return cleanupVisualization(taskGraphConf, oss.str());
    }

    /**
     * Moves the output connector outside of the execution pipeline graphs to cleanup how the graph looks during graph visualization.
     * @param graph the graph
     * @param str the dot file string to be cleaned up
     * @return the improved dot file text
     */
    std::string cleanupVisualization(TaskGraphConf<T, U> *graph, std::string str)
    {
      std::istringstream iss(str);


      std::ostringstream ossFinal;

      auto outputConnectorName = graph->getOutputConnector()->getDotId();
      auto inputConnectorName = graph->getInputConnector()->getDotId();


      std::string line;
      std::vector<std::string> endSavedLines;
      std::vector<std::string> begSavedLines;
      while(getline(iss, line))
      {
        if (line.find(inputConnectorName) != std::string::npos)
        {
          begSavedLines.push_back(line);
        }
        else if (line.find(outputConnectorName) != std::string::npos)
        {
          endSavedLines.push_back(line);
        }
        else
        {
          ossFinal << line << std::endl;
        }
      }

      for(std::string line2 : endSavedLines)
      {
        ossFinal << line2 << std::endl;
      }

      std::ostringstream ret;
      for (std::string line2 : begSavedLines)
      {
        ret << line2 << std::endl;
      }

      ret << ossFinal.str();


      return ret.str();
    }



   private:
    TaskGraphConf<T, U> *taskGraphConf; //!< The task graph that the TGTask is managing
    TaskGraphRuntime *runtime; //!< The runtime that is associate with the task graph
    bool waitForInitialization; //!< Whether to wait for initialization of the task graph or not
    std::string name; //!< The name of the TGTask

  };
}

#endif //HTGS_TGTASK_HPP