
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 4/18/16.
//
#include <gtest/gtest.h>
#include <htgs/log/TaskGraphSignalHandler.hpp>

#include "simpleGraphTests.h"
#include "matrixMulGraphTests.h"
#include "memMultiReleaseGraphTests.h"
#include "memReleaseOutsideGraphTests.h"
#include "recursiveGraphsTests.h"


#ifdef USE_CUDA
#include "simpleCudaGraphTests.h"

TEST(SimpleCudaGraph, CudaTaskCreation) {
  EXPECT_NO_FATAL_FAILURE(createCudaTask());
}

TEST(SimpleCudaGraph, CudaGraphCreation) {
  EXPECT_NO_FATAL_FAILURE(simpleCudaGraphCreation());
}

TEST(SimpleCudaGraph, CudaGraphExecution) {
  EXPECT_NO_FATAL_FAILURE(simpleCudaGraphExecution());
}

#endif

TEST(SimpleGraph, DataCreation) {
  EXPECT_NO_FATAL_FAILURE(createData());
}

TEST(SimpleGraph, AllocationAndFree) {
  EXPECT_NO_FATAL_FAILURE(memoryAllocAndFreeCheck());
}

TEST(SimpleGraph, TaskCreation) {
  EXPECT_NO_FATAL_FAILURE(createTask());
}

TEST(SimpleGraph, GraphCreation) {
  EXPECT_NO_FATAL_FAILURE(simpleGraphCreation());
}

TEST(SimpleGraph, GraphExecution) {
  EXPECT_NO_FATAL_FAILURE(simpleGraphExecution(1));
  EXPECT_NO_FATAL_FAILURE(simpleGraphExecution(2));
  EXPECT_NO_FATAL_FAILURE(simpleGraphExecution(5));
  EXPECT_NO_FATAL_FAILURE(simpleGraphExecution(10));
}


TEST(MatMulGraph, TaskCreation) {
  EXPECT_NO_FATAL_FAILURE(createMatMulTasks());
}

TEST(MatMulGraph, GraphCreation) {
  EXPECT_NO_FATAL_FAILURE(matMulGraphCreation());
}

TEST(MatMulGraph, GraphExecution) {
  EXPECT_NO_FATAL_FAILURE(matMulGraphExecution(16, 2, 1, 2.0));
  EXPECT_NO_FATAL_FAILURE(matMulGraphExecution(16, 2, 5, 4.0));
  EXPECT_NO_FATAL_FAILURE(matMulGraphExecution(64, 8, 1, 16.0));
  EXPECT_NO_FATAL_FAILURE(matMulGraphExecution(64, 8, 5, 100.0));
}

TEST(MemMultiRelease, GraphCreationStatic) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphCreation(false, false, htgs::MMType::Static));
}

TEST(MemMultiRelease, GraphExecutionStatic) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 1, false, false, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 1, false, false, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 1, false, false, htgs::MMType::Static));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 2, false, false, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 2, false, false, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 2, false, false, htgs::MMType::Static));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 3, false, false, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 1, 3, false, false, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 3, false, false, htgs::MMType::Static));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 5, false, false, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 1, 5, false, false, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 5, false, false, htgs::MMType::Static));
}

TEST(MemMultiRelease, GraphCreationDynamic) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphCreation(false, false, htgs::MMType::Dynamic));
}

TEST(MemMultiRelease, GraphExecutionDynamic) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 1, false, false, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 1, false, false, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 1, false, false, htgs::MMType::Dynamic));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 2, false, false, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 2, false, false, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 2, false, false, htgs::MMType::Dynamic));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 3, false, false, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 1, 3, false, false, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 3, false, false, htgs::MMType::Dynamic));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 5, false, false, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 1, 5, false, false, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 5, false, false, htgs::MMType::Dynamic));
}

TEST(MemMultiRelease, GraphCreationStaticWithGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphCreation(false, true, htgs::MMType::Static));
}


TEST(MemMultiRelease, GraphExecutionStaticWithGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 1, false, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 1, false, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 1, false, true, htgs::MMType::Static));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 2, false, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 2, false, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 2, false, true, htgs::MMType::Static));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 3, false, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 3, false, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 3, false, true, htgs::MMType::Static));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 5, false, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 5, false, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 5, false, true, htgs::MMType::Static));
}

TEST(MemMultiRelease, GraphCreationDynamicWithGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphCreation(false, true, htgs::MMType::Dynamic));
}

