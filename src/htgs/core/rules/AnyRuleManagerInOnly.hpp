// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file AnyRuleManagerInOnly.hpp
 * @author Timothy Blattner
 * @date February 24, 2017
 *
 * @brief Implements the base class for the rule manager, but only providing the input type.
 */
#ifndef HTGS_ANYRULEMANAGERINONLY_HPP
#define HTGS_ANYRULEMANAGERINONLY_HPP

#include "AnyRuleManager.hpp"
namespace htgs {
/**
 * @class AnyRuleManagerInOnly AnyRuleManagerInOnly.hpp <htgs/core/rules/AnyRuleManagerInOnly.hpp>
 * @brief Implements the base class for the rule manager, but only provides the input type.
 *
 * Contains functions that are specific only to the input type for the rule manager. This
 * is used by the bookkeeper for executing the rule manager.
 *
 * @tparam T the input type
 */
template<class T>
class AnyRuleManagerInOnly : public AnyRuleManager {

 public:

  /**
   * Destructor
   */
  virtual ~AnyRuleManagerInOnly() override {}

  /**
 * Processes the input data, which is forwarded to the IRule synchronously.
 * It is possible the data received is nullptr, at which it will first check for rule termination before
 * processing the null data.
 * @param data the input data
 *
 * @note This function should only be called by the HTGS API
   * @internal
 */
  virtual void executeTask(std::shared_ptr<T> data) = 0;

};

}
#endif //HTGS_ANYRULEMANAGERINONLY_HPP
