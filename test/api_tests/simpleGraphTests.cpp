
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 4/18/16.
//

#include <gtest/gtest.h>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/ExecutionPipeline.hpp>
#include "simple/data/SimpleData.h"
#include "simple/memory/SimpleAllocator.h"
#include "simple/tasks/SimpleTask.h"
#include "simple/rules/SimpleRule.h"
#include "simple/rules/SimpleDecompRule.h"
#include "simple/customEdges/BookkeeperCustomEdge.h"
#include "simple/customEdges/MemoryManagerCustomEdge.h"

void createData()
{
  SimpleData data1(1, 0);
  SimpleData data2(2, 0);
  SimpleData data3(3, 0);

  EXPECT_EQ(1, data1.getValue());
  EXPECT_EQ(2, data2.getValue());
  EXPECT_EQ(3, data3.getValue());
}

void memoryAllocAndFreeCheck()
{
  SimpleAllocator allocator(100);

  int *mem = allocator.memAlloc();
  allocator.memFree(mem);
}

void createTask()
{
  SimpleTask task(1, 0, false);
  EXPECT_EQ(1, task.getNumThreads());
  EXPECT_STREQ("SimpleTask0", task.getName().c_str());
  EXPECT_TRUE(task.getIsStartTask());
}

htgs::TaskGraph<SimpleData, SimpleData> *createGraphCustomEdge(int numChain, int numPipelines, int numThreads, bool useMemoryManager)
{
  htgs::TaskGraph<SimpleData, SimpleData> *tg = new htgs::TaskGraph<SimpleData, SimpleData>();

  SimpleTask *t;

  SimpleTask *startTask = nullptr, *endTask = nullptr;

  SimpleTask *prevTask = nullptr;
  for (int chain = 0; chain < numChain; chain++) {
    t = new SimpleTask(numThreads, chain, useMemoryManager);

    if (startTask == nullptr) {
      startTask = t;
      tg->addGraphInputConsumer(t);
    }
    else {
      if (chain == numChain-1)
      {
        htgs::Bookkeeper<SimpleData> *bk = new htgs::Bookkeeper<SimpleData>();
        auto *bke = new BookkeeperCustomEdge<SimpleData, SimpleData, SimpleData>(bk, t);
        SimpleRule *rule = new SimpleRule();
        bke->addRule(rule);
        tg->addEdge(prevTask, bk);
        tg->addCustomEdge(bke);
      }
      else {
        tg->addEdge(prevTask, t);
      }
    }

    if (chain == numChain - 1) {
      endTask = t;
      tg->addGraphOutputProducer(t);
    }
    prevTask = t;

  }

  auto mme = new MemoryManagerCustomEdge<int *>("test", startTask, endTask, new SimpleAllocator(1), 1, htgs::MMType::Static);
  tg->addCustomEdge(mme);

  htgs::TaskGraph<SimpleData, SimpleData> *mainGraph = new htgs::TaskGraph<SimpleData, SimpleData>();


  htgs::ExecutionPipeline<SimpleData, SimpleData> *execPipeline = new htgs::ExecutionPipeline<SimpleData, SimpleData>(numPipelines, tg);

  SimpleDecompRule *decompRule = new SimpleDecompRule(numPipelines);

  execPipeline->addInputRule(decompRule);

  mainGraph->addGraphInputConsumer(execPipeline);
  mainGraph->addGraphOutputProducer(execPipeline);

  mainGraph->incrementGraphInputProducer();


  if (numChain == 1)
    EXPECT_EQ(numChain+1, tg->getVertices()->size());
  else
    EXPECT_EQ(numChain+2, tg->getVertices()->size());

  EXPECT_EQ(1, mainGraph->getOutputProducers()->size());
  EXPECT_EQ(1, mainGraph->getInputConnector()->getProducerCount());

  EXPECT_EQ(1, mainGraph->getVertices()->size());


  return mainGraph;
}