TEST(MemMultiRelease, GraphExecutionDynamicWithGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 1, false, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 1, false, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 1, false, true, htgs::MMType::Dynamic));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 2, false, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 2, false, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 2, false, true, htgs::MMType::Dynamic));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 3, false, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 3, false, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 3, false, true, htgs::MMType::Dynamic));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 5, false, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 5, false, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 5, false, true, htgs::MMType::Dynamic));
}

TEST(MemMultiRelease, GraphCreationStaticWithAdditionalGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphCreation(true, true, htgs::MMType::Static));
}


TEST(MemMultiRelease, GraphExecutionStaticWithAdditionalGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 1, true, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 1, true, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 1, true, true, htgs::MMType::Static));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 2, true, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 2, true, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 2, true, true, htgs::MMType::Static));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 3, true, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 3, true, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 3, true, true, htgs::MMType::Static));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 5, true, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 5, true, true, htgs::MMType::Static));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 5, true, true, htgs::MMType::Static));
}


TEST(MemMultiRelease, GraphCreationDynamicWithAdditionalGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphCreation(true, true, htgs::MMType::Dynamic));
}

TEST(MemMultiRelease, GraphExecutionDynamicWithAdditionalGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 1, true, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 1, true, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 1, true, true, htgs::MMType::Dynamic));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 2, true, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 2, true, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 2, true, true, htgs::MMType::Dynamic));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 3, true, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 3, true, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 3, true, true, htgs::MMType::Dynamic));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 5, true, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 5, true, true, htgs::MMType::Dynamic));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 5, true, true, htgs::MMType::Dynamic));
}

TEST(MemReleaseOutsideGraph, GraphCreation) {
  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphCreation());
}

TEST(MemReleaseOutsideGraph, GraphExecution) {
  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(1, 1, 1));
  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(100, 2, 1));
  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(100, 10, 1));

  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(1, 1, 2));
  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(100, 2, 2));
  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(100, 10, 2));

  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(1, 1, 3));
  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(100, 2, 3));
  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(100, 10, 3));

  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(1, 1, 5));
  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(100, 2, 5));
  EXPECT_NO_FATAL_FAILURE(memReleaseOutsideGraphExecution(100, 10, 5));
}

TEST(RecursiveGraphs, TGTask) {
  EXPECT_NO_FATAL_FAILURE(testTGTasks(false, false, 1, 1));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(false, true, 1, 1));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(true, false, 1, 1));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(true, true, 1, 1));


  EXPECT_NO_FATAL_FAILURE(testTGTasks(false, false, 2, 2));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(false, true, 2, 2));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(true, false, 2, 2));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(true, true, 2, 2));

  EXPECT_NO_FATAL_FAILURE(testTGTasks(false, false, 5, 5));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(false, true, 5, 5));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(true, false, 5, 5));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(true, true, 5, 5));

  EXPECT_NO_FATAL_FAILURE(testTGTasks(false, false, 10, 10));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(false, true, 10, 10));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(true, false, 10, 10));
  EXPECT_NO_FATAL_FAILURE(testTGTasks(true, true, 10, 10));

}

TEST(RecursiveGraph, RecursiveTGTask) {
  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(1, 1, 1));
  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(2, 1, 1));
  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(5, 1, 1));
  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(10, 1, 1));

  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(2, 2, 2));
  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(5, 2, 2));
  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(10, 2, 2));

  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(5, 5, 5));
  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(10, 5, 5));

  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(10, 10, 10));
}

TEST(RecursiveGraph, RecursiveTGTaskWithExecPipeline) {
//  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(1, 1, 1, true, 1));
  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(2, 2, 2, true, 2));
//  EXPECT_NO_FATAL_FAILURE(testGraphsWithinGraphs(5, 5, 5, true, 5));
}

TEST(RecursiveGraph, TGTaskWithExecPipeline) {
  EXPECT_NO_FATAL_FAILURE(testTGTaskWithExecPipeline(1, 1, 1));
  EXPECT_NO_FATAL_FAILURE(testTGTaskWithExecPipeline(2, 2, 2));
  EXPECT_NO_FATAL_FAILURE(testTGTaskWithExecPipeline(5, 5, 5));
  EXPECT_NO_FATAL_FAILURE(testTGTaskWithExecPipeline(10, 10, 10));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();

  return ret;
}

