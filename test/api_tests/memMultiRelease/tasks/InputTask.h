
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 4/19/16.
//

#ifndef HTGS_INPUTTASK_H
#define HTGS_INPUTTASK_H
#include <htgs/api/ITask.hpp>

#include "../data/InputData.h"
#include "../data/ProcessedData.h"
#include "../memory/SimpleReleaseRule.h"

class InputTask : public htgs::ITask<InputData, ProcessedData> {
 public:
  InputTask(int numReleasers, bool graphReleaser, htgs::MMType memType) : numReleasers(numReleasers), graphReleaser(graphReleaser), memoryManagerType(memType) {}

  virtual void executeTask(std::shared_ptr<InputData> data) override {
    for (int i = 0; i < numReleasers; i++) {

      std::shared_ptr<htgs::MemoryData<int *>> mem;
      std::shared_ptr<htgs::MemoryData<int *>> mem2;
      if (graphReleaser)
      {
        if (this->hasMemGetter("mem2"))
        {
          mem2 = this->memGet<int *>("mem2", new SimpleReleaseRule());
        }
        else {
          switch(memoryManagerType)
          {
            case htgs::MMType::Static:
              mem2 = this->memGet<int *>("mem", new SimpleReleaseRule());
              break;
            case htgs::MMType::Dynamic:
              mem2 = this->memGet<int *>("mem", new SimpleReleaseRule(), 1);
              break;
            case htgs::MMType::UserManaged:
              this->allocUserManagedMemory("mem");
              break;
          }

        }
      }

      switch(memoryManagerType)
      {
        case htgs::MMType::Static:
          mem = this->memGet<int *>("mem", new SimpleReleaseRule());
          break;
        case htgs::MMType::Dynamic:
          mem = this->memGet<int *>("mem", new SimpleReleaseRule(), 1);
          break;
        case htgs::MMType::UserManaged:
          this->allocUserManagedMemory("mem");
          break;
      }

      addResult(new ProcessedData(data, i, mem, mem2));
    }
  }

  virtual std::string getName() override {
    return "InputTask";
  }
  virtual htgs::ITask<InputData, ProcessedData> *copy() override {
    return new InputTask(numReleasers, graphReleaser, memoryManagerType);
  }
  virtual bool isTerminated(std::shared_ptr<htgs::BaseConnector> inputConnector) override {
    return inputConnector->isInputTerminated();
  }

 private:
  int numReleasers;
  bool graphReleaser;
  htgs::MMType memoryManagerType;
};


#endif //HTGS_INPUTTASK_H
