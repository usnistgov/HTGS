
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.
/**
 * @file TaskGraph.hpp
 * @author Timothy Blattner
 * @date Nov 18, 2015
 *
 * @brief Implements the TaskGraph class responsible for managing ITask connections.
 * @details
 */
#ifndef HTGS_TASKGRAPH_HPP
#define HTGS_TASKGRAPH_HPP

#include <htgs/core/graph/edge/ProducerConsumerEdge.hpp>
#include <htgs/core/graph/edge/RuleEdge.hpp>
#include <htgs/core/graph/edge/MemoryEdge.hpp>
#include <htgs/core/memory/VoidMemoryAllocator.hpp>


#ifdef USE_CUDA
#include <cuda.h>
#include <htgs/core/memory/CudaMemoryManager.hpp>
#endif

namespace htgs {

/**
 * @class TaskGraph TaskGraph.hpp <htgs/api/TaskGraph.hpp>
 * @brief Manages a group of connected ITasks and their connections.
 * @details
 * Each ITask that is added into the TaskGraph is stored in the TaskGraph's metadata
 * to allow for quick copying using copy().
 *
 * The main methods for adding each ITask into the graph are
 * addEdge(), addGraphInputConsumer(), addGraphOutputProducer(), and addRule()
 *
 * When using these methods, the TaskGraph builds a Task, which is used to process an ITask's functionality.
 * Parameters for customizing the thread pool, polling abilities, etc., are specified in the ITask constructors:
 * ITask::ITask()
 *
 * Special memory edge functions are provided. For normal CPU memory edges use:
 * addMemoryManagerEdge()
 *
 * For Cuda memory use:
 * addCudaMemoryManagerEdge()
 *
 * Every TaskGraph has an input and output type (T and U). If a TaskGraph does not have an input or output type, then
 * the type can be specified as VoidData.
 *
 * To add data into the input of a TaskGraph use the produceData() functions. These must be used in conjunction
 * with the incrementGraphInputProducer() and finishedProducingData() functions to indicate a data input stream is
 * active and when that input stream is closing, respectively.
 *
 * To process the output of a TaskGraph use the consumeData() function. To determine if data is no longer being
 * produced by a TaskGraph use the isOutputTerminated() function.
 *
 * Example Usage:
 * @code
 * htgs::TaskGraph<MatrixBlockRequest, MatrixBlockRequest> *taskGraph = new htgs::TaskGraph<MatrixBlockRequest, MatrixBlockRequest>();
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
 * taskGraph->addRule(bkTask, scalMulTask, loadRule);
 *
 * // Add memory edges
 * MatrixAllocator *matrixAlloc = new MatrixAllocator(blockSize, blockSize);
 * int poolSize = 50;
 * taskGraph->addMemoryManagerEdge("MatrixA", loadMatrixTask, scalMulTask, matrixAlloc, 50);
 * taskGraph->addMemoryManagerEdge("MatrixB", loadMatrixTask, scalMulTask, matrixAlloc, 50);
 *
 * // Setup graph input/output
 * taskGraph->addGraphInputConsumer(loadMatrixTask);
 * taskGraph->addGraphOutputProducer(scalMulTask);
 *
 * // Indicate input data stream
 * taskGraph->incrementGraphInputProducer();
 *
 * // Setup runtime and execute
 * htgs::Runtime *runtime = new htgs::Runtime(taskGraph);
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
 *   std::shared_ptr<MatrixBlockRquest> mbr = taskGraph->consumeData();
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
class TaskGraphConf: public AnyTaskGraphConf {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:

  /**
   * Constructs a TaskGraph
   */
  TaskGraphConf() : AnyTaskGraphConf(0, 1) {
    this->input = std::shared_ptr<Connector<T>>(new Connector<T>());
    this->output = std::shared_ptr<Connector<U>>(new Connector<U>());

    graphConsumerTaskScheduler = nullptr;
    graphProducerTaskScheduler = nullptr;

    edges = new std::list<EdgeDescriptor *>();
  }

