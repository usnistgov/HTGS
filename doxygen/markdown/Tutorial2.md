Tutorial 2 {#tutorial2}
========
[TOC]

In this tutorial we will be introducing two components of HTGS: (1) How to representing algorithms with dependencies
and (2) How to stay within memory limits.

The source code for the tutorial is found in "test/tutorial2"

To demonstrate these capabilities, we will be implementing the Hadamard product between
two matrices of equal size. There are three
attributes to keep an eye out for. First, transforming an algorithm's representation to enable
better pipelining and parallelism, second, adding memory edges to ensure the graph will not use all of
the available system memory, and third, how to represent a dependency to ensure data is loaded prior to execution.


Objectives {#objectives}
=======
1. How to encapsulate IData objects
2. How to use local variables in ITasks
3. How to exploit algorithm parallelism
  - Transforming the algorithm into independent parts (i.e. block decomposition)
  - Creating thread pools based on compute needs

4. How to pipeline tasks
  - With the goal of overlapping I/O with compute

5. How to manage dependencies
6. How to manage memory
  - Using a static memory manager
  - Using a user-managed memory manager
7. How to handle memory between a TaskGraph's task and the main thread


API Used {#api-used}
======

- \<htgs/api/IData.hpp\>
- \<htgs/api/ITask.hpp\>
- \<htgs/api/TaskGraph.hpp\>
- \<htgs/api/Runtime.hpp\>
- \<htgs/api/MemoryData.hpp\>
- \<htgs/api/IMemoryAllocator.hpp\>
- \<htgs/api/IMemoryReleaseRule.hpp\>
- \<htgs/api/IRule.hpp\>


Implementation {#implementation}
======

Below we analyze the various components used in defining the Hadamard product
algorithm. First, we will present the algorithm, then transform the algorithm into a dataflow graph. Finally the dataflow
 graph is altered into a TaskGraph.

The basic representation of the Hadamard algorithm is \f$A \circ B = C\f$, which computes the element-wise matrix multiplication
between two matrices that share the same dimensions.

Pseudo code:
~~~~
Given matrices A and B, each with the same dimensions

for_each(row) {
  for_each(col) {
    C[row][col] = A[row][col] * B[row][col];
  }
}
~~~~

One goal of HTGS is to exploit parallelism and aid in orchestrating pipeline workflows. In order to represent
an algorithm to exploit these components, we must understand the parallelism of the Hadamard product.

One thing that is clear is each loop iteration can be done in parallel, such that threads can process each element of C in parallel
without any need for synchronization.
When initiating these computations in a TaskGraph there is some overhead
with sending data between tasks and allocating data. Therefore, the best method for processing the Hadamard product
is to decompose the matrices into blocks and have each thread compute a Hadamard product on the matching row, column
matrix blocks. Below is a figure demonstrating the block decomposition of the Hadamard Product.

<img src="figures/hadamardProductFigure.png" alt="Block Size Impact" style="width: 35%;"/>

Given the block decomposition variation of the algorithm, we transform the Hadamard product into a dataflow graph.
For this implementation we assume the data has been saved to disk and distributed into separate files, where each file
represents a block of either matrix A or matrix B. A sample dataset can be downloaded from (link to dataset). Datasets
can be generated using the test/matrix-ops/testScalarMultiply.

Dataflow graph:

<img src="figures/tut2Hadamard-dataflow.png" style="width: 45%;"/>

TaskGraph:

<img src="figures/tut2Hadamard-taskgraph.png" style="width: 80%;"/>

Using the dataflow graph, we analyze the various data requirements that need to be created for the TaskGraph.
First, we need a data object to represent block read requests. The block read request should request the
row, column block that is to be read. Next, there are two reads required to compute one Hadamard product task, so
 a data object to encapsulate both reads into a single object, which stores each matrix will be needed. Finally, the result of the
 Hadamard product produces a block of data that represents matrix C's sub-matrix.

 Comparing the TaskGraph and dataflow representations, we merged the two read functions into a single read task. A flag within block request
 data can indicate whether to read from matrix A or B. Merging the two read functions is not required, and we could represent
 two separate read tasks for matrices A and B. One important component of HTGS is the input of a TaskGraph can only
 feed data to one Task. Therefore, if we were to separate the read into two Tasks, we would need a Bookkeeper to distribute
 the block requests between the two readers. In the next tutorial, we will demonstrate this concept.

 After reading data, a block of matrix data is then sent into a bookkeeper. The bookkeeper is
 used to manage programmer-defined rules that are synchronously accessed to manage the state of the computation and
 satisfy algorithmic data dependencies. In this example,
 we use a LoadRule to keep track of what blocks of matrices A and B have been loaded. When a matching row, column of both matrices
 are loaded, then the rule will produce data for the Hadamard product task.

 Every task in this TaskGraph can have one or more threads processing data. This allows the read task to pipeline
 with the Hadamard product task, overlapping the I/O of reading a matrix block with computing the
 Hadamard product of two blocks. There are two important concerns with this design. First, what is the ideal
 block size to use and second, what if the matrix being operated on cannot fit into memory.

 For determining the block size, we need to understand two performance attributes of HTGS. First, there is a small overhead involved with
 passing data between tasks, and second, we want to have enough data flowing in the task graph to enable pipelining.
 To demonstrate these components, below is a plot showing the impact of block size versus runtime.
  The source code that generated this plot can be found in "tests/tutorial2-a". Note: In this example we altered
  the read task to generate the matrix, which eliminated some uncertainty with disk read I/O. We repeated the
  experiment 5 times for each block size and reported the average runtime.

<img src="figures/blocksize-impact.png" alt="Block Size Impact" style="width: 40%;"/>

In the graph, we see that there is a penalty incurred with small block sizes. This is attributed to data overwhelming
  the FIFO queues and not enough computation per data element sent to the
Hadamard task. Increasing the block size improves the runtime, but if the block size becomes too large (such as the size of the entire matrix),
then the TaskGraph does not benefit from pipelining.

The other concern involves memory limitations. As each task executes, there is no throttling between tasks, so it is
possible that one task could allocate data faster than another task can process data.
 Using a  htgs::MemoryManager, we can connect two tasks with a memory edge to allow one task to throttle another task.
The role of the MemoryManager is to allocate, free, recycle, and update the state of htgs::MemoryData. The MemoryManager
is connected to two task's by using the htgs::TaskGraph::addMemoryManagerEdge function. This function specifies
 one task is requesting(getting) memory, and the other task is releasing memory. There is a memory pool
 associated with the memory manager, therefore if the memory pool is empty, then any task requesting memory will block and wait.
 The MemoryData will only be added back into the memory pool based on user-defined memory release rules. These
 rules are synchronously accessed by the MemoryManager and update the state of the memory, which should ultimately
 allow the memory to be released. If the memory release rule does not follow the data schedule, then it is possible
  for a TaskGraph to deadlock.

The figure below shows the TaskGraph annotated with memory edges. We will discuss the [memory manager implementation](#memory-manager)
in more detail in a later section.

<img src="figures/tut2Hadamard-taskgraph-memory.png" style="width: 90%;"/>

In the annotated TaskGraph, we connect two memory edges (for matrices A and B) between the read and Hadamard product tasks.
Using these edges, the Hadamard product task can throttle the reader. We connect one addition edge to the Hadamard
product task, which is released by the main thread. This connection allows the Hadamard product task to be throttled by
the main thread, which is useful if the post-processing computation cannot keep up with the data production rate of the
Hadamard product task.

In the next sections, we implement each of the components for the TaskGraph using HTGS.

## Data {#data}

There are three types of data that must be represented for the Hadamard product algorithm's TaskGraph.
First, MatrixRequestData requests a particular row, column block to be read from disk.
The read Task will produce MatrixBlockData that contains the matrix data that was read and the request.
Finally, when two MatrixBlockData's that are within the same row, column have been loaded, then the LoadRule will produce the
pair of blocks to the Hadamard Product task. We store these blocks in MatrixBlockMulData.

In this implementation, we use a variety of methods for encapsulating and reusing data objects. For example, the MatrixBlockData
holds onto the MatrixRequestData as well as MemoryData that is sent from the MemoryManager. The MatrixBlockMulData, holds
two instances of MatrixBlockData, one for matrix A and the other for matrix B. Encapsulating the data across objects that pass
from task to task reduces code size, while holding onto the metadata that is needed to compute the algorithm.

We also templatized the MatrixBlockData so we can represent multiple types of data to further reduce code size. In this particular case,
the read task is producing MatrixBlockData that holds MemoryData<double *>. Whereas, the Hadamard product task is producing
double * data. By representing MatrixBlockData with a template, we are able to define multiple types for a single data object implementation.
When connecting two tasks that use templatized data, the input and output types of the connected tasks must have the
same template type.

For tasks that have a memory manager edge attached to them, it is important to pass the shared pointer that is aquired from
the memory edge to the task that will be releasing that memory. This can be done by representing the shared pointer within the data.

### MatrixRequestData {#matrix-request}

~~~~{.c}
#include <htgs/api/IData.hpp>
enum class MatrixType {
  MatrixA,
  MatrixB,
  MatrixC
};

class MatrixRequestData : public htgs::IData {
 public:
  MatrixRequestData(int row, int col, MatrixType type) : row(row), col(col), type(type) { }

  int getRow() const {
    return row;
  }
  int getCol() const {
    return col;
  }
  MatrixType getType() const {
    return type;
  }

 private:
  int row;
  int col;
  MatrixType type;
};


~~~~

### MatrixBlockData {#matrix-data}

~~~~~~~~~~~~~~~~~{.c}
#include <htgs/api/MemoryData.hpp>

typedef std::shared_ptr<htgs::MemoryData<double *>> MatrixMemoryData_t;

template <class T>
class MatrixBlockData : public htgs::IData
{
 public:

  MatrixBlockData(const std::shared_ptr<MatrixRequestData> &request,
                  const T &matrixData,
                  int matrixWidth,
                  int matrixHeight) :
  request(request), matrixData(matrixData), matrixWidth(matrixWidth), matrixHeight(matrixHeight) { }

  const std::shared_ptr<MatrixRequestData> &getRequest() const {
    return request;
  }
  const T &getMatrixData() const {
    return matrixData;
  }
  int getMatrixWidth() const {
    return matrixWidth;
  }
  int getMatrixHeight() const {
    return matrixHeight;
  }

 private:
  std::shared_ptr<MatrixRequestData> request;
  T matrixData;
  int matrixWidth;
  int matrixHeight;
};
~~~~~~~~~~~~~~~~~

### MatrixBlockMulData {#matrix-mul-data}

~~~~~~~~~~~~~~~~~{.c}
#include <htgs/api/IData.hpp>
class MatrixBlockMulData : public htgs::IData {
 public:

  MatrixBlockMulData(const std::shared_ptr<MatrixBlockData<MatrixMemoryData_t>> &matrixA, const std::shared_ptr<MatrixBlockData<MatrixMemoryData_t>> &matrixB) :
  matrixA(matrixA), matrixB(matrixB) { }

  const std::shared_ptr<MatrixBlockData<MatrixMemoryData_t>> &getMatrixA() const {
    return matrixA;
  }
  const std::shared_ptr<MatrixBlockData<MatrixMemoryData_t>> &getMatrixB() const {
    return matrixB;
  }

 private:
  std::shared_ptr<MatrixBlockData<MatrixMemoryData_t>> matrixA;
  std::shared_ptr<MatrixBlockData<MatrixMemoryData_t>> matrixB;
};
~~~~~~~~~~~~~~~~~
### Notes {#data-notes}

- IData can represent a wide variety of data
  + Encapsulating multiple IData objects into a single IData object
  + Templatizing IData
- MemoryData that is allocated from an ITask should be passed within IData to the ITask that is responsible for releasing that MemoryData
- Enums stored in IData can be used to switch between various operations for an ITask


## Tasks {#tasks}

For the Hadamard product algorithm, we need to implement two ITasks: Read and Hadamard product.

The ReadMatrixTask is responsible for reading matrices A or B depending on the type of MatrixRequestData
passed to the read task. The matrix data is saved to disk such that each block is its own file. If
a new block size is specified, then the matrix data will have to be regenerated. There are multiple
 techniques for reading blocks of data, such as using memory mapped files. With this approach we ensure that
 each file is accessed by a separate thread. In the next tutorial, we will show multiple variations of this such as using
   memory mapped files for I/O.
 The directory structure for the data is stored as follows: "data/tutorial2/4096x4096blksize1024/matrix<A/B>".
 Prior to reading data from disk, the ReadTask must allocate the memory needed for storing the data. This memory is
 acquired from the memory edge from either matrixA or matrixB MemoryManagers. To get memory from the memory manager,
 we use the htgs::ITask::memGet function. If an ITask calls memGet and it was not declared the memory getter for the
 MemoryManager edge, then the program will report an assertion failed or segementation fault.
 To check if a memory edge exists use the htgs::ITask::hasMemGetter function.

 The HadamardProductTask is responsible for the scalar product of two matrices. These matrices are stored
 in the MatrixBlockMulData object. Prior to multiplying the matrices, the HadamardProductTask needs
 to allocate memory for the resulting matrix, which will be added to the output of the TaskGraph. In this implementation
 we will demonstrate the user-managed memory manager. The various types and functionality of the memory managers is described
 in the [MemoryManager](#memory-manager) section.
 After the task has completed multiplying matrices A and B together, the Hadamard task must
 release the memory for matrices A and B so that the memory manager can recycle their pointers for the read task. To
  release memory use the htgs::ITask::memRelease function. As described for
 memGet, if the ITask calls memRelease and it was not declared the memory releaser for the MemoryManager edge, then
 the program will report an assertion failed or segmentation fault. To check if a memory release edge exists use htgs::ITask::hasMemReleaser.

Each of the tasks within a TaskGraph have at least one thread responsible for executing their functionality.
The ITask(numThreads) constructor is used to specify the number of threads for an ITask. The Runtime uses this parameter
to create copies of the ITask and bind each copy to separate TaskSchedulers, which is then bound to separate threads. Each TaskScheduler
shares the same input and output Connectors, which have thread safe blocking queues. Therefore, when data enters
a connector, one of the TaskSchedulers waiting for that data will wake up and begin processing the data.

### ReadMatrixTask {#read-matrix-task}

~~~~~~~~~~{.c}
#include <htgs/api/ITask.hpp>
#include <cmath>
#include "../memory/MatrixMemoryRule.h"

class ReadMatrixTask : public htgs::ITask<MatrixRequestData, MatrixBlockData<MatrixMemoryData_t>>
{
 public:
  ReadMatrixTask(int numThreads, int blockSize, int fullMatrixWidth, int fullMatrixHeight, std::string directory) :
      ITask(numThreads), blockSize(blockSize), fullMatrixHeight(fullMatrixHeight), fullMatrixWidth(fullMatrixWidth), directory(directory)
  {
    numBlocksRows = (int)ceil((double)fullMatrixHeight / (double)blockSize);
    numBlocksCols = (int)ceil((double)fullMatrixWidth / (double)blockSize);
  }
  virtual ~GenMatrixTask() { }
  virtual void initialize(int pipelineId, int numPipeline) { }
  virtual void shutdown() { }

  virtual void executeTask(std::shared_ptr<MatrixRequestData> data) {
    std::string matrixName;

    switch (data->getType())
    {
      case MatrixType::MatrixA: matrixName = "matrixA"; break;
      case MatrixType::MatrixB: matrixName = "matrixB"; break;
      case MatrixType::MatrixC: return;
    }
    MatrixMemoryData_t matrixData = this->memGet<double *>(matrixName, new MatrixMemoryRule(1));

    int row = data->getRow();
    int col = data->getCol();

    int matrixWidth;
    int matrixHeight;

    if (col == numBlocksCols-1 && fullMatrixWidth % blockSize != 0)
      matrixWidth = fullMatrixWidth % blockSize;
    else
      matrixWidth = blockSize;

    if (row == numBlocksRows-1 && fullMatrixHeight % blockSize != 0)
      matrixHeight = fullMatrixHeight % blockSize;
    else
      matrixHeight = blockSize;

    std::string fileName(directory + "/" + matrixName + "/" + std::to_string(row) + "_" + std::to_string(col));

    // Read data
    std::ifstream file(fileName, std::ios::binary);

    file.read((char *)matrixData->get(), sizeof(double) * matrixWidth * matrixHeight);

    addResult(new MatrixBlockData<MatrixMemoryData_t>(data, matrixData, matrixWidth, matrixHeight));

  }
  virtual std::string getName() {
    return "ReadMatrixTask";
  }
  virtual htgs::ITask<MatrixRequestData, MatrixBlockData<MatrixMemoryData_t>> *copy() {
    return new ReadMatrixTask(this->getNumThreads(), blockSize, fullMatrixWidth, fullMatrixHeight, directory);
  }
  virtual bool isTerminated(std::shared_ptr<htgs::BaseConnector> inputConnector) {
    return inputConnector->isInputTerminated();
  }

  int getNumBlocksRows() const {
    return numBlocksRows;
  }
  int getNumBlocksCols() const {
    return numBlocksCols;
  }
 private:
  int blockSize;
  int fullMatrixWidth;
  int fullMatrixHeight;
  int numBlocksRows;
  int numBlocksCols;
  std::string directory;

};
~~~~~~~~~~

### HadamardProductTask {#hadamard-product-task}

~~~~~~~~~~{.c}
#include "../data/MatrixBlockMulData.h"
#include "../data/MatrixBlockData.h"
#include <htgs/api/ITask.hpp>

class HadamardProductTask : public htgs::ITask<MatrixBlockMulData, MatrixBlockData<double *>>
{
 public:
  HadamardProductTask(int numThreads) : ITask(numThreads) {}
  virtual ~HadamardProductTask() { }
  virtual void initialize(int pipelineId, int numPipeline) { }
  virtual void shutdown() { }
  virtual void executeTask(std::shared_ptr<MatrixBlockMulData> data) {

    auto matAData = data->getMatrixA();
    auto matBData = data->getMatrixB();

    MatrixMemoryData_t matrixA = matAData->getMatrixData();
    MatrixMemoryData_t matrixB = matBData->getMatrixData();

    int width = matAData->getMatrixWidth();
    int height = matAData->getMatrixHeight();

    // This function will block if memory is not ready to be allocated
    this->allocUserManagedMemory("outputMem");

    double *result = new double[width*height];

    for (int i = 0; i < matAData->getMatrixWidth() * matAData->getMatrixHeight(); i++)
    {
      result[i] = matrixA->get()[i] * matrixB->get()[i];
    }

    auto matRequest = matAData->getRequest();

    std::shared_ptr<MatrixRequestData> matReq(new MatrixRequestData(matRequest->getRow(), matRequest->getCol(), MatrixType::MatrixC));

    addResult(new MatrixBlockData<double *>(matReq, result, width, height));

    this->memRelease("matrixA", matrixA);
    this->memRelease("matrixB", matrixB);
  }
  virtual std::string getName() {
    return "HadamardProductTask";
  }
  virtual htgs::ITask<MatrixBlockMulData, MatrixBlockData<double *>> *copy() {
    return new HadamardProductTask(this->getNumThreads());
  }
  virtual bool isTerminated(std::shared_ptr<htgs::BaseConnector> inputConnector) {
    return inputConnector->isInputTerminated();
  }
};
~~~~~~~~~~
### Notes {#task-notes}

- Use the htgs::ITask::memGet function to get memory from a memory edge (htgs::ITask::memRelease to release)
  + htgs::TaskGraph::addMemoryManagerEdge declares the ITask responsible for getting and releasing memory
  + If an ITask calls memGet and it is not declared the memory getter from the TaskGraph, then the program will segmentation fault
- Specify a pool of threads for an ITask using the ITask(numThreads) constructor

## Managing Dependencies with a Bookkeeper and IRules {#bookkeeper}

The htgs::Bookkeeper is an ITask that is defined with the HTGS API. It is used to manage dependencies, as well as provide a mechanism
for branching the flow of data in multiple directions. Each branch from a Bookkeeper can have different output data types.
The bookkeeper implements this functionality by using a list of RuleManagers.
When data enters a bookkeeper, the data is passed to each htgs::RuleManager. The RuleManager is attached to
a consumer ITask which will process the output data of the RuleManager. The RuleManager holds onto user-defined [htgs::IRules](@ref htgs::IRule),
which are used to determine whether data entering the RuleManager will pass or create data to the RuleManager's consumer.
Each IRule is synchronously accessed using a mutex, such that when an IRule is processing data it is guaranteed that there will
be only one thread at a time processing that rule. Because of this attribute, the same IRule instance can be safely shared
among multiple Bookkeepers and RuleManagers. This is particularly important for the htgs::ExecutionPipeline, which will create
a copy of an entire TaskGraph. We will demonstrate the functionality of ExecutionPipelines in a later tutorial.

Below is a diagram outlining the functionality of the Bookkeeper ITask.

<img src="figures/taskBookkeeper.png" style="width: 50%;"/>

To use the bookkeeper, an htgs::IRule must be defined. The IRule represents a rule that processes input
 and decides when to pass data to an ITask. For the Hadamard product algorithm, we use the MatrixLoadRule to determine
 when a matching row, column block for both matrices A and B have been loaded. When it detects that the condition is satisfied,
 then the rule will produce MatrixBlockMulData for the Hadamard task. To add a bookkeeper rule connection to a consumer use
 the htgs::TaskGraph::addRule function.

 The IRule has an input and output type. The input type must match the Bookeeper type and the output type must match the input
 type of the consumer ITask.

 There are three pure virtual functions that define the behavior for each IRule:

 1. htgs::IRule::isRuleTerminated
   + Determines if the rule is terminated or not. If the RuleManager's input connector is finished producing data, then
   the IRule will also be terminated. This function is not required to terminate an IRule. When all IRule's for a RuleManager
   have terminated, the Connector to the consumer ITask is notified.
 2. htgs::IRule::shutdownRule
   + Called when the IRule is terminating. If there is memory allocated for an IRule, that memory should NOT be deallocated
   in this function. Instead, memory deallocation should take place in the IRule's  destructor.
 3. htgs::IRule::applyRule
   - Applies the rule on input data. To add data to the consumer ITask attached to the rule, use htgs::IRule::addResult.

 To aid in managing the state of computation, the IRule class has helper functions to allocate a htgs::StateContainer.
 The StateContainer is a templatized class that stores a one or two dimensional array of data. The class contains three
 functions for checking, updating, and fetching the data: get, set, and has. htgs::StateContainer::get will retrieve the data stored within the container;
 htgs::StateContainer::set stores data at the specified location within the container, and htgs::StateContainer::has checks whether data exists at a location or not.
 The MatrixLoadRule uses the StateContainer to check for its dependencies and pass the pointer for that dependency to
 the HadamardProduct ITask.


### MatrixLoadRule {#load-rule}
~~~~{.c}
#include <htgs/api/IRule.hpp>
#include "../data/MatrixBlockData.h"
#include "../data/MatrixBlockMulData.h"

class MatrixLoadRule: public htgs::IRule<MatrixBlockData<MatrixMemoryData_t>, MatrixBlockMulData> {

 public:
  MatrixLoadRule(int blockWidth, int blockHeight) {
    this->blockWidth = blockWidth;
    this->blockHeight = blockHeight;

    this->arrayAState = this->allocStateContainer(blockWidth, blockHeight);
    this->arrayBState = this->allocStateContainer(blockWidth, blockHeight);
  }

  ~MatrixLoadRule() {
    delete arrayAState;
    delete arrayBState;
  }

  bool isRuleTerminated(int pipelineId) {
    return false;
  }

  void shutdownRule(int pipelineId) { }

  void applyRule(std::shared_ptr<MatrixBlockData<MatrixMemoryData_t>> data, int pipelineId) {
    std::shared_ptr<MatrixRequestData> request = data->getRequest();

    switch (request->getType()) {
      case MatrixType::MatrixA:
        this->arrayAState->set(request->getRow(), request->getCol(), data);

        if (this->arrayBState->has(request->getRow(), request->getCol())) {
          addResult(new MatrixBlockMulData(data, this->arrayBState->get(request->getRow(), request->getCol())));
        }
        break;
      case MatrixType::MatrixB:
        this->arrayBState->set(request->getRow(), request->getCol(), data);

        if (this->arrayAState->has(request->getRow(), request->getCol())) {
          addResult(new MatrixBlockMulData(this->arrayAState->get(request->getRow(), request->getCol()), data));
        }
        break;
      case MatrixType::MatrixC:
        break;
    }
  }

  std::string getName() {
    return "MatrixLoadRule";
  }

 private:
  int blockWidth;
  int blockHeight;
  htgs::StateContainer<std::shared_ptr<MatrixBlockData<MatrixMemoryData_t>>> *arrayAState;
  htgs::StateContainer<std::shared_ptr<MatrixBlockData<MatrixMemoryData_t>>> *arrayBState;
};
~~~~

### Notes {#bookkeeper-notes}
- The htgs::Bookkeeper is an ITask defined by the HTGS API that manages depedencies between one or more consumer ITasks.
- A htgs::IRule defines the connection between a Bookkeeper and a consumer ITask.
  + There can be multiple IRules for each Bookkeeper to consumer connection, these connections are managed by the htgs::RuleManager
- The htgs::IRule input type must match the htgs::Bookkeeper type, and the htgs::IRule output type must match the consumer ITask's input type
- htgs::IRule's are accessed synchronously
- Every htgs::IRule implements three functions: isRuleTerminated, shutdownRule, and applyRule
  + shutdownRule should NOT deallocate memory. Memory deallocation should be done in the IRule's destructor
- htgs::IRule::addResult should be used within htgs::IRule::applyRule to pass data to the IRule's consumer task
- The htgs::StateContainer can be used to help manage the state of computation for input data or other types of state data



## Throttling Tasks with a Memory Manager {#memory-manager}
The htgs::MemoryManager is an ITask that is created when connecting two ITasks together as a memory edge. A memory edge
is similar to any other edge,  except it connects two ITasks directly rather than through a TaskScheduler. When connecting
 two ITask's, one task is declared the memGetter and the other task is the memReleaser. The memGetter requests data from
 the output htgs::Connector of the MemoryManager, and the memReleaser sends data to the input htgs::Connector for the MemoryManager.
 Each memory edge contains a name associated with the edge. This way a single ITask can have multiple memory edges
 connected to it assuming each edge has a unique name.

Every ITask has the capabilities to get memory data from a memory manager edge. Using the htgs::TaskGraph::addMemoryManagerEdge
function, two ITasks will be connected by a MemoryManager. The ITask that is getting memory must be in
 the graph instance that is used to call the addMemoryManagerEdge function. The parameters for the addMemoryManagerEdge function
 specifies the name of the edge, the memGetter ITask, the memReleaser ITask, the IMemoryAllocator, the memory pool size, and the
 type of memory manager to use.
 The name that is specified by the addMemoryManagerEdge function must be used by the ITasks that will be releasing/getting memory
 to/from that MemoryManager.

 The memory allocator defines the type of data to be allocated, as well as how the memory is allocated/freed. The type
  parameter defined in the allocator is used as the type template when getting memory for an ITask. The
 memory pool size is used to throttle the ITask that is getting memory. If the pool is empty, then the ITask's threads
 will wait until memory has been released to the memory pool for the MemoryManager.

 When an ITask gets memory, it specifies the type of memory, the name of the memory manger edge, and that memory's
 htgs::IMemoryReleaseRule. The IMemoryReleaseRule is a programmer-defined interface that describes how/when that memory data can
 be released. If there is a possibility that the memory edge was not added, then the ITask can check if
  the memory edge exists using the [ITask::hasMemGetter](@ref htgs::ITask::hasMemGetter) or
  [ITask::hasMemReleaser](@ref htgs::ITask::hasMemReleaser) functions. If an ITask attempts to get or release memory from
  an edge that does not exist, then the program will fail an assertion or segmentation fault.

  There are three types of MemoryManagers: Static, Dynamic, and UserManaged. Each MemoryManager type modifies how/when
  memory is allocated and freed.

  All MemoryManager types allocates a pool of htgs::MemoryData during the ITask::initialize phase. However, the Static
    MemoryManager will also allocate the internal memory that the MemoryData is holding. The Static MemoryManager will
    release this memory once the MemoryManager is shutdown.

  In the next sections, we will go into the details as to how each of these MemoryManager types vary and process MemoryData.

### Static Memory Manager {#static-mm}
The static MemoryManager is a memory manager that recycles memory when it is released. All of the memory for the memory pool is allocated once
when the MemoryManager is initialized and freed when the MemoryManager is shutdown. The diagram below shows how
MemoryData is processed when an ITask releases memory. First, when MemoryData enters the MemoryManager,
the MemoryData's state is updated using the htgs::IMemoryReleaseRule::memoryUsed function.

Next, the MemoryManager checks the htgs::IMemoryReleaseRule::canReleaseMemory function to determine whether the state indicates the memory
can be freed or not. If the state indicates it can be freed, then the MemoryData is inserted into the MemoryPool. If it is
not freed, then the MemoryData will not be recycled. Next, the MemoryPool is emptied into the MemoryManager's output Connector
so the ITask getting memory can wake up and acquire MemoryData.

<img src="figures/staticMemoryManager.png" style="width: 100%;"/>

### Dynamic Memory Manager {#dynamic-mm}
The dynamic MemoryManager contains a similar structure as the static MemoryManager with how it processes MemoryData.
The primary difference is when the dynamic
MemoryManager allocates and frees memory. In the static MemoryManager all memory is allocated during initialization and freed
during shutdown. The dynamic MemoryManager allocates and frees memory on-the-fly. As shown in the diagram below, when MemoryData
is sent from the memReleaser, if the memory can be released, then the MemoryManager will free the memory that the MemoryData
is managing. When the memGetter acquires memory from the MemoryManager, the ITask will allocate the memory that the MemoryData
 is managing prior to returning the MemoryData to the ITask. Using these mechanisms the memory is allocated and freed
 on demand.

<img src="figures/dynamicMemoryManager.png" style="width: 100%;"/>

### User Managed Memory Manager {#user-managed-mm}

The user managed MemoryManager assumes the programmer is taking the necessary precautions with memory allocation, state, and freeing of memory.
The MemoryManager in this case acts purely as a throttling mechanism to aid in keeping track of how many memory allocations are
in flight within a TaskGraph. The ITask that is allocating memory calls htgs::ITask::allocUserManagedMemory to get permission
to allocate memory. If there is no memory within the MemoryPool of the user managed MemoryManager, then this function will block
until the memReleaser releases memory. There are no IMemoryReleaseRules associated with MemoryManagers, and any
memory release metadata must be managed by the programmer.

<img src="figures/userMemoryManager.png" style="width: 90%;"/>


Within the htgs::MemoryData there are two functions that must be defined to use a MemoryManager edge:
(1) htgs::IMemoryAllocator, and (2) htgs::IMemoryReleaseRule. The IMemoryAllocator is specified when creating the memory edge,
 while the IMemoryReleaseRule is added to MemoryData when the memGetter gets memory. For the HadamardProduct task, we
  define the MatrixAllocator and MatrixMemoryRule as the htgs::IMemoryAllocator and htgs::IMemoryReleaseRule, respectively.
  Below is the code that defines these classes.

 The MatrixAllocator creates a one-dimensional array with the specified size using the new operator and releases the
 memory using the delete[] operator.

 The MatrixMemoryRule uses a release count to determine when the memory is ready to be released. When the memoryGetter
  requests memory, a new instance of the MatrixMemoryRule is defined and bound to the MemoryData that is fetched. This
  IMemoryReleaseRule is then used by the MemoryManager by calling the htgs::IMemoryReleaseRule::memoryUsed() function, which
  decrements the release count. The MemoryManager then checks if the memory can be released with the
  htgs::IMemoryReleaseRule::canReleaseMemory(), which returns true if the release count is zero.

  Each matrix block in the HadamardProduct is only used once, so after the specified row, column has been computed, then the memory associated
  with that computation can be released. This aspect is represented by defining the release count for the MatrixMemoryRule
  to be one.

### MatrixAllocator {#matrix-allocator}
~~~~ {.c}
#include <htgs/api/IMemoryAllocator.hpp>

class MatrixAllocator : public htgs::IMemoryAllocator<double *> {
 public:
  MatrixAllocator(int width, int height) : IMemoryAllocator((size_t) width * height) { }

  double *memAlloc(size_t size) {
    double *mem = new double[size];
    return mem;
  }

  double *memAlloc() {
    double *mem = new double[this->size()];
    return mem;
  }

  void memFree(double *&memory) {
    delete[] memory;
  }

};
~~~~

### MatrixMemoryRule {#matrix-memory-rule}
~~~~ {.c}
#include <htgs/api/IMemoryReleaseRule.hpp>

class MatrixMemoryRule: public htgs::IMemoryReleaseRule {
 public:

  MatrixMemoryRule(int releaseCount) : releaseCount(releaseCount) {
  }

  void memoryUsed() {
    releaseCount--;
  }

  bool canReleaseMemory() {
    return releaseCount == 0;
  }

 private:
  int releaseCount;
};
~~~~

### Notes {#memorymanagement-notes}
- MemoryManager manages the memory that is used/recycled within a TaskGraph as well as to help throttle a TaskGraph to
ensure the TaskGraph does not allocate too much memory.
- There are three types of memory managers: Static, Dynamic, and UserManaged
    + Type is defined when adding the memory edge to the htgs::TaskGraph
    + Primary difference is how/when memory is allocated/freed
- When connecting a memory getter with a memory releaser, the memory getter must already be added to the TaskGraph
prior to declaring the memory getter for the given TaskGraph.
    + The memory releaser does not have to be within the same TaskGraph
    + The memory releaser can be bound to a TaskGraph to be accessed by the main thread (or another separate thread that has an instance of the TaskGraph)
- The htgs::IMemoryAllocator defines what type of memory is allocated, how the memory is allocated, and how the memory is freed
    + Use the type defined for the IMemoryAllocator<Type> when getting memory from within an ITask using htgs::ITask::memGet<Type>()
- The htgs::IMemoryReleaseRule is used to define the state of htgs::MemoryData to help determine when the htgs::MemoryManager can recycle/free memory
    + The IMemoryReleaseRule is added to htgs::MemoryData when an ITask gets memory.
    + Each htgs::MemoryData should have a separate instances of htgs::IMemoryReleaseRule objects (a new instance per MemoryData)
- The htgs::IMemoryReleaseRule should follow the data flow behavior of the TaskGraph to ensure that the memory gets released.
    + If memory does not get released and the memory getter is waiting for memory, then deadlock can occur.
    + There should be enough elements in the memory pool to progress the TaskGraph so that the IMemoryReleaseRule can allow data to be released.
- The memory edge that connects two ITasks is a separate edge from the main TaskGraph connections.
    + The main connections use TaskSchedulers, the memory edge directly connects two ITasks together.


## Creating and Executing the TaskGraph {#create-and-execute-taskgraph}

As shown in Tutorial1, we use the TaskGraph to connect all our components that can then be executed
using threads. The new TaskGraph functions we will be using to connect these components are: htgs::TaskGraph::addRule(),
htgs::TaskGraph::addMemoryManagerEdge(), and htgs::TaskGraph::addGraphUserManagedMemoryManagerEdge().

Below is the main function for computing the HadamardProduct using HTGS. The matrix dimensions are 1024x1024 and a block size of
256. Modifying the block size or matrix dimensions will require recreating the matrix data with the
test/matrix-ops/generateMatrix.cpp test case.

To add a dependency IRule to a graph, use the htgs::TaskGraph::addRule function. This requires the creation of a htgs::Bookkeeper,
the htgs::ITask consumer task, and the htgs::IRule that produces data for the consumer.

There are a number of ways for adding memory edges to a TaskGraph. Prior to adding any memory edges, the ITask that is
acting as the memGetter must be added to the graph prior to adding the memory edge to that graph.

The functions for adding a memory edge to a graph are defined below:

1) htgs::TaskGraph::addUserManagedMemoryManagerEdge(std::string name, BaseITask *memGetter, BaseITask *memReleaser, int memoryPoolSize)
    - Creates a memory edge that is managed by the programmer/user. The MemoryManager acts to simply manage how much memory has been
    allocated. Does not use any allocators. Within an ITask use htgs::ITask::allocUserManagedMemory to get permission to
    allocate memory and htgs::ITask::memRelease(name, pipelineId).

2) htgs::TaskGraph::addMemoryManagerEdge(std::string name, BaseITask *memGetter, BaseITask *memReleaser, IMemoryAllocator<V> *allocator, int memoryPoolSize, htgs::MMType type)
    - Creates a memory edge that is managed based on htgs::IMemoryReleaseRule's attached to htgs::MemoryData. The htgs::IMemoryAllocator<V> defines
    how memory is allocated and freed and determines the type of the memory being allocated. The memory pool size determines the
    number of elements in the memory pool. If the MemoryManager is static, then all of the memory will be allocated during initialization.

3) htgs::TaskGraph::addGraphUserManagedMemoryManagerEdge(std::string name, BaseITask *memGetter, int memoryPoolSize)
    - Similar to htgs::TaskGraph::addUserManagedMemoryManagerEdge, except the memory releaser exists outside of the task graph.
    This methodology is demonstrated in the HadamardProduct main function when processing data produced by the TaskGraph.

4) htgs::TaskGraph::addGraphMemoryManagerEdge(std::string name, BaseITask *memGetter, IMemoryAllocator<V> *allocator, int memoryPoolSize, htgs::MMType type)
    - Similar to htgs::addMemoryManagerEdge, except the memory releaser exists outside of the task graph.

5) htgs::TaskGraph::addMemoryManagerEdge(BaseITask *memGetter, BaseITask *memReleaser, MemoryManager<V> *memoryManager, bool ignoreMemGetterErrors)
    - Creates a memory manager edge with the provided MemoryManager. This allows for additional special types of memory managers,
    such as MemoryManager's allocating memory for other devices such as with OpenCL.

As shown there are functions for designating the memory releaser to be attached to the TaskGraph. This means that any user
with an instance of a TaskGraph can release memory for a memGetter Task within the TaskGraph. Here are the functions that
 can be used to interact with the memory releaser attached to the TaskGraph when using the htgs::TaskGraph::addGraphUserManagedMemoryManagerEdge
 and htgs::TaskGraph::addGraphMemoryManagerEdge.

1) htgs::TaskGraph::hasMemReleaser(std::string name)
    - Checks if the TaskGraph is a memory releaser for the named memory edge.

