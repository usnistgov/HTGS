//
// Created by tjb3 on 2/22/17.
//

/**
 * @file RuleScheduler.hpp
 * @author Timothy Blattner
 * @date Feb 22, 2017
 *
 * @brief  Implements a RuleScheduler, which connects a Bookkeeper to another ITask using an IRule.
 * @details
 */
#ifndef HTGS_RULESCHEDULER_HPP
#define HTGS_RULESCHEDULER_HPP

#include "../../api/IRule.hpp"
#include "../graph/Connector.hpp"
#include "AnyRuleSchedulerInOnly.hpp"

namespace htgs {

template<class V, class W>
class IRule;

/**
 * @class RuleScheduler RuleScheduler.hpp <htgs/core/rules/RuleScheduler.hpp>
 * @brief Connects a Bookkeeper to another ITask using one or more IRule(s).
 * @details
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
template <class T, class U>
class RuleScheduler : public AnyRuleSchedulerInOnly<T> {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:

  /**
   * @internal
   * Creates a rule scheduler with a rule
   * @param rule the rule
   *
   * @note This function should only be called by the HTGS API
   */
  RuleScheduler(std::shared_ptr<htgs::IRule<T, U>> rule) : rule(rule), pipelineId(0) { }


  void executeTask(std::shared_ptr<T> data) override {

    std::unique_lock<std::mutex> lock(rule->getMutex());

    // Check if the rule is expecting data or not
    checkRuleTermination();

    DEBUG_VERBOSE("Rule: " << rule->getName() << " consuming data: " << data);
    std::list<std::shared_ptr<U>> *output = rule->applyRuleFunction(data, pipelineId);

    if (output != nullptr)
    {
      DEBUG_VERBOSE("Rule: " << rule->getName() << " producing data size: " << output->size());
      if (this->connector != nullptr)
        this->connector->produceData(output);
    }

    // Check if the rule is ready to be terminated after processing data (in case no more data
    checkRuleTermination();
  }

  RuleScheduler<T, U> *copy() override {
    return new RuleScheduler<T, U>(this->rule);
  }

  std::string getName() override {
    return this->rule->getName();
  }

  void debug() override {
    DEBUG(this->getName() << " output connector: " << this->connector);
  }

  std::shared_ptr<AnyConnector> getConnector() override {
    return this->connector;
  }


  void initialize(size_t pipelineId, size_t numPipelines) override
  {
    DEBUG_VERBOSE("Initialized " << this->getName() << " pipeline id: " << pipelineId);
    this->pipelineId = pipelineId;
  }

  void shutdown() override {
    DEBUG("Shutting down " << this->getName() << " pipeline id: " << pipelineId);

    // Check if the rule scheduler was terminated by it's rule
    if (!this->terminated) {

      // Close any active connections
      DEBUG("Waking up connector");
      this->connector->producerFinished();
      this->connector->wakeupConsumer();
    }

    // Shutdown the rule's pipeline ID
    rule->shutdownRule(this->pipelineId);
  }

  bool isTerminated() override
  {
    return terminated;
  }

  void setOutputConnector(std::shared_ptr<AnyConnector> connector) override {
    this->connector = std::dynamic_pointer_cast<Connector<U>>(connector);
    this->connector->incrementInputTaskCount();
    DEBUG_VERBOSE("Connector " << this->connector << " adding producer: " << this->getName() << " " << this <<
                               " to connector " << this->connector);
  }

 private:

  void checkRuleTermination()
  {
    if (!terminated) {
      // Check if the rule is ready to be terminated before and after processing data
      if (rule->isRuleTerminated(pipelineId)) {
        terminated = true;
        this->connector->producerFinished();
        if (this->connector->isInputTerminated()) {
          this->connector->wakeupConsumer();
        }
      }
    }
  }

  std::shared_ptr<IRule<T, U>> rule; //!< The rule associated with the RuleScheduler
  int pipelineId; //!< The execution pipeline id
  std::shared_ptr<htgs::Connector<U>> connector; //!< The connector for producing data from the rule
  bool terminated; //!< Whether this RuleScheduler is terminated or not

};

}

#endif //HTGS_RULESCHEDULER_HPP
