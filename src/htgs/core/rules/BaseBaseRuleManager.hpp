
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file BaseBaseRuleManager.hpp
 * @author Timothy Blattner
 * @date Nov 23, 2015
 *
 * @brief Holds parent class for BaseRuleManager removing all template arguments.
 */
#ifndef HTGS_BASEBASERULEMANAGER_H
#define HTGS_BASEBASERULEMANAGER_H

#include <iostream>
#include <functional>

namespace htgs {
class BaseConnector;

/**
* @class BaseBaseRuleManager BaseBaseRuleManager.hpp <htgs/core/rules/BaseBaseRuleManager.hpp>
* @brief Parent class for BaseRuleManager, which removes all template arguments for RuleManager
* @details
* The BaseBaseRuleManager strips all of the template arguments so multiple varieties of RuleManager
* types can be stored.
*
* @note Make remaining functions pure virtual functions
*/
class BaseBaseRuleManager {
 public:

  /**
   * Destructor
   */
  virtual ~BaseBaseRuleManager() { }

  /**
   * Creates a copy of the RuleManager
   * @return the copy of the RuleManager
   * @note This function should only be called by the HTGS API
   */
  virtual BaseBaseRuleManager *copy() {
    std::cerr << "Called BaseBaseRuleManager 'copy' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the connector that the RuleManager is producing data for.
   * @return the connector that the RuleManager is producing data for
   */
  virtual std::shared_ptr<BaseConnector> getConnector() {
    std::cerr << "Called BaseBaseRuleManager 'getConnector' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Sets the output connector that the RuleManager is producing data for.
   * @param connector the connector the RuleManager will produce data for.
   * @note This function should only be called by the HTGS API
   */
  virtual void setOutputConnector(std::shared_ptr<BaseConnector> connector) {
    std::cerr << "Called BaseBaseRuleManager 'setOutputConnector' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the name of the RuleManager.
   * @return the name of the RuleManager.
   */
  virtual std::string getName() {
    std::cerr << "Called BaseBaseRuleManager 'getName' virtual function" << std::endl;
    throw std::bad_function_call();
  }
};
}

#endif //HTGS_BASEBASERULEMANAGER_H