2) htgs::TaskGraph::memRelease(std::string name, std::shared_ptr<MemoryData<V>> memory)
    - Releases memory for the named memory edge. (Static and Dynamic MMType)

3) htgs::TaskGraph::memRelease(std::string name, int pipelineId)
    - Releases memory for the named memory edge. (UserManaged MMType)


Belows is the source code implementation for setup, construction of the task TaskGraph, executing the TaskGraph, and processing
the output of the TaskGraph.

### Main function (Hadamard Product) {#main-function}

~~~~~~~~~~{.c}
#include <htgs/api/TaskGraph.hpp>
#include <htgs/api/Runtime.hpp>
#include "data/MatrixRequestData.h"
#include "data/MatrixBlockData.h"
#include "tasks/ReadMatrixTask.h"
#include "memory/MatrixAllocator.h"
#include "rules/MatrixLoadRule.h"
#include "tasks/HadamardProductTask.h"
#include "../api/SimpleClock.h"

int main()
{
  int width = 1024;
  int height = 1024;
  int blockSize = 256;
  int numReadThreads = 1;
  int numProdThreads = 10;

  std::string directory("data/tutorial2/1024x1024blksize256");

  ReadMatrixTask *readMatTask = new ReadMatrixTask(numReadThreads, blockSize, width, height, directory);
  MatrixMulBlkTask *prodTask = new MatrixMulBlkTask(numProdThreads);

  int numBlocksCols = readMatTask->getNumBlocksCols();
  int numBlocksRows = readMatTask->getNumBlocksRows();

  MatrixLoadRule *loadRule = new MatrixLoadRule(numBlocksCols, numBlocksRows);
  auto bookkeeper = new htgs::Bookkeeper<MatrixBlockData<MatrixMemoryData_t>>();

  auto taskGraph = new htgs::TaskGraph<MatrixRequestData, MatrixBlockData<double *>>();

  taskGraph->addGraphInputConsumer(readMatTask);
  taskGraph->addEdge(readMatTask, bookkeeper);
  taskGraph->addRule(bookkeeper, prodTask, loadRule);
  taskGraph->addGraphOutputProducer(prodTask);

  taskGraph->addGraphUserManagedMemoryManagerEdge("outputMem", prodTask, 50);

  taskGraph->addMemoryManagerEdge("matrixA", readMatTask, prodTask, new MatrixAllocator(blockSize, blockSize), 100, htgs::MMType::Static);
  taskGraph->addMemoryManagerEdge("matrixB", readMatTask, prodTask, new MatrixAllocator(blockSize, blockSize), 100, htgs::MMType::Static);

  taskGraph->incrementGraphInputProducer();

  htgs::Runtime *runtime = new htgs::Runtime(taskGraph);

  SimpleClock clk;
  clk.start();

  runtime->executeRuntime();

  for (int row = 0; row < numBlocksRows; row++)
  {
    for (int col = 0; col < numBlocksCols; col++)
    {
      MatrixRequestData * matrixA = new MatrixRequestData(row, col, MatrixType::MatrixA);
      MatrixRequestData * matrixB = new MatrixRequestData(row, col, MatrixType::MatrixB);

      taskGraph->produceData(matrixA);
      taskGraph->produceData(matrixB);
    }
  }

  taskGraph->finishedProducingData();

  while (!taskGraph->isOutputTerminated())
  {
    auto data = taskGraph->consumeData();

    if (data != nullptr) {
      std::cout << "Result received: " << data->getRequest()->getRow() << ", " << data->getRequest()->getCol() <<std::endl;
      double *mem = data->getMatrixData();
      delete [] mem;

      taskGraph->memRelease("outputMem", 0);
    }
  }

  taskGraph->finishReleasingMemory();

  runtime->waitForRuntime();

  clk.stopAndIncrement();

  std::cout << "Finished running in " << clk.getAverageTime(TimeVal::MILLI) << " ms" << std::endl;

  delete runtime;
}
~~~~~~~~~~

