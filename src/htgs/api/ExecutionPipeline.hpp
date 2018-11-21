
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file ExecutionPipeline.hpp
 * @author Timothy Blattner
 * @date Nov 30, 2015
 *
 * @brief ExecutionPipeline encapsulates a task graph and duplicates it, such that each
 * duplicate task graph executes concurrently.
 *
 */


#ifndef HTGS_EXECUTIONPIPELINE_H
#define HTGS_EXECUTIONPIPELINE_H

#include <cstring>

#include <htgs/core/rules/ExecutionPipelineBroadcastRule.hpp>
#include <htgs/api/ITask.hpp>
#include <htgs/api/Bookkeeper.hpp>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>
#include <htgs/api/IData.hpp>

namespace htgs {

  class TaskGraphRuntime;

  template<class V, class W>
  class IRule;

  template<class V, class W>
  class TaskGraphConf;



/**
 * @class ExecutionPipeline ExecutionPipeline.hpp <htgs/api/ExecutionPipeline.hpp>
 * @brief The ExecutionPipeline class is used to duplicate task graphs, such that each duplicate executes concurrently.
 * @details Each task within the task graph is an exact duplicate of the original's. The only
 * difference is the pipelineId passed to each task. The pipelineId indicates which task graph
 * the task belongs too. This value can be used as a rank to distribute computation and determine if additional
 * functionality is needed to process data (such as copying data from one GPU to another).
 *
 * An ExecutionPipeline can be used to execute across multiple GPUs on a single system. Any ICudaTask will
 * automatically be bound to a separate GPU based on the CUContext array passed to the ICudaTask. The size of the CUContext
 * array should match the number of execution pipelines specified. If data exists between two GPUs, then additional
 * copying may be required, see ICudaTask for details and example usage for copying data between GPUs.
 *
 * The execution pipeline can be used to distribute data among each task graph by adding a rule that
 * uses the pipelineId parameter in an IRule. See addInputRule(IRule <T, T> *rule)
 *
 * If you wish to share a rule with multiple execution pipelines or bookkeepers, you must wrap the rule into a
 * std::shared_ptr prior to calling the addInputRule function.
 *
 *
 * Example usage:
 * @code
 * htgs::TaskGraphConf<MatrixData, MatrixData> *subGraph = new htgs::TaskGraphConf<MatrixData, MatrixData>();
 * ... build subGraph
 *
 * int numPipelines = 3;
 * htgs::ExecutionPipeline<MatrixData, MatrixData> *execPipeline = new htgs::ExecutionPipeline<MatrixData, MatrixData>(numPipelines, subGraph);
 * execPipeline->addInputRule(new DecompRule());
 * PostProcessTask *taskOutsideExecPipeline = new PostProcessTask();
 *
 * htgs::TaskGraph<MatrixData, VoidData> *mainGraph = new htgs::TaskGraph<MatrixData, VoidData>();
 *
 * // Declares that the execPipeline connects to the input of the mainGraph
 * mainGraph->setGraphConsumerTask(execPipeline);
 * mainGraph->addEdge(execPipeline, taskOutsideExecPipeline);
 *
 * htgs::TaskGraphRuntime *runTime = new htgs::TaskGraphRuntime(mainGraph);
 *
 * while(hasDataToAdd)
 * {
 *   // Add data to graph
 *   mainGraph->produceData(data);
 * }
 *
 * mainGraph->finishProducingData();
 *
 * // Executes the main graph (spawns sub graphs for execPipeline)
 * runTime->executeAndWaitForRunTime();
 *
 * @endcode
 *
 * @tparam T the input type for the ExecutionPipeline ITask, must derive from IData.
 * @tparam U the output type for the ExecutionPipeline ITask, must derive from IData.
 */
template<class T, class U>
class ExecutionPipeline : public ITask<T, U> {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:
  /**
   * Creates an execution pipeline, which encapsulates a graph and duplicates it numPipelines times
   * @param numPipelines the number of times to duplicate the graph
   * @param graph the graph that the execution pipeline manages
   * @param name the name of the execution pipeline
   * @param waitForInit Indicates that the execution pipeline will wait for its underlying graphs to initialize prior to marking that it is initialized.
   */
  ExecutionPipeline(size_t numPipelines, TaskGraphConf<T, U> *graph, std::string name = "Execution Pipeline", bool waitForInit = true) {
    this->numPipelinesExec = numPipelines;
    this->graph = graph;
    this->inputBk = new Bookkeeper<T>();
    this->runtimes = new std::vector<TaskGraphRuntime *>();
    this->inputRules = std::shared_ptr<IRuleList<T, T>>(new IRuleList<T, T>());
    this->graphs = new std::vector<TaskGraphConf<T, U> *>();
    this->waitForInit = waitForInit;
    this->name = name;
  }

