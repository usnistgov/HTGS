
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.
/**
 * @file TaskGraphConf.hpp
 * @author Timothy Blattner
 * @date Nov 18, 2015
 *
 * @brief Implements the task graph configuration class responsible for managing ITask connections.
 * @details
 */
#ifndef HTGS_TASKGRAPHCONF_HPP
#define HTGS_TASKGRAPHCONF_HPP

#include <htgs/core/graph/edge/ProducerConsumerEdge.hpp>
#include <htgs/core/graph/edge/RuleEdge.hpp>
#include <htgs/core/graph/edge/MemoryEdge.hpp>
#include <htgs/core/comm/TaskGraphCommunicator.hpp>
#include <htgs/core/graph/profile/TaskGraphProfiler.hpp>

#ifdef USE_CUDA
#include <cuda.h>
#include <htgs/core/memory/CudaMemoryManager.hpp>
#endif

#ifdef WS_PROFILE
#include "../../../WebSocketProfiler.hpp"
#endif

namespace htgs {

/**
 * @class TaskGraphConf TaskGraphConf.hpp <htgs/api/TaskGraphConf.hpp>
 * @brief Manages a group of connected ITasks and their connections.
 * @details
 * Each ITask that is added into the TaskGraphConf is stored in the TaskGraphConf's metadata
 * to allow for quick copying using copy().
 *
 * The main methods for adding each ITask into the graph are
 * addEdge(), addRuleEdge(), addMemoryManagerEdge(),
 * addCudaMemoryManagerEdge(), setGraphConsumerTask(), and addGraphProducerTask()
 *
 * When using these methods, the TaskGraphConf builds a TaskManager, which manages an ITask.
 * Parameters for customizing the thread pool, polling abilities, etc., are specified in the ITask constructors:
 * ITask::ITask()
 *
 * Special memory edge functions are provided. For normal CPU memory edges use:
 * addMemoryManagerEdge()
 *
 * For Cuda memory use:
 * addCudaMemoryManagerEdge()
 *
 * Every TaskGraphConf has an input and output type (T and U). If a TaskGraph does not have an input or output type, then
 * the data type can be specified as VoidData. There can be only one task consuming data from the graph. If multiple tasks
 * need to process data from the input, then add a bookkeeper as the first task and rules to distribute data. There can be
 * any number of tasks producing output data for the graph.
 *
 * To add data into the input of a TaskGraph use the produceData() function. Once finished producing data for the graph, use
 * the finishedProducingData() function to indicate a data input stream is
 * is closing. If additional data streams are added as input for the graph, then use
 * the incrementGraphProducer() function. By default a task graph starts with one producer for the graph for the main thread,
 * if there are no producers (such as the first task in the graph begins processing immediately), then call finishedProducingData().
 *
 * To process the output of a TaskGraph use the consumeData() function. To determine if data is no longer being
 * produced by a TaskGraph use the isOutputTerminated() function. The output of the consumeData function could produce
 * nullptr data when the graph is closing.
 *
 * Example Usage:
 * @code
 * htgs::TaskGraphConf<MatrixBlockRequest, MatrixBlockRequest> *taskGraph = new htgs::TaskGraphConf<MatrixBlockRequest, MatrixBlockRequest>();
 *
 * int numLoadThreads = 2;
 * int numMulThreads = 20;
 *
 * LoadMatrixTask *loadMatrixTask = new LoadMatrixTask(numLoadThreads, blockSize, width, height);
 * ScalarMultiplyTask *scalMulTask = new ScalarMultiplyTask(numMulThreads, blockSize, width, height);
 * htgs::Bookkeeper<MatrixBlockData> *bkTask = new htgs::Bookkeeper<MatrixBlockData>();
 *
 * MatrixLoadRule *loadRule = new MatrixLoadRule(width/blockSize, height/blockSize);
 *
 * // Add tasks to graph
 * taskGraph->addEdge(loadMatrixTask, bkTask);
 * taskGraph->addRuleEdge(bkTask, loadRule, scalMulTask);
 *
 * // Add memory edges
 * MatrixAllocator *matrixAlloc = new MatrixAllocator(blockSize, blockSize);
 * int poolSize = 50;
 * taskGraph->addMemoryManagerEdge("MatrixA", loadMatrixTask, matrixAlloc, 50);
 * taskGraph->addMemoryManagerEdge("MatrixB", loadMatrixTask, matrixAlloc, 50);
 *
 * // Setup graph input/output
 * taskGraph->setGraphConsumerTask(loadMatrixTask);
 * taskGraph->addGraphProducerTask(scalMulTask);
 *
 * // Setup runtime and execute
 * htgs::TaskGraphRuntime *runtime = new htgs::TaskGraphRuntime(taskGraph);
 * runtime->executeRuntime();
 *
 * // Add input to graph
 * for (blockRow = 0; blockRow < blockHeight; blockRow++)
 * {
 *   for(blockCol = 0; blockCol < blockWidth; blockCol++)
 *   {
 *     // Request to multiply BlockedA[blockRow, blockCol] .* BlockedB[blockRow, blockCol]
 *     taskGraph->produceData(new MatrixBlockRequest(blockRow, blockCol, "MatrixA"));
 *     taskGraph->produceData(new MatrixBlockRequest(blockRow, blockCol, "MatrixB"));
 *   }
 * }
 *
 * // Indicate finished producing data
 * taskGraph->finishedProducingData();
 *
 * // Process taskGraph output
 * while (!taskGraph->isOutputTerminated())
 * {
 *   std::shared_ptr<MatrixBlockRequest> mbr = taskGraph->consumeData();
 *   if (mbr != nullptr)
 *   {
 *     // ... apply post-processing
 *   }
 * }
 *
 * runtime->waitForRuntime();
 * @endcode
 *
 * @tparam T the input data type for the TaskGraph, T must derive from IData.
 * @tparam U the output data type for the TaskGraph, U must derive from IData.
 */
template<class T, class U>
class TaskGraphConf : public AnyTaskGraphConf {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:

