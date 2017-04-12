// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file TaskGraphCommunicator.hpp
 * @author Timothy Blattner
 * @date March 3, 2017
 *
 * @brief Implements the task graph communicator task that communicates from each task to
 * all other tasks in a graph.
 *
 */
#ifndef HTGS_TASKGRAPHCOMMUNICATOR
#define HTGS_TASKGRAPHCOMMUNICATOR

#include <unordered_map>
#include <htgs/core/graph/AnyConnector.hpp>
#include <htgs/core/queue/BlockingQueue.hpp>
#include <thread>
#include "DataPacket.hpp"

namespace htgs {

class TaskGraphCommunicator;

/**
 * @typedef TaskCommMap
 * A mapping between the name of a task and its task graph communicator
 */
typedef std::unordered_map<std::string, TaskGraphCommunicator *> TaskCommMap;

/**
 * @typedef TaskCommPair
 * A pair used for the TaskCommMap
 */
typedef std::pair<std::string, TaskGraphCommunicator *> TaskCommPair;

/**
 * @class TaskGraphCommunicator TaskGraphCommunicator.hpp <htgs/core/comm/TaskGraphCommunicator.hpp>
 * @brief Implements the task graph communicator where a task's address and name are mapped to
 * their input connectors.
 *
 * This class's run function is bound to a thread once all task graphs (including sub-graphs), have
 * been built and spawned. Once all threads are active, then a thread is bound to the task graph
 * communicator. All graphs within execution pipelines and the main root graph have a separate
 * task graph communicator, but share the task address mapping.
 *
 * Initially the main graph represents the root of a tree of graphs with branches being defined
 * by execution pipelines. Once all threads and execution pipelines have been created, then the
 * root task graph communicator gathers all addresses and creates the mapping. Once the mapping
 * is complete, then all task graph communicators are bound to separate threads. The mapping is
 * read-only at this point.
 *
 * Each task can submit a data packet into the task graph communicator, which will then send the
 * data directly into the input connector for that data packet's destination.
 *
 * A DataPacket is inserted into the task graph communicator, which provides meta data for looking up the end point
 * location for the data packet. The data packet holds IData, which is then inserted into the end point's input connector.
 *
 * @note The IData type must match the end point input connector's data type.
 *
 *
 */
class TaskGraphCommunicator {
 public:

  /**
   * Constructs the task graph communicator.
   * If the parent specified is nullptr, then this instance is the root communicator within a tree of communicators.
   * @param parent the parent communicator or nullptr if this communicator is the root
   * @param address the address that this task graph communicator represents
   */
  TaskGraphCommunicator(TaskGraphCommunicator *parent, std::string address)
      : parentComm(parent), address(address) {

    taskNameConnectorMap = new std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>>();
    flattenedTaskNameConnectorMap = nullptr;
    children = new TaskCommMap();
    if (parentComm != nullptr) {
      parentComm->addChild(this);
    }

    numGraphsSpawned = 0;
    numGraphsReceived = 0;

    terminated = false;

    thread = nullptr;
  }

  ~TaskGraphCommunicator() {
    delete taskNameConnectorMap;
    taskNameConnectorMap = nullptr;

    delete children;
    children = nullptr;

    delete thread;
    thread = nullptr;
  }

  /**
   * Spawns threads only if the task graph communicator calling this function is the root communicator.
   * @note If the parent communicator is nullptr, then that instance is the root.
   */
  void rootSpawnThreads() {
    // Validate this is root
    if (this->parentComm == nullptr) {
      // Flatten lookup table for parent and children
      this->flattenedTaskNameConnectorMap = std::shared_ptr<std::unordered_multimap<std::string,
                                                                                    std::shared_ptr<AnyConnector>>>(new std::unordered_multimap<
          std::string,
          std::shared_ptr<AnyConnector>>());

      this->processFlattenTaskNameConnectorMap(this->flattenedTaskNameConnectorMap);

      this->spawnChildrenThreads();
    }
  }

  /**
   * Spawns the threads for all children communicator
   * @note this function is only called by the parent communicator.
   */
  void spawnChildrenThreads() {
    this->spawnThread();

    if (this->children->size() > 0) {
      for (auto commChild : *this->children) {
        commChild.second->spawnChildrenThreads();
      }
    }
  }

  /**
   * Spawns the thread for this communicator
   */
  void spawnThread() {
    this->thread = new std::thread(&TaskGraphCommunicator::run, this);
  }

  /**
   * Sets the number of graphs spawned
   * @param numGraphsSpawned the number of graphs spawned
   */
  void setNumGraphsSpawned(size_t numGraphsSpawned) { this->numGraphsSpawned = numGraphsSpawned; }

