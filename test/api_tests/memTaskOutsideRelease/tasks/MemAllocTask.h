
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 5/17/16.
//

#include "../data/MultiMemData.h"
#include "../../memMultiRelease/memory/SimpleReleaseRule.h"
#ifndef HTGS_MEMALLOCTASK_H
#define HTGS_MEMALLOCTASK_H

class MemAllocTask : public htgs::ITask<MultiMemData, MultiMemData>
{
 public:
  MemAllocTask(int taskNum) : taskNum(taskNum) {}

  virtual ~MemAllocTask() {
  }

  virtual void executeTask(std::shared_ptr<MultiMemData> data) {
    auto mem = this->memGet<int *>("memEdge" + std::to_string(taskNum), new SimpleReleaseRule());
    data->setMem(taskNum, mem);
    addResult(data);
  }

  virtual std::string getName() {
    return "MemAllocTask" + std::to_string(taskNum);
  }
  virtual htgs::ITask<MultiMemData, MultiMemData> *copy() {
    return new MemAllocTask(taskNum);
  }
  virtual bool isTerminated(std::shared_ptr<htgs::BaseConnector> inputConnector) {
    return inputConnector->isInputTerminated();
  }

 private:
  int taskNum;
};

#endif //HTGS_MEMALLOCTASK_H
