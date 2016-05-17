
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 4/18/16.
//
#include <gtest/gtest.h>
#include <htgs/api/TaskGraph.hpp>
#include <htgs/api/Runtime.hpp>

#include "matrixMulGraphTests.h"
#include "matrixMul/data/MatrixRequestData.h"
#include "matrixMul/data/MatrixBlockData.h"
#include "matrixMul/tasks/GenMatrixTask.h"
#include "matrixMul/tasks/MatrixAccumTask.h"
#include "matrixMul/tasks/MatrixMulBlkTask.h"
#include "matrixMul/tasks/OutputTask.h"
#include "matrixMul/rules/MatrixAccumulateRule.h"
#include "matrixMul/rules/MatrixLoadRule.h"
#include "matrixMul/rules/MatrixDistributeRule.h"
#include "matrixMul/rules/MatrixOutputRule.h"
#include "matrixMul/memory/MatrixAllocator.h"


void validateResults(double *resultMatrix, int dim, double initValue)
{
  double *vals = new double[dim *dim];
  double *seqResult = new double[dim *dim];

  for (int i = 0; i < dim*dim; i++)
  {
    vals[i] = initValue;
  }

  for (int aRow = 0; aRow < dim; aRow++)
  {
    for (int bCol = 0; bCol < dim; bCol++)
    {
      double sum = 0.0;
      for (int k =0; k < dim; k++)
      {
        sum += vals[aRow * dim + k] * vals[k * dim + bCol];
      }
      seqResult[aRow*dim+bCol] = sum;
    }
  }

  bool valid = true;
  for (int i = 0; i < dim*dim; i++)
  {
    if (resultMatrix[i] != seqResult[i]) {
      valid = false;
      break;
    }
  }

  EXPECT_TRUE(valid);

  delete []vals;
}


void createMatMulTasks()
{
  GenMatrixTask gTask(1, 2, 16, 16, "matrixA", 1.0);
  MatrixAccumTask maTask(1);
  MatrixMulBlkTask mmBlk(1);
  OutputTask oTask;


  EXPECT_EQ(8, gTask.getNumBlocksCols());
  EXPECT_EQ(8, gTask.getNumBlocksRows());
  EXPECT_EQ(1, gTask.getNumThreads());
  EXPECT_EQ(1, maTask.getNumThreads());
  EXPECT_EQ(1, mmBlk.getNumThreads());
  EXPECT_EQ(1, oTask.getNumThreads());
}


htgs::TaskGraph<MatrixRequestData, MatrixBlockData<double *>> *createMatMulGraph(int numThreads, int dim, int blockSize, double initValue)
{
  auto taskGraph = new htgs::TaskGraph<MatrixRequestData, MatrixBlockData<double *>>();

  GenMatrixTask *genAMatTask = new GenMatrixTask(1, blockSize, dim, dim, "A", initValue);
  GenMatrixTask *genBMatTask = new GenMatrixTask(1, blockSize, dim, dim, "B", initValue);
  MatrixMulBlkTask *mmulTask = new MatrixMulBlkTask(numThreads);
  MatrixAccumTask * accumTask = new MatrixAccumTask((int)ceil((double)numThreads/2.0));

  OutputTask *outputTask = new OutputTask();

  int blkHeightMatB = genBMatTask->getNumBlocksRows();
  int blkWidthMatB = genBMatTask->getNumBlocksCols();

  int blkHeightMatA = genAMatTask->getNumBlocksRows();
  int blkWidthMatA = genAMatTask->getNumBlocksCols();

  MatrixDistributeRule *distributeRuleMatA = new MatrixDistributeRule(MatrixType::MatrixA);
  MatrixDistributeRule *distributeRuleMatB = new MatrixDistributeRule(MatrixType::MatrixB);

  MatrixLoadRule *loadRule = new MatrixLoadRule(blkWidthMatA, blkHeightMatA, blkWidthMatB, blkHeightMatB);
  MatrixAccumulateRule *accumulateRule = new MatrixAccumulateRule(blkWidthMatB, blkHeightMatA, blkWidthMatA);

  MatrixOutputRule *outputRule = new MatrixOutputRule(blkWidthMatB, blkHeightMatA, blkWidthMatA);

  auto distributeBk = new htgs::Bookkeeper<MatrixRequestData>();
  auto matMulBk = new htgs::Bookkeeper<MatrixBlockData<MatrixMemoryData_t>>();
  auto matAccumBk = new htgs::Bookkeeper<MatrixBlockData<double *>>();


  taskGraph->addGraphInputConsumer(distributeBk);
  taskGraph->addRule(distributeBk, genAMatTask, distributeRuleMatA);
  taskGraph->addRule(distributeBk, genBMatTask, distributeRuleMatB);

  taskGraph->addEdge(genAMatTask, matMulBk);
  taskGraph->addEdge(genBMatTask, matMulBk);

  taskGraph->addRule(matMulBk, mmulTask, loadRule);

  taskGraph->addEdge(mmulTask, matAccumBk);
  taskGraph->addRule(matAccumBk, accumTask, accumulateRule);
  taskGraph->addEdge(accumTask, matAccumBk);

  taskGraph->addRule(matAccumBk, outputTask, outputRule);
  taskGraph->addGraphOutputProducer(outputTask);

  taskGraph->addMemoryManagerEdge("matrixA", genAMatTask, mmulTask, new MatrixAllocator(blockSize, blockSize), 100, htgs::MMType::Static);
  taskGraph->addMemoryManagerEdge("matrixB", genBMatTask, mmulTask, new MatrixAllocator(blockSize, blockSize), 100, htgs::MMType::Static);

  taskGraph->incrementGraphInputProducer();

  EXPECT_EQ(10, taskGraph->getVertices()->size());
  EXPECT_EQ(1, taskGraph->getInputConnector()->getProducerCount());
  EXPECT_FALSE(taskGraph->isOutputTerminated());

  return taskGraph;
}

