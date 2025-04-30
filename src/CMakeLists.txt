
# NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
# NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
# You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake-modules")

# Add a dummy target to make the export work
add_library(htgs INTERFACE)
target_include_directories(htgs INTERFACE $<INSTALL_INTERFACE:include>)

# Install the target
install(TARGETS htgs EXPORT htgsTargets)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/htgsConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/htgsConfig.cmake"
        @ONLY
)

install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/htgsConfig.cmake"
        DESTINATION
        "${CMAKE_INSTALL_PREFIX}/share/htgs"
)

install(
        EXPORT
        htgsTargets
        NAMESPACE
        htgs::
        DESTINATION
        "${CMAKE_INSTALL_PREFIX}/share/htgs"
        FILE htgsTargets.cmake
)


install(DIRECTORY htgs
        DESTINATION include
        COMPONENT headers)



if (BUILD_MAIN)

    find_package(NVTX QUIET)

    if(NVTX_FOUND)
        message(STATUS "FOUND IT")
        add_definitions(-DUSE_NVTX)
        include_directories(${NVTX_INCLUDE_DIR})
        link_libraries(${NVTX_LIBRARIES})
    endif()

    add_definitions(-DPROFILE)
#    add_definitions(-DWS_PROFILE)
#    add_definitions(-DUSE_PRIORITY_QUEUE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wall -Wextra -Wno-unused-parameter")


    set(INC_ALL
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/Bookkeeper.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/ExecutionPipeline.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/ICudaTask.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/IData.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/IMemoryAllocator.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/IMemoryReleaseRule.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/IRule.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/ITask.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/MemoryData.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/TGTask.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/TaskGraphConf.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/TaskGraphRuntime.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/api/VoidData.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/comm/DataPacket.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/comm/TaskGraphCommunicator.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/AnyConnector.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/AnyTaskGraphConf.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/Connector.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/edge/EdgeDescriptor.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/edge/GraphRuleProducerEdge.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/edge/MemoryEdge.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/edge/ProducerConsumerEdge.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/edge/RuleEdge.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/profile/NVTXProfiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/profile/TaskGraphProfiler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/profile/TaskManagerProfile.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/memory/AnyMemoryAllocator.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/memory/CudaMemoryManager.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/memory/MemoryManager.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/memory/MemoryPool.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/queue/BlockingQueue.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/queue/PriorityBlockingQueue.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/rules/AnyIRule.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/rules/AnyRuleManager.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/rules/AnyRuleManagerInOnly.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/rules/ExecutionPipelineBroadcastRule.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/rules/RuleManager.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/task/AnyITask.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/task/AnyTaskManager.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/task/TaskManager.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/debug/debug_message.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/log/TaskGraphSignalHandler.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/log/log_message.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/types/MMType.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/types/TaskGraphDotGenFlags.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/types/Types.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/utils/ProfileUtils.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/edge/GraphEdge.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/edge/GraphTaskProducerEdge.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/htgs/core/graph/edge/GraphTaskConsumerEdge.hpp
            )

        message(STATUS ${INC_ALL})

    find_package(Threads REQUIRED)
    #find_package(HTGSVisualizer QUIET)

    link_libraries(${CMAKE_THREAD_LIBS_INIT})
    include_directories(".")


    #include_directories(${LIBHTGS_VISUALIZER_INCLUDE_DIRS})

    #add_definitions(${HTGS_VISUALIZER_DEFINITIONS})

    add_executable(main "main.cpp" ${INC_ALL})

    #target_link_libraries(main ${LIBHTGS_VISUALIZER_LIBRARIES})

endif(BUILD_MAIN)