htgs::TaskGraph<SimpleData, SimpleData> *createGraph(int numChain, int numPipelines, int numThreads, bool useMemoryManager)
{
  htgs::TaskGraph<SimpleData, SimpleData> *tg = new htgs::TaskGraph<SimpleData, SimpleData>();

  SimpleTask *t;

  SimpleTask *startTask = nullptr, *endTask = nullptr;

  SimpleTask *prevTask = nullptr;
  for (int chain = 0; chain < numChain; chain++) {
    t = new SimpleTask(numThreads, chain, useMemoryManager);

    if (startTask == nullptr) {
      startTask = t;
      tg->addGraphInputConsumer(t);
    }
    else {
      if (chain == numChain-1)
      {
        htgs::Bookkeeper<SimpleData> *bk = new htgs::Bookkeeper<SimpleData>();
        SimpleRule *rule = new SimpleRule();
        tg->addEdge(prevTask, bk);
        tg->addRule(bk, t, rule);
      }
      else {
        tg->addEdge(prevTask, t);
      }
    }

    if (chain == numChain - 1) {
      endTask = t;
      tg->addGraphOutputProducer(t);
    }
    prevTask = t;

  }

  tg->addMemoryManagerEdge("test", startTask, endTask, new SimpleAllocator(1), 1, htgs::MMType::Static);

  htgs::TaskGraph<SimpleData, SimpleData> *mainGraph = new htgs::TaskGraph<SimpleData, SimpleData>();


  htgs::ExecutionPipeline<SimpleData, SimpleData> *execPipeline = new htgs::ExecutionPipeline<SimpleData, SimpleData>(numPipelines, tg);

  SimpleDecompRule *decompRule = new SimpleDecompRule(numPipelines);

  execPipeline->addInputRule(decompRule);

  mainGraph->addGraphInputConsumer(execPipeline);
  mainGraph->addGraphOutputProducer(execPipeline);

  mainGraph->incrementGraphInputProducer();


  if (numChain == 1)
    EXPECT_EQ(numChain+1, tg->getVertices()->size());
  else
    EXPECT_EQ(numChain+2, tg->getVertices()->size());

  EXPECT_EQ(1, mainGraph->getOutputProducers()->size());
  EXPECT_EQ(1, mainGraph->getInputConnector()->getProducerCount());

  EXPECT_EQ(1, mainGraph->getVertices()->size());

  return mainGraph;
}

void launchGraph(htgs::TaskGraph<SimpleData, SimpleData> *graph, int numDataGenerated, int numPipelines)
{
  htgs::Runtime *rt = new htgs::Runtime(graph);

  for (int i = 0; i < numDataGenerated; i++) {
    for (int pid = 0; pid < numPipelines; pid++) {
      graph->produceData(new SimpleData(i, pid));
    }
  }

  graph->finishedProducingData();

  rt->executeRuntime();

  int count = 0;
  while(!graph->isOutputTerminated())
  {
    auto data = graph->consumeData();
    if (data != nullptr)
    {
      count++;
    }
  }



  rt->waitForRuntime();
  EXPECT_EQ(numDataGenerated*numPipelines, count);
  EXPECT_NO_FATAL_FAILURE(delete rt);
}

void simpleGraphCreation()
{
  auto graph1 = createGraph(1, 1, 1, false);
  auto graph2 = createGraph(2, 1, 1, false);
  auto graph3 = createGraph(10, 1, 1, false);
  auto graph4 = createGraph(100, 1, 1, false);

  auto graphMem1 = createGraph(1, 1, 1, true);
  auto graphMem2 = createGraph(2, 1, 1, true);
  auto graphMem3 = createGraph(10, 1, 1, true);
  auto graphMem4 = createGraph(100, 1, 1, true);

  auto graphMemExecP1 = createGraph(1, 1, 1, true);
  auto graphMemExecP2 = createGraph(2, 2, 1, true);
  auto graphMemExecP3 = createGraph(10, 10, 1, true);
  auto graphMemExecP4 = createGraph(100, 100, 1, true);

  EXPECT_NO_FATAL_FAILURE(delete graph1);
  EXPECT_NO_FATAL_FAILURE(delete graph2);
  EXPECT_NO_FATAL_FAILURE(delete graph3);
  EXPECT_NO_FATAL_FAILURE(delete graph4);

  EXPECT_NO_FATAL_FAILURE(delete graphMem1);
  EXPECT_NO_FATAL_FAILURE(delete graphMem2);
  EXPECT_NO_FATAL_FAILURE(delete graphMem3);
  EXPECT_NO_FATAL_FAILURE(delete graphMem4);

  EXPECT_NO_FATAL_FAILURE(delete graphMemExecP1);
  EXPECT_NO_FATAL_FAILURE(delete graphMemExecP2);
  EXPECT_NO_FATAL_FAILURE(delete graphMemExecP3);
  EXPECT_NO_FATAL_FAILURE(delete graphMemExecP4);

}

