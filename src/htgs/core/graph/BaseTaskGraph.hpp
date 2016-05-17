
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file BaseTaskGraph.hpp
 * @author Timothy Blattner
 * @date Nov 25, 2015
 *
 * @brief Creates a parent TaskGraph that removes the template arguments.
 */
#ifndef HTGS_BASETASKGRAPH_H
#define HTGS_BASETASKGRAPH_H

#include "../task/BaseTaskScheduler.hpp"

namespace htgs {
/**
 * @class BaseTaskGraph BaseTaskGraph.hpp <htgs/core/graph/BaseTaskGraph.hpp>
 * @brief Creates a parent TaskGraph that removes the template arguments.
 *
 */
class BaseTaskGraph {
 public:
  /**
   * Virtual destructor
   */
  virtual ~BaseTaskGraph() { }

  /**
   * Pure virtual function to get the vertices of the TaskGraph
   * @return the vertices of the TaskGraph
   */
  virtual std::list<BaseTaskScheduler *> *getVertices() const = 0;

  /**
   * Pure virtual function to add a copy of a TaskScheduler into the TaskGraph.
   * @param taskCopy the task that was copied.
   */
  virtual void addTaskCopy(BaseTaskScheduler *taskCopy) = 0;

  /**
   * Writes the dot representation of the task graph to disk
   * @param file the file path (will not create directories)
   */
  virtual void writeDotToFile(std::string file) = 0;

};


}

#endif //HTGS_BASETASKGRAPH_H
