
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file TaskGraphRuntime.hpp
 * @author Timothy Blattner
 * @date Nov 25, 2015
 *
 * @brief Spawns threads and binds them to the appropriate ITask within a TaskGraph
 * @details
 */
#ifndef HTGS_TASKGRAPHRUNTIME_HPP
#define HTGS_TASKGRAPHRUNTIME_HPP

#include <thread>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/core/task/AnyTaskManager.hpp>

namespace htgs {
/**
 * @class TaskGraphRuntime TaskGraphRuntime.hpp <htgs/api/TaskGraphRuntime.hpp>
 * @brief Spawns threads and binds them to the appropriate ITask within a TaskGraph.
 * @details
 *
 * Each thread is bound to a separate ITask instance. If an ITask has more than one thread associated
 * with it, then the Runtime will create a deep copy of the ITask, which is bound to the thread.
 * This means that each thread has a different ITask instance.
 *
 * This process is done for every ITask in the TaskGraph that the Runtime is responsible for.
 *
 * If an ITask is an ExecutionPipeline, then the thread responsible for the ExecutionPipeline will create
 * additional TaskGraphRuntimes, one for each TaskGraph within the ExecutionPipeline.
 *
 * A Runtime can be executed asynchronously with executeRuntime(), allowing for interaction with the main TaskGraph to
 * submit/receive data to/from the TaskGraph.
 *
 * To wait for the Runtime to finish processing all of the data for a TaskGraph, use waitForRuntime(). Be sure to indicate
 * that the input data stream for the graph is closing prior to calling waitForRuntime() (see below).
 *
 * To execute and wait for the Runtime, use executeAndWaitForRuntime(). If data is being produced for the task graph,
 * then the TaskGraph::finishedProducingData function must be called prior to waiting for the runtime
 * in order for the task graph to know that the input to the graph has finished and the tasks processing that input can be notified.
 *
 * Example Usage:
 * @code
 * htgs::TaskGraphConf<Data1, Data2> *taskGraph = new htgs::TaskGraphConf<Data1, Data2>();
 * ...
 *
 * // If adding data to the TaskGraph, must use TaskGraph::setGraphConsumerTask
 * taskGraph->setGraphConsumerTask(someTask);
 *
 * // To receive data from the TaskGraph use  TaskGraph::addGraphOutputProducer
 * taskGraph->addGraphProducerTask(someOutputTask);
 *
 * htgs::TaskGraphRuntime *runtime = new htgs::TaskGraphRuntime(taskGraph);
 *
 * // Launch the runtime, will return after all threads have been configured for the taskGraph
 * runTime->executeRuntime();
 *
 * // Add data to the TaskGraph
 * for(int elem = 0 ; elem < numElems; elem++)
 *  taskGraph->produceData(new Data1(elem));
 *
 * // Indicate finished producing data
 * taskGraph->finishedProducingData();
 *
 * // Process the output until there is no more output to process
 * while(!taskGraph->isInputTerminated())
 * {
 *   std::shared_ptr<Data2> data = taskGraph->consumeData();
 *
 *   if (data != nullptr) {
 *     // Do post processing
 *   }
 * }
 *
 * // Wait for the Runtime to finish
 * runTime->waitForRuntime();
 * @endcode
 *
 */
class TaskGraphRuntime {

 public:

  /**
   * Constructs a Runtime for a TaskGraph
   * @param graph the graph the Runtime is representing
   */
  TaskGraphRuntime(AnyTaskGraphConf *graph) {
    this->graph = graph;
    this->executed = false;
  }

  /**
   * Destructor
   */
  ~TaskGraphRuntime() {
    for (TaskManagerThread *t : runtimeThreads) {
      if (t) {
        delete t;
        t = nullptr;
      }
    }

    for (std::thread *t : threads) {
      if (t) {
        delete t;
        t = nullptr;
      }
    }

    if (graph) {
      delete graph;
      graph = nullptr;
    }
  }

