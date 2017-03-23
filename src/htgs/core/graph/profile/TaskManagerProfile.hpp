//
// Created by tjb3 on 3/22/17.
//

#ifndef HTGS_TASKMANAGERPROFILE_HPP
#define HTGS_TASKMANAGERPROFILE_HPP

#include <cstddef>
#include <ostream>
#include <htgs/types/TaskGraphDotGenFlags.hpp>
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
#ifdef PROFILE
    // TODO: Hide versus show

    if ((flags & DOTGEN_FLAG_HIDE_PROFILE_COMP_TIME) == 0)
      ret += "computeTime: " + std::to_string((double)computeTime/1000000.0) + " sec\n";

    if ((flags & DOTGEN_FLAG_HIDE_PROFILE_WAIT_TIME) == 0)
      ret += "waitTime: " + std::to_string((double)waitTime/1000000.0) + " sec\n";

    if ((flags & DOTGEN_FLAG_HIDE_PROFILE_MAX_Q_SZ) == 0)
      ret += "maxQueueSize: " + std::to_string(maxQueueSize) + "\n";

#endif
    return ret;
  }


  friend std::ostream &operator<<(std::ostream &os, const TaskManagerProfile &profile) {
    os << "computeTime: " << profile.computeTime << " waitTime: " << profile.waitTime << " maxQueueSize: "
       << profile.maxQueueSize;
    return os;
  }

  double getValue(int colorFlag)
  {
    if (colorFlag == DOTGEN_COLOR_COMP_TIME)
      return computeTime;
    else if (colorFlag == DOTGEN_COLOR_WAIT_TIME)
      return waitTime;
    else if (colorFlag == DOTGEN_COLOR_MAX_Q_SZ)
      return maxQueueSize;
    else
      return 0.0;

  }

  unsigned long long int getComputeTime() const {
    return computeTime;
  }

  unsigned long long int getWaitTime() const {
    return waitTime;
  }

  size_t getMaxQueueSize() const {
    return maxQueueSize;
  }

  void sum(TaskManagerProfile *other)
  {
    this->computeTime += other->getComputeTime();
    this->waitTime += other->getWaitTime();
  }

  void average(int sum)
  {
    this->computeTime = (unsigned long long int)(this->computeTime / (double)sum);
    this->computeTime = (unsigned long long int)(this->waitTime / (double)sum);
  }

 private:
  unsigned long long int computeTime;
  unsigned long long int waitTime;
  size_t maxQueueSize;

};


}



#endif //HTGS_TASKMANAGERPROFILE_HPP
