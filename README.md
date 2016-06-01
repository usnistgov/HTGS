# Hybrid Task Graph Scheduler (HTGS)

An application programming interface to generate hybrid pipeline workflow systems

The API is designed to aid in creating task graphs for algorithms to obtain performance across CPUs and multiple co-processors.

Installation Instructions
========
## Dependencies:
1) g++/gcc version 4.8.4+

2) pthreads

3) doxygen (optional)

## Building and testing HTGS:
1) Download the [Zip](https://github.com/usnistgov/HTGS/archive/master.zip)

2) Extract

3) Create 'build' directory

4) In 'build' directory run cmake-gui $HTGS directory$ (or ccmake)

5) Configure parameters, such as CMAKE_INSTALL_PREFIX and whether to create the documentation or run google test suite

6) Generate make file

7) Run 'make'

8) Run 'make install'


To build the documentation run:

'make doc'

To execute the test suite run:

'make run-test' after running make


Motivation
=========
Modern compute systems are highly complex to program on, particularly when trying to balance among multiple GPU cards,
multi-core CPUs, and multiple disks. The hybrid task graph scheduler application programmer interface (HTGS API)
provides tools to transform an algorithm into a hybrid TaskGraph that is used to execute on these systems with the
aim of extracting performance through full utilization of its compute resources. Using a hybrid TaskGraph, an algorithm is executed across
many CPU threads as a multiple-producer, multiple-consumer application. Each task (function) within the graph will overlap
with other tasks, thus enabling overlapping computation with other computations (whether on CPU or multiple GPUs) and
computation with I/O (possibly multiple storage devices). This method of pipelining is the primary technique to gain performance.
The API also provides mechanisms for throttling data producers through memory managers. Each memory manager is bound
to a particular device (CPU/GPU) and manages the data based on programmer-defined rules. This approach enables data intensive
applications to work within memory limits.


Approach
=======
The intent of the hybrid task graph scheduler (HTGS) API is to transform an algorithm into a hybrid TaskGraph, which is
used to fully utilize a high performance compute system (multi-core CPUs, multiple accelerators, and multiple disks).
The elements of the hybrid TaskGraph are created using an object-oriented approach where the components within the TaskGraph
are interfaces that are used to implement depedencies, memory rules, decomposition rules, and computational functions.
Connecting these components with data edges formulates the TaskGraph. Speciality ITask objects can also be defined to expand the API
to enable new types of computation on hybrid systems; for example, the HTGS API defines an ICudaTask for NVIDIA CUDA GPUs.

The hybrid TaskGraph is a technique inspired by past research where an algorithm is represented as a hybrid pipeline
workflow system. [1][2] This API generalizes the hybrid pipeline workflow system approach.

In hybrid pipeline workflow systems, each computational component is pipelined through a multiple producer multiple consumer model.
By doing so, a pipeline is formed to overlap computation with I/O, such as disk or PCI express data transfers. The hybrid TaskGraph
implements this technique such that each ITask, which is managed by
a TaskScheduler, is executed using one or more CPU threads. When data is sent to a TaskScheduler, the ITask is executed.
Assuming data is flowing among multiple ITasks, each execute function will run concurrently. If a TaskScheduler
is overwhelmed with data, then a thread pool can be associated with a TaskScheduler. Threads within the thread pool are bound
to duplicate copies of its TaskScheduler and the underlying ITask.

An ITask runs as soon as data is sent from the TaskScheduler. Using this method there are two types of parallelism: (1)
Task parallelism: Each task executes concurrently when data is available, and (2) data parallelism: A task has a thread pool
to help process large quantities of data. If a graph executes data on the GPU, then the sub-graph that represents the
GPU computation can be encapsulated into an ExecutionPipeline ITask, which will duplicate the sub-graph; one for each
GPU on a system. Each sub-graph processes data concurrently across all GPUs attached to a system.

Another key component of HTGS is its underlying memory management system. In many TaskGraphs one ITask may be required to
allocate memory that is used by another ITask. If the ITask allocating memory processes data much faster than the other ITask,
then the memory allocation could use all the available memory in a system. To provide better fine-grained control of memory
between two ITask's a MemoryManager is introduced, which acts as a memory edge between two ITasks; one ITask that gets memory,
while the other releases memory. The memory that is allocated is recycled using a memory pool and is only released based
on memory rules that are defined and attached to memory. This system effectively
throttles a TaskGraph as the ITask getting memory will block and wait until memory is available from the MemoryManager.
For CUDA memory, a CudaMemoryManager is used.