  /**
   * Waits for the Runtime to finish executing.
   * Should call execute first, otherwise this function will return immediately.
   */
  void waitForRuntime() {
    for (std::thread *t : threads) {
      if (t->joinable())
        t->join();
    }

    this->graph->shutdown();
  }

  /**
   * Executes the Runtime and then waits for it to finish processing.
   */
  void executeAndWaitForRuntime() {
    executeRuntime();
    waitForRuntime();
  }

  /**
   * Terminates the Runtime.
   * This function will only mark the thread to be terminated, but will only end once the thread
   * has finished processing its last data. Will not terminate threads that are in a WAIT state.
   */
  void terminateAll() {
    for (TaskManagerThread *t : runtimeThreads) {
      t->terminate();
    }
  }

  /**
   * Executes the Runtime
   */
  void executeRuntime() {
    if (executed)
      return;

    // Initialize graph and setup task graph taskGraphCommunicator
    this->graph->initialize();

    std::list<AnyTaskManager *> *vertices = this->graph->getTaskManagers();
    std::list<AnyTaskManager *> newVertices;
    HTGS_DEBUG_VERBOSE("Launching runtime for " << vertices->size() << " vertices");


    for (AnyTaskManager *task : *vertices) {

      size_t numThreads = task->getNumThreads();

      HTGS_DEBUG_VERBOSE("Spawning " << numThreads << " threads for task " << task->getName());

      if (numThreads > 0) {
        std::list<AnyTaskManager *> taskList;
        std::shared_ptr<std::atomic_size_t>
            atomicNumThreads = std::shared_ptr<std::atomic_size_t>(new std::atomic_size_t(numThreads));
        taskList.push_back(task);

        for (size_t i = 1; i < numThreads; i++) {
          AnyTaskManager *taskCopy = task->copy(true);

#ifdef WS_PROFILE
          // Generate . . . and send data . . .
          std::shared_ptr<ProfileData> producerData(new CreateNodeProfile(taskCopy->getTaskFunction(), graph, taskCopy->getName()));
          graph->sendProfileData(producerData);

          std::shared_ptr<ProfileData> connectorConsumerData(new CreateEdgeProfile(taskCopy->getInputConnector().get(), taskCopy->getTaskFunction(), "", nullptr));
          std::shared_ptr<ProfileData> producerConnectorData(new CreateEdgeProfile(taskCopy->getTaskFunction(), taskCopy->getOutputConnector().get(), "", nullptr));

          graph->sendProfileData(connectorConsumerData);
          graph->sendProfileData(producerConnectorData);
#endif

          // Add communicator to task copy to enable communication
          taskCopy->setTaskGraphCommunicator(graph->getTaskGraphCommunicator());
          taskList.push_back(taskCopy);
          newVertices.push_back(taskCopy);
        }
        size_t threadId = 0;
        for (AnyTaskManager *taskItem : taskList) {

          TaskManagerThread *runtimeThread = new TaskManagerThread(threadId, taskItem, atomicNumThreads);
          std::thread *thread = new std::thread(&TaskManagerThread::run, runtimeThread);
          this->threads.push_back(thread);
          runtimeThreads.push_back(runtimeThread);
          threadId++;
        }

      } else {
        std::cerr << task->getName() << " has no threads specified." << std::endl;
      }
    }


#ifdef WS_PROFILE
    std::shared_ptr<ProfileData> graphCreationComplete(new GraphCompleteProfile(graph));
    graph->sendProfileData(graphCreationComplete);
#endif

    for (AnyTaskManager *newVertex : newVertices) {
      graph->addTaskManager(newVertex);
    }


    this->executed = true;

    graph->finishedSetup();
  }

 private:
  std::list<std::thread *> threads; //!< A list of all threads spawned for the Runtime
  AnyTaskGraphConf *graph; //!< The TaskGraph associated with the Runtime
  std::list<TaskManagerThread *> runtimeThreads; //!< The list of TaskManagers bound to each thread
  bool executed; //!< Whether the Runtime has been executed

};
}

#endif //HTGS_TASKGRAPHRUNTIME_HPP