
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
#ifndef HTGS_TASKGRAPH_H
#define HTGS_TASKGRAPH_H
#include <map>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <cassert>

#include "../core/graph/TaskGraphDotGenFlags.h"
#include "../core/memory/VoidMemoryAllocator.hpp"
#include "Bookkeeper.hpp"
#include "VoidData.hpp"
#include "../core/task/BaseTaskScheduler.hpp"
#include "../core/graph/BaseTaskGraph.hpp"
#include "../core/graph/history/ProducerConsumerKey.hpp"
#include "../core/graph/history/BookkeeperKey.hpp"
#include "../core/graph/history/MemoryManagerKey.hpp"
#include "../core/memory/MemoryManager.hpp"
#include "ICustomEdge.hpp"
#ifdef USE_CUDA
#include "../core/memory/CudaMemoryManager.hpp"
#endif

namespace htgs {

/**
 * @typedef ConnectorMap
 * Creates a mapping between a Task and it's input Connector.
 */
typedef std::map<BaseTaskScheduler *, std::shared_ptr<BaseConnector>> ConnectorMap;

/**
 * @typedef ConnectorPair
 * Defines a pair to be added into a ConnectorMap.
 */
typedef std::pair<BaseTaskScheduler *, std::shared_ptr<BaseConnector>> ConnectorPair;

/**
 * @typedef ITaskMap
 * Creates a mapping between an ITask and it's Task handler.
 */
typedef std::map<BaseITask *, BaseTaskScheduler *> ITaskMap;

/**
 * @typedef ITaskPair
 * Defines a pair to be added into an ITaskMap
 */
typedef std::pair<BaseITask *, BaseTaskScheduler *> ITaskPair;

/**
 * @typedef RuleEdgeMap
 * Creates a mapping between two ITask's and the RuleManager that manages the rules between the two ITask's.
 * The first ITask represents the Bookkeeper, the second is the consumer ITask
 */
typedef std::map<std::pair<BaseITask *, BaseITask *>, BaseBaseRuleManager *> RuleEdgeMap;

/**
 * @typedef RuleEdgePair
 * Defines a pair to be added into a RuleEdgeMap
 */
typedef std::pair<std::pair<BaseITask *, BaseITask *>, BaseBaseRuleManager *> RuleEdgePair;

/**
 * @typedef CustomEdgeMap
 * Defines a mapping between a producer/consumer ITask and the ICustomEdge that it uses.
 */
typedef std::map<std::pair<BaseTaskScheduler *, BaseTaskScheduler *>, ICustomEdge *> CustomEdgeMap;

/**
 * @typedef CustomEdgePair
 * Defines a pair to be added to the CustomEdgeMap
 */
typedef std::pair<std::pair<BaseTaskScheduler *, BaseTaskScheduler *>, ICustomEdge *> CustomEdgePair;

/**
 * @typedef MemAllocMap
 * Defines a mapping between a BaseMemoryAllocator and its shared_ptr
 */
typedef std::map<BaseMemoryAllocator *, std::shared_ptr<BaseMemoryAllocator>> MemAllocMap;

/**
 * @typedef MemAllocPair;
 * Defines a pair to be added to the MemAllocMap
 */
typedef std::pair<BaseMemoryAllocator *, std::shared_ptr<BaseMemoryAllocator>> MemAllocPair;

/**
 * @typedef MemManagerMap
 * Defines a mapping between the memory edge name and the memory manager that is managing that edge.
 */
typedef std::map<std::string, BaseITask *> MemManagerMap;

/**
 * @typedef MemManagerPair
 * Defines a pair to be added to the MemManagerMap
 */
typedef std::pair<std::string, BaseITask *> MemManagerPair;

/**
 * @typedef MemGetterMap
 * Defines a mapping between the memory getter BaseITask and the MemManagerMap
 */
typedef std::map<BaseITask *, MemManagerMap *> MemGetterMap;

/**
 * @typedef MemGetterPair
 * Defines a pair to be added to the MemGetterMap
 */
typedef std::pair<BaseITask *, MemManagerMap *> MemGetterPair;

/**
 * @typedef IRuleMap
 * Defines a mapping between an IRule pointer and the shared pointer of that IRule
 */
typedef std::map<BaseIRule *, std::shared_ptr<BaseIRule>> IRuleMap;

/**
 * @typedef IRulePair
 * Defines a pair to be added to the IRuleMap
 */
typedef std::pair<BaseIRule *, std::shared_ptr<BaseIRule>> IRulePair;

 /**
 * Structure that compares a custom edge pair
 */
struct CustomEdgeComparator
{
  /**
   * Compares a custom edge pair
   * @param pair the custom edge pair
   */
  CustomEdgeComparator(CustomEdgePair pair) : _p(pair) { }

  /**
   * Operator to compare a custom edge pair
   */
  bool operator() (CustomEdgePair const& p) {
    // Checks to ensure all elements of the custom edge pair are equivalent to the tested pair
    return p.first.first == _p.first.first && p.first.second == _p.first.second && p.second == p.second;
  }
  CustomEdgePair _p; //!< The custom edge pair to compare with
};
  
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
class TaskGraph: public BaseTaskGraph {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:

  /**
   * Constructs a TaskGraph
   */
  TaskGraph() {
    this->input = std::shared_ptr<Connector<T>>(new Connector<T>());
    this->output = std::shared_ptr<Connector<U>>(new Connector<U>());
    this->pipelineId = 0;

    graphInputConsumers = new std::list<BaseTaskScheduler *>();
    graphOutputProducers = new std::list<BaseTaskScheduler *>();

    edges = new std::list<std::shared_ptr<BaseConnector>>();
    vertices = new std::list<BaseTaskScheduler *>();

    producerConsumerKeys = new std::list<ProducerConsumerKey *>();
    bookkeeperKeys = new std::list<BookkeeperKey *>();
    memoryManagerKeys = new std::list<MemoryManagerKey *>();

    consumerTaskConnectorMap = new ConnectorMap();

    iTaskMap = new ITaskMap();

    ruleEdgeMap = new RuleEdgeMap();

    customEdgeList = new std::list<CustomEdgePair>();

    memAllocMap = new MemAllocMap();

    memGetterMap = new MemGetterMap();

    iRuleMap = new IRuleMap();

    memReleaser = std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> >> (new std::unordered_map<std::string, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> >());
    mmTypeMap = std::shared_ptr<std::unordered_map<std::string, MMType>>(new std::unordered_map<std::string, MMType>());
  }

  /**
   * Constructs a TaskGraph with the specified input and output Connector.
   * @param input
   * @param output
   */
  TaskGraph(std::shared_ptr<Connector<T>> input, std::shared_ptr<Connector<U>> output) {
    this->input = input;
    this->output = output;
    this->pipelineId = 0;

    graphInputConsumers = new std::list<BaseTaskScheduler *>();
    graphOutputProducers = new std::list<BaseTaskScheduler *>();

    edges = new std::list<std::shared_ptr<BaseConnector>>();
    vertices = new std::list<BaseTaskScheduler *>();

    producerConsumerKeys = new std::list<ProducerConsumerKey *>();
    bookkeeperKeys = new std::list<BookkeeperKey *>();
    memoryManagerKeys = new std::list<MemoryManagerKey *>();

    consumerTaskConnectorMap = new ConnectorMap();

    iTaskMap = new ITaskMap();

    ruleEdgeMap = new RuleEdgeMap();

    customEdgeList = new std::list<CustomEdgePair>();

    memAllocMap = new MemAllocMap();

    memGetterMap = new MemGetterMap();

    iRuleMap = new IRuleMap();

    memReleaser = std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> >> (new std::unordered_map<std::string, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> >());
    mmTypeMap = std::shared_ptr<std::unordered_map<std::string, MMType>>(new std::unordered_map<std::string, MMType>());
  }

