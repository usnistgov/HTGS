
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file TaskManager.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief Implements a TaskManager that interacts with an ITask and holds the input and output Connector for the ITask
 */
#ifndef HTGS_TASKMANAGER_HPP
#define HTGS_TASKMANAGER_HPP

#include <chrono>
#include <vector>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <sstream>

#include <htgs/core/task/AnyTaskManager.hpp>
#include <htgs/api/ITask.hpp>

namespace htgs {

template<class T, class U>
class ITask;

/**
 * @class TaskManager TaskManager.hpp <htgs/core/task/TaskManager.hpp>
 * @brief Encapsulates an ITask to interact with an ITask's functionality.
 * @details
 * The TaskManager interacts with the TaskManagerThread to process an ITask's input and output data.
 * The core logic of a TaskManager is implemented with the ITask representing the computational and logic functionality.
 *
 * When the TaskManager is ready to be terminated the thread associated with the TaskManager and the output Connector will be notified.
 *
 * @tparam T the input data type for the TaskManager, T must derive from IData.
 * @tparam U the output data type for the TaskManager, U must derive from IData.
 * @note This class should only be called by the HTGS API
 * @note \#define PROFILE to enable profiling.
 */
template<class T, class U>
class TaskManager : public AnyTaskManager {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:
  /**
   * Constructs a TaskManager with an ITask as the task function and specific runtime parameters.
   * @param taskFunction the functionality for the TaskManager
   * @param numThreads the number of threads to operate with the TaskManager
   * @param isStartTask whether the TaskManager is a start task or not (immediately launches the ITask::execute when bound to a thread)
   * @param pipelineId the pipeline Id associated with the TaskManager
   * @param numPipelines the number of pipelines
   * @param address the address of the task graph that owns this task
   */
  TaskManager(ITask<T, U> *taskFunction,
              size_t numThreads,
              bool isStartTask,
              size_t pipelineId,
              size_t numPipelines,
              std::string address) :
      super(numThreads, isStartTask, pipelineId, numPipelines, address),
      inputConnector(nullptr), outputConnector(nullptr), taskFunction(taskFunction), runtimeThread(nullptr) {
    taskFunction->setTaskManager(this);
  }

  /**
   * Constructs a TaskManager with an ITask as the task function and specific runtime parameters.
   * @param taskFunction the functionality for the TaskManager
   * @param numThreads the number of threads to operate with the TaskManager
   * @param isStartTask whether the TaskManager is a start task or not (immediately launches the ITask::execute when bound to a thread)
   * @param poll whether the TaskManager should poll for data
   * @param microTimeoutTime the timeout time in microseconds
   * @param pipelineId the pipeline Id associated with the TaskManager
   * @param numPipelines the number of pipelines
   * @param address the address of the task graph that owns this task
   */
  TaskManager(ITask<T, U> *taskFunction, size_t numThreads, bool isStartTask, bool poll, size_t microTimeoutTime,
              size_t pipelineId, size_t numPipelines, std::string address) : super(numThreads,
                                                                                   isStartTask,
                                                                                   poll,
                                                                                   microTimeoutTime,
                                                                                   pipelineId,
                                                                                   numPipelines,
                                                                                   address),
                                                                             inputConnector(nullptr),
                                                                             outputConnector(nullptr),
                                                                             taskFunction(taskFunction),
                                                                             runtimeThread(nullptr) {
    taskFunction->setTaskManager(this);
  }


  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////// INHERITED FUNCTIONS /////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  ~TaskManager() override {
    delete taskFunction;
    taskFunction = nullptr;
  }

  std::shared_ptr<AnyConnector> getInputConnector() override { return this->inputConnector; }

  std::shared_ptr<AnyConnector> getOutputConnector() override { return this->outputConnector; }

  void initialize() override {
    DEBUG("initializing: " << this->prefix() << " " << this->getName() << std::endl);
    this->taskFunction->initialize(this->getPipelineId(), this->getNumPipelines(), this);
  }

  void setRuntimeThread(TaskManagerThread *runtimeThread) override { this->runtimeThread = runtimeThread; }

  ITask<T, U> *getTaskFunction() override {
    return this->taskFunction;
  }

