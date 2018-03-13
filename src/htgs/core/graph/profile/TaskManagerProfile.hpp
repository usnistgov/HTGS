// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file TaskManagerProfile.hpp
 * @author Timothy Blattner
 * @date March 22, 2017
 *
 * @brief Implements the TaskManagerProfile class that is used to gather profile data for a task manager.
 */

#ifndef HTGS_TASKMANAGERPROFILE_HPP
#define HTGS_TASKMANAGERPROFILE_HPP

#include <cstddef>
#include <ostream>
#include <htgs/types/TaskGraphDotGenFlags.hpp>
namespace htgs {

/**
 * @class TaskManagerProfile TaskManagerProfile.hpp <htgs/core/graph/profile/TaskManagerProfile.hpp>
 * @brief Implements a task manager profile that holds profiling data for a task manager.
 *
 * @note Add the PROFILE directive during compilation to enable profiling
 */
class TaskManagerProfile {
 public:

  /**
   * Constructs a task manager profile with no profiling data.
   */
  TaskManagerProfile() {
    computeTime = 0;
    waitTime = 0;
    maxQueueSize = 0;
    memoryWaitTime = 0;
  }

  /**
   * Constructs a task manager profile with profiling data
   * @param computeTime the compute time
   * @param waitTime the wait time
   * @param maxQueueSize the max queue size
   * @param memoryWaitTime the amount of time spent waiting for data from a memory manager
   */
  TaskManagerProfile(unsigned long long int computeTime, unsigned long long int waitTime, size_t maxQueueSize, unsigned long long int memoryWaitTime)
      : computeTime(computeTime), waitTime(waitTime), maxQueueSize(maxQueueSize), memoryWaitTime(memoryWaitTime) {}

  /**
   * Generates the dot contents for the task manager profile. The flags control which
   * profiling data to add or hide.
   * @param flags The DOTGEN flags to control profiling timings that are to be hidden.
   * @return the profiling data for dot graphviz.
   */
  std::string genDot(int flags) {
    std::string ret = "";
#ifdef PROFILE
    if ((flags & DOTGEN_FLAG_HIDE_PROFILE_COMP_TIME) == 0)
      ret += "computeTime: " + std::to_string((double)computeTime/1000000.0) + " s\n";

    if ((flags & DOTGEN_FLAG_HIDE_PROFILE_WAIT_TIME) == 0)
      ret += "waitTime: " + std::to_string((double)waitTime/1000000.0) + " s\n";

    if ((flags & DOTGEN_FLAG_HIDE_PROFILE_MAX_Q_SZ) == 0)
      ret += "maxQueueSize: " + std::to_string(maxQueueSize) + "\n";

    if ((flags & DOTGEN_FLAG_HIDE_MEMORY_WAIT_TIME) == 0 && memoryWaitTime > 0)
      ret += "memoryWaitTime: " + std::to_string((double)memoryWaitTime/1000000.0) + " sec\n";
#endif
    return ret;
  }

  /**
   * Output stream operator to output the task manager profile to a stream.
   * @param os the output stream
   * @param profile the profile to output
   * @return the output stream
   */
  friend std::ostream &operator<<(std::ostream &os, const TaskManagerProfile &profile) {
    os << "computeTime: " << profile.computeTime << " waitTime: " << profile.waitTime << " maxQueueSize: "
       << profile.maxQueueSize << (profile.memoryWaitTime == 0 ? "" : " memoryWaitTime: " + profile.memoryWaitTime);
    return os;
  }

  /**
   * Gets one of the values based on the DOTGEN color flag.
   * @param colorFlag the DOTGEN flag used to control which value to output
   * @return the profile value
   */
  double getValue(int colorFlag) {
    if (colorFlag == DOTGEN_COLOR_COMP_TIME)
      return (double)computeTime;
    else if (colorFlag == DOTGEN_COLOR_WAIT_TIME)
      return (double)waitTime;
    else if (colorFlag == DOTGEN_COLOR_MAX_Q_SZ)
      return (double)maxQueueSize;
    else if (colorFlag == DOTGEN_COLOR_MEMORY_WAIT_TIME)
      return (double)memoryWaitTime;
    else
      return 0.0;

  }

  /**
   * Gets the compute time
   * @return the compute time
   */
  unsigned long long int getComputeTime() const {
    return computeTime;
  }

  /**
   * Gets the wait time
   * @return the wait time
   */
  unsigned long long int getWaitTime() const {
    return waitTime;
  }

  /**
   * Gets the maximum queue size
   * @return the maximum queue size
   */
  size_t getMaxQueueSize() const {
    return maxQueueSize;
  }

  /**
   * Gets the memory wait time
   * @return the amount of time the task is waiting for memory
   */
  unsigned long long int getMemoryWaitTime() const {
    return memoryWaitTime;
  }

  /**
   * Computes the sum for the compute time and wait time between this profile and some other profile.
   * This is used when computing the average compute/wait time among multiple task managers.
   * @param other the other task manager
   */
  void sum(TaskManagerProfile *other) {
    this->computeTime += other->getComputeTime();
    this->waitTime += other->getWaitTime();
    this->memoryWaitTime += other->getMemoryWaitTime();
  }

  /**
   * Sets the max queue size for the profile
   * @param maxQueueSize the max queue size
   */
  void setMaxQueueSize(size_t maxQueueSize) { this->maxQueueSize = maxQueueSize; }

  /**
   * Computes the average compute and wait time for the profile
   * @param count the number of items to divide by
   */
  void average(int count) {
    this->computeTime = (unsigned long long int) (this->computeTime / (double) count);
    this->waitTime = (unsigned long long int) (this->waitTime / (double) count);
    this->memoryWaitTime = (unsigned long long int) (this->memoryWaitTime / (double) count);
  }

 private:
  unsigned long long int computeTime; //!< The compute time for the task manager
  unsigned long long int waitTime; //!< The wait time for the task manager
  unsigned long long int memoryWaitTime; //!< The time spent waiting for memory from the memory manager
  size_t maxQueueSize; //!< The maximum queue size for the task manager

};
}
#endif //HTGS_TASKMANAGERPROFILE_HPP