  /**
   * Prints the address of the parent communicator recursively to std::cout.
   * @param prefix the prefix for printing
   */
  void printParents(std::string prefix) {
    std::cout << prefix << "Address = " << this->address << std::endl;

    if (this->getParentComm() == nullptr) {
      std::cout << std::endl << "=====DONE=====" << std::endl;
      return;
    }

    this->getParentComm()->printParents(prefix + "\t\t");

  }

  /**
   * Prints the task graph communicator tree recursively to std::cout.
   * @param prefix the prefix for printing
   */
  void printTree(std::string prefix) {
//    std::cout << "Num graphs spawned = " << numGraphsSpawned << " Graphs received: " << numGraphsReceived << std::endl;
    if (this->getParentComm() == nullptr) {
      std::cout << "PARENT addr: " << this->getAddress() << std::endl;
    } else {
      std::cout << "Parent address = " << this->getParentComm()->getAddress() << std::endl;
    }

    std::cout << prefix << "Num children: " << this->getChildren()->size() << " Num connectors = "
              << taskNameConnectorMap->size() << std::endl;
    for (auto conn : *taskNameConnectorMap) {
      std::cout << prefix << "\t\t" << conn.first << std::endl;
    }

    for (auto child : *this->getChildren()) {
      std::cout << prefix << " CHILD addr: " << child.first << std::endl;
      child.second->printTree(prefix + "\t");
    }

  }

  /**
   * Gets the parent communicator
   * @return the parent communicator
   * @note If the parent communicator is nullptr, then that communicator is the root.
   */
  TaskGraphCommunicator *getParentComm() const {
    return parentComm;
  }

  /**
   * Gets the children for the task graph communicator
   * @return the children
   */
  TaskCommMap *getChildren() const {
    return children;
  }

  /**
   * Checks if the root can spawn threads yet or not.
   * This function will recursively be called until it reaches the root communicator. The root
   * communicator will then verify if the number of graphs received is equal to the number of graphs spawned.
   * If they are equal, then all of the threads will be initiated. Doing so ensures all tasks and sub-graphs have
   * completed spawning and the mapping between all tasks in the graph has been completed.
   */
  void checkRootSpawnThreads() {
    if (this->parentComm == nullptr) {
      if (numGraphsReceived == numGraphsSpawned) {
        this->rootSpawnThreads();
      }
    } else {
      this->parentComm->checkRootSpawnThreads();
    }
  }

  /**
   * Increments the number of graphs received by the root communicator.
   *
   * This function is called recursively, and only the root communicator is incremented.
   */
  void incrementRootCommunicatorGraphs() {
    // Check if this is the root
    if (this->parentComm == nullptr) {
      numGraphsReceived++;

      // If all the graphs have produced their updates, then begin the communication threads
      if (numGraphsReceived == numGraphsSpawned) {
        this->rootSpawnThreads();
      }

    } else {
      this->parentComm->incrementRootCommunicatorGraphs();
    }
  }

  /**
   * Gets the number of graphs received by the root communicator.
   * This function is called recursively until the root is reached.
   *
   * @return the number of graphs received by the root communicator
   */
  size_t getRootNumGraphsReceived() {
    if (this->parentComm == nullptr) {
      return numGraphsReceived;
    } else {
      return this->parentComm->getRootNumGraphsReceived();
    }
  }

  /**
   * Gets the number of graphs spawned by the root communicator.
   * This function is called recursively until the root is reached
   *
   * @return the number of graphs spawned by the root communicator
   */
  size_t getRootTotalSubGraphsSpawned() {
    if (this->parentComm == nullptr) {
      return numGraphsSpawned;
    } else {
      return this->parentComm->getRootTotalSubGraphsSpawned();
    }
  }

  /**
   * Flattens the mapping between the addresses and task manager names and their connnectors.
   * Doing so allows for constant time look-up for the address to task name connectors
   * @param flattenedTaskNameConnectorMap the mapping between the address and manager names to their connects, which is shared among all communicators.
   * @note this function is called prior to spawning threads for the task graph communicators
   */
  void processFlattenTaskNameConnectorMap(std::shared_ptr<std::unordered_multimap<std::string,
                                                                                  std::shared_ptr<AnyConnector>>> flattenedTaskNameConnectorMap) {
    this->flattenedTaskNameConnectorMap = flattenedTaskNameConnectorMap;

    for (auto nameConnectorPair : *this->taskNameConnectorMap) {
      this->flattenedTaskNameConnectorMap->insert(nameConnectorPair);
    }

    // Send to children
    if (this->children->size() > 0) {
      for (auto child : *this->children) {
        child.second->processFlattenTaskNameConnectorMap(this->flattenedTaskNameConnectorMap);
      }
    }

  }

