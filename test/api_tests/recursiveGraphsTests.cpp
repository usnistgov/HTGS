// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 11/13/18.
//

#include <htgs/api/TaskGraphConf.hpp>
#include <gtest/gtest.h>
#include "recursiveGraphsTests.h"
#include "simple/data/SimpleData.h"
#include "simple/tasks/SimpleTask.h"
#include "simple/rules/SimpleRule.h"
#include "simple/rules/SimpleDecompRule.h"

htgs::TaskGraphConf<SimpleData, SimpleData> *createGraphWithExecPipeline(int numChain, size_t numPipelines, size_t numThreads)
{
  htgs::TaskGraphConf<SimpleData, SimpleData> *tg = new htgs::TaskGraphConf<SimpleData, SimpleData>();

  SimpleTask *t;

  SimpleTask *startTask = nullptr, *endTask = nullptr;

  SimpleTask *prevTask = nullptr;
  for (int chain = 0; chain < numChain; chain++) {
    t = new SimpleTask(numThreads, chain, false, false);

    if (startTask == nullptr) {
      startTask = t;
      tg->setGraphConsumerTask(t);
    }
    else {
      if (chain == numChain-1)
      {
        htgs::Bookkeeper<SimpleData> *bk = new htgs::Bookkeeper<SimpleData>();
        SimpleRule *rule = new SimpleRule();
        tg->addEdge(prevTask, bk);
        tg->addRuleEdge(bk, rule, t);
      }
      else {
        tg->addEdge(prevTask, t);
      }
    }

    if (chain == numChain - 1) {
      endTask = t;
      endTask->setReleaseMem(true);

      tg->addGraphProducerTask(t);
    }
    prevTask = t;

  }

  htgs::TaskGraphConf<SimpleData, SimpleData> *mainGraph = new htgs::TaskGraphConf<SimpleData, SimpleData>();


  htgs::ExecutionPipeline<SimpleData, SimpleData> *execPipeline = new htgs::ExecutionPipeline<SimpleData, SimpleData>(numPipelines, tg);
  SimpleDecompRule *decompRule = new SimpleDecompRule(numPipelines);

  execPipeline->addInputRule(decompRule);

  mainGraph->setGraphConsumerTask(execPipeline);
  mainGraph->addGraphProducerTask(execPipeline);

  if (numChain == 1)
    EXPECT_EQ(numChain, tg->getTaskManagers()->size());
  else
    EXPECT_EQ(numChain+1, tg->getTaskManagers()->size());

  EXPECT_EQ(1, mainGraph->getInputConnector()->getProducerCount());

  EXPECT_EQ(1, mainGraph->getTaskManagers()->size());

  return mainGraph;
}

htgs::TaskGraphConf<SimpleData, SimpleData> *createGraph(int numChain, size_t numThreads, htgs::TGTask<SimpleData, SimpleData> *tgTask)
{
  htgs::TaskGraphConf<SimpleData, SimpleData> *tg = new htgs::TaskGraphConf<SimpleData, SimpleData>();

  htgs::ITask<SimpleData, SimpleData> *t;
  htgs::ITask<SimpleData, SimpleData> *startTask = nullptr;

  htgs::ITask<SimpleData, SimpleData> *prevTask = nullptr;
  for (int chain = 0; chain < numChain; chain++) {

    if (tgTask != nullptr && chain == 0) {
     t = tgTask;
    } else {
      t = new SimpleTask(numThreads, chain, false, false);
    }

    if (startTask == nullptr) {
        startTask = t;
        tg->setGraphConsumerTask(t);
    } else {
        tg->addEdge(prevTask, t);
    }

    if (chain == numChain - 1) {
        tg->addGraphProducerTask(t);
    }

    prevTask = t;

  }

  EXPECT_EQ(numChain, tg->getTaskManagers()->size());

  return tg;
}


void testTGTasks(bool graphIsConsumer, bool graphIsProducer, int numChain, size_t numThreads)
{
  auto graph = createGraph(numChain, numThreads, nullptr);

  auto tgTask = graph->createTaskGraphTask();

  auto mainGraph = new htgs::TaskGraphConf<SimpleData, SimpleData>();

  if (graphIsConsumer)
  {
    mainGraph->setGraphConsumerTask(tgTask);
  } else {
    SimpleTask *consumer = new SimpleTask(numThreads, 0, false);
    mainGraph->setGraphConsumerTask(consumer);
    mainGraph->addEdge(consumer, tgTask);
  }

  if (graphIsProducer)
  {
    mainGraph->addGraphProducerTask(tgTask);
  } else{
    SimpleTask *producer = new SimpleTask(numThreads, 0, false);
    mainGraph->addEdge(tgTask, producer);
    mainGraph->addGraphProducerTask(producer);
  }

  htgs::TaskGraphRuntime *runtime = new htgs::TaskGraphRuntime(mainGraph);
  runtime->executeRuntime();
  mainGraph->waitForInitialization();

  int numData = 100;

  for (int i = 0; i < numData; i++)
  {
    mainGraph->produceData(new SimpleData(99, 0));
  }

  mainGraph->finishedProducingData();

  int count = 0;
  while(!mainGraph->isOutputTerminated()) {
    auto data = mainGraph->consumeData();

    if (data != nullptr)
    {
      EXPECT_EQ(data->getValue(), 99);
      count++;
    }
  }

  runtime->waitForRuntime();

#ifdef HTGS_TEST_OUTPUT_DOTFILE
  mainGraph->writeDotToFile("testTGTaskGraph.dot");
#endif

  EXPECT_EQ(count, numData);

  delete runtime;
}


