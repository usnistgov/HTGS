
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 4/18/16.
//

#include <gtest/gtest.h>
#include "simpleGraphTests.h"
#include "matrixMulGraphTests.h"
#include "memMultiReleaseGraphTests.h"
#include "memReleaseOutsideGraphTests.h"

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


TEST(SimpleGraph, GraphCustomEdgeCreation) {
  EXPECT_NO_FATAL_FAILURE(simpleGraphCreationWithCustomEdges());
}

TEST(SimpleGraph, GraphCustomEdgeExecution) {
  EXPECT_NO_FATAL_FAILURE(simpleGraphExecutionWithCustomEdges(1));
  EXPECT_NO_FATAL_FAILURE(simpleGraphExecutionWithCustomEdges(2));
  EXPECT_NO_FATAL_FAILURE(simpleGraphExecutionWithCustomEdges(5));
  EXPECT_NO_FATAL_FAILURE(simpleGraphExecutionWithCustomEdges(10));
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

TEST(MemMultiRelease, GraphCreationUserManaged) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphCreation(false, false, htgs::MMType::UserManaged));
}

TEST(MemMultiRelease, GraphExecutionUserManaged) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 1, false, false, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 1, false, false, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 1, false, false, htgs::MMType::UserManaged));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 2, false, false, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 2, false, false, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 2, false, false, htgs::MMType::UserManaged));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 3, false, false, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 1, 3, false, false, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 3, false, false, htgs::MMType::UserManaged));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 5, false, false, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 1, 5, false, false, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 5, false, false, htgs::MMType::UserManaged));
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

TEST(MemMultiRelease, GraphCreationUserManagedWithGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphCreation(false, true, htgs::MMType::UserManaged));
}

TEST(MemMultiRelease, GraphExecutionUserManagedWithGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 1, false, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 1, false, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 1, false, true, htgs::MMType::UserManaged));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 2, false, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 2, false, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 2, false, true, htgs::MMType::UserManaged));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 3, false, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 3, false, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 3, false, true, htgs::MMType::UserManaged));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 5, false, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 5, false, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 5, false, true, htgs::MMType::UserManaged));
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


TEST(MemMultiRelease, GraphCreationUserManagedWithAdditionalGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphCreation(true, true, htgs::MMType::UserManaged));
}

TEST(MemMultiRelease, GraphExecutionUserManagedWithAdditionalGraphMemoryEdge) {
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 1, true, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 1, true, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 1, true, true, htgs::MMType::UserManaged));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 2, true, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 2, true, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 2, true, true, htgs::MMType::UserManaged));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 3, true, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 3, true, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 3, true, true, htgs::MMType::UserManaged));

  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(1, 1, 5, true, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 2, 5, true, true, htgs::MMType::UserManaged));
  EXPECT_NO_FATAL_FAILURE(multiReleaseGraphExecution(100, 10, 5, true, true, htgs::MMType::UserManaged));
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

// TODO: Create test case to test task interacting with memory inside of another graph (task releasing for task in execPipeline)
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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
