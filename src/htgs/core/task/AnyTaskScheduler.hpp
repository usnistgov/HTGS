// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file AnyTaskScheduler.hpp
 * @author Timothy Blattner
 * @date Nov 18, 2015
 *
 * @brief Implements the parent class for a Task to remove the template arguments and the TaskSchedulerThread to attach a thread to a Task.
 */
#ifndef HTGS_ANYTASKSCHEDULER_HPP
#define HTGS_ANYTASKSCHEDULER_HPP


#include <atomic>
#include <memory>
#include <vector>

#include <htgs/types/Types.hpp>
#include "AnyITask.hpp"

namespace htgs {
class TaskSchedulerThread;

/**
 * @class AnyTaskScheduler AnyTaskScheduler.hpp <htgs/core/task/AnyTaskScheduler.hpp>
 * @brief The parent class for a Task that removes the template arguments.
 * @details
 * The AnyTaskScheduler provides access to functionality that does not require template arguments and
 * allows storage of a Task.
 *
 * @note This class should only be called by the HTGS API
 */
class AnyTaskScheduler {
 public:

  /**
 * Constructs an AnyTaskScheduler with an ITask as the task function and specific runtime parameters.
 * @param taskFunction the functionality for the TaskScheduler
 * @param numThreads the number of threads to operate with the TaskScheduler
 * @param isStartTask whether the TaskScheduler is a start task or not (immediately launches the ITask::execute when bound to a thread)
 * @param pipelineId the pipeline Id associated with the TaskScheduler
 * @param numPipelines the number of pipelines
 */
  AnyTaskScheduler(size_t numThreads, bool isStartTask, size_t pipelineId, size_t numPipelines) {
    this->taskComputeTime = 0L;
    this->taskWaitTime = 0L;
    this->poll = false;
    this->timeout = 0L;
    this->numThreads = numThreads;
    this->threadId = 0;
    this->startTask = isStartTask;
    this->pipelineId = pipelineId;
    this->numPipelines = numPipelines;
    this->alive = true;
    this->pipelineConnectorList = std::shared_ptr<std::vector<std::shared_ptr<AnyConnector>>>(new std::vector<std::shared_ptr<AnyConnector>>());
  }

  /**
   * Constructs an AnyTaskScheduler with an ITask as the task function and specific runtime parameters.
   * @param numThreads the number of threads to operate with the TaskScheduler
   * @param isStartTask whether the TaskScheduler is a start task or not (immediately launches the ITask::execute when bound to a thread)
   * @param poll whether the TaskScheduler should poll for data
   * @param microTimeoutTime the timeout time in microseconds
   * @param pipelineId the pipeline Id associated with the TaskScheduler
   * @param numPipelines the number of pipelines
   */
  AnyTaskScheduler(size_t numThreads, bool isStartTask, bool poll, size_t microTimeoutTime, size_t pipelineId, size_t numPipelines) {
    this->taskComputeTime = 0L;
    this->taskWaitTime = 0L;
    this->poll = poll;
    this->timeout = microTimeoutTime;
    this->numThreads = numThreads;
    this->threadId = 0;
    this->startTask = isStartTask;
    this->pipelineId = pipelineId;
    this->numPipelines = numPipelines;
    this->alive = true;
    // TODO: Can get rid of this . . .
    this->pipelineConnectorList = std::shared_ptr<std::vector<std::shared_ptr<AnyConnector>>>(new std::vector<std::shared_ptr<AnyConnector>>());
  }

  /**
   * Constructs an AnyTaskScheduler with an ITask as the task function and specific runtime parameters
   * @param numThreads the number of threads to operate with the TaskScheduler
   * @param isStartTask whether the TaskScheduler is a start task or not (immediately launches the ITask::execute when bound to a thread)
   * @param poll whether the TaskScheduler should poll for data
   * @param microTimeoutTime the timeout time in microseconds
   * @param pipelineId the pipeline Id associated with the TaskScheduler
   * @param numPipelines the number of pipelines
   * @param pipelineConnectorList the list of Connectors from a pipeline that feed to this TaskScheduler and copies of this TaskScheduler
   */
  AnyTaskScheduler(size_t numThreads, bool isStartTask, bool poll, size_t microTimeoutTime,
  size_t pipelineId, size_t numPipelines, std::shared_ptr<std::vector<std::shared_ptr<AnyConnector>>> pipelineConnectorList) {
    this->taskComputeTime = 0L;
    this->taskWaitTime = 0L;
    this->poll = poll;
    this->timeout = microTimeoutTime;
    this->numThreads = numThreads;
    this->threadId = 0;
    this->startTask = isStartTask;
    this->pipelineId = pipelineId;
    this->numPipelines = numPipelines;
    this->alive = true;
    this->pipelineConnectorList = pipelineConnectorList;
  }


  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////// VIRTUAL FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Destructor
   */
  virtual ~AnyTaskScheduler() { };

