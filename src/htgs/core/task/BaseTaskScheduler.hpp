
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file BaseTaskScheduler.hpp
 * @author Timothy Blattner
 * @date Nov 18, 2015
 *
 * @brief Implements the parent class for a Task to remove the template arguments and the BaseTaskRuntimeThread to attach a thread to a Task.
 */
#ifndef HTGS_BASETASKSCHEDULER_H
#define HTGS_BASETASKSCHEDULER_H

class BaseITask;

#include <atomic>
#include <memory>
#include "../graph/BaseConnector.hpp"
#include "../task/BaseITask.hpp"
#include "../../debug/debug_message.h"
#include "BaseITask.hpp"

namespace htgs {
template<class T, class U>
class ITask;

class BaseConnector;

class BaseTaskSchedulerRuntimeThread;

class BaseITask;

/**
 * @class BaseTaskScheduler BaseTaskScheduler.hpp <htgs/core/task/BaseTaskScheduler.hpp>
 * @brief The parent class for a Task that removes the template arguments.
 * @details
 * The BaseTaskScheduler provides access to functionality that does not require template arguments and
 * allows storage of a Task.
 *
 * @note This class should only be called by the HTGS API
 */
class BaseTaskScheduler {
 public:
  /**
   * Destructor
   */
  virtual ~BaseTaskScheduler() { };

  /**
   * Executes the Task
   */
  virtual void executeTask() {
    std::cerr << "Called BaseTaskScheduler 'executeTask' virtual function" << std::endl;
    throw std::bad_function_call();
  };

  /**
   * Initializes the Task
   */
  virtual void initialize() {
    std::cerr << "Called BaseTaskScheduler 'initialize' virtual function" << std::endl;
    throw std::bad_function_call();
  };

  /**
   * Shuts down the Task
   */
  virtual void shutdown() {
    std::cerr << "Called BaseTaskScheduler 'shutdown' virtual function" << std::endl;
    throw std::bad_function_call();
  };

  /**
   * Copies the Task
   * @param deep whether a deep copy is required
   * @return the Task copy
   */
  virtual BaseTaskScheduler *copy(bool deep) {
    std::cerr << "Called BaseTaskScheduler 'copy' virtual function" << std::endl;
    throw std::bad_function_call();
  };