Sample execution:
~~~~
./tutorial2
Result received: 1, 0
Result received: 0, 2
Result received: 1, 3
Result received: 0, 3
Result received: 2, 0
Result received: 0, 0
Result received: 0, 1
Result received: 1, 2
Result received: 1, 1
Result received: 2, 1
Result received: 2, 2
Result received: 3, 1
Result received: 3, 0
Result received: 2, 3
Result received: 3, 2
Result received: 3, 3
Finished running in 5.67444 ms
~~~~


### Notes {#taskgraph-notes}
- The htgs::TaskGraph::addRule is used to add an htgs::IRule to a TaskGraph
    + Manages dependencies
    + Determines when data is sent to a consumer ITask.
- Multiple functions for adding the memory edge to a TaskGraph
    + Allows for custom MemoryManager tasks
    + Support for TaskGraph memory releasers
- For memory edges, the memory pool size parameter is import to specify based on amount of memory available and the dataflow behavior
    in relation to the memory usage behavior
    + If not enough memory is available to allow for memory to be released, then the TaskGraph will deadlock
    + The more memory available, the more data elements that can flow through the TaskGraph to improve pipelining.
- Similar to the ITask API to operate with memory edges, the TaskGraph has a similar API to manage the memory edges attached
     to the TaskGraph.


Summary {#summary}
======

In this tutorial, we looked at parallelism/pipelining, handling dependencies and memory management.
- A method for representing an algorithm to maximize pipelining and parallelism
- More advanced methods for representing htgs::IData
- How to specify a pool of threads for an htgs::ITask
- The functions needed to define a dependency using htgs::IRule
- The various types of memory managers
- How to use the memory management system with htgs::IMemoryReleaseRule and htgs::IMemoryAllocator
- The importance of understanding memory usage behavior in relationship to memory edges to prevent TaskGraph deadlocks.

In the next tutorial, we will be implementing an in-core matrix multiplication using block decomposition and the openblas API.
The tutorial will introduce how to use computational functions from other APIs (openblas), using a Bookkeeper that branches execution,
and how to define multiple memory release edges for the same memory manager. There will be a couple iterations of tutorial3 to
demonstrate alternative methods for allocating memory, such as using memory mapped files versus the file structure shown
in this tutorial. In tutorial 4, we will take this computation and set it up as an out-of-core algorithm.

Additional information:
- Generating the a dot file using htgs::TaskGraph::writeDotToFile after executing a TaskGraph will create a dot file that
visualizes all of the threading that takes place within the htgs::Runtime.
