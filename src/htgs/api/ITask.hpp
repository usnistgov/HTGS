
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file ITask.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief An interface to process input data and forward results within a TaskGraph
 * @details
 */
#ifndef HTGS_ITASK_HPP
#define HTGS_ITASK_HPP


#include <functional>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <list>
#include <assert.h>
#include <sstream>
#include <htgs/core/graph/Connector.hpp>
#include <htgs/core/task/TaskManager.hpp>

namespace htgs {

template <class T, class U>
class TaskManager;

/**
 * @class ITask ITask.hpp <htgs/api/ITask.hpp>
 * @brief An interface to process input data and forward results within a TaskGraph
 * @details
 *
 * To use the ITask, a new class inherits ITask and defines the pure virtual functions. The ITask is
 * then connected into a TaskGraph, which is bound to a Runtime. The ITask contains metadata that describes
 * threading and scheduling rules. Using this metadata, the Runtime spawns threads as a pool. Each thread
 * is bound to a separate instance of the ITask, which is generated through the copy function. 
 *
 * The purpose of this interface is to provide the functions necessary to represent computational and logical operations
 * for algorithms, which are added to a TaskGraph.
 * Custom behavior for an ITask can be implemented, as demonstrated with other classes that derive from ITask; i.e.,
 * Bookkeeper, ExecutionPipeline, and ICudaTask.
 *
 * An ITask should represent some component of an algorithm, such that multiple threads can
 * concurrently process and stream data through a TaskGraph. The main pieces that impact the performance
 * are: (1) Memory requirements, (2) Data dependencies, and (3) Computational complexity.
 *
 * There are two methods for handling memory.
 *
 * The first type of memory for an ITask is local memory. This
 * type should be allocated in the initialize() function and freed in the shutdown() function.
 * It is duplicated (one per thread) and should be local to that thread only.
 *
 * The second type of memory is shared memory, which can be used by other tasks in a TaskGraph. One ITask is responsible
 * for getting memory, while another ITask is responsible for releasing memory.
 * This memory is managed by an external MemoryManager, which allocates the memory, connects the getter
 * and the releaser, and frees the memory once the TaskGraph is finished. Use the TaskGraph::addMemoryManagerEdge
 * to attach shared memory between two ITasks. The memory that is acquired should be incorporated into the
 * output data of the ITask and forwarded until it is released.
 *
 * An ITask can get and release memory using the memGet() and memRelease() routines, respectively. If there are cases where
 * the ITask getting or releasing memory may not a memory edge, then that task can use the hasMemGetter and
 * has memReleaser routines to verify the edge exists.
 *
 * If there are multiple computational ITasks within a TaskGraph, the number of threads processing each ITask
 * should be determined based on the workload of each ITask with the aim of reducing the wait period for every ITask (if possible).
 * By doing so, the overall compute time of a TaskGraph can be evenly distributed. The number of threads in use for computation
 * should not exceed the number of logical cores on a system.
 *
 * TODO: Rework comments
 * There are two types of initialize functions. The basic ITask::initialize(int pipelineId, int numPipelines) is
 * the most commonly used variant. The ITask::initialize(int pipelineId, int numPipeline, TaskManager<T, U> *ownerTask,
 * std::shared_ptr<ConnectorVector> pipelineConnectorList) can be used for more advanced operations such as processing data from
 * other execution pipelines using the pipelineConnectorList.
 *
 * Example Implementation:
 * @code
 * class ReadTask : public htgs::ITask<Data1, Data2>
 * {
 *  public:
 *   ReadTask(int numThreads, long memorySize) : ITask(numThreads), memorySize(memorySize) {}
 *
 *   virtual void initialize(int pipelineId, int numPipelines)
 *   {
 *     // This memory will be allocated multiple times (one for each thread that is bound to the ReadTask ITask)
 *     reusableMemory = new double[memorySize];
 *   }
 *
 *   virtual void shutdown() { delete [] reusableMemory; }
 *
 *   virtual void executeTask(std::shared_ptr<Data1> data)
 *   {
 *     ...
 *     // Shared memory getter
 *     std::shared_ptr<htgs::MemoryData<int *>> readBuffer = this->memGet<int *>("readMemory", new ReleaseCountRule(1));
 *     readData(data->getFile(), readBuffer->get());
 *
 *     // Shared memory release example
 *     if (this->hasMemReleaser("otherMemory")
 *     	this->memRelease("otherMemory", data->getMemory());
 *
 *     addResult(new Data2(readBuffer));
 *     ...
 *   }
 *
 *   virtual std::string getName() { return "ReadTask"; }
 *
 *   virtual htgs::ITask<Data1, Data2> *copy() { return new ReadTask(this->getNumThreads(), memorySize); }
 *
 *   // Optional. Default will check inputConnector->isInputTerminated();
 *   virtual bool isTerminated(std::shared_ptr<htgs::BaseConnector> inputConnector) { return inputConnector->isInputTerminated(); }
 *
 *  private:
 *   // reusableMemory is not used in computation, but shows an example of local memory allocation
 *   double *reusableMemory;
 *   long memorySize;
 * }
 * @endcode
 *
 * Usage Example:
 * @code
 * int numThreadsPreprocess = 1;
 * int numThreadsRead = 2;
 * int numThreadsMultiply = 10;
 *
 * htgs::TaskGraph<htgs::VoidData, htgs::VoidData> *taskGraph = new htgs::TaskGraph<htgs::VoidData, htgs::VoidData>();
 *
 * PreProcessTask *preProcTask = new PreProcessTask(numThreadsPreprocess);
 * ReadTask *readTask = new ReadTask(numThreadsRead);
 * MultiplyTask *mulTask = new MultiplyTask(numThreadsMultiply);
 *
 * // Add tasks to task graph (each task must be added before using addMemoryManagerEdge)
 * taskGraph->addEdge(preProcTask, readTask);
 * taskGraph->addEdge(readTask, mulTask);
 *
 * // Add memory edges. The types for the allocator must match the type specified when an ITask uses memGet
 * // Memory pool size is specified based on algorithm scheduling and memory release rules.
 * int otherMemoryPoolSize = 100;
 * int readMemoryPoolSize = 200;
 *
 * // Creates the memory edge "otherMemory" with preProcTask as the getter, and readTask as the releaser
 * taskGraph->addMemoryManagerEdge("otherMemory", preProcTask, readTask, new OtherMemoryAllocator(), otherMemoryPoolSize);
 *
 * // Creates the memory edge "readMemory" with readTask as the getter, and mulTask as the releaser
 * taskGraph->addMemoryManagerEdge("readMemory", readTask, mulTask, new ReadMemoryAllocator(), readMemoryPoolSize);
 *
 * htgs::Runtime *executeGraph = new htgs::Runtime(taskGraph);
 *
 * // Launches threads and binds them to ITasks
 * executeGraph->executeAndWaitForRuntime();
 * @endcode
 *
 * @tparam T the input data type for the ITask, T must derive from IData.
 * @tparam U the output data type for the ITask, U must derive from IData.
 */
template<class T, class U>
class ITask: public AnyITask {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:

