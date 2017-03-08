//
// Created by tjb3 on 3/1/17.
//

#ifndef HTGS_PRODUCERCONSUMEREDGE_HPP
#define HTGS_PRODUCERCONSUMEREDGE_HPP

#include "EdgeDescriptor.hpp"
#include <htgs/api/ITask.hpp>
#include <htgs/core/graph/AnyTaskGraphConf.hpp>

namespace htgs {

template <class T, class U, class W>
class ProducerConsumerEdge : public EdgeDescriptor
{
 public:

  ProducerConsumerEdge(ITask<T, U> *producer, ITask<U, W> *consumer) : producer(producer), consumer(consumer) {}

  ~ProducerConsumerEdge() override {}

  void applyEdge(AnyTaskGraphConf *graph) override {
    // TODO: What if the connector is either the input or output of a graph . . .
    TaskManager<T, U> *producerTaskManager = graph->getTaskManager(producer);
    TaskManager<U, W> *consumerTaskManager = graph->getTaskManager(consumer);

    auto connector = consumerTaskManager->getInputConnector();

    if (connector == nullptr) {
      connector = std::shared_ptr<Connector<U>>(new Connector<U>());
    }

    connector->incrementInputTaskCount();

    consumerTaskManager->setInputConnector(connector);
    producerTaskManager->setOutputConnector(connector);
  }

  EdgeDescriptor *copy(AnyTaskGraphConf *graph) override {
    return new ProducerConsumerEdge(graph->getCopy(producer), graph->getCopy(consumer));
  }

 private:
  ITask<T, U> *producer;
  ITask<U, W> *consumer;

};
}
#endif //HTGS_PRODUCERCONSUMEREDGE_HPP
