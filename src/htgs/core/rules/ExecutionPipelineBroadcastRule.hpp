// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.
/**
 * @file ExecutionPipelineBroadcastRule.hpp
 * @author Timothy Blattner
 * @date March 3, 2017
 * @brief Implements the default execution pipeline rule that broadcasts data to all pipelines.
 */
#ifndef HTGS_EXECUTIONPIPELINEBROADCASTRULE_HPP
#define HTGS_EXECUTIONPIPELINEBROADCASTRULE_HPP
#include <htgs/api/IRule.hpp>
namespace htgs {

/**
 * @class ExecutionPipelineBroadcastRule ExecutionPipelineBroadcastRule.hpp <htgs/core/rules/ExecutionPipelineBroadcastRule.hpp>
 * @brief the default execution pipeline rule that is used if no other rule is specified for an execution pipeline.
 *
 * When constructing an ExecutionPipeline task, rules must be added to the task to indicate how data is distributed
 * within pipelines. If no rule is specified, then this rule is added automatically during initialization of the task.
 *
 *
 * @tparam T the input/output type for the rule, must be of type IData.
 */
template <class T>
class ExecutionPipelineBroadcastRule : public IRule<T, T>
{
 public:
  ~ExecutionPipelineBroadcastRule() override {}
  bool canTerminateRule(size_t pipelineId) override { return false; }
  void shutdownRule(size_t pipelineId) override { }
  std::string getName() override {
    return "DefaultBroadcastRule";
  }
  void applyRule(std::shared_ptr<T> data, size_t pipelineId) override {
    this->addResult(data);
  }
};

}
#endif //HTGS_EXECUTIONPIPELINEBROADCASTRULE_HPP