  /**
   * Constructs a TaskGraph
   */
  TaskGraphConf() : AnyTaskGraphConf(0, 1, "") {
    this->input = std::shared_ptr<Connector<T>>(new Connector<T>());
    this->output = std::shared_ptr<Connector<U>>(new Connector<U>());

    this->input->incrementInputTaskCount();
    this->graphConsumerTaskManager = nullptr;
    this->graphProducerTaskManagers = new std::list<AnyTaskManager *>();

    this->edges = new std::list<EdgeDescriptor *>();

    this->taskConnectorCommunicator = new TaskGraphCommunicator(nullptr, this->getAddress());

#ifdef WS_PROFILE
    // Create web socket profiler task
    WebSocketProfiler *profileTask = new WebSocketProfiler();

    // Create input connector and task manager
    std::shared_ptr<Connector<ProfileData>> wsConnector(new Connector<ProfileData>());
    this->wsProfileTaskManager= new TaskManager<ProfileData, VoidData>(profileTask,
                                                                       profileTask->getNumThreads(),
                                                                       profileTask->isStartTask(),
                                                                       profileTask->isPoll(),
                                                                       profileTask->getMicroTimeoutTime(),
                                                                       0,
                                                                       1,
                                                                       "0");

    this->wsProfileTaskManager->setInputConnector(wsConnector);

    // Add task to communicator
    TaskNameConnectorPair pair("0:" + this->wsProfileTaskManager->getName(), wsConnector);
    this->taskConnectorCommunicator->addTaskNameConnectorPair(pair);
#endif

  }

  /**
   * Constructs a TaskGraph
   * @param pipelineId the pipelineId for this graph
   * @param numPipelines the number of pipelines for the graph
   * @param baseAddress the base address for the task graph to build upon for multiple levels of execution pipelines
   * @param parentCommunicator the parent task graph communicator
   */
  TaskGraphConf(size_t pipelineId,
                size_t numPipelines,
                std::string baseAddress,
                TaskGraphCommunicator *parentCommunicator
#ifdef WS_PROFILE
                , TaskManager<ProfileData, VoidData> *wsProfileTaskManager
#endif
  ) : AnyTaskGraphConf(pipelineId, numPipelines, baseAddress) {
    this->input = std::shared_ptr<Connector<T>>(new Connector<T>());
    this->output = std::shared_ptr<Connector<U>>(new Connector<U>());

    this->input->incrementInputTaskCount();

    graphConsumerTaskManager = nullptr;
    graphProducerTaskManagers = new std::list<AnyTaskManager *>();

    edges = new std::list<EdgeDescriptor *>();

    taskConnectorCommunicator = new TaskGraphCommunicator(parentCommunicator, this->getAddress());

#ifdef WS_PROFILE
    this->wsProfileTaskManager = wsProfileTaskManager;
    this->wsProfileThread = nullptr;
#endif

  }

