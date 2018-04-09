// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file TaskGraphSignalHandler.hpp
 * @author Timothy Blattner
 * @date April 5, 2018
 *
 * @brief Implements a signal handler to catch events such as termination and killing of the process. Once a signal
 * is caught, all task graphs that are registered with the signal handler will be written as a dot file. The dot file
 * is output in the working directory with the name of the signal as a prefix and '-graph-output.dot' as the suffix.
 *
 * @note This class should only be included from main due to the instantiation of a static variable. If this class is
 * included in multiple object files, then there will be linking errors.
 */
#ifndef HTGS_TASKGRAPHSIGNALHANDLER_HPP
#define HTGS_TASKGRAPHSIGNALHANDLER_HPP

#include <htgs/core/graph/AnyTaskGraphConf.hpp>
#include <csignal>
#include <vector>
#include <cstring>
namespace htgs {
/**
 * @class TaskGraphSignalHandler TaskGraphSignalHandler.hpp <htgs/log/TaskGraphSignalHandler.hpp>
 * @brief Implements a signal handler to catch events such as termination and killing of the process. Once a signal
 * is caught, all task graphs that are registered with the signal handler will be written as a dot file. The dot file
 * is output in the working directory with the name of the signal as a prefix and '<#>-graph-output.dot' as the suffix.
 *
 * Example usage:
 * @code
 * int main() {
 *   ...
 *   auto taskGraph = new htgs::TaskGraphConf...
 *
 *   htgs::TaskGraphSignalHandler::registerTaskGraph(taskGraph);
 *   htgs::TaskGraphSignalHandler::registerSignal(); // Default signal is SIGTERM
 *   htgs::TaskGraphSignalHandler::registerSignal(SIGKILL);
 *
 *   auto runtime = new htgs::TaskGraphRuntime(taskGraph);
 *   runtime->executeRuntime();
 *
 *   // If the program is killed/terminated then the signal handler will automatically output the task graph that was registered
 *
 * }
 * @endcode
 *
 * @note This class should only be included from main due to the instantiation of a static variable. If this class is
 * included in multiple object files, then there will be linking errors.
 */
class TaskGraphSignalHandler {
 public:

  /**
   * Function that handles signals.
   * Use TaskGraphSignalHandler::registerSignal to signal to this function.
   * @param signum the signal number that was triggered
   */
  static void handleSignal(int signum = SIGTERM) {
#ifdef _WIN32
    std::string signalString(std::to_string(signum));
#else
    std::string signalString(strsignal(signum));
#endif
    for (size_t i = 0; i < instances.size(); i++)
    {
      instances[i]->writeDotToFile(signalString + "-" + std::to_string(i) + "-graph-output.dot", DOTGEN_FLAG_SHOW_CONNECTOR_VERBOSE);
    }

    exit(signum);
  }

  /**
   * Registers a task graph to be displayed when a signal is fired.
   * @param taskGraph the task graph to be displayed. Calling this function on multiple graphs will output multiple dot files.
   */
  static void registerTaskGraph(AnyTaskGraphConf *taskGraph)
  {
    instances.push_back(taskGraph);
  }

  /**
   * Registers a signal for handling. (default SIGTERM)
   * @param signum
   */
  static void registerSignal(int signum = SIGTERM)
  {
    std::signal(signum, TaskGraphSignalHandler::handleSignal);
  }

 private:
  static std::vector<AnyTaskGraphConf *> instances; //!<< The task graph instances
};
}

std::vector<htgs::AnyTaskGraphConf *> htgs::TaskGraphSignalHandler::instances;

#endif //HTGS_TASKGRAPHSIGNALHANDLER_HPP
