
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file ICustomEdge.hpp
 * @author Timothy Blattner
 * @date Feb 9, 2016
 *
 * @brief Provides an interface to define custom edges in a TaskGraph.
 * This is required if custom edges are to be duplicated for an ExecutionPipline.
 */
#ifndef HTGS_ICUSTOMEDGE_H
#define HTGS_ICUSTOMEDGE_H

#include "../core/graph/BaseConnector.hpp"

namespace htgs {

/**
 * @class ICustomEdge ICustomEdge.hpp <htgs/api/ICustomEdge.hpp>
 * @brief Interface for defining a custom edge to be added into a TaskGraph.
 * @details
 *
 * The purpose of this interface is to define an edge with additional functionality. Two examples
 * that demonstrates this concept is reimplementing the Bookkeeper and MemoryManager using a custom edge.
 * The source code for these is found in test/api/BookkeeperCustomEdge.h and test/api/MemoryManagerCustomEdge.h
 *
 * The ICustomEdge assumes that both the producer and consumer tasks that are connected by the
 * custom edge are within the same TaskGraph. If each are within two separate graphs (example:
 * one is inside of a TaskGraph added to an ExecutionPipeline and the other is within a graph that
 * holds onto the ExecutionPipeline), then the ICustomEdge will not work. To add more advanced
 * edges (such as connecting two tasks from two different TaskGraphs) it is required to inherit
 * the TaskGraph and add custom functionality (see TaskGraph::addRule or TaskGraph::addMemoryManagerEdge )
 *
 * Example Implementation (Bookkeeper):
 * @code
 * template <class T, class U, class V>
 * class BookkeeperCustomEdge : public htgs::ICustomEdge {
 * public:
 *    BookkeeperCustomEdge(htgs::Bookkeeper<T> *bk, htgs::ITask<U, V> *consumer) {
 *      this->ruleManager = new htgs::RuleManager<T, U>();
 *      ...
 *    }
 *    ...
 *    virtual void applyGraphConnection(htgs::BaseTaskScheduler *producer, htgs::BaseTaskScheduler *consumer,
 *                                        htgs::BaseTaskGraph *taskGraph) override {
 *
 *      htgs::Bookkeeper<T> *bk = (htgs::Bookkeeper<T> *)producer->getTaskFunction();
 *
 *      // Create edge from bk -> ruleManager -> consumer
 *      bk->addRuleManager(this->ruleManager);
 *      ruleManager->setOutputConnector(connector);
 *      connector->setConsumer(consumer);
 *    }
 *    ...
 *
 *    void addRule(htgs::IRule<T, U> *rule) { this->ruleManager->addRule(rule); }
 *    ...
 * private:
 *   htgs::RuleManager<T, U> *ruleManager;
 *
 * };
 * @endcode
 *
 * Example Usage:
 * @code
 * BookkeeperCustomEdge<FFTData, PCIAMData, PCIAMData> *bkCustomEdge = new BookkeeperCustomEdge<FFTData, PCIAMData, PCIAMData>(bookkeeper, pciamTask);
 * bkCustomEdge->addRule(stitchingRule);
 *
 * taskGraph->addCustomEdge(bkCustomEdge);
 *
 * @endcode
 */
class ICustomEdge {
 public:
  /**
   * Creates a custom edge with a default priority.
   * When the ICustomEdge is added into a task graph, its ordering will be updated
   * based on the order in which each ICustomEdge is added into the TaskGraph.
   * When the edge is copied, the lowest order edge will be added first.
   */
  ICustomEdge() { }

  /**
   * Destructor to release custom edge memory.
   */
  virtual ~ICustomEdge() { }

  /**
   * Pure virtual function that creates a copy of the custom edge.
   * If a TaskGraph is copied using execution pipelines, this routine is used
   * to duplicate the custom edge. Should only copy components that define the custom edge.
   * The producer and consumer tasks will be passed to ICustomEdge::applyGraphConnection
   */
  virtual ICustomEdge *copy() = 0;

