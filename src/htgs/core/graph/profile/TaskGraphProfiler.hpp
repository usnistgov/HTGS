// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 3/22/17.
//

/**
 * @file TaskGraphProfiler.hpp
 * @author Timothy Blattner
 * @date March 22, 2017
 *
 * @brief Implements the task graph profiler for gathering and communicating the results via graphviz.
 */

#ifndef HTGS_TASKGRAPHPROFILER_HPP
#define HTGS_TASKGRAPHPROFILER_HPP

#include "TaskManagerProfile.hpp"
#include <htgs/core/graph/AnyTaskGraphConf.hpp>
#include <set>
namespace htgs {

/**
 * @class TaskGraphProfiler TaskGraphProfiler.hpp <htgs/core/graph/profile/TaskGraphProfiler.hpp>
 * @brief The task graph profiler that gathers profile data and communicates via graphviz.
 *
 * A TaskGraphConf uses this class to gather all profile data for visually outputting the task graph
 * as a dot file. DOTGEN flags are used to specify options to enable/disable features for the graph.
 *
 *
 * @note To enable profiling you must add the directive PROFILE prior to compilation. If PROFILE is
 * not defined, then a basic visualization is done showing just the graph structure.
 *
 */
class TaskGraphProfiler {
 public:

  /**
   * Constructs the task graph profiler.
   * @param flags the DOTGEN flags to enable/disable features
   */
  TaskGraphProfiler(int flags) : flags(flags) {
    taskManagerProfiles = new std::map<AnyTaskManager *, TaskManagerProfile *>();
  }

  /**
   * Destructor
   */
  ~TaskGraphProfiler() {
    for (auto v : *taskManagerProfiles) {
      delete v.second;
      v.second = nullptr;
    }

    delete taskManagerProfiles;
    taskManagerProfiles = nullptr;
  }

  /**
   * Builds a profile for the graph, (called after execution is done)
   * @param graphConf the graph that is profiled
   */
  void buildProfile(AnyTaskGraphConf *graphConf) {
    graphConf->gatherProfilingData(taskManagerProfiles);
  }

  /**
   * Prints the profile data to console.
   */
  void printProfiles() {
    for (auto t : *taskManagerProfiles) {
      std::cout << t.first->getName() << " addr: " << t.first->getAddress() << " id: " << t.first->getThreadId()
                << " Profile: " << *t.second << std::endl;
    }
  }

  /**
   * Generates the dot profile for the graph.
   *
   * Only the tasks that have been defined within the current dot graph will have their
   * profiles included. The color flag is used to identify which profiling to use when coloring
   * the nodes.
   *
   * @param curDotGraph the current dot graph that includes all tasks and edges used for the graph.
   * @param colorFlag specifies which profile data to use when generating the color map, 0 = no color map
   * @return the dot graph with labeling for profiling if profiling is enabled.
   * @note The directive PROFILE must be defined to enable outputting profile data.
   */
  std::string genDotProfile(std::string curDotGraph, int colorFlag) {
    std::string ret = "";

    // If all threading is disabled, then compute the averages only, based on first thread
    if ((flags & DOTGEN_FLAG_SHOW_ALL_THREADING) == 0) {
      computeAverages();
    }

    bool useColorMap = false;
    std::unordered_map<std::string, std::string> *colorMap = nullptr;
    if (colorFlag != 0) {
      useColorMap = true;
      colorMap = genColorMap(colorFlag);
    }

    for (auto t : *taskManagerProfiles) {
      auto tMan = t.first;
      auto tProfile = t.second;

      auto tFun = tMan->getTaskFunction();

      std::string dotId = tMan->getTaskFunction()->getDotId();

      if (curDotGraph.find(dotId + ";") != std::string::npos) {

        std::string inOutLabel =
            (((flags & DOTGEN_FLAG_SHOW_IN_OUT_TYPES) != 0) ? ("\nin: " + tFun->inTypeName() + "\nout: "
                + tFun->outTypeName()) : "");
        std::string threadLabel =
            (((flags & DOTGEN_FLAG_SHOW_ALL_THREADING) != 0) ? "" : (" x" + std::to_string(tFun->getNumThreads())));
        ret += dotId + "[label=\"" + tFun->getDotLabelName() +
            (tFun->debugDotNode() != "" ? ("\n" + tFun->debugDotNode() + "\n") : "") +
            threadLabel + inOutLabel + "\n" +
            tProfile->genDot(flags) +
            (tFun->getDotCustomProfile() != "" ? ("\n" + tFun->getDotCustomProfile() + "\n") : "") +
            "\",shape=" + tFun->getDotShape() +
            ",style=filled" +
            ",fillcolor=" + tFun->getDotFillColor()
            + (useColorMap ? ",penwidth=5,color=\"" + colorMap->at(dotId) + "\"" : ", color="
                + tFun->getDotShapeColor())
            + ",width=.2,height=.2];\n";
      }
    }

    delete colorMap;
    colorMap = nullptr;

    return ret;
  }

