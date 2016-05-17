
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 4/19/16.
//

#include "memMultiReleaseGraphTests.h"

#include <htgs/api/TaskGraph.hpp>
#include <htgs/api/ExecutionPipeline.hpp>
#include <gtest/gtest.h>
#include "memMultiRelease/data/ProcessedData.h"
#include "memMultiRelease/tasks/InputTask.h"
#include "memMultiRelease/rules/MemDistributeRule.h"
#include "memMultiRelease/tasks/OutputMemReleaseTask.h"
#include "memMultiRelease/rules/SimpleDecompRule.h"
#include "memMultiRelease/memory/SimpleMemoryAllocator.h"

std::pair<htgs::TaskGraph<InputData, ProcessedData> *, htgs::TaskGraph<InputData, ProcessedData> *>
createMultiReleaseGraph(int numPipelines, int numReleasers, bool useSeparateGraphEdge, bool useGraphReleaser, htgs::MMType type)
{
  auto taskGraph = new htgs::TaskGraph<InputData, ProcessedData>();

  InputTask *inputTask = new InputTask(numReleasers, useGraphReleaser, type);
  htgs::Bookkeeper<ProcessedData> *bk = new htgs::Bookkeeper<ProcessedData>();

  taskGraph->addGraphInputConsumer(inputTask);
  taskGraph->addEdge(inputTask, bk);

  SimpleMemoryAllocator *allocator = new SimpleMemoryAllocator(1);

  int memoryPoolSizeMemEdge = numReleasers + (useGraphReleaser && !useSeparateGraphEdge ? numReleasers : 0);
  int memoryPoolSizeMem2Edge = numReleasers;

  for (int i = 0; i < numReleasers; i++) {
    MemDistributeRule *mRule = new MemDistributeRule(i);
    OutputMemReleaseTask *outputTask = new OutputMemReleaseTask(i, type);

    taskGraph->addRule(bk, outputTask, mRule);
    taskGraph->addGraphOutputProducer(outputTask);

    switch (type) {
      case htgs::MMType::Static:
        taskGraph->addMemoryManagerEdge("mem", inputTask, outputTask, allocator, memoryPoolSizeMemEdge, type);
        break;
      case htgs::MMType::Dynamic:
        taskGraph->addMemoryManagerEdge("mem", inputTask, outputTask, allocator, memoryPoolSizeMemEdge, type);
        break;
      case htgs::MMType::UserManaged:
        taskGraph->addUserManagedMemoryManagerEdge("mem", inputTask, outputTask, memoryPoolSizeMemEdge);
        break;
    }
  }

  if (useGraphReleaser)
  {
    if (useSeparateGraphEdge)
    {
      taskGraph->addGraphMemoryManagerEdge("mem2", inputTask, allocator, memoryPoolSizeMem2Edge, htgs::MMType::Static);
    }
    else{
      switch(type)
      {
        case htgs::MMType::Static:
          taskGraph->addGraphMemoryManagerEdge("mem", inputTask, allocator, memoryPoolSizeMemEdge, type);
          break;
        case htgs::MMType::Dynamic:
          taskGraph->addGraphMemoryManagerEdge("mem", inputTask, allocator, memoryPoolSizeMemEdge, type);
          break;
        case htgs::MMType::UserManaged:
          taskGraph->addGraphUserManagedMemoryManagerEdge("mem", inputTask, memoryPoolSizeMemEdge);
          break;
      }
    }
  }

  auto execPipeline = new htgs::ExecutionPipeline<InputData, ProcessedData>(numPipelines, taskGraph);
  auto decompRule = new SimpleDecompRule(numPipelines);

  execPipeline->addInputRule(decompRule);

  auto mainGraph = new htgs::TaskGraph<InputData, ProcessedData>();

  mainGraph->addGraphInputConsumer(execPipeline);
  mainGraph->addGraphOutputProducer(execPipeline);
  mainGraph->incrementGraphInputProducer();


  int add = 0;
  if (useSeparateGraphEdge&&useGraphReleaser)
    add = 1;

  EXPECT_EQ(numReleasers+3+add, taskGraph->getVertices()->size());

  EXPECT_EQ(1, mainGraph->getOutputProducers()->size());
  EXPECT_EQ(1, mainGraph->getInputConnector()->getProducerCount());

  EXPECT_EQ(1, mainGraph->getVertices()->size());
  std::pair<htgs::TaskGraph<InputData, ProcessedData> *, htgs::TaskGraph<InputData, ProcessedData> *>
      retPair (mainGraph, taskGraph);
  return retPair;
}

void launchGraph(htgs::TaskGraph<InputData, ProcessedData> *mainGraph,
                 htgs::TaskGraph<InputData, ProcessedData> *subGraph,
                 int numDataGenerated, int numPipelines, int numReleasers, bool useSeparateEdge, bool useGraphMemReleaser, htgs::MMType type)
{
  htgs::Runtime *rt = new htgs::Runtime(mainGraph);

  for (int i = 0; i < numDataGenerated; i++) {
    for (int pid = 0; pid < numPipelines; pid++) {
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
        if (useSeparateEdge)
        {
          subGraph->memRelease("mem2", data->getMem2());
        }
        else{
          switch(type)
          {
            case htgs::MMType::Static:
              subGraph->memRelease("mem", data->getMem2());
              break;
            case htgs::MMType::Dynamic:
              subGraph->memRelease("mem", data->getMem2());
              break;
            case htgs::MMType::UserManaged:
              subGraph->memRelease("mem", data->getData()->getPipelineId());
              break;
          }
        }
      }


      count++;
    }
  }
  subGraph->finishReleasingMemory();

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

  EXPECT_NO_FATAL_FAILURE(delete graph1.first);
  EXPECT_NO_FATAL_FAILURE(delete graph2.first);
  EXPECT_NO_FATAL_FAILURE(delete graph3.first);
  EXPECT_NO_FATAL_FAILURE(delete graph4.first);
  EXPECT_NO_FATAL_FAILURE(delete graph5.first);
  EXPECT_NO_FATAL_FAILURE(delete graph6.first);
  EXPECT_NO_FATAL_FAILURE(delete graph7.first);
  EXPECT_NO_FATAL_FAILURE(delete graph8.first);
}

void multiReleaseGraphExecution(int numDataGen, int numReleasers, int numPipelines, bool useSeparateEdge, bool useGraphReleaser, htgs::MMType type) {
  auto graphPair = createMultiReleaseGraph(numPipelines, numReleasers, useSeparateEdge, useGraphReleaser, type);
  EXPECT_NO_FATAL_FAILURE(launchGraph(graphPair.first, graphPair.second, numDataGen, numPipelines, numReleasers, useSeparateEdge, useGraphReleaser, type));
}
