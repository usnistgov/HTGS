// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file AnyTaskManager.hpp
 * @author Timothy Blattner
 * @date Nov 18, 2015
 *
 * @brief Implements the parent class for a Task to remove the template arguments and the TaskManagerThread to attach a thread to a Task.
 */
#ifndef HTGS_ANYTASKMANAGER_HPP
#define HTGS_ANYTASKMANAGER_HPP

#include <atomic>
#include <memory>
#include <vector>

#include <htgs/types/Types.hpp>
#include <htgs/core/comm/TaskGraphCommunicator.hpp>
#include <htgs/core/graph/profile/TaskManagerProfile.hpp>
#include <htgs/core/task/AnyITask.hpp>
#include <htgs/core/graph/profile/NVTXProfiler.hpp>
#ifdef USE_NVTX
#include <nvtx3/nvToolsExt.h>
#endif

#ifdef WS_PROFILE
#include <htgs/core/graph/profile/ProfileData.hpp>
#include <htgs/core/graph/profile/CustomProfile.hpp>
#endif

namespace htgs {
class TaskManagerThread;

/**
 * @class AnyTaskManager AnyTaskManager.hpp <htgs/core/task/AnyTaskManager.hpp>
 * @brief The parent class for a Task that removes the template arguments.
 * @details
 * The AnyTaskManager provides access to functionality that does not require template arguments and
 * allows storage of a Task.
 *
 * @note This class should only be called by the HTGS API
 */
class AnyTaskManager {
 public:

  /**
 * Constructs an AnyTaskManager with an ITask as the task function and specific runtime parameters.
 * @param numThreads the number of threads to operate with the TaskManager
 * @param isStartTask whether the TaskManager is a start task or not (immediately launches the ITask::execute when bound to a thread)
 * @param pipelineId the pipeline Id associated with the TaskManager
 * @param numPipelines the number of pipelines
 * @param address the address of the task graph that owns this task
 */
  AnyTaskManager(size_t numThreads, bool isStartTask, size_t pipelineId, size_t numPipelines, std::string address) {
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
    this->address = address;
  }

  /**
   * Constructs an AnyTaskManager with an ITask as the task function and specific runtime parameters.
   * @param numThreads the number of threads to operate with the TaskManager
   * @param isStartTask whether the TaskManager is a start task or not (immediately launches the ITask::execute when bound to a thread)
   * @param poll whether the TaskManager should poll for data
   * @param microTimeoutTime the timeout time in microseconds
   * @param pipelineId the pipeline Id associated with the TaskManager
   * @param numPipelines the number of pipelines
   * @param address the address of the task graph that owns this task
   */
  AnyTaskManager(size_t numThreads,
                 bool isStartTask,
                 bool poll,
                 size_t microTimeoutTime,
                 size_t pipelineId,
                 size_t numPipelines,
                 std::string address) {
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
    this->address = address;
  }

  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////// VIRTUAL FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Destructor
   */
  virtual ~AnyTaskManager() {  };

  /**
   * Gets the ITask function associated with the TaskManager
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
   * Copies the TaskManager
   * @param deep whether a deep copy is required
   * @return the TaskManager copy
   */
  virtual AnyTaskManager *copy(bool deep)  = 0;

  /**
   * Initializes the TaskManager
   */
  virtual void initialize() = 0;

  /**
   * Executes the TaskManager.
   * Using the following procedure:
   * 0. If the ITask is a start task, then send ITask::executeTask with nullptr and set that it is no longer a startTask

   * 1. Checks if the ITask::isTerminated, if it is then reduce thread pool count for the runtime and wakeup
   * any tasks waiting on this TaskManager's input queue. If the thread pool count is zero, then indicate that
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
   * Sets the thread that is executing this TaskManager
   * @param runtimeThread the thread that is executing the TaskManager
   */
  virtual void setRuntimeThread(TaskManagerThread *runtimeThread) = 0;

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

  /**
   * Terminates all Connector edges.
   * This is called after all threads have shutdown.
   */
  virtual void terminateConnections() = 0;