  /**
   * Gets the ITask function associated with the TaskScheduler
   * @return the ITask
   */
  virtual AnyITask *getTaskFunction() = 0;

  /**
   * Gets the input Connector
   * @return the input connector
   */
  virtual std::shared_ptr<AnyConnector> getInputConnector() = 0;

  /**
   * Gets the output Connector
   * @return the output connector
   */
  virtual std::shared_ptr<AnyConnector> getOutputConnector() = 0;

  /**
   * Copies the TaskScheduler
   * @param deep whether a deep copy is required
   * @return the TaskScheduler copy
   */
  virtual AnyTaskScheduler *copy(bool deep)  = 0;


  /**
   * Initializes the TaskScheduler
   */
  virtual void initialize() = 0;


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
  virtual void executeTask() = 0;

  /**
   * Sets the thread that is executing this TaskScheduler
   * @param runtimeThread the thread that is executing the TaskScheduler
   */
  virtual void setRuntimeThread(TaskSchedulerThread *runtimeThread) = 0;

  /**
   * Sets the input BaseConnector
   * @param connector the input connector
   */
  virtual void setInputConnector(std::shared_ptr<AnyConnector> connector) = 0;

  /**
   * Sets the output BaseConnector
   * @param connector the output connector
   */
  virtual void setOutputConnector(std::shared_ptr<AnyConnector> connector) = 0;

  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  // TODO: This may not be necessary with new changes .. .
  /**
   * Adds the input Connector for this TaskScheduler to the pipeline connector list.
   * Each Connector added represents one of the other Connectors that is attached
   * to a copy of this TaskScheduler that is within the same ExecutionPipeline.
   * @param pipelineId the pipeline Id
   */
  void addPipelineConnector(size_t pipelineId) {
    (*pipelineConnectorList)[pipelineId] = this->getInputConnector();
  }

  // TODO: This may not be necessary with new changes . . .
  /**
   * Adds a Connector for a TaskScheduler that is in an ExecutionPipeline
   * Each Connector added represents one of the other Connectors that is attached
   * to a copy of this TaskScheduler that is within the same ExecutionPipeline.
   * @param pipelineId the pipeline Id
   * @param connector the connector to add
   */
  void addPipelineConnector(size_t pipelineId, std::shared_ptr<AnyConnector> connector) {
    (*pipelineConnectorList)[pipelineId] = connector;
  }

  // TODO: The pipeline connector list may not be necessary anymore . . .
  /**
   * Gets the pipeline connector list for this task
   * @return the pipeline connector list
   */
  std::shared_ptr<ConnectorVector> getPipelineConnectors() { return this->pipelineConnectorList; }

  // TODO: No need to resize with new changes . . .
  /**
   * Sets the number of pipelines associated with the TaskScheduler
   * @param numPipelines the number of pipelines
   */
  void setNumPipelines(size_t numPipelines) {
    this->numPipelines = numPipelines;
    if (this->pipelineConnectorList->capacity() < this->numPipelines) {
      this->pipelineConnectorList->resize((unsigned long) this->numPipelines);
    }
  }

  /**
   * Gets the number of pipelines that this task scheduler belongs too.
   * @return the number of pipelines spawned from the execution pipeline task
   */
  size_t getNumPipelines() { return this->numPipelines; }

  /**
   * Sets the pipeline Id associated with the TaskScheduler
   * @param id the pipeline Id
   */
  void setPipelineId(size_t id) {
    this->pipelineId = id;
    this->getTaskFunction()->setPipelineId(id);
  }

  /**
   * Gets the pipeline identifer for this task from 0 to number of pipelines - 1.
   * @return the pipeline identifier
   */
  size_t getPipelineId() { return this->pipelineId; }

  /**
   * Gets the number of threads associated with this TaskScheduler
   * @return the number of the threads that will execute the TaskScheduler
   */
  size_t getNumThreads() const { return this->numThreads; }

  /**
   * Sets the alive state for this task scheduler
   * @param val the value to set, true = alive, false = dead/terminating
   */
  void setAlive(bool val) { this->alive = val; }

  /**
   * Gets whether the TaskScheduler is alive or not
   * @return whether the TaskScheduler is alive
   * @retval TRUE if the TaskScheduler is alive
   * @retval FALSE if the TaskScheduler is not alive
   */
  bool isAlive() { return this->alive; }