  /**
   * Destructor, handles releasing all ITask memory that is managed by the TaskGraph.
   */
  ~TaskGraphConf() override {
    for (auto edge : *edges) {
      if (edge != nullptr) {
        delete edge;
        edge = nullptr;
      }
    }
    delete edges;
    edges = nullptr;

#ifdef WS_PROFILE
    if (wsProfileThread != nullptr)
    {
      this->wsProfileTaskManager->getInputConnector()->producerFinished();
      this->wsProfileTaskManager->getInputConnector()->wakeupConsumer();
      this->wsProfileThread->join();
    }
#endif

    if (taskConnectorCommunicator != nullptr)
      taskConnectorCommunicator->terminateGracefully();

    delete taskConnectorCommunicator;
    taskConnectorCommunicator = nullptr;

    delete graphProducerTaskManagers;
    graphProducerTaskManagers = nullptr;
  }

  AnyTaskGraphConf *copy() override {
    return copy(this->getPipelineId(), this->getNumPipelines());
  }

  /**
   * Creates a mirror copy of the TaskGraph with the specified pipelineId and number of pipelines
   * @param pipelineId the pipeline Id
   * @param numPipelines the number of pipelines
   * @return the copy of the task graph
   */
  TaskGraphConf<T, U> *copy(size_t pipelineId, size_t numPipelines) {
    return copy(pipelineId, numPipelines, nullptr, nullptr, this->getAddress(), nullptr);
  }

  /**
   * Creates a mirror copy of the TaskGraph with the specified pipelineId and number of pipelines, and updates the input
   * and output connectors for the graph copy.
   * @param pipelineId the pipeline Id
   * @param numPipelines the number of pipelines
   * @param input the input connector to be used for the graph's input
   * @param output the output connector to be used for the graph's output
   * @param baseAddress the base address for the task graph to build upon for multiple levels of execution pipelines
   * @param parentCommunicator the parent task graph communicator
   * @return the copy of the task graph
   */
  TaskGraphConf<T, U> *copy(size_t pipelineId,
                            size_t numPipelines,
                            std::shared_ptr<Connector<T>> input,
                            std::shared_ptr<Connector<U>> output,
                            std::string baseAddress,
                            TaskGraphCommunicator *parentCommunicator) {
#ifdef WS_PROFILE
    TaskGraphConf<T, U> *graphCopy = new TaskGraphConf<T, U>(pipelineId, numPipelines, baseAddress, parentCommunicator, this->wsProfileTaskManager);
#else
    TaskGraphConf<T, U> *graphCopy = new TaskGraphConf<T, U>(pipelineId, numPipelines, baseAddress, parentCommunicator);
#endif

    // Copy the tasks to form lookup between old ITasks and new copies
    graphCopy->copyTasks(this->getTaskManagers());

    if (input != nullptr) {
      graphCopy->setInputConnector(input);
    }

    if (output != nullptr) {
      graphCopy->setOutputConnector(output);
    }

    // Copy the graph producer and consumer tasks
    graphCopy->copyAndUpdateGraphConsumerTask(this->graphConsumerTaskManager);
    graphCopy->copyAndUpdateGraphProducerTasks(this->graphProducerTaskManagers);

    for (EdgeDescriptor *edgeDescriptor : *edges) {
      // Copy the edge, using the graph copy as a reference for where to get task copies
      EdgeDescriptor *edgeCopy = edgeDescriptor->copy(graphCopy);

      // Apply the edge on the graph copy
      edgeCopy->applyEdge(graphCopy);

      graphCopy->addEdgeDescriptor(edgeCopy);
    }

    return graphCopy;
  }

