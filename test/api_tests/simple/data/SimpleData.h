
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by Hobbs on 2/15/2016.
//

#ifndef HTGS_SIMPLEDATA_H
#define HTGS_SIMPLEDATA_H


#include <htgs/api/IData.hpp>
#include <htgs/api/MemoryData.hpp>

class SimpleData: public htgs::IData {

 public:
  SimpleData(int value, int pipelineId) : value(value), pipelineId(pipelineId) {}

  int getPipelineId()
  {
    return pipelineId;
  }

  int getValue()
  {
    return value;
  }

  std::shared_ptr<htgs::MemoryData<int *>> getMem() { return mem; }

  void setMem(std::shared_ptr<htgs::MemoryData<int *>> mem) { this->mem = mem;}


 private:
  int value;
  int pipelineId;
  std::shared_ptr<htgs::MemoryData<int *>> mem;
};


#endif //HTGS_SIMPLEDATA_H