  /**
   * Gets the input BaseConnector
   * @return the input connector
   */
  virtual std::shared_ptr<BaseConnector> getInputBaseConnector() {
    std::cerr << "Called BaseTaskScheduler 'getInputBaseConnector' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the output BaseConnector
   * @return the output connector
   */
  virtual std::shared_ptr<BaseConnector> getOutputBaseConnector() {
    std::cerr << "Called BaseTaskScheduler 'getOutputBaseConnector' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Sets the input BaseConnector
   * @param connector the input connector
   */
  virtual void setInputConnector(std::shared_ptr<BaseConnector> connector) {
    std::cerr << "Called BaseTaskScheduler 'setInputConnector' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Sets the output BaseConnector
   * @param connector the output connector
   */
  virtual void setOutputConnector(std::shared_ptr<BaseConnector> connector) {
    std::cerr << "Called BaseTaskScheduler 'setOutputConnector' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Provides debug output
   */
  virtual void debug() {
    std::cerr << "Called BaseTaskScheduler 'debug' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the name of the Task
   * @return the name of the Task
   */
  virtual std::string getName() {
    std::cerr << "Called BaseTaskScheduler 'getName' virtual function" << std::endl;
    throw std::bad_function_call();
  }
  /**
   * Gets the name of the ITask with it's pipeline ID
   * @return  the name of the task with the pipeline ID
   */
  virtual std::string getNameWithPipID() {
    std::cerr << "Called BaseTaskScheduler 'getNameWithPipID' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the ITask function associated with the Task
   * @return the ITask
   */
  virtual BaseITask *getTaskFunction() {
    std::cerr << "Called BaseTaskScheduler 'getTaskFunction' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets whether the Task is alive or not
   * @return whether the Task is alive
   * @retval TRUE if the Task is alive
   * @retval FALSE if the Task is not alive
   */
  virtual bool isAlive() {
    std::cerr << "Called BaseTaskScheduler 'isAlive' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Sets the pipeline Id associated with the Task
   * @param id the pipeline Id
   */
  virtual void setPipelineId(int id) {
    std::cerr << "Called BaseTaskScheduler 'setPipelineId' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Sets the number of pipelines associated with the Task
   * @param numPipelines the number of pipelines
   */
  virtual void setNumPipelines(int numPipelines) {
    std::cerr << "Called BaseTaskScheduler 'setNumPipelines' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Adds a Connector for a Task that is in an ExecutionPipeline
   * Each Connector added represents one of the other Connectors that is attached
   * to a copy of this Task that is within the same ExecutionPipeline.
   * @param pipelineId the pipeline Id
   * @param connector the connector to add
   */
  virtual void addPipelineConnector(int pipelineId, std::shared_ptr<BaseConnector> connector) {
    std::cerr << "Called BaseTaskScheduler 'addPipelineConnector' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Adds the input Connector for this task to the pipeline connector list.
   * Each Connector added represents one of the other Connectors that is attached
   * to a copy of this Task that is within the same ExecutionPipeline.
   * @param pipelineId the pipeline Id
   */
  virtual void addPipelineConnector(int pipelineId) {
    std::cerr << "Called BaseTaskScheduler 'addPipelineConnector' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the number of threads associated with this Task
   * @return the number of the threads that will execute the Task
   */
  virtual int getNumThreads() const {
    std::cerr << "Called BaseTaskScheduler 'getNumThreads' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Sets the thread that is executing this Task
   * @param runtimeThread the thread that is executing the task
   */
  virtual void setRuntimeThread(BaseTaskSchedulerRuntimeThread *runtimeThread) {
    std::cerr << "Called BaseTaskScheduler 'setRuntimeThread' virtual function" << std::endl;
    throw std::bad_function_call();
  }
  /**
   * Sets the thread id associated with the Task
   * @param id the thread id
   */
  virtual void setThreadId(int id) {
    std::cerr << "Called BaseTaskScheduler 'setThreadId' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the dot notation for this task.
   * @param flags gen dot flags
   */
  virtual std::string getDot(int flags) {
    std::cerr << "Called BaseTaskScheduler 'getDot' virtual function" << std::endl;
    throw std::bad_function_call();
  }


#ifdef PROFILE
  virtual long long int getComputeTime() { return 0; }
  virtual long long int getWaitTime() { return 0; }
  virtual int getMaxQueueSize() { return 0;}
  virtual void gatherComputeTime(std::unordered_multimap<std::string, long long int> *mmap) {}
  virtual void gatherWaitTime(std::unordered_multimap<std::string, long long int> *mmap) {}
  virtual void gatherMaxQSize(std::unordered_multimap<std::string, int> *mmap) {}
  virtual std::string genDotProfile(int flags, std::unordered_map<std::string, double> *mmap, std::string desc, std::unordered_map<std::string, std::string> *colorMap) { return "";}
#endif


};

/**
 * @class BaseTaskSchedulerRuntimeThread BaseTaskScheduler.hpp <htgs/task/BaseTaskScheduler.hpp>
 * @brief Manages a BaseTaskScheduler that is bound to a thread for execution
 * @details
 * A Runtime will spawn a thread and bind it to the run function
 * within this class. If a Task has more than one threads associated
 * with it, then this class is duplicated one per thread, each with
 * a separate copy of the original BaseTaskScheduler.
 *
 * @note This class should only be called by the HTGS API
 */
class BaseTaskSchedulerRuntimeThread {
 public:
  /**
   * Constructs a BaseTaskSchedulerRuntimeThread with a specified BaseTaskScheduler and atomic number of threads
   * that is shared among all other threads that operate with a copy of the same BaseTaskScheduler
   * @param threadId the thread Id for the task
   * @param task the task the thread is associated with
   * @param numThreads the number of threads that a task contains
   */
  BaseTaskSchedulerRuntimeThread(int threadId, BaseTaskScheduler *task, std::shared_ptr<std::atomic_int> numThreads) {
    this->task = task;
    this->terminated = false;
    this->numThreads = numThreads;
    this->task->setRuntimeThread(this);
    this->task->setThreadId(threadId);
  }

  /**
   * Destructor
   */
  ~BaseTaskSchedulerRuntimeThread() {
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
  int getThreadsRemaining() { return *this->numThreads; }

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
    int current = this->numThreads->fetch_sub(1)-1;
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
  std::shared_ptr<std::atomic_int> numThreads; //!< The number of total threads managing the TaskScheduler
  BaseTaskScheduler *task; //!< The TaskScheduler that is called from the thread
};
}

#endif //HTGS_BASETASKSCHEDULER_H
