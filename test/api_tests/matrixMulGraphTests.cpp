
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

//
// Created by tjb3 on 4/18/16.
//
#include <gtest/gtest.h>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>

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


void validateResults(double *resultMatrix, size_t dim, double initValue)
{
  double *vals = new double[dim *dim];
  double *seqResult = new double[dim *dim];

  for (size_t i = 0; i < dim*dim; i++)
  {
    vals[i] = initValue;
  }

  for (size_t aRow = 0; aRow < dim; aRow++)
  {
    for (size_t bCol = 0; bCol < dim; bCol++)
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


htgs::TaskGraphConf<MatrixRequestData, MatrixBlockData<double *>> *createMatMulGraph(size_t numThreads, size_t dim, size_t blockSize, double initValue)
{
  auto taskGraph = new htgs::TaskGraphConf<MatrixRequestData, MatrixBlockData<double *>>();

  GenMatrixTask *genAMatTask = new GenMatrixTask(1, blockSize, dim, dim, "A", initValue);
  GenMatrixTask *genBMatTask = new GenMatrixTask(1, blockSize, dim, dim, "B", initValue);
  MatrixMulBlkTask *mmulTask = new MatrixMulBlkTask(numThreads);
  MatrixAccumTask * accumTask = new MatrixAccumTask((int)ceil((double)numThreads/2.0));

  OutputTask *outputTask = new OutputTask();

  size_t blkHeightMatB = genBMatTask->getNumBlocksRows();
  size_t blkWidthMatB = genBMatTask->getNumBlocksCols();

  size_t blkHeightMatA = genAMatTask->getNumBlocksRows();
  size_t blkWidthMatA = genAMatTask->getNumBlocksCols();

  std::shared_ptr<MatrixDistributeRule> distributeRuleMatA = std::make_shared<MatrixDistributeRule>(MatrixType::MatrixA);
  std::shared_ptr<MatrixDistributeRule> distributeRuleMatB = std::make_shared<MatrixDistributeRule>(MatrixType::MatrixB);

  std::shared_ptr<MatrixLoadRule> loadRule = std::make_shared<MatrixLoadRule>(blkWidthMatA, blkHeightMatA, blkWidthMatB, blkHeightMatB);
  std::shared_ptr<MatrixAccumulateRule> accumulateRule = std::make_shared<MatrixAccumulateRule>(blkWidthMatB, blkHeightMatA, blkWidthMatA);

  std::shared_ptr<MatrixOutputRule> outputRule = std::make_shared<MatrixOutputRule>(blkWidthMatB, blkHeightMatA, blkWidthMatA);

  auto distributeBk = new htgs::Bookkeeper<MatrixRequestData>();
  auto matMulBk = new htgs::Bookkeeper<MatrixBlockData<MatrixMemoryData_t>>();
  auto matAccumBk = new htgs::Bookkeeper<MatrixBlockData<double *>>();


  taskGraph->setGraphConsumerTask(distributeBk);
  taskGraph->addRuleEdge(distributeBk, distributeRuleMatA, genAMatTask);
  taskGraph->addRuleEdge(distributeBk, distributeRuleMatB, genBMatTask);

  taskGraph->addEdge(genAMatTask, matMulBk);
  taskGraph->addEdge(genBMatTask, matMulBk);

  taskGraph->addRuleEdge(matMulBk, loadRule, mmulTask);

  taskGraph->addEdge(mmulTask, matAccumBk);
  taskGraph->addRuleEdge(matAccumBk, accumulateRule, accumTask);
  taskGraph->addEdge(accumTask, matAccumBk);

  taskGraph->addRuleEdge(matAccumBk, outputRule, outputTask);
  taskGraph->setGraphProducerTask(outputTask);

  std::shared_ptr<MatrixAllocator> matrixAllocator = std::make_shared<MatrixAllocator>(blockSize, blockSize);

  taskGraph->addMemoryManagerEdge<double *>("matrixA", genAMatTask, matrixAllocator, 100, htgs::MMType::Static);
  taskGraph->addMemoryManagerEdge<double *>("matrixB", genBMatTask, matrixAllocator, 100, htgs::MMType::Static);


  EXPECT_EQ(10, taskGraph->getTaskManagers()->size());
  EXPECT_EQ(1, taskGraph->getInputConnector()->getProducerCount());
  EXPECT_FALSE(taskGraph->isOutputTerminated());

  return taskGraph;
}

double * launchGraph(htgs::TaskGraphConf<MatrixRequestData, MatrixBlockData<double *>> *graph, size_t dim, size_t blockSize)
{
  size_t numBlksHeight = (size_t)ceil((double)dim / (double)blockSize);
  size_t numBlksWidth = (size_t)ceil((double)dim / (double)blockSize);

  double *result = new double[dim*dim];

  htgs::TaskGraphRuntime *runtime = new htgs::TaskGraphRuntime(graph);

  runtime->executeRuntime();

  for (size_t row = 0; row < numBlksHeight; row++)
  {
    for (size_t col = 0; col < numBlksWidth; col++)
    {
      MatrixRequestData *matrixA = new MatrixRequestData(row, col, MatrixType::MatrixA);
      graph->produceData(matrixA);

      MatrixRequestData *matrixB = new MatrixRequestData(row, col, MatrixType::MatrixB);
      graph->produceData(matrixB);

    }
  }

  graph->decrementGraphProducer();

  while (!graph->isOutputTerminated())
  {
    auto data = graph->consumeData();
    if (data != nullptr) {
      size_t row = data->getRequest()->getRow();
      size_t col = data->getRequest()->getCol();
      size_t width = data->getMatrixWidth();
      size_t height = data->getMatrixHeight();

      size_t startIndex = blockSize*col+blockSize*row*dim;

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

void matMulGraphExecution(size_t dim, size_t blockSize, size_t numThreads, double initValue) {
  auto graph = createMatMulGraph(numThreads, dim, blockSize, initValue);
  double *result = launchGraph(graph, dim, blockSize);
  validateResults(result, dim, initValue);
}