  /**
   * Pure virtual function that applies the connection for the TaskGraph.
   * The producer and consumer tasks are passed in as the two vertices that should be connected.
   * The connector provided is an optional parameter and will only be defined if useConnector() returns
   * true.
   *
   * The BaseTaskGraph is provided to add any new vertices that the TaskGraph should be aware of. It is
   * mandatory to provide any new vertices that are created inside of this function to the TaskGraph so
   * that threads for that task can be spawned.
   *
   * The types for the producer and consumer are determined by the createProducerTask and createConsumerTask
   * functions, respectively. If a consumer or producer Task was already added into a TaskGraph from a previous call
   * to TaskGraph::addEdge, then the Task associated with that call is passed into the applyGraphConnection function.
   *
   * The logic behind this function is to describe how the producer and consumer tasks are connected. One basic
   * example that does a simple connection between the two:
   * @code
   * connector->setProducer(producer);
   * connector->setConsumer(consumer);
   * @endcode
   *
   * Both the producer and consumer tasks are already within the TaskGraph. If a producer or consumer Task
   * is outside of the TaskGraph, then ICustomEdge will not work. To add more complex functionality such as
   * connecting edges between two TaskGraphs, then the programmer must define that functionality through
   * creating inheriting the TaskGraph and creates a new type of TaskGraph.
   *
   * The pipelineId specified is the execution pipeline id that this graph is bound to. Any tasks that are created for
   * this TaskGraph should have their pipelineIds set to the passed in pipelineId.
   *
   * @param producer the producer task
   * @param consumer the consumer task
   * @param connector (optional) connector that should be used between the producer and consumer tasks
   * @param pipelineId the pipelineId that the taskGraph belongs to
   * @param taskGraph the taskGraph that the edge is added to
   */
  virtual void applyGraphConnection(BaseTaskScheduler *producer,
                                    BaseTaskScheduler *consumer,
                                    std::shared_ptr<BaseConnector> connector,
                                    int pipelineId,
                                    htgs::BaseTaskGraph *taskGraph) = 0;

  /**
   * Creates the connector associated with the custom edge
   */
  virtual BaseConnector *createConnector() = 0;

  /**
   * Creates the producer task for the custom edge (should be associated with the ProducerITask).
   * For a given ITask<T, U> iTask, the Task should be constructed using
   * Task<T, U>::createTask(iTask);
   * @return the Task associated with the producer
   */
  virtual BaseTaskScheduler *createProducerTask() = 0;

  /**
   * Creates the consumer task for the custom edge (should be associated with the ConsumerITask).
   * For a given ITask<T, U> iTask, the Task should be constructed using
   * Task<T, U>::createTask(iTask);
   * @return the Task associated with the consumer
   */
  virtual BaseTaskScheduler *createConsumerTask() = 0;

  /**
   * Gets the ITask associated with the producer.
   * This ITask is used to lookup the Task that holds the producer. If
   * no task is found, then it will create the producer Task with createProducerTask
   * @return the ITask associated with the producer
   */
  virtual BaseITask *getProducerITask() = 0;

  /**
   * Gets the ITask associated with the consumer.
   * This ITask is used to lookup the Task that holds the consumer. If
   * no task is found, then it will create the consumer Task with createConsumerTask
   * @return the ITask associated with the consumer
   */
  virtual BaseITask *getConsumerITask() = 0;

  /**
   * Gets whether a Connector should be created for the edge.
   * This can be useful for new Connections that may not be associated with the
   * TaskGraph such as a memory edge.
   * If this value is set to true, then the consumer ITask will be used to lookup an
   * existing input connector for the consumer. If one is not found, then it will create
   * a new Connector using createConnector
   * @return whether the connector should be created or not
   * @retval true if the connector should be created
   * @retval false if the connector should not be created
   */
  virtual bool useConnector() = 0;
};
}


#endif //HTGS_ICUSTOMEDGE_H

