// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 2/24/17.
//
/**
 * @file EdgeDescriptor.hpp
 * @author Timothy Blattner
 * @date February 24, 2017
 * @brief Implements the edge descriptor interface to build edges for a task graph.
 */

#ifndef HTGS_EDGEDESCRIPTOR_HPP
#define HTGS_EDGEDESCRIPTOR_HPP

#include <cstddef>
#include <htgs/core/graph/AnyTaskGraphConf.hpp>

namespace htgs {

class AnyTaskGraphConf;

/**
 * @class EdgeDescriptor EdgeDescriptor.hpp <htgs/core/graph/edge/EdgeDescriptor.hpp>
 * @brief The edge descriptor is an interface used to describe how an edge is applied and copied
 * to a task graph.
 *
 * The edge descriptor is purely an interface that only contains two functions: (1) applyEdge, and
 * (2) copy.
 *
 * applyEdge adds the edge to a task graph, which is supplied within its parameter.
 *
 * copy is used to create a copy for a particular task graph. The task graph has helper functions to get copies from that
 * task graph to ensure the proper instances are generated for tasks being added.
 *
 */
class EdgeDescriptor {
 public:
  /**
   * Destructor
   */
  virtual ~EdgeDescriptor() {}

  /**
   * Applies an edge to a task graph.
   *
   * An edge is added to the supplied task graph. Each ITask is obtains a task manager that is to be used to manage the
   * ITask. The graph has helper function AnyTaskGraphConf::getTaskManager. This is used to get the correct task manager
   * that is to be used for a given ITask.
   *
   * @param graph the task graph configuration that the edge is applied to.
   */
  virtual void applyEdge(AnyTaskGraphConf *graph) = 0;

  /**
   * Creates a copy of the edge descriptor to be added to other graphs, such as those within execution pipelines.
   *
   * The edge descriptor typically has ITasks that are added to a TaskManager, which is then added to the task graph.
   * The copy function is used to copy the ITasks and any other meta data that is needed for applying the edge. The implementation
   * should use the AnyTaskGraphConf::getCopy function to get copies of the ITask.
   *
   * @param graph the graph you are getting ITask copies from.
   * @return a new EdgeDescriptor to be used to apply edges to a future task graph configuration.
   */
  virtual EdgeDescriptor *copy(AnyTaskGraphConf *graph) = 0;

};
}

#endif //HTGS_EDGEDESCRIPTOR_HPP
