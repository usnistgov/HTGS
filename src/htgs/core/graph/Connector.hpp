
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file Connector.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief Provides the Connector class for managing input/output of AbsData between Tasks
 */
#ifndef HTGS_CONNECTOR_H
#define HTGS_CONNECTOR_H


#include <atomic>

#include "../task/TaskScheduler.hpp"
#include "../queue/BlockingQueue.hpp"
#include "../../debug/debug_message.h"
#include "../queue/PriorityBlockingQueue.hpp"
#include "BaseConnector.hpp"
#include "../../api/IData.hpp"

namespace htgs {
template<class T>
class PriorityBlockingQueue;

class IData;

template<class T, class U>
class TaskScheduler;

/**
 * @class Connector Connector.hpp <htgs/core/graph/Connector.hpp>
 * @brief Manages the input/output of IData between Tasks.
 * @details
 * Each IData that is produced for the Connector is inserted based on
 * the priority specified by the IData (lowest order value first by default IData::getOrder()).
 *
 * The Connector manages how many Tasks are producing and consuming data for a particular Connector.
 * For a given ITask, if that ITask has more than one thread associated with it, then each thread
 * acts as a separate producer for the Connector. The Connector will not indicate it has
 * finished producing data until all producers have indicated that they have finished.
 *
 * If a data stream is producing data outside of a TaskGraph, then that stream must indicate it is
 * an InputTask otherwise the ITask associated with this Connector may terminate prior to processing data.
 * By incrementing the input task count, will ensure the Connector stays open for an ITask until the stream
 * producing data indicates it has finished. To increment the input Task count use incrementInputTaskCount() and
 * to indicate the input has finished producing data use producerFinished().
 *
 * @tparam T the input/output data type for the Connector, T  must derive from IData.
 * @note This class should only be called by the HTGS API
 */
template<class T>
class Connector: public BaseConnector {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");

 public:
  /**
   * Initializes the Connector with no producer tasks.
   */
  Connector() {
    this->producerTaskCount = 0;
  }

  /**
   * Destructor
   */
  ~Connector() { }

  /**
   * Increments the number of producers for the Connector
   */
  void incrementInputTaskCount() { this->producerTaskCount += 1; }

  /**
   * Checks whether the producer for this Connector has finished pushing data onto its priority queue.
   * @return whether the input has terminated or not
   * @retval TRUE if the input has terminated and no more data is in the priority queue.
   * @retval FALSE if there is still data to be processed.
   */
  bool isInputTerminated() { return this->producerTaskCount == 0 && this->queue.isEmpty(); }

  /**
   * Indicates that a producer has finished producing data.
   */
  void producerFinished() {
    this->producerTaskCount -= 1;
  }

  /**
   * Polls for data for a consumer given a timeout.
   * @param timeout the timeout time in microseconds
   * @return the data or nullptr
   * @retval DATA the next data that is on the priority queue
   * @retval nullptr if the timeout time expires
   * @note This function will block until data is available or the timeout time has expired.
   */
  std::shared_ptr<T> pollConsumeData(long timeout) {
    std::shared_ptr<T> data = this->queue.poll(timeout);
    return data;
  }

  /**
   * Consumes data from the priority queue.
   * @return the data
   * @note This function will block until data is available.
   */
  std::shared_ptr<T> consumeData() {
    std::shared_ptr<T> data = this->queue.Dequeue();
    return data;
  }

  /**
   * Produces data into the priority queue.
   * @param data the data to be added
   */
  void produceData(std::shared_ptr<T> data) {
    DEBUG_VERBOSE("Connector " << this << " producing data: " << data);
    this->queue.Enqueue(data);
  }

  /**
   * Produces a list of data adding each element into the prioirty queue.
   * @param data the list of data t obe added
   */
  void produceData(std::list<std::shared_ptr<T>> *data) {
    for (std::shared_ptr<T> v : *data) {
      DEBUG_VERBOSE("Connector " << this << " producing list data: " << v);

      this->queue.Enqueue(v);
    }
  }

  /**
   * Creates a shallow copy of the Connector.
   * Does not copy the priority queue contents.
   * @return the shallow copy of the Connector
   */
  Connector<T> *copy() {
    return new Connector<T>();
  }

  /**
   * Wakes up a consumer waiting for data.
   */
  void wakeupConsumer() { this->queue.Enqueue(nullptr); }

  /**
   * Gets the number of producers adding data to the Connector.
   */
  long getProducerCount() { return this->producerTaskCount; }

  /**
   * Provide profile output for the produce operation
   * @param numThreads the number of threads associated with producing data
   * @note \#define PROFILE to enable profiling
   */
  void profileProduce(int numThreads) {
#ifdef PROFILE
    std::cout << "produce (per thread) wait time: " << (queue.getEnqueueWaitTime()/numThreads) << " us, lock time: " << (queue.getEnqueueLockTime()/numThreads) << " us" << std::endl;
#endif
  }

  /**
   * Provides profile output for the consume operation
   * @param numThreads the number of threads associated with consuming data
   * @param showQueueSize whether to show the max queue size or not
   * @note \#define PROFILE to enable profiling
   */
  void profileConsume(int numThreads, bool showQueueSize) {
#ifdef PROFILE
    if(showQueueSize)
        std::cout << "consume (per thread) wait time: " << (queue.getDequeueWaitTime()/numThreads) << " us, lock time: " << (queue.getDequeueLockTime()/numThreads) << " us, largest queue size: " << queue.getQueueActiveMaxSize() << std::endl;
    else
        std::cout << "consume (per thread) wait time: " << (queue.getDequeueWaitTime()/numThreads) << " us, lock time: " << (queue.getDequeueLockTime()/numThreads) << " us" << std::endl;
#endif
  }

  /**
   * Gets the id used for dot graphs for GraphViz
   * @return the unique id associated with this Connector
   */
  virtual std::string getDotId() {
    std::ostringstream inConn;
    inConn << this;
    std::string inConnStr = inConn.str();
    inConnStr.erase(0, 1);

    return inConnStr;

  }

  /**
   * Generates the dot representation for this connector
   * @return the dot representation
   */
  virtual std::string genDot() {

//    return getDotId() + "[label=\"\",shape=box,style=filled,color=black,width=.2,height=.2];\n";
    return getDotId() + "[label=\"" + std::to_string(this->getProducerCount()) +"\",shape=box,style=rounded,color=black,width=.2,height=.2];\n";

  }

 private:

#ifdef USE_PRIORITY_QUEUE
  PriorityBlockingQueue<std::shared_ptr<T>>
#else
  BlockingQueue<std::shared_ptr<T>>
#endif
      queue; //!< The pblocking queue associated with the connector (thread safe) (can be switched to a priority queue using the USE_PRIORITY_QUEUE directive)
  std::atomic_long producerTaskCount; //!< The number of producers adding data to the connector
};
}


#endif //HTGS_CONNECTOR_H