  /**
   * Constructs a TaskGraph
   * @param pipelineId the pipelineId for this graph
   * @param numPipelines the number of pipelines for the graph
   */
  TaskGraphConf(size_t pipelineId, size_t numPipelines) : AnyTaskGraphConf(pipelineId, numPipelines) {
    this->input = std::shared_ptr<Connector<T>>(new Connector<T>());
    this->output = std::shared_ptr<Connector<U>>(new Connector<U>());

    graphConsumerTaskScheduler = nullptr;
    graphProducerTaskScheduler = nullptr;

    edges = new std::list<EdgeDescriptor *>();
  }

  /**
   * Destructor, handles releasing all ITask memory that is managed by the TaskGraph.
   */
  ~TaskGraphConf() override {
    for (auto edge : *edges)
    {
      if (edge != nullptr)
      {
        delete edge;
        edge = nullptr;
      }
    }
    delete edges;
    edges = nullptr;
  }

  AnyTaskGraphConf *copy() override
  {
    return copy(this->getPipelineId(), this->getNumPipelines());
  }


  /**
   * Creates a mirror copy of the TaskGraph with the specified pipelineId and number of pipelines
   * @param pipelineId the pipeline Id
   * @param numPipelines the number of pipelines
   * @return the copy of the task graph
   */
  TaskGraphConf<T, U> *copy(size_t pipelineId, size_t numPipelines)
  {
    return copy(pipelineId, numPipelines, nullptr, nullptr);
  }


