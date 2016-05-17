
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by Hobbs on 2/15/2016.
//

#ifndef HTGS_SIMPLECLOCK_H
#define HTGS_SIMPLECLOCK_H


#include <chrono>

enum class TimeVal {
  MILLI,
  NANO,
  SEC
};

class SimpleClock {

 public:

  SimpleClock() {
    duration = 0;
    count = 0;
  }
  void start() {
    startTime = std::chrono::high_resolution_clock::now();
  }

  void stopAndIncrement() {
    stop();
    incrementDuration();
  }

  void stop() {
    endTime = std::chrono::high_resolution_clock::now();
  }

  void incrementDuration() {
    this->duration = this->duration + std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
    count = count + 1;
  }

  long long int getDuration() {
    return duration;
  }

  long getCount() {
    return count;
  }

  double getAverageTime(TimeVal val) {
    double avg = (double) this->getDuration() / (double) this->getCount();

    switch (val) {
      case TimeVal::MILLI:
        return avg / 1000000;
      case TimeVal::NANO:
        return avg;
      case TimeVal::SEC:
        return avg / 1000000000;
    }
  }

 private:
  std::chrono::time_point<std::chrono::system_clock> startTime;
  std::chrono::time_point<std::chrono::system_clock> endTime;

  long long int duration;
  long count;
};


#endif //HTGS_SIMPLECLOCK_H
