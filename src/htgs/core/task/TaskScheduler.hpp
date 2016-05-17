
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file TaskScheduler.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief Implements a TaskScheduler that interacts with an ITask and holds the input and output Connector for the ITask
 */
#ifndef HTGS_TASKSCHEDULER_H
#define HTGS_TASKSCHEDULER_H


#include <chrono>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <sstream>
#include "../graph/BaseConnector.hpp"
#include "../../debug/debug_message.h"
#include "BaseTaskScheduler.hpp"
#include "../graph/Connector.hpp"
#include "../../api/IData.hpp"
#include "../../api/ITask.hpp"

namespace htgs {
#ifdef PROFILE
std::mutex ioMutex; //!< An ioMutex to synchronize writing to console
#endif

class IData;

template<class T>
class Connector;

template<class T, class U>
class ITask;

/**
 * @class TaskScheduler TaskScheduler.hpp <htgs/core/task/TaskScheduler.hpp>
 * @brief Encapsulates an ITask to interact with an ITask's functionality.
 * @details
 * The TaskScheduler interacts with the BaseTaskSchedulerRuntimeThread to process an ITask's input and output data.
 * The core logic of a TaskScheduler is implemented with the ITask representing the computational and logic functionality.
 *
 * When the TaskScheduler is ready to be terminated the thread associated with the TaskScheduler and the output Connector will be notified.
 * Using this approach each ITask that is terminated if the input Connector has finished producing data will be
 * closed.
 *
 * @tparam T the input data type for the TaskScheduler, T must derive from IData.
 * @tparam U the output data type for the TaskScheduler, U must derive from IData.
 * @note This class should only be called by the HTGS API
 * @note \#define PROFILE to enable profiling.
 */
template<class T, class U>
class TaskScheduler: public BaseTaskScheduler {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:
  /**
   * Constructs a TaskScheduler with an ITask as the task function and specific runtime parameters.
   * @param taskFunction the functionality for the TaskScheduler
   * @param numThreads the number of threads to operate with the TaskScheduler
   * @param isStartTask whether the TaskScheduler is a start task or not (immediately launches the ITask::execute when bound to a thread)
   * @param pipelineId the pipeline Id associated with the TaskScheduler
   * @param numPipelines the number of pipelines
   */
  TaskScheduler(ITask<T, U> *taskFunction, int numThreads, bool isStartTask, int pipelineId, int numPipelines) {
    this->taskFunction = taskFunction;
    this->taskComputeTime = 0L;
    this->taskWaitTime = 0L;
    this->poll = false;
    this->timeout = 0L;
    this->numThreads = numThreads;
    this->isStartTask = isStartTask;
    this->inputConnector = nullptr;
    this->outputConnector = nullptr;
    this->pipelineId = pipelineId;
    this->numPipelines = numPipelines;
    this->alive = true;
    this->pipelineConnectorList = std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>>(new std::vector<std::shared_ptr<BaseConnector>>());
    this->runtimeThread = nullptr;
  }

  /**
   * Constructs a TaskScheduler with an ITask as the task function and specific runtime parameters.
   * @param taskFunction the functionality for the TaskScheduler
   * @param numThreads the number of threads to operate with the TaskScheduler
   * @param isStartTask whether the TaskScheduler is a start task or not (immediately launches the ITask::execute when bound to a thread)
   * @param poll whether the TaskScheduler should poll for data
   * @param microTimeoutTime the timeout time in microseconds
   * @param pipelineId the pipeline Id associated with the TaskScheduler
   * @param numPipelines the number of pipelines
   */
  TaskScheduler(ITask<T, U> *taskFunction, int numThreads, bool isStartTask, bool poll, long microTimeoutTime,
                int pipelineId, int numPipelines) {
    this->taskFunction = taskFunction;
    this->taskComputeTime = 0L;
    this->taskWaitTime = 0L;
    this->poll = poll;
    this->timeout = microTimeoutTime;
    this->numThreads = numThreads;
    this->isStartTask = isStartTask;
    this->inputConnector = nullptr;
    this->outputConnector = nullptr;
    this->pipelineId = pipelineId;
    this->numPipelines = numPipelines;
    this->alive = true;
    this->pipelineConnectorList = std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>>(new std::vector<std::shared_ptr<BaseConnector>>());
    this->runtimeThread = nullptr;
  }

