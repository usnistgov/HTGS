//
// Created by tjb3 on 3/31/17.
//

#include "SimpleCudaTask.h"
#include "../memory/SimpleCudaReleaseRule.h"
SimpleCudaTask::SimpleCudaTask(int *cudaIds, size_t numGpus) : ICudaTask(cudaIds, numGpus), doReleaseMemory(false) {}
void SimpleCudaTask::executeTask(std::shared_ptr<SimpleCudaData> data) {

  if (this->hasMemoryEdge("cudaMemEdge"))
  {
    data->setCudaData(this->getMemory<double>("cudaMemEdge", new SimpleCudaReleaseRule()));
  }

  if (doReleaseMemory) {
    data->getCudaData()->releaseMemory();
  }

  addResult(data);
}
htgs::ITask<SimpleCudaData, SimpleCudaData> *SimpleCudaTask::copy() {
  return new SimpleCudaTask(getCudaIds(), getNumGPUs());
}
std::string SimpleCudaTask::getName() {
  return "SimpleCudaTask";
}