  /**
   * Adds the mapping between a task's address and its name to the input connector for that task.
   *
   * This will add all of these mappings to this task graph communicator. After which the parent
   * communicator increments the number of communicator graphs. If this is called by the parent
   * communicator, then it will check if it is ready to spawn threads
   *
   * @param o
   */
  void addTaskNameConnectorMap(std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>> *o) {
    for (auto nameConnectorPair : *o) {
      taskNameConnectorMap->insert(nameConnectorPair);
    }

    // Ignore the root, as we only care about sub graphs.
    if (this->parentComm != nullptr) {
      incrementRootCommunicatorGraphs();
    } else {
      checkRootSpawnThreads();
    }

//    std::cout << "Adding graph connectors for graph " << this->address << " Total count = "
//              << this->getRootNumGraphsReceived() << " out of " << this->getRootTotalSubGraphsSpawned() << std::endl;

  }

  /**
   * Adds a child communicator for this task graph communicator
   * @param comm the child communicator
   */
  void addChild(TaskGraphCommunicator *comm) {
//    std::cout << "Adding child: " << comm->getAddress() << " to " << this->address << std::endl;
    this->mutex.lock();
    children->insert(TaskCommPair(comm->getAddress(), comm));
    this->mutex.unlock();
  }

  /**
   * Gets the address of the task graph communicator.
   *
   * This matches the address of the task graph that owns the communicator.
   * @return the address
   */
  std::string getAddress() const {
    return address;
  }

  /**
   * Gracefully terminates the task graph communicator thread.
   */
  void terminateGracefully() {
    if (this->thread != nullptr) {
      this->dataQueue.Enqueue(nullptr);
      this->thread->join();
    }
  }

  /**
   * Main run function for the thread, which processes data packets until it is terminated.
   */
  void run() {
    while (!terminated) {
      this->processDataPacket();
    }
  }

  /**
   * Produces data packet to be processed for the task graph communicator
   * @param data the data
   * @note this function is thread safe.
   */
  void produceDataPacket(std::shared_ptr<DataPacket> data) {
    this->dataQueue.Enqueue(data);
  }

  /**
   * Processes one data packet.
   * If the data packet is nullptr, then the thread will be terminated.
   *
   * If there are multiple entries that share the same address and task name, then an error is produced.
   * Every task must have a unique name if the communicator is to be used.
   */
  void processDataPacket() {
    auto packet = dataQueue.Dequeue();

//    std::cout << "Received data packet: "<< packet << std::endl;
    if (packet == nullptr) {
      terminated = true;
      return;
    }

    std::string endPoint = packet->getDestAddr() + ":" + packet->getDestName();

    // Get connector
    size_t numItems = flattenedTaskNameConnectorMap->count(endPoint);

    if (numItems == 1) {
      auto connIterator = flattenedTaskNameConnectorMap->find(endPoint);

      // Gets the connector for the end point
      auto endPointConnector = connIterator->second;

      // Add data
      endPointConnector->produceAnyData(packet->getData());

    } else {
      if (numItems == 0)
        std::cerr << "Graph is unable to find destination task name: '" << endPoint
                  << "'. Make sure the task's name exists within the graph. Origin: " << packet->getOriginAddr() << ":"
                  << packet->getOriginName() << std::endl;
      else
        std::cerr << "Graph has tasks with duplicate name: '" << endPoint
                  << "' to send data between tasks, each task should have a unique name! Origin: "
                  << packet->getOriginAddr() << ":" << packet->getOriginName() << std::endl;

    }
  }

  /**
   * Gets whether the task communicator is terminated or not
   * @return true if the communicator is terminated, otherwise false
   * @retval TRUE if the communicator is terminated
   * @retval FALSE if the communicator is not terminated
   */
  bool isTerminated() { return this->terminated; }

 private:
  std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>>
      *taskNameConnectorMap; //!< The local mapping between the task graph communicator and its task graph.

  std::shared_ptr<std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>>>
      flattenedTaskNameConnectorMap; //!< The flattened mapping shared between all task graph communicators.

  TaskGraphCommunicator *parentComm; //!< The parent communicator (or nullptr if this is the root communicator).
  std::string address; //!< The address of the communicator.

  size_t numGraphsSpawned; //!< The number of graphs spawned.
  std::atomic_size_t numGraphsReceived; //!< The number of graphs received.

  TaskCommMap *children; //!< The children communicator.

  std::mutex mutex; //!< A mutex used to ensure thread safety.
  BlockingQueue<std::shared_ptr<DataPacket>> dataQueue; //!< The data queue to hold data packets.

  volatile bool terminated; //!< Flag used to indicate if the communicator is terminated or not.
  std::thread *thread; //!< The communicator thread.

};

}
#endif //HTGS_TASKGRAPHCOMMUNICATOR