  /**
   * Creates an ITask with number of threads equal to 1.
   */
  ITask() : super() { }

  /**
   * Constructs an ITask with a specified number of threads.
   * @param numThreads the number of threads associated with this ITask
   */
  ITask(size_t numThreads) : super(numThreads) {  }

  /**
   * Constructs an ITask with a specified number of threads as well as additional scheduling options.
   * @param numThreads the number of threads associated with this ITask
   * @param isStartTask whether this ITask starts executing immediately and passes nullptr to executeTask()
   * @param poll whether this task will poll for data, if the timeout period expires, nullptr is passed to executeTask()
   * @param microTimeoutTime the timeout period for checking for data
   * @note If the ITask is declared as a start task or is polling, then executeTask() should properly handle nullptr data
   */
  ITask(size_t numThreads, bool isStartTask, bool poll, size_t microTimeoutTime) : super(numThreads, isStartTask, poll, microTimeoutTime){ }


  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////// VIRTUAL FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  virtual ~ITask() override { }

  virtual void initialize() override {}

  /**
 * Pure virtual function that is called when an ITask's thread is to execute on data
 * @param data the data to be executed
 * @note To send output data use addResult()
 * @note If the ITask is a start task or is polling, data might be nullptr
 */
  virtual void executeTask(std::shared_ptr<T> data) = 0;