void simpleGraphExecution(int numPipelines)
{
  auto graph1 = createGraph(10, numPipelines, 2, false);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graph1, 1, numPipelines));

  auto graph2 = createGraph(10, numPipelines, 2, false);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graph2, 10, numPipelines));

  auto graph3 = createGraph(10, numPipelines, 2, false);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graph3, 100, numPipelines));

  auto graphMem1 = createGraph(10, numPipelines, 2, true);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graphMem1, 1, numPipelines));

  auto graphMem2 = createGraph(10, numPipelines, 2, true);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graphMem2, 10, numPipelines));

  auto graphMem3 = createGraph(10, numPipelines, 2, true);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graphMem3, 100, numPipelines));

}

void simpleGraphCreationWithCustomEdges()
{
  auto graph1 = createGraphCustomEdge(1, 1, 1, false);
  auto graph2 = createGraphCustomEdge(2, 1, 1, false);
  auto graph3 = createGraphCustomEdge(10, 1, 1, false);
  auto graph4 = createGraphCustomEdge(100, 1, 1, false);

  auto graphMem1 = createGraphCustomEdge(1, 1, 1, true);
  auto graphMem2 = createGraphCustomEdge(2, 1, 1, true);
  auto graphMem3 = createGraphCustomEdge(10, 1, 1, true);
  auto graphMem4 = createGraphCustomEdge(100, 1, 1, true);

  auto graphMemExecP1 = createGraphCustomEdge(1, 1, 1, true);
  auto graphMemExecP2 = createGraphCustomEdge(2, 2, 1, true);
  auto graphMemExecP3 = createGraphCustomEdge(10, 10, 1, true);
  auto graphMemExecP4 = createGraphCustomEdge(100, 100, 1, true);

  EXPECT_NO_FATAL_FAILURE(delete graph1);
  EXPECT_NO_FATAL_FAILURE(delete graph2);
  EXPECT_NO_FATAL_FAILURE(delete graph3);
  EXPECT_NO_FATAL_FAILURE(delete graph4);

  EXPECT_NO_FATAL_FAILURE(delete graphMem1);
  EXPECT_NO_FATAL_FAILURE(delete graphMem2);
  EXPECT_NO_FATAL_FAILURE(delete graphMem3);
  EXPECT_NO_FATAL_FAILURE(delete graphMem4);

  EXPECT_NO_FATAL_FAILURE(delete graphMemExecP1);
  EXPECT_NO_FATAL_FAILURE(delete graphMemExecP2);
  EXPECT_NO_FATAL_FAILURE(delete graphMemExecP3);
  EXPECT_NO_FATAL_FAILURE(delete graphMemExecP4);
}

void simpleGraphExecutionWithCustomEdges(int numPipelines)
{
  auto graph1 = createGraphCustomEdge(10, numPipelines, 10, false);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graph1, 1, numPipelines));

  auto graph2 = createGraphCustomEdge(10, numPipelines, 10, false);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graph2, 10, numPipelines));

  auto graph3 = createGraphCustomEdge(10, numPipelines, 10, false);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graph3, 100, numPipelines));

  auto graphMem1 = createGraphCustomEdge(10, numPipelines, 10, true);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graphMem1, 1, numPipelines));

  auto graphMem2 = createGraphCustomEdge(2, numPipelines, 10, true);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graphMem2, 10, numPipelines));

  auto graphMem3 = createGraphCustomEdge(10, numPipelines, 10, true);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graphMem3, 100, numPipelines));

}