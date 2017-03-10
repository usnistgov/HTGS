
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by Hobbs on 2/15/2016.
//

#include "SimpleTask.h"
#include "../memory/SimpleReleaseRule.h"

SimpleTask::~SimpleTask() {

}

void SimpleTask::initialize() {
  initializeTime.stopAndIncrement();
  firstDataTime.start();
  this->pipelineId = this->getPipelineId();
}

void SimpleTask::shutdown() {
}

void SimpleTask::executeTask(std::shared_ptr<SimpleData> data) {
  if (data == nullptr) {
    executeTime.start();
  }
  else {
    executeTime.stopAndIncrement();
    executeTime.start();
  }
  if (!firstData) {
    firstDataTime.stopAndIncrement();
    firstData = true;
  }

  if (data != nullptr) {
    if (useMemoryManager) {
      if (this->hasMemoryEdge("test")) {
        std::shared_ptr<htgs::MemoryData<int *>> mem = this->getMemory<int *>("test", new SimpleReleaseRule());
        data->setMem(mem);
      }

      if (this->releaseMem) {
        this->releaseMemory(data->getMem());
      }
    }
    addResult(data);

  }

}

std::string SimpleTask::getName() {
  return "SimpleTask" + std::to_string(chainNum);
}

htgs::ITask<SimpleData, SimpleData> *SimpleTask::copy() {
  return new SimpleTask(this->getNumThreads(), this->chainNum, this->useMemoryManager, this->releaseMem);
}

bool SimpleTask::canTerminate(std::shared_ptr<htgs::AnyConnector> inputConnector) {
  if (inputConnector->isInputTerminated()) {
    totalTime.stopAndIncrement();
  }

  return inputConnector->isInputTerminated();
}

void SimpleTask::profile() {
  std::cout << "Time from construction to termination: " << totalTime.getDuration() << std::endl;
  std::cout << "Time from construction to initialize: " << initializeTime.getDuration() << std::endl;
  std::cout << "Time from initialize to first data: " << firstDataTime.getDuration() << std::endl;

  long execCount = executeTime.getCount();
  long long int execTime = executeTime.getDuration();

  std::cout << "Time for execute: " << executeTime.getDuration() << " for " << executeTime.getCount()
      << " items . . . items per second: " << (((double) execCount / (double) execTime) * 1000000000.0) << std::endl;
}
void SimpleTask::setReleaseMem(bool releaseMem) {
  this->releaseMem = releaseMem;
}