Steps to Programming with HTGS
======
There are three steps involved in implementing an algorithm using HTGS.

1. Represent the algorithm as a dataflow graph. Where nodes in the graph represent computational entities, and edges
represent data dependencies. The computational entities should represent large components of a computation. For example
the Fast Fourier Transform of an image.

2. Implement functionality of each computational entity as independent function calls. Parameters to the function calls
could represent the data from the edges shown in the dataflow graph.

3. Use the HTGS API:
  - Create an ITask for each computational entity.
  - IData is used to represent any data that is needed for each ITask
    + Data is transmitted as input and output from an ITask, which is based on the ITask's template types T, U, respectively.
  - Connect each entity using the TaskGraph::addEdge or TaskGraph::addBookkeeperRule
    + If dependencies occur among multiple ITask's, then a Bookkeeper is used to process those dependencies. Define
an IRule that processes the dependency, which should produce work for the ITask that requires the dependency.
  - If there are memory concerns, then two tasks can be connected through a memory edge with TaskGraph::addMemoryManagerEdge.
    + One task gets the memory, and the other releases the memory. The memory is released based on a programmer-defined IMemoryRelease rule
  - For CUDA ITask's, partition the graph into CUDA-only computation and formulate the partitioned graph into an ExecutionPipeline
    + The ExecutionPipeline can be used to duplicate the graph such that each duplicated graph executes on a separate GPU in parallel



Overview of HTGS {#overview}
============
The HTGS API is split into two modules:

1. The User API
   - Classes that begin with 'I' denote interfaces that are implemented: ITask, IMemoryReleaseRule, etc.

2. The Core API

The user API is found in <htgs/api/...> and contains the API that programmers use to define a TaskGraph. The majority
 of programs should only use the user API.

The core API is found in <htgs/core/...> and holds the underlying sub-systems that the user API operates with.

Although there is a separation between the user and core APIs, there is a method to add new functionality into HTGS.
The [ICustomEdge](https://pages.nist.gov/HTGS/doxygen/classhtgs_1_1_i_custom_edge.html) is an interface that informs a TaskGraph on how to create a customized edge between
producer and consumer ITasks.
The new types of edges can either directly connect two ITask's together or two TaskSchedulers.
The main limiting factor of the ICustomEdge is it assumes that both ITasks will be added to the same TaskGraph.
If the two ITasks must be in two different TaskGraphs, then a new type of TaskGraph will need to be created by inheriting
the [TaskGraph](https://pages.nist.gov/HTGS/doxygen/classhtgs_1_1_task_graph.html) class.


Examples {#examples}
=========

[Tutorial 1](https://pages.nist.gov/HTGS/doxygen/tutorial1.html)

[Tutorial 2](https://pages.nist.gov/HTGS/doxygen/tutorial2.html)

- How to represent data using IData
- How to create an ITask
- How to create a TaskGraph
- How to execute a TaskGraph
- Sending data to TaskGraphs
- Post-processing data from TaskGraphs
- How to manage dependencies
- How to use the MemoryManager
- How to use ICudaTask
- How to use ExecutionPipelines
- Strategies for performance
- Profiling
- Debugging

Credits {#credits}
=========
Timothy Blattner

Walid Keyrouz

Milton Halem

Mary Brady

Shuvra Bhattacharyya

How to cite our work {#cite-us}
=========
If you are using the HTGS API in your research please use one of the following methods to cite us.

1) Cite the github page

2) T. Blattner, W. Keyrouz, M. Halem, M. Brady and S. S. Bhattacharyya, "A hybrid task graph scheduler for high performance image processing workflows," 2015 IEEE Global Conference on Signal and Information Processing (GlobalSIP), Orlando, FL, 2015, pp. 634-637.


Citations
------
[1]: [Blattner, T.; Keyrouz, W.; Chalfoun, J.; Stivalet, B.; Brady, M.; Shujia Zhou, "A Hybrid CPU-GPU System for Stitching Large Scale Optical Microscopy Images," in Parallel Processing (ICPP), 2014 43rd International Conference on , vol., no., pp.1-9, 9-12 Sept. 2014
doi: 10.1109/ICPP.2014.9](http://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=6957209&isnumber=6957198)

[2]: Blattner, T.; "A Hybrid CPU/GPU Pipeline Workflow System". Master's thesis, 2013. University of Maryland Baltimore County

Contact Us
=========
Timothy Blattner (timothy.blattner ( at ) nist.gov)
