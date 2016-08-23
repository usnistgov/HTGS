
# NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
# NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
# You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
link_libraries(${CMAKE_THREAD_LIBS_INIT})
#add_definitions(-DDEBUG_FLAG)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wl,--no-as-needed ${CMAKE_THREAD_LIBS_INIT}")

set(SIMPLE_SRC
        simple/SimpleClock.h
        simple/data/SimpleData.h
        simple/memory/SimpleAllocator.h
        simple/memory/SimpleReleaseRule.h
        simple/rules/SimpleDecompRule.cpp
        simple/rules/SimpleDecompRule.h
        simple/rules/SimpleRule.h
        simple/tasks/SimpleTask.cpp
        simple/tasks/SimpleTask.h
        simpleGraphTests.cpp
        simpleGraphTests.h
        simple/customEdges/BookkeeperCustomEdge.h
        simple/customEdges/MemoryManagerCustomEdge.h)


set(MATRIXMUL_SRC
        matrixMul/data/MatrixBlockData.h
        matrixMul/data/MatrixBlockMulData.h
        matrixMul/data/MatrixRequestData.h
        matrixMul/memory/MatrixAllocator.h
        matrixMul/memory/MatrixMemoryRule.h
        matrixMul/rules/MatrixAccumulateRule.h
        matrixMul/rules/MatrixDistributeRule.h
        matrixMul/rules/MatrixLoadRule.h
        matrixMul/rules/MatrixOutputRule.h
        matrixMul/tasks/MatrixAccumTask.h
        matrixMul/tasks/MatrixMulBlkTask.h
        matrixMul/tasks/OutputTask.h
        matrixMul/tasks/GenMatrixTask.h
        matrixMulGraphTests.cpp
        matrixMulGraphTests.h
        )

set(MEMMULTIRELEASE_SRC
        memMultiRelease/data/InputData.h
        memMultiRelease/data/ProcessedData.h
        memMultiRelease/tasks/InputTask.h
        memMultiRelease/tasks/OutputMemReleaseTask.h
        memMultiRelease/rules/SimpleDecompRule.h
        memMultiRelease/rules/MemDistributeRule.h
        memMultiReleaseGraphTests.cpp
        memMultiReleaseGraphTests.h
        memMultiRelease/memory/SimpleReleaseRule.h memMultiRelease/memory/SimpleMemoryAllocator.h)



set(MEMRELEASEOUTSIDE_SRC
        memReleaseOutsideGraphTests.cpp
        memReleaseOutsideGraphTests.h
        memTaskOutsideRelease/data/MultiMemData.h memTaskOutsideRelease/tasks/MemAllocTask.h memTaskOutsideRelease/tasks/MemReleaseTask.h memTaskOutsideRelease/rules/MemAllocDistributeRule.h ../../src/htgs/core/graph/TaskGraphDotGenFlags.h)


add_executable(runAPITests ${SIMPLE_SRC} ${MATRIXMUL_SRC} ${MEMMULTIRELEASE_SRC} ${MEMRELEASEOUTSIDE_SRC}
        api_check.cpp
        )

target_link_libraries(runAPITests gtest gtest_main)

add_custom_target(run-test ALL DEPENDS runAPITests
	COMMAND ${CMAKE_BINARY_DIR}/test/api_tests/runAPITests)
