//
// Created by tjb3 on 3/31/17.
//

#include "SimpleCudaTask.h"
#include "../memory/SimpleCudaReleaseRule.h"
SimpleCudaTask::SimpleCudaTask(CUcontext *contexts, int *cudaIds, size_t numGpus) : ICudaTask(contexts,
                                                                                                 cudaIds,
                                                                                                 numGpus),
                                                                                    doReleaseMemory(false) {}
void SimpleCudaTask::executeTask(std::shared_ptr<SimpleCudaData> data) {

  if (this->hasMemoryEdge("cudaMemEdge"))
  {
    data->setCudaData(this->getMemory<double>("cudaMemEdge", new SimpleCudaReleaseRule()));
  }

  if (doReleaseMemory) {
    this->releaseMemory(data->getCudaData());
  }

  addResult(data);
}
htgs::ITask<SimpleCudaData, SimpleCudaData> *SimpleCudaTask::copy() {
  return new SimpleCudaTask(getContexts(), getCudaIds(), getNumGPUs());
}
std::string SimpleCudaTask::getName() {
  return "SimpleCudaTask";
}
