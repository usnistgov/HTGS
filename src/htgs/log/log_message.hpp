
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file log_message.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief Provides functionality for log messaging
 */
#ifndef HTGS_LOG_MESSAGE_HPP
#define HTGS_LOG_MESSAGE_HPP

#include <iosfwd>
#include <iostream>

/**
 * @def VERBOSE
 * Defines verbose mode
 */
#define VERBOSE 1

/**
 * Prints a log message to std::cout with the specified level.
 * If the specified message level is greater than defined LOG_LEVEL
 * or if LOG_FLAG is not defined, then this equates to a no op
 * @param msg the message
 * @param level the message level
 * @note \#define LOG_FLAG to enable log messages
 *
 */
#define LG_MSG_LEVEL(msg, level) if (!LOG_ENABLED || LOG_LEVEL < level) {} \
        else lglog() << " "  << msg

/**
 * Prints a log message to std::cout with standard level
 * If LOG_FLAG is not defined, this equates to a no op
 * @param msg the message
 * @note \#define LOG_FLAG to enable log messages
 *
 */
#define LG(msg) LG_MSG_LEVEL(msg, 0)

/**
 * Prints a log message to std::cout with VERBOSE level.
 * If LOG_FLAG is not defined or the LOG_LEVEL is not VERBOSE, then this equates to a no op
 * @param msg the message
 * @note \#define LOG_FLAG to enable log messages
 * @note \#define LOG_LEVEL_VERBOSE to enable VERBOSE debugging
 */
#define LG_VERBOSE(msg) LG_MSG_LEVEL(msg, VERBOSE)

/**
 * @def LOG_LEVEL
 * Defines the debug level for printing debug messages
 */
#ifdef LOG_LEVEL_VERBOSE
#define LOG_LEVEL VERBOSE
#else
#define LOG_LEVEL 0
#endif

/**
 * @def LOG_ENABLED
 * Defines whether debug is enabled or disabled
 */
#ifdef LOG_FLAG
#define LOG_ENABLED 1
#else
#define LOG_ENABLED 0
#endif

/**
 * Log structure for processing various types of arguments for std::cout
 */
struct lglog {
  //! @cond Doxygen_Suppress
  std::ostream &os_;
  mutable bool has_endl_;
  lglog(std::ostream &os = std::cout) : os_(os), has_endl_(false) {}
  ~lglog() { if (!has_endl_) os_ << std::endl; }
  template<typename T>
  static bool has_endl(const T &) { return false; }
  static bool has_endl(char c) { return (c == '\n'); }
  static bool has_endl(std::string s) { return has_endl(*s.rbegin()); }
  static bool has_endl(const char *s) { return has_endl(std::string(s)); }
  template<typename T>
  static bool same_manip(T &(*m)(T &), T &(*e)(T &)) { return (m == e); }
  const lglog &operator<<(std::ostream &(*m)(std::ostream &)) const {
    has_endl_ = same_manip(m, std::endl);
    os_ << m;
    return *this;
  }
  template<typename T>
  const lglog &operator<<(const T &v) const {
    has_endl_ = has_endl(v);
    os_ << v;
    return *this;
  }
  //! @endcond
};

#endif //HTGS_LOG_MESSAGE_HPP