  /**
   * Constructs a TaskScheduler with an ITask as the task function and specific runtime parameters
   * @param taskFunction the functionality for the TaskScheduler
   * @param numThreads the number of threads to operate with the TaskScheduler
   * @param isStartTask whether the TaskScheduler is a start task or not (immediately launches the ITask::execute when bound to a thread)
   * @param poll whether the TaskScheduler should poll for data
   * @param microTimeoutTime the timeout time in microseconds
   * @param pipelineId the pipeline Id associated with the TaskScheduler
   * @param numPipelines the number of pipelines
   * @param pipelineConnectorList the list of Connectors from a pipeline that feed to this TaskScheduler and copies of this TaskScheduler
   */
  TaskScheduler(ITask<T, U> *taskFunction, int numThreads, bool isStartTask, bool poll, long microTimeoutTime,
                int pipelineId, int numPipelines, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> pipelineConnectorList) {
    this->taskFunction = taskFunction;
    this->taskComputeTime = 0L;
    this->taskWaitTime = 0L;
    this->poll = poll;
    this->timeout = microTimeoutTime;
    this->numThreads = numThreads;
    this->isStartTask = isStartTask;
    this->inputConnector = nullptr;
    this->outputConnector = nullptr;
    this->pipelineId = pipelineId;
    this->numPipelines = numPipelines;
    this->alive = true;
    this->pipelineConnectorList = pipelineConnectorList;
    this->runtimeThread = nullptr;
  }

  /**
   * Destructor
   */
  ~TaskScheduler() {
    delete taskFunction;
    taskFunction = nullptr;
  }

  /**
   * Adds the input Connector for this TaskScheduler to the pipeline connector list.
   * Each Connector added represents one of the other Connectors that is attached
   * to a copy of this TaskScheduler that is within the same ExecutionPipeline.
   * @param pipelineId the pipeline Id
   */
  void addPipelineConnector(int pipelineId) {
    (*pipelineConnectorList)[pipelineId] = this->getInputBaseConnector();
  }

  /**
   * Adds a Connector for a TaskScheduler that is in an ExecutionPipeline
   * Each Connector added represents one of the other Connectors that is attached
   * to a copy of this TaskScheduler that is within the same ExecutionPipeline.
   * @param pipelineId the pipeline Id
   * @param connector the connector to add
   */
  void addPipelineConnector(int pipelineId, std::shared_ptr<BaseConnector> connector) {
    (*pipelineConnectorList)[pipelineId] = connector;
  }

  /**
   * Sets the pipeline Id associated with the TaskScheduler
   * @param id the pipeline Id
   */
  void setPipelineId(int id) {
    this->pipelineId = id;
    this->taskFunction->setPipelineId(id);
  }

  /**
   * Sets the number of pipelines associated with the TaskScheduler
   * @param numPipelines the number of pipelines
   */
  void setNumPipelines(int numPipelines) {
    this->numPipelines = numPipelines;
    if (this->pipelineConnectorList->capacity() < this->numPipelines) {
      this->pipelineConnectorList->resize((unsigned long) this->numPipelines);
    }
  }

  /**
   * Sets the input Connector
   * @param connector the input connector
   */
  void setInputConnector(std::shared_ptr<Connector<T>> connector) { this->inputConnector = connector; }

  /**
   * Sets the output Connector
   * @param connector the output connector
   */
  void setOutputConnector(std::shared_ptr<Connector<U>> connector) { this->outputConnector = connector; }

  /**
   * Gets the output Connector
   * @return the output Connector
   */
  std::shared_ptr<Connector<U>> getOutputConnector() const { return this->outputConnector; }

  /**
   * Gets the input Connector
   * @return the input connector
   */
  std::shared_ptr<Connector<T>> getInputConnector() const { return this->inputConnector; }

  /**
   * Gets the input BaseConnector
   * @return the input connector
   */
  std::shared_ptr<BaseConnector> getInputBaseConnector() { return this->inputConnector; }

  /**
   * Gets the output BaseConnector
   * @return the output connector
   */
  std::shared_ptr<BaseConnector> getOutputBaseConnector() { return this->outputConnector; }

  /**
   * Sets the input BaseConnector
   * @param connector the input connector
   */
  void setInputConnector(std::shared_ptr<BaseConnector> connector) { this->inputConnector = std::dynamic_pointer_cast<Connector<T>>(connector); }

  /**
   * Sets the output BaseConnector
   * @param connector the output connector
   */
  void setOutputConnector(std::shared_ptr<BaseConnector> connector) { this->outputConnector = std::dynamic_pointer_cast<Connector<U>>(connector); }

  /**
   * Gets the ITask function associated with the TaskScheduler
   * @return the ITask
   */
  ITask<T, U> *getTaskFunction() { return this->taskFunction; };

  /**
   * Gets the number of threads associated with this TaskScheduler
   * @return the number of the threads that will execute the TaskScheduler
   */
  int getNumThreads() const { return this->numThreads; }

