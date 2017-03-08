//
// Created by tjb3 on 2/24/17.
//

#ifndef HTGS_ANYRULEMANAGERINONLY_HPP
#define HTGS_ANYRULEMANAGERINONLY_HPP

#include "AnyRuleManager.hpp"
namespace htgs {
template<class T>
class AnyRuleManagerInOnly : public AnyRuleManager {

 public:

  /**
   * Destructor
   */
  virtual ~AnyRuleManagerInOnly() override {}

  /**
 * @internal
 * Processes the input data, which is forwarded to the IRule synchronously.
 * It is possible the data received is nullptr, at which it will first check for rule termination before
 * processing the null data.
 * @param data the input data
 *
 * @note This function should only be called by the HTGS API
 */
  virtual void executeTask(std::shared_ptr<T> data) = 0;

};

}
#endif //HTGS_ANYRULEMANAGERINONLY_HPP
