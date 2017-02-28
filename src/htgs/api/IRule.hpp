
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file IRule.hpp
 * @author Timothy Blattner
 * @date Nov 11, 2015
 *
 * @brief Provides an interface to send data along RuleManager edges for processing state and dependencies
 * @details
 */
#ifndef HTGS_IRULE_HPP
#define HTGS_IRULE_HPP

#include <iostream>
#include <functional>
#include <list>
#include <mutex>
#include <htgs/core/rules/RuleScheduler.hpp>
#include <htgs/core/rules/AnyIRule.hpp>
#include <htgs/types/StateContainer.hpp>

#include "IData.hpp"

namespace htgs {

template <class T, class U>
class RuleScheduler;

template <class T>
class StateContainer;

/**
 * @class IRule IRule.hpp <htgs/api/IRule.hpp>
 * @brief Provides an interface to send data along RuleManager edges for processing state and dependencies
 * @details
 *
 * The IRule is attached to a RuleManager, which can have one or more IRules.
 * Each IRule within a RuleManager, must match the input and output types of the RuleManager.
 * The RuleManager is attached to a Bookkeeper.
 *
 * IRule behavior is defined based on the needs of an algorithm. In many cases, the IRule is used
 * to manage the global state of the computation and issue new work when dependencies are satisfied.
 *
 * RuleManagers access each IRule synchronously, so every IRule should not require a significant amount of
 * computation to finish processing an IRule.
 *
 * When a RuleManager is duplicated for an ExecutionPipeline, the IRule's within that RuleManager are shared,
 * thus ensuring that no race conditions occur when updating the state when multiple RuleManagers are attempting
 * to process the same IRule.
 *
 * Example Implementation
 * @code
 * class SimpleIRule : public htgs::IRule<Data1, Data2>
 * {
 *  public:
 *   SimpleIRule() { allocateStateArray(); }
 *   ~SimpleIRule() { deallocateStateArray(); }
 *   virtual bool isRuleTerminated(int pipelineId) { return false; }
 *   virtual void shutdownRule(int pipelineId) { }
 *   virtual void applyRule(std::shared_ptr<Data1> data, int pipelineId)
 *   {
 *		// Process state updates
 *		state[data.getRow()][data.getCol()] = data;
 *
 *		// Check for some dependency
 *		if (state[data.getRow()+1][data.getCol()])
 *		{
 *			addResult(new Data2(data, state[data.getRow()+1][data.getCol()]));
 *		}
 *   }
 *   virtual std::string getName() { return "SimpleIRule"; }
 *
 * private:
 *  std::shared_ptr<Data1> **state;
 * };
 * @endcode
 *
 * Example Usage:
 * @code
 * htgs::Bookkeeper<Data1> *bkTask = new htgs::Bookkeeper<Data1>();
 *
 * // SimpleIRule has input type 'Data1' and output type 'Data2'
 * SimpleIRule *simpleRule = new SimpleIRule();
 *
 * // SimpleTask has input type 'Data2'
 * SimpleTask *resultTask = new SimpleTask();
 *
 * htgs::TaskGraph<VoidData, VoidData> *taskGraph = new htgs::TaskGraph<VoidData, VoidData>();
 *
 * // Add the rule that connects the bkTask to the resultTask (can have multiple rules between a bkTask and a resultTask)
 * taskGraph->addRule(bkTask, resultTask, simpleRule);
 * @endcode
 *
 * @tparam T the input data type for the IRule, T must derive from IData.
 * @tparam U the output data type for the IRule, U must derive from IData.
 *
 */
template<class T, class U>
class IRule : public AnyIRule {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");
 public:

  /**
   * Creates an IRule
   */
  IRule() { }

  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////// VIRTUAL FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Destructor
   */
  virtual ~IRule() override { }

  virtual bool isRuleTerminated(size_t pipelineId) override { return false; }

  virtual void shutdownRule(size_t pipelineId) override { }

  virtual std::string getName() override {
    return "Unnamed IRule";
  }

  /**
  * Pure virtual function to process input data.
  * Use the addResult function to add values to the output edge.
  * @param data the input data
  * @param pipelineId the pipelineId
  *
  * @note To send data to the next edge use addResult
  */
  virtual void applyRule(std::shared_ptr<T> data, size_t pipelineId) = 0;

  /**
   * Initializes this IRule with a rule scheduler
   * @param ruleScheduler the rule scheduler that is used for this IRule
   */
  void initialize(RuleScheduler<T, U> *ruleScheduler)
  {
    this->ruleScheduler = ruleScheduler;
  }

  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * @internal
   * Applies the virtual rule function and processes output
   * @param data the input data
   * @param pipelineId the pipelineId
   * @note This function should only be called by the HTGS API
   */
  void applyRuleFunction(std::shared_ptr<T> data, size_t pipelineId) {
    applyRule(data, pipelineId);
  }

  /**
   * Adds a result value to the output
   * @param result the result value that is added
   */
  void addResult(std::shared_ptr<U> result) {
    this->ruleScheduler->addResult(result);
  }

  /**
   * Adds a result value to the output.
   * This will convert the pointer into a shared pointer.
   * @param result the result value that is added
   */
  void addResult(U *result) {
    this->ruleScheduler->addResult(std::shared_ptr<U>(result));
  }

  /**
   * Allocates a two dimensional state container using the input type of the IRule.
   * @param height the height of the state container
   * @param width the width of the state container
   * @return a pointer to the state container allocated
   */
  StateContainer<std::shared_ptr<T>> *allocStateContainer(size_t height, size_t width)
  {
    return new StateContainer<std::shared_ptr<T>>(height, width, nullptr);
  }

  /**
   * Allocates a two dimensional state container using the template argument.
   * @param height the height of the state container
   * @param width the width of the state container
   * @param defaultValue the value that represents no data or default value
   * @return a pointer to the state container allocated
   * @tparam V the state container type
   */
  template <class V>
  StateContainer<V> *allocStateContainer(size_t height, size_t width, V defaultValue)
  {
    return new StateContainer<V>(height, width, defaultValue);
  }

  /**
   * Allocates a one dimensional state container using the input type of the IRule.
   * @param size the size of the state container
   * @return a pointer to the state container allocated
   */
  StateContainer<std::shared_ptr<T>> *allocStateContainer(size_t size)
  {
    return new StateContainer<std::shared_ptr<T>>(size, 0, nullptr);
  }

  /**
   * Allocates a one dimensional state container using the input type of the IRule.
   * @param size the size of the state container
   * @param defaultValue the value that represents no data or default value
   * @return a pointer to the state container allocated
   * @tparam V the state container type
   */
  template <class V>
  StateContainer<V> *allocStateContainer(size_t size, V defaultValue)
  {
    return new StateContainer<V>(size, 0, defaultValue);
  }

 private:
  RuleScheduler<T, U> *ruleScheduler; //!< The rule scheduler that schedules this IRule

};


}


#endif //HTGS_IRULE_HPP
