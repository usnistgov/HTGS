
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 4/19/16.
//

#include "memMultiReleaseGraphTests.h"

#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/ExecutionPipeline.hpp>
#include <gtest/gtest.h>
#include "memMultiRelease/data/ProcessedData.h"
#include "memMultiRelease/tasks/InputTask.h"
#include "memMultiRelease/rules/MemDistributeRule.h"
#include "memMultiRelease/tasks/OutputMemReleaseTask.h"
#include "memMultiRelease/rules/SimpleDecompRule.h"
#include "memMultiRelease/memory/SimpleMemoryAllocator.h"

htgs::TaskGraphConf<InputData, ProcessedData> *
createMultiReleaseGraph(size_t numPipelines, size_t numReleasers, bool useSeparateGraphEdge, bool useGraphReleaser, htgs::MMType type)
{
  auto taskGraph = new htgs::TaskGraphConf<InputData, ProcessedData>();

  InputTask *inputTask = new InputTask(numReleasers, useGraphReleaser, type);
  htgs::Bookkeeper<ProcessedData> *bk = new htgs::Bookkeeper<ProcessedData>();

  taskGraph->setGraphConsumerTask(inputTask);
  taskGraph->addEdge(inputTask, bk);

  SimpleMemoryAllocator *allocator = new SimpleMemoryAllocator(1);

  size_t memoryPoolSizeMemEdge = numReleasers + (useGraphReleaser && !useSeparateGraphEdge ? numReleasers : 0);
  size_t memoryPoolSizeMem2Edge = numReleasers;
  taskGraph->addMemoryManagerEdge("mem", inputTask, allocator, memoryPoolSizeMemEdge, type);

  for (int i = 0; i < numReleasers; i++) {
    MemDistributeRule *mRule = new MemDistributeRule(i);
    OutputMemReleaseTask *outputTask = new OutputMemReleaseTask(i, type);

    taskGraph->addRuleEdge(bk, mRule, outputTask);
    taskGraph->addGraphProducerTask(outputTask);
  }

  if (useGraphReleaser && useSeparateGraphEdge)
  {
      taskGraph->addMemoryManagerEdge<int>("mem2", inputTask, allocator, memoryPoolSizeMem2Edge, htgs::MMType::Static);
  }

  auto execPipeline = new htgs::ExecutionPipeline<InputData, ProcessedData>(numPipelines, taskGraph);
  auto decompRule = new SimpleDecompRule();

  execPipeline->addInputRule(decompRule);

  auto mainGraph = new htgs::TaskGraphConf<InputData, ProcessedData>();

  mainGraph->setGraphConsumerTask(execPipeline);
  mainGraph->addGraphProducerTask(execPipeline);


  int add = 0;
  if (useSeparateGraphEdge&&useGraphReleaser)
    add = 1;

  EXPECT_EQ(numReleasers+3+add, taskGraph->getTaskManagers()->size());

  EXPECT_EQ(1, mainGraph->getInputConnector()->getProducerCount());

  EXPECT_EQ(1, mainGraph->getTaskManagers()->size());

  return mainGraph;
}

void launchGraph(htgs::TaskGraphConf<InputData, ProcessedData> *mainGraph,
                 size_t numDataGenerated, size_t numPipelines, size_t numReleasers, bool useSeparateEdge, bool useGraphMemReleaser, htgs::MMType type)
{
  htgs::TaskGraphRuntime *rt = new htgs::TaskGraphRuntime(mainGraph);

  for (size_t i = 0; i < numDataGenerated; i++) {
    for (size_t pid = 0; pid < numPipelines; pid++) {
      mainGraph->produceData(new InputData(i, pid));
    }
  }

  mainGraph->finishedProducingData();

  rt->executeRuntime();

  int count = 0;
  while(!mainGraph->isOutputTerminated())
  {
    auto data = mainGraph->consumeData();
    if (data != nullptr)
    {
      if (useGraphMemReleaser)
      {
        mainGraph->releaseMemory(data->getMem2());
      }
      count++;
    }
  }

  rt->waitForRuntime();

  EXPECT_EQ(numDataGenerated*numPipelines*numReleasers, count);
  EXPECT_NO_FATAL_FAILURE(delete rt);
}

void multiReleaseGraphCreation(bool useSeparateEdge, bool useGraphReleaser, htgs::MMType type) {
  auto graph1 = createMultiReleaseGraph(1, 1, useSeparateEdge, useGraphReleaser, type);
  auto graph2 = createMultiReleaseGraph(1, 2, useSeparateEdge, useGraphReleaser, type);
  auto graph3 = createMultiReleaseGraph(1, 4, useSeparateEdge, useGraphReleaser, type);
  auto graph4 = createMultiReleaseGraph(1, 8, useSeparateEdge, useGraphReleaser, type);
  auto graph5 = createMultiReleaseGraph(2, 1, useSeparateEdge, useGraphReleaser, type);
  auto graph6 = createMultiReleaseGraph(2, 2, useSeparateEdge, useGraphReleaser, type);
  auto graph7 = createMultiReleaseGraph(2, 4, useSeparateEdge, useGraphReleaser, type);
  auto graph8 = createMultiReleaseGraph(2, 8, useSeparateEdge, useGraphReleaser, type);

  EXPECT_NO_FATAL_FAILURE(delete graph1);
  EXPECT_NO_FATAL_FAILURE(delete graph2);
  EXPECT_NO_FATAL_FAILURE(delete graph3);
  EXPECT_NO_FATAL_FAILURE(delete graph4);
  EXPECT_NO_FATAL_FAILURE(delete graph5);
  EXPECT_NO_FATAL_FAILURE(delete graph6);
  EXPECT_NO_FATAL_FAILURE(delete graph7);
  EXPECT_NO_FATAL_FAILURE(delete graph8);
}

void multiReleaseGraphExecution(size_t numDataGen, size_t numReleasers, size_t numPipelines, bool useSeparateEdge, bool useGraphReleaser, htgs::MMType type) {
  auto graph = createMultiReleaseGraph(numPipelines, numReleasers, useSeparateEdge, useGraphReleaser, type);
//  graph->writeDotToFile("test.dot");
  EXPECT_NO_FATAL_FAILURE(launchGraph(graph, numDataGen, numPipelines, numReleasers, useSeparateEdge, useGraphReleaser, type));
}