  /**
   * Creates a mirror copy of the TaskGraph with the specified pipelineId and number of pipelines, and updates the input
   * and output connectors for the graph copy.
   * @param pipelineId the pipeline Id
   * @param numPipelines the number of pipelines
   * @param input the input connector to be used for the graph's input
   * @param output the output connector to be used for the graph's output
   * @return the copy of the task graph
   */
  TaskGraphConf<T, U> *copy(size_t pipelineId, size_t numPipelines, std::shared_ptr<Connector<T>> input, std::shared_ptr<Connector<U>> output)
  {
    TaskGraphConf<T, U> *graphCopy = new TaskGraphConf<T, U>(pipelineId, numPipelines);

    // Copy the tasks to form lookup between old ITasks and new copies
    graphCopy->copyTasks(this->getTaskSchedulers());

    if (input != nullptr)
    {
      graphCopy->setInputConnector(input);
    }

    if (output != nullptr)
    {
      graphCopy->setOutputConnector(output);
    }

    // Copy the graph producer and consumer tasks
    graphCopy->copyAndUpdateGraphConsumerTask(this->graphConsumerTaskScheduler);
    graphCopy->copyAndUpdateGraphProducerTask(this->graphProducerTaskScheduler);


    for (EdgeDescriptor *edgeDescriptor : *edges)
    {
      // Copy the edge, using the graph copy as a reference for where to get task copies
      EdgeDescriptor *edgeCopy = edgeDescriptor->copy(graphCopy);

      // Apply the edge on the graph copy
      edgeCopy->applyEdge(graphCopy);

      graphCopy->addEdgeDescriptor(edgeCopy);
    }

    graphCopy->updateIdAndNumPipelines(pipelineId, numPipelines);


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
  void addEdge(ITask<V, W> *producer, ITask<W, X> *consumer)
  {
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
   */
  template<class V, class IRuleType, class W, class X>
  void addRuleEdge(Bookkeeper<V> *bookkeeper, std::shared_ptr<IRuleType> rule, ITask<W, X> *consumer)
  {
    static_assert(std::is_base_of<IRule<V, W>, IRuleType>::value, "Type mismatch for IRule<V, W>, V must match the input type of the bookkeeper and W must match the input type of the consumer!");
    std::shared_ptr<IRule<V, W>> ruleCast = std::static_pointer_cast<IRule<V, W>>(rule);
    auto re = new RuleEdge<V, W, X>(bookkeeper, ruleCast, consumer);
    re->applyEdge(this);
    this->addEdgeDescriptor(re);
  }

#ifdef USE_CUDA
  /**
   * Adds a CudaMemoryManager edge with the specified name to the TaskGraphConf.
   * This will create a CudaMemoryManager that is bound to some Cuda GPU based on the pipelineId of
   * the TaskGraphConf.
   * @param name the name of the memory edge
   * @param getMemoryEdges the ITask that is getting memory
   * @param releaseMemoryEdges the ITask that is releasing memory
   * @param allocator the allocator describing how memory is allocated (should allocate Cuda memory)
   * @param memoryPoolSize the size of the memory pool that is allocated by the CudaMemoryManager
   * @param type the type of memory manager
   * @param contexts the array of all Cuda contexts
   * @note the memoryPoolSize can cause out of memory errors for the GPU if the allocator->size() * memoryPoolSize exceeds the total GPU memory
   * @tparam V the type of memory; i.e. 'cufftDoubleComplex *'
   */
  template <class IMemoryAllocatorType>
  void addCudaMemoryManagerEdge(std::string name, AnyITask *getMemoryEdges, AnyITask *releaseMemoryEdges,
          std::shared_ptr<IMemoryAllocatorType> allocator, size_t memoryPoolSize, MMType type, CUcontext * contexts) {
    static_assert(std::is_base_of<IMemoryAllocator<V>, IMemoryAllocatorType>::value, "Type mismatch for allocator, allocator must be a MemoryAllocator!");

    std::shared_ptr<IMemoryAllocator<V>> memAllocator = std::static_pointer_cast<IMemoryAllocator<V>>(allocator);

    MemoryManager<V> *memoryManager = new CudaMemoryManager<V>(name, contexts, memoryPoolSize, memAllocator, type);


    MemoryEdge<V> *memEdge = new MemoryEdge<V>(name, getMemoryTask, releaseMemoryTask, memoryManager);
    memEdge->applyEdge(this);
    this->addEdgeDescriptor(memEdge);
  }
#endif

  /**
   * Adds a MemoryManager that is managed by the user.
   * This edge will enable an ITask to use the MemoryManager to throttle how much data is allocated.
   * @param name the name of the memory edge
   * @param memGetter the ITask that will be getting memory
   * @param memReleaser the ITask that will be releasing memory
   * @param memoryPoolSize the size of the memory pool
   */
  void addUserManagedMemoryManagerEdge(std::string name,
                                       AnyITask *getMemoryTask,
                                       AnyITask *releaseMemoryTask,
                                       size_t memoryPoolSize) {
    auto voidAllocator = std::make_shared<VoidMemoryAllocator>();
    MemoryManager<void *> *memoryManager = new MemoryManager<void *>(name, memoryPoolSize, voidAllocator, MMType::UserManaged);

    MemoryEdge<void *> *memEdge = new MemoryEdge<void *>(name, getMemoryTask, releaseMemoryTask, memoryManager);
    memEdge->applyEdge(this);
    this->addEdgeDescriptor(memEdge);
  }

  /**
   * Adds a MemoryManager edge with the specified name to the TaskGraph.
   * @param name the name of the memory edge
   * @param getMemoryTask the ITask that is getting memory
   * @param releaseMemoryTask the ITask that is releasing memory
   * @param allocator the allocator describing how memory is allocated (should allocate Cuda memory)
   * @param memoryPoolSize the size of the memory pool that is allocated by the CudaMemoryManager
   * @param type the type of memory manager
   * @note the memoryPoolSize can cause out of memory errors for the system if the allocator->size() * memoryPoolSize exceeds the total system memory
   * @tparam V the type of memory; i.e., 'double *'
   */
  template<class V, class IMemoryAllocatorType>
  void addMemoryManagerEdge(std::string name, AnyITask *getMemoryTask, AnyITask *releaseMemoryTask,
                            std::shared_ptr<IMemoryAllocatorType> allocator, size_t memoryPoolSize, MMType type) {
    static_assert(std::is_base_of<IMemoryAllocator<V>, IMemoryAllocatorType>::value, "Type mismatch for allocator, allocator must be a MemoryAllocator!");

    std::shared_ptr<IMemoryAllocator<V>> memAllocator = std::static_pointer_cast<IMemoryAllocator<V>>(allocator);

    MemoryManager<V> *memoryManager = new MemoryManager<V>(name, memoryPoolSize, memAllocator, type);

    MemoryEdge<V> *memEdge = new MemoryEdge<V>(name, getMemoryTask, releaseMemoryTask, memoryManager);
    memEdge->applyEdge(this);
    this->addEdgeDescriptor(memEdge);
  }


  AnyTaskScheduler *getGraphConsumerTaskScheduler() override {
    return this->graphConsumerTaskScheduler;
  }

  AnyTaskScheduler *getGraphProducerTaskScheduler() override {
    return this->graphProducerTaskScheduler;
  }

  std::shared_ptr<AnyConnector> getInputConnector() override {
    return this->input;
  }

  std::shared_ptr<AnyConnector> getOutputConnector() override {
    return this->output;
  }

  void setInputConnector(std::shared_ptr<Connector<T>> input) {
    this->input = input;
  }

  void setOutputConnector(std::shared_ptr<Connector<T>> output) {
    this->output = output;
  }

  void incrementGraphProducer()
  {
    this->input->incrementInputTaskCount();
  }

  void decrementGraphProducer()
  {
    this->input->producerFinished();
  }

  template<class W>
  void setGraphConsumerTask(ITask<T, W> *task)
  {
    // TODO: Number of active connections for Connector
    // TODO: Check about setting connector to nullptr for previous task
//    if (this->graphConsumerTaskScheduler != nullptr)
//      this->graphConsumerTaskScheduler->setInputConnector(nullptr);

    this->graphConsumerTaskScheduler = this->getTaskScheduler(task);

    this->graphConsumerTaskScheduler->setInputConnector(this->input);

  }

  template<class W>
  void setGraphProducerTask(ITask<W, U> * task)
  {
    // TODO: Number of active connections for Connector
    // TODO: Check about setting connector to nullptr for previous task
//    if (this->graphProducerTaskScheduler != nullptr)
//      this->graphProducerTaskScheduler->setOutputConnector(nullptr);

    if (this->graphProducerTaskScheduler == nullptr)
    {
      this->output->incrementInputTaskCount();
    }

    this->graphProducerTaskScheduler = this->getTaskScheduler(task);

    this->graphProducerTaskScheduler->setOutputConnector(this->output);
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

  void setOutputConnector(std::shared_ptr<AnyConnector> connector)
  {
    if (graphProducerTaskScheduler != nullptr)
      graphProducerTaskScheduler->setOutputConnector(connector);

    this->output = std::dynamic_pointer_cast<Connector<U>>(connector);

  }

//  /**
//   * Writes the dot representation of the task graph to disk
//   * @param file the file path (will not create directories)
//   */
//  void writeDotToFile(std::string file) {
//    writeDotToFile(file, 0);
//  }
//
//  void writeDotToFile(std::string file, int flags) {
//    std::ofstream f(file);
//    f << genDotGraph(flags);
//    f.flush();
//
//    std::cout << "Writing dot file for task graph to " << file << std::endl;
//  }
//
//
//  /**
//   * Generate the content only of the graph (excludes all graph definitions and attributes)
//   */
//  std::string genDotGraphContent(int flags) {
//    std::ostringstream oss;
//
//    for (AnyTaskScheduler *bTask : *vertices) {
//      oss << bTask->getDot(flags);
//    }
//
//    if ((flags & DOTGEN_FLAG_HIDE_MEM_EDGES) != 0) {
//
//      if (memReleaser->size() > 0) {
//        for (const auto &kv : *this->memReleaser) {
//          auto connector = kv.second->at(this->pipelineId);
//          oss << std::string("mainThread") << " -> " << connector->getDotId() << ";" << std::endl;
//        }
//
//        oss << "mainThread[label=\"Main Thread\"];\n";
//      }
//    }
//
//    return oss.str();
//  }
//
//  /**
//   * Generates the dot graph as a string
//   */
//  std::string genDotGraph(int flags) {
//    std::ostringstream oss;
//
//    oss << "digraph { rankdir=\"TB\"" << std::endl;
//    oss << "forcelabels=true;" << std::endl;
//    oss << "node[shape=record, fontsize=10, fontname=\"Verdana\"];" << std::endl;
//    oss << "edge[fontsize=10, fontname=\"Verdana\"];" << std::endl;
//    oss << "graph [compound=true];" << std::endl;
//
//    for (AnyTaskScheduler *bTask : *vertices) {
//      oss << bTask->getDot(flags);
//    }
//
//    if (this->graphInputConsumers->size() > 0)
//      oss << this->input->getDotId() << "[label=\"Graph Input\n" << this->input->getProducerCount() <<  (((DOTGEN_FLAG_SHOW_IN_OUT_TYPES & flags) != 0) ? ("\n"+this->input->typeName()) : "") << "\"];" << std::endl;
//
//    if (this->graphOutputProducers->size() > 0)
//      oss << "{ rank = sink; " << this->output->getDotId() << "[label=\"Graph Output\n" << this->output->getProducerCount() <<  (((DOTGEN_FLAG_SHOW_IN_OUT_TYPES & flags) != 0) ? ("\n"+this->output->typeName()) : "") << "\"]; }" << std::endl;
//
//
//    if ((flags & DOTGEN_FLAG_HIDE_MEM_EDGES) == 0) {
//      if (memReleaser->size() > 0) {
//        for (const auto &kv : *this->memReleaser) {
//          for (const auto &memConnector : *kv.second)
//            oss << std::string("mainThread") << " -> " << memConnector->getDotId() << ";" << std::endl;
//        }
//      }
//    }
//
//    if (oss.str().find("mainThread") != std::string::npos)
//    {
//      oss << "{ rank = sink; mainThread[label=\"Main Thread\", fillcolor = aquamarine4]; }\n";
//    }
//
//
//#ifdef PROFILE
//    std::string desc = "";
//    std::unordered_map<std::string, double> *timeMap;
//    std::unordered_map<std::string, std::string> *colorMap;
//
//    if ((flags & DOTGEN_FLAG_SHOW_PROFILE_COMP_TIME) != 0)
//    {
//      desc = "Compute Time (sec): ";
//      timeMap = this->getComputeTimeAverages();
//
//    }
//    else if ((flags & DOTGEN_FLAG_SHOW_PROFILE_WAIT_TIME) != 0)
//    {
//      desc = "Wait Time (sec): ";
//      timeMap = this->getWaitTimeAverages();
//    }
//    else if ((flags & DOTGEN_FLAG_SHOW_PROFILE_MAX_Q_SZ) != 0)
//    {
//      desc = "Max Q Size";
//      timeMap = this->getMaxQSizeAverages();
//    }
//
//    if (desc != "") {
//      colorMap = this->genColorMap(timeMap);
//      oss << this->genProfileGraph(flags, timeMap, desc, colorMap);
//
//      delete timeMap;
//      delete colorMap;
//    }
//#endif
//
//    oss << "}" << std::endl;
//
//    return oss.str();
//  }
//
//
//#ifdef PROFILE
//  std::unordered_map<std::string, double> *getComputeTimeAverages()
//  {
//    std::unordered_multimap<std::string, long long int> mmap;
//
//    this->gatherComputeTime(&mmap);
//
//    return computeAverages<long long int>(&mmap, 1000000.0);
//  }
//
//  std::unordered_map<std::string, double> *getWaitTimeAverages()
//  {
//    std::unordered_multimap<std::string, long long int> mmap;
//
//    this->gatherWaitTime(&mmap);
//
//    return computeAverages<long long int>(&mmap, 1000000.0);
//  }
//
//  std::unordered_map<std::string, double> *getMaxQSizeAverages()
//  {
//    std::unordered_multimap<std::string, int> mmap;
//
//    this->gatherMaxQSize(&mmap);
//
//    return computeAverages<int>(&mmap, 1.0);
//  }
//
//  template <typename Type>
//  std::unordered_map<std::string, double> *computeAverages(std::unordered_multimap<std::string, Type> *mmap, double divisor)
//  {
//    std::unordered_map<std::string, double> *ret = new std::unordered_map<std::string, double>();
//    std::string current("");
//    Type total = 0;
//    int count = 0;
//    // Loop over each
//    for (auto v : *mmap)
//    {
//      if (current != v.first) {
//        if (count != 0)
//        {
//          double avg = (double)total / (double) count;
//          ret->insert(std::pair<std::string, double>(current, (avg/divisor)));
//        }
//        else if (current != "")
//        {
//          ret->insert(std::pair<std::string, double>(current, 0.0));
//        }
//
//        current = v.first;
//        total = 0;
//        count = 0;
//      }
//
//      if (v.second > 0) {
//        total += v.second;
//        count++;
//      }
//    }
//    return ret;
//  }
//
//  void gatherComputeTime(std::unordered_multimap<std::string, long long int> *mmap) {
//    for (AnyTaskScheduler *bTask : *vertices) {
//      bTask->gatherComputeTime(mmap);
//    }
//  }
//
//
//  void gatherWaitTime(std::unordered_multimap<std::string, long long int> *mmap) {
//    for (AnyTaskScheduler *bTask : *vertices) {
//      bTask->gatherWaitTime(mmap);
//    }
//  }
//
//  virtual void gatherMaxQSize(std::unordered_multimap<std::string, int> *mmap) {
//    for (AnyTaskScheduler *bTask : *vertices) {
//      bTask->gatherMaxQSize(mmap);
//    }
//  }
//
//  std::unordered_map<std::string, std::string> *genColorMap(std::unordered_map<std::string, double> *mmap)
//  {
//    std::unordered_map<std::string, std::string> *colorMap = new std::unordered_map<std::string, std::string>();
//
//    int rColor[10] = {0,0,0,0,85,170,255,255,255,255};
//    int gColor[10] = {0,85,170,255,255,255,255,170,85,0};
//    int bColor[10] = {255,255,255,255,170,85,0,0,0,0};
//
//    std::deque<double> vals;
//    double maxTime = 0.0;
//    double totalTime = 0.0;
//    for (auto v : *mmap)
//    {
//      if (v.second > 0) {
//        totalTime += v.second;
//      }
//
//      if (maxTime < v.second)
//        maxTime = v.second;
//    }
//
//    for (auto v : *mmap)
//    {
//      if (v.second == 0.0) {
//        colorMap->insert(std::pair<std::string, std::string>(v.first, "black"));
//        continue;
//      }
//
//      int red = 0;
//      int green = 0;
//      int blue = 0;
//
//      // compute percentage of totalTime
//      int perc = (int) (v.second / maxTime * 100.0);
//
//      if (perc % 10 != 0)
//        perc = perc + 10 - (perc % 10);
//
//      int index = (perc / 10);
//
//      if (index < 0)
//        index = 0;
//
//      if (index >= 10)
//        index = 9;
//
//      red = rColor[index];
//      green = gColor[index];
//      blue = bColor[index];
//
//      char hexcol[16];
//
//      snprintf(hexcol, sizeof(hexcol), "%02x%02x%02x", red & 0xff, green & 0xff ,blue & 0xff);
//      std::string color(hexcol);
//      color = "#" + color;
//
//      colorMap->insert(std::pair<std::string, std::string>(v.first, color));
//
//    }
//
//    return colorMap;
//  }
//
//  std::string genProfileGraph(int flags, std::unordered_map<std::string, double> *mmap, std::string desc, std::unordered_map<std::string, std::string> *colorMap) {
//    std::ostringstream oss;
//
//    for (AnyTaskScheduler *bTask : *vertices) {
//      if (mmap->find(bTask->getNameWithPipID()) == mmap->end()) {
//        continue;
//      }
//
//      oss << bTask->genDotProfile(flags, mmap, desc, colorMap);
//    }
//
//    return oss.str();
//  }
//#endif
//

//
//  /**
//   * Provides debug output for the TaskGraphConf
//   * @note \#define DEBUG_FLAG to enable debugging
//   */
//  void debug() const {
//
//    DEBUG("-----------------------------------------------");
//    DEBUG("TaskGraphConf -- num vertices: " << vertices->size() << " num edges: " << edges->size() <<
//        " -- DETAILS:");
//
//    for (AnyTaskScheduler *t : *vertices) {
//      t->debug();
//    }
//
//    for (std::shared_ptr<AnyConnector> c : *edges) {
//      DEBUG("Connector: " << c << " total producers: " << c->getProducerCount());
//    }
//
//    DEBUG("Graph input: " << this->input << " total producers: " <<
//        (input ? this->input->getProducerCount() : 0));
//    DEBUG("Graph output: " << this->output << " total producers: " <<
//        (output ? this->output->getProducerCount() : 0));
//    DEBUG("-----------------------------------------------");
//  }
//
//  /**
//   * Processes all of the input for the TaskGraphConf until no more input is available
//   * @note \#define DEBUG_FLAG to enable debugging
//   */
//  void debugInput() {
//    while (!this->input->isInputTerminated())
//      DEBUG("data = " << this->input->consumeData() << std::endl);
//  }


 private:

  //! @cond Doxygen_Suppress



  void copyAndUpdateGraphConsumerTask(AnyTaskScheduler *taskScheduler)
  {
    if (taskScheduler != nullptr) {
      AnyTaskScheduler *copy = this->getTaskSchedulerCopy(taskScheduler->getTaskFunction());
      this->graphConsumerTaskScheduler = copy;
      this->graphConsumerTaskScheduler->setInputConnector(this->input);
      this->addTaskScheduler(this->graphConsumerTaskScheduler);
    }
  }

  void copyAndUpdateGraphProducerTask(AnyTaskScheduler *taskScheduler)
  {
    if (taskScheduler != nullptr) {
      AnyTaskScheduler *copy = this->getTaskSchedulerCopy(taskScheduler->getTaskFunction());

      // TODO: Number of active connections for Connector
      this->graphProducerTaskScheduler = copy;
      this->graphProducerTaskScheduler->setOutputConnector(this->output);
      this->output->incrementInputTaskCount();
      this->addTaskScheduler(this->graphProducerTaskScheduler);
    }
  }

  void addEdgeDescriptor(EdgeDescriptor *edge)
  {
    this->edges->push_back(edge);
  }

  //! @endcond
  std::list<EdgeDescriptor *> *edges;

  AnyTaskScheduler *graphConsumerTaskScheduler; //!< The list of consumers accessing the TaskGraph's input connector
  AnyTaskScheduler *graphProducerTaskScheduler; //!< The list of producers that are outputting data to the TaskGraph's output connector

  std::shared_ptr<Connector<T>> input; //!< The input connector for the TaskGraph
  std::shared_ptr<Connector<U>> output; //!< The output connector for the TaskGraph

};
}

#endif //HTGS_TASKGRAPH_HPP