  /**
   * Adds an edge to the graph, where one task produces data for a consumer task
   * @tparam V the input type for the producer task
   * @tparam W the output/input types for the producer/consumer tasks
   * @tparam X the output type for the consumer task
   * @param producer the task that is producing data
   * @param consumer the task that consumes the data from the producer task
   */
  template<class V, class W, class X>
  void addEdge(ITask<V, W> *producer, ITask<W, X> *consumer) {
    auto pce = new ProducerConsumerEdge<V, W, X>(producer, consumer);
    pce->applyEdge(this);
    this->addEdgeDescriptor(pce);
  }

  /**
   * Creates a rule edge that is managed by a bookkeeper
   * @tparam V the input type for the bookkeeper and rule
   * @tparam IRuleType the IRule that determines when to produce data for the edge (must match the input types of both the bookkeeper and the consumer task)
   * @tparam W the output/input type for the rule/consumer task
   * @tparam X the output type for the consumer task
   * @param bookkeeper the bookkeeper task that manages this edge
   * @param rule the rule that determines when to produce data for the edge
   * @param consumer the consumer of the rule
   * @note Use this function if the rule connecting the  bookkeeper and consumer are shared among multiple graphs that you create.
   */
  template<class V, class IRuleType, class W, class X>
  void addRuleEdge(Bookkeeper<V> *bookkeeper, std::shared_ptr<IRuleType> rule, ITask<W, X> *consumer) {
    static_assert(std::is_base_of<IRule<V, W>, IRuleType>::value,
                  "Type mismatch for IRule<V, W>, V must match the input type of the bookkeeper and W must match the input type of the consumer!");
    std::shared_ptr<IRule<V, W>> ruleCast = std::static_pointer_cast<IRule<V, W>>(rule);
    auto re = new RuleEdge<V, W, X>(bookkeeper, ruleCast, consumer);
    re->applyEdge(this);
    this->addEdgeDescriptor(re);
  }

  /**
 * Creates a rule edge that is managed by a bookkeeper
 * @tparam V the input type for the bookkeeper and rule
 * @tparam W the output/input type for the rule/consumer task
 * @tparam X the output type for the consumer task
 * @param bookkeeper the bookkeeper task that manages this edge
 * @param iRule the rule that determines when to produce data for the edge
 * @param consumer the consumer of the rule
 */
  template<class V, class W, class X>
  void addRuleEdge(Bookkeeper<V> *bookkeeper, IRule<V, W> *iRule, ITask<W, X> *consumer) {
    std::shared_ptr<IRule<V, W>> rule = super::getIRule(iRule);

    auto re = new RuleEdge<V, W, X>(bookkeeper, rule, consumer);
    re->applyEdge(this);
    this->addEdgeDescriptor(re);
  }

#ifdef USE_CUDA
  /**
   * Adds a CudaMemoryManager edge with the specified name to the TaskGraphConf.
   * This will create a CudaMemoryManager that is bound to some Cuda GPU based on the pipelineId of
   * the TaskGraphConf.
   * @param name the name of the memory edge, should be unique compared to all memory edges added to the TaskGraphConf and any TaskGraphConf within an ExecutionPipeline
   * @param getMemoryTask the ITask that is getting memory
   * @param allocator the allocator describing how memory is allocated (should allocate Cuda memory)
   * @param memoryPoolSize the size of the memory pool that is allocated by the CudaMemoryManager
   * @param type the type of memory manager
   * @param contexts the array of all Cuda contexts
   * @note the memoryPoolSize can cause out of memory errors for the GPU if the allocator->size() * memoryPoolSize exceeds the total GPU memory
   * @tparam V the type of memory; i.e. 'cufftDoubleComplex *'
   */
  template<class V, class IMemoryAllocatorType>
  void addCudaMemoryManagerEdge(std::string name,
                                AnyITask *getMemoryTask,
                                std::shared_ptr<IMemoryAllocatorType> allocator,
                                size_t memoryPoolSize,
                                MMType type,
                                CUcontext *contexts) {
    static_assert(std::is_base_of<IMemoryAllocator<V>, IMemoryAllocatorType>::value,
                  "Type mismatch for allocator, allocator must be a MemoryAllocator!");

    std::shared_ptr<IMemoryAllocator<V>> memAllocator = std::static_pointer_cast<IMemoryAllocator<V>>(allocator);

    MemoryManager<V> *memoryManager = new CudaMemoryManager<V>(name, contexts, memoryPoolSize, memAllocator, type);

    MemoryEdge<V> *memEdge = new MemoryEdge<V>(name, getMemoryTask, memoryManager);
    memEdge->applyEdge(this);
    this->addEdgeDescriptor(memEdge);
  }

