
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file AnyIRule.hpp
 * @author Timothy Blattner
 * @date Apr 21, 2016
 *
 * @brief Base class for an htgs::IRule to hide the template arguments.
 * @details
 */
#ifndef HTGS_BASEIRULE_HPP
#define HTGS_BASEIRULE_HPP

namespace htgs {
/**
 * @class AnyIRule AnyIRule.hpp <htgs/core/rules/AnyIRule.hpp>
 * @brief Base class for an htgs::IRule.
 */
class AnyIRule {
 public:

  /**
   * Creates an AnyIRule with locks enabled.
   */
  AnyIRule() : useLocks(true) {}

  /**
   * Creates an AnyIRule with locks specified
   * @param useLocks whether to use locks on the rule or not to ensure one thread accesses the rule at a time
   */
  AnyIRule(bool useLocks) : useLocks(useLocks) {}


  /**
   * Destructor
   */
  virtual ~AnyIRule() {}

  /**
   * Virtual function to determine if a rule is ready to be terminated.
   * If there is no more data entering the RuleManager that is managing this IRule,
   * then the rule will be automatically terminated.
   * @param pipelineId the pipelineId associated with this rule
   * @return whether the rule should be terminated or not
   * @retval TRUE if the rule should be terminated
   * @retval FALSE if the rule should not be terminated
   * @note The rule will automatically be terminated if the input ITask has terminated.
   */
  virtual bool canTerminateRule(size_t pipelineId) = 0;

  /**
   * Virtual function that handles when a rule is being shutdown for a particular pipelineId
   * @param pipelineId the pipelineId to shutdown
   * @note This function can be used to release memory, but if there are multiple pipelines
   * managed by an ExecutionPipeline, then the memory release should occur in a destructor.
   */
  virtual void shutdownRule(size_t pipelineId) = 0;

  /**
   * Virtual function to get the name of the IRule
   * @return the name of the IRule
   */
  virtual std::string getName() = 0;

  /**
   * Gets the mutex associated with this IRule
   * @return the mutex
   *
   * @note This function should only be called by the HTGS API
   */
  std::mutex &getMutex() {
    return mutex;
  }

  /**
   * Gets whether the rule should use locks or not.
   * @return TRUE if locks should be used, otherwise false
   * @retval TRUE the lock will be used to ensure mutual exclusion when accessing this rule across multiple threads
   * @retval FALSE the lock will not be used, and any thread may access the rule asynchronously
   */
  bool canUseLocks() const {
    return useLocks;
  }

 private:
  std::mutex
      mutex; //!< The mutex associated with this IRule to ensure no more than one thread is processing the rule at a time
  bool useLocks; //!< Will enable using the mutex to lock the rule to ensure this rule is only accessed by a thread at a time
};
}

#endif //HTGS_BASEIRULE_HPP