  /**
   * Gathers profiling data for the TaskProfiler
   * @param taskManagerProfiles the mapping of the task manager to its TaskManagerProfile
   */
  virtual void gatherProfileData(std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles)  = 0;

  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Prints the profiling data to std::cout
   */
  void printProfile() {
    std::cout << "===================== " << this->getName() << " " << prefix() << " ===================" << std::endl;
    std::cout << "COMPUTE TIME: " << getComputeTime() << " us   WAIT TIME: " << getWaitTime() << " us" << std::endl;

    if (this->getInputConnector() != nullptr) {
      std::cout << "Input connector: ";
      this->getInputConnector()->profileConsume(this->getNumThreads(), true);
    }
//    if (this->getOutputConnector() != nullptr) {
//      std::cout << "Output connector: ";
//      this->getOutputConnector()->profileProduce(this->getNumThreads());
//    }
    this->getTaskFunction()->profileITask();
    std::cout << "-------------------------- " << this->getName() << " (thread: " << this->getThreadId()
              << ") -------------------------- " << std::endl << std::endl;

    this->getTaskFunction()->printProfile();

  }

  /**
   * Sets the task graph communicator
   * @param communicator the task graph communicator
   */
  void setTaskGraphCommunicator(TaskGraphCommunicator *communicator) {
    this->taskGraphCommunicator = communicator;
    this->getTaskFunction()->setTaskGraphCommunicator(this->taskGraphCommunicator);
  }

  /**
   * Sends data packet along task graph communicator
   * @param packet the data packet to communicate
   */
  void sendDataPacket(std::shared_ptr<DataPacket> packet)
  {
    this->taskGraphCommunicator->produceDataPacket(packet);
  }

  /**
   * Updates the address, pipelineID, and number of pipelines for the task manager.
   * @param address the address (or task graph address) of this task manager
   * @param pipelineId the ID for which execution pipeline this task belongs
   * @param numPipelines the number of pipelines that exist for the execution pipeline
   */
  void updateAddressAndPipelines(std::string address, size_t pipelineId, size_t numPipelines) {
    this->numPipelines = numPipelines;
    this->address = address;
    this->pipelineId = pipelineId;
  }

  /**
   * Gets the address of the task manager.
   * Can also be thought of as the address for the task graph that this task belongs too.
   * @return the address
   */
  std::string getAddress() {
    return this->address;
  }

  /**
   * Sets the number of pipelines associated with the TaskManager
   * @param numPipelines the number of pipelines
   */
  void setNumPipelines(size_t numPipelines) {
    this->numPipelines = numPipelines;
    this->getTaskFunction()->setNumPipelines(numPipelines);
  }

  /**
   * Gets the number of pipelines that this task manager belongs too.
   * @return the number of pipelines spawned from the execution pipeline task
   */
  size_t getNumPipelines() { return this->numPipelines; }

  /**
   * Sets the pipeline Id associated with the TaskManager
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
   * Gets the number of threads associated with this TaskManager
   * @return the number of the threads that will execute the TaskManager
   */
  size_t getNumThreads() const { return this->numThreads; }

  /**
   * Sets the alive state for this task manager
   * @param val the value to set, true = alive, false = dead/terminating
   */
  void setAlive(bool val) { this->alive = val; }

  /**
   * Gets whether the TaskManager is alive or not
   * @return whether the TaskManager is alive
   * @retval TRUE if the TaskManager is alive
   * @retval FALSE if the TaskManager is not alive
   */
  bool isAlive() { return this->alive; }

  /**
   * Sets whether this task manager is a start task or not, which will immediately begin executing
   * by sending nullptr data to the underlying task as soon as this task executes.
   * @param val the value to set, true = is a start task, false = not a start task
   * @note Should be set before a task begins executing (attached to a thread)
   */
  void setStartTask(bool val) { this->startTask = val; }

  /**
   * Gets whether this task manager will begin executing immediately with nullptr data or not.
   * @return whether the task manager will start immediately.
   * @retval TRUE if the task manager will begin executing immediately
   * @retval FALSE if the task manager will not begin and wait for its first input data
   */
  bool isStartTask() { return this->startTask; }

