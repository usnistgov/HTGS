//
// Created by tjb3 on 11/30/18.
//

#ifndef HTGS_GRAPHTASKCONSUMEREDGE_HPP
#define HTGS_GRAPHTASKCONSUMEREDGE_HPP

#include <htgs/core/graph/edge/GraphEdge.hpp>


namespace htgs {
  template <class T, class U>
  class GraphTaskConsumerEdge : public GraphEdge<T>
  {
   public:
    GraphTaskConsumerEdge(ITask<T, U> *task, std::shared_ptr<Connector<T>> graphConnector) : GraphEdge<T>(graphConnector), task(task) {}

    ~GraphTaskConsumerEdge() {}

    void applyEdge(AnyTaskGraphConf *graph) override {
      auto tMan = graph->getTaskManager(task);
      auto connector = this->getGraphConnector();
      tMan->setInputConnector(connector);
    }

    GraphTaskConsumerEdge<T, U> *copy(AnyTaskGraphConf *graph) override {
      return new GraphTaskConsumerEdge<T, U>(graph->getCopy(task), std::static_pointer_cast<Connector<T>>(graph->getInputConnector()));
    }

    void updateEdge(std::shared_ptr<Connector<T>> newConnector, AnyTaskGraphConf *graph) override {
      auto taskManager = graph->getTaskManager(task);
      taskManager->setInputConnector(newConnector);
      this->setConnector(newConnector);
    }

    AnyTaskManager *getTaskManager(AnyTaskGraphConf *graph) {
      return graph->getTaskManager(task);
    }

   private:
    ITask<T, U> * task;
  };
}
#endif //HTGS_GRAPHTASKCONSUMEREDGE_HPP