  /**
   * Sets whether this task scheduler is a start task or not, which will immediately begin executing
   * by sending nullptr data to the underlying task as soon as this task executes.
   * @param val the value to set, true = is a start task, false = not a start task
   * @note Should be set before a task begins executing (attached to a thread)
   */
  void setStartTask(bool val) { this->startTask = val; }

  /**
   * Gets whether this task scheduler will begin executing immediately with nullptr data or not.
   * @return whether the task scheduler will start immediately.
   * @retval TRUE if the task scheduler will begin executing immediately
   * @retval FALSE if the task scheduler will not begin and wait for its first input data
   */
  bool isStartTask() { return this->startTask; }

  /**
   * Gets whether the task scheduler is polling for data or not
   * @return whether the task scheduler is polling or not
   * @retval TRUE if the task scheduler is polling for data from its input
   * @retval FALSE if the task scheduler is not polling (waiting) for data from its input
   */
  bool isPoll() { return this->poll; }

  /**
   * Gets the timeout period in microseconds for the task when the task is polling for data.
   * @return the timeout time in microseconds for polling
   */
  size_t getTimeout() { return this->timeout; }

  /**
   * Increments the compute time profile value
   * @param val the value to increment by
   */
  void incTaskComputeTime(long val) { this->taskComputeTime += val;}

  /**
   * Increments the wait time profile value
   * @param val the value to increment by
   */
  void incWaitTime(long val) { this->taskWaitTime += val; }

  /**
   * Shuts down the TaskScheduler
   */
  void shutdown() {
    DEBUG("shutting down: " << this->prefix() << " " << this->getName() << std::endl);
    this->getTaskFunction()->shutdown();
  }

  /**
   * Gets the name of the ITask
   * @return the name of the ITask
   */
  std::string getName() { return this->getTaskFunction()->getName(); }


  /**
   * Provides debug output
   * @note \#define DEBUG_FLAG to enable debugging.
   */
  void debug() {
    DEBUG(prefix() << this->getName() << " input connector: " << getInputConnector() << " output connector: " <<
                   getOutputConnector() << " Details: " << std::endl);
    this->getTaskFunction()->debug();
  }


  /**
 * Gets the name of the ITask with it's pipeline ID
 * @return  the name of the task with the pipeline ID
 */
  std::string getNameWithPipelineId() { return this->getTaskFunction()->getNameWithPipelineId(); }

  /**
 * Gets the dot notation for this TaskScheduler.
 */
  std::string getDot(int flags) {
    if ((flags & DOTGEN_FLAG_SHOW_ALL_THREADING) != 0) {
      return this->getTaskFunction()->genDot(flags, this->getInputConnector(), this->getOutputConnector());
    } else if (this->threadId == 0){
      return this->getTaskFunction()->genDot(flags, this->getInputConnector(), this->getOutputConnector());
    }
    else
    {
      return "";
    }
  }

  /**
   * Sets the thread id associated with the TaskScheduler
   * @param id the thread id
   */
  void setThreadId(size_t id) {
    this->threadId = id;
  }

#ifdef PROFILE
  std::string genDotProfile(int flags, std::unordered_map<std::string, double> *mmap, std::string desc,
                            std::unordered_map<std::string, std::string> *colorMap) {
    if ((flags & DOTGEN_FLAG_SHOW_ALL_THREADING) != 0) {
      double val = 0.0;
      if (desc == "Compute Time (sec): ")
        val = this->taskComputeTime / 1000000;
      else if (desc == "Wait Time (sec): ")
        val = this->taskWaitTime / 1000000;
      else if (desc == "Max Q Size: ")
        val = this->inputConnector != nullptr ? this->inputConnector->getMaxQueueSize() : 0;
      return this->taskFunction->getDotProfile(flags, mmap, val, desc, colorMap);
    } else if (this->threadId == 0){
      return this->taskFunction->getDotProfile(flags, mmap, mmap->at(this->getNameWithPipID()), desc, colorMap);
    }
    else
    {
      return "";
    }
  }

  void gatherComputeTime(std::unordered_multimap<std::string, long long int> *mmap)
  {
    mmap->insert(std::pair<std::string, long long int>(this->getNameWithPipID(), this->taskComputeTime));
    this->taskFunction->gatherComputeTime(mmap);
  }

  void gatherWaitTime(std::unordered_multimap<std::string, long long int> *mmap)
  {
    mmap->insert(std::pair<std::string, long long int>(this->getNameWithPipID(), this->taskWaitTime));
    this->taskFunction->gatherWaitTime(mmap);
  }

  void gatherMaxQSize(std::unordered_multimap<std::string, int> *mmap)
  {
    if (inputConnector != nullptr)
      mmap->insert(std::pair<std::string, int>(this->getNameWithPipID(), this->inputConnector->getMaxQueueSize()));
    this->taskFunction->gatherMaxQSize(mmap);
  }

