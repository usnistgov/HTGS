//
// Created by tjb3 on 3/31/17.
//

#ifndef HTGS_SIMPLECUDARELEASERULE_H
#define HTGS_SIMPLECUDARELEASERULE_H
#include <htgs/api/IMemoryReleaseRule.hpp>
class SimpleCudaReleaseRule : public htgs::IMemoryReleaseRule {
 public:

  void memoryUsed() override {

  }
  bool canReleaseMemory() override {
    return true;
  }
};
#endif //HTGS_SIMPLECUDARELEASERULE_H
