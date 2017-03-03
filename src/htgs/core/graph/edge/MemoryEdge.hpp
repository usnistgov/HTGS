//
// Created by tjb3 on 3/2/17.
//

#ifndef HTGS_MEMORYEDGE_HPP
#define HTGS_MEMORYEDGE_HPP

#include <htgs/core/memory/MemoryManager.hpp>
#include "EdgeDescriptor.hpp"

#ifdef USE_CUDA
#include <cuda.h>
#include <htgs/core/memory/CudaMemoryManager.hpp>
#endif

namespace htgs {
template <class T>
class MemoryEdge : public EdgeDescriptor
{
 public:
  MemoryEdge(const std::string &memoryEdgeName,
             AnyITask *getMemoryTask,
             AnyITask *releaseMemoryTask,
             const std::shared_ptr<IMemoryAllocator<T>> &allocator,
             size_t memoryPoolSize,
             MMType managerType)
      : memoryEdgeName(memoryEdgeName),
        getMemoryTask(getMemoryTask),
        releaseMemoryTask(releaseMemoryTask),
        allocator(allocator),
        memoryPoolSize(memoryPoolSize),
        managerType(managerType),
        cudaMemoryManager(false) {}

#ifdef USE_CUDA
  MemoryEdge(const std::string &memoryEdgeName,
             AnyITask *getMemoryTask,
             AnyITask *releaseMemoryTask,
             const std::shared_ptr<IMemoryAllocator<T>> &allocator,
             size_t memoryPoolSize,
             MMType managerType,
             CUcontext *contexts)
      : memoryEdgeName(memoryEdgeName),
        getMemoryTask(getMemoryTask),
        releaseMemoryTask(releaseMemoryTask),
        allocator(allocator),
        memoryPoolSize(memoryPoolSize),
        managerType(managerType),
        cudaMemoryManager(true)
        contexts(contexts) {}
#endif

~MemoryEdge() override {

  }
  void applyEdge(AnyTaskGraph *graph) override {

    // Check to make sure that the getMemoryTask or releaseMemoryTasks do not have this named edge already
    if (getMemoryTask->hasGetMemoryEdge(memoryEdgeName))
      throw std::runtime_error("Error getMemoryTask: " + getMemoryTask->getName() + " already has the memory edge: " + memoryEdgeName);

    if (releaseMemoryTask->hasReleaseMemoryEdge(memoryEdgeName))
      throw std::runtime_error("Error getMemoryTask: " + releaseMemoryTask->getName() + " already has the memory edge: " + memoryEdgeName);

    if (!graph->hasTask(getMemoryTask))
      throw std::runtime_error("Error getMemoryTask: " + getMemoryTask->getName() + " must be added to the graph you are connecting the memory edge too.");

    if (!graph->hasTask(releaseMemoryTask))
      throw std::runtime_error("Error releaseMemoryTask: " + releaseMemoryTask->getName() + " must be added to the graph you are connecting the memory edge too.");

    // Create the memory manager task . . .
    MemoryManager<T> *memoryManager = nullptr;
#ifdef USE_CUDA
    if (cudaMemoryManager)
      memoryManager = new CudaMemoryManager<T>(memoryEdgeName, contexts, memoryPoolSize, allocator, managerType);
    else
      memoryManager = new MemoryManager<T>(memoryEdgeName, memoryPoolSize, allocator, managerType);
#else
    memoryManager = new MemoryManager<T>(memoryEdgeName, memoryPoolSize, allocator, managerType);
#endif

    auto memTaskScheduler = graph->getTaskScheduler(memoryManager);

    auto getMemoryConnector = std::shared_ptr<Connector<MemoryData<T>>>(new Connector<MemoryData<T>>());
    auto releaseMemoryConnector = std::shared_ptr<Connector<MemoryData<T>>>(new Connector<MemoryData<T>>());

    memTaskScheduler->setInputConnector(releaseMemoryConnector);
    memTaskScheduler->setOutputConnector(getMemoryConnector);

    releaseMemoryConnector->incrementInputTaskCount();

    getMemoryTask->attachGetMemoryEdge(memoryEdgeName, getMemoryConnector, managerType);

    // TODO: Remove outside graph boolean
    releaseMemoryTask->attachReleaseMemoryEdge(memoryEdgeName, releaseMemoryConnector, managerType, false);

  }
  EdgeDescriptor *copy(AnyTaskGraph *graph) override {
#ifdef USE_CUDA
    if (cudaMemoryManager)
      return new MemoryEdge<T>(memoryEdgeName, graph->getCopy(getMemoryTask), graph->getCopy(releaseMemoryTask), allocator, memoryPoolSize, managerType, contexts);
    else
      return new MemoryEdge<T>(memoryEdgeName, graph->getCopy(getMemoryTask), graph->getCopy(releaseMemoryTask), allocator, memoryPoolSize, managerType);
#else
    return new MemoryEdge<T>(memoryEdgeName, graph->getCopy(getMemoryTask), graph->getCopy(releaseMemoryTask), allocator, memoryPoolSize, managerType);

#endif
  }
 private:

  std::string memoryEdgeName;
  AnyITask *getMemoryTask;
  AnyITask *releaseMemoryTask;
  std::shared_ptr<IMemoryAllocator<T>> allocator;
  size_t memoryPoolSize;
  MMType managerType;

  bool cudaMemoryManager;

#ifdef USE_CUDA
  CUcontext *contexts;
#endif

};
}

#endif //HTGS_MEMORYEDGE_HPP
