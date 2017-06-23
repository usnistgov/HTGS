Tutorial 3B {#tutorial3b}
========
[TOC]

In this tutorial we revisit [Tutorial 3A](@ref tutorial3a) to investigate 
performance through the HTGS profiling/debugging visualization. One powerful component of HTGS is its high level abstractions that is explicitly represented throughout the analysis and implementation. These abstractions persist throughout execution, which can be used to profile and identify bottlenecks at a high level of abstraction.


Objectives {#tut3b-objectives}
=======
1. How to use the built-in dot generation tools that HTGS provides.
  - Use GraphViz to convert dot file into image
  - Bit-flags to control visualization.
2. How to profile an implementation with HTGS.
3. Methods to improve utilization within a task.


API Used {#tut3b-api-used}
======

- \<htgs/api/TaskGraphRuntime.hpp\>
- \<htgs/api/TaskGraphConf.hpp\>
- \<htgs/api/ITask.hpp\>
- \<htgs/types/TaskGraphDotGenFlags.hpp\>

## Implementation, Data, Tasks, and Dependencies {#tut3b-imp-data-tasks-dep}

This tutorial uses the same implementation, data, tasks, and dependencies as presented in [Tutorial 3A Data](@ref tut2a-data).

## Debugging and Profiling htgs::TaskGraphConf {#tut3b-debug-taskgraph}

Building a htgs::TaskGraphConf will create an explicit represenatation of the task graph, which can be visualized using htgs::TaskGraphConf::writeDotToFile. This function can be called before or after launching the graph using the htgs::TaskGraphRuntime. 

1. If called prior to calling htgs::TaskGraphRuntime::executeRuntime 
  - Will visualize the task graph configuration
  - htgs::ExecutionPipeline will dynamically create new htgs::TaskGraphConf and htgs::TaskGraphRuntime
    + Visualizing prior to execution provides a preview of the graph encapsulated within the htgs::ExecutionPipeline
  - Debugging

2. If called after calling htgs::TaskGraphRuntime::executeRuntime 
  - Can be used to visualize profiling by specifying "-DPROFILE" during compilation
  - Can visualize during execution to identify errors/deadlocks
    + A system to visualize the graph during execution via a web browser in quasi-realtime is in development
  - Debugging and profiling

There are nine flags that are specified in the \<htgs/types/TaskGraphDotGenFlags.hpp\> header file, which can be used to configure the output of the visualization.

1. DOTGEN_FLAG_HIDE_MEM_EDGES
  - Hides memory manager task and associated memory edge
2. DOTGEN_FLAG_SHOW_ALL_THREADING
  - Shows every task that is created (including those created for thread pools)
  - Only shows threads if called after execution 
3. DOTGEN_FLAG_SHOW_IN_OUT_TYPES
  - Displays the input and output types for each task
4. DOTGEN_FLAGS_HIDE_PROFILE_COMP_TIME
  - Hides the compute time profiling data
5. DOTGEN_FLAGS_HIDE_PROFILE_MAX_Q_SZ
  - Hides the maximum queue size profiling data
6. DOTGEN_FLAGS_HIDE_PROFILE_WAIT_TIME
  - Hides the wait time profiling data
7. DOTGEN_COLOR_COMP_TIME 
  - Colors the tasks based on compute time
  - Creates a separate file with the prefix: 'color-compute-time-"
  - Must specify "-DPROFILE" during compilation
8. DOTGEN_COLOR_MAX_Q_SZ 
  - Colors the tasks based on the maximum queue size
  - Creates a separate file with the prefix: 'color-max-q-sz-"
  - Must specify "-DPROFILE" during compilation
9. DOTGEN_COLOR_WAIT_TIME 
  - Colors the tasks based on the wait time
  - Creates a separate file with the prefix: 'color-wait-time-"
  - Must specify "-DPROFILE" during compilation

When profiling a htgs::TaskGraphConf, it is best to call htgs::TaskGraphConf::writeDotToFile after a call to htgs::TaskGraphRuntime::waitForRuntime. This ensures that the graph has finished executing.

Below are examples of visualizing the htgs::TaskGraphConf from [Tutorial3A](@ref tutorial3a) before and after executing the graph.


### Visualizing Before Executing a htgs::TaskGraphConf  {#tut3b-vis-before}

~~~~{.c}
// Create tasks 
...

// Build the task graph
auto taskGraph = htgs::TaskGraphConf<MatrixRequestData, MatrixRequestData>();
taskGraph->setGraphConsumerTask(distributeBk);
taskGraph->addRuleEdge(distributeBk, distributeRuleMatA, readAMatTask);
taskGraph->addRuleEdge(distributeBk, distributeRuleMatB, readBMatTask);

taskGraph->addEdge(readAMatTask, matMulBk);
taskGraph->addEdge(readBMatTask, matMulBk);

taskGraph->addRuleEdge(matMulBk, loadRule, mmulTask);

taskGraph->addEdge(mmulTask, matAccumBk);
taskGraph->addRuleEdge(matAccumBk, accumulateRule, accumTask);
taskGraph->addEdge(accumTask, matAccumBk);

taskGraph->addRuleEdge(matAccumBk, outputRule, outputTask);
taskGraph->addGraphProducerTask(outputTask);

// Write graph to file
taskGraph->writeDotToFile("simple-graph.dot");
taskGraph->writeDotToFile("in-out-graph.dot", DOTGEN_FLAG_SHOW_IN_OUT_TYPES);

// Execute graph
...
~~~~

The graphs above were converted to a png (or pdf with -Tpdf) file using the following commands:
~~~~
dot -Tpng -o simple-graph.png simple-graph.dot
dot -Tpng -o in-out-graph.png in-out-graph.dot
~~~~

[simple-graph.png](simple-graph.png) 

[in-out-graph.png](in-out-graph.png)

In the figures above, each graph shows the explicit representation that is to be used to process matrix multiplication. Tasks have the "x<#>" notation, where <#> represents the number of tasks copied and bound to separate threads. These threads have not been spawned yet, but provide an indicator as to how many threads will be spawned to execute each task within the graph. Every htgs::Bookkeeper have annotated output edges that provide the name of the rule that is responsible for processing that edge. 

In addition there are nodes between tasks that represent the htgs::Connector, which manages the thread-safe queue for the task. The number within this node represents the number of active connections that will be producing data for the connector. During execution, if this number reaches zero, then that edge is considered no longer receiving data (terminated), which can be checked with
htgs::Connector::isInputTerminated. At the top and bottom of the visualization are the "Graph Input" and "Graph Output" nodes, which represent the input and output connectors 
for the htgs::TaskGraphConf.

In the in-out-graph.png figure, every task have additional details showing the specific input and output data types for each task.

These figures can be used to ensure the graph representation that is done during analysis maps well into the actual implementation.

### Visualizing After Executing a htgs::TaskGraphConf {#tut3b-vis-after}

During compilation we include "-DPROFILE" to enable profiling of the graph.

~~~~{.c}
// Create tasks 
...

// Build the task graph
auto taskGraph = htgs::TaskGraphConf<MatrixRequestData, MatrixRequestData>();
taskGraph->setGraphConsumerTask(distributeBk);
taskGraph->addRuleEdge(distributeBk, distributeRuleMatA, readAMatTask);
taskGraph->addRuleEdge(distributeBk, distributeRuleMatB, readBMatTask);

taskGraph->addEdge(readAMatTask, matMulBk);
taskGraph->addEdge(readBMatTask, matMulBk);

taskGraph->addRuleEdge(matMulBk, loadRule, mmulTask);

taskGraph->addEdge(mmulTask, matAccumBk);
taskGraph->addRuleEdge(matAccumBk, accumulateRule, accumTask);
taskGraph->addEdge(accumTask, matAccumBk);

taskGraph->addRuleEdge(matAccumBk, outputRule, outputTask);
taskGraph->addGraphProducerTask(outputTask);

// Execute graph
auto runtime = new htgs::TaskGraphRuntime(taskGraph);

// Produce data
...

taskGraph->finishedProducingData();

taskGraph->waitForRuntime();

taskGraph->writeDotToFile("profile-graph.dot");
taskGraph->writeDotToFile("profile-all-threads-graph.dot", DOTGEN_FLAG_SHOW_ALL_THREADING);
taskGraph->writeDotToFile("profile-graph.dot", DOTGEN_COLOR_COMP_TIME | DOTGEN_COLOR_MAX_Q_SZ | DOTGEN_COLOR_WAIT_TIME);
~~~~

We execute this version using the following command (includes associated output):
~~~
./tutorial3 --block-size 256 --width-b 2048 --height-a 2048 --shared-dim 2048 --num-workers 20
Writing dot file for task graph to profile-graph.dot
Writing dot file for task graph to profile-all-threads-graph.dot
Writing dot file for task graph with compute time coloring to color-compute-time-profile-graph.dot
Writing dot file for task graph with wait time coloring to color-wait-time-profile-graph.dot
Writing dot file for task graph with max Q size coloring to color-max-q-sz-profile-graph.dot
htgs, 20, accum-threads: 10, width-b: 2048, height-a: 2048, shared-dim: 2048, blockSize: 256, time:1291.56, end-to-end:1291.83
~~~

The code above will generate five visualizations. (with adding -DPROFILE to compilation)
1. profile-graph.dot
  - Generates profiling data for each task.
  - Thread pool timings are averaged for each task
2. profile-all-threads.dot
  - Generates the profiling for each task with threads fully expanded
3. color-compute-time-profile-graph.dot
  - Generates profiling data for each task, where tasks are colored based on compute time
4. color-wait-time-profile-graph.dot
  - Generates profiling data for each task, where tasks are colored based on wait time
5. color-max-q-sz-profile-graph.dot
  - Generates profiling data for each task, where tasks are colored based on the maximum queue size

As shown in the source code above, each DOTGEN flag can be aggregated using the bit-wise OR operator.

Below are the resulting figures for each graph after using the following commands:
~~~~
dot -Tpng -o profile-graph.png profile-graph.dot
dot -Tpng -o profile-all-threads.png profile-all-threads.dot
dot -Tpng -o color-compute-time-profile-graph.png color-compute-time-profile-graph.dot
dot -Tpng -o color-wait-time-profile-graph.png color-wait-time-profile-graph.dot
dot -Tpng -o color-max-q-sz-profile-graph.png color-max-q-sz-profile-graph.dot
~~~~

[profile-graph.png](profile-graph.png)

[profile-all-threads-graph.png](profile-all-threads-graph.png)

[color-compute-time-profile-graph.png](color-compute-time-profile-graph.png)

[color-wait-time-profile-graph.png](color-wait-time-profile-graph.png)

[color-max-q-sz-profile-graph.png](color-max-q-sz-profile-graph.png)

profile-graph.png shows the compute time, wait time, and max queue size for each task. Any task that has multiple threads associated are reporting the average timings for that task. 

profile-all-threads-graph.png shows the fully expanded graph and all threading.

These graphs are useful to get a glimpse at the overall timings for each task. Using the coloring options provides a much clearer picture to pinpoint the bottleneck(s) within an implementation. First, the color-compute-time-profile-graph.png creates a color map based around the compute times. The cooler blue times are less impactful, whereas the hot red times have high impact. The MatMulBlkTask is colored bright red, which indicates this task is the main bottleneck. The color-wait-time
and color-max-q-sz graphs support this observation. In addition, the color-wait-time and color-max-q-sz are showing that the MatMulAccumTask and MatMulOutPutTaskWithDisk tasks are spending the majority of their time waiting for data. Improving the execution time of the MatMulBlkTask should reduce reduce the wait time for these tasks; however, in some cases modifying traversal patterns for inserting data into the htgs::TaskGraphConf from the main thread or optimizing data locality can provide additional benefits. 

To improve the performance of this implementation, we investigate methods for improving the MatMulBlkTask.

## Improving Utilization of Matrix Multiplication in HTGS {#tut3b-optimize}

The MatMulBlkTask can be thought of as an in-core computational kernel that executes on the CPU. Methods for fine-grained parallelism, such as utilizing L1 cache and instruction-level parallelism can be used to improve the overall CPU utilization for this task. Alternatively, an accelerator could be incorporated into the graph to offload the computation to high bandwidth massively parallel hardware (shown in [Tutorial4A](@ref tutorial4a)). 

The current implementation of matrix multiplication uses a triple nested for loop that iterates across the entire matrix, shown [here](https://github.com/usnistgov/HTGS-Tutorials/blob/master/tutorial-utils/matrix-library/operations/matmul.cpp). Instead of taking the time to optimize this code, existing in-core compute kernels are available, such as those from [OpenBLAS](http://www.openblas.net/). OpenBLAS implements various basic linear algebra operators and will generate a per-architecture optimized library. Using this library should boost the performance for our MatMulBlkTask.

Before incorporating OpenBLAS into the MatMulBlkTask, one additional component can be taken into account to quantify this improvement.

Each htgs::ITask contain two additional functions that will add customizable details to the visualization. This can be applied before or after execution of an htgs::TaskGraphConf.

Implementing the htgs::ITask::debugDotNode is intended to add debug output. The htgs::ITask::getDotCustomProfile can be used to provide additional custom profiling data that will be added to the dot visualization. This can be used in conjunction with htgs::ITask::getComputeTime, to obtain the compute time of the htgs:ITask in microseconds.

Below, we incorporate a GFLops benchmark into the MatMulBlkTask. This allows us to track the performance within this task.

~~~{.c}
#include "../../tutorial-utils/matrix-library/operations/matmul.h"
#include "../../tutorial-utils/matrix-library/data/MatrixBlockData.h"
#include "../../tutorial-utils/matrix-library/data/MatrixBlockMulData.h"

#include <htgs/api/ITask.hpp>

class MatMulBlkTask : public htgs::ITask<MatrixBlockMulData<double *>, MatrixBlockData<double *>> {

 public:
  MatMulBlkTask(size_t numThreads, bool colMajor) :
      ITask(numThreads), colMajor(colMajor), numOps(0) {}


  virtual void executeTask(std::shared_ptr<MatrixBlockMulData<double *>> data) {

    auto matAData = data->getMatrixA();
    auto matBData = data->getMatrixB();

    double *matrixA = matAData->getMatrixData();
    double *matrixB = matBData->getMatrixData();

    size_t width = matBData->getMatrixWidth();
    size_t height = matAData->getMatrixHeight();

    size_t lda = matAData->getLeadingDimension();
    size_t ldb = matBData->getLeadingDimension();

    size_t ldc;

    if (colMajor)
      ldc = height;
    else
      ldc = width;

    double *result = new double[width * height];

    computeMatMul(height, width, matAData->getMatrixWidth(), 1.0, matrixA, lda,
                  matrixB, ldb, 0.0, result, ldc, colMajor);

	// Compute total number of operations being done by this task
    numOps += height * width * matAData->getMatrixWidth();

    std::shared_ptr<MatrixRequestData> matReq(new MatrixRequestData(matAData->getRequest()->getRow(),
                                                                    matBData->getRequest()->getCol(),
                                                                    MatrixType::MatrixC));

    this->addResult(new MatrixBlockData<double *>(matReq, result, width, height, ldc));

  }
  virtual std::string getName() {
    return "MatMulBlkTask";
  }

  virtual MatMulBlkTask *copy() {
    return new MatMulBlkTask(this->getNumThreads(), colMajor);
  }

  // Output gflops performance
  std::string getDotCustomProfile() override {
    auto microTime = this->getTaskComputeTime();
    double numGFlop = ((double)numOps * 2.0) * 1.0e-9d;

    double timeSec = (double)microTime / 1000000.0;

    return "Performance: " + std::to_string(numGFlop / timeSec) + " gflops";
  }

 private:
  bool colMajor;
  size_t numOps;

};

~~~

Using the above implementation of the task outputs the number of gflops performed by a task.

Using this variant, we execute with the following command and output the task graph dot file.

~~~~
./tutorial3 --block-size 128 --width-b 2048 --height-a 2048 --shared-dim 2048 --num-workers 20
Writing dot file for task graph to profile-all-threads-graph.dot
Writing dot file for task graph with compute time coloring to color-compute-time-profile-graph.dot
htgs, 20, accum-threads: 10, width-b: 2048, height-a: 2048, shared-dim: 2048, blockSize: 128, time:1268.35, end-to-end:1268.84
~~~~

This implementation achieves 13.5 GFlops. Viewing the graph visualization, we see the first thread for the MatMulBlkTask is achieving 0.7 GFlops. With 20 threads
this is approximately 14 GFlops, note that this is not taking into account the final accumulation step. To view the actual GFlop rating for each MatMulBlkTask thread, we can use the DOTGEN_FLAG_SHOW_ALL_THREADING flag, also shown below.

[Profile with GFlops](color-compute-time-profile-gflops-graph.png)

[Profile with all threads and GFlops](profile-all-threads-gflops-graph.png)

Now that we have a clear metric to base our task performance off of, we incorporate OpenBLAS into the computation.

Example call for OpenBLAS, also found [here](https://github.com/usnistgov/HTGS-Tutorials/blob/master/tutorial-utils/matrix-library/operations/matmul.cpp):
~~~
  cblas_dgemm(ColMajor ? CblasColMajor : CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, alpha, A, LDA, B, LDB, beta, C, LDC);
~~~

To enable using OpenBLAS in the HTGS-Tutorials, define the following during cmake (or cmake-gui/ccmake)


1. -DLIBOPENBLAS_INCLUDE_DIR={PATH_TO_OPENBLAS INCLUDE DIR}
2. -DLIBOPENBLAS_LIBRARY={PATH_TO_OPENBLAS LIB DIR}

Defining these options will enable using OpenBLAS for Tutorial 3. We execute the same command as above with OpenBLAS enabled.

~~~~
./tutorial3 --block-size 128 --width-b 2048 --height-a 2048 --shared-dim 2048 --num-workers 20
Writing dot file for task graph to profile-all-threads-graph.dot
Writing dot file for task graph with compute time coloring to color-compute-time-profile-graph.dot
htgs, 20, accum-threads: 10, width-b: 2048, height-a: 2048, shared-dim: 2048, blockSize: 128, time:116.368, end-to-end:116.83
~~~~

With OpenBLAS enabled, the implementation now achieves 148.1 GFlops. The graph visualization now shows the first thread achieves 5.92 Gflops, which is approximately, 118.4 GFlops. Viewing all of the threads shows a clearer picture about utilization, where some threads achieved up to 12.79 GFlops.

[Profile with OpenBLAS](color-compute-time-profile-gflops-openblas-graph.png)

[Profile with all threads OpenBLAS](profile-all-threads-gflops-openblas-graph.png)

Overall, this shows an overall speedup of 10x for this small problem size.

Now that the MatMulBlk task has been improved, we can experiment further to analyze the impact of block-size. I leave this task up to you to see how far you can push the GFlops performance for the MatMulBlk task by altering the matrix dimensions and block sizes.

### Notes {#tut3b-taskgraph-notes}
- Using the htgs::TaskGraphConf::writeDotToFile can provide extremely useful profiling data the maps directly into the analysis
  + Can be used for profiling and debugging
  + Using DOTGEN flags from \<htgs/types/TaskGraphDotGenFlags.hpp\> can customize the output
  + Enable debugging by compiling with -DPROFILE
- Adding custom profile output with htgs::ITask::getDotCustomProfile can be used to view per-task utilization
  + Added to comptuational task can show GFlops
  + Could also be used to profile bandwidth
- Benefits available to per-task optimizations that take better advantage of the underlying architecture

Summary {#tut3b-summary}
======

In this tutorial, we revisited the matrix multiplication implementation from [Tutorial3A](@ref tutorial3a). 
1. Methods for visually debugging and profiling the htgs::TaskGraphConf
2. Approaches for customizing the visualization to include additional attributes
  - Can be used to show GFlops per task or other metrics (i.e. bandwidth)
3. Strategies to optimize
  - Take advantage of BLAS libraries to utilize optimized compute kernels

Working at a higher level abstractions within HTGS enables analyzing the algorithm at that abstraction,
which can be used to identify bottlenecks and make decisions on a per-task basis. More advanced techniques, such as
changing data representations and traversal patterns can also be used to improve utilization and data reuse. 

An implementation that adds memory mapped files and the disk are also available in the HTGS-Tutorials code-base. 
Memory mapped files will only work on Linux and OSX systems. Next, Tutorial4 will revisit matrix multiplication and
incorporate NVIDIA GPUs into the computation. If you do not have an NVIDIA GPU, then skip to Tutorial6.