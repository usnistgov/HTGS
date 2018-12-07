//
// Created by tjb3 on 11/30/18.
//

#ifndef HTGS_GRAPHOUTPUTTASKEDGE_HPP
#define HTGS_GRAPHOUTPUTTASKEDGE_HPP
#include <htgs/core/graph/edge/GraphEdge.hpp>

namespace htgs {
  template <class T, class U>
  class GraphTaskProducerEdge : public GraphEdge<U> {
   public:
    GraphTaskProducerEdge(ITask<T, U> *task, std::shared_ptr<Connector<U>> graphConnector) : GraphEdge<U>(graphConnector), task(task) {}

    ~GraphTaskProducerEdge() {}

    void applyEdge(AnyTaskGraphConf *graph) override {
      auto tMan = graph->getTaskManager(task);
      auto connector = this->getGraphConnector();

      tMan->setOutputConnector(connector);
      connector->incrementInputTaskCount();
    }

    GraphTaskProducerEdge<T, U> *copy(AnyTaskGraphConf *graph) override {
      return new GraphTaskProducerEdge<T, U>(graph->getCopy(task), std::static_pointer_cast<Connector<U>>(graph->getOutputConnector()));
    }

    void updateEdge(std::shared_ptr<Connector<U>> newConnector, AnyTaskGraphConf *graph) override {
      auto tMan = graph->getTaskManager(task);
      tMan->setOutputConnector(newConnector);
      this->setConnector(newConnector);
    }

    AnyTaskManager *getTaskManager(AnyTaskGraphConf *graph) {
      return graph->getTaskManager(task);
    }

   private:
    ITask<T, U> * task;
  };
}

#endif //HTGS_GRAPHOUTPUTTASKEDGE_HPP