  /**
   * Destructor, handles releasing all ITask memory that is managed by the TaskGraph.
   */
  ~TaskGraph() {
    // ONLY release memory for things inside graph (edges/vertices)
    for (BaseTaskScheduler *task : *vertices) {
      if (task != nullptr) {
        delete task;
        task = nullptr;
      }
    }

    for (ProducerConsumerKey *pck : *producerConsumerKeys) {
      if (pck != nullptr) {
        delete pck;
        pck = nullptr;
      }
    }

    for (BookkeeperKey *bkk : *bookkeeperKeys) {
      if (bkk != nullptr) {
        delete bkk;
        bkk = nullptr;
      }
    }

    for (MemoryManagerKey *mmk : *memoryManagerKeys) {
      if (mmk != nullptr) {
        delete mmk;
        mmk = nullptr;
      }
    }

    for (CustomEdgePair pair : *customEdgeList) {
      if (pair.second != nullptr) {
        delete pair.second;
        pair.second = nullptr;
      }
    }

    delete graphInputConsumers;
    graphInputConsumers = nullptr;
    delete graphOutputProducers;
    graphOutputProducers = nullptr;
    delete edges;
    edges = nullptr;
    delete vertices;
    vertices = nullptr;
    delete producerConsumerKeys;
    producerConsumerKeys = nullptr;
    delete bookkeeperKeys;
    bookkeeperKeys = nullptr;
    delete memoryManagerKeys;
    memoryManagerKeys = nullptr;
    delete consumerTaskConnectorMap;
    consumerTaskConnectorMap = nullptr;
    delete memAllocMap;
    memAllocMap = nullptr;

    for (MemGetterPair p : *memGetterMap)
    {
      delete p.second;
      p.second = nullptr;
    }

    delete memGetterMap;
    memGetterMap = nullptr;

    delete iTaskMap;
    iTaskMap = nullptr;

    delete ruleEdgeMap;
    ruleEdgeMap = nullptr;

    delete customEdgeList;
    customEdgeList = nullptr;

    delete iRuleMap;
    iRuleMap = nullptr;
  }

  /**
   * Writes the dot representation of the task graph to disk
   * @param file the file path (will not create directories)
   */
  void writeDotToFile(std::string file) {
    writeDotToFile(file, 0);
  }

  void writeDotToFile(std::string file, int flags) {
    std::ofstream f(file);
    f << genDotGraph(flags);
    f.flush();

    std::cout << "Writing dot file for task graph to " << file << std::endl;
  }


  /**
   * Generate the content only of the graph (excludes all graph definitions and attributes)
   */
  std::string genDotGraphContent(int flags) {
    std::ostringstream oss;

    for (BaseTaskScheduler *bTask : *vertices) {
      oss << bTask->getDot(flags);
    }

    if ((flags & DOTGEN_FLAG_HIDE_MEM_EDGES) != 0) {

      if (memReleaser->size() > 0) {
        for (const auto &kv : *this->memReleaser) {
          auto connector = kv.second->at(this->pipelineId);
          oss << std::string("mainThread") << " -> " << connector->getDotId() << ";" << std::endl;
        }

        oss << "mainThread[label=\"Main Thread\"];\n";
      }
    }

    return oss.str();
  }

  /**
   * Generates the dot graph as a string
   */
  std::string genDotGraph(int flags) {
    std::ostringstream oss;

    oss << "digraph { rankdir=\"TB\"" << std::endl;
    oss << "forcelabels=true;" << std::endl;
    oss << "node[shape=record, fontsize=10, fontname=\"Verdana\"];" << std::endl;
    oss << "edge[fontsize=10, fontname=\"Verdana\"];" << std::endl;
    oss << "graph [compound=true];" << std::endl;

    for (BaseTaskScheduler *bTask : *vertices) {
      oss << bTask->getDot(flags);
    }

    if (this->graphInputConsumers->size() > 0)
      oss << this->input->getDotId() << "[label=\"Graph Input\n" << this->input->getProducerCount() <<  (((DOTGEN_FLAG_SHOW_IN_OUT_TYPES & flags) != 0) ? ("\n"+this->input->typeName()) : "") << "\"];" << std::endl;

    if (this->graphOutputProducers->size() > 0)
      oss << "{ rank = sink; " << this->output->getDotId() << "[label=\"Graph Output\n" << this->output->getProducerCount() <<  (((DOTGEN_FLAG_SHOW_IN_OUT_TYPES & flags) != 0) ? ("\n"+this->output->typeName()) : "") << "\"]; }" << std::endl;


    if ((flags & DOTGEN_FLAG_HIDE_MEM_EDGES) == 0) {
      if (memReleaser->size() > 0) {
        for (const auto &kv : *this->memReleaser) {
          for (const auto &memConnector : *kv.second)
            oss << std::string("mainThread") << " -> " << memConnector->getDotId() << ";" << std::endl;
        }
      }
    }

    if (oss.str().find("mainThread") != std::string::npos)
    {
      oss << "{ rank = sink; mainThread[label=\"Main Thread\", fillcolor = aquamarine4]; }\n";
    }


#ifdef PROFILE
    std::string desc = "";
    std::unordered_map<std::string, double> *timeMap;
    std::unordered_map<std::string, std::string> *colorMap;

    if ((flags & DOTGEN_FLAG_SHOW_PROFILE_COMP_TIME) != 0)
    {
      desc = "Compute Time (sec): ";
      timeMap = this->getComputeTimeAverages();

    }
    else if ((flags & DOTGEN_FLAG_SHOW_PROFILE_WAIT_TIME) != 0)
    {
      desc = "Wait Time (sec): ";
      timeMap = this->getWaitTimeAverages();
    }
    else if ((flags & DOTGEN_FLAG_SHOW_PROFILE_MAX_Q_SZ) != 0)
    {
      desc = "Max Q Size";
      timeMap = this->getMaxQSizeAverages();
    }

    if (desc != "") {
      colorMap = this->genColorMap(timeMap);
      oss << this->genProfileGraph(flags, timeMap, desc, colorMap);

      delete timeMap;
      delete colorMap;
    }
#endif

    oss << "}" << std::endl;

    return oss.str();
  }


#ifdef PROFILE
  std::unordered_map<std::string, double> *getComputeTimeAverages()
  {
    std::unordered_multimap<std::string, long long int> mmap;

    this->gatherComputeTime(&mmap);

    return computeAverages<long long int>(&mmap, 1000000.0);
  }

  std::unordered_map<std::string, double> *getWaitTimeAverages()
  {
    std::unordered_multimap<std::string, long long int> mmap;

    this->gatherWaitTime(&mmap);

    return computeAverages<long long int>(&mmap, 1000000.0);
  }

  std::unordered_map<std::string, double> *getMaxQSizeAverages()
  {
    std::unordered_multimap<std::string, int> mmap;

    this->gatherMaxQSize(&mmap);

    return computeAverages<int>(&mmap, 1.0);
  }

  template <typename Type>
  std::unordered_map<std::string, double> *computeAverages(std::unordered_multimap<std::string, Type> *mmap, double divisor)
  {
    std::unordered_map<std::string, double> *ret = new std::unordered_map<std::string, double>();
    std::string current("");
    Type total = 0;
    int count = 0;
    // Loop over each
    for (auto v : *mmap)
    {
      if (current != v.first) {
        if (count != 0)
        {
          double avg = (double)total / (double) count;
          ret->insert(std::pair<std::string, double>(current, (avg/divisor)));
        }
        else if (current != "")
        {
          ret->insert(std::pair<std::string, double>(current, 0.0));
        }

        current = v.first;
        total = 0;
        count = 0;
      }

      if (v.second > 0) {
        total += v.second;
        count++;
      }
    }
    return ret;
  }

  void gatherComputeTime(std::unordered_multimap<std::string, long long int> *mmap) {
    for (BaseTaskScheduler *bTask : *vertices) {
      bTask->gatherComputeTime(mmap);
    }
  }


  void gatherWaitTime(std::unordered_multimap<std::string, long long int> *mmap) {
    for (BaseTaskScheduler *bTask : *vertices) {
      bTask->gatherWaitTime(mmap);
    }
  }

  virtual void gatherMaxQSize(std::unordered_multimap<std::string, int> *mmap) {
    for (BaseTaskScheduler *bTask : *vertices) {
      bTask->gatherMaxQSize(mmap);
    }
  }

  std::unordered_map<std::string, std::string> *genColorMap(std::unordered_map<std::string, double> *mmap)
  {
    std::unordered_map<std::string, std::string> *colorMap = new std::unordered_map<std::string, std::string>();

    int rColor[10] = {0,0,0,0,85,170,255,255,255,255};
    int gColor[10] = {0,85,170,255,255,255,255,170,85,0};
    int bColor[10] = {255,255,255,255,170,85,0,0,0,0};

    std::deque<double> vals;
    double maxTime = 0.0;
    double totalTime = 0.0;
    for (auto v : *mmap)
    {
      if (v.second > 0) {
        totalTime += v.second;
      }

      if (maxTime < v.second)
        maxTime = v.second;
    }

    for (auto v : *mmap)
    {
      if (v.second == 0.0) {
        colorMap->insert(std::pair<std::string, std::string>(v.first, "black"));
        continue;
      }

      int red = 0;
      int green = 0;
      int blue = 0;

      // compute percentage of totalTime
      int perc = (int) (v.second / maxTime * 100.0);

      if (perc % 10 != 0)
        perc = perc + 10 - (perc % 10);

      int index = (perc / 10);

      if (index < 0)
        index = 0;

      if (index >= 10)
        index = 9;

      red = rColor[index];
      green = gColor[index];
      blue = bColor[index];

      char hexcol[16];

      snprintf(hexcol, sizeof(hexcol), "%02x%02x%02x", red & 0xff, green & 0xff ,blue & 0xff);
      std::string color(hexcol);
      color = "#" + color;

      colorMap->insert(std::pair<std::string, std::string>(v.first, color));

    }

    return colorMap;
  }

