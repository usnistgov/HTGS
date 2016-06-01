
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file RuleManager.hpp
 * @author Timothy Blattner
 * @date Nov 18, 2015
 *
 * @brief  Implements a RuleManager, which connects a Bookkeeper to another ITask using one or more IRule(s).
 * @details
 */
#ifndef HTGS_RULEMANAGER_H
#define HTGS_RULEMANAGER_H


#include <memory>
#include <list>

#include "../../api/IRule.hpp"

#include "../../debug/debug_message.h"
#include "BaseRuleManager.hpp"

namespace htgs {
template<class V, class W>
class IRule;

/**
 * @class RuleManager RuleManager.hpp <htgs/core/rules/RuleManager.hpp>
 * @brief Connects a Bookkeeper to another ITask using one or more IRule(s).
 * @details
 *
 * When data is forwarded to the RuleManager from the Bookkeeper, the data is passed to each
 * IRule that is associated with the RuleManager. Each IRule is responsible for determining when/if
 * data is ready to be sent to the ITask the RuleManager is bound to.
 *
 * The input and output types of each IRule added to a RuleManager
 * must match the input and output types of the RuleManager.
 *
 * Example Usage:
 * @code
 * htgs::TaskGraph<htgs::VoidData, htgs::VoidData> *taskGraph = new htgs::TaskGraph<htgs::VoidData, htgs::VoidData>();
 * htgs::Bookkeeper<Data1> *bkTask = new htgs::Bookkeeper<Data1>();
 *
 * // DataRule implements IRule<Data1, Data2> to be compatible with ruleMan
 * DataRule *rule = new DataRule();
 *
 * // The output task for the RuleManager with input type Data2
 * Data2ProcessingTask *data2Task = new Data2ProcessingTask();
 *
 * // Creates edge between the Bookkeeper and the Data2ProcessingTask, where rule defines when data is sent
 * taskGraph->addRule(bkTask, data2Task, rule);
 * @endcode
 *
 * @tparam T the input data type for the RuleManager, T must derive from IData.
 * @tparam U the output data type for the RuleManager, U  must derive from IData.
 */
template<class T, class U>
class RuleManager: public BaseRuleManager<T> {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");

 public:
  /**
   * Constructs a RuleManager
   */
  RuleManager() : rules(new std::list<std::shared_ptr<IRule<T, U>>>()) {
    this->_isTerminated = false;
    this->connector = nullptr;
    this->ruleNames = "";
    this->pipelineId = 0;
    this->output = new std::list<std::shared_ptr<U>>();
  }

  /**
   * @internal
   * Creates a rule manager with rules and the names of the rules
   * @param rules the rules
   * @param ruleNames the names of each rule
   *
   * @note This function should only be called by the HTGS API
   */
  RuleManager(std::shared_ptr<std::list<std::shared_ptr<IRule<T, U>>>> rules, std::string ruleNames) {
    this->rules = rules;
    this->_isTerminated = false;
    this->connector = nullptr;
    this->ruleNames = ruleNames;
    this->pipelineId = 0;
    this->output = new std::list<std::shared_ptr<U>>();
  }

  /**
   * Destructor
   */
  ~RuleManager() {
    delete output;
    output = nullptr;
  }

