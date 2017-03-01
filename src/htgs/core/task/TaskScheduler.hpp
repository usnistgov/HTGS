
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
#ifndef HTGS_TASKSCHEDULER_HPP
#define HTGS_TASKSCHEDULER_HPP


#include <chrono>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <sstream>

#include <htgs/core/task/AnyTaskScheduler.hpp>
#include <htgs/api/ITask.hpp>

namespace htgs {

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
class TaskScheduler: public AnyTaskScheduler {
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
  TaskScheduler(ITask<T, U> *taskFunction, size_t numThreads, bool isStartTask, size_t pipelineId, size_t numPipelines) :
      super(numThreads, isStartTask, pipelineId, numPipelines),
      inputConnector(nullptr), outputConnector(nullptr), taskFunction(taskFunction), runtimeThread(nullptr) {
    taskFunction->setTaskScheduler(this);
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
  TaskScheduler(ITask<T, U> *taskFunction, size_t numThreads, bool isStartTask, bool poll, size_t microTimeoutTime,
                size_t pipelineId, size_t numPipelines) : super(numThreads, isStartTask, poll, microTimeoutTime, pipelineId, numPipelines),
                         inputConnector(nullptr), outputConnector(nullptr), taskFunction(taskFunction), runtimeThread(nullptr) {
    taskFunction->setTaskScheduler(this);
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
  TaskScheduler(ITask<T, U> *taskFunction, size_t numThreads, bool isStartTask, bool poll, size_t microTimeoutTime,
                size_t pipelineId, size_t numPipelines, std::shared_ptr<std::vector<std::shared_ptr<AnyConnector>>> pipelineConnectorList)
      : super(numThreads, isStartTask, poll, microTimeoutTime, pipelineId, numPipelines, pipelineConnectorList),
        inputConnector(nullptr), outputConnector(nullptr), taskFunction(taskFunction), runtimeThread(nullptr) {
    taskFunction->setTaskScheduler(this);
  }



  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////// INHERITED FUNCTIONS /////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  ~TaskScheduler() override {
    delete taskFunction;
    taskFunction = nullptr;
  }


  std::shared_ptr<AnyConnector> getInputConnector() override { return this->inputConnector; }


  std::shared_ptr<AnyConnector> getOutputConnector() override { return this->outputConnector; }

  void initialize() override {
    DEBUG("initializing: " << this->prefix() << " " << this->getName() << std::endl);
    this->taskFunction->initialize(this->getPipelineId(), this->getNumPipelines(), this, this->getPipelineConnectors());
  }

  void setRuntimeThread(TaskSchedulerThread *runtimeThread) override { this->runtimeThread = runtimeThread; }

  AnyITask *getTaskFunction() override {
    return this->taskFunction;
  }

  AnyTaskScheduler *copy(bool deep)  override {
    ITask<T, U> *iTask = this->taskFunction->copyITask();

    TaskScheduler<T, U>
        *newTask = new TaskScheduler<T, U>(iTask, this->getNumThreads(), this->isStartTask(), this->isPoll(), this->getTimeout(),
                                           this->getPipelineId(), this->getNumPipelines(), this->getPipelineConnectors());
    if (deep) {
      newTask->setInputConnector(this->getInputConnector());
      newTask->setOutputConnector(this->getOutputConnector());
    }
    return (AnyTaskScheduler *) newTask;
  }

  void executeTask() override {
    std::shared_ptr<T> data = nullptr;

    DEBUG_VERBOSE(prefix() << "Running task: " << this->getName());

    if (this->isStartTask()) {
      DEBUG_VERBOSE(prefix() << this->getName() << " is a start task");
      this->setStartTask(false);
      auto start = std::chrono::high_resolution_clock::now();
      this->taskFunction->executeTask(nullptr);
      auto finish = std::chrono::high_resolution_clock::now();

      this->incTaskComputeTime(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());
      return;
    } else if (this->taskFunction->canTerminate(this->inputConnector)) {

      DEBUG(prefix() << this->getName() << " task function is terminated");
      this->processTaskFunctionTerminated();

      return;
    }
    auto start = std::chrono::high_resolution_clock::now();

    if (this->isPoll())
      data = this->inputConnector->pollConsumeData(this->getTimeout());
    else
      data = this->inputConnector->consumeData();

    auto finish = std::chrono::high_resolution_clock::now();

    this->incWaitTime(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());

    DEBUG_VERBOSE(prefix() << this->getName() << " received data: " << data << " from " << inputConnector);

    if (data != nullptr) {
      start = std::chrono::high_resolution_clock::now();
      this->taskFunction->executeTask(data);
      finish = std::chrono::high_resolution_clock::now();

      this->incTaskComputeTime(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());
    }

  }

  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Sets the input BaseConnector
   * @param connector the input connector
   */
  void setInputConnector(std::shared_ptr<AnyConnector> connector) override {
    if (connector != nullptr)
      this->inputConnector = std::dynamic_pointer_cast<Connector<T>>(connector);
    else
      this->inputConnector = nullptr;

  }

  /**
   * Sets the output BaseConnector
   * @param connector the output connector
   */
  void setOutputConnector(std::shared_ptr<AnyConnector> connector) override {
    if (connector != nullptr)
      this->outputConnector = std::dynamic_pointer_cast<Connector<U>>(connector);
    else
      this->outputConnector = nullptr;
  }

  /**
   * Adds the result data to the output connector
   * @param result the result that is added to the output for this task
   */
  void addResult(std::shared_ptr<U> result)
  {
    if (this->outputConnector != nullptr)
      this->outputConnector->produceData(result);
  }




 private:

  //! @cond Doxygen_Suppress
  void processTaskFunctionTerminated() {
#ifdef PROFILE
    {
      // TODO: Handle mutex
//          std::unique_lock<std::mutex> lock(ioMutex);
          std::cout << "===================== " << this->getName() << " "<< prefix() << " ===================" << std::endl;
          std::cout << "COMPUTE TIME: " << taskComputeTime << " us   WAIT TIME: " << taskWaitTime << " us" << std::endl;

          if (this->getInputConnector() != nullptr) {
              std::cout << "Input connector: ";
              this->getInputConnector()->profileConsume(this->numThreads, true);
          }
          if (this->getOutputConnector() != nullptr) {
              std::cout << "Output connector: ";
              this->getOutputConnector()->profileProduce(this->numThreads);
          }
          this->getTaskFunction()->profileITask();
          std::cout << "-------------------------- " << this->getName() << " (thread: " << this->threadId << ") -------------------------- " << std::endl << std::endl;
      }
#endif

    // Task is now terminated, so it is no longer alive
    this->setAlive(false);

    // Wake up the threads for this task
    this->getInputConnector()->wakeupConsumer();

    // If there is a runtime thread, then begin termination
    if (this->runtimeThread != nullptr) {
      this->runtimeThread->terminate();

      // If this is the last thread for this task then close the output
      if (this->runtimeThread->decrementAndCheckNumThreadsRemaining()) {
        if (this->getOutputConnector() != nullptr) {
          this->getOutputConnector()->producerFinished();

          if (this->getOutputConnector()->isInputTerminated())
            this->getOutputConnector()->wakeupConsumer();
        }

        // Notify the memory release edge memory manager task that it is no longer receiving data
        std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<std::vector<std::shared_ptr<AnyConnector>>> >> memReleasers = this->getTaskFunction()->getReleaseMemoryEdges();

        DEBUG(prefix() << " " << this->getName() << " Shutting down " << memReleasers->size() << " memory releasers");
        for (std::pair<std::string, std::shared_ptr<std::vector<std::shared_ptr<AnyConnector>>> > pair : *memReleasers) {

          // TODO: We should be able to remove all instances of 'odd' behavior like releasing memory that is not within this graph
          if (this->getTaskFunction()->isReleaseMemoryOutsideGraph(pair.first))
          {
            DEBUG(prefix() << " " << this->getName() << " Shutting down ALL memory releasers : " <<  pair.first
                           << " with " << pair.second->size() << " connectors");
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
            std::shared_ptr<AnyConnector> connector = pair.second->at(this->getPipelineId());
            connector->producerFinished();


            if (connector->isInputTerminated())
              connector->wakeupConsumer();
          }
        }

      }
    }
    else {
      if (this->getOutputConnector() != nullptr) {
        this->getOutputConnector()->producerFinished();

        if (this->getOutputConnector()->isInputTerminated()) {
          this->getOutputConnector()->wakeupConsumer();
        }
      }
    }
  }
  //! @endcond

  typedef AnyTaskScheduler super;

  std::shared_ptr<Connector<T>> inputConnector; //!< The input connector for the scheduler (queue to get data from)
  std::shared_ptr<Connector<U>> outputConnector; //!< The output connector for the scheduler (queue to send data)
  ITask<T, U> *taskFunction; //!< The task that is managed by the scheduler
  TaskSchedulerThread *runtimeThread; //!< The thread that is executing this task's runtime

};
}


#endif //HTGS_TASKSCHEDULER_HPP
