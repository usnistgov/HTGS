
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 5/17/16.
//

#include <htgs/api/VoidData.hpp>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/ExecutionPipeline.hpp>
#include <gtest/gtest.h>
#include "memReleaseOutsideGraphTests.h"
#include "memTaskOutsideRelease/data/MultiMemData.h"
#include "memTaskOutsideRelease/tasks/MemAllocTask.h"
#include "memTaskOutsideRelease/tasks/MemReleaseTask.h"
#include "memTaskOutsideRelease/rules/MemAllocDistributeRule.h"
#include "memMultiRelease/memory/SimpleMemoryAllocator.h"

htgs::TaskGraphConf<MultiMemData, htgs::VoidData> *createMemReleaseOutideGraph(size_t numPipelines, size_t numAllocators)
{
  auto taskGraph = new htgs::TaskGraphConf<MultiMemData, MultiMemData>();

  MemAllocTask *prevTask = nullptr;

  MemReleaseTask *releaseTask = new MemReleaseTask();

  for (size_t i = 0; i < numAllocators; i++)
  {
    MemAllocTask *allocTask = new MemAllocTask(i);

    if (i == 0)
    {
      taskGraph->setGraphConsumerTask(allocTask);
    }
    else if (i == numAllocators-1)
    {
      taskGraph->addGraphProducerTask(allocTask);
    }

    if (prevTask != nullptr)
    {
      taskGraph->addEdge(prevTask, allocTask);
    }

    SimpleMemoryAllocator *memAlloc = new SimpleMemoryAllocator(1);
    taskGraph->addMemoryManagerEdge("memEdge" + std::to_string(i), allocTask, memAlloc, 1, htgs::MMType::Static);

    prevTask = allocTask;
  }

  auto execPipeline = new htgs::ExecutionPipeline<MultiMemData, MultiMemData>(numPipelines, taskGraph);
  auto decompRule = std::make_shared<MemAllocDistributeRule>();

  execPipeline->addInputRule(decompRule);

  auto mainGraph = new htgs::TaskGraphConf<MultiMemData, htgs::VoidData>();

  mainGraph->setGraphConsumerTask(execPipeline);
  mainGraph->addEdge(execPipeline, releaseTask);

  EXPECT_EQ(numAllocators*2, taskGraph->getTaskManagers()->size());
  EXPECT_EQ(2, mainGraph->getTaskManagers()->size());

  return mainGraph;
}


void launchGraph(htgs::TaskGraphConf<MultiMemData, htgs::VoidData> *taskGraph, size_t numData, size_t numPipelines, size_t numAllocators)
{
  htgs::TaskGraphRuntime *rt = new htgs::TaskGraphRuntime(taskGraph);

  for (size_t i = 0; i < numData; i++)
  {
    for (size_t id = 0; id < numPipelines; id++)
    {
      taskGraph->produceData(new MultiMemData(id, numAllocators));
    }
  }

  taskGraph->finishedProducingData();

  rt->executeAndWaitForRuntime();

  EXPECT_NO_FATAL_FAILURE(delete rt);
}

void memReleaseOutsideGraphCreation() {
  auto graph1 = createMemReleaseOutideGraph(1, 1);
  auto graph2 = createMemReleaseOutideGraph(1, 2);
  auto graph3 = createMemReleaseOutideGraph(1, 4);
  auto graph4 = createMemReleaseOutideGraph(1, 8);
  auto graph5 = createMemReleaseOutideGraph(2, 1);
  auto graph6 = createMemReleaseOutideGraph(2, 2);
  auto graph7 = createMemReleaseOutideGraph(2, 4);
  auto graph8 = createMemReleaseOutideGraph(2, 8);

  EXPECT_NO_FATAL_FAILURE(delete graph1);
  EXPECT_NO_FATAL_FAILURE(delete graph2);
  EXPECT_NO_FATAL_FAILURE(delete graph3);
  EXPECT_NO_FATAL_FAILURE(delete graph4);
  EXPECT_NO_FATAL_FAILURE(delete graph5);
  EXPECT_NO_FATAL_FAILURE(delete graph6);
  EXPECT_NO_FATAL_FAILURE(delete graph7);
  EXPECT_NO_FATAL_FAILURE(delete graph8);
}

void memReleaseOutsideGraphExecution(size_t numData, size_t numAllocators, size_t numPipelines) {
  auto graph = createMemReleaseOutideGraph(numPipelines, numAllocators);

#ifdef HTGS_TEST_OUTPUT_DOTFILE
  graph->writeDotToFile("memReleaseOutsideGraph.dot");
#endif

  EXPECT_NO_FATAL_FAILURE(launchGraph(graph, numData, numPipelines, numAllocators));
}