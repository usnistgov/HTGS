//
// Created by tjb3 on 2/22/17.
//

/**
 * @file BaseRuleScheduler.hpp
 * @author Timothy Blattner
 * @date Feb 22, 2017
 *
 * @brief  Implements a BaseRuleScheduler, which connects a Bookkeeper to another ITask using an IRule.
 * @details
 */

#ifndef HTGS_BASERULESCHEDULER_HPP
#define HTGS_BASERULESCHEDULER_HPP

#include "htgs/core/graph/AnyConnector.hpp"

namespace htgs {

/**
 * @class BaseRuleScheduler BaseRuleScheduler.hpp <htgs/core/rules/BaseRuleScheduler.hpp>
 * @brief Connects a Bookkeeper to another ITask using one IRule.
 * @details
 *
 * Removes the template parameters for the RuleScheduler
 *
 * When data is forwarded to the RuleScheduler from the Bookkeeper, the data is passed to an
 * IRule that is associated with the RuleScheduler. Each IRule is responsible for determining when/if
 * data is ready to be sent to the ITask the RuleScheduler is bound to.
 *
 * The input and output types of each IRule added to a RuleScheduler
 * must match the input and output types of the RuleScheduler.
 *
 * Example Usage:
 * @code
 * htgs::TaskGraph<htgs::VoidData, htgs::VoidData> *taskGraph = new htgs::TaskGraph<htgs::VoidData, htgs::VoidData>();
 * htgs::Bookkeeper<Data1> *bkTask = new htgs::Bookkeeper<Data1>();
 *
 * // DataRule implements IRule<Data1, Data2> to be compatible with ruleMan
 * DataRule *rule = new DataRule();
 *
 * // The output task for the RuleScheduler with input type Data2
 * Data2ProcessingTask *data2Task = new Data2ProcessingTask();
 *
 * // Creates edge between the Bookkeeper and the Data2ProcessingTask, where rule defines when data is sent
 * taskGraph->addRule(bkTask, data2Task, rule);
 * @endcode
 *
 * @tparam T the input data type for the RuleScheduler, T must derive from IData.
 * @tparam U the output data type for the RuleScheduler, U  must derive from IData.
 */
class AnyRuleScheduler {
 public:
  /**
   * @internal
   * Initializes the RuleScheduler.
   *
   * @param pipelineId the pipelineID
   * @param numPipelines the number of pipelines
   *
   * @note This function should only be called by the HTGS API
   */
  virtual void initialize(size_t pipelineId, size_t numPipelines) = 0;

  /**
   * @internal
   * Shuts down the RuleScheduler.
   * Will also shutdown the rule associated with the RuleScheduler
   * Only called if the bookkeeper associated with the RuleScheduler is shutting down (all rules schedulers are
   * closed and input is no longer producing data)
   *
   * @note This function should only be called by the HTGS API
   */
  virtual void shutdown() = 0;

  /**
   * @internal
   * Checks whether the RuleScheduler is terminated or not
   * @param inputConnector the input connector for the RuleScheduler
   * @return Whether the RuleScheduler is terminated or not
   * @retval TRUE if the RuleScheduler is terminated
   * @retval FALSE if the RuleScheduler is not terminated
   *
   * @note This function should only be called by the HTGS API
   */
  virtual bool isTerminated() = 0;


  /**
  * @internal
  * Sets the output connector that the RuleScheduler is attached to
  * @param connector the output connector
  *
  * @note This function should only be called by the HTGS API
  */
  virtual void setOutputConnector(std::shared_ptr<AnyConnector> connector) = 0;


  /**
   * @internal
   * Creates a copy of the RuleScheduler.
   * The original and all copies share the same rule and access them synchronously.
   * @return the RuleScheduler copy
   *
   * @note This function should only be called by the HTGS API
   */
  virtual AnyRuleScheduler *copy() = 0;

 /**
  * @internal
  * Gets the output connector associated with the RuleScheduler
  * @return the output connector
  *
  * @note This function should only be called by the HTGS API
  */
  virtual std::shared_ptr<AnyConnector> getConnector() = 0;

  /**
   * Gets the name of the RuleScheduler and the names of all IRules that it manages.
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
#endif //HTGS_BASERULESCHEDULER_HPP
