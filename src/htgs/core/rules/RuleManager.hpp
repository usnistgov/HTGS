//
// Created by tjb3 on 2/22/17.
//

/**
 * @file RuleManager.hpp
 * @author Timothy Blattner
 * @date Feb 22, 2017
 *
 * @brief  Implements a RuleManager, which connects a Bookkeeper to another ITask using an IRule.
 * @details
 */
#ifndef HTGS_RULEMANAGER_HPP
#define HTGS_RULEMANAGER_HPP

#include "../../api/IRule.hpp"
#include "../graph/Connector.hpp"
#include "AnyRuleManagerInOnly.hpp"

namespace htgs {

template<class V, class W>
class IRule;

/**
 * @class RuleManager RuleManager.hpp <htgs/core/rules/RuleManager.hpp>
 * @brief Connects a Bookkeeper to another ITask using one or more IRule(s).
 * @details
 *
 * When data is forwarded to the RuleManager from the Bookkeeper, the data is passed to an
 * IRule that is associated with the RuleManager. Each IRule is responsible for determining when/if
 * data is ready to be sent to the ITask the RuleManager is bound to.
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
template<class T, class U>
class RuleManager : public AnyRuleManagerInOnly<T> {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:

  /**
   * Creates a rule manager with a rule
   * @param rule the rule
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  RuleManager(std::shared_ptr<htgs::IRule<T, U>> rule)
      : rule(rule), pipelineId(0), numPipelines(1), terminated(false) {}

  /**
   * Destructor
   */
  virtual ~RuleManager() override {}

  void executeTask(std::shared_ptr<T> data) override {

    this->rule->getMutex().lock();

    // Check if the rule is expecting data or not
    checkRuleTermination();

    DEBUG_VERBOSE("Rule: " << rule->getName() << " consuming data: " << data);
    auto result = rule->applyRuleFunction(data, pipelineId);

    if (result != nullptr && result->size() > 0) {
      if (this->connector != nullptr) {
        this->connector->produceData(result);
      }
    }


    // Check if the rule is ready to be terminated after processing data (in case no more data
    checkRuleTermination();

    this->rule->getMutex().unlock();
  }

  RuleManager<T, U> *copy() override {
    return new RuleManager<T, U>(this->rule);
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

  void initialize(size_t pipelineId, size_t numPipelines, std::string address) override {
    DEBUG_VERBOSE("Initialized " << this->getName() << " pipeline id: " << pipelineId);
    this->pipelineId = pipelineId;
    this->numPipelines = numPipelines;
    this->address = address;
  }

  void shutdown() override {
    DEBUG("Shutting down " << this->getName() << " pipeline id: " << pipelineId);

    // Check if the rule manager was terminated by it's rule
    if (!this->terminated) {

      // Close any active connections
      DEBUG("Waking up connector");
      this->connector->producerFinished();
      this->connector->wakeupConsumer();
    }

    // Shutdown the rule's pipeline ID
    rule->shutdownRule(this->pipelineId);
  }

  bool isTerminated() override {
    return terminated;
  }

  void setOutputConnector(std::shared_ptr<AnyConnector> connector) override {
    this->connector = std::static_pointer_cast<Connector<U>>(connector);
    DEBUG_VERBOSE("Connector " << this->connector << " adding producer: " << this->getName() << " " << this <<
                               " to connector " << this->connector);
  }

 private:

  //! @cond Doxygen_Suppress
  void checkRuleTermination() {
    if (!terminated) {
      // Check if the rule is ready to be terminated before and after processing data
      if (rule->canTerminateRule(pipelineId)) {
        terminated = true;
        this->connector->producerFinished();
        if (this->connector->isInputTerminated()) {
          this->connector->wakeupConsumer();
        }
      }
    }
  }
  //! @endcond


  std::shared_ptr<IRule<T, U>> rule; //!< The rule associated with the RuleManager
  size_t pipelineId; //!< The execution pipeline id
  size_t numPipelines; //!< The number of execution pipelines
  std::string address; //!< The address for the rule manager
  std::shared_ptr<htgs::Connector<U>> connector; //!< The connector for producing data from the rule
  volatile bool terminated; //!< Whether this RuleManager is terminated or not

};

}

#endif //HTGS_RULEMANAGER_HPP