  virtual bool canTerminate(std::shared_ptr<AnyConnector> inputConnector)  override {
    return inputConnector->isInputTerminated();
  }

  virtual void shutdown() override {}

  virtual std::string getName() override {
    return "UnnamedITask";
  }

  virtual std::string getDotLabelName() override {
    return this->getName();
  }

  virtual std::string getDotShapeColor() override {
    return "black";
  }

  virtual std::string getDotShape() override {
    return "box";
  }


  virtual ITask<T, U> *copy() = 0;

  virtual size_t getNumGraphsSpawned() { return 0; }

  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Copies the ITask (including a copy of all memory edges)
   * @param deep whether to do a deep copy and copy the memory managers as well
   * @return the copy of the ITask
   */
  ITask<T, U> *copyITask(bool deep) override
  {
    ITask<T,U> *iTaskCopy = copy();

    if (deep)
      copyMemoryEdges(iTaskCopy);

    return iTaskCopy;
  }

  /**
   * Adds results to the output list to be sent to the next connected ITask in a TaskGraph
   * @param result the result data to be passed
   */
  void addResult(std::shared_ptr<U> result) {
    this->ownerTask->addResult(result);
  }

  /**
   * Adds results to the output list to be sent to the next connected ITask in a TaskGraph.
   * The result pointer will be wrapped into a shared smart pointer and then placed in the output list.
   * @param result the result data to be passed
   */
  void addResult(U *result) {
   this->ownerTask->addResult(std::shared_ptr<U>(result));
  }

  /**
   * Function that is called when an ITask is being initialized by it's owner thread.
   * This initialize function contains the TaskManager associated with the ITask.
   * @param pipelineId the pipelineId, only used if the ITask is inside of an ExecutionPipeline
   * @param numPipeline the number of pipelines, only used if the ITask is inside of an ExecutionPipeline
   * @param ownerTask the owner Task for this ITask
   * ICudaTask's in an execution pipeline
   */
  void initialize(size_t pipelineId, size_t numPipeline, TaskManager<T, U> *ownerTask) {
    this->ownerTask = ownerTask;
    super::initialize(pipelineId, numPipeline);
  }

  std::string inTypeName() override final
  {
    int status;
    char *realName = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    std::string ret(realName);

    free(realName);

    return ret;

  }

  std::string outTypeName() override final
  {
    int status;
    char *realName = abi::__cxa_demangle(typeid(U).name(), 0, 0, &status);
    std::string ret(realName);

    free(realName);

    return ret;

  }

  std::string getAddress() override final
  {
    return ownerTask->getAddress();
  }

  /**
   * Sets the owner task manager for this ITask
   * @param ownerTask the task manager that owns this ITask
   */
  void setTaskManager(TaskManager<T, U> *ownerTask)
  {
    this->ownerTask = ownerTask;
  }

  /**
   * Gets the owner task manager for this ITask
   * @return the owner task manager
   */
  TaskManager<T, U> *getOwnerTaskManager()
  {
    return this->ownerTask;
  }

  virtual void gatherProfileData(std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles) { }

 private:

  typedef AnyITask super;

  TaskManager<T, U> *ownerTask; //!< The owner task for this ITask


};

}


#endif //HTGS_ITASK_HPP
