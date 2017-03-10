//
// Created by tjb3 on 3/3/17.
//

#ifndef HTGS_EXECUTIONPIPELINEBROADCASTRULE_HPP
#define HTGS_EXECUTIONPIPELINEBROADCASTRULE_HPP
#include <htgs/api/IRule.hpp>
namespace htgs {
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
