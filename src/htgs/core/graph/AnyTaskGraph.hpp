//
// Created by tjb3 on 2/24/17.
//

#ifndef HTGS_ANYTASKGRAPH_HPP
#define HTGS_ANYTASKGRAPH_HPP

#include <list>
#include "../task/AnyTaskScheduler.hpp"

namespace htgs {

class AnyTaskGraph {
 public:

  /**
  * Pure virtual function to get the vertices of the TaskGraph
  * @return the vertices of the TaskGraph
  */
  virtual std::list<AnyTaskScheduler *> *getVertices() const = 0;

  /**
   * Pure virtual function to add a copy of a TaskScheduler into the TaskGraph.
   * @param taskCopy the task that was copied.
   */
  virtual void addTaskCopy(AnyTaskScheduler *taskCopy) = 0;

  /**
   * Writes the dot representation of the task graph to disk
   * @param file the file path (will not create directories)
   */
  virtual void writeDotToFile(std::string file) = 0;

  /**
   * Writes the dot representation of the task graph to disk
   * @param file the file path (will not create directories)
   * @param dotGenFlags the various dot file generation flags to use
   */
  virtual void writeDotToFile(std::string file, int dotGenFlags) = 0;

 private:




};

}

#endif //HTGS_ANYTASKGRAPH_HPP