  /**
   * Creates an execution pipeline, which encapsulates a graph and duplicates it numPipelines times
   * @param numPipelines the number of times to duplicate the graph
   * @param graph the graph that the execution pipeline manages
   * @param rules the list of decomposition rules that will be used for this pipeline
   * @param name the name of the execution pipeline
   * @param waitForInit Indicates that the execution pipeline will wait for its underlying graphs to initialize prior to marking that it is initialized.
   */
  ExecutionPipeline(size_t numPipelines, TaskGraphConf<T, U> *graph, std::shared_ptr<IRuleList<T, T>> rules, std::string name = "Execution Pipeline", bool waitForInit = true) {
    this->numPipelinesExec = numPipelines;
    this->graph = graph;
    this->inputBk = new Bookkeeper<T>();
    this->runtimes = new std::vector<TaskGraphRuntime *>();
    this->inputRules = rules;
    this->graphs = new std::vector<TaskGraphConf<T, U> *>();
    this->waitForInit = waitForInit;
    this->name = name;
  }

  /**
   * Destructor that frees all memory allocated by the execution pipeline
   */
  ~ExecutionPipeline() {
    for (TaskGraphRuntime *runtime : *runtimes) {
      HTGS_DEBUG_VERBOSE("Execution pipeline: " << this << " Freeing memory for runtime: " << runtime);
      delete runtime;
      runtime = nullptr;
    }


    HTGS_DEBUG_VERBOSE("Execution pipeline: " << this << " Freeing memory for graph: " << graph);
    delete graph;
    graph = nullptr;

    delete runtimes;
    runtimes = nullptr;

    delete inputBk;
    inputBk = nullptr;

    delete graphs;
    graphs = nullptr;

//    delete inputRules;
//    inputRules = nullptr;
  }

  size_t getNumGraphsSpawned() override {
    return this->numPipelinesExec * graph->getNumberOfSubGraphs() + this->numPipelinesExec;
//        (graph->getNumberOfSubGraphs() == 0 ? 1 : graph->getNumberOfSubGraphs());
  }

  /**
   * Adds an input rule, which can be used for domain decomposition.
   * This rule should use the pipelineId parameter in the IRule::applyRule function to aid in
   * distributing data to the appropriate execution pipeline TaskGraph
   * @param rule the rule to handle distributing data between pipelines
   */
  void addInputRule(IRule<T, T> *rule) {
    this->inputRules->push_back(std::shared_ptr<IRule<T, T>>(rule));
  }

  /**
   * Adds an input rule, which can be used for domain decomposition.
   * This rule should use the pipelineId parameter in the IRule::applyRule function to aid in
   * distributing data to the appropriate execution pipeline TaskGraph
   * This variant should be used if the rule is intended to be shared with multiple bookkeepers or other execution pipelines.
   * @param rule the rule as a shared_ptr to handle distributing data between pipelines
   */
  void addInputRule(std::shared_ptr<IRule<T, T>> rule) {
    this->inputRules->push_back(rule);
  }

  /**
   * Initializes the execution pipeline and duplicates the task graph based on the number of pipelines. If wait for initialization
   * is set to true, then this function will only return once all threads from all sub-graphs have been spawned and
   * initialized.
   *
   * @note This function should only be called by the HTGS API
   */
  void initialize() {
    HTGS_DEBUG("Initializing Execution pipeline with " << this->numPipelinesExec << " pipelines");

    // Add a default broadcast rule if the pipeline has no rules
    if (this->inputRules->size() == 0) {
      std::cerr
          << "ERROR: Your execution pipeline does not have any decomposition rules. You must add at least one: addInputRule(IRule)"
          << std::endl;
      exit(-1);
//      this->addInputRule(new ExecutionPipelineBroadcastRule<T>());
    }

    // Create output connector using the execution pipelines output
    std::shared_ptr<Connector<U>>
        outputConnector = std::static_pointer_cast<Connector<U>>(this->getOwnerTaskManager()->getOutputConnector());

    for (size_t i = 0; i < numPipelinesExec; i++) {
      HTGS_DEBUG("Adding pipeline " << i);
      TaskGraphConf<T, U>
          *graphCopy = this->graph->copy(i, this->numPipelinesExec, nullptr, outputConnector, this->getAddress());
      // TODO: Remove or Add #ifdef this->getTaskGraphCommunicator());


#ifdef WS_PROFILE
      // TODO: Update parent for graphCopy to point to the execution pipeline
#endif
      HTGS_DEBUG("Setting up input and output of pipeline " << i);

      for (std::shared_ptr<IRule<T, T>> rule : *this->inputRules) {

        RuleManager<T, T> *ruleManager = new RuleManager<T, T>(rule /* TODO: Remove or Add #ifdef , this->getTaskGraphCommunicator()*/);
        ruleManager->setOutputConnector(graphCopy->getInputConnector());
        ruleManager->initialize(i, this->numPipelinesExec, this->getAddress());

        this->inputBk->addRuleManager(ruleManager);
      }

      graphs->push_back(graphCopy);

    }

    for (TaskGraphConf<T, U> *g : *graphs) {
      TaskGraphRuntime *runtime = new TaskGraphRuntime(g);
      runtime->executeRuntime();
      this->runtimes->push_back(runtime);
    }

    if (waitForInit) {
      for (TaskGraphConf <T, U> *g : *graphs) {
        g->waitForInitialization();
      }
    }
  }

