
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file debug_message.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief Provides functionality for debug messaging
 */
#ifndef HTGS_DEBUG_MESSAGE_HPP
#define HTGS_DEBUG_MESSAGE_HPP

#include <iosfwd>
#include <iostream>

#ifndef NDEBUG
#define HTGS_ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << message << ": Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << std::endl; \
            std::terminate(); \
        } \
    } while (false)
#else
#define HTGS_ASSERT(condition, message) do { } while (false)
#endif

/**
 * @def HTGS_VERBOSE
 * Defines verbose mode
 */
#define HTGS_VERBOSE 1


/**
 * Prints a debug message to std::cerr with the specified level.
 * If the specified message level is greater than defined DEBUG_LEVEL
 * or if DEBUG_FLAG is not defined, then this equates to a no op.
 * Each message includes the file and line number for where the debug is called
 * @param msg the message
 * @param level the message level
 * @note \#define DEBUG_FLAG to enable debug messages
 *
 */
#define HTGS_DEBUG_MSG_LEVEL(msg, level) if (!HTGS_DEBUG_ENABLED || HTGS_DEBUG_LEVEL < level) {} \
        else htgs_dbglog() << __FILE__ << ":" << __LINE__ << " " << msg

/**
 * Prints a debug message to std::cerr with standard level
 * If DEBUG_FLAG is not defined, this equates to a no op
 * Each message includes the file and line number for where the debug is called
 * @param msg the message
 * @note \#define DEBUG_FLAG to enable debug messages
 *
 */
#define HTGS_DEBUG(msg) HTGS_DEBUG_MSG_LEVEL(msg, 0)

/**
 * Prints a debug message to std:cerr with VERBOSE level.
 * If DEBUG_FLAG is not defined or the DEBUG_LEVEL is not VERBOSE, then this equates to a no op
 * Each message includes the file and line number for where the debug is called
 * @param msg the message
 * @note \#define DEBUG_FLAG to enable debug messages
 * @note \#define DEBUG_LEVEL_VERBOSE to enable VERBOSE debugging
 */
#define HTGS_DEBUG_VERBOSE(msg) HTGS_DEBUG_MSG_LEVEL(msg, HTGS_VERBOSE)

/**
 * @def HTGS_DEBUG_LEVEL
 * Defines the debug level for printing debug messages
 */
#ifdef HTGS_DEBUG_LEVEL_VERBOSE
#define HTGS_DEBUG_LEVEL HTGS_VERBOSE
#else
#define HTGS_DEBUG_LEVEL 0
#endif

/**
 * @def HTGS_DEBUG_ENABLED
 * Defines whether debug is enabled or disabled
 */
#ifdef HTGS_DEBUG_FLAG
#define HTGS_DEBUG_ENABLED 1
#else
#define HTGS_DEBUG_ENABLED 0
#endif

/**
 * Debug logging structure for processing various types of arguments for std::cerr
 */
struct htgs_dbglog {
  //! @cond Doxygen_Suppress
  std::ostream &os_;
  mutable bool has_endl_;
  htgs_dbglog(std::ostream &os = std::cerr) : os_(os), has_endl_(false) {}
  ~htgs_dbglog() { if (!has_endl_) os_ << std::endl; }
  template<typename T>
  static bool has_endl(const T &) { return false; }
  static bool has_endl(char c) { return (c == '\n'); }
  static bool has_endl(std::string s) { return has_endl(*s.rbegin()); }
  static bool has_endl(const char *s) { return has_endl(std::string(s)); }
  template<typename T>
  static bool same_manip(T &(*m)(T &), T &(*e)(T &)) { return (m == e); }
  const htgs_dbglog &operator<<(std::ostream &(*m)(std::ostream &)) const {
    has_endl_ = same_manip(m, std::endl);
    os_ << m;
    return *this;
  }
  template<typename T>
  const htgs_dbglog &operator<<(const T &v) const {
    has_endl_ = has_endl(v);
    os_ << v;
    return *this;
  }
  //! @endcond
};

#endif //HTGS_DEBUG_MESSAGE_HPP