  /**
   * Adds a rule to the RuleManager
   * @param rule the rule to add
   */
  void addRule(std::shared_ptr<IRule<T, U>> rule) {
    this->rules->push_back(rule);
    if (this->rules->size() == 1)
      this->ruleNames = this->ruleNames + rule->getName();
    else
      this->ruleNames = this->ruleNames + "," + rule->getName();
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
  void initialize(int pipelineId, int numPipelines)
  {
    DEBUG_VERBOSE("Initialized " << this->getName() << " pipeline id: " << pipelineId);
    this->pipelineId = pipelineId;
  }

  /**
   * @internal
   * Shuts down the RuleManager.
   * Will also shutdown each IRule associated with the RuleManager
   *
   * @note This function should only be called by the HTGS API
   */
  void shutdown() {
    DEBUG("Shuttind down " << this->getName() << " pipeline id: " << pipelineId);
    if (!this->_isTerminated) {
      DEBUG("Waking up connector");
      this->connector->producerFinished();
      this->connector->wakeupConsumer();
      this->_isTerminated = true;

      for (std::shared_ptr<IRule<T, U>> rule : *rules) {
        if (!rule->isRuleTerminated(this->pipelineId)) {
          rule->shutdownRule(this->pipelineId);
        }
      }
    }
  }

  /**
   * @internal
   * Creates a copy of the RuleManager.
   * The original and all copies share the same set of rules and access them synchronously.
   * @return the RuleManager copy
   *
   * @note This function should only be called by the HTGS API
   */
  RuleManager<T, U> *copy() {
    return new RuleManager<T, U>(this->rules, this->ruleNames);
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
  bool isTerminated(std::shared_ptr<BaseConnector> inputConnector) { return this->_isTerminated; }

  /**
   * @internal
   * Processes the input data, which is forwarded to each IRule synchronously.
   * The output for all IRules are aggregated and shipped to the output Connector.
   * @param data the input data
   *
   * @note This function should only be called by the HTGS API
   */
  void executeTask(std::shared_ptr<T> data) {
    bool isTerminated = false;

    output->clear();

    for (std::shared_ptr<IRule<T, U>> rule : *rules) {
      {
        std::unique_lock<std::mutex> lock(rule->getMutex());
        if (rule->isRuleTerminated(this->pipelineId)) {
          continue;
        }

        DEBUG_VERBOSE("Rule: " << rule->getName() << " consuming data: " << data);
        std::list<std::shared_ptr<U>> *ruleOut = rule->applyRuleFunction(data, this->pipelineId);

        if (ruleOut != nullptr) {
          DEBUG_VERBOSE("Rule: " << rule->getName() << " producing data size: " << ruleOut->size());
          for (std::shared_ptr<U> o : *ruleOut) {
            DEBUG_VERBOSE("Rule: " << rule->getName() << " producing data: " << o);
            output->push_back(o);
          }
        }

        if (rule->isRuleTerminated(this->pipelineId)) {
          isTerminated = true;
        }
      }
    }

    if (this->connector != nullptr)
      this->connector->produceData(output);

    if (isTerminated) {
      DEBUG_VERBOSE(this->getName() << " is terminating");
      this->_isTerminated = true;
      this->connector->producerFinished();
      if (this->connector->isInputTerminated()) {
        this->connector->wakeupConsumer();
      }

//                for (IRule<T, U> *rule : *rules)
//                {
//                    if (rule->isRuleTerminated(this->pipelineId))
//                    {
//                        rule->shutdownRule(this->pipelineId);
//                    }
//                }

    }

  }

  /**
   * @internal
   * Sets the output connector that the RuleManager is attached to
   * @param connector the output connector
   *
   * @note This function should only be called by the HTGS API
   */
  void setOutputConnector(std::shared_ptr<BaseConnector> connector) {
    this->connector = std::dynamic_pointer_cast<Connector<U>>(connector);
    this->connector->incrementInputTaskCount();
    DEBUG_VERBOSE("Connector " << this->connector << " adding producer: " << this->getName() << " " << this <<
        " to connector " << this->connector);
  }

  /**
   * @internal
   * Gets the output connector associated with the RuleManager
   * @return the output connector
   *
   * @note This function should only be called by the HTGS API
   */
  std::shared_ptr<BaseConnector> getConnector() {
    return this->connector;
  }

  /**
   * @internal
   * Sets the output connector that the RuleManager is attached to
   * @param connector the output connector
   *
   * @note This function should only be called by the HTGS API
   */
  void setOutputConnector(std::shared_ptr<Connector<U>> connector) {
    this->connector = connector;
    this->connector->incrementInputTaskCount();
    DEBUG_VERBOSE("Connector " << this->connector << " adding producer: " << this->getName() << " " << this <<
        " to connector " << this->connector);
  }

  /**
   * Gets the name of the RuleManager and the names of all IRules that it manages.
   * @return the name
   */
  std::string getName() {
    return std::string(
        "RuleManager (" + std::to_string(this->pipelineId) + "): " + std::to_string(this->rules->size()) +
            " rule(s) {" + this->ruleNames + "}");
  }

  /**
   * Gets the rule names associated with the RuleManager
   */
  std::string getRuleNames() {
    return this->ruleNames;
  }

  /**
   * Provides debug output
   * @note \#define DEBUG_FLAG to enable debugging
   */
  void debug() {
    DEBUG(this->getName() << " output connector: " << this->connector);
  }


 private:
  std::shared_ptr<std::list<std::shared_ptr<IRule<T, U>>>> rules; //!< The list of rules associated with the RuleManager
  int pipelineId; //!< The execution pipeline id
  bool _isTerminated; //!< Whether the RuleManager is terminated or not
  std::shared_ptr<Connector<U>> connector; //!< The connector for producing data from the IRules
  std::string ruleNames; //!< A concatenated list of all the rules managed by the RuleManager
  std::list<std::shared_ptr<U>> *output; //!< The list of output data produced by the rules
};
}
#endif //HTGS_RULEMANAGER_H
