//
// Created by tjb3 on 3/3/17.
//

#ifndef HTGS_TASKGRAPHCOMMUNICATOR
#define HTGS_TASKGRAPHCOMMUNICATOR

#include <unordered_map>
#include <htgs/core/graph/AnyConnector.hpp>
#include <htgs/core/queue/BlockingQueue.hpp>
#include "DataPacket.hpp"

namespace htgs {

class TaskGraphCommunicator;


//typedef std::unordered_map<std::string, AnyTaskGraphConf *> TaskGraphAddressMap;

//typedef std::pair<std::string, AnyTaskGraphConf *> TaskGraphAddressPair;

typedef std::unordered_map<std::string, TaskGraphCommunicator *> TaskCommMap;

typedef std::pair<std::string, TaskGraphCommunicator *> TaskCommPair;

class TaskGraphCommunicator {
 public:

  TaskGraphCommunicator(TaskGraphCommunicator *parent, std::string address)
      : parentComm(parent), address(address)
  {

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

  void rootSpawnThreads()
  {
    // Validate this is root
    if (this->parentComm == nullptr)
    {
      // Flatten lookup table for parent and children
      this->flattenedTaskNameConnectorMap = std::shared_ptr<std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>>>(new std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>>());

      this->processFlattenTaskNameConnectorMap(this->flattenedTaskNameConnectorMap);

      this->spawnChildrenThreads();
    }
  }

  void spawnChildrenThreads()
  {
    this->spawnThread();

    if (this->children->size() > 0) {
      for (auto commChild : *this->children) {
        commChild.second->spawnChildrenThreads();
      }
    }
  }

  void spawnThread()
  {
    this->thread = new std::thread(&TaskGraphCommunicator::run, this);
  }

  void setNumGraphsSpawned(size_t numGraphsSpawned) { this->numGraphsSpawned = numGraphsSpawned; }

  void printParents(std::string prefix)
  {
//    std::cout << "Num graphs spawned = " << numGraphsSpawned << " Graphs received: " << numGraphsReceived << std::endl;
    std::cout << prefix << "Address = " << this->address << std::endl;

    if (this->getParentComm() == nullptr)
    {
      std::cout << std::endl << "=====DONE=====" << std::endl;
      return;
    }

    this->getParentComm()->printParents(prefix +"\t\t");

  }

  void printTree(TaskGraphCommunicator *comm, std::string prefix)
  {
    std::cout << "Num graphs spawned = " << numGraphsSpawned << " Graphs received: " << numGraphsReceived << std::endl;
    if (comm->getParentComm() == nullptr)
    {
      std::cout << "PARENT addr: " << comm->getAddress() << std::endl;
    } else{
      std::cout << "Parent address = " << comm->getParentComm()->getAddress() << std::endl;
    }

    std::cout << prefix << "Num children: " << comm->getChildren()->size() << std::endl;
    for (auto child : *comm->getChildren())
    {
      std::cout << prefix << " CHILD addr: " << child.first << std::endl;
      printTree(child.second, prefix+"\t");
    }

  }

  TaskGraphCommunicator *getParentComm() const {
    return parentComm;
  }

  TaskCommMap *getChildren() const {
    return children;
  }

  void incrementRootGraph()
  {
    // Check if this is the root
    if (this->parentComm == nullptr)
    {
      numGraphsReceived++;

      // If all the graphs have produced their updates, then begin the communication threads
      if (numGraphsReceived == numGraphsSpawned)
      {
        this->rootSpawnThreads();
      }

    }
    else
    {
      this->parentComm->incrementRootGraph();
    }
  }

  size_t getRootNumGraphsReceived()
  {
    if (this->parentComm == nullptr)
    {
      return numGraphsReceived;
    }
    else
    {
      return this->parentComm->getRootNumGraphsReceived();
    }
  }

  size_t getRootTotalSubGraphsSpawned()
  {
    if (this->parentComm == nullptr)
    {
      return numGraphsSpawned;
    }
    else
    {
      return this->parentComm->getRootTotalSubGraphsSpawned();
    }
  }

  void processFlattenTaskNameConnectorMap(std::shared_ptr<std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>>> flattenedTaskNameConnectorMap)
  {
    this->flattenedTaskNameConnectorMap = flattenedTaskNameConnectorMap;

    for (auto nameConnectorPair : *this->taskNameConnectorMap)
    {
      this->flattenedTaskNameConnectorMap->insert(nameConnectorPair);
    }

    // Send to children
    if (this->children->size() > 0)
    {
      for (auto child : *this->children)
      {
        child.second->processFlattenTaskNameConnectorMap(this->flattenedTaskNameConnectorMap);
      }
    }

  }

  void addTaskNameConnectorMap(std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>> *o)
  {
    for (auto nameConnectorPair : *o)
    {
      taskNameConnectorMap->insert(nameConnectorPair);
    }

    // Ignore the root, as we only care about sub graphs.
    if (this->parentComm != nullptr) {
      incrementRootGraph();
    }

//    std::cout << "Adding graph connectors for graph " << this->address << " Total count = "
//              << this->getRootNumGraphsReceived() << " out of " << this->getRootTotalSubGraphsSpawned() << std::endl;

  }

  void addChild(TaskGraphCommunicator *comm)
  {
//    std::cout << "Adding child: " << comm->getAddress() << " to " << this->address << std::endl;
    this->mutex.lock();
    children->insert(TaskCommPair(comm->getAddress(), comm));
    this->mutex.unlock();
  }

  std::string getAddress() const {
    return address;
  }

  void terminateGracefully()
  {
    if (this->thread != nullptr) {
      this->dataQueue.Enqueue(nullptr);
      this->thread->join();
    }
  }

  void run()
  {
    std::cout << "Thread running for " << this->getAddress() << std::endl;
    while(!terminated)
    {
      this->processDataPacket();
    }
  }

  void produceDataPacket(std::shared_ptr<DataPacket> data)
  {
    this->dataQueue.Enqueue(data);
  }


  void processDataPacket()
  {
    auto packet = dataQueue.Dequeue();

    if (packet == nullptr)
    {
      terminated = true;
      return;
    }

    std::string endPoint = packet->getDestAddr() + ":" + packet->getDestName();

    // Get connector
    size_t numItems = taskNameConnectorMap->count(endPoint);

    if (numItems == 1)
    {
      auto connIterator = taskNameConnectorMap->find(endPoint);

      // Gets the connector for the end point
      auto endPointConnector = connIterator->second;

      // Add data
      endPointConnector->produceAnyData(packet->getData());

    } else{
      std::cerr << "Graph has tasks with duplicate name: " << packet->getDestName() << " to send data between tasks, each task should have a unique name!" << std::endl;
    }
  }

  bool isTerminated() { return this->terminated; }

 private:
  std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>> *taskNameConnectorMap;

  std::shared_ptr<std::unordered_multimap<std::string, std::shared_ptr<AnyConnector>>> flattenedTaskNameConnectorMap;

  TaskGraphCommunicator *parentComm;
  std::string address;

  size_t numGraphsSpawned;
  std::atomic_size_t numGraphsReceived;

  TaskCommMap *children;

  std::mutex mutex;
  BlockingQueue <std::shared_ptr<DataPacket>> dataQueue;

  volatile bool terminated;

  std::thread *thread;

};

}
#endif //HTGS_TASKGRAPHCOMMUNICATOR
