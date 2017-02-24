
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file MemoryManagerKey.hpp
 * @author Timothy Blattner
 * @date Nov 23, 2015
 *
 * @brief Provides functionality for copying a MemoryManager in a TaskGraph
 */
#ifndef HTGS_MEMORYMANAGERKEY_H
#define HTGS_MEMORYMANAGERKEY_H

#include "htgs/core/task/AnyTaskScheduler.hpp"

namespace htgs {
/**
 * @class MemoryManagerKey MemoryManagerKey.hpp <htgs/core/graph/history/MemoryManagerKey.hpp>
 * @brief Provides functionality for copying a MemoryManager in a TaskGraph
 * @note This class should only be called by the HTGS API
 */
class MemoryManagerKey {
 public:
  /**
   * Creates the memory manager key that describes how the MemoryManager is added into the TaskGraph
   * @param name the name of the edge
   * @param memGetter the ITask getting memory
   * @param memReleaser the ITask releasing memory
   * @param memTask the MemoryManager Task
   * @param type the memory manager type
   * @param isReleaserOutsideGraph whether the mem releaser is outside of the graph or not
   */
  MemoryManagerKey(std::string name,
                   AnyITask *memGetter,
                   AnyITask *memReleaser,
                   AnyTaskScheduler *memTask,
                   MMType type,
                   bool isReleaserOutsideGraph) {
    this->name = name;
    this->memGetter = memGetter;
    this->memReleaser = memReleaser;
    this->memTask = memTask;
    this->type = type;
    this->isReleaserOutsideGraph = isReleaserOutsideGraph;
  }

  /**
   * Destructor
   */
  ~MemoryManagerKey() { }

  /**
   * Gets the name of the edge
   * @return the name
   */
  std::string getName() const {
    return this->name;
  }

  /**
   * Gets the ITask that is getting memory
   * @return the ITask getting memory
   */
  AnyITask *getMemGetter() const {
    return this->memGetter;
  }

  /**
   * Gets the ITask that is releasing memory
   * @return the ITask releasing memory
   */
  AnyITask *getMemReleaser() const {
    return this->memReleaser;
  }

  /**
   * Gets the MemoryManager Task
   * @return the MemoryManager Task
   */
  AnyTaskScheduler *getMemTask() const {
    return this->memTask;
  }

  /**
   * Gets the memory manager type
   * @return the memory manager type
   */
  MMType getMMType() const {
    return this->type;
  }

  /**
   * Gets whether the memory releaser is outside the graph or not
   * @return whether the memory releaser is outside of the graph
   * @retval TRUE if the memory releaser exists outside of the graph that the memory edge is being added to
   * @retval FALSE if the memory releaser exists inside of the graph that the memory edge is being added to
   */
  bool isIsReleaserOutsideGraph() const {
    return isReleaserOutsideGraph;
  }
 private:
  MMType type; //!< The memory manager type
  std::string name; //!< The name of the memory edge
  AnyITask *memGetter; //!< The ITask that is getting memory
  AnyITask *memReleaser; //!< The ITask that is releasing memory
  AnyTaskScheduler *memTask; //!< The TaskScheduler that manages the MemoryManager
  bool isReleaserOutsideGraph; //!< Whether the memReleaser is outside the graph or not
};
}

#endif //HTGS_PRODUCERCONSUMERKEY_H
