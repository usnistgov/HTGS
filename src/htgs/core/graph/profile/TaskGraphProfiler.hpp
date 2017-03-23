//
// Created by tjb3 on 3/22/17.
//

#ifndef HTGS_TASKGRAPHPROFILER_HPP
#define HTGS_TASKGRAPHPROFILER_HPP

#include "TaskManagerProfile.hpp"
#include <htgs/core/graph/AnyTaskGraphConf.hpp>
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

  std::string genDotProfile(std::string curGraph)
  {
    // TODO: Generate color map ...

    std::string ret = "";
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
            "\",shape=" + tFun->getDotShape()
            +",color=" + tFun->getDotShapeColor()
            +",width=.2,height=.2];\n";

//        ret += dotId + "[xlabel=\"" + tProfile->genDot(flags) + "\"]" + "\n";
      }

    }

    return ret;
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


 private:
  // Store stats for a given task . . . aka have the info needed to build the profile from a graph

  // Map AnyTaskManager * -> Profile stats
  std::map<AnyTaskManager *, TaskManagerProfile *> *taskManagerProfiles;

  int flags;

  // Basic stats
  // task compute time
  // task wait time
  // Max Queue Size for task's connector











};
}

#endif //HTGS_TASKGRAPHPROFILER_HPP