void testGraphsWithinGraphs(int numGraphs, int numChain, size_t numThreads, bool useExecPipeline, size_t numPipelines)
{

  htgs::TGTask<SimpleData, SimpleData> *tgTask = nullptr;

  for (int i = 0; i < numGraphs; i++)
  {
    auto graph = createGraph(numChain, numThreads, tgTask);
    tgTask = graph->createTaskGraphTask();
  }

  htgs::TaskGraphConf<SimpleData, SimpleData> *mainGraph = new htgs::TaskGraphConf<SimpleData, SimpleData>();

  if (useExecPipeline)
  {
    htgs::TaskGraphConf<SimpleData, SimpleData> *graphConf = new htgs::TaskGraphConf<SimpleData, SimpleData>();
    graphConf->setGraphConsumerTask(tgTask);
    graphConf->addGraphProducerTask(tgTask);

    htgs::ExecutionPipeline<SimpleData, SimpleData> *execPipeline = new htgs::ExecutionPipeline<SimpleData, SimpleData>(numPipelines, graphConf);
    execPipeline->addInputRule(new SimpleRule());

    mainGraph->setGraphConsumerTask(execPipeline);
    mainGraph->addGraphProducerTask(execPipeline);
  }
  else {
    mainGraph->setGraphConsumerTask(tgTask);
    mainGraph->addGraphProducerTask(tgTask);
  }

  if (useExecPipeline)
  {
    EXPECT_EQ(mainGraph->getNumberOfSubGraphs(), numGraphs * numPipelines + numPipelines);
  } else{
    EXPECT_EQ(mainGraph->getNumberOfSubGraphs(), numGraphs);
  }


  htgs::TaskGraphRuntime *runtime = new htgs::TaskGraphRuntime(mainGraph);
  runtime->executeRuntime();
  mainGraph->waitForInitialization();

  int numData = 100;

  for (int i = 0; i < numData; i++)
  {
    mainGraph->produceData(new SimpleData(99, 0));
  }

  mainGraph->finishedProducingData();

  int count = 0;
  while(!mainGraph->isOutputTerminated()) {
    auto data = mainGraph->consumeData();

    if (data != nullptr)
    {
      EXPECT_EQ(data->getValue(), 99);
      count++;
    }
  }

  runtime->waitForRuntime();

#ifdef HTGS_TEST_OUTPUT_DOTFILE
  mainGraph->writeDotToFile("testGraphsWithinGraphs.dot");
#endif

  EXPECT_EQ(1, mainGraph->getTaskManagers()->size());
  EXPECT_EQ(count, numData * numPipelines);

  delete runtime;
  runtime = nullptr;

}


void testTGTaskWithExecPipeline(int numPipelines, int numChain, size_t numThreads)
{
  auto graph = createGraphWithExecPipeline(numChain, numPipelines, 1);

  auto mainGraph = new htgs::TaskGraphConf<SimpleData, SimpleData>();

  auto tgTask = graph->createTaskGraphTask();

  mainGraph->setGraphConsumerTask(tgTask);
  mainGraph->addGraphProducerTask(tgTask);

  EXPECT_EQ(mainGraph->getNumberOfSubGraphs(), numPipelines+1);


  htgs::TaskGraphRuntime *runtime = new htgs::TaskGraphRuntime(mainGraph);
  runtime->executeRuntime();
  mainGraph->waitForInitialization();

  int numData = 100;

  for (int i = 0; i < numData; i++)
  {
    mainGraph->produceData(new SimpleData(99, i%numPipelines));
  }

  mainGraph->finishedProducingData();

  int count = 0;
  while(!mainGraph->isOutputTerminated()) {
    auto data = mainGraph->consumeData();

    if (data != nullptr)
    {
      EXPECT_EQ(data->getValue(), 99);
      count++;
    }
  }

  runtime->waitForRuntime();

#ifdef HTGS_TEST_OUTPUT_DOTFILE
  mainGraph->writeDotToFile("testTGTaskWithExecPipeline.dot");
#endif

  EXPECT_EQ(1, mainGraph->getTaskManagers()->size());
  EXPECT_EQ(count, numData);

  delete runtime;
  runtime = nullptr;
}