  /**
   * Shuts down the execution pipeline.
   * @note This function should only be called by the HTGS API
   */
  void shutdown() {
    HTGS_DEBUG("Shutting down " << this->getName());
    this->inputBk->shutdown();

    // Spawn thread for each runtime to properly wait without blocking.
    std::vector<std::thread *> shutdownThreads;

    for (size_t i = 0; i < runtimes->size(); i++)
    {
      std::thread *t = new std::thread(&ExecutionPipeline<T, U>::shutdownParallel, this, i);
      shutdownThreads.push_back(t);
    }

    for (std::thread *t : shutdownThreads)
    {
      if (t->joinable())
        t->join();

      delete t;
    }

//
//    for (std::vector<TaskGraphRuntime *>::reverse_iterator r = runtimes->rbegin();
//         r != runtimes->rend(); ++r ) {
//      (*r)->waitForRuntime();
//    }

//    for (TaskGraphRuntime *r : *this->runtimes) {
//      r->waitForRuntime();
//    }
  }

  /**
   * Waits for all runtimes in parallel to update overall runtime of each graph.
   * @param id the thread ID
   */
  void shutdownParallel(int id) {
    (*runtimes)[id]->waitForRuntime();
  }

  /**
   * Executes the execution pipeline task on data and forwards that data to the input rules.
   * The input rules should parse the data and determine the correct pipelineId to forward the data to.
   * @param data the data to be forwarded to the proper execution pipeline
   * @note This function should only be called by the HTGS API
   */
  void executeTask(std::shared_ptr<T> data) {
    if (data != nullptr) {
      this->inputBk->executeTask(data);
    }
  }

  /**
   * Gets the name for the execution pipeline
   * @return the name of the execution pipeline
   */
  std::string getName() {
    return name;
  }

  /**
   * Makes a copy of the execution pipeline.
   * Copies the graph and reuses the same input decomposition rules
   * @return the copied execution pipeline task
   * @note This function should only be called by the HTGS API
   */
  ITask<T, U> *copy() {
    return new ExecutionPipeline<T, U>(this->numPipelinesExec,
                                       this->graph->copy(this->getPipelineId(), this->getNumPipelines()),
                                       this->inputRules, this->name, this->waitForInit);
  }

  /**
   * Provides debugging output for the execution pipeline.
   * @note \#define DEBUG_FLAG to enable debugging
   */
  void debug() {
    HTGS_DEBUG(this->getName() << " " << numPipelinesExec << " pipelines; details:");
    inputBk->debug();
  }

  virtual void gatherProfileData(std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles) override {
    // Gather profile data for each graph
    if (graphs->size() > 0) {
      for (auto g : *graphs) {
        g->gatherProfilingData(taskManagerProfiles);
      }
    } else {
      graph->gatherProfilingData(taskManagerProfiles);
    }

  }

  void printProfile() override {
    for (auto g : *graphs) {
      g->printProfile();
    }
  }

//#ifdef PROFILE
//  std::string getDotProfile(int flags,
//                            std::unordered_map<std::string, double> *mmap, double val,
//                            std::string desc, std::unordered_map<std::string, std::string> *colorMap)
//  {
//    std::ostringstream oss;
//
//    if (graphs->size() > 0)
//    {
//      for (auto g : *graphs)
//      {
//        oss << g->genProfileGraph(flags, mmap, desc, colorMap);
//      }
//    }
//    else
//    {
//      oss << graph->genProfileGraph(flags, mmap, desc, colorMap);
//    }
//
//    return oss.str();
//  }
//
//#endif

