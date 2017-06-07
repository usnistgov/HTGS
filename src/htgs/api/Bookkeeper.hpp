
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

#ifndef HTGS_BOOKKEEPER_HPP
#define HTGS_BOOKKEEPER_HPP

#include "ITask.hpp"
#include "VoidData.hpp"
#include "htgs/core/rules/AnyRuleManagerInOnly.hpp"
#include "htgs/core/rules/RuleManager.hpp"

namespace htgs {

/**
 * @class Bookkeeper Bookkeeper.hpp <htgs/api/Bookkeeper.hpp>
 * @brief Special task used to manage rules.
 * @details The Bookkeeper manages one or more rules, which customizes the flow of data within a task graph.
 * These rules specify a scheduling over input data. When data enters the Bookkeeper, each rule executes on that data
 * synchronously.
 *
 * To use the bookkeeper, create an IRule and an ITask. The IRule represents the rule that produces data
 * to the ITask. An edge is created with a RuleManager acting as the intermediary between the IRule and the ITask.
 * These RuleManagers are created automatically when adding the bookkeeper using the TaskGraphConf::addRuleEdge function.
 * Each RuleManager represents a different edge to another ITask connected to a Bookkeeper and have one
 * IRule connecting the ITask.
 *
 * If you wish to share an IRule with multiple Bookkeepers, you must wrap the IRule into a shared_ptr prior to
 * calling the TaskGraphConf::addRuleEdge function.
 *
 * 
 *
 * Example usage:
 * @code
 * htgs::TaskGraphConf<htgs::VoidData, htgs::VoidData> *tGraph = new htgs::TaskGraphConf<VoidData, VoidData>();
 *
 * htgs::Bookkeeper<MatrixData> *bk = new htgs::BookKeeper<MatrixData>();
 *
 * // ScalMultiplyTask has inputType: MultiMatrixData
 * ScalMultiplyTask *scalMul = new ScalMultiplyTask();
 *
 * // Submits data from bk to scalMul using rule MatrixRule. Only submits data if MatrixRule adds results
 * tGraph->addRuleEdge(bk, scalMul, new MatrixRule());
 * @endcode
 *
 * @tparam T the input data type for the Bookkeeper ITask, T must derive from IData.
 *
 */
template<class T>
class Bookkeeper : public ITask<T, VoidData> {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");

 public:

  /**
   * Constructs a bookkeeper
   */
  Bookkeeper() {
    this->ruleManagers = new std::list<AnyRuleManagerInOnly<T> *>();
  }

  /**
   * Destructor destroys RuleManager memory
   */
  virtual ~Bookkeeper() override {
    for (AnyRuleManagerInOnly<T> *ruleManager : *ruleManagers) {
      delete ruleManager;
      ruleManager = nullptr;
    }
    delete ruleManagers;
    ruleManagers = nullptr;
  }

  /**
   * Adds rule manager to this bookkeeper
   * @param ruleManager the rule manager
   * @note This function should only be called by the HTGS API - users of the HTGS API should instead use addRuleManager(RuleManager<T, U> *ruleManager)
   */
  void addRuleManager(AnyRuleManager *ruleManager) {
    AnyRuleManagerInOnly<T> *baseRuleMan = (AnyRuleManagerInOnly<T> *) ruleManager;

    DEBUG_VERBOSE(this << "----" << this->getName() << " adding rule manager " << baseRuleMan->getName());

    ruleManagers->push_back(baseRuleMan);
    ruleManagerInfo = ruleManagerInfo + " " + baseRuleMan->getName();
  }

  /**
   * Adds rule manager to this bookkeeper
   * @param ruleManager the rule manager
   * @tparam U the output type for the rule manager (output edge type)
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
   * @note This function should only be called by the HTGS API
   */
  void executeTask(std::shared_ptr<T> data) override {

    for (AnyRuleManagerInOnly<T> *ruleManager : *ruleManagers) {
//      DEBUG_VERBOSE(this->getName() << " executing " + ruleManager->getName());
      ruleManager->executeTask(data);
    }
  }

