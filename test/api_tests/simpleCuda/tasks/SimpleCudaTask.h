//
// Created by tjb3 on 3/31/17.
//

#ifndef HTGS_SIMPLECUDATASK_H
#define HTGS_SIMPLECUDATASK_H

#include <htgs/api/ICudaTask.hpp>
#include "../data/SimpleCudaData.h"
class SimpleCudaTask : public htgs::ICudaTask<SimpleCudaData, SimpleCudaData> {
 public:
  SimpleCudaTask(CUcontext *contexts, int *cudaIds, size_t numGpus);

  void executeTask(std::shared_ptr<SimpleCudaData> data) override;
  htgs::ITask<SimpleCudaData, SimpleCudaData> *copy() override;

  std::string getName() override;

  void setDoReleaseMemory(bool doReleaseMemory) { this->doReleaseMemory = doReleaseMemory; }

 private:
  bool doReleaseMemory;
};

#endif //HTGS_SIMPLECUDATASK_H
