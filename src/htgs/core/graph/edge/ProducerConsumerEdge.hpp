// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file ProducerConsumerEdge.hpp
 * @author Timothy Blattner
 * @date March 1, 2017
 *
 * @brief Implements a producer consumer edge, which is a type of edge descriptor.
 */
#ifndef HTGS_PRODUCERCONSUMEREDGE_HPP
#define HTGS_PRODUCERCONSUMEREDGE_HPP

#include "EdgeDescriptor.hpp"
#include <htgs/api/ITask.hpp>
#include <htgs/core/graph/AnyTaskGraphConf.hpp>
#ifdef WS_PROFILE
#include <htgs/core/graph/profile/CustomProfile.hpp>
#endif

namespace htgs {

/**
 * @class ProducerConsumerEdge ProducerConsumerEdge.hpp <htgs/core/graph/edge/ProducerConsumerEdge.hpp>
 * @brief Implements the producer consumer edge that connects two tasks where one task is producing
 * data and the other is consuming.
 *
 * The edge is applied by getting the task managers for two ITasks and setting the output and
 * input connectors for the producer and consumer tasks to be the same, respectively.
 *
 * When the edge is copied the ITasks that represent the producer and consumer are retrieved
 * from the task graph that will become the copied graph.
 *
 * @tparam T the input type of the producer task
 * @tparam U the output type of the producer task and the input type of the consumer task
 * @tparam W the output type of the consumer task
 */
template<class T, class U, class W>
class ProducerConsumerEdge : public EdgeDescriptor {
 public:

  /**
   * Constructs a producer consumer edge.
   * @param producer the task producing data
   * @param consumer the task consuming the data from the producer task
   */
  ProducerConsumerEdge(ITask<T, U> *producer, ITask<U, W> *consumer) : producer(producer), consumer(consumer) {}

  ~ProducerConsumerEdge() override {}

  void applyEdge(AnyTaskGraphConf *graph) override {
    TaskManager<T, U> *producerTaskManager = graph->getTaskManager(producer);
    TaskManager<U, W> *consumerTaskManager = graph->getTaskManager(consumer);

    auto connector = consumerTaskManager->getInputConnector();

    if (connector == nullptr) {
      connector = std::shared_ptr<Connector<U>>(new Connector<U>());
    }

    connector->incrementInputTaskCount();

    consumerTaskManager->setInputConnector(connector);
    producerTaskManager->setOutputConnector(connector);
#ifdef WS_PROFILE
    // Add nodes
    std::shared_ptr<ProfileData> producerData(new CreateNodeProfile(producer, nullptr, producer->getName()));
    std::shared_ptr<ProfileData> consumerData(new CreateNodeProfile(consumer, nullptr, consumer->getName()));
    std::shared_ptr<ProfileData> connectorData(new CreateNodeProfile(connector.get(), nullptr, std::to_string(connector->getProducerCount())));

    graph->sendProfileData(producerData);
    graph->sendProfileData(consumerData);
    graph->sendProfileData(connectorData);

    std::shared_ptr<ProfileData> producerConnectorData(new CreateEdgeProfile(producer, connector.get(), "", nullptr));
    std::shared_ptr<ProfileData> connectorConsumerData(new CreateEdgeProfile(connector.get(), consumer, "", nullptr));

    graph->sendProfileData(producerConnectorData);
    graph->sendProfileData(connectorConsumerData);
#endif
  }

  EdgeDescriptor *copy(AnyTaskGraphConf *graph) override {
    return new ProducerConsumerEdge(graph->getCopy(producer), graph->getCopy(consumer));
  }

 private:
  ITask<T, U> *producer; //!< The producer ITask
  ITask<U, W> *consumer; //!< The consumer ITask

};
}
#endif //HTGS_PRODUCERCONSUMEREDGE_HPP
