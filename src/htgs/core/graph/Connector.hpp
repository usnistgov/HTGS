
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
#ifndef HTGS_CONNECTOR_HPP
#define HTGS_CONNECTOR_HPP

#include <atomic>

#include <list>

#if defined( __GLIBCXX__ ) || defined( __GLIBCPP__ )
#include <cxxabi.h>
#endif

#include <htgs/api/IData.hpp>
#ifdef USE_PRIORITY_QUEUE
#include <htgs/core/queue/PriorityBlockingQueue.hpp>
#else
#include <htgs/core/queue/BlockingQueue.hpp>
#include <htgs/debug/debug_message.hpp>
#endif

#include "AnyConnector.hpp"

namespace htgs {

/**
 * @class Connector Connector.hpp <htgs/core/graph/Connector.hpp>
 * @brief Manages the input/output of IData between Tasks.
 * @details
 * Each IData that is produced for the Connector is inserted based on
 * the priority specified by the IData (lowest order value first by default IData::getOrder()).
 *
 * Priority queue is enabled by defining the USE_PRIORITY_QUEUE directive during compilation.
 *
 * The Connector manages how many Tasks are producing and consuming data for a particular Connector.
 * For a given ITask, if that ITask has more than one thread associated with it, then each thread
 * acts as a separate producer for the Connector. The Connector will not indicate it has
 * finished producing data until all producers have indicated that they have finished.
 *
 * If a data stream is producing data outside of a TaskGraphConf, then that stream must indicate it is
 * an InputTask otherwise the ITask associated with this Connector may terminate prior to processing data.
 * By incrementing the input task count, will ensure the Connector stays open for an ITask until the stream
 * producing data indicates it has finished. To increment the input Task count use incrementInputTaskCount() and
 * to indicate the input has finished producing data use producerFinished().
 *
 * @tparam T the input/output data type for the Connector, T  must derive from IData.
 * @note This class should only be called by the HTGS API
 * @note Enable priority queue by adding the USE_PRIORITY_QUEUE directive.
 */
template<class T>
class Connector : public AnyConnector {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");

 public:
  /**
   * Initializes the Connector with no producer tasks.
   */
  Connector() {}

  /**
   * Destructor
   */
  ~Connector() {}

  bool isInputTerminated() override { return super::getProducerCount() == 0 && this->queue.isEmpty(); }

  Connector<T> *copy() override { return new Connector<T>(); }

  void wakeupConsumer() override { this->queue.Enqueue(nullptr); }

  void profileProduce(size_t numThreads) override {}

  void profileConsume(size_t numThreads, bool showQueueSize) override {
#ifdef PROFILE
    std::cout << "consume largest queue size: " << queue.getQueueActiveMaxSize() << std::endl;
#endif
  }

  size_t getMaxQueueSize() override {
#ifdef PROFILE
    return queue.getQueueActiveMaxSize();
#else
    return 0;
#endif
  }

  void resetMaxQueueSize() override {
#ifdef PROFILE
    this->queue.resetMaxQueueSize();
#endif
  }

  void produceAnyData(std::shared_ptr<IData> data) override {
    DEBUG_VERBOSE("Connector " << this << " producing any data: " << data);
    std::shared_ptr<T> dataCast = std::dynamic_pointer_cast<T>(data);
    this->queue.Enqueue(dataCast);

  }

  /**
   * Polls for data for a consumer given a timeout.
   * @param timeout the timeout time in microseconds
   * @return the data or nullptr
   * @retval DATA the next data that is on the queue
   * @retval nullptr if the timeout time expires
   *
   * @note This function will block until data is available or the timeout time has expired.
   * @internal
   */
  std::shared_ptr<T> pollConsumeData(size_t timeout) {
    std::shared_ptr<T> data = this->queue.poll(timeout);
    return data;
  }

  /**
   * Consumes data from the queue.
   * @return the data
   *
   * @note This function will block until data is available.
   * @internal
   */
  std::shared_ptr<T> consumeData() {
    std::shared_ptr<T> data = this->queue.Dequeue();
    return data;
  }

  /**
   * Produces data into the queue.
   * @param data the data to be added
   */
  void produceData(std::shared_ptr<T> data) {
    DEBUG_VERBOSE("Connector " << this << " producing data: " << data);
    this->queue.Enqueue(data);
  }

  /**
   * Produces a list of data adding each element into the queue.
   * @param data the list of data t obe added
   */
  void produceData(std::list<std::shared_ptr<T>> *data) {
    for (std::shared_ptr<T> v : *data) {
      DEBUG_VERBOSE("Connector " << this << " producing list data: " << v);

      this->queue.Enqueue(v);
    }
  }

  /**
   * Gets the demangled type name of the connector
   * @return the demangled type name
   */
  std::string typeName() override {
#if defined( __GLIBCXX__ ) || defined( __GLIBCPP__ )
    int status;
    char *realName = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
    std::string ret(realName);

    free(realName);

    return ret;
#else
    return typeid(T).name();
#endif

  }

 private:
  //! @cond Doxygen_Suppress
  typedef AnyConnector super;
  //! @endcond

#ifdef USE_PRIORITY_QUEUE
  PriorityBlockingQueue<std::shared_ptr<T>>
#else
  BlockingQueue <std::shared_ptr<T>>
#endif
      queue; //!< The blocking queue associated with the connector (thread safe) (can be switched to a priority queue using the USE_PRIORITY_QUEUE directive)
};
}

#endif //HTGS_CONNECTOR_HPP
