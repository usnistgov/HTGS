
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file BaseRuleManager.hpp
 * @author Timothy Blattner
 * @date Nov 18, 2015
 *
 * @brief Implements the parent class for a RuleManager that removes the output template type for the RuleManager.
 */
#ifndef HTGS_BASERULEMANAGER_H
#define HTGS_BASERULEMANAGER_H

#include <functional>
#include "BaseBaseRuleManager.hpp"
#include "../../api/IData.hpp"

namespace htgs {
/**
 * @class BaseRuleManager BaseRuleManager.hpp <htgs/core/rules/BaseRuleManager.hpp>
 * @brief Parent class for the RuleManager that removes the output template type of the RuleManager.
 * @details
 * By removing the output type, a Bookkeeper is able to store multiple types of RuleManagers producing
 * data for multiple output types.
 */
template<class T>
class BaseRuleManager: public BaseBaseRuleManager {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");

 public:

  /**
   * Destructor
   */
  virtual ~BaseRuleManager() { }

  /**
   * @internal
   * Processes the input data, which is forwarded to each IRule synchronously.
   * The output for all IRules are aggregated and shipped to the output Connector.
   * @param data the input data
   *
   * @note This function should only be called by the HTGS API
   */
  virtual void executeTask(std::shared_ptr<T> data) {
    std::cerr << "Called BaseRuleManager 'executeTask' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * @internal
   * Initializes the RuleManager.
   *
   * @param pipelineId the pipelineID
   * @param numPipelines the number of pipelines
   *
   * @note This function should only be called by the HTGS API
   */
  virtual void initialize(int pipelineId, int numPipelines) {
    std::cerr << "Called BaseRuleManager 'initialize' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * @internal
   * Shuts down the RuleManager.
   * Will also shutdown each IRule associated with the RuleManager
   *
   * @note This function should only be called by the HTGS API
   */
  virtual void shutdown() {
    std::cerr << "Called BaseRuleManager 'shutdown' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * @internal
   * Checks whether the RuleManager is terminated or not
   * @param inputConnector the input connector for the RuleManager
   * @return Whether the RuleManager is terminated or not
   * @retval TRUE if the RuleManager is terminated
   * @retval FALSE if the RuleManager is not terminated
   *
   * @note This function should only be called by the HTGS API
   */
  virtual bool isTerminated(std::shared_ptr<BaseConnector> inputConnector) {
    std::cerr << "Called BaseRuleManager 'isTerminated' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the name of the RuleManager and the names of all IRules that it manages.
   * @return the name
   */
  virtual std::string getName() {
    std::cerr << "Called BaseRuleManager 'getName' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * @internal
   * Creates a copy of the RuleManager.
   * The original and all copies share the same set of rules and access them synchronously.
   * @return the RuleManager copy
   *
   * @note This function should only be called by the HTGS API
   */
  virtual BaseRuleManager<T> *copy() {
    std::cerr << "Called BaseRuleManager 'copy' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Provides debug output
   * @note \#define DEBUG_FLAG to enable debugging
   */
  virtual void debug() {
    std::cerr << "Called BaseRuleManager 'debug' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * @internal
   * Gets the output connector associated with the RuleManager
   * @return the output connector
   *
   * @note This function should only be called by the HTGS API
   */
  virtual std::shared_ptr<BaseConnector> getConnector() {
    std::cerr << "Called BaseRuleManager 'getConnector' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * @internal
   * Sets the output connector that the RuleManager is attached to
   * @param connector the output connector
   *
   * @note This function should only be called by the HTGS API
   */
  virtual void setOutputConnector(std::shared_ptr<BaseConnector> connector) {
    std::cerr << "Called BaseRuleManager 'setOutputConnector' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the rule names associated with the RuleManager
  */
  virtual std::string getRuleNames() {
    std::cerr << "Called BaseRuleManager 'getRuleNames' virtual function" << std::endl;
    throw std::bad_function_call();
  }
};
}
#endif //HTGS_BASERULEMANAGER_H