  /**
 * Adds a CudaMemoryManager edge with the specified name to the TaskGraphConf.
 * This will create a CudaMemoryManager that is bound to some Cuda GPU based on the pipelineId of
 * the TaskGraphConf.
 * @param name the name of the memory edge, should be unique compared to all memory edges added to the TaskGraphConf and any TaskGraphConf within an ExecutionPipeline
 * @param getMemoryTask the ITask that is getting memory
 * @param allocator the allocator describing how memory is allocated (should allocate Cuda memory)
 * @param memoryPoolSize the size of the memory pool that is allocated by the CudaMemoryManager
 * @param type the type of memory manager
 *e @param contexts the array of all Cuda contexts
 * @note the memoryPoolSize can cause out of memory errors for the GPU if the allocator->size() * memoryPoolSize exceeds the total GPU memory
 * @tparam V the type of memory; i.e. 'cufftDoubleComplex *'
 */
  template<class V>
  void addCudaMemoryManagerEdge(std::string name,
                                AnyITask *getMemoryTask,
                                IMemoryAllocator<V> *allocator,
                                size_t memoryPoolSize,
                                MMType type,
                                CUcontext *contexts) {

    std::shared_ptr<IMemoryAllocator<V>> memAllocator = this->getMemoryAllocator(allocator);

    MemoryManager<V> *memoryManager = new CudaMemoryManager<V>(name, contexts, memoryPoolSize, memAllocator, type);

    MemoryEdge<V> *memEdge = new MemoryEdge<V>(name, getMemoryTask, memoryManager);
    memEdge->applyEdge(this);
    this->addEdgeDescriptor(memEdge);
  }
#endif

  /**
   * Adds a MemoryManager edge with the specified name to the TaskGraphConf.
   * @param name the name of the memory edge, should be unique compared to all memory edges added to the TaskGraphConf and any TaskGraphConf within an ExecutionPipeline
   * @param getMemoryTask the ITask that is getting memory
   * @param allocator the allocator describing how memory is allocated
   * @param memoryPoolSize the size of the memory pool that is allocated by the MemoryManager
   * @param type the type of memory manager
   * @note the memoryPoolSize can cause out of memory errors for the system if the allocator->size() * memoryPoolSize exceeds the total system memory
   * @tparam V the type of memory; i.e., 'double'
   * @note Use this function if the rule connecting the  bookkeeper and consumer are shared among multiple graphs that you create.
   */
  template<class V, class IMemoryAllocatorType>
  void addMemoryManagerEdge(std::string name, AnyITask *getMemoryTask,
                            std::shared_ptr<IMemoryAllocatorType> allocator, size_t memoryPoolSize, MMType type) {
    static_assert(std::is_base_of<IMemoryAllocator<V>, IMemoryAllocatorType>::value,
                  "Type mismatch for allocator, allocator must be a MemoryAllocator!");

    std::shared_ptr<IMemoryAllocator<V>> memAllocator = std::static_pointer_cast<IMemoryAllocator<V>>(allocator);

    MemoryManager<V> *memoryManager = new MemoryManager<V>(name, memoryPoolSize, memAllocator, type);

    MemoryEdge<V> *memEdge = new MemoryEdge<V>(name, getMemoryTask, memoryManager);
    memEdge->applyEdge(this);
    this->addEdgeDescriptor(memEdge);
  }