  /**
   * @copydoc ITask::genDot
   * @note This function will generate the dot notation for all sub-graphs within the execution pipeline.
   * @note If the dot notation is generated prior to execution, then a virtual pipeline is created. Generating the dot notation
   * after execution will show the actual sub-graphs.
   */
  std::string genDot(int flags,
                     std::string dotId,
                     std::shared_ptr<AnyConnector> input,
                     std::shared_ptr<AnyConnector> output) override {
    std::ostringstream oss;

    oss << input->genDot(flags);

    // Get inputRule edge name
    std::string inputRuleNames;
    int count = 0;
    for (std::shared_ptr<IRule<T, T>> rule : *inputRules) {
      if (count == 0)
        inputRuleNames = inputRuleNames + rule->getName();
      else
        inputRuleNames = inputRuleNames + ", " + rule->getName();
      count++;
    }

    // Draw decomposition rule edge to each graph
    if (graphs->size() > 0) {
      for (auto g : *graphs) {
        oss << input->getDotId() << " -> " << g->getInputConnector()->getDotId() << "[label=\"" << inputRuleNames
            << "\"];" << std::endl;
      }
    } else {
      oss << input->getDotId() << " -> " << graph->getInputConnector()->getDotId() << "[label=\"" << inputRuleNames
          << "\"];" << std::endl;
    }

    // Setup subgraph to draw execPipeline graphs
    if (this->graphs->size() > 0) {
      int pipeline = 0;
      for (auto g : *graphs) {
        std::string computeTimeStr = g->getGraphComputeTime() == 0 ? "" : "Compute time: " + std::to_string((double)g->getGraphComputeTime() / 1000000.0) + " s\n";
        std::string createTimeStr = g->getGraphCreationTime() == 0 ? "" : "Creation time: " + std::to_string((double)g->getGraphCreationTime() / 1000000.0) + " s\n";


        oss << "subgraph cluster_" << dotId << std::to_string(pipeline) << " {" << std::endl;
        oss << "label=\"" << getName() << std::to_string(pipeline) << "\n" << computeTimeStr << createTimeStr << "\";" << std::endl;
        oss << "style=\"dashed\";" << std::endl;
        oss << "style =\"filled\";" << std::endl;
        oss << "fillcolor=lightgrey;" << std::endl;
        oss << "color=orange;" << std::endl;
        oss << g->genDotGraphContent(flags);
        oss << "}" << std::endl;
        pipeline++;

        oss.str(cleanupVisualization(g, oss.str()));

      }
    } else {
      oss << "subgraph cluster_" << dotId << " {" << std::endl;
      oss << "label=\"" << getName() << " x" << this->numPipelinesExec << "\";" << std::endl;
      oss << "style=\"dashed\";" << std::endl;
      oss << "style =\"filled\";" << std::endl;
      oss << "fillcolor=lightgrey;" << std::endl;
      oss << "color=orange;" << std::endl;

      graph->setOutputConnector(output);
      oss << graph->genDotGraphContent(flags);
      oss << "}" << std::endl;

       oss.str(cleanupVisualization(graph, oss.str()));
    }

    return oss.str();

  }

 private:

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

    std::string line;
    std::vector<std::string> savedLines;
    while(getline(iss, line))
    {
      if (line.find(outputConnectorName) == std::string::npos)
      {
        ossFinal << line << std::endl;
      }
      else
      {
        savedLines.push_back(line);
      }
    }

    for(std::string line2 : savedLines)
    {
      ossFinal << line2 << std::endl;
    }

    return ossFinal.str();
  }

  size_t numPipelinesExec; //!< The number of pipelines that will spawn from the ExecutionPipeline
  Bookkeeper<T> *inputBk; //!< The input Bookkeeper for the ExecutionPipeline
  TaskGraphConf<T, U> *graph; //!< The graph that the ExecutionPipeline manages, duplicates, and executes
  std::shared_ptr<IRuleList<T, T>> inputRules; //!< The rules associated with the input Bookkeeper for decomposition
  std::vector<TaskGraphRuntime *>
      *runtimes; //!< The list of Runtimes that will execute the TaskGraphs (one for each duplicate TaskGraph)
  std::vector<TaskGraphConf<T, U> *> *graphs; //!< The list of duplicate TaskGraphs
  bool waitForInit; //!< Flag whether to wait for initialization of sub-graphs to complete or not
  std::string name; //!< The name given to the execution pipeline task
};
}

#endif //HTGS_EXECUTIONPIPELINE_H
