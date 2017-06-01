// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file RuleEdge.hpp
 * @author Timothy Blattner
 * @date March 1, 2017
 *
 * @brief Implements the rule edge, which is an edge descriptor.
 *
 */

#ifndef HTGS_RULEEDGE_HPP
#define HTGS_RULEEDGE_HPP

#include "EdgeDescriptor.hpp"
#include <htgs/api/ITask.hpp>
#include <htgs/core/graph/AnyTaskGraphConf.hpp>
#include <htgs/api/Bookkeeper.hpp>

namespace htgs {

/**
 * @class RuleEdge RuleEdge.hpp <htgs/core/graph/edge/RuleEdge.hpp>
 * @brief Implements the rule edge that is added to the graph.
 *
 * This edge connects a bookkeeper with some consumer task via a rule. The rule is used to decide
 * when to produce data, often based on the state of the computation.
 *
 * When applying the edge, the bookkeeper and consumer tasks are created. A new rule manager is
 * created to manage the rule. This rule manager is added to the bookkeeper and uses the input
 * connector from the consumer task to produce data.
 *
 * During edge copying the bookkeeper and consumer tasks are copied. The rule is reused. This
 * mechanism shares the rule among multiple bookkeepers, but is acceptable as the rule defines
 * a mutex to ensure no race conditions.
 *
 * @tparam T the input type of the Bookkeeper and IRule
 * @tparam U the output type of the IRule and the input type of the consumer ITask
 * @tparam W the output type of the consumer ITask
 */
template<class T, class U, class W>
class RuleEdge : public EdgeDescriptor {
 public:

  /**
   * Creates a rule edge.
   * @param bookkeeper the bookkeeper task
   * @param rule the rule
   * @param consumer the consumer task
   */
  RuleEdge(Bookkeeper<T> *bookkeeper, std::shared_ptr<IRule<T, U>> rule, ITask<U, W> *consumer) :
      bookkeeper(bookkeeper), rule(rule), consumer(consumer) {}

  ~RuleEdge() override {}

  void applyEdge(AnyTaskGraphConf *graph) override {
    graph->getTaskManager(bookkeeper);
    TaskManager<U, W> *consumerTaskManager = graph->getTaskManager(consumer);

    auto connector = consumerTaskManager->getInputConnector();

    if (connector == nullptr) {
      connector = std::shared_ptr<Connector<U>>(new Connector<U>());
    }

    RuleManager<T, U> *ruleManager = new RuleManager<T, U>(rule);
    ruleManager->setOutputConnector(connector);

    connector->incrementInputTaskCount();

    consumerTaskManager->setInputConnector(connector);
    bookkeeper->addRuleManager(ruleManager);

#ifdef WS_PROFILE
    // Add nodes
    std::shared_ptr<ProfileData> producerData(new CreateNodeProfile(bookkeeper, "Bookkeeper"));
    std::shared_ptr<ProfileData> consumerData(new CreateNodeProfile(consumer, consumer->getName()));
    std::shared_ptr<ProfileData> connectorData(new CreateNodeProfile(connector.get(), std::to_string(connector->getProducerCount())));

    graph->sendProfileData(producerData);
    graph->sendProfileData(consumerData);
    graph->sendProfileData(connectorData);

    std::shared_ptr<ProfileData> producerConnectorData(new CreateEdgeProfile(bookkeeper, connector.get()));
    std::shared_ptr<ProfileData> connectorConsumerData(new CreateEdgeProfile(connector.get(), consumer));

    graph->sendProfileData(producerConnectorData);
    graph->sendProfileData(connectorConsumerData);
#endif

  }

  EdgeDescriptor *copy(AnyTaskGraphConf *graph) override {
    return new RuleEdge<T, U, W>((Bookkeeper<T> *) graph->getCopy(bookkeeper), rule, graph->getCopy(consumer));
  }

 private:
  Bookkeeper<T> *bookkeeper; //!< The bookkeeper task
  std::shared_ptr<IRule<T, U>> rule; //!< the rule
  ITask<U, W> *consumer; //!< the consumer task
};
}
#endif //HTGS_RULEEDGE_HPP
