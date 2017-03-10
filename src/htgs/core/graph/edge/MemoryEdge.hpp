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
             MemoryManager<T> *memoryManager)
      : memoryEdgeName(memoryEdgeName),
        getMemoryTask(getMemoryTask),
        memoryManager(memoryManager)
        {}

~MemoryEdge() override { }

  void applyEdge(AnyTaskGraphConf *graph) override {

    // Check to make sure that the getMemoryTask or releaseMemoryTasks do not have this named edge already
    if (getMemoryTask->hasMemoryEdge(memoryEdgeName))
      throw std::runtime_error("Error getMemoryTask: " + getMemoryTask->getName() + " already has the memory edge: " + memoryEdgeName);

    if (!graph->hasTask(getMemoryTask))
      throw std::runtime_error("Error getMemoryTask: " + getMemoryTask->getName() + " must be added to the graph you are connecting the memory edge too.");

    auto memTaskManager = graph->getTaskManager(memoryManager);

    auto getMemoryConnector = std::shared_ptr<Connector<MemoryData<T>>>(new Connector<MemoryData<T>>());
    auto releaseMemoryConnector = std::shared_ptr<Connector<MemoryData<T>>>(new Connector<MemoryData<T>>());

    memTaskManager->setInputConnector(releaseMemoryConnector);
    memTaskManager->setOutputConnector(getMemoryConnector);

    releaseMemoryConnector->incrementInputTaskCount();

    getMemoryTask->attachMemoryEdge(memoryEdgeName, getMemoryConnector, releaseMemoryConnector, memoryManager->getType());
  }
  EdgeDescriptor *copy(AnyTaskGraphConf *graph) override {
    return new MemoryEdge<T>(memoryEdgeName, graph->getCopy(getMemoryTask), (MemoryManager<T> *)graph->getCopy(memoryManager));
  }
 private:

  std::string memoryEdgeName;
  AnyITask *getMemoryTask;
  MemoryManager<T> *memoryManager;

};
}

#endif //HTGS_MEMORYEDGE_HPP
