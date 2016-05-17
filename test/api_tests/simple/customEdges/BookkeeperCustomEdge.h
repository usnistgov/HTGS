
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 2/9/16.
//

#ifndef HTGS_BOOKKEEPERCUSTOMEDGE_H
#define HTGS_BOOKKEEPERCUSTOMEDGE_H


#include <htgs/api/ICustomEdge.hpp>
#include <map>
#include <htgs/api/TaskGraph.hpp>

template<class T, class U, class V>
class BookkeeperCustomEdge: public htgs::ICustomEdge {

 public:
  BookkeeperCustomEdge(htgs::Bookkeeper<T> *bk, htgs::ITask<U, V> *consumer) {
    this->ruleManager = new htgs::RuleManager<T, U>();
    this->bk = bk;
    this->consumer = consumer;
  }

  BookkeeperCustomEdge(htgs::Bookkeeper<T> *bk, htgs::ITask<U, V> *consumer, htgs::RuleManager<T, U> *ruleManager) {
    this->ruleManager = ruleManager;
    this->bk = bk;
    this->consumer = consumer;
  }


  ~BookkeeperCustomEdge() { }

  virtual htgs::ICustomEdge *copy() override {
    return new BookkeeperCustomEdge<T, U, V>(bk, consumer, ruleManager->copy());
  }

  virtual void applyGraphConnection(htgs::BaseTaskScheduler *producer, htgs::BaseTaskScheduler *consumer,
                                    std::shared_ptr<htgs::BaseConnector> connector, int pipelineId, htgs::BaseTaskGraph *taskGraph) override {

    htgs::Bookkeeper<T> *bk = (htgs::Bookkeeper<T> *) producer->getTaskFunction();

    // Create edge from bk -> ruleMan -> consumer
    bk->addRuleManager(this->ruleManager);
    ruleManager->setOutputConnector(connector);
    consumer->setInputConnector(connector);
  }

  virtual htgs::BaseConnector *createConnector() override {
    return new htgs::Connector<U>();
  }

  virtual htgs::BaseTaskScheduler *createProducerTask() override {
    return htgs::TaskScheduler<T, htgs::VoidData>::createTask(this->bk);
  }

  virtual htgs::BaseTaskScheduler *createConsumerTask() override {
    return htgs::TaskScheduler<U, V>::createTask(this->consumer);
  }

  virtual htgs::BaseITask *getProducerITask() override {
    return bk;
  }

  virtual htgs::BaseITask *getConsumerITask() override {
    return consumer;
  }

  void addRule(htgs::IRule<T, U> *rule) {
    ruleManager->addRule(std::shared_ptr<htgs::IRule<T, U>>(rule));
  }

  virtual bool useConnector() override {
    return true;
  }
 private:
  htgs::RuleManager<T, U> *ruleManager;
  htgs::Bookkeeper<T> *bk;
  htgs::ITask<U, V> *consumer;
};


#endif //HTGS_BOOKKEEPERCUSTOMEDGE_H

