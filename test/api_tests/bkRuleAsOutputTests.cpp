// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 12/7/18.
//

#include <htgs/api/TaskGraphConf.hpp>
#include <gtest/gtest.h>
#include "bkRuleAsOutputTests.h"
#include "simple/tasks/SimpleTask.h"
#include "simple/rules/SimpleDecompRule.h"
#include "simple/rules/SimpleRule.h"


htgs::TaskGraphConf<SimpleData, SimpleData> *createGraphWithExecPipelineRuleAsOutput(int numChain, size_t numPipelines, size_t numThreads)
{
  htgs::TaskGraphConf<SimpleData, SimpleData> *tg = new htgs::TaskGraphConf<SimpleData, SimpleData>();

  SimpleTask *t;

  SimpleTask *startTask = nullptr, *endTask = nullptr;

  SimpleTask *prevTask = nullptr;
  htgs::Bookkeeper<SimpleData> *bk = new htgs::Bookkeeper<SimpleData>();

  if (numChain == 1) {
    tg->setGraphConsumerTask(bk);
  }
  else {
    for (int chain = 0; chain < numChain; chain++) {
      t = new SimpleTask(numThreads, chain, false, false);

      if (startTask == nullptr) {
        startTask = t;
        tg->setGraphConsumerTask(t);
      } else {
        if (chain == numChain - 1) {
          SimpleRule *rule = new SimpleRule();
          tg->addEdge(prevTask, bk);
          tg->addRuleEdge(bk, rule, t);
        } else {
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
  }

  SimpleRule *outputRule = new SimpleRule();
  tg->addRuleEdgeAsGraphProducer(bk, outputRule);

  htgs::TaskGraphConf<SimpleData, SimpleData> *mainGraph = new htgs::TaskGraphConf<SimpleData, SimpleData>();


  htgs::ExecutionPipeline<SimpleData, SimpleData> *execPipeline = new htgs::ExecutionPipeline<SimpleData, SimpleData>(numPipelines, tg);
  SimpleDecompRule *decompRule = new SimpleDecompRule(numPipelines);

  execPipeline->addInputRule(decompRule);

  mainGraph->addGraphProducerTask(execPipeline);

  htgs::Bookkeeper<SimpleData> *outputBk2 = new htgs::Bookkeeper<SimpleData>();
  SimpleRule *outputRule2 = new SimpleRule();
  SimpleRule *outputRule3 = new SimpleRule();

  mainGraph->setGraphConsumerTask(outputBk2);
  mainGraph->addRuleEdgeAsGraphProducer(outputBk2, outputRule2);
  mainGraph->addRuleEdge(outputBk2, outputRule3, execPipeline);


  if (numChain == 1)
    EXPECT_EQ(numChain, tg->getTaskManagers()->size());
  else
    EXPECT_EQ(numChain+1, tg->getTaskManagers()->size());

  EXPECT_EQ(1, mainGraph->getInputConnector()->getProducerCount());

  EXPECT_EQ(2, mainGraph->getTaskManagers()->size());

  return mainGraph;
}



void testBkAsOutput(int numPipelines, int numChain, size_t numThreads)
{
  auto graph = createGraphWithExecPipelineRuleAsOutput(numChain, numPipelines, 1);

  auto mainGraph = new htgs::TaskGraphConf<SimpleData, SimpleData>();

  auto tgTask = graph->createTaskGraphTask();

  htgs::Bookkeeper<SimpleData> *bk = new htgs::Bookkeeper<SimpleData>();
  SimpleRule *simpleRule = new SimpleRule();
  SimpleRule *simpleRule2 = new SimpleRule();

  mainGraph->setGraphConsumerTask(bk);

  mainGraph->addRuleEdge(bk, simpleRule, tgTask);

  mainGraph->addRuleEdgeAsGraphProducer(bk, simpleRule2);
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
  mainGraph->writeDotToFile("testBkAsOutput.dot");
#endif

  EXPECT_EQ(2, mainGraph->getTaskManagers()->size());
  EXPECT_EQ(count, numChain == 1 ? numData * 3 : numData * 4);

  delete runtime;
  runtime = nullptr;
}


