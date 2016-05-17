
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 2/10/16.
//

#include <htgs/api/ICustomEdge.hpp>
#include <htgs/api/IMemoryAllocator.hpp>
#include <htgs/core/memory/MemoryManager.hpp>
#include <limits.h>
#include <htgs/core/graph/BaseTaskGraph.hpp>

#ifndef HTGS_MEMORYMANAGERCUSTOMEDGE_H
#define HTGS_MEMORYMANAGERCUSTOMEDGE_H

template<class T>
class MemoryManagerCustomEdge: public htgs::ICustomEdge {
 public:
  MemoryManagerCustomEdge(std::string name, htgs::BaseITask *memGetter, htgs::BaseITask *memReleaser,
                          htgs::IMemoryAllocator<T> *allocator, int memoryPoolSize, htgs::MMType type)
  {
    std::shared_ptr<htgs::IMemoryAllocator<T>> alloc(allocator);
    this->memManager = new htgs::MemoryManager<T>(name, memoryPoolSize, alloc, type);
    this->name = name;
    this->memGetter = memGetter;
    this->memReleaser = memReleaser;
  }
  MemoryManagerCustomEdge(std::string name, htgs::BaseITask *memGetter, htgs::BaseITask *memReleaser,
                          htgs::MemoryManager<T> *memoryManager)
  {
    this->memManager = memoryManager;
    this->name = name;
    this->memGetter = memGetter;
    this->memReleaser = memReleaser;
  }


  virtual ~MemoryManagerCustomEdge() {

  }

  virtual htgs::ICustomEdge *copy() override {
    return new MemoryManagerCustomEdge(this->name, this->memGetter, this->memReleaser, this->memManager->copy());
  }

  virtual void applyGraphConnection(htgs::BaseTaskScheduler *producer, htgs::BaseTaskScheduler *consumer,
                                    std::shared_ptr<htgs::BaseConnector> connector, int pipelineId, htgs::BaseTaskGraph *taskGraph) override {

    htgs::TaskScheduler<htgs::MemoryData<T>, htgs::MemoryData<T>>
        *memTask = htgs::TaskScheduler<htgs::MemoryData<T>, htgs::MemoryData<T>>::createTask(memManager);

    memTask->setPipelineId(pipelineId);


    std::shared_ptr<htgs::Connector<htgs::MemoryData<T>>> inputConnector = std::shared_ptr<htgs::Connector<htgs::MemoryData<T>>>(new htgs::Connector<htgs::MemoryData<T>>());
    std::shared_ptr<htgs::Connector<htgs::MemoryData<T>>> outputConnector = std::shared_ptr<htgs::Connector<htgs::MemoryData<T>>>(new htgs::Connector<htgs::MemoryData<T>>());

    memTask->setInputConnector(inputConnector);
    memTask->setOutputConnector(outputConnector);
    outputConnector->incrementInputTaskCount();

    producer->getTaskFunction()->attachMemReleaser(name, memTask->getInputBaseConnector(), memManager->getType(), false);
    consumer->getTaskFunction()->attachMemGetter(name, memTask->getOutputBaseConnector(), memManager->getType());

    inputConnector->incrementInputTaskCount();

    taskGraph->getVertices()->push_back(memTask);
  }

  virtual htgs::BaseConnector *createConnector() override {
    return nullptr;
  }

  virtual htgs::BaseTaskScheduler *createProducerTask() override {
    return nullptr;
  }

  virtual htgs::BaseTaskScheduler *createConsumerTask() override {
    return nullptr;
  }

  virtual htgs::BaseITask *getProducerITask() override {
    return memReleaser;
  }

  virtual htgs::BaseITask *getConsumerITask() override {
    return memGetter;
  }

  virtual bool useConnector() override {
    return false;
  }

 private:
  std::string name;
  htgs::BaseITask *memGetter;
  htgs::BaseITask *memReleaser;
  htgs::MemoryManager<T> *memManager;

};
#endif //HTGS_MEMORYMANAGERCUSTOMEDGE_H

