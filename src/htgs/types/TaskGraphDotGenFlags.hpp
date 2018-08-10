// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file TaskGraphDotGenFlags.hpp
 * @author Timothy Blattner
 * @date August 22, 2016
 *
 * @brief Defines DOTGEN flags used for dot file generation.
 */

#ifndef HTGS_TASKGRAPHDOTGENFLAGS_HPP
#define HTGS_TASKGRAPHDOTGENFLAGS_HPP

/**
 * @def DOTGEN_FLAG_HIDE_MEM_EDGES
 * @brief Hides memory edges during dot generation
 */
#define DOTGEN_FLAG_HIDE_MEM_EDGES         1 << 0

/**
 * @def DOTGEN_FLAG_SHOW_ALL_THREADING
 * @brief Shows all threading fully expanded during dot generation
 */
#define DOTGEN_FLAG_SHOW_ALL_THREADING     1 << 1

/**
 * @def DOTGEN_FLAG_SHOW_IN_OUT_TYPES
 * @brief Shows input and output types for all tasks
 */
#define DOTGEN_FLAG_SHOW_IN_OUT_TYPES      1 << 2

/**
 * @def DOTGEN_FLAG_HIDE_PROFILE_COMP_TIME
 * @brief Hides profiling data for compute time
 */
#define DOTGEN_FLAG_HIDE_PROFILE_COMP_TIME 1 << 3

/**
 * @def DOTGEN_FLAG_HIDE_PROFILE_MAX_Q_SZ
 * @brief Hides profiling data for maximum queue size
 */
#define DOTGEN_FLAG_HIDE_PROFILE_MAX_Q_SZ  1 << 4

/**
 * @def DOTGEN_FLAG_HIDE_PROFILE_WAIT_TIME
 * @brief Hides profiling data for wait time
 */
#define DOTGEN_FLAG_HIDE_PROFILE_WAIT_TIME 1 << 5

/**
 * @def DOTGEN_COLOR_COMP_TIME
 * @brief Creates color map using compute time
 */
#define DOTGEN_COLOR_COMP_TIME             1 << 6

/**
 * @def DOTGEN_COLOR_MAX_Q_SZ
 * @brief Creates color map using maximum queue size
 */
#define DOTGEN_COLOR_MAX_Q_SZ              1 << 7

/**
 * @def DOTGEN_COLOR_WAIT_TIME
 * @brief Creates color map using wait time
 */
#define DOTGEN_COLOR_WAIT_TIME             1 << 8

/**
 * @def DOTGEN_FLAG_HIDE_MEMORY_WAIT_TIME
 * @brief Hides profiling data for waiting for memory
 */
#define DOTGEN_FLAG_HIDE_MEMORY_WAIT_TIME 1 << 9

/**
 * @def DOTGEN_COLOR_MEMORY_WAIT_TIME
 * @brief Creates color map using memory wait time
 */
#define DOTGEN_COLOR_MEMORY_WAIT_TIME 1 << 10

/**
 * @def DOTGEN_FLAG_SHOW_CURRENT_Q_SZ
 * @brief Displays the current queue size within each connector.
 */
#define DOTGEN_FLAG_SHOW_CURRENT_Q_SZ 1 << 11

/**
 * @def DOTGEN_FLAG_SHOW_CONNECTOR_VERBOSE
 * @brief Shows verbose information within each connector in the graph.
 */
#define DOTGEN_FLAG_SHOW_CONNECTOR_VERBOSE 1 << 12

/**
 * @def DOTGEN_FLAG_SHOW_TASK_LIVING_STATUS
 * @brief Shows the number of threads that are alive running the task.
 */
#define DOTGEN_FLAG_SHOW_TASK_LIVING_STATUS 1 << 13

#endif //HTGS_TASKGRAPHDOTGENFLAGS_HPP