  /**
 * Adds a MemoryManager edge with the specified name to the TaskGraphConf.
 * @param name the name of the memory edge, should be unique compared to all memory edges added to the TaskGraphConf and any TaskGraphConf within an ExecutionPipeline
 * @param getMemoryTask the ITask that is getting memory
 * @param allocator the allocator describing how memory is allocated
 * @param memoryPoolSize the size of the memory pool that is allocated by the MemoryManager
 * @param type the type of memory manager
 * @note the memoryPoolSize can cause out of memory errors for the system if the allocator->size() * memoryPoolSize exceeds the total system memory
 * @tparam V the type of memory; i.e., 'double'
 */
  template<class V>
  void addMemoryManagerEdge(std::string name,
                            AnyITask *getMemoryTask,
                            IMemoryAllocator<V> *allocator,
                            size_t memoryPoolSize,
                            MMType type) {

    std::shared_ptr<IMemoryAllocator<V>> memAllocator = super::getMemoryAllocator(allocator);

    MemoryManager<V> *memoryManager = new MemoryManager<V>(name, memoryPoolSize, memAllocator, type);

    MemoryEdge<V> *memEdge = new MemoryEdge<V>(name, getMemoryTask, memoryManager);
    memEdge->applyEdge(this);
    this->addEdgeDescriptor(memEdge);
  }

  AnyTaskManager *getGraphConsumerTaskManager() override {
    return this->graphConsumerTaskManager;
  }

  std::list<AnyTaskManager *> *getGraphProducerTaskManagers() override {
    return this->graphProducerTaskManagers;
  }

  std::shared_ptr<AnyConnector> getInputConnector() override {
    return this->input;
  }

  std::shared_ptr<AnyConnector> getOutputConnector() override {
    return this->output;
  }

  TaskGraphCommunicator *getTaskGraphCommunicator() const { return this->taskConnectorCommunicator; }

  void updateCommunicator() override {

    auto taskNameConnectorMap = this->getTaskConnectorNameMap();

    // Send the map to the taskGraphCommunicator
    this->taskConnectorCommunicator->addTaskNameConnectorMap(taskNameConnectorMap);

    for (auto t : *this->getTaskManagers()) {
      t->setTaskGraphCommunicator(this->taskConnectorCommunicator);
    }

#ifdef WS_PROFILE
    if (this->getAddress() == "0") {
      // Create thread
      std::shared_ptr<std::atomic_size_t>
          atomicNumThreads = std::shared_ptr<std::atomic_size_t>(new std::atomic_size_t(1));
      TaskManagerThread *runtimeThread = new TaskManagerThread(0, this->wsProfileTaskManager, atomicNumThreads);
      this->wsProfileThread = new std::thread(&TaskManagerThread::run, runtimeThread);
    }
    usleep(300000);
#endif

  }

  /**
   * Sets the input connector for the task graph
   * @param input the input connector
   */
  void setInputConnector(std::shared_ptr<Connector<T>> input) {
    this->input = input;
  }

  /**
   * Sets the output connector for the task graph
   * @param output the output connector
   */
  void setOutputConnector(std::shared_ptr<Connector<T>> output) {
    this->output = output;
  }

  /**
   * Increments the number of producers for the task graph.
   * @note The input connector is automatically incremented when creating a
   * graph, so this should only be called if additional sources will be
   * producing data other than the main function.
   */
  void incrementGraphProducer() {
    this->input->incrementInputTaskCount();
  }

  /**
   * Decrements the input connector and wakes up any consumer of the graph's
   * input if the input connector is finished producing data.
   * @note This should be called by the main thread when all data is
   * finished being produced for this task graph.
   */
  void finishedProducingData() {
    this->input->producerFinished();
    if (this->input->getProducerCount() == 0) {
      this->input->wakeupConsumer();
    }

  }

  /**
   * Sets the task that is consuming data from the input of the graph.
   * @tparam W the output type of the task
   * @param task the task that consumes data that is added into the graph.
   * @note Only one task consumes data from the graph. If multiple tasks need
   * data from the graph, then a bookkeeper should be added to distribute
   * data among the multiple tasks.
   */
  template<class W>
  void setGraphConsumerTask(ITask<T, W> *task) {
    this->graphConsumerTaskManager = this->getTaskManager(task);
    this->graphConsumerTaskManager->setInputConnector(this->input);
  }

  /**
   * Sets the task that is producing data for the output of the graph.
   * @tparam W the input type of the task
   * @param task the task that produces data that is added as output for the graph.
   * @note There can be multiple tasks that produces for the graph.
   */
  template<class W>
  void addGraphProducerTask(ITask<W, U> *task) {
    this->output->incrementInputTaskCount();

    AnyTaskManager *taskManager = this->getTaskManager(task);
    taskManager->setOutputConnector(this->output);

    this->graphProducerTaskManagers->push_back(taskManager);
  }

