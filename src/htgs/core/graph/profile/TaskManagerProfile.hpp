//
// Created by tjb3 on 3/22/17.
//

#ifndef HTGS_TASKMANAGERPROFILE_HPP
#define HTGS_TASKMANAGERPROFILE_HPP

#include <cstddef>
#include <ostream>
namespace htgs {
class TaskManagerProfile {
 public:
  TaskManagerProfile()
  {
    computeTime = 0;
    waitTime = 0;
    maxQueueSize = 0;
  }

  TaskManagerProfile(unsigned long long int computeTime, unsigned long long int waitTime, size_t maxQueueSize)
      : computeTime(computeTime), waitTime(waitTime), maxQueueSize(maxQueueSize) {}


  std::string genDot(int flags)
  {
    std::string ret = "";
    ret += "computeTime: " + std::to_string((double)computeTime/1000000.0) + " sec\n";
    ret += "waitTime: " + std::to_string((double)waitTime/1000000.0) + " sec\n";
    ret += "maxQueueSize: " + std::to_string(maxQueueSize) + "\n";

    return ret;
  }


  friend std::ostream &operator<<(std::ostream &os, const TaskManagerProfile &profile) {
    os << "computeTime: " << profile.computeTime << " waitTime: " << profile.waitTime << " maxQueueSize: "
       << profile.maxQueueSize;
    return os;
  }

 private:
  unsigned long long int computeTime;
  unsigned long long int waitTime;
  size_t maxQueueSize;

};


}



#endif //HTGS_TASKMANAGERPROFILE_HPP
