
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file BookkeeperKey.hpp
 * @author Timothy Blattner
 * @date Nov 23, 2015
 *
 * @brief Provides functionality for copying a Bookkeeper entry in a TaskGraph
 */
#ifndef HTGS_BOOKKEEPERKEY_H
#define HTGS_BOOKKEEPERKEY_H

#include "../../task/BaseITask.hpp"
#include "../../rules/BaseBaseRuleManager.hpp"

namespace htgs {

/**
 * @class BookkeeperKey BookkeeperKey.hpp <htgs/core/graph/history/BookkeeperKey.hpp>
 * @brief Provides functionality for copying a Bookkeeper entry in a TaskGraph
 * @note This class should only be called by the HTGS API
 */
class BookkeeperKey {
 public:
  /**
   * Creates a bookkeeper key for graph copying.
   *
   * The bookkeeper key is used to reproduce how a Bookkeeper, its RuleManager, and the consumer output ITask
   * are added into a TaskGraph.
   *
   * @param bkTask
   * @param bk
   * @param ruleMan
   * @param outputTask
   */
  BookkeeperKey(BaseTaskScheduler *bkTask, BaseITask *bk, BaseBaseRuleManager *ruleMan, BaseTaskScheduler *outputTask)
      : bkTask(
      bkTask),
        bk(bk),
        ruleMan(ruleMan),
        outputTask(outputTask) { }

  /**
   * Destructor
   */
  ~BookkeeperKey() { }

  /**
   * Gets the BaseTaskScheduler that manages the Bookkeeper
   * @return the BaseTaskScheduler that manages the Bookkeeper
   */
  BaseTaskScheduler *getBkTask() const {
    return bkTask;
  }

  /**
   * Gets the Bookkeeper ITask
   * @return the Bookkeeper ITask
   */
  BaseITask *getBk() const {
    return bk;
  }

  /**
   * Gets the RuleManager
   * @return the RuleManager
   */
  BaseBaseRuleManager *getRuleMan() const {
    return ruleMan;
  }

  /**
   * Gets the output BaseTaskScheduler that will consume data.
   * @return the consumer BaseTaskScheduler
   */
  BaseTaskScheduler *getOutputTask() const {
    return outputTask;
  }

 private:
  BaseTaskScheduler *bkTask; //!< The TaskScheduler that manages the Bookkeeper ITask
  BaseITask *bk; //!< The ITask that is the Bookkeeper
  BaseBaseRuleManager *ruleMan; //!< The RuleManager that connects the Bookkeeper to a consumer ITask
  BaseTaskScheduler *outputTask; //!< The TaskScheduler that manages the consumer ITask
};
}


#endif //HTGS_BOOKKEEPERKEY_H
