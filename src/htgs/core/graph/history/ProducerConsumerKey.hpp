
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file ProducerConsumerKey.hpp
 * @author Timothy Blattner
 * @date Nov 23, 2016
 *
 * @brief Provides the functionality for copying an edge in a TaskGraph
 */
#ifndef HTGS_PRODUCERCONSUMERKEY_H
#define HTGS_PRODUCERCONSUMERKEY_H

#include "htgs/core/task/AnyTaskScheduler.hpp"

namespace htgs {
/**
 *
 * @class ProducerConsumerKey ProducerConsumerKey.hpp <htgs/core/graph/history/ProducerConsumerKey.hpp>
 * @brief Provides the functionality for copying an edge in a TaskGraph
 *
 * @note This function should only be called by the HTGS API
 */
class ProducerConsumerKey {
 public:
  /**
   * Creates the producer consumer key that describes how the edge was added into the TaskGraph
   * @param producer the producer TaskScheduler
   * @param consumer the consumer TaskScheduler
   */
  ProducerConsumerKey(AnyTaskScheduler *producer, AnyTaskScheduler *consumer) {
    this->producer = producer;
    this->consumer = consumer;
  }

  /**
   * Destructor.
   */
  ~ProducerConsumerKey() { }

  /**
   * Gets the producer TaskScheduler
   * @return the producer TaskScheduler
   */
  AnyTaskScheduler *getProducer() const {
    return this->producer;
  }

  /**
   * Gets the consumer TaskScheduler
   * @return the consumer TaskScheduler
   */
  AnyTaskScheduler *getConsumer() const {
    return this->consumer;
  }

 private:
  AnyTaskScheduler *producer; //!< The TaskScheduler that manages the producer
  AnyTaskScheduler *consumer; //!< The TaskScheduler that manages the consumer
};
}

#endif //HTGS_PRODUCERCONSUMERKEY_H
