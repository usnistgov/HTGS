//
// Created by tjb3 on 3/31/17.
//

#include <gtest/gtest.h>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>
#include "simpleCudaGraphTests.h"
#include "simpleCuda/util-cuda.h"
#include "simpleCuda/tasks/SimpleCudaTask.h"
#include "simpleCuda/memory/SimpleCudaAllocator.h"

void createCudaTask()
{
  size_t numGpus = 1;
  int *gpuIds = new int[1]{0};
  CUcontext *contexts = initCuda(1, gpuIds);
  SimpleCudaTask task(contexts, gpuIds, numGpus);

  EXPECT_EQ(1, task.getNumGPUs());
  EXPECT_STREQ("SimpleCudaTask", task.getName().c_str());
  EXPECT_FALSE(task.isStartTask());

  delete [] gpuIds;
  delete contexts;
}


htgs::TaskGraphConf<SimpleCudaData, SimpleCudaData> *createCudaGraph(int numChain, size_t numGPUs, int *gpuIds, CUcontext *contexts)
{
  htgs::TaskGraphConf<SimpleCudaData, SimpleCudaData> *tg = new htgs::TaskGraphConf<SimpleCudaData, SimpleCudaData>();

  SimpleCudaTask *t;

  SimpleCudaTask *startTask = nullptr, *endTask = nullptr;

  SimpleCudaTask *prevTask = nullptr;
  for (int chain = 0; chain < numChain; chain++) {
    t = new SimpleCudaTask(contexts, gpuIds, numGPUs);

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
  tg->addCudaMemoryManagerEdge("cudaMemEdge", startTask, alloc, 1, htgs::MMType::Static, contexts);

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
  EXPECT_EQ(numDataGenerated, count);
  EXPECT_NO_FATAL_FAILURE(delete rt);
}

void simpleCudaGraphCreation()
{
  size_t numGpus = 1;
  int *gpuIds = new int[1]{0};
  CUcontext *contexts = initCuda(1, gpuIds);

  auto graph1 = createCudaGraph(1, numGpus, gpuIds, contexts);
  auto graph2 = createCudaGraph(2, numGpus, gpuIds, contexts);
  auto graph3 = createCudaGraph(10, numGpus, gpuIds, contexts);
  auto graph4 = createCudaGraph(100, numGpus, gpuIds, contexts);

  EXPECT_NO_FATAL_FAILURE(delete graph1);
  EXPECT_NO_FATAL_FAILURE(delete graph2);
  EXPECT_NO_FATAL_FAILURE(delete graph3);
  EXPECT_NO_FATAL_FAILURE(delete graph4);

  shutdownCuda(numGpus, contexts);
  delete []gpuIds;
}

void simpleCudaGraphExecution()
{
  size_t numGpus = 1;
  int *gpuIds = new int[1]{0};
  CUcontext *contexts = initCuda(1, gpuIds);

  auto graph1 = createCudaGraph(1, numGpus, gpuIds, contexts);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph1, 1));

  auto graph2 = createCudaGraph(1, numGpus, gpuIds, contexts);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph2, 10));

  auto graph3 = createCudaGraph(1, numGpus, gpuIds, contexts);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph3, 100));

  auto graph4 = createCudaGraph(10, numGpus, gpuIds, contexts);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph4, 1));

  auto graph5 = createCudaGraph(10, numGpus, gpuIds, contexts);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph5, 10));

  auto graph6 = createCudaGraph(10, numGpus, gpuIds, contexts);
  EXPECT_NO_FATAL_FAILURE(launchCudaGraph(graph6, 100));


  shutdownCuda(numGpus, contexts);
  delete []gpuIds;
}