  /**
   * Gets whether the task manager is polling for data or not
   * @return whether the task manager is polling or not
   * @retval TRUE if the task manager is polling for data from its input
   * @retval FALSE if the task manager is not polling (waiting) for data from its input
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
  void incTaskComputeTime(int64_t val) { this->taskComputeTime += val; }

  /**
   * Increments the wait time profile value
   * @param val the value to increment by
   */
  void incWaitTime(int64_t val) { this->taskWaitTime += val; }

  /**
   * Shuts down the TaskManager
   */
  void shutdown() {
    HTGS_DEBUG("shutting down: " << this->prefix() << " " << this->getName() << std::endl);
#ifdef USE_NVTX
    nvtxRangeId_t rangeId = this->nvtxProfiler->startRangeShuttingDown();
#endif
    this->getTaskFunction()->shutdown();

#ifdef USE_NVTX
    this->nvtxProfiler->endRangeShuttingDown(rangeId);
#endif
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
    HTGS_DEBUG(prefix() << this->getName() << " input connector: " << getInputConnector() << " output connector: " <<
                   getOutputConnector() << " Details: " << std::endl);
    this->getTaskFunction()->debug();
  }

  /**
 * Gets the name of the ITask with it's pipeline ID
 * @return  the name of the task with the pipeline ID
 */
  std::string getNameWithPipelineId() { return this->getTaskFunction()->getNameWithPipelineId(); }

  /**
 * Gets the dot notation for this TaskManager.
 */
  std::string getDot(int flags) {
    if ((flags & DOTGEN_FLAG_SHOW_ALL_THREADING) != 0) {
      return this->getTaskFunction()->genDot(flags, this->getInputConnector(), this->getOutputConnector());
    } else if (this->threadId == 0) {
      return this->getTaskFunction()->genDot(flags, this->getInputConnector(), this->getOutputConnector());
    } else {
      return "";
    }
  }

  /**
   * Sets the thread id associated with the TaskManager
   * @param id the thread id
   */
  void setThreadId(size_t id) {
    this->threadId = id;
  }

  /**
   * Gets the thread id associated with the TaskManager
   * @return the thread id
   */
  size_t getThreadId() {
    return this->threadId;
  }
  /**
   * Gets the compute time for the task manager, removing the memory wait time.
   * Use getExecuteTime() to get the entire runtime of the task, including wait time.
   * @return the compute time
   * @note Must define the directive PROFILE to enable profiling
   */
  unsigned long long int getComputeTime() {
#ifdef PROFILE
    return taskComputeTime - this->getTaskFunction()->getMemoryWaitTime();
#else
    return 0;
#endif
  }

  /**
   * Gets the total execution time for the task manager, including any waiting for memory within the execute function.
   * @return the total time spent in the execute function.
   */
  unsigned long long int getExecuteTime() {
#ifdef PROFILE
    return taskComputeTime;
#else
    return 0;
#endif
  }

  /**
   * Gets the wait time for the task manager
   * @return the wait time
   * @note Must define the directive PROFILE to enable profiling
   */
  unsigned long long int getWaitTime() {
#ifdef PROFILE
    return taskWaitTime;
#else
    return 0;
#endif
  }

  /**
   * Gets the maximum size the input queue became during execution.
   * @return the maximum input queue size
   * @note Must define the directive PROFILE to enable profiling
   */
  size_t getMaxQueueSize() {
#ifdef PROFILE
    return this->getInputConnector() != nullptr ? this->getInputConnector()->getMaxQueueSize() : 0;
#else
    return 0;
#endif
  }

  /**
   * Resets the profile data for this task.
   */
  void resetProfile() {
    taskComputeTime = 0;
    taskWaitTime = 0;
    if (this->getInputConnector() != nullptr)
    {
      this->getInputConnector()->resetMaxQueueSize();
    }

  }

