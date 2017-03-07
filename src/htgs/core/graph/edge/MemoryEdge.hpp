//
// Created by tjb3 on 3/2/17.
//

#ifndef HTGS_MEMORYEDGE_HPP
#define HTGS_MEMORYEDGE_HPP

#include <htgs/core/memory/MemoryManager.hpp>
#include "EdgeDescriptor.hpp"

namespace htgs {
template <class T>
class MemoryEdge : public EdgeDescriptor
{
 public:
  MemoryEdge(const std::string &memoryEdgeName,
             AnyITask *getMemoryTask,
             AnyITask *releaseMemoryTask,
             MemoryManager<T> *memoryManager)
      : memoryEdgeName(memoryEdgeName),
        getMemoryTask(getMemoryTask),
        releaseMemoryTask(releaseMemoryTask),
        memoryManager(memoryManager)
        {}

~MemoryEdge() override {

  }
  void applyEdge(AnyTaskGraphConf *graph) override {

    // Check to make sure that the getMemoryTask or releaseMemoryTasks do not have this named edge already
    if (getMemoryTask->hasGetMemoryEdge(memoryEdgeName))
      throw std::runtime_error("Error getMemoryTask: " + getMemoryTask->getName() + " already has the memory edge: " + memoryEdgeName);

    if (releaseMemoryTask->hasReleaseMemoryEdge(memoryEdgeName))
      throw std::runtime_error("Error getMemoryTask: " + releaseMemoryTask->getName() + " already has the memory edge: " + memoryEdgeName);

    if (!graph->hasTask(getMemoryTask))
      throw std::runtime_error("Error getMemoryTask: " + getMemoryTask->getName() + " must be added to the graph you are connecting the memory edge too.");

    if (!graph->hasTask(releaseMemoryTask))
      throw std::runtime_error("Error releaseMemoryTask: " + releaseMemoryTask->getName() + " must be added to the graph you are connecting the memory edge too.");

    auto memTaskScheduler = graph->getTaskScheduler(memoryManager);

    auto getMemoryConnector = std::shared_ptr<Connector<MemoryData<T>>>(new Connector<MemoryData<T>>());
    auto releaseMemoryConnector = std::shared_ptr<Connector<MemoryData<T>>>(new Connector<MemoryData<T>>());

    memTaskScheduler->setInputConnector(releaseMemoryConnector);
    memTaskScheduler->setOutputConnector(getMemoryConnector);

    releaseMemoryConnector->incrementInputTaskCount();

    getMemoryTask->attachGetMemoryEdge(memoryEdgeName, getMemoryConnector, memoryManager->getType());

    // TODO: Remove outside graph boolean
    releaseMemoryTask->attachReleaseMemoryEdge(memoryEdgeName, releaseMemoryConnector, memoryManager->getType(), false);

  }
  EdgeDescriptor *copy(AnyTaskGraphConf *graph) override {
    return new MemoryEdge<T>(memoryEdgeName, graph->getCopy(getMemoryTask), graph->getCopy(releaseMemoryTask), (MemoryManager<T> *)graph->getCopy(memoryManager));
  }
 private:

  std::string memoryEdgeName;
  AnyITask *getMemoryTask;
  AnyITask *releaseMemoryTask;
  MemoryManager<T> *memoryManager;

};
}

#endif //HTGS_MEMORYEDGE_HPP
