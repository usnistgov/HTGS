
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file BaseConnector.hpp
 * @author Timothy Blattner
 * @date Nov 23, 2015
 *
 * @brief Holds parent class for Connector, removes template type of Connector.
 */
#ifndef HTGS_BASECONNECTOR_H
#define HTGS_BASECONNECTOR_H

#include <bits/stl_list.h>
#include <iostream>
#include <functional>
#include "../task/BaseTaskScheduler.hpp"

namespace htgs {
class BaseTaskScheduler;

/**
 * @class BaseConnector BaseConnector.hpp <htgs/core/graph/BaseConnector.hpp>
 * @brief Parent class for Connector, which removes the template type of the Connector.
 * @details Used within data structures to hold various types of Connectors. Each
 * Connector is built using TaskGraph routines to add ITasks to a TaskGraph.
 *
 * Each Connector holds onto a priority queue that acquires/distributes IData from a producer/consumer ITask.
 *
 * Most common use for this class is to indicate when the producer for this
 * Connector has finished pushing data onto its priority queue.
 * @code
 * inputConnector->isInputTerminated();
 * @endcode
 */
class BaseConnector {
 public:
  /**
   * Virtual destructor.
   */
  virtual ~BaseConnector() { }

  /**
   * Awakens all Tasks that are consuming data from this connector.
   * This function passes nullptr to each consumer to check whether that consumer is ready to be
   * terminated.
   * @note This function should only be called by the HTGS API
   */
  virtual void wakeupConsumer() {
    std::cerr << "Called BaseConnector 'wakeupConsumer' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Checks whether the producer for this Connector has finished pushing data onto its priority queue.
   * @return whether the input has terminated or not
   * @retval TRUE if the input has terminated and no more data is in the priority queue.
   * @retval FALSE if there is still data to be processed.
   */
  virtual bool isInputTerminated() {
    std::cerr << "Called BaseConnector 'isInputTerminated' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Indicates to the Connector that the producer has finished producing data for the Connector.
   * @note This function should only be called by the HTGS API
   */
  virtual void producerFinished() {
    std::cerr << "Called BaseConnector 'producerFinished' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the number of producers producing data for the connector.
   */
  virtual long getProducerCount() {
    std::cerr << "Called BaseConnector 'getProducerCount' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Creates a copy of the BaseConnector
   * @return a copy of the BaseConnector
   * @note This function should only be called by the HTGS API
   */
  virtual BaseConnector *copy() {
    std::cerr << "Called BaseConnector 'copy' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Increments the number of tasks producing data for the Connector
   * @note This function should only be called by the HTGS API
   */
  virtual void incrementInputTaskCount() {
    std::cerr << "Called BaseConnector 'incrementInputTaskCount' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Provide profile output for the produce operation
   * @param numThreads the number of threads associated with producing data
   * @note \#define PROFILE to enable profiling
   */
  virtual void profileProduce(int numThreads) {
    std::cerr << "Called BaseConnector 'profileProduce' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Provides profile output for the consume operation
   * @param numThreads the number of threads associated with consuming data
   * @param showQueueSize whether to show the max queue size or not
   * @note \#define PROFILE to enable profiling
   */
  virtual void profileConsume(int numThreads, bool showQueueSize) {
    std::cerr << "Called BaseConnector 'profileConsume' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Gets the id used for dot graphs for GraphViz
   * @return the unique id associated with this Connector
   */
  virtual std::string getDotId() {
    std::cerr << "Called BaseConnector 'getDotId' virtual function" << std::endl;
    throw std::bad_function_call();
  }

  /**
   * Generates the dot representation for this connector
   * @return the dot representation
   */
  virtual std::string genDot() {
    std::cerr << "Called BaseConnector 'genDot' virtual function" << std::endl;
    throw std::bad_function_call();
  }

};
}

#endif //HTGS_BASECONNECTOR_H