  /**
   * Produces data for the input of the TaskGraph.
   * Must specify the TaskGraph input using addGraphInputConsumer() and use
   * incrementGraphInputProducer() to indicate an input stream is feeding data to the TaskGraph
   *
   * @param data the data being added to the TaskGraph input
   *
   * @note The data being passed will be wrapped into a std::shared_ptr<T>(data)
   */
  void produceData(T *data) {
    std::shared_ptr<T> dataPtr = std::shared_ptr<T>(data);
    this->input->produceData(dataPtr);
  }

  /**
   * Produces data for the input of the TaskGraph.
   * Must specify the TaskGraph input using addGraphInputConsumer() and use
   * incrementGraphInputProducer() to indicate an input stream is feeding data to the TaskGraph
   *
   * @param data the data being added to the TaskGraph input
   */
  void produceData(std::shared_ptr<T> data) {
    this->input->produceData(data);
  }

  /**
   * Adds a list of data into the TaskGraph
   * Must specify the TaskGraph input using addGraphInputConsumer() and use
   * incrementGraphInputProducer() to indicate an input stream is feeding data to the TaskGraph
   *
   * @param dataList the list of data to be added.
   */
  void produceData(std::list<std::shared_ptr<T>> *dataList) {
    this->input->produceData(dataList);
    if (this->input->isInputTerminated())
      this->input->wakeupConsumer();
  }

  /**
   * Consumes data from the output of a TaskGraph.
   * It is possible for consumeData to return nullptr if the last Task has finished.
   * Therefore, when consuming data from a TaskGraph it is important to have a check for nullptr prior to
   * processing that data.
   * @return one data element from the output of the TaskGraph or nullptr if the last task is closing.
   * @note The task producing data for the TaskGraph will send nullptr to the connector, so the thread consuming data
   * should check for nullptr prior to processing the data.
   */
  std::shared_ptr<U> consumeData() {
    return this->output->consumeData();
  }

  /**
   * Polls for data from the output of the TaskGraph
   * @param microTimeout the timeout time in microseconds
   * @return the data or nullptr if the timeout period expires.
   */
  std::shared_ptr<U> pollData(size_t microTimeout) {
    return this->output->pollConsumeData(microTimeout);
  }

  /**
   * Checks if the output of the TaskGraph has finished producing data
   * @return whether the output is finished or not
   * @retval TRUE if the output is no longer producing data
   * @retval FALSE if the output is not finished producing data
   */
  bool isOutputTerminated() {
    return this->output->isInputTerminated();
  }

  /**
   * Sets the output connector for the task graph configuration
   * @param connector the output connector
   */
  void setOutputConnector(std::shared_ptr<AnyConnector> connector) {
    if (graphProducerTaskManagers != nullptr) {
      for (auto task : *graphProducerTaskManagers)
        task->setOutputConnector(connector);
    }

    this->output = std::dynamic_pointer_cast<Connector<U>>(connector);

  }

  /**
   * Releases memory back to its memory manager.
   * @tparam V the type of memory data
   * @param memory the memory
   * @note The m_data_t must have originated within this task graph.
   */
  template<class V>
  void releaseMemory(m_data_t<V> memory) {
    std::shared_ptr<DataPacket> dataPacket = std::shared_ptr<DataPacket>(new DataPacket("TaskGraph",
                                                                                        this->getAddress(),
                                                                                        memory->getMemoryManagerName(),
                                                                                        memory->getAddress(),
                                                                                        memory));
    this->taskConnectorCommunicator->produceDataPacket(dataPacket);
  }