  /**
   * Initializes the TaskScheduler
   */
  void initialize() {
    DEBUG("initializing: " << this->prefix() << " " << this->getName() << std::endl);
    this->taskFunction->initializeITask(this->pipelineId, this->numPipelines, this, this->pipelineConnectorList);
  }

  /**
   * Shuts down the TaskScheduler
   */
  void shutdown() {
    DEBUG("shutting down: " << this->prefix() << " " << this->getName() << std::endl);
    this->taskFunction->shutdown();
  }

  /**
   * Sets the thread that is executing this TaskScheduler
   * @param runtimeThread the thread that is executing the TaskScheduler
   */
  void setRuntimeThread(BaseTaskSchedulerRuntimeThread *runtimeThread) { this->runtimeThread = runtimeThread; }

  /**
   * Gets the name of the ITask
   * @return the name of the ITask
   */
  std::string getName() { return this->taskFunction->getName(); }

  /**
   * Copies the TaskScheduler
   * @param deep whether a deep copy is required
   * @return the TaskScheduler copy
   */
  BaseTaskScheduler *copy(bool deep) {
    ITask<T, U> *iTask;
    iTask = this->taskFunction->copyITask();

    TaskScheduler<T, U>
        *newTask = new TaskScheduler<T, U>(iTask, this->numThreads, this->isStartTask, this->poll, this->timeout,
                                           this->pipelineId, this->numPipelines, this->pipelineConnectorList);
    if (deep) {
      newTask->setInputConnector(this->getInputBaseConnector());
      newTask->setOutputConnector(this->getOutputConnector());
    }
    return newTask;
  }

  /**
   * Executes the TaskScheduler.
   * Using the following procedure:
   * 0. If the ITask is a start task, then send ITask::executeTask with nullptr and set that it is no longer a startTask
   
   * 1. Checks if the ITask::isTerminated, if it is then reduce thread pool count for the runtime and wakeup
   * any tasks waiting on this TaskScheduler's input queue. If the thread pool count is zero, then indicate that
   * this task is no longer producing data  and wakup all consumers waiting on the output connector. Also indicate
   * this task is no longer releasing memory and wakeup
   * all memory managers that this task is releasing memory to.
   *
   * 2. Get input from input Connector. (optional polls for data, if timeout period expires, then will recheck if the ITask is terminated and try to get input again)
   *
   * 3. If the data is not nullptr, then sends the data to the ITask::executeTask function.
   *
   * @note \#define PROFILE to enable profiling.
   */
  void executeTask() {
    std::shared_ptr<T> data = nullptr;

    DEBUG_VERBOSE(prefix() << "Running task: " << this->getName());

    if (isStartTask) {
      DEBUG_VERBOSE(prefix() << this->getName() << " is a start task");
      isStartTask = false;
      auto start = std::chrono::high_resolution_clock::now();
      this->taskFunction->executeTask(nullptr);
      auto finish = std::chrono::high_resolution_clock::now();

      this->taskComputeTime += std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
      return;
    } else if (this->taskFunction->isTerminated(this->inputConnector)) {

#ifdef PROFILE
      {
          std::unique_lock<std::mutex> lock(ioMutex);
          std::cout << "===================== " << this->getName() << " "<< prefix() << " ===================" << std::endl;
          std::cout << "COMPUTE TIME: " << taskComputeTime << " us   WAIT TIME: " << taskWaitTime << " us" << std::endl;

          if (this->inputConnector != nullptr) {
              std::cout << "Input connector: ";
              this->inputConnector->profileConsume(this->numThreads, true);
          }
          if (this->outputConnector != nullptr) {
              std::cout << "Output connector: ";
              this->outputConnector->profileProduce(this->numThreads);
          }
          this->taskFunction->profileITask();
          std::cout << "-------------------------- " << this->getName() << " (thread: " << this->threadId << ") -------------------------- " << std::endl << std::endl;
      }
#endif


      DEBUG(prefix() << this->getName() << " task function is terminated");
      this->alive = false;
      this->inputConnector->wakeupConsumer();


      if (this->runtimeThread != nullptr) {
        this->runtimeThread->terminate();

        if (this->runtimeThread->decrementAndCheckNumThreadsRemaining()) {
          if (this->outputConnector != nullptr) {
            this->outputConnector->producerFinished();

            if (this->outputConnector->isInputTerminated())
              this->outputConnector->wakeupConsumer();
          }

          std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> >> memReleasers = this->taskFunction->getMemReleasers();

          DEBUG(prefix() << " " << this->getName() << " Shutting down " << memReleasers->size() <<
              " memory releasers");
          for (std::pair<std::string, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> > pair : *memReleasers) {


            if (this->taskFunction->isMemReleaserOutsideGraph(pair.first))
            {
              DEBUG(prefix() << " " << this->getName() << " Shutting down ALL memory releasers : " <<
                  pair.first << " with " << pair.second->size() << " connectors");
              for (auto connector : *pair.second)
              {
                connector->producerFinished();


                if (connector->isInputTerminated())
                  connector->wakeupConsumer();
              }
            }
            else {
              DEBUG(prefix() << " " << this->getName() << " Shutting down memory releaser : " <<
                  pair.first << " with " << pair.second->size() << " connectors");
              std::shared_ptr<BaseConnector> connector = pair.second->at((unsigned long) this->pipelineId);
              connector->producerFinished();


              if (connector->isInputTerminated())
                connector->wakeupConsumer();
            }
          }

        }
      }
      else {
        if (this->outputConnector != nullptr) {
          this->outputConnector->producerFinished();

          if (this->outputConnector->isInputTerminated()) {
            this->outputConnector->wakeupConsumer();
          }
        }
      }
      return;
    }
    auto start = std::chrono::high_resolution_clock::now();

    if (poll)
      data = this->inputConnector->pollConsumeData(this->timeout);
    else
      data = this->inputConnector->consumeData();

    auto finish = std::chrono::high_resolution_clock::now();

    this->taskWaitTime += std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();

    DEBUG_VERBOSE(prefix() << this->getName() << " received data: " << data << " from " << inputConnector);

    if (data != nullptr) {
      auto start = std::chrono::high_resolution_clock::now();
      this->taskFunction->executeTask(data);
      auto finish = std::chrono::high_resolution_clock::now();

      this->taskComputeTime += std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
    }

  }

