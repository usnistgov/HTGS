//
// Created by tjb3 on 3/22/17.
//

#ifndef HTGS_TASKGRAPHPROFILER_HPP
#define HTGS_TASKGRAPHPROFILER_HPP

#include "TaskManagerProfile.hpp"
#include <htgs/core/graph/AnyTaskGraphConf.hpp>
#include <set>
namespace htgs {
class TaskGraphProfiler {
 public:

  /**
   * Constructs the task graph profiler.
   */
  TaskGraphProfiler(int flags) : flags(flags)
  {
    taskManagerProfiles = new std::map<AnyTaskManager *, TaskManagerProfile *>();
  }

  ~TaskGraphProfiler() {
    for (auto v : *taskManagerProfiles)
    {
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
  void buildProfile(AnyTaskGraphConf *graphConf)
  {
    graphConf->gatherProfilingData(taskManagerProfiles);
  }

  void printProfiles() {
    for (auto t : *taskManagerProfiles)
    {
      std::cout << t.first->getName() << " addr: " << t.first->getAddress() << " id: " <<  t.first->getThreadId() << " Profile: " << *t.second << std::endl;
    }
  }

  std::string genDotProfile(std::string curGraph, int colorFlag)
  {
    std::string ret = "";

    // If all threading is disabled, then compute the averages only, based on first thread
    if ((flags & DOTGEN_FLAG_SHOW_ALL_THREADING) == 0)
    {
      computeAverages();
    }

    bool useColorMap = false;
    std::unordered_map<std::string, std::string> *colorMap = nullptr;
    if (colorFlag != 0)
    {
      useColorMap = true;
      colorMap = genColorMap(colorFlag);
    }

    for (auto t : *taskManagerProfiles)
    {
      auto tMan = t.first;
      auto tProfile = t.second;

      auto tFun = tMan->getTaskFunction();

      std::string dotId = tMan->getTaskFunction()->getDotId();

      if (curGraph.find(dotId + ";") != std::string::npos) {

        std::string inOutLabel = (((flags & DOTGEN_FLAG_SHOW_IN_OUT_TYPES) != 0) ? ("\nin: "+ tFun->inTypeName() + "\nout: " + tFun->outTypeName()) : "");
        std::string threadLabel = (((flags & DOTGEN_FLAG_SHOW_ALL_THREADING) != 0) ? "" : (" x" + std::to_string(tFun->getNumThreads())));
        ret += dotId + "[label=\"" + tFun->getDotLabelName() +
            (tFun->debugDotNode() != "" ? ("\n"+tFun->debugDotNode()+"\n") : "") +
            threadLabel + inOutLabel + "\n" +
            tProfile->genDot(flags) +
            "\",shape=" + tFun->getDotShape()//
            + (useColorMap ? ",style=filled,penwidth=5,fillcolor=white,color=\"" + colorMap->at(dotId) + "\"" : ", color=" + tFun->getDotShapeColor())
            +",width=.2,height=.2];\n";
      }
    }

    delete colorMap;
    colorMap = nullptr;

    return ret;
  }

 private:
  void computeAverages()
  {
    std::map<AnyTaskManager *, TaskManagerProfile *> finalProfiles;
    // Address + name + thread ID = unique

    // Group using Address + name ... thread ID = 0 is final version

    std::multimap<std::string, std::pair<AnyTaskManager *, TaskManagerProfile *>> averageMap;

    std::set<std::string> keys;

    // Gather multimap
    for (auto t : *taskManagerProfiles)
    {
      auto tMan = t.first;
      std::string key = tMan->getAddress() + tMan->getName();
      keys.insert(key);
      averageMap.insert(std::pair<std::string, std::pair<AnyTaskManager *, TaskManagerProfile *>>(key, t));
    }

    // Loop through each key
    for (auto key : keys)
    {
      auto valRange = averageMap.equal_range(key);

      AnyTaskManager *mainManager = nullptr;
      TaskManagerProfile *finalProfile = nullptr;

      int count = 0;
      for (auto i = valRange.first; i != valRange.second; ++i)
      {
        auto profilePair = (*i).second;

        if (finalProfile == nullptr)
        {
          finalProfile = profilePair.second;
        }
        else
        {
          finalProfile->sum(profilePair.second);
        }

        if (profilePair.first->getThreadId() == 0)
        {
          mainManager = profilePair.first;
        }
        count++;
      }

      if (finalProfile != nullptr && mainManager != nullptr) {
        finalProfile->average(count);
        finalProfiles.insert(std::pair<AnyTaskManager *, TaskManagerProfile *>(mainManager, finalProfile));
      }
      else
      {
        std::cout << "Something screwy happened . . ." << std::endl;
      }
    }

    taskManagerProfiles->clear();

    for (auto t : finalProfiles)
    {
      taskManagerProfiles->insert(t);
    }

  }

  std::unordered_map<std::string, std::string> *genColorMap(int colorFlag)
  {
    std::unordered_map<std::string, std::string> *colorMap = new std::unordered_map<std::string, std::string>();

    int rColor[10] = {0,0,0,0,85,170,255,255,255,255};
    int gColor[10] = {0,85,170,255,255,255,255,170,85,0};
    int bColor[10] = {255,255,255,255,170,85,0,0,0,0};

    std::deque<double> vals;
    double maxTime = 0.0;
    double totalTime = 0.0;
    for (auto v : *taskManagerProfiles)
    {
      double val = v.second->getValue(colorFlag);
      if (val > 0) {
        totalTime += val;
      }

      if (maxTime < val)
        maxTime = val;
    }

    for (auto v : *taskManagerProfiles)
    {
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

      snprintf(hexcol, sizeof(hexcol), "%02x%02x%02x", red & 0xff, green & 0xff ,blue & 0xff);
      std::string color(hexcol);
      color = "#" + color;

      colorMap->insert(std::pair<std::string, std::string>(v.first->getTaskFunction()->getDotId(), color));
    }

    return colorMap;
  }


  std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles;
  int flags;
};
}

#endif //HTGS_TASKGRAPHPROFILER_HPP