  /**
   * Generates the dot graph as a string
   */
  std::string genDotGraph(int flags, int colorFlag) override {
    std::ostringstream oss;

    // Create header info for graphViz dot file
    oss << "digraph { rankdir=\"TB\"" << std::endl;
    oss << "forcelabels=true;" << std::endl;
    oss << "node[shape=record, fontsize=10, fontname=\"Verdana\"];" << std::endl;
    oss << "edge[fontsize=10, fontname=\"Verdana\"];" << std::endl;
    oss << "graph [compound=true];" << std::endl;

    // Gather profile data
    TaskGraphProfiler profiler(flags);
    profiler.buildProfile(this);

    for (AnyTaskManager *bTask : *this->getTaskManagers()) {
      oss << bTask->getDot(flags);
    }
    oss << profiler.genDotProfile(oss.str(), colorFlag);

    if (getGraphConsumerTaskManager() != nullptr)
      oss << this->getInputConnector()->getDotId() << "[label=\"Graph Input\n"
          << this->getInputConnector()->getProducerCount()
          << (((DOTGEN_FLAG_SHOW_IN_OUT_TYPES & flags) != 0) ? ("\n" + this->getInputConnector()->typeName()) : "")
          << "\"];" << std::endl;

    if (getGraphProducerTaskManagers()->size() > 0)
      oss << "{ rank = sink; " << this->getOutputConnector()->getDotId() << "[label=\"Graph Output\n"
          << this->getOutputConnector()->getProducerCount()
          << (((DOTGEN_FLAG_SHOW_IN_OUT_TYPES & flags) != 0) ? ("\n" + this->getOutputConnector()->typeName()) : "")
          << "\"]; }" << std::endl;

    if (oss.str().find("mainThread") != std::string::npos) {
      oss << "{ rank = sink; mainThread[label=\"Main Thread\", fillcolor = aquamarine4]; }\n";
    }

    oss << "}" << std::endl;

    return oss.str();
  }

  /**
   * Provides debug output for the TaskGraphConf
   * @note \#define DEBUG_FLAG to enable debugging
   */
  void debug() {
    DEBUG("-----------------------------------------------");
    DEBUG("TaskGraphConf -- num vertices: " << this->getTaskManagers()->size() << " -- DETAILS:");

    for (AnyTaskManager *t : *this->getTaskManagers()) {
      t->debug();
    }
    DEBUG("-----------------------------------------------");
  }

#ifdef WS_PROFILE
  void sendProfileData(std::shared_ptr<ProfileData> profileData)
  {
    this->wsProfileTaskManager->getInputConnector()->produceAnyData(profileData);
  }
#endif

 private:

  //! @cond Doxygen_Suppress



  void copyAndUpdateGraphConsumerTask(AnyTaskManager *taskManager) {
    if (taskManager != nullptr) {
      AnyTaskManager *copy = this->getTaskManagerCopy(taskManager->getTaskFunction());
      this->graphConsumerTaskManager = copy;
      this->graphConsumerTaskManager->setInputConnector(this->input);
      this->addTaskManager(this->graphConsumerTaskManager);
    }
  }

  void copyAndUpdateGraphProducerTasks(std::list<AnyTaskManager *> *taskManagers) {
    for (auto taskManager : *taskManagers) {
      if (taskManager != nullptr) {
        AnyTaskManager *copy = this->getTaskManagerCopy(taskManager->getTaskFunction());

        copy->setOutputConnector(this->output);
        this->graphProducerTaskManagers->push_back(copy);

        this->output->incrementInputTaskCount();
        this->addTaskManager(copy);
      }
    }
  }

  void addEdgeDescriptor(EdgeDescriptor *edge) {
    this->edges->push_back(edge);
  }

  typedef AnyTaskGraphConf super;
  //! @endcond

  std::list<EdgeDescriptor *> *
      edges; //!< The list of edges for the graph, represented by edge descriptors to define how the edges are copied/added.

  AnyTaskManager *graphConsumerTaskManager; //!< The consumer accessing the TaskGraph's input connector
  std::list<AnyTaskManager *> *
      graphProducerTaskManagers; //!< The list of producers that are outputting data to the TaskGraph's output connector

  std::shared_ptr<Connector<T>> input; //!< The input connector for the TaskGraph
  std::shared_ptr<Connector<U>> output; //!< The output connector for the TaskGraph

  TaskGraphCommunicator *taskConnectorCommunicator; //!< The task graph communicator for the task graph.

#ifdef WS_PROFILE
  TaskManager<ProfileData, VoidData> *wsProfileTaskManager; // !< The task manager for the web socket profiler
  std::thread *wsProfileThread; // !< The thread for the web socket profiler task manager
#endif

};
}

#endif //HTGS_TASKGRAPHCONF_HPP