double * launchGraph(htgs::TaskGraph<MatrixRequestData, MatrixBlockData<double *>> *graph, int dim, int blockSize)
{
  int numBlksHeight = (int)ceil((double)dim / (double)blockSize);
  int numBlksWidth = (int)ceil((double)dim / (double)blockSize);

  double *result = new double[dim*dim];

  htgs::Runtime *runtime = new htgs::Runtime(graph);

  runtime->executeRuntime();

  for (int row = 0; row < numBlksHeight; row++)
  {
    for (int col = 0; col < numBlksWidth; col++)
    {
      MatrixRequestData *matrixA = new MatrixRequestData(row, col, MatrixType::MatrixA);
      graph->produceData(matrixA);

      MatrixRequestData *matrixB = new MatrixRequestData(row, col, MatrixType::MatrixB);
      graph->produceData(matrixB);

    }
  }

  graph->finishedProducingData();

  while (!graph->isOutputTerminated())
  {
    auto data = graph->consumeData();
    if (data != nullptr) {
      int row = data->getRequest()->getRow();
      int col = data->getRequest()->getCol();
      int width = data->getMatrixWidth();
      int height = data->getMatrixHeight();

      int startIndex = blockSize*col+blockSize*row*dim;

      double *resLoc = result + (blockSize*col+blockSize*row*dim);

      for (int i = 0; i < height; i++)
      {
        for (int j = 0; j < width; j++)
        {
          resLoc[i*dim+j] = data->getMatrixData()[i*width+j];
        }
      }
    }
  }

  runtime->waitForRuntime();
  delete runtime;

  return result;
}


void matMulGraphCreation()
{
  auto graph1 = createMatMulGraph(1, 16, 2, 10.0);
  auto graph2 = createMatMulGraph(2, 32, 8, 10.0);
  auto graph3 = createMatMulGraph(4, 64, 16, 10.0);
  auto graph4 = createMatMulGraph(8, 128, 32, 10.0);

  EXPECT_NO_FATAL_FAILURE(delete graph1);
  EXPECT_NO_FATAL_FAILURE(delete graph2);
  EXPECT_NO_FATAL_FAILURE(delete graph3);
  EXPECT_NO_FATAL_FAILURE(delete graph4);
}

void matMulGraphExecution(int dim, int blockSize, int numThreads, double initValue) {
  auto graph = createMatMulGraph(numThreads, dim, blockSize, initValue);
  double *result = launchGraph(graph, dim, blockSize);
  validateResults(result, dim, initValue);
}










