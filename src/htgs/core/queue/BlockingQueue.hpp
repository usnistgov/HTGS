
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file BlockingQueue.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief Implements a thread-safe BlockingQueue
 */
#ifndef HTGS_BLOCKINGQUEUE_HPP
#define HTGS_BLOCKINGQUEUE_HPP

#include <condition_variable>
#include <deque>
#include <queue>

namespace htgs {
/**
 * @class BlockingQueue BlockingQueue.hpp <htgs/core/queue/BlockingQueue.hpp>
 * @brief Creates a thread-safe queue that will wait when no data is available and can block if the queue is full.
 * @details
 * If the size of the queue is specified to be less than 0, then the queue will not block if the queue is full.
 *
 */
template<class T>
class BlockingQueue {
 public:
  /**
   * Creates a blocking queue that will only block a data requester when the queue is empty.
   */
  BlockingQueue() {
    this->queueSize = 0;
#ifdef PROFILE
    enqueueLockTime = 0;
    dequeueLockTime = 0;
    enqueueWaitTime = 0;
    dequeueWaitTime = 0;
    queueActiveMaxSize = 0;
#endif
  }

  /**
   * Creates a blocking queue that will block a data requester when the queue is empty or full.
   * @param qSize
   */
  BlockingQueue(size_t qSize) {
    this->queueSize = qSize;
#ifdef PROFILE
    enqueueLockTime = 0;
    dequeueLockTime = 0;
    enqueueWaitTime = 0;
    dequeueWaitTime = 0;
    queueActiveMaxSize = 0;
#endif
  }

  /**
   * Destructor
   */
  ~BlockingQueue() {
  }

  /**
   * Gets the remaining capacity of the queue based on the queueSize.
   * This function should only be used if the queueSize > 0
   * @return the remaining size of the queue before it is full
   */
  size_t remainingCapacity() {
    if (queueSize == 0) {
      std::cerr << __FILE__ << ":" << __LINE__
          << "ERROR: Requesting remaining capacity on BlockingQueue that does not have a max size" << std::endl;
    }
    return queueSize - queue.size();
  }

  /**
   * Gets whether the queue is empty or not
   * @return whether the queue is empty
   * @retval TRUE if the queue is empty
   * @retval FALSE if the queue is not empty
   */
  bool isEmpty() {
    return queue.empty();
  }

  /**
   * Gets the number of elements in the queue
   * @return the number of elements in the queue
   */
  size_t size() {
    return queue.size();
  }

  /**
   * @internal
   * Removes an element from the queue
   * @return an element from the queue
   *
   * @note This function is not thread safe.
   */
  T remove() {
    T res = this->queue.front();
    this->queue.pop();
    return res;
  }

  /**
   * Adds an element into the queue
   * @param value the element to be added
   * @note Is thread safe.
   * @note Will block if the maximum queue size > 0 and the number of elements in the queue is equal to the maximum queue size
   */
  void Enqueue(T const &value) {

#ifdef PROFILE
            auto start = std::chrono::high_resolution_clock::now();
#endif
      std::unique_lock<std::mutex> lock(this->mutex);
#ifdef PROFILE
    auto end = std::chrono::high_resolution_clock::now();
    this->enqueueLockTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
#endif
      if (this->queueSize > 0) {
#ifdef PROFILE
        start = std::chrono::high_resolution_clock::now();
#endif
        this->condition.wait(lock, [=] { return this->queue.size() != queueSize; });
#ifdef PROFILE
        end = std::chrono::high_resolution_clock::now();
      this->enqueueWaitTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
#endif
      }
      queue.push(value);

#ifdef PROFILE
    if (queue.size() > queueActiveMaxSize)
        queueActiveMaxSize = queue.size();
#endif

    this->condition.notify_one();

  }

  /**
   * Removes an element from the queue
   * @return the next element in the queue
   * @note Is thread safe.
   * @note Will block if the queue is empty.
   */
  T Dequeue() {
#ifdef PROFILE
    auto start = std::chrono::high_resolution_clock::now();
#endif
    std::unique_lock<std::mutex> lock(this->mutex);
#ifdef PROFILE
    auto end = std::chrono::high_resolution_clock::now();
    this->dequeueLockTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
#endif
    this->condition.wait(lock, [=] { return !this->queue.empty(); });
#ifdef PROFILE
    end = std::chrono::high_resolution_clock::now();
    this->dequeueWaitTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
#endif
    T res = this->queue.front();
    this->queue.pop();
    return res;
  }

  /**
   * Polls for data given the specified timeout time in microseconds.
   * @param timeout the timeout time in microseconds
   * @return the data or nullptr if the timeout expires
   * @retval data if data exists prior to the timeout time
   * @retval nullptr if no data exists after the timeout time expires
   */
  T poll(size_t timeout) {
    std::unique_lock<std::mutex> lock(this->mutex);
    if (this->condition.wait_for(lock, std::chrono::microseconds(timeout),
                                 [=] { return !this->queue.empty(); })) {
      T res = this->queue.front();
      this->queue.pop();
      return res;
    }
    return nullptr;
  }

#ifdef PROFILE
  unsigned long long int getEnqueueLockTime() const {
      return enqueueLockTime;
  }

  unsigned long long int getDequeueLockTime() const {
      return dequeueLockTime;
  }

  unsigned long long int getEnqueueWaitTime() const {
      return enqueueWaitTime;
  }

  unsigned long long int getDequeueWaitTime() const {
      return dequeueWaitTime;
  }

  size_t getQueueActiveMaxSize() const {
      return queueActiveMaxSize;
  }
#endif


 private:
#ifdef PROFILE
  unsigned long long int enqueueLockTime; //!< The time to lock before enqueue
  unsigned long long int dequeueLockTime; //!< The time to lock before dequeue
  unsigned long long int enqueueWaitTime; //!< The time waiting to enqueue
  unsigned long long int dequeueWaitTime; //!< The time waiting to dequeue
  size_t queueActiveMaxSize; //!< The maximum size the queue reached in its lifetime
#endif
  size_t queueSize; //!< The maximum size of the queue, set to -1 for infinite size
  std::queue<T> queue; //!< The FIFO queue
  std::mutex mutex; //!< The mutex to ensure thread safety
  std::condition_variable condition; //!< The condition variable used for waking up waiting threads
};
}


#endif //HTGS_BLOCKINGQUEUE_HPP
