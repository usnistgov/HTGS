//
// Created by tjb3 on 2/24/17.
//

#ifndef HTGS_ANYRULESCHEDULERINONLY_HPP
#define HTGS_ANYRULESCHEDULERINONLY_HPP

#include "AnyRuleScheduler.hpp"
namespace htgs {
template<class T>
class AnyRuleSchedulerInOnly : public AnyRuleScheduler {

 public:
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
#endif //HTGS_ANYRULESCHEDULERINONLY_HPP