  AnyTaskManager *copy(bool deep) override {
    ITask<T, U> *iTask = this->taskFunction->copyITask(deep);

    TaskManager<T, U>
        *newTask =
        new TaskManager<T, U>(iTask, this->getNumThreads(), this->isStartTask(), this->isPoll(), this->getTimeout(),
                              this->getPipelineId(), this->getNumPipelines(), this->getAddress());
    if (deep) {
      newTask->setInputConnector(this->getInputConnector());
      newTask->setOutputConnector(this->getOutputConnector());
    }
    return (AnyTaskManager *) newTask;
  }

  void executeTask() override {

    std::shared_ptr<T> data = nullptr;

    DEBUG_VERBOSE(prefix() << "Running task: " << this->getName());

    if (this->isStartTask()) {
      DEBUG_VERBOSE(prefix() << this->getName() << " is a start task");
      this->setStartTask(false);
      auto start = std::chrono::high_resolution_clock::now();

#ifdef WS_PROFILE
      this->sendWSProfileUpdate(StatusCode::EXECUTE);
#endif

      this->taskFunction->executeTask(nullptr);

#ifdef WS_PROFILE
      this->sendWSProfileUpdate(StatusCode::WAITING);
#endif
      auto finish = std::chrono::high_resolution_clock::now();

      this->incTaskComputeTime(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());
      return;
    } else if (this->taskFunction->canTerminate(this->inputConnector)) {

      DEBUG(prefix() << this->getName() << " task function is terminated");
      this->processTaskFunctionTerminated();

      return;
    }
    auto start = std::chrono::high_resolution_clock::now();

#ifdef WS_PROFILE
    this->sendWSProfileUpdate(StatusCode::WAITING);
#endif
    if (this->isPoll())
      data = this->inputConnector->pollConsumeData(this->getTimeout());
    else
      data = this->inputConnector->consumeData();

    auto finish = std::chrono::high_resolution_clock::now();

#if defined (WS_PROFILE) && defined (VERBOSE_WS_PROFILE)
    auto waitTime = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
#endif
    this->incWaitTime(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());

    DEBUG_VERBOSE(prefix() << this->getName() << " received data: " << data << " from " << inputConnector);

    if (data != nullptr || this->isPoll()) {
      start = std::chrono::high_resolution_clock::now();
#ifdef WS_PROFILE
//      sendWSProfileUpdate(this->inputConnector.get(), StatusCode::CONSUME_DATA);
      this->sendWSProfileUpdate(StatusCode::EXECUTE);
#endif
      this->taskFunction->executeTask(data);
      finish = std::chrono::high_resolution_clock::now();

#ifdef WS_PROFILE
      // Produce meta data for task
      std::string metaDataString = this->taskFunction->profileStr();
#ifdef VERBOSE_WS_PROFILE
      // Send compute time and wait time meta
      metaDataString = metaDataString + ";waitTime:" + std::to_string(waitTime.count()) + ";computeTime:" + std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());
#endif
      if (metaDataString != "")
      {
        sendWSMetaProfileUpdate(metaDataString);
      }
#endif

      this->incTaskComputeTime(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());
    }

  }



  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  void gatherProfileData(std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles) override {
#ifdef WS_PROFILE
    if (this->getName() == "WebSocketProfiler")
      return;
#endif
    // Create profile data for this task
    TaskManagerProfile
        *profileData = new TaskManagerProfile(this->getComputeTime(), this->getWaitTime(), this->getMaxQueueSize());
    taskManagerProfiles->insert(std::pair<AnyTaskManager *, TaskManagerProfile *>(this, profileData));

    // Pass gatherProfileData to ITask for further processing
    this->taskFunction->gatherProfileData(taskManagerProfiles);

  }

  /**
   * Sets the input BaseConnector
   * @param connector the input connector
   */
  void setInputConnector(std::shared_ptr<AnyConnector> connector) override {
    if (connector != nullptr)
      this->inputConnector = std::static_pointer_cast<Connector<T>>(connector);
    else
      this->inputConnector = nullptr;

  }

  /**
   * Sets the output BaseConnector
   * @param connector the output connector
   */
  void setOutputConnector(std::shared_ptr<AnyConnector> connector) override {
    if (connector != nullptr)
      this->outputConnector = std::static_pointer_cast<Connector<U>>(connector);
    else
      this->outputConnector = nullptr;
  }

  /**
   * Adds the result data to the output connector
   * @param result the result that is added to the output for this task
   */
  void addResult(std::shared_ptr<U> result) {
    if (this->outputConnector != nullptr) {
      this->outputConnector->produceData(result);
#ifdef WS_PROFILE
      if (result != nullptr)
        sendWSProfileUpdate(this->outputConnector.get(), StatusCode::PRODUCE_DATA);
#endif
    }
  }

 private:

  //! @cond Doxygen_Suppress
  void processTaskFunctionTerminated() {
    // Task is now terminated, so it is no longer alive
    this->setAlive(false);

    // Wake up the threads for this task
    this->getInputConnector()->wakeupConsumer();

    // If there is a runtime thread, then begin termination
    if (this->runtimeThread != nullptr) {
      this->runtimeThread->terminate();

#ifdef WS_PROFILE
      this->sendWSProfileUpdate(StatusCode::SHUTDOWN);
#endif

      // If this is the last thread for this task then close the output
      if (this->runtimeThread->decrementAndCheckNumThreadsRemaining()) {

#ifdef WS_PROFILE
        // Update output connector, this task is no longer producing for it
        this->sendWSProfileUpdate(this->getOutputConnector().get(), StatusCode::DECREMENT);
#endif
        if (this->getOutputConnector() != nullptr) {
          this->getOutputConnector()->producerFinished();

//          if (this->getOutputConnector()->isInputTerminated())
            this->getOutputConnector()->wakeupConsumer();
        }

        auto memManagerConnectorMap = this->getTaskFunction()->getReleaseMemoryEdges();

        DEBUG(prefix() << " " << this->getName() << " Shutting down " << memManagerConnectorMap->size()
                       << " memory releasers");
        for (auto nameManagerPair : *memManagerConnectorMap) {
          DEBUG(prefix() << " " << this->getName() << " Shutting down memory manager: " << nameManagerPair.first);


          std::shared_ptr<AnyConnector> connector = nameManagerPair.second;
#ifdef WS_PROFILE
          // TODO: This might not be necessary
          // Update memory manager connector, this task is no longer producing for it
          this->sendWSProfileUpdate(connector.get(), StatusCode::DECREMENT);
#endif
          connector->producerFinished();

          if (connector->isInputTerminated())
            connector->wakeupConsumer();
        }
      }
    } else {
      if (this->getOutputConnector() != nullptr) {
        this->getOutputConnector()->producerFinished();

        if (this->getOutputConnector()->isInputTerminated()) {
          this->getOutputConnector()->wakeupConsumer();
        }
      }
    }
  }

