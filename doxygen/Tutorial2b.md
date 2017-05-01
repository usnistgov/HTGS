Tutorial 2B {#tutorial2b}
========
[TOC]

In this tutorial we expand on [Tutorial 2A](@ref tutorial2a) by adding two components to the previous task graph: (1) Reading the matrices from disk and (2) Using memory managers to stay within memory limits. 

The source code for the tutorial is found [here](location updated).

Objectives {#tut2b-objectives}
=======
1. How to pipeline tasks
  - With the goal of overlapping I/O with compute
2. How to manage memory
3. How to handle memory between a TaskGraph's task and the main thread


API Used {#tut2b-api-used}
======

- \<htgs/api/IData.hpp\>
- \<htgs/api/ITask.hpp\>
- \<htgs/api/TaskGraphConf.hpp\>
- \<htgs/api/TaskGraphRuntime.hpp\>
- \<htgs/api/MemoryData.hpp\>
- \<htgs/api/IMemoryAllocator.hpp\>
- \<htgs/api/IMemoryReleaseRule.hpp\>
- \<htgs/api/IRule.hpp\>


Implementation {#tut2b-implementation}
======

This tutorial uses the same algorithm and representation for the Hadamard product presented in [Tutorial2A Implementation](@ref tut2a-implementation).

The primary change is altering the generate matrix task to read from disk and add support for systems with limited memory. This is particularly useful if the full matrix does not fit into system memory. To incorporate this design, we annotate the graph to include memory edges.

![Original task graph from Tutorial 2A](tut2Hadamard-taskgraph.png)
<br><br>
![Task graph with annotated memory edges](tut2Hadamard-taskgraph-memory.png)

Using the memory edges, we are able to ensure that a fixed number of block-sized matrices flow through the graph, staying within memory limits. The memory is allocated using the htgs::MemoryManager. A htgs::IMemoryReleaseRule is specified to determine when the memory can be released. For the Hadamard product, this rule is trivial because the memory can be released as soon as the data has been used for computation. In more complex examples, such as matrix
multiplication ([Tutorial 3](@ref tutorial3a)), data reuse must be carefully analyzed to ensure that the system does not deadlock; i.e. the memory never gets released because its release rule cannot be satisfied based on the traversal of data. The way an algorithm uses its data is important to understand to optimize these release rules, which ideally keeps data close to its processor for as long as possible.

The Hadamard product with memory management creates three memory edges for the two input matrices and the output matrix. In the pictoral representation, we mark that the Hadamard product task releases the memory for matrices A and B. Matrix C has a floating edge indicating that the memory will be released elsewhere. In actuality, the HTGS API only specifies the task that is allocating memory. This will be described in further detail later on in this tutorial.


## Data {#tut2b-data}

This tutorial uses the same data as presented in [Tutorial 2A Data](@ref tut2a-data).

## Tasks {#tut2b-tasks}

This tutorial uses the same tasks as in [Tutorial 2A tasks](@ref tut2a-tasks), except the Hadamard product task will now
get and release memory with the htgs::ITask::getMemory and htgs::ITask::releaseMemory functions. Also, we will modify the GenMatrixTask to read the data from disk.

The ReadDiskMatrixTask is responsible for reading matrices A or B depending on the type of MatrixRequestData passed to the read task. The matrix data is loaded from disk such that each block has its own file. If a new block size is specified, then the matrix data will have to be regenerated. There are multiple techniques for reading blocks of data, such as using memory mapped files and optimized file formats for specifying sub-regions without reading the entire file. In the next tutorial, we will show how to use memory mapped files for I/O (Linux/MacOS only), which can be used to improve bandwidth.


The directory structure for the data is stored as follows: "data/tutorial2/4096x4096blksize1024/matrix<A/B>".
Prior to reading data from disk, the ReadDiskMatrixTask allocates from main memory to store the data. This memory is
 acquired from a htgs::MemoryManager for matrix A or matrix B. To get memory from the htgs::MemoryManager,
 we use the htgs::ITask::getMemory function with the name of the memory edge and the release rule that is associated with the memory. The release rule
is used to identify when the memory can be recycled. This allows for the htgs::ITask to express how it plans on using the data and ideally can be setup
to only have to load the data once and release once all computation is done for that data. If the data were to be released prior to all computation being done, then a loop would need to be created (such as adding a new htgs::Bookkeeper with the appropriate htgs::IRule edge) to load the data a second time from disk.
If a htgs::ITask calls htgs::ITask::getMemory and it had not declared the name of the htgs::MemoryManager for the edge, then the program will report an assertion failed or segmentation fault.
 To check if a memory edge exists use htgs::ITask::hasMemoryEdge.

 The matrices can be created and saved to disk using the tutorial-utils function: [checkAndValidateMatrixBlockFiles](https://github.com/usnistgov/HTGS-Tutorials/blob/master/tutorial-utils/util-matrix.cpp), which will check to see if all the necessary files are available on disk, and generate them if any are missing.

 The HadamardProductTaskWithReleaseMem is responsible for the scalar product of two matrices. These matrices are stored
 in the MatrixBlockMulData object. Prior to multiplying the matrices, the task 
 allocates memory for the resulting matrix, which will be added to the output of the htgs::TaskGraphConf.
 
 After the Hadamard product task has completed processing matrices A and B together, the task must
 release the memory so that the htgs::MemoryManager can recycle that  memory for the read task. Releasing memory is done using the htgs::MemoryData (or htgs::m_data_t from <htgs/types/Types.hpp>) that was received from htgs::ITask::getMemory, which is passed to the htgs::ITask::releaseMemory function. The htgs::m_data_t contains metadata, which describes where the memory was allocated, allowing the memory to return to its corresponding memory manager.
The main stipulation with using this functionality is that all memory managers in a htgs::TaskGraphConf must have unique names (the names of the edges) and that the memory was allocated in the same graph that it is released. 

In the ReadDiskMatrixTask we use the
[matrixTypeToString](https://github.com/usnistgov/HTGS-Tutorials/blob/master/tutorial-utils/enums/MatrixType.cpp) function to convert
the matrix type enum to a string representation. This implementation is found in
the enums folder in tutorial-utils.

### ReadDiskMatrixTask {#tut2b-read-matrix-task}

~~~~~~~~~~{.c}
#include <htgs/api/ITask.hpp>
#include <cmath>
#include <fstream>
#include "../rules/MatrixMemoryRule.h"
#include "../../enums/MatrixType.h"

class ReadDiskMatrixTask : public htgs::ITask<MatrixRequestData, MatrixBlockData<htgs::m_data_t<double>>> {

 public:

  ReadDiskMatrixTask(size_t numThreads, size_t blockSize, size_t fullMatrixWidth, size_t fullMatrixHeight, std::string directory, MatrixType matrixType, bool computeReleaseCount) :
      ITask(numThreads),
      blockSize(blockSize),
      fullMatrixHeight(fullMatrixHeight),
      fullMatrixWidth(fullMatrixWidth),
      directory(directory),
      matrixType(matrixType),
      computeReleaseCount(computeReleaseCount)
  {
    numBlocksRows = (size_t) ceil((double) fullMatrixHeight / (double) blockSize);
    numBlocksCols = (size_t) ceil((double) fullMatrixWidth / (double) blockSize);
    if (computeReleaseCount)
    {
      switch(matrixType)
      {

        case MatrixType::MatrixA:
          releaseCount = numBlocksCols;
          break;
        case MatrixType::MatrixB:
          releaseCount = numBlocksRows;
          break;
        case MatrixType::MatrixC:break;
        case MatrixType::MatrixAny:break;
      }
    } else{
      releaseCount = 1;
    }

  }

  virtual ~ReadDiskMatrixTask() {}

  virtual void executeTask(std::shared_ptr<MatrixRequestData> data) {
    std::string matrixName = matrixTypeToString(data->getType());

    htgs::m_data_t<double> matrixData = this->getMemory<double>(matrixName, new MatrixMemoryRule(releaseCount));

    size_t row = data->getRow();
    size_t col = data->getCol();

    size_t matrixWidth;
    size_t matrixHeight;

    if (col == numBlocksCols - 1 && fullMatrixWidth % blockSize != 0)
      matrixWidth = fullMatrixWidth % blockSize;
    else
      matrixWidth = blockSize;

    if (row == numBlocksRows - 1 && fullMatrixHeight % blockSize != 0)
      matrixHeight = fullMatrixHeight % blockSize;
    else
      matrixHeight = blockSize;

    std::string fileName(directory + "/" + matrixName + "/" + std::to_string(row) + "_" + std::to_string(col));

    // Read data
    std::ifstream file(fileName, std::ios::binary);

    file.read((char *) matrixData->get(), sizeof(double) * matrixWidth * matrixHeight);

    addResult(new MatrixBlockData<htgs::m_data_t<double>>(data, matrixData, matrixWidth, matrixHeight, matrixWidth));

  }
  virtual std::string getName() {
    return "ReadDiskMatrixTask(" + matrixTypeToString(matrixType) + ")";
  }
  virtual ReadDiskMatrixTask *copy() {
    return new ReadDiskMatrixTask(this->getNumThreads(), blockSize, fullMatrixWidth, fullMatrixHeight, directory, matrixType, computeReleaseCount);
  }

  size_t getNumBlocksRows() const {
    return numBlocksRows;
  }

  size_t getNumBlocksCols() const {
    return numBlocksCols;
  }
 private:
  size_t blockSize;
  size_t fullMatrixWidth;
  size_t fullMatrixHeight;
  size_t numBlocksRows;
  size_t numBlocksCols;
  std::string directory;
  MatrixType matrixType;
  bool computeReleaseCount;
  size_t releaseCount;

};
~~~~~~~~~~

### HadamardProductTaskWithReleaseMem {#tut2b-hadamard-product-task}

~~~~~~~~~~{.c}
#include "../../../tutorial-utils/matrix-library/data/MatrixBlockMulData.h"
#include "../../../tutorial-utils/matrix-library/data/MatrixBlockData.h"
#include <htgs/api/ITask.hpp>

class HadamardProductTaskWithReleaseMem : public htgs::ITask<MatrixBlockMulData<htgs::m_data_t<double>>, MatrixBlockData<htgs::m_data_t<double>>> {

 public:
  HadamardProductTaskWithReleaseMem(size_t numThreads) : ITask(numThreads) {}

  virtual ~HadamardProductTaskWithReleaseMem() { }

  virtual void executeTask(std::shared_ptr<MatrixBlockMulData<htgs::m_data_t<double>>> data) {

    auto matAData = data->getMatrixA();
    auto matBData = data->getMatrixB();

    htgs::m_data_t<double> matrixA = matAData->getMatrixData();
    htgs::m_data_t<double> matrixB = matBData->getMatrixData();


    size_t width = matAData->getMatrixWidth();
    size_t height = matAData->getMatrixHeight();

    htgs::m_data_t<double> result = this->getMemory<double>("result", new MatrixMemoryRule(1));

    for (size_t i = 0; i < matAData->getMatrixWidth() * matAData->getMatrixHeight(); i++) {
      result->get()[i] = matrixA->get(i) * matrixB->get(i);
    }

    auto matRequest = matAData->getRequest();

    std::shared_ptr<MatrixRequestData>
        matReq(new MatrixRequestData(matRequest->getRow(), matRequest->getCol(), MatrixType::MatrixC));

    addResult(new MatrixBlockData<htgs::m_data_t<double>>(matReq, result, width, height, width));

    this->releaseMemory(matrixA);
    this->releaseMemory(matrixB);

  }
  virtual std::string getName() {
    return "HadamardProductTaskWithReleaseMem";
  }
  virtual HadamardProductTaskWithReleaseMem *copy() {
    return new HadamardProductTaskWithReleaseMem(this->getNumThreads());
  }

};
~~~~~~~~~~
### Notes {#tut2b-task-notes}

- Use the htgs::ITask::getMemory function to get memory from a memory edge
  + htgs::TaskGraphConf::addMemoryManagerEdge declares the htgs::ITask responsible for getting memory
  + If an ITask calls htgs::ITask::getMemory and that htgs::ITask was not designated having a memory edge, then the program will segmentation fault
- Releasing memory is done with the htgs::ITask::releaseMemory function
  + Uses the htgs::m_data_t to provide meta data to return the memory to the htgs::MemoryManager that allocated the memory
  + The htgs::MemoryData is forwarded to the htgs::MemoryManager using the htgs::TaskGraphCommunicator
      - Functionality with the htgs::TaskGraphCommunicator is mostly restricted to memory management

## Managing Dependencies with a Bookkeeper and IRules {#tut2b-bookkeeper}

This tutorial reuses all of the htgs::Bookkeeper and htgs::IRule implementations from [Tutorial2A](@ref tut2a-bookkeeper)

## Throttling Tasks with a Memory Manager {#tut2b-memory-manager}

The htgs::MemoryManager is a htgs::ITask that is created when adding a memory edge to a htgs::ITask. A memory edge
is similar to any other edge,  except it connects directly to the htgs::ITask rather than through a htgs::TaskManager. This edge
acts as a mechism for the task to get htgs::MemoryData from a htgs::MemoryManager. The htgs::MemoryManager maintains a pool of htgs::MemoryData.
Each htgs::MemoryData is inserted into the htgs::MemoryManager's output htgs::Connector
This output htgs::Connector is shared between the htgs::ITask getting
memory and the htgs::MemoryManager. If the htgs::Connector has no data, then the htgs::ITask will wait until the htgs::MemoryManager adds data
along this edge. 

a htgs::MemoryManager releases/recycles memory based on the htgs::IMemoryReleaseRule that is attached to the htgs::MemoryData. The htgs::IMemoryReleaseRule
is added to the htgs::MemoryData using the htgs::ITask::getMemory function. This is used to update the state of the htgs::MemoryData's htgs::IMemoryReleaseRule,
which determines when
the htgs::MemoryData can be readded into the htgs::MemoryManager's memory pool. As soon as the htgs::MemoryData is released/recycled, it is sent to the output htgs::Connector for the htgs::MemoryManager. Returning htgs::MemoryData to its htgs::MemoryManager is done by passing
the htgs::MemoryData that is received from the htgs::ITask::getMemory function to the htgs::ITask::releaseMemory function. The htgs::MemoryData
contain meta data that describes the name of the htgs::MemoryManager task and its address. This ensures the htgs::MemoryData returns to the task that had allocated it.

Every memory edge contains a name associated with the edge. The name is used as an identifier for getting and releasing memory. As such, the names of
the memory edges must be unique within a htgs::TaskGraphConf.

Any htgs::ITask can be specified as a task that is getting memory from a htgs::MemoryManager using the htgs::TaskGraphConf::addMemoryManagerEdge
function, which creates a htgs::MemoryManager and attaches it to the htgs::ITask. The htgs::ITask that is specified must already exist within the htgs::TaskGraphConf prior
to using the htgs::TaskGraphConf::addMemoryManagerEdge Allocation, freeing, and the type of memory used within the htgs::MemoryManager is defined 
with a htgs::IMemoryAllocator. The memory pool size is used to define the number of htgs::MemoryData that is allocated within the memory pool. The htgs::IMemoryAllocator and
the memory pool size must take the data traversal behavior of the algorithm to ensure that there is enough memory data being sent from the htgs::MemoryManager and that there 
is not too much memory being allocated.

  There are two types of MemoryManagers: Static and Dynamic. Each MemoryManager type modifies how/when
  memory is allocated and freed.

  All MemoryManager types allocates a pool of htgs::MemoryData during the htgs::ITask::initialize phase. However, the Static
    MemoryManager will also allocate the memory associated with the htgs::MemoryData using the htgs::IMemoryAllocator::memAlloc function. The Static MemoryManager will
    release this memory once the destructor for the htgs::MemoryManager is called.

  In the next sections, we will go into the details as to how each of these MemoryManager types vary and process MemoryData.

### Static Memory Manager {#tut2b-static-mm}

The static MemoryManager is a memory manager that recycles memory when it is released. All of the memory for the memory pool is allocated once
when the MemoryManager is initialized and freed when the MemoryManager is shutdown. The diagram below shows how
MemoryData is processed when an ITask releases memory. First, when MemoryData enters the MemoryManager,
the MemoryData's state is updated using the htgs::IMemoryReleaseRule::memoryUsed function.

Next, the MemoryManager checks the htgs::IMemoryReleaseRule::canReleaseMemory function to determine whether the state indicates the memory
can be freed or not. If the state indicates it can be freed, then the MemoryData is inserted into the MemoryPool. If it is
not freed, then the MemoryData will not be recycled. Next, the MemoryPool is emptied into the MemoryManager's output Connector
so the ITask getting memory can wake up and acquire MemoryData.

![Static memory manager](staticMemoryManager.png)

### Dynamic Memory Manager {#tut2b-dynamic-mm}

The dynamic MemoryManager contains a similar structure as the static MemoryManager with how it processes MemoryData.
The primary difference is when the dynamic
MemoryManager allocates and frees memory. In the static MemoryManager all memory is allocated during initialization and freed
during shutdown. The dynamic MemoryManager allocates and frees memory on-the-fly. As shown in the diagram below, when MemoryData
is sent from the releaseMemoryEdges, if the memory can be released, then the MemoryManager will free the memory that the MemoryData
is managing. When the memoryEdges acquires memory from the MemoryManager, the ITask will allocate the memory that the MemoryData
 is managing prior to returning the MemoryData to the ITask. Using these mechanisms the memory is allocated and freed
 on demand.

 ![Dynamic memory manager](dynamicMemoryManager.png)

Within the htgs::MemoryData there are two functions that must be defined to use a MemoryManager edge:
(1) htgs::IMemoryAllocator, and (2) htgs::IMemoryReleaseRule. The Ihtgs::MemoryAllocator is specified when creating the memory edge,
 while the IMemoryReleaseRule is added to MemoryData when the memoryEdges gets memory. For the Hadamard product graph, we
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

### MatrixAllocator {#tut2b-matrix-allocator}

~~~~ {.c}
 #include <htgs/api/IMemoryAllocator.hpp>
 
 // MatrixAllocator<Type> to specify multiple types of memory; i.e. double or float 
 template <class Type>
 class MatrixAllocator : public htgs::IMemoryAllocator<Type> {
  public:
   MatrixAllocator(size_t width, size_t height) : htgs::IMemoryAllocator<Type>(width * height) {}
 
   Type *memAlloc(size_t size) {
     Type *mem = new Type[size];
     return mem;
   }
 
   Type *memAlloc() {
     Type *mem = new Type[this->size()];
     return mem;
   }
 
   void memFree(Type *&memory) {
     delete[] memory;
   }
 
 };
~~~~

### MatrixMemoryRule {#tut2b-matrix-memory-rule}

~~~~ {.c}
#include <htgs/api/IMemoryReleaseRule.hpp>
 
 class MatrixMemoryRule : public htgs::IMemoryReleaseRule {
  public:
 
   MatrixMemoryRule(size_t releaseCount) : releaseCount(releaseCount) {
   }
 
   void memoryUsed() {
     releaseCount--;
   }
 
   bool canReleaseMemory() {
     return releaseCount == 0;
   }
 
  private:
   size_t releaseCount;
 };
~~~~

### Notes {#tut2b-memorymanagement-notes}
- htgs::MemoryManager manages the memory that is used/recycled within a htgs::TaskGraphConf as well as to help throttle a htgs::TaskGraphConf to
ensure the htgs::TaskGraphConf does not allocate too much memory.
- There are two types of memory managers: Static and Dynamic
    + Primary difference is how/when memory is allocated/freed
- When connecting a memory edge, the htgs:ITask that is getting memory must already be added to the htgs::TaskGraphConf
- The htgs::IMemoryAllocator defines the type of memory that is allocated, how the memory is allocated, and how the memory is freed
    + Use the type defined for the IMemoryAllocator<Type> when getting memory from within a htgs::ITask using htgs::ITask::getMemory<Type>()
- The htgs::IMemoryReleaseRule is used to define the state of htgs::MemoryData to help determine when the htgs::MemoryManager can recycle/free memory
    + The htgs::IMemoryReleaseRule is added to htgs::MemoryData when a htgs::ITask gets memory using htgs::ITask::getMemory.
    + Each htgs::MemoryData should have a separate instance of the htgs::IMemoryReleaseRule (a new instance per htgs::MemoryData)
- The htgs::IMemoryReleaseRule should follow the data flow behavior of the htgs::TaskGraphConf to ensure that the memory gets released.
    + If memory does not get released and the memory getter is waiting for memory, then deadlock can occur.
    + There should be enough elements in the memory pool to progress the htgs::TaskGraphConf so that the IMemoryReleaseRule can allow data to be released.
- Use htgs::m_data_t<Type> as a shorthand for std::shared_ptr<htgs::MemoryData<Type>>
    + std::shared_ptr<htgs::MemoryData<Type>> is returned from the htgs::ITask::getMemory function 


## Creating and Executing the TaskGraph {#tut2b-create-and-execute-taskgraph}

Using the implementation from [tutorial 2a](tut2a-create-and-execute-taskgraph), we augment the graph using the HTGS API to specify the memory manager edges.

Belows is the source code implementation for setup, construction of the task TaskGraph, executing the TaskGraph, and processing
the output of the htgs::TaskGraphConf.

As before, the traversal order with operating on matrices A and B for the Hadamard product is defined using
the htgs::TaskGraphConf::produceData function. If the structure presented below were to change the traversal
to first iterate through requests for matrix A, followed by
requests for matrix B in an entirely separate loop, then the requests for A would have to be fully processed
prior to initiating the first computation on the Hadamard product. Using this traversal pattern would be
detrimental to the memory management system as all of matrix A would have to be loaded prior to loading a single
block of B, and if the system were to run out of memory prior to loading a block of B, then the graph would 
deadlock. Special care must be taken to how an algorithm initiates work into a graph that takes into account
the behavior a traversal pattern has on memory and the computation.

The main function can be used to post-process data coming from the htgs::TaskGraphConf. If the data was allocated using a memory edge, then the main thread can use the
htgs::TaskGraphConf::releaseMemory function to return the memory to its htgs::MemoryManager.



### Main function (Hadamard Product) {#tut2b-main-function}

~~~~~~~~~~{.c}
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>
#include "../../tutorial-utils/matrix-library/data/MatrixRequestData.h"
#include "../../tutorial-utils/matrix-library/data/MatrixBlockData.h"
#include "../../tutorial-utils/matrix-library/tasks/ReadDiskMatrixTask.h"
#include "../../tutorial-utils/matrix-library/allocator/MatrixAllocator.h"
#include "../rules/HadamardLoadRule.h"
#include "tasks/HadamardProductTaskWithReleaseMem.h"
#include "../../tutorial-utils/SimpleClock.h"
#include "../../tutorial-utils/util-matrix.h"

int main(int argc, char *argv[]) {
  size_t width = 1024;
  size_t height = 1024;
  size_t blockSize = 256;
  size_t numReadThreads = 1;
  size_t numProdThreads = 10;
  std::string directory("data");

  for (int arg = 1; arg < argc; arg++) {
    std::string argvs(argv[arg]);

    if (argvs == "--width") {
      arg++;
      width = (size_t)atoi(argv[arg]);
    }

    if (argvs == "--height") {
      arg++;
      height = (size_t)atoi(argv[arg]);
    }

    if (argvs == "--block-size") {
      arg++;
      blockSize = (size_t)atoi(argv[arg]);
    }

    if (argvs == "--num-readers") {
      arg++;
      numReadThreads = (size_t)atoi(argv[arg]);
    }

    if (argvs == "--num-workers") {
      arg++;
      numProdThreads = (size_t)atoi(argv[arg]);
    }

    if (argvs == "--dir") {
      arg++;
      directory = argv[arg];
    }

    if (argvs == "--help") {
      std::cout << argv[0]
                << " help: [--width <#>] [--height <#>] [--block-size <#>] [--num-readers <#>] [--num-workers <#>] [--dir <dir>] [--help]"
                << std::endl;
      exit(0);
    }
  }

  // Check directory for matrix files based on the given file size
  checkAndValidateMatrixBlockFiles(directory, width, height, width, height, blockSize, false);

  // Use new tasks that contain functions for getting and releasing memory
  ReadDiskMatrixTask *readMatTask = new ReadDiskMatrixTask(numReadThreads, blockSize, width, height, directory, MatrixType::MatrixAny, false);
  HadamardProductTaskWithReleaseMem *prodTask = new HadamardProductTaskWithReleaseMem(numProdThreads);

  size_t numBlocksCols = readMatTask->getNumBlocksCols();
  size_t numBlocksRows = readMatTask->getNumBlocksRows();

  // The data type that is passed now uses htgs::MemoryData
  HadamardLoadRule<htgs::m_data_t<double>> *loadRule = new HadamardLoadRule<htgs::m_data_t<double>>(numBlocksCols, numBlocksRows);
  auto bookkeeper = new htgs::Bookkeeper<MatrixBlockData<htgs::m_data_t<double>>>();

  auto taskGraph = new htgs::TaskGraphConf<MatrixRequestData, MatrixBlockData<htgs::m_data_t<double>>>();

  taskGraph->setGraphConsumerTask(readMatTask);
  taskGraph->addEdge(readMatTask, bookkeeper);
  taskGraph->addRuleEdge(bookkeeper, loadRule, prodTask);
  taskGraph->addGraphProducerTask(prodTask);

  MatrixAllocator<double> *matAlloc = new MatrixAllocator<double>(blockSize, blockSize);

  // Add memory edges
  taskGraph->addMemoryManagerEdge("result", prodTask, matAlloc, 50, htgs::MMType::Static);
  taskGraph->addMemoryManagerEdge("MatrixA", readMatTask, matAlloc, 100, htgs::MMType::Static);
  taskGraph->addMemoryManagerEdge("MatrixB", readMatTask, matAlloc, 100, htgs::MMType::Static);


  htgs::TaskGraphRuntime *runtime = new htgs::TaskGraphRuntime(taskGraph);

  SimpleClock clk;
  clk.start();

  runtime->executeRuntime();

  for (size_t row = 0; row < numBlocksRows; row++) {
    for (size_t col = 0; col < numBlocksCols; col++) {
      MatrixRequestData *matrixA = new MatrixRequestData(row, col, MatrixType::MatrixA);
      MatrixRequestData *matrixB = new MatrixRequestData(row, col, MatrixType::MatrixB);

      taskGraph->produceData(matrixA);
      taskGraph->produceData(matrixB);
    }
  }

  taskGraph->finishedProducingData();

  while (!taskGraph->isOutputTerminated()) {
    auto data = taskGraph->consumeData();

    if (data != nullptr) {
      std::cout << "Result received: " << data->getRequest()->getRow() << ", " << data->getRequest()->getCol()
                << std::endl;

      // Release memory from the main thread
      taskGraph->releaseMemory(data->getMatrixData());
    }
  }

  runtime->waitForRuntime();

  clk.stopAndIncrement();

  std::cout << "width: " << width << ", height: " << height << ", blocksize: " << blockSize << ", time: "
            << clk.getAverageTime(TimeVal::MILLI) << " ms" << std::endl;

  delete runtime;
}
~~~~~~~~~~

Sample execution:
~~~~
./tutorial2-with-disk 
Result received: 1, 3
Result received: 2, 1
Result received: 2, 0
Result received: 1, 2
Result received: 0, 0
Result received: 0, 1
Result received: 0, 2
Result received: 0, 3
Result received: 1, 0
Result received: 1, 1
Result received: 3, 2
Result received: 2, 3
Result received: 2, 2
Result received: 3, 0
Result received: 3, 1
Result received: 3, 3
width: 1024, height: 1024, blocksize: 256, time: 5.78517 ms
~~~~


### Notes {#tut2b-taskgraph-notes}
- For memory edges, the memory pool size parameter is import to specify based on amount of memory available and the dataflow behavior
    in relation to the memory usage behavior
    + If not enough memory is available to allow for memory to be released, then the TaskGraph will deadlock
    + The more memory available, the more data elements that can flow through the TaskGraph to improve pipelining.
- Use the htgs::TaskGraphConf::releaseMemory function to release any memory that is sent along the output of the htgs::TaskGraphConf


Summary {#tut2b-summary}
======

In this tutorial, we looked at augmenting a graph to use disk and memory management.
- The various types of memory managers
- How to use the memory management system with htgs::IMemoryReleaseRule and htgs::IMemoryAllocator
- The importance of understanding memory usage behavior in relationship to memory edges to prevent TaskGraph deadlocks.
- How the separation of concerns can be used to reuse existing code and add new functionality

I encourage you to play around with parts A and B of the tutorials and see how performance is impacted using larger matrices, different block sizes,
and various thread configurations. These can be quickly modified using the command-line parameters: -\-width, -\-height, -\-block-size, -\-num-readers, and -\-num-workers.

In the next tutorial, we will be implementing an in-core matrix multiplication using block decomposition and (optionally) the OpenBLAS API.
The tutorial will introduce how to implement a slightly more complex dependency and the impacts of data traversal. We also show how to improve
the utilization of a compute kernel by using an optimized variant. 
In addition, we will look into debugging and profiling using the htgs::TaskGraphConf::writeDotToFile.

To demonstrate these aspects, we have split Tutorial 3 into 3 parts.

1. Basic matrix multiplication
2. Identifying bottlenecks with profiling and debugging tools and ways to optimize with OpenBLAS
3. Computing with memory mapped files and identifying traversal patterns.
