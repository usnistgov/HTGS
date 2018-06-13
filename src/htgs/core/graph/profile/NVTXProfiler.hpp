// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file NVTXProfiler.hpp
 * @author Timothy Blattner
 * @date May 11, 2018
 *
 * @brief NVTX Profiler uses NVIDIA's NVTX API to produce profiling metrics that are visualized
 * with Nsight Systems (https://developer.nvidia.com/nsight-systems).
 *
 * @details There are two profiling modes of operation that can be enabled:
 * (1) (default, USE_NVTX) per task profiling with one domain per task and (2) (USE_MINIMAL_NVTX) per thread profiling with shared NVTX domains.
 *
 * Option 1 is the default mode of operation when you compile with NVTX enabled. This provides a nice
 * visualization of how the tasks ran and interacted with eachother. Currently Nsight Systems limits the
 * number of domains to 25, so if your graph contains more than 24 tasks, then the profiling execution will have
 * undefined behavior. For graphs of this size, it is recommended to enable option 2.
 *
 * @note To enable NVTX profiling you must add the USE_NVTX directive.
 * @note For graphs with more than 24 tasks, you must add both USE_NVTX and USE_MINIMAL_NVTX directives.
 * @note Graphs that contain less than 25 tasks can also use the MINIMAL mode for NVTX.
 * @note Add 'FindNVTX.cmake' to your project to assist in finding the necessary includes and libraries for use with NVTX
 *
 */


#ifndef HTGS_NVTXPROFILER_H
#define HTGS_NVTXPROFILER_H

#ifdef USE_NVTX
#include <nvtx3/nvToolsExt.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#define TASK_GRAPH_PREFIX_NAME "graph"

#define NVTX_COLOR_INITIALIZING    0xFF123456
#define NVTX_COLOR_EXECUTING       0xFF72ff68
#define NVTX_COLOR_WAITING         0xFFff7f83
#define NVTX_COLOR_WAITING_FOR_MEM 0xFFffc86a
#define NVTX_COLOR_RELEASE_MEM     0xFF7fbdff
#define NVTX_COLOR_SHUTTING_DOWN   0xFF654321

//#define NVTX_COLOR_INITIALIZING    0xFF123456
//#define NVTX_COLOR_EXECUTING       0xFF72ff68
//#define NVTX_COLOR_WAITING         0xFFff6872
//#define NVTX_COLOR_WAITING_FOR_MEM 0xFFffc868
//#define NVTX_COLOR_RELEASE_MEM     0xFF68aeff
//#define NVTX_COLOR_SHUTTING_DOWN   0xFF654321


namespace htgs {

/**
 * @class NVTXProfiler NVTXProfiler.hpp <htgs/core/graph/profile/NVTXProfiler.hpp>
 * @brief A class to wrap calls to the NVTX library for tracking events that occur within an HTGS task graph
 * @details
 * HTGS uses the NVTX API and NVIDIA Nsight Systems to visualize the execution of a graph of tasks.
 *
 * There are two profiling modes that can be enabled at compile-time.
 * (1) (default) Per task profiling with one NVTX domain per task.
 * (2) Per thread profiling with shared NVTX domains for each event.
 *
 * In mode 1, each task shares a single domain with all of its threads. This has the effect of visualizing all
 * the threads within a task to identify precisely what that task is doing at any moment in time. This is useful for
 * visualizing the interaction between tasks and identify bottlenecks. This mode can only be used if there are less than
 * 25 tasks in your graph. Currently NVTX and Nsight Systems has a strict limit of 25 NVTX domains during execution.
 *
 * In mode 2, there exists seven domains per htgs::TaskGraphRuntime. This mode is useful for large graphs with more than 24 tasks.
 * In this visualization each thread outputs several domains, which are shared across all tasks and threads; initialize,
 * execute, wait, wait for memory, release memory, and shutdown.
 *
 * Currently it is recommended to limit the use of NVTX profiling to graphs that do not contain large ExecutionPipelines due
 * to the NVTX domains limitation.
 *
 * @note To enable NVTX profiling you must add the USE_NVTX directive.
 * @note For graphs with more than 24 tasks, you must add both USE_NVTX and USE_MINIMAL_NVTX directives.
 * @note Graphs that contain less than 25 tasks can also use the MINIMAL mode for NVTX.
 * @note Add 'FindNVTX.cmake' to your project to assist in finding the necessary includes and libraries for use with NVTX
 */
class NVTXProfiler {
 public:

  /**
   * Constructs the NVTX profiler
   * @param prefixName the prefix name that is inserted in front of each of the profiling event attributes
   * @param taskDomain the domain used during mode 1 operation
   * @param domainInitialize the initialize domain used during mode 2 operation
   * @param domainExecute the execute domain used during mode 2 operation
   * @param domainWait the wait domain used during mode 2 operation
   * @param domainWaitForMem the wait for memory domain used during mode 2 operation
   * @param domainReleaseMem the release memory domain used during mode 2 operation
   * @param domainShutdown the shutdown domain used during mode 2 operation
   */
  NVTXProfiler(std::string prefixName, nvtxDomainHandle_t taskDomain, nvtxDomainHandle_t domainInitialize, nvtxDomainHandle_t domainExecute, nvtxDomainHandle_t domainWait, nvtxDomainHandle_t domainWaitForMem, nvtxDomainHandle_t domainReleaseMem, nvtxDomainHandle_t domainShutdown)
   : taskDomain(taskDomain), domainInitialize(domainInitialize), domainExecute(domainExecute), domainWait(domainWait), domainWaitForMem(domainWaitForMem), domainReleaseMem(domainReleaseMem), domainShutdown(domainShutdown) {

    initializeName = prefixName + ":Initializing";
    executeName = prefixName + ":Executing";
    waitName = prefixName + ":Waiting";
    waitForMemName = prefixName + ":Waiting for memory";
    releaseMemName = prefixName + ":Releasing memory";
    shutdownName = prefixName + ":Shutting down";

    initializeAttrib = createEventAttribute(NVTX_COLOR_INITIALIZING);
    executeAttrib = createEventAttribute(NVTX_COLOR_EXECUTING);

    waitAttrib = createEventAttribute(NVTX_COLOR_WAITING);
    waitAttrib->payloadType = NVTX_PAYLOAD_TYPE_INT64;
    waitAttrib->payload.ullValue = 0;

    waitForMemAttrib = createEventAttribute(NVTX_COLOR_WAITING_FOR_MEM);
    releaseMemAttrib = createEventAttribute(NVTX_COLOR_RELEASE_MEM);
    shutdownAttrib = createEventAttribute(NVTX_COLOR_SHUTTING_DOWN);

#ifdef USE_MINIMAL_NVTX
    initializeAttrib->messageType = NVTX_MESSAGE_TYPE_ASCII;
    initializeAttrib->message.ascii = initializeName.c_str();

    executeAttrib->messageType = NVTX_MESSAGE_TYPE_ASCII;
    executeAttrib->message.ascii = executeName.c_str();

    waitAttrib->messageType = NVTX_MESSAGE_TYPE_ASCII;
    waitAttrib->message.ascii = waitName.c_str();

    waitForMemAttrib->messageType = NVTX_MESSAGE_TYPE_ASCII;
    waitForMemAttrib->message.ascii = waitForMemName.c_str();

    releaseMemAttrib->messageType = NVTX_MESSAGE_TYPE_ASCII;
    releaseMemAttrib->message.ascii = releaseMemName.c_str();

    shutdownAttrib->messageType = NVTX_MESSAGE_TYPE_ASCII;
    shutdownAttrib->message.ascii = shutdownName.c_str();
#else
    initializeString = nvtxDomainRegisterStringA(taskDomain, initializeName.c_str());
    executeString = nvtxDomainRegisterStringA(taskDomain, executeName.c_str());
    waitString = nvtxDomainRegisterStringA(taskDomain, waitName.c_str());
    waitForMemString = nvtxDomainRegisterStringA(taskDomain, waitForMemName.c_str());
    releaseMemString = nvtxDomainRegisterStringA(taskDomain, releaseMemName.c_str());
    shutdownString = nvtxDomainRegisterStringA(taskDomain, shutdownName.c_str());

    initializeAttrib->messageType = NVTX_MESSAGE_TYPE_REGISTERED;
    initializeAttrib->message.registered = initializeString;

    executeAttrib->messageType = NVTX_MESSAGE_TYPE_REGISTERED;
    executeAttrib->message.registered = executeString;

    waitAttrib->messageType = NVTX_MESSAGE_TYPE_REGISTERED;
    waitAttrib->message.registered = waitString;

    waitForMemAttrib->messageType = NVTX_MESSAGE_TYPE_REGISTERED;
    waitForMemAttrib->message.registered = waitForMemString;

    releaseMemAttrib->messageType = NVTX_MESSAGE_TYPE_REGISTERED;
    releaseMemAttrib->message.registered = releaseMemString;

    shutdownAttrib->messageType = NVTX_MESSAGE_TYPE_REGISTERED;
    shutdownAttrib->message.registered = shutdownString;
#endif
  }

  /**
   * Destructor
   */
  ~NVTXProfiler() {
    delete initializeAttrib;
    delete executeAttrib;
    delete waitAttrib;
    delete waitForMemAttrib;
    delete releaseMemAttrib;
    delete shutdownAttrib;

#ifndef USE_MINIMAL_NVTX
    nvtxDomainDestroy(taskDomain);
#endif
  }

  /**
   * Adds a release marker into the timeline to show when the task released memory.
   */
  void addReleaseMarker() {
#ifdef USE_MINIMAL_NVTX
    nvtxDomainMarkEx(domainReleaseMem, releaseMemAttrib);
#else
    nvtxDomainMarkEx(taskDomain, releaseMemAttrib);
#endif
  }

  /**
   * Starts tracking intialization in the timeline to show when the task has started its initialization phase.
   * @return the range id
   */
  nvtxRangeId_t startRangeInitializing() {
#ifdef USE_MINIMAL_NVTX
    nvtxDomainRangePushEx(domainInitialize, initializeAttrib);
    return nvtxDomainRangeStartEx(domainInitialize, initializeAttrib);
#else
    return nvtxDomainRangeStartEx(taskDomain, initializeAttrib);
#endif
  }

  /**
   * Starts tracking execution in the timeline to show when the task has started executing on data.
   * @return the range id
   */
  nvtxRangeId_t startRangeExecuting() {
#ifdef USE_MINIMAL_NVTX
    nvtxDomainRangePushEx(domainExecute, executeAttrib);
    return nvtxDomainRangeStartEx(domainExecute, executeAttrib);
#else
    return nvtxDomainRangeStartEx(taskDomain, executeAttrib);
#endif
  }

  /**
   * Starts tracking execution in the timeline to show when the task has started waiting for data.
   * This event shows the current queue size in the payload within the attribute.
   * @param queueSize the queue size
   * @return the range id
   */
  nvtxRangeId_t startRangeWaiting(uint64_t queueSize) {
    waitAttrib->payload.ullValue = queueSize;
#ifdef USE_MINIMAL_NVTX
    nvtxDomainRangePushEx(domainWait, waitAttrib);
    return nvtxDomainRangeStartEx(domainWait, waitAttrib);
#else
    return nvtxDomainRangeStartEx(taskDomain, waitAttrib);
#endif
  }

  /**
   * Starts tracking waiting for memory in the timeline to show when the task has started waiting for memory from a memory manager.
   * @return the range id
   */
  nvtxRangeId_t startRangeWaitingForMemory() {
#ifdef USE_MINIMAL_NVTX
    nvtxDomainRangePushEx(domainWaitForMem, waitForMemAttrib);
    return nvtxDomainRangeStartEx(domainWaitForMem, waitForMemAttrib);
#else
    return nvtxDomainRangeStartEx(taskDomain, waitForMemAttrib);
#endif
  }

  /**
   * Starts tracking shutdown in the timeline to show when the task has started its shutdown phase.
   * @return the range id
   */
  nvtxRangeId_t startRangeShuttingDown() {
#ifdef USE_MINIMAL_NVTX
    nvtxDomainRangePushEx(domainShutdown, shutdownAttrib);
    return nvtxDomainRangeStartEx(domainShutdown, shutdownAttrib);
#else
    return nvtxDomainRangeStartEx(taskDomain, shutdownAttrib);
#endif
  }

  /**
   * Ends tracking the initialization phase for a task
   * @param range the range id that was acquired from startRangeInitializing
   */
  void endRangeInitializing(nvtxRangeId_t range) {
#ifdef USE_MINIMAL_NVTX
    nvtxDomainRangePop(domainInitialize);
    return nvtxDomainRangeEnd(domainInitialize, range);
#else
    nvtxDomainRangeEnd(taskDomain, range);
#endif
  }

  /**
   * Ends tracking the execute for a task
   * @param range the range id that was acquired from startRangeExecuting
   */
  void endRangeExecuting(nvtxRangeId_t range) {
#ifdef USE_MINIMAL_NVTX
    nvtxDomainRangePop(domainExecute);
    nvtxDomainRangeEnd(domainExecute, range);
#else
      nvtxDomainRangeEnd(taskDomain, range);
#endif
  }

  /**
   * Ends tracking the waiting for data for a task
   * @param range the range id that was acquired from startRangeWaiting
   */
  void endRangeWaiting(nvtxRangeId_t range) {
#ifdef USE_MINIMAL_NVTX
    nvtxDomainRangePop(domainWait);
    nvtxDomainRangeEnd(domainWait, range);
#else
    nvtxDomainRangeEnd(taskDomain, range);
#endif
  }

  /**
   * Ends tracking the waiting for memory from a memory edge.
   * @param range the range id that was acquired from startRangeWaitingForMemory
   */
  void endRangeWaitingForMem(nvtxRangeId_t range) {
#ifdef USE_MINIMAL_NVTX
    nvtxDomainRangePop(domainWaitForMem);
    nvtxDomainRangeEnd(domainWaitForMem, range);
#else
    nvtxDomainRangeEnd(taskDomain, range);
#endif
  }

  /**
   * Ends tracking the shutdown phase for a task
   * @param range the range id that was acquired from startRangeShuttingDown
   */
  void endRangeShuttingDown(nvtxRangeId_t range) {
#ifdef USE_MINIMAL_NVTX
    nvtxDomainRangePop(domainShutdown);
    nvtxDomainRangeEnd(domainShutdown, range);
#else
    nvtxDomainRangeEnd(taskDomain, range);
#endif
  }

  /**
   * Gets the task domain.
   * @return the task domain
   * @note Only use if the program is compile with the USE_NVTX directive. DO NOT use if USE_MINIMAL_NVTX is defined.
   * @note This domain can be used to add custom user-defined NVTX events into the timeline.
   */
  const nvtxDomainRegistration *getTaskDomain() const {
    return taskDomain;
  }

 private:
  //! @cond Doxygen_Suppress
  nvtxEventAttributes_t *createEventAttribute(uint32_t color) {
    nvtxEventAttributes_t *event = new nvtxEventAttributes_t;
    bzero(event, NVTX_EVENT_ATTRIB_STRUCT_SIZE);
    event->version = NVTX_VERSION;
    event->size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
    event->colorType = NVTX_COLOR_ARGB;
    event->color = color;
    return event;
  }

  std::string initializeName{};
  std::string executeName{};
  std::string waitName{};
  std::string waitForMemName{};
  std::string releaseMemName{};
  std::string shutdownName{};

  nvtxDomainHandle_t taskDomain;

  nvtxDomainHandle_t domainInitialize;
  nvtxDomainHandle_t domainExecute;
  nvtxDomainHandle_t domainWait;
  nvtxDomainHandle_t domainWaitForMem;
  nvtxDomainHandle_t domainReleaseMem;
  nvtxDomainHandle_t domainShutdown;

  nvtxStringHandle_t initializeString{};
  nvtxStringHandle_t executeString{};
  nvtxStringHandle_t waitString{};
  nvtxStringHandle_t waitForMemString{};
  nvtxStringHandle_t releaseMemString{};
  nvtxStringHandle_t shutdownString{};

  nvtxEventAttributes_t *initializeAttrib;
  nvtxEventAttributes_t *executeAttrib;
  nvtxEventAttributes_t *waitAttrib;
  nvtxEventAttributes_t *waitForMemAttrib;
  nvtxEventAttributes_t *releaseMemAttrib;
  nvtxEventAttributes_t *shutdownAttrib;
  //! @endcond

};
}

#endif
#endif //HTGS_NVTXPROFILER_H
