//
// Created by tjb3 on 3/3/17.
//

#ifndef HTGS_GRAPHCOMMUNICATORTHREAD_HPP
#define HTGS_GRAPHCOMMUNICATORTHREAD_HPP
#include <htgs/core/queue/BlockingQueue.hpp>
#include <htgs/api/TaskGraphConf.hpp>
#include "DataPacket.hpp"
namespace htgs {
class GraphCommunicatorThread {
 public:

  void run()
  {
    while(true) {
      std::shared_ptr<DataPacket> data = dataQueue->Dequeue();

      // If nullptr data is received, then things are terminating
      if (data == nullptr)
        break;

      // Check graph pipeline Id
//      if (data->getDestPipelineId() == graph->getPipelineId())
//      {
        // If they match, then check for named task

        // If no match is found for the named task, then
//      }

      // Check graph for named task



    }
  }

 private:

  // Have a thread safe queue holding pointer to things . . .
  BlockingQueue <std::shared_ptr<DataPacket>> *dataQueue;
  TaskGraphConf *graph;

};
}
#endif //HTGS_GRAPHCOMMUNICATORTHREAD_HPP
