
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file BaseITask.hpp
 * @author Timothy Blattner
 * @date Nov 23, 2015
 *
 * @brief Implements parent ITask, removing template arguments.
 * @details
 */
#ifndef HTGS_BASEITASK_H
#define HTGS_BASEITASK_H


#include "../memory/MMType.h"
#include "../graph/BaseConnector.hpp"
#include "../rules/BaseBaseRuleManager.hpp"

namespace htgs {
class BaseBaseRuleManager;

/**
 * @class BaseITask BaseITask.hpp <htgs/core/task/BaseITask.hpp>
 * @brief Implements the parent ITask, which removes the template arguments of an ITask.
 * @details
 * Used anywhere the template arguments for an ITask are not needed.
 *
 */
class BaseITask {
 public:
  /**
   * Destructor
   */
  virtual ~BaseITask() { }

  /**
   * Pure virtual function to copy an ITask
   * @return the copy of the ITask
   */
  virtual BaseITask *copy() = 0;

  /**
   * Pure virtual function to get the name of an ITask
   * @return the name of the ITask
   */
  virtual std::string getName() = 0;

  /**
   * Virtual function used for Bookkeeper
   * @param ruleManager the RuleManager to add
   * @note This function should only be called by the HTGS API
   */
  virtual void addRuleManager(BaseBaseRuleManager *ruleManager) {
    std::cerr << "Called BaseITask 'addRuleManager' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Sets the pipeline Id for this ITask.
   * @param pipelineId the pipelineId
   * @note This function should only be used by the HTGS API
   */
  virtual void setPipelineId(int pipelineId) {
    std::cerr << "Called BaseITask 'setPipelineId' virtual function" << std::endl;
    throw std::bad_function_call();  }

  /**
   * Virtual function for attaching a memGetter Connector.
   * The memGetter Connector is the output Connector for a MemoryManager.
   * @param name the name of the memory edge.
   * @param connector the connector of the MemoryManager to attach.
   * @param type the memory manager type
   * @note This function should only be called by the HTGS API
   */
  virtual void attachMemGetter(std::string name, std::shared_ptr<BaseConnector> connector, MMType type) {
    std::cerr << "Called BaseITask 'attachMemGetter' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Virtual function for attached a memReleaser Connector.
   * The memReleaser Connector is the input Connector for a MemoryManager.
   * @param name the name of the memory edge.
   * @param connector the connector of the MemoryManager to attach.
   * @param type the memory manager type
   * @param outsideMemManGraph indicates if this ITask exists outside of the graph where the memory manager is
   * @note This function should only be called by the HTGS API
   */
  virtual void attachMemReleaser(std::string name, std::shared_ptr<BaseConnector> connector, MMType type, bool outsideMemManGraph) {
    std::cerr << "Called BaseITask 'attachMemReleaser' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
 * Checks whether this ITask contains a memGetter for a specified name
 * @param name the name of the memGetter
 * @return whether this ITask has a memGetter with the specified name
 * @retval TRUE if the ITask has a memGetter with the specified name
 * @retval FALSE if the ITask does not have a memGetter with the specified name
 * @note To add a memGetter to this ITask use TaskGraph::addMemoryManagerEdge
 */
  virtual bool hasMemGetter(std::string name) {
    std::cerr << "Called BaseITask 'hasMemGetter' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Checks whether this ITask contains a memReleaser for a specified name
   * @param name the name of the memReleaser
   * @return whether this ITask has a memReleaser with the specified name
   * @retval TRUE if the ITask has a memReleaser with the specified name
   * @retval FALSE if the ITask does not have a memReleaser with the specified name
   * @note To add a memReleaser to this ITask use TaskGraph::addMemoryManagerEdge
   */
  virtual bool hasMemReleaser(std::string name) {
    std::cerr << "Called BaseITask 'hasMemReleaser' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the id used for dot nodes.
   */
  virtual std::string getDotId() {
    std::cerr << "Called BaseITask 'getDotId' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the memory manager type for a given name
   * @param name the name of the memory manager edge
   * @return the memory manager type for the specified name
   */
  virtual MMType getMemoryManagerType(std::string name) {
    std::cerr << "Called BaseITask 'getMemoryManagerType' virtual function" << std::endl;
    throw std::bad_function_call();
  }

};
}

#endif //HTGS_BASEITASK_H