  /**
   * Gets the task's compute time.
   * @return the compute time in microseconds.
   */
  unsigned long long int getTaskComputeTime()
  {
    return taskComputeTime;
  }

  //! @cond Doxygen_Suppress
  std::string prefix() {
    return std::string(
        "Thread id: " + std::to_string(this->threadId) + " (out of " + std::to_string(this->numThreads)
            + "); Pipeline id " + std::to_string(this->pipelineId) + " (out of " + std::to_string(this->numPipelines) +
            ") Address: " + this->getAddress());
  }


#ifdef USE_NVTX
  void setProfiler(NVTXProfiler *profiler) {
    this->nvtxProfiler = profiler;
  }
  NVTXProfiler *getProfiler() const {
    return nvtxProfiler;
  }

  void releaseProfiler() {
    delete nvtxProfiler;
    nvtxProfiler = nullptr;
  }
#endif
  //! @endcond


 private:

  unsigned long long int taskComputeTime; //!< The total compute time for the task
  unsigned long long int taskWaitTime; //!< The total wait time for the task

  size_t timeout; //!< The timeout time for polling in microseconds
  bool poll; //!< Whether the manager should poll for data

  bool startTask; //!< Whether the task should start immediately
  bool alive; //!< Whether the task is still alive

  size_t threadId; //!< The thread id for the task (set after initialization)
  size_t numThreads; //!< The number of threads spawned for the manager

  size_t pipelineId; //!< The execution pipeline id
  size_t numPipelines; //!< The number of execution pipelines
  std::string address; //!< The address of the task graph this manager belongs too

  TaskGraphCommunicator *taskGraphCommunicator; //!< Task graph communicator
#ifdef USE_NVTX
  NVTXProfiler *nvtxProfiler;
#endif
};

/**
 * @class TaskManagerThread AnyTaskManager.hpp <htgs/task/AnyTaskManager.hpp>
 * @brief Manages a TaskManager that is bound to a thread for execution
 * @details
 * A Runtime will spawn a thread and bind it to the run function
 * within this class. If a Task has more than one threads associated
 * with it, then this class is duplicated one per thread, each with
 * a separate copy of the original TaskManager.
 *
 * @note This class should only be called by the HTGS API
 */
class TaskManagerThread {
 public:
  /**
   * Constructs a TaskManagerThread with a specified AnyTaskManager and atomic number of threads
   * that is shared among all other threads that operate with a copy of the same AnyTaskManager
   * @param threadId the thread Id for the task
   * @param task the task the thread is associated with
   * @param numThreads the number of threads that a task contains
   */
  TaskManagerThread(size_t threadId, AnyTaskManager *task, std::shared_ptr<std::atomic_size_t> numThreads) {
    this->task = task;
    this->terminated = false;
    this->numThreads = numThreads;
    this->task->setRuntimeThread(this);
    this->task->setThreadId(threadId);
    this->numThreadsAfterDecrement = *this->numThreads;
  }

  /**
   * Destructor
   */
  ~TaskManagerThread() {
  }

  /**
   * Executes the task until the underlying Task has been terminated
   * @return status code
   * @retval 0 Completed successfully
   */
  int run(void) {
    // TODO: Remove?
//#ifdef USE_NVTX
//    NVTXProfiler *profiler = new NVTXProfiler();
//    task->setProfiler(profiler);
//#endif

    HTGS_DEBUG("Starting Thread for task : " << task->getName());
    this->task->initialize();
    while (!this->terminated) {
      this->task->executeTask();
    }
    this->task->shutdown();


    if (numThreadsAfterDecrement == 0)
    {
      this->task->terminateConnections();
    }

#ifdef USE_NVTX
    if(hasNoThreadsRemaining())
    {
      this->task->releaseProfiler();
    }
#endif

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
    numThreadsAfterDecrement = current;
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
  std::shared_ptr<std::atomic_size_t> numThreads; //!< The number of total threads managing the TaskManager
  AnyTaskManager *task; //!< The TaskManager that is called from the thread
  size_t numThreadsAfterDecrement;
};

}

#endif //HTGS_ANYTASKMANAGER_HPP
