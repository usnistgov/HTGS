// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 3/31/17.
//

#include <gtest/gtest.h>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>
#include "simpleCudaGraphTests.h"
#include "simpleCuda/tasks/SimpleCudaTask.h"
#include "simpleCuda/memory/SimpleCudaAllocator.h"

void createCudaTask()
{
  size_t numGpus = 1;
  int *gpuIds = new int[1]{0};
  SimpleCudaTask task(gpuIds, numGpus);

  EXPECT_EQ(1, task.getNumGPUs());
  EXPECT_STREQ("SimpleCudaTask", task.getName().c_str());
  EXPECT_FALSE(task.isStartTask());

  delete [] gpuIds;
}


htgs::TaskGraphConf<SimpleCudaData, SimpleCudaData> *createCudaGraph(int numChain, size_t numGPUs, int *gpuIds)
{
  htgs::TaskGraphConf<SimpleCudaData, SimpleCudaData> *tg = new htgs::TaskGraphConf<SimpleCudaData, SimpleCudaData>();

  SimpleCudaTask *t;

  SimpleCudaTask *startTask = nullptr, *endTask = nullptr;

  SimpleCudaTask *prevTask = nullptr;
  for (int chain = 0; chain < numChain; chain++) {
    t = new SimpleCudaTask(gpuIds, numGPUs);

    if (startTask == nullptr) {
      startTask = t;
      tg->setGraphConsumerTask(t);
    }
    else {
      tg->addEdge(prevTask, t);
    }

    if (chain == numChain - 1) {
      endTask = t;
      endTask->setDoReleaseMemory(true);

      tg->addGraphProducerTask(t);
    }
    prevTask = t;

  }

  SimpleCudaAllocator *alloc = new SimpleCudaAllocator(1);
  tg->addCudaMemoryManagerEdge("cudaMemEdge", startTask, alloc, 1, htgs::MMType::Static, gpuIds);

  EXPECT_EQ(numChain+1, tg->getTaskManagers()->size());

  return tg;
}

void launchCudaGraph(htgs::TaskGraphConf<SimpleCudaData, SimpleCudaData> *graph, int numDataGenerated)
{
  htgs::TaskGraphRuntime *rt = new htgs::TaskGraphRuntime(graph);

  for (int i = 0; i < numDataGenerated; i++) {
    graph->produceData(new SimpleCudaData());
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

#ifdef HTGS_TEST_OUTPUT_DOTFILE
  graph->writeDotToFile("testCudaGraph.dot");
#endif

  EXPECT_EQ(numDataGenerated, count);
  EXPECT_NO_FATAL_FAILURE(delete rt);
}

void simpleCudaGraphCreation()
{
  size_t numGpus = 1;
  int *gpuIds = new int[1]{0};

  auto graph1 = createCudaGraph(1, numGpus, gpuIds);
  auto graph2 = createCudaGraph(2, numGpus, gpuIds);
  auto graph3 = createCudaGraph(10, numGpus, gpuIds);
  auto graph4 = createCudaGraph(100, numGpus, gpuIds);

  EXPECT_NO_FATAL_FAILURE(delete graph1);
  EXPECT_NO_FATAL_FAILURE(delete graph2);
  EXPECT_NO_FATAL_FAILURE(delete graph3);
  EXPECT_NO_FATAL_FAILURE(delete graph4);

  delete []gpuIds;
}

void simpleCudaGraphExecution()
{
  size_t numGpus = 1;
  int *gpuIds = new int[1]{0};

  auto graph1 = createCudaGraph(1, numGpus, gpuIds);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph1, 1));

  auto graph2 = createCudaGraph(1, numGpus, gpuIds);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph2, 10));

  auto graph3 = createCudaGraph(1, numGpus, gpuIds);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph3, 100));

  auto graph4 = createCudaGraph(10, numGpus, gpuIds);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph4, 1));

  auto graph5 = createCudaGraph(10, numGpus, gpuIds);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph5, 10));

  auto graph6 = createCudaGraph(10, numGpus, gpuIds);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph6, 100));


  delete []gpuIds;
}
