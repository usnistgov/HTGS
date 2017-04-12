// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file MemoryEdge.hpp
 * @author Timothy Blattner
 * @date March 2, 2017
 *
 * @brief Implements the memory edge, which is an edge descriptor.
 */

#ifndef HTGS_MEMORYEDGE_HPP
#define HTGS_MEMORYEDGE_HPP

#include <htgs/core/memory/MemoryManager.hpp>
#include "EdgeDescriptor.hpp"

namespace htgs {

/**
 * @class MemoryEdge MemoryEdge.hpp <htgs/core/graph/edge/MemoryEdge.hpp>
 * @brief Implements the memory edge that is added to the graph.
 *
 * This edge connects a memory manager to a task that is receiving the memory.
 *
 * When applying the edge, the memory manager task is created and its associated input and output connectors. The
 * output connector is added to the task that is getting memory to receive the memory data from the memory manager.
 *
 * During edge copying the task getting memory, and the memory manager are copied. The memory edge name is reused.
 *
 * @tparam T the type of data that is allocated by the memory manager
 */
template<class T>
class MemoryEdge : public EdgeDescriptor {
 public:
  /**
   * Creates a memory edge.
   * @param memoryEdgeName the name of the memory edge
   * @param getMemoryTask the task getting memory
   * @param memoryManager the memory manager task
   */
  MemoryEdge(const std::string &memoryEdgeName,
             AnyITask *getMemoryTask,
             MemoryManager<T> *memoryManager)
      : memoryEdgeName(memoryEdgeName),
        getMemoryTask(getMemoryTask),
        memoryManager(memoryManager) {}

  ~MemoryEdge() override {}

  void applyEdge(AnyTaskGraphConf *graph) override {

    // Check to make sure that the getMemoryTask or releaseMemoryTasks do not have this named edge already
    if (getMemoryTask->hasMemoryEdge(memoryEdgeName))
      throw std::runtime_error(
          "Error getMemoryTask: " + getMemoryTask->getName() + " already has the memory edge: " + memoryEdgeName);

    if (!graph->hasTask(getMemoryTask))
      throw std::runtime_error("Error getMemoryTask: " + getMemoryTask->getName()
                                   + " must be added to the graph you are connecting the memory edge too.");

    auto memTaskManager = graph->getTaskManager(memoryManager);

    auto getMemoryConnector = std::shared_ptr<Connector<MemoryData<T>>>(new Connector<MemoryData<T>>());
    auto releaseMemoryConnector = std::shared_ptr<Connector<MemoryData<T>>>(new Connector<MemoryData<T>>());

    memTaskManager->setInputConnector(releaseMemoryConnector);
    memTaskManager->setOutputConnector(getMemoryConnector);

    releaseMemoryConnector->incrementInputTaskCount();

    getMemoryTask->attachMemoryEdge(memoryEdgeName,
                                    getMemoryConnector,
                                    releaseMemoryConnector,
                                    memoryManager->getType());
  }
  EdgeDescriptor *copy(AnyTaskGraphConf *graph) override {
    return new MemoryEdge<T>(memoryEdgeName,
                             graph->getCopy(getMemoryTask),
                             (MemoryManager<T> *) graph->getCopy(memoryManager));
  }
 private:

  std::string memoryEdgeName; //!< The name of the memory edge
  AnyITask *getMemoryTask; //!< The task that is getting memory
  MemoryManager<T> *memoryManager; //!< the memory manager task

};
}

#endif //HTGS_MEMORYEDGE_HPP
