
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file Bookkeeper.hpp
 * @author Timothy Blattner
 * @date Nov 11, 2015
 *
 * @brief Implements the Bookkeeper class
 *
 */

#ifndef HTGS_BOOKKEEPER_H
#define HTGS_BOOKKEEPER_H

#include "ITask.hpp"
#include "VoidData.hpp"
#include "../core/rules/RuleManager.hpp"

namespace htgs {

template<class U>
class BaseRuleManager;


/**
 * @class Bookkeeper Bookkeeper.hpp <htgs/api/Bookkeeper.hpp>
 * @brief Special task used to manage rules.
 * @details The Bookkeeper manages one or more rules, which customizes the flow of data within a task graph.
 * These rules specify a scheduling over input data. When data enters the Bookkeeper, each rule executes on that data
 * synchronously.
 *
 * To use the bookkeeper, create a RuleManager and add all desired rules to that RuleManager. Every RuleManager
 * added represents a different edge to another task connected to a Bookkeeper.
 *
 * Example usage:
 * @code
 * htgs::TaskGraph<htgs::VoidData, htgs::VoidData> *tGraph = new htgs::TaskGraph<VoidData, VoidData>();
 *
 * htgs::Bookkeeper<MatrixData> *bk = new htgs::BookKeeper<MatrixData>();
 *
 * // ScalMultiplyTask has inputType: MultiMatrixData
 * ScalMultiplyTask *scalMul = new ScalMultiplyTask();
 *
 * // Submits data from bk to scalMul using rule MatrixRule. Only submits data if MatrixRule adds results
 * tGraph->addRule(bk, scalMul, new MatrixRule());
 * @endcode
 *
 * @tparam T the input data type for the Bookkeeper ITask, T must derive from IData.
 *
 */
template<class T>
class Bookkeeper: public ITask<T, VoidData> {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");

 public:

  /**
   * Constructs a bookkeeper
   */
  Bookkeeper() {
    this->ruleManagers = new std::list<BaseRuleManager<T> *>();
  }

  /**
   * Destructor destroys RuleManager memory
   */
  ~Bookkeeper() {
    for (BaseRuleManager<T> *ruleManager : *ruleManagers) {
      delete ruleManager;
      ruleManager = nullptr;
    }
    delete ruleManagers;
    ruleManagers = nullptr;
  }

  /**
   * Adds rule manager to this bookkeeper
   * @param ruleManager the rule manager
   * @note This function should only be called by the HTGS API - use addRuleManager(RuleManager<T, U> *ruleManager)
   */
  void addRuleManager(BaseBaseRuleManager *ruleManager) {
    BaseRuleManager<T> *baseRuleMan = (BaseRuleManager<T> *) ruleManager;

    DEBUG_VERBOSE(this << "----" << this->getName() << " adding rule manager " << baseRuleMan->getName());

    ruleManagers->push_back(baseRuleMan);
    ruleManagerInfo = ruleManagerInfo + " " + baseRuleMan->getName();
  }

  /**
   * Adds rule manager to this bookkeeper
   * @param ruleManager the rule manager
   * @tparam U the output type for the rulemanager (output edge type)
   */
  template<class U>
  void addRuleManager(RuleManager<T, U> *ruleManager) {
    DEBUG_VERBOSE(this << "----" << this->getName() << " adding rule manager " << ruleManager->getName());

    ruleManagers->push_back(ruleManager);

    ruleManagerInfo = ruleManagerInfo + " " + ruleManager->getName();
  }

  /**
   * Executes the bookkeeper on data.
   * The bookkeeper forwards the data to each rule manager.
   * @param data the data
   *
   * @note This function should only be called by the HTGS API
   */
  void executeTask(std::shared_ptr<T> data) {
    for (BaseRuleManager<T> *ruleManager : *ruleManagers) {
//      DEBUG_VERBOSE(this->getName() << " executing " + ruleManager->getName());
      ruleManager->executeTask(data);
    }
  }

  /**
   * Gets the name of this bookkeeper and all rule managers managed by it.
   * @return the name of this bookkeeper
   */
  std::string getName() {
    return "Bookkeeper -- " + std::to_string(this->ruleManagers->size()) + " rule manager(s): " +
        this->ruleManagerInfo;
  }

  /**
   * Provides debug output for the rule manager.
   * @note \#define DEBUG_FLAG to enable debugging
   */
  void debug() {
    DEBUG(this->getName() << " Details:");
    for (BaseRuleManager<T> *ruleManager : *ruleManagers) {
      DEBUG("Executing rule manager: " << ruleManager->getName());
      ruleManager->debug();
    }

  }

  /**
   * Initializes the bookkeeper and all RuleManagers.
   * @param pipelineId the pipeline id
   * @param numPipelines the number of pipelines
   * bookkeepers in an execution pipeline
   *
   * @note This function should only be called by the HTGS API
   */
  void initialize(int pipelineId, int numPipelines) {
    for (BaseRuleManager<T> *ruleManager : *ruleManagers) {
      ruleManager->initialize(pipelineId, numPipelines);
    }
  }

  /**
   * Shuts down this bookkeeper and all of it's RuleManagers
   *
   * @note This function should only be called by the HTGS API
   */
  void shutdown() {

    DEBUG("Shutting down " << this->getName());
    for (BaseRuleManager<T> *ruleManager : *ruleManagers) {
      ruleManager->shutdown();
    }
  }

  /**
   * Creates a shallow copy of this bookkeeper
   * @return a shallow copy (No rule managers) of this bookkeeper
   *
   * @note This function should only be called by the HTGS API
   */
  Bookkeeper<T> *copy() { return new Bookkeeper<T>(); }

  /**
   * Checks if this bookkeeper is ready to be terminated
   * @param inputConnector the input connector that sends data to this bookkeeper
   * @return whether the bookkeeper can be terminated, uses inputConnector->isInputTerminated()
   * @retval TRUE if it is ready to be terminated
   * @retval FALSE if it is not ready to be terminated
   *
   * @note This function should only be called by the HTGS API
   */
  bool isTerminated(std::shared_ptr<BaseConnector> inputConnector) {
    return inputConnector->isInputTerminated();
  }

  /**
   * Generates the dot notation for the bookkeeper
   */
  std::string genDot(std::string idStr) {
    std::ostringstream oss;
    for (BaseRuleManager<T> *ruleMan : *ruleManagers) {
      std::ostringstream ruleManOss;
      ruleManOss << ruleMan->getConnector();

      std::string ruleManStr = ruleManOss.str();
      ruleManStr.erase(0, 1);
      oss << idStr << " -> " << ruleManStr << "[label=\"" << ruleMan->getRuleNames() << "\"];" << std::endl;
    }

    oss << idStr + "[label=\"Bookkeeper\"];\n";

    return oss.str();
  }


 private:
  std::list<BaseRuleManager<T> *> *ruleManagers; //!< The list of ruleManagers (one per consumer)
  std::string ruleManagerInfo; //!< A string representation of all rule managers
};
}


#endif //HTGS_BOOKKEEPER_H