  long long int getComputeTime() { return taskComputeTime; }
  long long int getWaitTime() { return taskWaitTime; }
  int getMaxQueueSize() { return this->inputConnector != nullptr ? this->inputConnector->getMaxQueueSize() : 0;}
#endif


  //! @cond Doxygen_Suppress
  std::string prefix() {
    return std::string(
        "Thread id: " + std::to_string(this->threadId) + " (out of " + std::to_string(this->numThreads)
            + "); Pipeline id " + std::to_string(this->pipelineId) + " (out of " + std::to_string(this->numPipelines) +
            ") ");
  }
  //! @endcond


 private:

  unsigned long long int taskComputeTime; //!< The total compute time for the task
  unsigned long long int taskWaitTime; //!< The total wait time for the task

  size_t timeout; //!< The timeout time for polling in microseconds
  bool poll; //!< Whether the scheduler should poll for data

  bool startTask; //!< Whether the task should start immediately
  bool alive; //!< Whether the task is still alive

  size_t threadId; //!< The thread id for the task (set after initialization)
  size_t numThreads; //!< The number of threads spawned for the scheduler

  size_t pipelineId; //!< The execution pipeline id
  size_t numPipelines; //!< The number of execution pipelines

  std::shared_ptr<ConnectorVector> pipelineConnectorList; //!< The execution pipeline connector list (one for each pipeline that share the same ITask functionality)
};


/**
 * @class TaskSchedulerThread AnyTaskScheduler.hpp <htgs/task/AnyTaskScheduler.hpp>
 * @brief Manages a TaskScheduler that is bound to a thread for execution
 * @details
 * A Runtime will spawn a thread and bind it to the run function
 * within this class. If a Task has more than one threads associated
 * with it, then this class is duplicated one per thread, each with
 * a separate copy of the original TaskScheduler.
 *
 * @note This class should only be called by the HTGS API
 */
class TaskSchedulerThread {
 public:
  /**
   * Constructs a TaskSchedulerThread with a specified AnyTaskScheduler and atomic number of threads
   * that is shared among all other threads that operate with a copy of the same AnyTaskScheduler
   * @param threadId the thread Id for the task
   * @param task the task the thread is associated with
   * @param numThreads the number of threads that a task contains
   */
  TaskSchedulerThread(size_t threadId, AnyTaskScheduler *task, std::shared_ptr<std::atomic_size_t> numThreads) {
    this->task = task;
    this->terminated = false;
    this->numThreads = numThreads;
    this->task->setRuntimeThread(this);
    this->task->setThreadId(threadId);
  }

  /**
   * Destructor
   */
  ~TaskSchedulerThread() {
  }

  /**
   * Executes the task until the underlying Task has been terminated
   * @return status code
   * @retval 0 Completed successfully
   */
  int run(void) {
    DEBUG("Starting Thread for task : " << task->getName());
    this->task->initialize();
    while (!this->terminated) {
      this->task->executeTask();
    }
    this->task->shutdown();

    return 0;
  }

  /**
   * Gets the number of threads remaining
   * @return the number of threads remaining
   */
  size_t getThreadsRemaining() { return *this->numThreads; }

  /**
   * Decrements the number of threads remaining by one.
   */
  void decrementNumThreadsRemaining() { (*this->numThreads)--; }

  /**
   * Decrements the number of threads and checks if there are no threads remaining in a single operation.
   * @return whether there are no more threads executing the Task
   * @retval TRUE if there are no more threads executing a Task
   * @retval FALSE if there are threads executing a Task
   */
  bool decrementAndCheckNumThreadsRemaining() {
    // Performs pre-decrement
    size_t current = this->numThreads->fetch_sub(1) - 1;
    return current == 0;
  }

  /**
   * Checks if there are no more threads executing a Task
   * @return whether there are no more threads executing the Task
   * @retval TRUE if there are no more threads executing a Task
   * @retval FALSE if there are threads executing a Task
   */
  bool hasNoThreadsRemaining() { return (*this->numThreads) == 0; }

  /**
   * Indicates that the thread is ready to be terminated.
   * This function will mark the thread is ready to be terminated, but will only end once the thread has
   * finished processing its last data.
   */
  void terminate() { this->terminated = true; }

 private:
  volatile bool terminated; //!< Whether the thread is ready to be terminated or not
  std::shared_ptr<std::atomic_size_t> numThreads; //!< The number of total threads managing the TaskScheduler
  AnyTaskScheduler *task; //!< The TaskScheduler that is called from the thread
};

}

#endif //HTGS_ANYTASKSCHEDULER_HPP