  /**
   * Gets the name of this bookkeeper and all rule managers it controls.
   * @return the name of this bookkeeper
   */
  std::string getName() override {
    return "Bookkeeper -- " + std::to_string(this->ruleManagers->size()) + " rule(s): " +
        this->ruleManagerInfo;
  }

  /**
   * Gets just the name "Bookkeeper" for the dot label
   * @return the name of the bookkeeper
   */
  virtual std::string getDotLabelName() override {
    return "Bookkeeper";
  }

  /**
   * Provides debug output for the rule manager.
   * @note \#define DEBUG_FLAG to enable debugging
   */
  void debug() override {
    DEBUG(this->getName() << " Details:");
    for (AnyRuleManagerInOnly<T> *ruleManager : *ruleManagers) {
      DEBUG("Executing rule manager: " << ruleManager->getName());
      ruleManager->debug();
    }

  }

  /**
   * Initializes the bookkeeper and all RuleManagers.
   * @note This function should only be called by the HTGS API
   */
  void initialize() override {
    for (AnyRuleManagerInOnly<T> *ruleManager : *ruleManagers) {
      ruleManager->initialize(this->getPipelineId(), this->getNumPipelines(), this->getAddress());
    }
  }

  /**
   * Shuts down this bookkeeper and all of it's RuleManagers
   * @note This function should only be called by the HTGS API
   */
  void shutdown() {

    DEBUG("Shutting down " << this->getName());
    for (AnyRuleManagerInOnly<T> *ruleManager : *ruleManagers) {
      ruleManager->shutdown();
    }
  }

  /**
   * Creates a shallow copy of this bookkeeper
   * @return a shallow copy (No rule managers) of this bookkeeper
   * @note This function should only be called by the HTGS API
   */
  Bookkeeper<T> *copy() { return new Bookkeeper<T>(); }

  /**
   * Generates the dot notation for the bookkeeper
   */
  std::string genDot(int flags, std::string idStr) {
    std::ostringstream oss;
    for (AnyRuleManagerInOnly<T> *ruleMan : *ruleManagers) {
      std::ostringstream ruleManOss;
      ruleManOss << ruleMan->getConnector();

      std::string ruleManStr = ruleManOss.str();
      ruleManStr.erase(0, 1);
      oss << idStr << " -> " << ruleManStr << "[label=\"" << ruleMan->getName() << "\"];" << std::endl;
    }
    std::string inOutLabel = (((flags & DOTGEN_FLAG_SHOW_IN_OUT_TYPES) != 0) ? ("\nin: " + this->inTypeName()) : "");

    oss << idStr + "[label=\"Bookkeeper" + inOutLabel + "\"];\n";

    return oss.str();
  }

//#ifdef PROFILE
// TODO: Remove
//  std::string getDotProfile(int flags,
//                                    std::unordered_map<std::string, double> *mmap, double val,
//                                    std::string desc, std::unordered_map<std::string, std::string> *colorMap)
//  {
//    std::string inOutLabel = (((flags & DOTGEN_FLAG_SHOW_IN_OUT_TYPES) != 0) ? ("\nin: "+ this->inTypeName()) : "");
//    return this->getDotId() + "[label=\"" + "Bookkeeper" + inOutLabel + "\n" + desc + "\n" + std::to_string(val) + "\",shape=box,style=filled,fillcolor=white,penwidth=5,color=\""+colorMap->at(this->getNameWithPipID()) + "\",width=.2,height=.2];\n";
//  }
//#endif

 private:
  std::list<AnyRuleManagerInOnly<T> *> *ruleManagers; //!< The list of ruleManagers (one per consumer)
  std::string ruleManagerInfo; //!< A string representation of all rule managers
};
}

#endif //HTGS_BOOKKEEPER_HPP
