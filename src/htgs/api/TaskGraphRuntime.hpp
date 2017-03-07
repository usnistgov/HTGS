
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file Runtime.hpp
 * @author Timothy Blattner
 * @date Nov 25, 2015
 *
 * @brief Spawns threads and binds them to the appropriate ITask within a TaskGraph
 * @details
 */
#ifndef HTGS_RUNTIME_HPP
#define HTGS_RUNTIME_HPP


#include <thread>
#include "htgs/core/graph/AnyTaskGraphConf.hpp"
#include "htgs/core/task/AnyTaskScheduler.hpp"

namespace htgs {
/**
 * @class Runtime Runtime.hpp <htgs/api/Runtime.hpp>
 * @brief Spawns threads and binds them to the appropriate ITask within a TaskGraph.
 * @details
 *
 * Each thread is bound to a separate ITask instance. If an ITask has more than one thread associated
 * with it, then the Runtime will create a deep copy instance of the ITask, which is bound to the thread.
 * This means that each thread has a different ITask instance.
 *
 * This process is done for every ITask in the TaskGraph that the Runtime is responsible for.
 *
 * If an ITask is an ExecutionPipeline, then the thread responsible for the ExecutionPipeline will create
 * additional Runtimes, one for each TaskGraph within the ExecutionPipeline.
 *
 * A Runtime can be executed asynchronously, allowing for interaction with the main TaskGraph to
 * submit/receive data to/from the TaskGraph using executeRuntime().
 *
 * To wait for the Runtime to finish processing all of the data for a TaskGraph, use waitForRuntime().
 *
 * To execute and wait for the Runtime, use executeAndWaitForRuntime(). If data is being produced for the task graph,
 * then the TaskGraph::finishedProducingData function must be called prior to waiting for the runtime
 * in order for the task graph to know that the
 * input to the graph has finished and the tasks processing that input can be notified.
 *
 * Example Usage:
 * @code
 * htgs::TaskGraph<Data1, Data2> *taskGraph = new htgs::TaskGraph<Data1, Data2>();
 * ...
 *
 * // If adding data to the TaskGraph, must use TaskGraph::addGraphInputConsumer and increment the input producers
 * taskGraph->addGraphInputConsumer(someTask);
 * taskGraph->incrementGraphInputProducer();
 *
 * // To receive data from the TaskGraph use  TaskGraph::addGraphOutputProducer
 * taskGraph->addGraphOutputProducer(someOutputTask);
 *
 * htgs::Runtime *runtime = new htgs::Runtime(taskGraph);
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
 * while(!taskGraph->getOutputConnector()->isInputTerminated())
 * {
 *   std::shared_ptr<Data2> data = taskGraph->getOuputConnector()->consumeData();
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
    for (TaskSchedulerThread *t : runtimeThreads) {
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
      t->join();
    }
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
    for (TaskSchedulerThread *t : runtimeThreads) {
      t->terminate();
    }
  }

  /**
   * Executes the Runtime
   */
  void executeRuntime() {
    // TODO: Spawn ioMutex and share ? ?
    if (executed)
      return;

    std::list<AnyTaskScheduler *> *vertices = this->graph->getTaskSchedulers();
    std::list<AnyTaskScheduler *> newVertices;
    DEBUG_VERBOSE("Launching runtime for " << vertices->size() << " vertices");
    for (AnyTaskScheduler *task : *vertices) {
      size_t numThreads = task->getNumThreads();

      DEBUG_VERBOSE("Spawning " << numThreads << " threads for task " << task->getName());

      if (numThreads > 0) {
        std::list<AnyTaskScheduler *> taskList;
        std::shared_ptr<std::atomic_size_t> atomicNumThreads = std::shared_ptr<std::atomic_size_t>(new std::atomic_size_t(numThreads));
        taskList.push_back(task);


        for (size_t i = 1; i < numThreads; i++) {
          AnyTaskScheduler *taskCopy = task->copy(true);
          taskList.push_back(taskCopy);
          newVertices.push_back(taskCopy);
        }
        size_t threadId = 0;
        for (AnyTaskScheduler *taskItem : taskList) {
          TaskSchedulerThread *runtimeThread = new TaskSchedulerThread(threadId, taskItem, atomicNumThreads);
          std::thread *thread = new std::thread(&TaskSchedulerThread::run, runtimeThread);
          this->threads.push_back(thread);
          runtimeThreads.push_back(runtimeThread);
          threadId++;
        }

      }
    }

    for (AnyTaskScheduler *newVertex : newVertices)
    {
      graph->addTaskScheduler(newVertex);
    }

    this->executed = true;
  }


 private:
  std::list<std::thread *> threads; //!< A list of all threads spawned for the Runtime
  AnyTaskGraphConf *graph; //!< The TaskGraph associated with the Runtime
  std::list<TaskSchedulerThread *> runtimeThreads; //!< The list of TaskSchedulers bound to each thread
  bool executed; //!< Whether the Runtime has been executed

};
}


#endif //HTGS_RUNTIME_HPP