#ifdef WS_PROFILE
  void sendWSProfileUpdate(StatusCode code)
  {
    if (this->getName() == "WebSocketProfiler")
      return;
    std::shared_ptr<ProfileData> updateStatus(new ChangeStatusProfile(this->getTaskFunction(), code));
    std::shared_ptr<DataPacket> dataPacket(new DataPacket(this->getName(), this->getAddress(), "WebSocketProfiler", "0", updateStatus));
    this->sendDataPacket(dataPacket);
  }

  void sendWSProfileUpdate(void * addr, StatusCode code)
  {
    if (this->getName() == "WebSocketProfiler")
      return;
    std::shared_ptr<ProfileData> updateStatus(new ChangeStatusProfile(addr, code));
    std::shared_ptr<DataPacket> dataPacket(new DataPacket(this->getName(), this->getAddress(), "WebSocketProfiler", "0", updateStatus));
    this->sendDataPacket(dataPacket);
  }

  void sendWSMetaProfileUpdate(std::string metaData)
  {
    if (this->getName() == "WebSocketProfiler")
      return;
    std::shared_ptr<ProfileData> updateStatus(new UpdateMetadataProfile(this->getTaskFunction(), metaData));
    std::shared_ptr<DataPacket> dataPacket(new DataPacket(this->getName(), this->getAddress(), "WebSocketProfiler", "0", updateStatus));
    this->sendDataPacket(dataPacket);
  }
#endif

  typedef AnyTaskManager super;
  //! @endcond

  std::shared_ptr<Connector<T>> inputConnector; //!< The input connector for the manager (queue to get data from)
  std::shared_ptr<Connector<U>> outputConnector; //!< The output connector for the manager (queue to send data)
  ITask<T, U> *taskFunction; //!< The task that is managed by the manager
  TaskManagerThread *runtimeThread; //!< The thread that is executing this task's runtime
};
}

#endif //HTGS_TASKMANAGER_HPP
