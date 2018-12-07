//
// Created by tjb3 on 11/30/18.
//

#ifndef HTGS_GRAPHOUTPUTRULEEDGE_HPP
#define HTGS_GRAPHOUTPUTRULEEDGE_HPP

#include <htgs/api/Bookkeeper.hpp>
#include <htgs/core/graph/edge/GraphEdge.hpp>
#include <htgs/core/graph/AnyTaskGraphConf.hpp>


namespace htgs {
  template <class T, class U>
  class GraphRuleProducerEdge : public GraphEdge<U> {
   public:
    GraphRuleProducerEdge(Bookkeeper<T> *bookkeeper, std::shared_ptr<IRule<T, U>> rule, std::shared_ptr<Connector<U>> graphConnector) :
        GraphEdge<U>(graphConnector), bookkeeper(bookkeeper), rule(rule)
    {}


    ~GraphRuleProducerEdge() override {}

    void applyEdge(AnyTaskGraphConf *graph) override {
      graph->getTaskManager(bookkeeper);
      auto connector = this->getGraphConnector();


      RuleManager<T, U> *ruleManager = new RuleManager<T, U>(rule);
      ruleManager->setOutputConnector(connector);

      connector->incrementInputTaskCount();

      bookkeeper->addRuleManager(ruleManager);
    }

    void updateEdge(std::shared_ptr<Connector<T>> newConnector, AnyTaskGraphConf *graph) override {
      auto taskManager = graph->getTaskManager(bookkeeper);
      auto oldConnector = this->getGraphConnector();

      Bookkeeper<T> *tmBookkeeper = (Bookkeeper<T> *)taskManager->getTaskFunction();

      // Find the RuleManager that has the same out
      std::list<AnyRuleManagerInOnly<T> *> *ruleManagers = tmBookkeeper->getRuleManagers();

      for (AnyRuleManagerInOnly<T> *ruleManager : *ruleManagers)
      {
        if (ruleManager->getConnector() == oldConnector)
        {
          ruleManager->setOutputConnector(newConnector);
        }
      }

      this->setConnector(newConnector);
    }

    AnyTaskManager *getTaskManager(AnyTaskGraphConf *graph) {
      return graph->getTaskManager(bookkeeper);
    }

    GraphRuleProducerEdge<T, U> *copy(AnyTaskGraphConf *graph) override {
      return new GraphRuleProducerEdge<T, U>((Bookkeeper<T> *)graph->getCopy(bookkeeper), rule, std::static_pointer_cast<Connector<U>>(graph->getOutputConnector()));
    }

   private:
    Bookkeeper<T> *bookkeeper;
    std::shared_ptr<IRule<T, U>> rule;
  };
}

#endif //HTGS_GRAPHOUTPUTRULEEDGE_HPP
