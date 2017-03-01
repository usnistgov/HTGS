//
// Created by tjb3 on 3/1/17.
//

#ifndef HTGS_PRODUCERCONSUMEREDGE_HPP
#define HTGS_PRODUCERCONSUMEREDGE_HPP

#include "EdgeDescriptor.hpp"
#include <htgs/api/ITask.hpp>
#include <htgs/core/graph/AnyTaskGraph.hpp>

namespace htgs {

template <class T, class U, class W>
class ProducerConsumerEdge : public EdgeDescriptor
{
 public:

  ProducerConsumerEdge(ITask<T, U> *producer, ITask<U, W> *consumer) : producer(producer), consumer(consumer) {}

  ~ProducerConsumerEdge() override {}

  void applyEdge(AnyTaskGraph *graph) override {
    TaskScheduler<T, U> *producerTaskScheduler = graph->getTaskScheduler(producer);
    TaskScheduler<U, W> *consumerTaskScheduler = graph->getTaskScheduler(consumer);

    auto connector = consumerTaskScheduler->getInputConnector();

    if (connector == nullptr)
    {
      connector = std::shared_ptr<Connector<U>>(new Connector<U>());
    }

    connector->incrementInputTaskCount();

    consumerTaskScheduler->setInputConnector(connector);
    producerTaskScheduler->setOutputConnector(connector);
  }

  EdgeDescriptor *copy() override {
    // TODO: Need to pass graph in and "Get" the copies . . . Current version will not work, but gives general idea . . .
    return new ProducerConsumerEdge(producer->copyITask(), consumer->copyITask());
  }

 private:
  ITask<T, U> *producer;
  ITask<U, W> *consumer;

};
}
#endif //HTGS_PRODUCERCONSUMEREDGE_HPP