 private:

  /**
   * Computes the averages for all profile data.
   */
  void computeAverages() {
    std::map<AnyTaskManager *, TaskManagerProfile *> finalProfiles;
    // Address + name + thread ID = unique

    // Group using Address + name ... thread ID = 0 is final version

    std::multimap<std::string, std::pair<AnyTaskManager *, TaskManagerProfile *>> averageMap;

    std::set<std::string> keys;

    // Gather multimap
    for (auto t : *taskManagerProfiles) {
      auto tMan = t.first;
      std::string key = tMan->getAddress() + tMan->getName();
      keys.insert(key);
      averageMap.insert(std::pair<std::string, std::pair<AnyTaskManager *, TaskManagerProfile *>>(key, t));
    }

    // Loop through each key
    for (auto key : keys) {
      auto valRange = averageMap.equal_range(key);

      AnyTaskManager *mainManager = nullptr;
      TaskManagerProfile *finalProfile = nullptr;

      int count = 0;
      for (auto i = valRange.first; i != valRange.second; ++i) {
        auto profilePair = (*i).second;

        if (finalProfile == nullptr) {
          finalProfile = profilePair.second;
        } else {
          finalProfile->sum(profilePair.second);
        }

        if (profilePair.first->getThreadId() == 0) {
          mainManager = profilePair.first;
        } else {
          delete profilePair.second;
          profilePair.second = nullptr;
        }
        count++;
      }

      if (finalProfile != nullptr && mainManager != nullptr) {
        finalProfile->average(count);
        finalProfiles.insert(std::pair<AnyTaskManager *, TaskManagerProfile *>(mainManager, finalProfile));
      } else {
        std::cout << "Something screwy happened . . ." << std::endl;
      }
    }

    taskManagerProfiles->clear();

    for (auto t : finalProfiles) {
      taskManagerProfiles->insert(t);
    }

  }

  /**
   * Generates the color map.
   *
   * The map is structured as TaskDotID string -> Color string.
   *
   * @param colorFlag selects which profile data to use when generating colors.
   * @return the color map
   */
  std::unordered_map<std::string, std::string> *genColorMap(int colorFlag) {
    std::unordered_map<std::string, std::string> *colorMap = new std::unordered_map<std::string, std::string>();

    int rColor[10] = {0, 0, 0, 0, 85, 170, 255, 255, 255, 255};
    int gColor[10] = {0, 85, 170, 255, 255, 255, 255, 170, 85, 0};
    int bColor[10] = {255, 255, 255, 255, 170, 85, 0, 0, 0, 0};

    std::deque<double> vals;
    double maxTime = 0.0;
    double totalTime = 0.0;
    for (auto v : *taskManagerProfiles) {
      double val = v.second->getValue(colorFlag);
      if (val > 0) {
        totalTime += val;
      }

      if (maxTime < val)
        maxTime = val;
    }

    for (auto v : *taskManagerProfiles) {
      if (v.second->getValue(colorFlag) == 0.0) {
        colorMap->insert(std::pair<std::string, std::string>(v.first->getTaskFunction()->getDotId(), "black"));
        continue;
      }

      int red = 0;
      int green = 0;
      int blue = 0;

      // compute percentage of totalTime
      int perc = (int) (v.second->getValue(colorFlag) / maxTime * 100.0);

      if (perc % 10 != 0)
        perc = perc + 10 - (perc % 10);

      int index = (perc / 10);

      if (index < 0)
        index = 0;

      if (index >= 10)
        index = 9;

      red = rColor[index];
      green = gColor[index];
      blue = bColor[index];

      char hexcol[16];

      snprintf(hexcol, sizeof(hexcol), "%02x%02x%02x", red & 0xff, green & 0xff, blue & 0xff);
      std::string color(hexcol);
      color = "#" + color;

      colorMap->insert(std::pair<std::string, std::string>(v.first->getTaskFunction()->getDotId(), color));
    }

    return colorMap;
  }

  std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles; //!< The profile data for all task managers
  int flags; //!< The DOTGEN bit flags
};
}

#endif //HTGS_TASKGRAPHPROFILER_HPP
