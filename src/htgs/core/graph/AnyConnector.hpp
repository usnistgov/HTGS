// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file AnyConnector.hpp
 * @author Timothy Blattner
 * @date Nov 23, 2015
 *
 * @brief Holds parent class for Connector, removes template type of Connector.
 */
#ifndef HTGS_ANYCONNECTOR_HPP
#define HTGS_ANYCONNECTOR_HPP

#include <atomic>
#include <sstream>

#include <htgs/api/IData.hpp>

namespace htgs {

/**
 * @class AnyConnector AnyConnector.hpp <htgs/core/graph/AnyConnector.hpp>
 * @brief Parent class for Connector, which removes the template type of the Connector.
 * @details Used to hold various types of Connectors. Each
 * Connector is built using an EdgeDescriptor routines to add ITasks to a TaskGraphConf.
 *
 * Each Connector holds onto a priority queue that acquires/distributes IData from a producer/consumer ITask.
 *
 * Most common use for this class is to indicate when the producer for this
 * Connector has finished pushing data onto its queue.
 *
 * Common usage:
 * @code
 * inputConnector->isInputTerminated();
 * @endcode
 */
class AnyConnector {
 public:

  /**
   * Constructor initializing the producer task count to 0.
   */
  AnyConnector() : producerTaskCount(0) {}

  /**
   * Virtual destructor.
   */
  virtual ~AnyConnector() {}

  /**
  * Indicates to the Connector that the producer has finished producing data for the Connector.
  *
  * @note This function should only be called by the HTGS API
   * @internal
  */
  void producerFinished() {
    this->producerTaskCount--;
  }

  /**
   * Gets the number of producers producing data for the connector.
   * @return the number of producers adding data for this connector.
   */
  size_t getProducerCount() {
    return this->producerTaskCount;
  }

  /**
   * Increments the number of tasks producing data for the Connector
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  void incrementInputTaskCount() { this->producerTaskCount++; }

  /**
 * Gets the id used for dot graphs for GraphViz
 * @return the unique id associated with this Connector
 */
  std::string getDotId() {
    std::ostringstream inConn;
    inConn << "x" << this;
    std::string inConnStr = inConn.str();

    return inConnStr;
  }

  /**
   * Generates the dot representation for this connector
   * @param flags dot gen flags
   * @return the dot representation
   */
  std::string genDot(int flags) {
    return getDotId() + "[label=\"" + std::to_string(this->getProducerCount())
        + "\",shape=box,style=rounded,color=black,width=.2,height=.2];\n";

  }

  /**
   * Gets the demangled type name of the connector
   * @return the demangled type name
   */
  virtual std::string typeName() = 0;

  /**
   * Checks whether the producer for this Connector has finished pushing data onto its queue.
   * @return whether the input has terminated or not
   * @retval TRUE if the input has terminated and no more data is in the queue.
   * @retval FALSE if there is still data to be processed.
   */
  virtual bool isInputTerminated() = 0;

  /**
   * Awakens all Tasks that are consuming data from this connector.
   * This function passes nullptr to each consumer to check whether that consumer is ready to be
   * terminated.
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  virtual void wakeupConsumer() = 0;

  /**
   * Creates a copy of the BaseConnector
   * @return a copy of the BaseConnector
   *
   * @note This function should only be called by the HTGS API
   * @internal
   */
  virtual AnyConnector *copy() = 0;

  /**
   * Produces any data into the queue. This function should be used with care as the
   * data will be dynamically cast to the type of Connector.
   * @param data the data that will be added to the Connector's queue.
   */
  virtual void produceAnyData(std::shared_ptr<IData> data) = 0;

  /**
   * Provide profile output for the produce operation
   * @param numThreads the number of threads associated with producing data
   * @note \#define PROFILE to enable profiling
   */
  virtual void profileProduce(size_t numThreads) = 0;

  /**
   * Provides profile output for the consume operation
   * @param numThreads the number of threads associated with consuming data
   * @param showQueueSize whether to show the max queue size or not
   * @note \#define PROFILE to enable profiling
   */
  virtual void profileConsume(size_t numThreads, bool showQueueSize) = 0;

  /**
   * Gets the maximum queue size that this connector has in its data queue.
   * @return the maximum queue size for the connector
   */
  virtual size_t getMaxQueueSize() = 0;

  /**
   * Gets the size of the queue that this connector has in its data queue.
   * @return the size of the queue for the connector
   */
  virtual size_t getQueueSize() = 0;

  /**
   * Resets the max queue size profile.
   */
  virtual void resetMaxQueueSize() = 0;

 private:
  std::atomic_size_t producerTaskCount; //!< The number of producers adding data to the connector

};
}

#endif //HTGS_ANYCONNECTOR_HPP