  std::string genProfileGraph(int flags, std::unordered_map<std::string, double> *mmap, std::string desc, std::unordered_map<std::string, std::string> *colorMap) {
    std::ostringstream oss;

    for (BaseTaskScheduler *bTask : *vertices) {
      if (mmap->find(bTask->getNameWithPipID()) == mmap->end()) {
        continue;
      }

      oss << bTask->genDotProfile(flags, mmap, desc, colorMap);
    }

    return oss.str();
  }
#endif

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
    this->input->produceData(std::shared_ptr<T>(data));
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
  std::shared_ptr<U> pollData(long microTimeout) {
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
   * @internal
   * Updates the TaskGraph input consumer
   * @param connector the connector to update the input consumers
   *
   * @note This function should only be called by the HTGS API
   */
  void updateGraphInputConsumers(std::shared_ptr<Connector<T>> connector) {
    for (BaseTaskScheduler *bs : *this->graphInputConsumers) {
      bs->setInputConnector(connector);
    }

    this->input = connector;
  }

  /**
   * @internal
   * Updates the TaskGraph output producers
   * @param connector the connector to update the output producers
   * @param increment whether to increment the number of producers for the connector
   *
   * @note This function should only be called by the HTGS API
   */
  void updateGraphOutputProducers(std::shared_ptr<Connector<U>> connector, bool increment) {
    for (BaseTaskScheduler *bs : *this->graphOutputProducers) {
      bs->setOutputConnector(connector);
      if (increment)
          connector->incrementInputTaskCount();
    }

    this->output = connector;
  }

  /**
   * Gets the output producers associated with this TaskGraph
   * @return the output producers for the TaskGraph
   */
  std::list<BaseTaskScheduler *> *getOutputProducers() {
    return this->graphOutputProducers;
  }


  /**
   * Adds a graph input consumer to process the input of the TaskGraph
   * @param iTask the ITask that will process the input
   */
  template<class V>
  void addGraphInputConsumer(ITask<T, V> *iTask) {
    TaskScheduler<T, V> * task = getTaskScheduler(iTask, false);
    addGraphInputConsumer(task);
  }

  /**
   * Adds a graph output producer to produce output for the TaskGraph
   * @param iTask the ITask that will produce output
   */
  template<class V>
  void addGraphOutputProducer(ITask<V, U> *iTask) {
    TaskScheduler<V, U> *task = getTaskScheduler(iTask, false);
    addGraphOutputProducer(task);
  }

  /**
   * Increments the number of producers sending data into the input for the TaskGraph.
   * @note Specify the TaskGraph input using addGraphInputConsumer() and when the input stream has finished producing data use finishedProducingData()
   */
  void incrementGraphInputProducer() {
    this->input->incrementInputTaskCount();
  }

  /**
   * Indicates that the input stream has finished producing data for the TaskGraph.
   * @note Must have a matching call to incrementGraphInputProducer()
   */
  void finishedProducingData() {
    this->input->producerFinished();
    this->input->wakeupConsumer();
  }

  /**
   * Creates a copy of the TaskGraph with the specified pipeline Id and number of pipelines.
   * The copy contains the same structure as the original TaskGraph, but with new instances of
   * every Task, ITask, and Connector
   * @param pipelineId the pipeline Id
   * @param numPipelines the number of pipelines
   * @return a copy of the TaskGraph
   */
  TaskGraph<T, U> *copy(int pipelineId, int numPipelines) {

    TaskGraph<T, U> *graphCopy = new TaskGraph<T, U>();

    // Pass the reference of the mem releaser and mmTypeMap to the copy
    graphCopy->setMMTypeMap(this->mmTypeMap);
    graphCopy->setMemReleaser(this->memReleaser);
    graphCopy->setPipelineId(pipelineId);

    std::map<BaseTaskScheduler *, BaseTaskScheduler *> copyMap;
    std::map<BaseITask *, BaseITask *> copyITaskMap;

    // Create a clone map to lookup pointers
    for (BaseTaskScheduler *task : *this->vertices) {
      BaseTaskScheduler *taskCopy = task->copy(false);
      taskCopy->setPipelineId(pipelineId);
      taskCopy->setNumPipelines(numPipelines);
      copyMap.insert(std::pair<BaseTaskScheduler *, BaseTaskScheduler *>(task, taskCopy));
      copyITaskMap.insert(std::pair<BaseITask *, BaseITask *>(task->getTaskFunction(), taskCopy->getTaskFunction()));
    }

    // Copy producer consumers
    for (ProducerConsumerKey *prConKey : *this->producerConsumerKeys) {
      BaseTaskScheduler *producer = prConKey->getProducer();
      BaseTaskScheduler *consumer = prConKey->getConsumer();

      BaseTaskScheduler *producerCopy = copyMap.find(producer)->second;
      BaseTaskScheduler *consumerCopy = copyMap.find(consumer)->second;

      graphCopy->addEdge(producerCopy, consumerCopy, producer->getOutputBaseConnector());

      consumerCopy->addPipelineConnector(pipelineId);
    }

    // Copy bookkeepers
    for (BookkeeperKey *bkKey : *this->bookkeeperKeys) {
      BaseTaskScheduler *bkTask = bkKey->getBkTask();
      BaseBaseRuleManager *ruleMan = bkKey->getRuleMan();
      BaseTaskScheduler *outputTask = bkKey->getOutputTask();

      BaseTaskScheduler *outputTaskCopy = copyMap.find(outputTask)->second;
      BaseTaskScheduler *bkTaskCopy = copyMap.find(bkTask)->second;
      BaseBaseRuleManager *ruleManCopy = ruleMan->copy();
      BaseITask *bkITask = bkTaskCopy->getTaskFunction();

      graphCopy->addRuleManager(bkTaskCopy, bkITask, ruleManCopy, outputTaskCopy, ruleMan->getConnector());

      outputTaskCopy->addPipelineConnector(pipelineId);

    }

    for (CustomEdgePair cep : *customEdgeList) {
      BaseTaskScheduler *producerTask = cep.first.first;
      BaseTaskScheduler *consumerTask = cep.first.second;

      ICustomEdge *customEdge = cep.second;

      // Find copy
      BaseTaskScheduler *producerTaskCopy = copyMap.find(producerTask)->second;
      BaseTaskScheduler *consumerTaskCopy = copyMap.find(consumerTask)->second;

      ICustomEdge *customEdgeCopy = customEdge->copy();

      graphCopy->addCustomEdge(producerTaskCopy, consumerTaskCopy, customEdgeCopy, pipelineId);
    }


    // Copy graph input consumers
    for (BaseTaskScheduler *task : *this->graphInputConsumers) {
      BaseTaskScheduler *taskCopy = copyMap.find(task)->second;
      graphCopy->addGraphInputConsumer(taskCopy);
    }

    // Copy graph output producers
    for (BaseTaskScheduler *task : *this->graphOutputProducers) {
      BaseTaskScheduler *taskCopy = copyMap.find(task)->second;
      graphCopy->addGraphOutputProducer(taskCopy);
    }

    // Copy memory managers
    for (MemoryManagerKey *mmKey : *this->memoryManagerKeys) {
      std::string name = mmKey->getName();
      BaseITask *memGetter = mmKey->getMemGetter();
      BaseITask *memReleaser = mmKey->getMemReleaser();
      BaseTaskScheduler *memTask = mmKey->getMemTask();

      BaseITask *memGetterCopy = nullptr;
      if (copyITaskMap.find(memGetter) != copyITaskMap.end())
        memGetterCopy = copyITaskMap.find(memGetter)->second;
      else
        memGetterCopy = memGetter;

      BaseITask *memReleaserCopy = nullptr;
      if (memReleaser != nullptr) {
        if (copyITaskMap.find(memReleaser) != copyITaskMap.end())
          memReleaserCopy = copyITaskMap.find(memReleaser)->second;
        else
          memReleaserCopy = memReleaser;
      }


      BaseTaskScheduler *memTaskCopy = copyMap.find(memTask)->second;


      if (memReleaserCopy == nullptr)
      {
        graphCopy->addGraphMemoryManagerEdge(name, memGetterCopy, memTaskCopy,
                                             memTask->getInputBaseConnector(), mmKey->getMMType());
      }
      else {
        graphCopy->addMemoryManagerEdge(name, memGetterCopy, memReleaserCopy, memTaskCopy,
                                        memTask->getInputBaseConnector(), mmKey->getMMType(), mmKey->isIsReleaserOutsideGraph());
      }
      memTaskCopy->addPipelineConnector(pipelineId);
    }


    return graphCopy;
  };

  /**
   * Adds an edge to the TaskGraph.
   * The output type of the produce must match the input type of the consumer. This will create two TaskScheduler
   * vertices (or lookup an existing vertex for the producer or consumer if that vertex already exists in the graph) and
   * connect the two vertices through a Connector.
   * @param producer the producer ITask
   * @param consumer the consumer ITask
   */
  template<class V, class W, class X>
  void addEdge(ITask<V, W> *producer, ITask<W, X> *consumer) {
    TaskScheduler<V, W> *producerTask = getTaskScheduler(producer, false);
    TaskScheduler<W, X> *consumerTask = getTaskScheduler(consumer, false);

    addEdge(producerTask, consumerTask);

  };

  /**
   * Adds an IRule to the TaskGraph connecting a Bookkeeper with a consumer ITask.
   * The Bookkeeper type must match the input of the IRule and
   * the output type of the IRule must match the input type of the ITask consumer
   * @param bk the Bookkeeper
   * @param consumer the ITask consumer
   * @param rule the IRule to connect the Bookkeeper and the consumer ITask
   */
  template<class V, class W, class X>
  void addRule(Bookkeeper<V> *bk, ITask<W, X> *consumer, IRule<V, W> * rule) {
    TaskScheduler<V, VoidData> *bkTask = getTaskScheduler(bk, false);
    TaskScheduler<W, X> *consumerTask = getTaskScheduler(consumer, false);
    RuleManager<V, W> *ruleMan = getRuleManager(bk, consumer);
    std::shared_ptr<IRule<V, W>> iRuleShr = getIRule(rule);

    ruleMan->addRule(iRuleShr);

    addRuleManager(bkTask, bk, ruleMan, consumerTask);
  }

  /**
   * Adds a custom edge to the TaskGraph.
   * The ICustomEdge represents an interface to describe how to add the edge to the graph.
   * @param customEdge the custom edge to be added
   */
  void addCustomEdge(ICustomEdge *customEdge) {
    BaseTaskScheduler *producerTask = nullptr;

    if (this->iTaskMap->find(customEdge->getProducerITask()) != this->iTaskMap->end()) {
      producerTask = this->iTaskMap->find(customEdge->getProducerITask())->second;
    }
    else {
      producerTask = customEdge->createProducerTask();

      if (producerTask == nullptr) {
        std::cerr
            << "Custom edge defines the producer as nullptr, this may require that the producer should already exist in the graph"
            << std::endl;
        throw std::invalid_argument("Must add producer iTask to graph before adding as custom edge");
      }

      this->iTaskMap->insert(ITaskPair(customEdge->getProducerITask(), producerTask));
    }

    BaseTaskScheduler *consumerTask = nullptr;
    if (this->iTaskMap->find(customEdge->getConsumerITask()) != this->iTaskMap->end()) {
      consumerTask = this->iTaskMap->find(customEdge->getConsumerITask())->second;
    }
    else {
      consumerTask = customEdge->createConsumerTask();

      if (consumerTask == nullptr) {
        std::cerr
            << "Custom edge defines the consumer as nullptr, this may require that the consumer should already exist in the graph"
            << std::endl;
        throw std::invalid_argument("Must add consumer iTask to graph before adding as custom edge");
      }

      this->iTaskMap->insert(ITaskPair(customEdge->getConsumerITask(), consumerTask));
    }

    std::shared_ptr<BaseConnector> connector = nullptr;
    if (customEdge->useConnector()) {
      if (consumerTaskConnectorMap->find(consumerTask) != this->consumerTaskConnectorMap->end()) {
        connector = this->consumerTaskConnectorMap->find(consumerTask)->second;
      }
      else {
        connector = std::shared_ptr<BaseConnector>(customEdge->createConnector());
        consumerTaskConnectorMap->insert(ConnectorPair(consumerTask, connector));
      }
    }

    std::pair<BaseTaskScheduler *, BaseTaskScheduler *> prodConsPair(producerTask, consumerTask);

    CustomEdgePair testPair(prodConsPair, customEdge);
    auto i = std::find_if(customEdgeList->begin(), customEdgeList->end(), CustomEdgeComparator(testPair));

    if (i == customEdgeList->end()) {
      this->customEdgeList->push_back(testPair);
    }

    customEdge->applyGraphConnection(producerTask, consumerTask, connector, 0, this);

    updateGraphHistory(producerTask, false, customEdge->useConnector());
    updateGraphHistory(consumerTask, customEdge->useConnector(), false);
  }

  /**
   * Adds a MemoryManager edge with the specified name to the TaskGraph, which connects a memory getter and releaser.
   * This function provides the ability to specify a custom type of memory manager.
   * @param memGetter the ITask that is getting memory
   * @param memReleaser the ITask that is releasing memory
   * @param memoryManager the memory manager responsible for sending/recycling memory
   * @param ignoreMemGetterErrors whether to ignore errors with the memory getter, used if the memory getter is already connected
   * to the same memory manager with the same edge name
   * @note the memoryPoolSize can cause out of memory errors for the system if the allocator->size() * memoryPoolSize exceeds the total system memory
   * @tparam V the type of memory; i.e., 'double *'
   */
  template <class V>
  void addMemoryManagerEdge(BaseITask *memGetter, BaseITask *memReleaser, MemoryManager<V> *memoryManager, bool ignoreMemGetterErrors)
  {
    // Check if the Memory Manager task exists or not
    // If it does then get that task and do not create connectors
    TaskScheduler<MemoryData<V>, MemoryData<V>> *memTask = getTaskScheduler(memoryManager, true);

    bool isReleaserOutsideGraph;
    if (hasITask(memReleaser))
      isReleaserOutsideGraph = false;
    else
      isReleaserOutsideGraph = true;

    attachMemGetter(memoryManager->getMemoryManagerName(), memGetter, memoryManager->getType(), memTask->getOutputBaseConnector(), ignoreMemGetterErrors);
    attachMemReleaser(memoryManager->getMemoryManagerName(), memReleaser, memoryManager->getType(), memTask->getInputBaseConnector(), isReleaserOutsideGraph);

    memTask->getInputBaseConnector()->incrementInputTaskCount();

    updateGraphHistory(memTask, true, true);

    // Save this information for copy
    this->memoryManagerKeys->push_back(new MemoryManagerKey(memoryManager->getMemoryManagerName(), memGetter, memReleaser, memTask, memoryManager->getType(), isReleaserOutsideGraph));
  }

#ifdef USE_CUDA
  /**
   * Adds a CudaMemoryManager edge with the specified name to the TaskGraph.
   * This will create a CudaMemoryManager that is bound to some Cuda GPU based on the pipelineId of
   * the TaskGraph.
   * @param name the name of the memory edge
   * @param memGetter the ITask that is getting memory
   * @param memReleaser the ITask that is releasing memory
   * @param allocator the allocator describing how memory is allocated (should allocate Cuda memory)
   * @param memoryPoolSize the size of the memory pool that is allocated by the CudaMemoryManager
   * @param type the type of memory manager
   * @param contexts the array of all Cuda contexts
   * @note the memoryPoolSize can cause out of memory errors for the GPU if the allocator->size() * memoryPoolSize exceeds the total GPU memory
   * @tparam V the type of memory; i.e. 'cufftDoubleComplex *'
   */
  template <class V>
  void addCudaMemoryManagerEdge(std::string name, BaseITask *memGetter, BaseITask *memReleaser,
          IMemoryAllocator<V> *allocator, int memoryPoolSize, MMType type, CUcontext * contexts) {

      if (!hasITask(memGetter)) {
          std::cerr << " Must add iTask " << memGetter->getName() << " to graph before adding it as a memory edge" << std::endl;
          throw std::invalid_argument("Must add iTask to graph before adding as memory edge");
      }

      std::shared_ptr<IMemoryAllocator<V>> allocP = getMemoryAllocator(allocator);

      bool ignoreMemGetterErrors;
      CudaMemoryManager<V> *memManager = getCudaMemoryManager(memGetter, name, memoryPoolSize, allocP, type, contexts, &ignoreMemGetterErrors);

      addMemoryManagerEdge(memGetter, memReleaser, memManager, ignoreMemGetterErrors);
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
                                       BaseITask *memGetter,
                                       BaseITask *memReleaser,
                                       int memoryPoolSize) {
    addMemoryManagerEdge(name, memGetter, memReleaser, new VoidMemoryAllocator(), memoryPoolSize, MMType::UserManaged);
  }

  /**
   * Adds a MemoryManager edge with the specified name to the TaskGraph.
   * @param name the name of the memory edge
   * @param memGetter the ITask that is getting memory
   * @param memReleaser the ITask that is releasing memory
   * @param allocator the allocator describing how memory is allocated (should allocate Cuda memory)
   * @param memoryPoolSize the size of the memory pool that is allocated by the CudaMemoryManager
   * @param type the type of memory manager
   * @note the memoryPoolSize can cause out of memory errors for the system if the allocator->size() * memoryPoolSize exceeds the total system memory
   * @tparam V the type of memory; i.e., 'double *'
   */
  template<class V>
  void addMemoryManagerEdge(std::string name, BaseITask *memGetter, BaseITask *memReleaser,
                            IMemoryAllocator<V> *allocator, int memoryPoolSize, MMType type) {

    if (!hasITask(memGetter)) {
      std::cerr << " Must add iTask " << memGetter->getName() << " to graph before adding it to a memory edge"
          << std::endl;
      throw std::invalid_argument("Must add iTask to graph before adding to memory edge");
    }

    std::shared_ptr<IMemoryAllocator<V>> allocP = getMemoryAllocator(allocator);

    // mem getter errors occur when trying to add a memory edge to a mem getter that already contains the named edge
    bool ignoreMemGetterErrors;

    MemoryManager<V> *memManager = getMemoryManager(memGetter, name, memoryPoolSize, allocP, type, &ignoreMemGetterErrors);

    addMemoryManagerEdge(memGetter, memReleaser, memManager, ignoreMemGetterErrors);
  }

  /**
  * Adds a MemoryManager that is managed by the user with this graph as the memory releaser.
  * This edge will enable an ITask to use the MemoryManager to throttle how much data is allocated.
  * The throttling is achieved by adding MemoryData back to the TaskGraph
  * @param name the name of the memory edge
  * @param memGetter the ITask that will be getting memory
  * @param memoryPoolSize the size of the memory pool
  */
  void addGraphUserManagedMemoryManagerEdge(std::string name,
                                       BaseITask *memGetter,
                                       int memoryPoolSize) {
    addGraphMemoryManagerEdge(name, memGetter, new VoidMemoryAllocator(), memoryPoolSize, MMType::UserManaged);
  }

  /**
   * Adds a MemoryManager edge with the specified name to the TaskGraph.
   * The TaskGraph is used to release memory using the TaskGraph::memRelease function.
   * @param name the name of the memory edge
   * @param memGetter the ITask that is getting memory
   * @param allocator the allocator describing how memory is allocated (should allocate Cuda memory)
   * @param memoryPoolSize the size of the memory pool that is allocated by the CudaMemoryManager
   * @param type the type of memory manager
   * @note the memoryPoolSize can cause out of memory errors for the system if the allocator->size() * memoryPoolSize exceeds the total system memory
   * @tparam V the type of memory; i.e., 'double *'
   */
  template <class V>
  void addGraphMemoryManagerEdge(std::string name, BaseITask *memGetter,
                                 IMemoryAllocator<V> *allocator, int memoryPoolSize, MMType type)
  {
    if (!hasITask(memGetter)) {
      std::cerr << " Must add iTask " << memGetter->getName() << " to graph before adding it to a memory edge"
          << std::endl;
      throw std::invalid_argument("Must add iTask to graph before adding to memory edge");
    }

    std::shared_ptr<IMemoryAllocator<V>> allocP = getMemoryAllocator(allocator);

    // mem getter errors occur when trying to add a memory edge to a mem getter that already contains the named edge
    bool ignoreMemGetterErrors = false;

    MemoryManager<V> *memManager = getMemoryManager(memGetter, name, memoryPoolSize, allocP, type, &ignoreMemGetterErrors);

    TaskScheduler<MemoryData<V>, MemoryData<V>> *memTask = getTaskScheduler(memManager, true);

    attachMemGetter(name, memGetter, type, memTask->getOutputBaseConnector(), ignoreMemGetterErrors);

    DEBUG("Adding memory releaser " << name << " to " << "TaskGraph"<< " " << memReleaser <<
        " at connector " <<
        memTask->getInputBaseConnector());

    std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> vector;

    if (hasMemReleaser(name))
      vector = memReleaser->find(name)->second;
    else
      vector = std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>>(new std::vector<std::shared_ptr<BaseConnector>>());

    vector->push_back(memTask->getInputBaseConnector());

    mmTypeMap->insert(std::pair<std::string, MMType>(name, type));
    memReleaser->insert(std::pair<std::string, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>>>(name, vector));

    memTask->getInputBaseConnector()->incrementInputTaskCount();

    updateGraphHistory(memTask, true, true);

    this->memoryManagerKeys->push_back(new MemoryManagerKey(name, memGetter, nullptr, memTask, type, false));
  }

  /**
 * Checks whether this ITask contains a memReleaser for a specified name
 * @param name the name of the memReleaser
 * @return whether this ITask has a memReleaser with the specified name
 * @retval TRUE if the ITask has a memReleaser with the specified name
 * @retval FALSE if the ITask does not have a memReleaser with the specified name
 * @note To add a memReleaser to this TaskGraph use TaskGraph::addGraphMemoryManagerEdge
 */
  bool hasMemReleaser(std::string name) {
    return memReleaser->find(name) != memReleaser->end();
  }

  /**
 * Releases memory onto a memory edge
 * @param name the name of the memory edge
 * @param memory the memory to be released
 * @tparam V the MemoryData type
 * @note The name specified must have been attached to this ITask as a memReleaser using
 * the TaskGraph::addMemoryManagerEdge routine, which can be verified using hasMemReleaser()
 * @note Memory edge must be defined as MMType::Static OR MMType::Dynamic
 */
  template<class V>
  void memRelease(std::string name, std::shared_ptr<MemoryData<V>> memory) {
    assert(this->mmTypeMap->find(name)->second == MMType::Static
               || this->mmTypeMap->find(name)->second == MMType::Dynamic);
    std::shared_ptr<BaseConnector> conn = memReleaser->find(name)->second->at((unsigned long) memory->getPipelineId());
    std::shared_ptr<Connector<MemoryData<V>>> connector = std::dynamic_pointer_cast<Connector<MemoryData<V>>>(conn);
    connector->produceData(memory);
  }

  /**
   * Releases memory onto a memory edge
   * @param name the name of the memory edge
   * @param pipelineId the pipelineId to add data to
   * @note The name specified must have been attached to this ITask as a memReleaser using
   * the TaskGraph::addUserManagedEdge routine, which can be verified using hasMemReleaser()
   * @note Memory edge must be defined as MMType::UserManaged by using the TaskGraph::addUserManagedMemoryManagerEdge
   */
  void memRelease(std::string name, int pipelineId) {
    assert(this->mmTypeMap->find(name)->second == MMType::UserManaged);

    std::shared_ptr<MemoryData<void *>> memory(new MemoryData<void *>(nullptr));
    memory->setPipelineId(pipelineId);

    std::shared_ptr<BaseConnector> conn = memReleaser->find(name)->second->at((unsigned long) memory->getPipelineId());
    std::shared_ptr<Connector<MemoryData<void *>>> connector = std::dynamic_pointer_cast<Connector<MemoryData<void *>>>(conn);
    connector->produceData(memory);
  }

  /**
   * Indicates to the memory managers that are managing memory between the TaskGraph and ITask's created from TaskGraph::addGraphMemoryManager
   * can be shut down.
   * Use this function from the thread that is interacting with the TaskGraph's memory manager to ensure the memory managers
   * are shutting down.
   */
  void finishReleasingMemory()
  {
    DEBUG("TaskGraph" << " Shutting down " << memReleaser->size() <<
        " memory releasers");
    for (std::pair<std::string, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> > pair : *this->memReleaser) {
      DEBUG("TaskGraph " << " Shutting down memory releaser : " <<
          pair.first << " with " << pair.second->size() << " connectors");
      for (std::shared_ptr<BaseConnector> connector : *pair.second)
      {
        connector->producerFinished();

        if (connector->isInputTerminated())
          connector->wakeupConsumer();
      }


    }
  }

  /**
   * Gets the edges (Connector) associated with the TaskGraph
   * @return the list of connectors
   */
  std::list<std::shared_ptr<BaseConnector>> *getEdges() const {
    return this->edges;
  }

  /**
   * Gets the vertices (BaseTaskScheduler) associated with the TaskGraph
   * @return the list of tasks
   */
  std::list<BaseTaskScheduler *> *getVertices() const {
    return this->vertices;
  }

  /**
   * Pure virtual function to add a copy of a TaskScheduler into the TaskGraph.
   * @param taskCopy the task that was copied.
   */
  void addTaskCopy(BaseTaskScheduler *taskCopy)
  {
    if (std::find(vertices->begin(), vertices->end(), taskCopy) == vertices->end()) {
      this->vertices->push_back(taskCopy);
    }
  }

  /**
   * Provides debug output for the TaskGraph
   * @note \#define DEBUG_FLAG to enable debugging
   */
  void debug() const {

    DEBUG("-----------------------------------------------");
    DEBUG("TaskGraph -- num vertices: " << vertices->size() << " num edges: " << edges->size() <<
        " -- DETAILS:");

    for (BaseTaskScheduler *t : *vertices) {
      t->debug();
    }

    for (std::shared_ptr<BaseConnector> c : *edges) {
      DEBUG("Connector: " << c << " total producers: " << c->getProducerCount());
    }

    DEBUG("Graph input: " << this->input << " total producers: " <<
        (input ? this->input->getProducerCount() : 0));
    DEBUG("Graph output: " << this->output << " total producers: " <<
        (output ? this->output->getProducerCount() : 0));
    DEBUG("-----------------------------------------------");
  }

  /**
   * Processes all of the input for the TaskGraph until no more input is available
   * @note \#define DEBUG_FLAG to enable debugging
   */
  void debugInput() {
    while (!this->input->isInputTerminated())
      DEBUG("data = " << this->input->consumeData() << std::endl);
  }

  /**
   * @internal
   * Updates the pipelineIds and the number of pipelines for all tasks in the TaskGraph
   * @param pipelineId the pipeline Id
   * @param numPipelines the number of pipelines
   *
   * @note This function should only be called by the HTGS API
   */
  void updateIdAndNumPipelines(int pipelineId, int numPipelines) {
    for (BaseTaskScheduler *t : *this->vertices) {
      t->setPipelineId(pipelineId);
      t->setNumPipelines(numPipelines);
      t->addPipelineConnector(pipelineId);
    }
  }

  /**
   * Gets the input connector for the TaskGraph
   * @return the input connector
   */
  std::shared_ptr<Connector<T>> getInputConnector() {
    return this->input;
  }

  /**
   * Gets the output connector for the TaskGraph
   * @return the output connector
   */
  std::shared_ptr<Connector<U>> getOutputConnector() {
    return this->output;
  }
  /**
   * Sets the memory manager type map
   * @param pMap the shared pointer to the memory manager type map
   */
  void setMMTypeMap(std::shared_ptr<std::unordered_map<std::string, MMType>> pMap) { this->mmTypeMap = pMap; }

  /**
   * Sets the memory releaser mapping
   * @param memReleaser the memory releaser map
   */
  void setMemReleaser(std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> >> memReleaser) {
    this->memReleaser = memReleaser;
  }

  /**
   * Sets the pipeline Id that the TaskGraph is bound to.
   * @param pipelineId the pipelineId
   */
  void setPipelineId(int pipelineId) { this->pipelineId = pipelineId; }


 private:

  //! @cond Doxygen_Suppress
  template<class V, class W, class X>
  void addEdge(TaskScheduler<V, W> *producer, TaskScheduler<W, X> *consumer) {
    std::shared_ptr<Connector<W>> connector;

    if (this->consumerTaskConnectorMap->find(consumer) != this->consumerTaskConnectorMap->end()) {
      connector = std::dynamic_pointer_cast<Connector<W>>(this->consumerTaskConnectorMap->find(consumer)->second);
      producer->setOutputConnector(connector);
      connector->incrementInputTaskCount();

      this->consumerTaskConnectorMap->insert(ConnectorPair(consumer, connector));
    }
    else {
      connector = std::shared_ptr<Connector<W>>(new Connector<W>());
      producer->setOutputConnector(connector);
      consumer->setInputConnector(connector);
      connector->incrementInputTaskCount();

      this->consumerTaskConnectorMap->insert(ConnectorPair(consumer, connector));
    }

    updateGraphHistory(producer, false, true);
    updateGraphHistory(consumer, true, false);

    this->producerConsumerKeys->push_back(new ProducerConsumerKey(producer, consumer));
  }


  template<class V, class W, class X>
  void addRuleManager(TaskScheduler<V, VoidData> *bkTask, Bookkeeper<V> *bk, RuleManager<V, W> *ruleMan,
                      TaskScheduler<W, X> *consumerTask) {
    // Add rule to bookkeeper
    bk->addRuleManager(ruleMan);

    std::shared_ptr<Connector<W>> connector;
    if (consumerTaskConnectorMap->find(consumerTask) != this->consumerTaskConnectorMap->end()) {
      connector = std::dynamic_pointer_cast<Connector<W>> (this->consumerTaskConnectorMap->find(consumerTask)->second);
      ruleMan->setOutputConnector(connector);
    }
    else {
      connector = std::shared_ptr<Connector<W>>(new Connector<W>());
      ruleMan->setOutputConnector(connector);
      consumerTask->setInputConnector(connector);

      consumerTaskConnectorMap->insert(ConnectorPair(consumerTask, connector));
    }

    updateGraphHistory(bkTask, false, false);
    updateGraphHistory(consumerTask, true, false);

    std::pair<BaseITask *, BaseITask *> ruleEdge(bk, consumerTask->getTaskFunction());
    if (this->ruleEdgeMap->find(ruleEdge) == this->ruleEdgeMap->end()) {
      this->ruleEdgeMap->insert(RuleEdgePair(ruleEdge, ruleMan));
    }

    this->bookkeeperKeys->push_back(new BookkeeperKey(bkTask, bk, ruleMan, consumerTask));
  }


  void addCustomEdge(BaseTaskScheduler *producerTask, BaseTaskScheduler *consumerTask, ICustomEdge *customEdge, int pipelineId) {

    std::shared_ptr<BaseConnector> connector = nullptr;
    if (customEdge->useConnector()) {
      if (consumerTaskConnectorMap->find(consumerTask) != this->consumerTaskConnectorMap->end()) {
        connector = this->consumerTaskConnectorMap->find(consumerTask)->second;
      }
      else {
        connector = std::shared_ptr<BaseConnector>(customEdge->createConnector());
        consumerTaskConnectorMap->insert(ConnectorPair(consumerTask, connector));
      }
    }

    std::pair<BaseTaskScheduler *, BaseTaskScheduler *> prodConsPair(producerTask, consumerTask);
    CustomEdgePair testPair(prodConsPair, customEdge);
    auto i = std::find_if(customEdgeList->begin(), customEdgeList->end(), CustomEdgeComparator(testPair));

    if (i == customEdgeList->end())
      this->customEdgeList->push_back(testPair);

    customEdge->applyGraphConnection(producerTask, consumerTask, connector, pipelineId, this);

    updateGraphHistory(producerTask, false, customEdge->useConnector());
    updateGraphHistory(consumerTask, customEdge->useConnector(), false);
  }

  void addGraphInputConsumer(BaseTaskScheduler *task) {
    DEBUG_VERBOSE("Adding task graph input consumer: " << task << " to input connector " << this->input);

    std::shared_ptr<Connector<T>> connector;

    // If the task consuming is already in the graph, then use that task's connector
    if (this->consumerTaskConnectorMap->find(task) != this->consumerTaskConnectorMap->end()) {
      connector = std::dynamic_pointer_cast<Connector<T>>(this->consumerTaskConnectorMap->find(task)->second);
      this->input = connector;
      this->consumerTaskConnectorMap->insert(ConnectorPair(task, connector));
    }
    else {
      // task not found, use the graph's input
      connector = this->input;
      task->setInputConnector(connector);
      this->consumerTaskConnectorMap->insert(ConnectorPair(task, connector));
    }

    this->graphInputConsumers->push_back(task);

    updateGraphHistory(task, true, false);
  }

  void addGraphOutputProducer(BaseTaskScheduler *task) {
    DEBUG_VERBOSE("Adding task graph output producer: " << task << " to output connector " << this->output);

    task->setOutputConnector(output);
    output->incrementInputTaskCount();

    this->graphOutputProducers->push_back(task);

    updateGraphHistory(task, false, true);
  }

  void addGraphMemoryManagerEdge(std::string name, BaseITask *memGetter, BaseTaskScheduler *memTask,
                                 std::shared_ptr<BaseConnector> connector, MMType type)
  {
    // Check to make sure the memory edge's memgetter is already in the graph
    if (!hasITask(memGetter)) {
      std::cerr << " Must add iTask " << memGetter->getName() << " to graph before adding it to a memory edge"
          << std::endl;
      throw std::invalid_argument("Must add iTask to graph before adding to memory edge");
    }

    std::shared_ptr<BaseConnector> inputConnector;
    std::shared_ptr<BaseConnector> outputConnector;

    // If the memManager is not in this graph, then create a new one and update the memTask
    if (std::find(this->vertices->begin(), this->vertices->end(), memTask) == this->vertices->end()) {
      inputConnector = std::shared_ptr<BaseConnector>(connector->copy());
      outputConnector = std::shared_ptr<BaseConnector>(connector->copy());

      memTask->setInputConnector(inputConnector);
      memTask->setOutputConnector(outputConnector);
      outputConnector->incrementInputTaskCount();
    }
    else
    {
      // memManager task is already in the graph, so its connectors should exist
      inputConnector = memTask->getInputBaseConnector();
      outputConnector = memTask->getOutputBaseConnector();
    }

    DEBUG("Adding memory getter " << name << " to " << memGetter->getName() << " " << memGetter <<
        " at connector " << memTask->getOutputBaseConnector());

    memGetter->attachMemGetter(name, outputConnector, type);

    DEBUG("Adding memory releaser " << name << " to " << "TaskGraph"<< " " << memReleaser <<
        " at connector " << memTask->getInputBaseConnector());

    std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>> vector;

    // Get the vector of connectors for the given edge name (1 pipeline per vector element)
    if (hasMemReleaser(name))
      vector = memReleaser->find(name)->second;
    else
      vector = std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>>(new std::vector<std::shared_ptr<BaseConnector>>());

    // Add the connector
    vector->push_back(memTask->getInputBaseConnector());

    mmTypeMap->insert(std::pair<std::string, MMType>(name, type));
    memReleaser->insert(std::pair<std::string, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>>>(name, vector));

    inputConnector->incrementInputTaskCount();

    updateGraphHistory(memTask, true, true);

    this->memoryManagerKeys->push_back(new MemoryManagerKey(name, memGetter, nullptr, memTask, type, false));
  }

  void addMemoryManagerEdge(std::string name, BaseITask *memGetter, BaseITask *memReleaser, BaseTaskScheduler *memTask,
                            std::shared_ptr<BaseConnector> connector, MMType type, bool isReleaserOutsideGraph) {
    if (!hasITask(memGetter)) {
      std::cerr << " Must add iTask " << memGetter->getName() << " to graph before adding it as a memory edge"
          << std::endl;
      throw std::invalid_argument("Must add iTask to graph before adding as memory edge");
    }
    std::shared_ptr<BaseConnector> inputConnector;
    std::shared_ptr<BaseConnector> outputConnector;

    // If the memManager is not in this graph, then create a new one and update the memTask
    if (std::find(this->vertices->begin(), this->vertices->end(), memTask) == this->vertices->end()) {
      inputConnector = std::shared_ptr<BaseConnector>(connector->copy());
      outputConnector = std::shared_ptr<BaseConnector>(connector->copy());

      memTask->setInputConnector(inputConnector);
      memTask->setOutputConnector(outputConnector);
      outputConnector->incrementInputTaskCount();
    }
    else
    {
      // memManager task is already in the graph, so its connectors should exist
      inputConnector = memTask->getInputBaseConnector();
      outputConnector = memTask->getOutputBaseConnector();
    }

    DEBUG("Adding memory getter " << name << " to " << memGetter->getName() << " " << memGetter <<
        " at connector " << memTask->getOutputBaseConnector());
    DEBUG("Adding memory releaser " << name << " to " << memReleaser->getName() << " " << memReleaser <<
        " at connector " << memTask->getInputBaseConnector());

    // This functionality is internal to copying a graph, so inserting the memGetter/releaser with the same name is desired
    // One per pipeline
    memGetter->attachMemGetter(name, outputConnector, type);
    memReleaser->attachMemReleaser(name, inputConnector, type, isReleaserOutsideGraph);
    inputConnector->incrementInputTaskCount();

    updateGraphHistory(memTask, true, true);

    // Save this information for copy
    this->memoryManagerKeys->push_back(new MemoryManagerKey(name, memGetter, memReleaser, memTask, type, isReleaserOutsideGraph));
  }

  void addRuleManager(BaseTaskScheduler *bkTask,
                      BaseITask *bk,
                      BaseBaseRuleManager *ruleMan,
                      BaseTaskScheduler *consumerTask,
                      std::shared_ptr<BaseConnector> origConnector) {
    // Add rule to bookkeeper
    bk->addRuleManager(ruleMan);

    std::shared_ptr<BaseConnector> connector;
    if (consumerTaskConnectorMap->find(consumerTask) != this->consumerTaskConnectorMap->end()) {
      connector = this->consumerTaskConnectorMap->find(consumerTask)->second;
      ruleMan->setOutputConnector(connector);
    }
    else {
      connector = std::shared_ptr<BaseConnector>(origConnector->copy());
      ruleMan->setOutputConnector(connector);

      consumerTask->setInputConnector(connector);
    }

    updateGraphHistory(bkTask, false, false);
    updateGraphHistory(consumerTask, true, false);

    this->bookkeeperKeys->push_back(new BookkeeperKey(bkTask, bk, ruleMan, consumerTask));
  }


  void addEdge(BaseTaskScheduler *producer, BaseTaskScheduler *consumer, std::shared_ptr<BaseConnector> origConnector) {
    std::shared_ptr<BaseConnector> connector;

    if (this->consumerTaskConnectorMap->find(consumer) != this->consumerTaskConnectorMap->end()) {
      connector = this->consumerTaskConnectorMap->find(consumer)->second;

      producer->setOutputConnector(connector);
      connector->incrementInputTaskCount();

      this->consumerTaskConnectorMap->insert(ConnectorPair(consumer, connector));
    }
    else {
      connector = std::shared_ptr<BaseConnector>(origConnector->copy());

      producer->setOutputConnector(connector);
      consumer->setInputConnector(connector);
      connector->incrementInputTaskCount();

      this->consumerTaskConnectorMap->insert(ConnectorPair(consumer, connector));
    }

    updateGraphHistory(producer, false, true);
    updateGraphHistory(consumer, true, false);

    this->producerConsumerKeys->push_back(new ProducerConsumerKey(producer, consumer));
  }

  bool hasITask(BaseITask *task)
  {
    return this->iTaskMap->find(task) != this->iTaskMap->end();
  }

  template <class V>
  std::shared_ptr<IMemoryAllocator<V>> getMemoryAllocator(IMemoryAllocator<V> *allocator)
  {
    std::shared_ptr<IMemoryAllocator<V>> allocP;

    if (this->memAllocMap->find(allocator) == this->memAllocMap->end())
    {
      allocP = std::shared_ptr<IMemoryAllocator<V>>(allocator);
      memAllocMap->insert(MemAllocPair(allocator, allocP));
    }
    else
    {
      allocP = std::dynamic_pointer_cast<IMemoryAllocator<V>>(this->memAllocMap->at(allocator));
    }

    return allocP;
  }

  template <class V>
  MemoryManager<V> *getMemoryManager(BaseITask *memGetter, std::string name, int memoryPoolSize, std::shared_ptr<IMemoryAllocator<V>> allocP, MMType type, bool *ignoreMemGetterErrors)
  {
    MemoryManager<V> *memManager;

    // Check if memGetter has a memory edge, if it does then check if the specified name is created
    if (memGetterMap->find(memGetter) == this->memGetterMap->end())
    {
      // The memGetter does not have any memory edges associated with it
      memManager = new MemoryManager<V>(name, memoryPoolSize, allocP, type);
      MemManagerMap *mmMap = new MemManagerMap();
      mmMap->insert(MemManagerPair(name, memManager));
      memGetterMap->insert(MemGetterPair(memGetter, mmMap));
      *ignoreMemGetterErrors = false;
    }
    else
    {
      // MemGetter has some memory edges
      // Now identify if the specified named edge already exists
      MemManagerMap *mmMap = memGetterMap->at(memGetter);
      if (mmMap->find(name) == mmMap->end())
      {
        // No memory manager found with name, create a new one and add it
        memManager = new MemoryManager<V>(name, memoryPoolSize, allocP, type);
        mmMap->insert(MemManagerPair(name, memManager));
        *ignoreMemGetterErrors = false;
      }
      else
      {
        // Memory manager found with name, reuse the stored memory manager
        memManager = (MemoryManager<V> *)mmMap->at(name);
        *ignoreMemGetterErrors = true;
      }
    }

    return memManager;
  }

#ifdef USE_CUDA
  template <class V>
  CudaMemoryManager<V> *getCudaMemoryManager(BaseITask *memGetter, std::string name, int memoryPoolSize, std::shared_ptr<IMemoryAllocator<V>> allocP, MMType type, CUcontext *contexts, bool *ignoreMemGetterErrors)
  {
    CudaMemoryManager<V> *memManager;

    // Check if memGetter has a memory edge, if it does then check if the specified name is created
    if (memGetterMap->find(memGetter) == this->memGetterMap->end())
    {
      // The memGetter does not have any memory edges associated with it
      memManager = new CudaMemoryManager<V>(name, contexts, memoryPoolSize, allocP, type);
      MemManagerMap *mmMap = new MemManagerMap();
      mmMap->insert(MemManagerPair(name, memManager));
      memGetterMap->insert(MemGetterPair(memGetter, mmMap));
      *ignoreMemGetterErrors = false;
    }
    else
    {
      // MemGetter has some memory edges
      // Now identify if the specified named edge already exists
      MemManagerMap *mmMap = memGetterMap->at(memGetter);
      if (mmMap->find(name) == mmMap->end())
      {
        // No memory manager found with name, create a new one and add it
        memManager = new CudaMemoryManager<V>(name, contexts, memoryPoolSize, allocP, type);
        mmMap->insert(MemManagerPair(name, memManager));
        *ignoreMemGetterErrors = false;
      }
      else
      {
        // Memory manager found with name, reuse the stored memory manager
        memManager = (CudaMemoryManager<V> *)mmMap->at(name);
        *ignoreMemGetterErrors = true;
      }
    }

    return memManager;
  }
#endif

  template <class V, class W>
  TaskScheduler<V, W> *getTaskScheduler(ITask<V, W> *iTask, bool attachConnectors)
  {
    TaskScheduler<V, W> *taskScheduler;
    if (this->iTaskMap->find(iTask) == this->iTaskMap->end())
    {
      taskScheduler = TaskScheduler<V, W>::createTask(iTask);
      this->iTaskMap->insert(ITaskPair(iTask, taskScheduler));

      if (attachConnectors)
      {
        std::shared_ptr<Connector<V>> inputConnector(new Connector<V>());
        std::shared_ptr<Connector<W>> outputConnector(new Connector<W>());

        taskScheduler->setOutputConnector(outputConnector);
        taskScheduler->setInputConnector(inputConnector);
        outputConnector->incrementInputTaskCount();

      }
    }
    else
    {
      taskScheduler = (TaskScheduler<V, W> *)this->iTaskMap->at(iTask);
    }

    return taskScheduler;
  }

  template <class V, class W, class X>
  RuleManager<V, W> *getRuleManager(Bookkeeper<V> *bk, ITask<W, X> *consumer)
  {
    RuleManager<V, W> *ruleMan;
    std::pair<BaseITask *, BaseITask *> ruleEdge(bk, consumer);
    if (this->ruleEdgeMap->find(ruleEdge) != this->ruleEdgeMap->end()) {
      ruleMan = (RuleManager<V, W> *) this->ruleEdgeMap->find(ruleEdge)->second;
    }
    else {
      ruleMan = new RuleManager<V, W>();
      this->ruleEdgeMap->insert(RuleEdgePair(ruleEdge, ruleMan));
    }

    return ruleMan;
  }

  template <class V, class W>
  std::shared_ptr<IRule<V, W>> getIRule(IRule<V, W> *iRule)
  {
    std::shared_ptr<IRule<V, W>> iRuleShr;
    if (this->iRuleMap->find(iRule) != this->iRuleMap->end()) {
      std::shared_ptr<BaseIRule> baseRulePtr = this->iRuleMap->find(iRule)->second;
      iRuleShr = std::dynamic_pointer_cast<IRule<V, W>>(baseRulePtr);
    }
    else{
      iRuleShr = std::shared_ptr<IRule<V, W>>(iRule);
      this->iRuleMap->insert(IRulePair(iRule, iRuleShr));
    }
    return iRuleShr;
  };

  void attachMemGetter(std::string name, BaseITask *memGetter, MMType type, std::shared_ptr<BaseConnector> connector, bool ignoreMemGetterErrors)
  {
    DEBUG("Adding memory getter " << name << " to " << memGetter->getName() << " " << memGetter <<
        " at connector " << connector);

    if (memGetter->hasMemGetter(name))
    {
      if (!ignoreMemGetterErrors) {
        std::cerr << "Error memory getter edge " << name << " already exists for task: " << memGetter->getName()
            << std::endl;
      }
      return;
    }

    memGetter->attachMemGetter(name, connector, type);
  }

  void attachMemReleaser(std::string name, BaseITask *memReleaser, MMType type, std::shared_ptr<BaseConnector> connector, bool isReleaserOutsideGraph)
  {
    DEBUG("Adding memory releaser " << name << " to " << memReleaser->getName() << " " << memReleaser <<
        " at connector " << connector);

    if (memReleaser->hasMemReleaser(name))
    {
      std::cerr << "Error memory releaser edge " << name << " already exists for task: " << memReleaser->getName() << std::endl;
      return;
    }

    memReleaser->attachMemReleaser(name, connector, type, isReleaserOutsideGraph);
  }

  void updateGraphHistory(BaseTaskScheduler *taskScheduler, bool addInputEdge, bool addOutputEdge)
  {
    if (taskScheduler == nullptr)
      return;

    if (std::find(vertices->begin(), vertices->end(), taskScheduler) == vertices->end()) {
      this->vertices->push_back(taskScheduler);
    }

    if (this->iTaskMap->find(taskScheduler->getTaskFunction()) == this->iTaskMap->end()) {
      this->iTaskMap->insert(ITaskPair(taskScheduler->getTaskFunction(), taskScheduler));
    }

    if (addInputEdge) {
      if (std::find(edges->begin(), edges->end(), taskScheduler->getInputBaseConnector()) == edges->end()) {
        this->edges->push_back(taskScheduler->getInputBaseConnector());
      }
    }

    if (addOutputEdge) {
      if (std::find(edges->begin(), edges->end(), taskScheduler->getOutputBaseConnector()) == edges->end()) {
        this->edges->push_back(taskScheduler->getOutputBaseConnector());
      }
    }
  }


  //! @endcond

  std::list<BaseTaskScheduler *>
      *graphInputConsumers; //!< The list of consumers accessing the TaskGraph's input connector
  std::list<BaseTaskScheduler *>
      *graphOutputProducers; //!< The list of producers that are outputting data to the TaskGraph's output connector

  std::list<std::shared_ptr<BaseConnector>> *edges; //!< The list of edges (Connector) for the TaskGraph
  std::list<BaseTaskScheduler *> *vertices; //!< The list of vertices (BaseTaskScheduler) for the TaskGraph

  std::list<ProducerConsumerKey *>
      *producerConsumerKeys; //!< A list of producer-consumer pairs to aid in copying a TaskGraph
  std::list<BookkeeperKey *> *bookkeeperKeys; //!< A list of bookkeeper-consumer pairs to aid in copying a TaskGraph
  std::list<MemoryManagerKey *>
      *memoryManagerKeys; //!< A list of memory manager memGetter-memReleaser pairs to aid in copying a TaskGraph

  ConnectorMap *consumerTaskConnectorMap; //!< A mapping of consumer BaseTaskScheduler to its associated Connector

  ITaskMap *iTaskMap; //!< A mapping for each BaseITask to its associated BaseTaskScheduler

  RuleEdgeMap *ruleEdgeMap; //!< A mapping for each bookkeeper-consumer pair to its associated RuleManager

  IRuleMap *iRuleMap; //!< A mapping for each IRule pointer to the shared pointer for that IRule

  std::list<CustomEdgePair> *customEdgeList; //!< A list of each producer-consumer pair to its associated ICustomEdge

  std::shared_ptr<Connector<T>> input; //!< The input connector for the TaskGraph
  std::shared_ptr<Connector<U>> output; //!< The output connector for the TaskGraph

  MemAllocMap *memAllocMap; //!< A mapping for each IMemoryAllocator to its associated shared_ptr


  std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<std::vector<std::shared_ptr<BaseConnector>>>>>
      memReleaser; //!< A mapping from memory edge name to memory manager connector for releasing memory from the TaskGraph to a Task within the TaskGraph
  std::shared_ptr<std::unordered_map<std::string, MMType>> mmTypeMap; //!< A mapping from memory edge name to memory manager type for a TaskGraph

  MemGetterMap *memGetterMap; //!< A mapping for each memGetter to the named edge and the memory manager responsible for that edge

  int pipelineId; //!< The pipelineId for the task graph

};
}

#endif //HTGS_TASKGRAPH_H
