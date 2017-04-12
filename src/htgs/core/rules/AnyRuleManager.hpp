//
// Created by tjb3 on 2/22/17.
//

/**
 * @file AnyRuleManager.hpp
 * @author Timothy Blattner
 * @date Feb 22, 2017
 *
 * @brief  Implements a AnyRuleManager, which connects a Bookkeeper to another ITask using an IRule.
 * @details
 */

#ifndef HTGS_ANYRULEMANAGER_HPP
#define HTGS_ANYRULEMANAGER_HPP

#include "htgs/core/graph/AnyConnector.hpp"

namespace htgs {

/**
 * @class AnyRuleManager AnyRuleManager.hpp <htgs/core/rules/AnyRuleManager.hpp>
 * @brief Connects a Bookkeeper to another ITask using one IRule.
 * @details
 *
 * Removes the template parameters for the RuleManager
 *
 * When data is forwarded to the RuleManager from the Bookkeeper, the data is passed to an
 * IRule that is associated with the RuleManager. Each IRule is responsible for determining if/when
 * data is ready to be sent to the ITask that the RuleManager is bound to.
 *
 * The input and output types of each IRule added to a RuleManager
 * must match the input and output types of the RuleManager.
 *
 * Example Usage:
 * @code
 * htgs::TaskGraphConf<htgs::VoidData, htgs::VoidData> *taskGraph = new htgs::TaskGraphConf<htgs::VoidData, htgs::VoidData>();
 * htgs::Bookkeeper<Data1> *bkTask = new htgs::Bookkeeper<Data1>();
 *
 * // DataRule implements IRule<Data1, Data2> to be compatible with ruleMan
 * DataRule *rule = new DataRule();
 *
 * // The output task for the RuleManager with input type Data2
 * Data2ProcessingTask *data2Task = new Data2ProcessingTask();
 *
 * // Creates edge between the Bookkeeper and the Data2ProcessingTask, where rule defines when data is sent
 * taskGraph->addRuleEdge(bkTask, rule, data2Task);
 * @endcode
 *
 * @tparam T the input data type for the RuleManager, T must derive from IData.
 * @tparam U the output data type for the RuleManager, U  must derive from IData.
 */
class AnyRuleManager {
 public:

  /**
   * Destructor
   */
  virtual ~AnyRuleManager() {}

  /**
   * Initializes the RuleManager.
   *
   * @param pipelineId the pipelineID
   * @param numPipelines the number of pipelines
   * @param address the address for the bookkeeper task
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  virtual void initialize(size_t pipelineId, size_t numPipelines, std::string address) = 0;

  /**
   * Shuts down the RuleManager.
   * Will also shutdown the rule associated with the RuleManager
   * Only called if the bookkeeper associated with the RuleManager is shutting down (all rules managers are
   * closed and input is no longer producing data)
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  virtual void shutdown() = 0;

  /**
   * Checks whether the RuleManager is terminated or not
   * @return Whether the RuleManager is terminated or not
   * @retval TRUE if the RuleManager is terminated
   * @retval FALSE if the RuleManager is not terminated
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  virtual bool isTerminated() = 0;

  /**
  * Sets the output connector that the RuleManager is attached to
  * @param connector the output connector
  *
  * @note This function should only be called by the HTGS API
   * @internal
  */
  virtual void setOutputConnector(std::shared_ptr<AnyConnector> connector) = 0;

  /**
   * Creates a copy of the RuleManager.
   * The original and all copies share the same rule and access them synchronously.
   * @return the RuleManager copy
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  virtual AnyRuleManager *copy() = 0;

  /**
   * Gets the output connector associated with the RuleManager
   * @return the output connector
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  virtual std::shared_ptr<AnyConnector> getConnector() = 0;

  /**
   * Gets the name of the RuleManager and the names of all IRules that it manages.
   * @return the name
   */
  virtual std::string getName() = 0;

  /**
   * Provides debug output
   * @note \#define DEBUG_FLAG to enable debugging
   */
  virtual void debug() = 0;

};
}
#endif //HTGS_ANYRULEMANAGER_HPP