  /**
   * Creates a TaskScheduler using the parameters within the taskFunction
   * @param taskFunction the task function to create the TaskScheduler from
   */
  static TaskScheduler<T, U> *createTask(ITask<T, U> *taskFunction) {
    return new TaskScheduler<T, U>(taskFunction,
                                   taskFunction->getNumThreads(),
                                   taskFunction->getIsStartTask(),
                                   taskFunction->isPoll(),
                                   taskFunction->getMicroTimeoutTime(),
                                   0,
                                   1);
  };

  /**
   * Gets whether the TaskScheduler is alive or not
   * @return whether the TaskScheduler is alive
   * @retval TRUE if the TaskScheduler is alive
   * @retval FALSE if the TaskScheduler is not alive
   */
  bool isAlive() { return this->alive; }

  /**
   * Provides debug output
   * @note \#define DEBUG_FLAG to enable debugging.
   */
  void debug() {
    DEBUG(prefix() << this->getName() << " input connector: " << inputConnector << " output connector: " <<
        outputConnector << " Details: " << std::endl);
    this->taskFunction->debug();
  }

  /**
   * Sets the thread id associated with the TaskScheduler
   * @param id the thread id
   */
  void setThreadId(int id) {
    this->threadId = id;
  }

  /**
   * Gets the dot notation for this TaskScheduler.
   */
  std::string getDot() {
    return this->taskFunction->getDot(this->inputConnector, this->outputConnector);
  }

  /**
   * Adds the result data to the output connector
   */
  void addResult(std::shared_ptr<U> result)
  {
    if (this->outputConnector != nullptr)
      this->outputConnector->produceData(result);
  }

 private:
  //! @cond Doxygen_Suppress
  std::string prefix() {
    return std::string(
        "Thread id: " + std::to_string(this->threadId) + " (out of " + std::to_string(this->numThreads)
            + "); Pipeline id " + std::to_string(this->pipelineId) + " (out of " + std::to_string(this->numPipelines) +
            ") ");
  }
  //! @endcond

  std::shared_ptr<Connector<T>> inputConnector; //!< The input connector for the scheduler (queue to get data from)
  std::shared_ptr<Connector<U>> outputConnector; //!< The output connector for the scheduler (queue to send data)

  long long int taskComputeTime; //!< The total compute time for the task
  long long int taskWaitTime; //!< The total wait time for the task

  ITask<T, U> *taskFunction; //!< The task that is managed by the scheduler

  long timeout; //!< The timeout time for polling in microseconds
  bool poll; //!< Whether the scheduler should poll for data

  bool isStartTask; //!< Whether the task should start immediately
  bool alive; //!< Whether the task is still alive

  int threadId; //!< The thread id for the task (set after initialization)
  int numThreads; //!< The number of threads spawned for the scheduler

  int pipelineId; //!< The execution pipeline id
  int numPipelines; //!< The number of execution pipelines

  std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>>
      pipelineConnectorList; //!< The execution pipeline connector list (one for each pipeline that share the same ITask functionality)
  BaseTaskSchedulerRuntimeThread *runtimeThread; //!< The thread that is executing this task's runtime
};
}


#endif //HTGS_TASKSCHEDULER_H
