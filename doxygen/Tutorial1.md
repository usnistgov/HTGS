Tutorial 1 {#tutorial1}
========
[TOC]

In this tutorial we will be introducing the basics of the Hybrid Task Graph Scheduler (HTGS) API.

The [Source Code](https://github.com/usnistgov/HTGS-Tutorials/tree/master/tutorial1) can be viewed in the HTGS-Tutorials github repository. 

We will be implementing a simple add function to add two numbers and
 return the result, which introduces the API and how to work with it.

Objectives {#tut1-objectives}
=======

1. How to represent data: (input and output)
  - How to customize the priority of data (optional)

2. How to operate on data with an ITask
3. How to add an ITask to a TaskGraph
4. How to add data into a TaskGraph
5. How to process data from a TaskGraph

API Used {#tut1-api-used}
======

- \<htgs/api/IData.hpp\>
- \<htgs/api/ITask.hpp\>
- \<htgs/api/TaskGraphConf.hpp\>
- \<htgs/api/TaskGraphRuntime.hpp\>

Implementation {#tut1-implementation}
======

Below we will go into detail on each of the components needed to implement the algorithm x+y=z. Before we
implement any code, we must analyze the algorithm and transform it into a
dataflow graph. The dataflow graph provides a high level view of the algorithm such as
data dependencies, control flow, and compute instances.

Algorithm: \f$x+y=z\f$

Dataflow graph:

![XY Dataflow](tut1XY-dataflow.png)

TaskGraph:

![XY Taskgraph](tut1XY-taskgraph.png)

We transform the dataflow graph into a TaskGraph. We have two input
types for the x+y operation, so we will compose the inputs into a single object
to hold both x and y data. The result of the graph is a single value, so another
data object is used to hold the ouput. There will be one compute task, which is
responsible for the operation x+y and producing the result z.


## Data {#tut1-data}
The algorithm we are implementing adds two numbers and returns a result. One htgs::IData class will be responsible for
passing the two numbers to a htgs::ITask and another htgs::IData class will store the output. **htgs::IData is defined by inheriting the htgs::IData
interface.**

When data is passed from htgs::ITask to htgs::ITask (or as input/output of a htgs::TaskGraphConf) it is inserted into a htgs::Connector, which uses
a FIFO queue to hold data (transformed into a priority queue using the USE_PRIORITY_QUEUE directive)

If a htgs::ITask expects multiple input values, then htgs::IData can act as a container where each of those input values are stored within a single object.

All data in HTGS is represented as htgs::IData. htgs::IData is an interface and contains only one (optional)
 virtual function: htgs::IData::compare. The compare function can be used to customize
 the ordering of data in the htgs::Connector's queue. **Priority is enabled only if the USE_PRIORITY_QUEUE directive is defined.**

### Input data implementation {#tut1-input-data}

~~~~{.c}
#include <htgs/api/IData.hpp>
// with ': public htgs::IData', InputData becomes a child of IData and can be used within htgs::ITask's
class InputData : public htgs::IData
{
 public:
  InputData(int x, int y) : x(x), y(y) {}

  int getX() const { return x; }
  int getY() const { return y; }

 private:
  int x;
  int y;

};

~~~~

### Output data implementation {#tut1-output-data}

~~~~~~~~~~~~~~~~~{.c}
#include <htgs/api/IData.hpp>

class OutputData : public htgs::IData
{
public:
  // Optional IData(size_t order) constructor to specify ordering of OutputData (must define USE_PRIORITY_QUEUE directive)
  OutputData(int result) : IData(result),  result(result) {}

  int getResult() const { return result; }

 private:
  int result;
};
~~~~~~~~~~~~~~~~~

### Notes {#tut1-data-notes}

- htgs::IData can hold any number of input parameters being sent to a htgs::ITask
- Assuming USE_PRIORITY_QUEUE directive is defined
    + Uses order constructor for htgs::IData::IData(size_t order), which will provide an ordering based on the lowest value first.
    + The ordering can be customized if the htgs::IData::compare function is overriden
    + Defined by specifying -DUSE_PRIORITY_QUEUE during compilation.
      - In CMake, add_definitions(-DUSE_PRIORITY_QUEUE)

Example altering the order of data to highest value first (USE_PRIORITY_QUEUE directive to enable priority):
~~~~~~~~~~~~{.c}
bool compare(const std::shared_ptr<OutputData> p2) const {
    // getOrder() is inherited from the htgs::IData class
    return this->getOrder() < p2->getOrder();
}
~~~~~~~~~~~~

## Tasks {#tut1-tasks}

To implement the add function of the algorithm, we will only be needing one htgs::ITask. **A task is
defined by inheriting htgs::ITask.**

The htgs::ITask specifies five virtual functions that customize the functionality of a task:
1. htgs::ITask::executeTask - Consumes one input htgs::IData object and produces zero or more output htgs::IData objects. Data is produced as output for the task by calling the htgs::ITask::addResult function.
2. htgs::ITask::copy - Creates a copy of the htgs::ITask, each copy is bound to a separate thread to form a thread pool for the task
3. htgs::ITask::initialize - (Optional) Called when a CPU thread has attached to the task. This can be used to allocate local task-level memory and/or bind the task and thread to an accelerator. DEFAULT: Does nothing.
4. htgs::ITask::canTerminate - (Optional) Identifies if the task is ready to terminate. DEFAULT: Terminates when there is no longer any input coming from the task using htgs::Connector:isInputTerminated
5. htgs::ITask::shutdown - (Optional) Called when the task is terminating. DEFAULT: Does nothing.

Every htgs::ITask has an input type and and output type.
These types are defined by the first and second template parameters, respectively.

The number of threads that are associated with a htgs::ITask and how it interacts with the thread are controlled using one of the four htgs::ITask::ITask constructors. For example using htgs::ITask::ITask(size_t numThreads) will allocate numThreads threads for this htgs::ITask. By doing so, increases the number of threads consuming input data. The htgs::ITask also has additional capabilities that can be used in conjunction with the htgs::ExecutionPipeline, such as getting the pipelineId. These advanced features will be described in a later tutorial.

A htgs::TaskManager is created when adding a htgs::ITask to a htgs::TaskGraphConf. The htgs::TaskManager interacts with the htgs::ITask by sending the htgs::ITask data and producing data when the htgs::ITask::addResult is called. The htgs::TaskManager can be controlled through some of the more advanced constructors from the htgs::ITask. Below is a brief description of what these parameters do.
1. numThreads - How many threads are spawned for the htgs::ITask (>=1)
2. isStartTask - If set to TRUE, then immediately processes one data item, sent as nullptr, to the htgs::ITask::executTask function. Can be used to begin processing producing data from a task, such as reading from disk.
3. poll -  If set to TRUE, will continually poll for data from the input connector based on a timeout period. If the timeout expires, then nullptr data is sent to the htgs::ITask::executTask function
4. microTimeoutTime - The timeout time in microseconds for polling.

The interaction between the ITask, htgs::TaskManager, and htgs::TaskManagerThread is shown below:

![](tut1-itask-callgraph.png)

### AddTask Implementation {#tut1-addtask-implementation}

~~~~~~~~~~{.c}

#include "../data/InputData.h"
#include "../data/OutputData.h"

#include <htgs/api/ITask.hpp>

// with ':public htgs::ITask<InputData, OutputData>', AddTask becomes a child of ITask, 
// which consumes data of type InputData and produces data of type OutputData
class AddTask : public htgs::ITask<InputData, OutputData>
{
public:
    virtual void executeTask(std::shared_ptr<InputData> data) override {
        // Adds x + y
        int sum = data->getX() + data->getY();

        // Sends data along output edge
        this->addResult(new OutputData(sum));
    }

    virtual AddTask *copy() override {
        return new AddTask();
    }

    // Optional
    virtual void initialize() override { }

    // Optional
    virtual void shutdown() override { }

    // Optional
    virtual bool canTerminate(std::shared_ptr<htgs::AnyConnector> inputConnector) override {
        return inputConnector->isInputTerminated();
    }
};
~~~~~~~~~~

### Notes {#tut1-task-notes}

- The input and output types of the htgs::ITask are defined by the first and second template parameters, respectively.
- Memory leaks are avoided by using the C++11 std::shared_ptr class. When adding data with htgs::ITask::addResult,
the memory pointer allocated will be automatically wrapped into a std::shared_ptr to ensure it gets
freed once all references have finished referring to it. Alternatively, you can pass a std::shared_ptr to htgs::ITask::addResult

## Creating and Executing the htgs::TaskGraphConf {#tut1-create-and-execute-taskgraph}

The htgs::TaskGraphConf is used to connect htgs::ITask's into a graph that is executed concurrently. The premise of the htgs::TaskGraphConf is
to provide a separate of concerns between managing memory, dependencies, and computation. A htgs::TaskGraphConf has an input and output type, which
are used to produce and consume data to/from the htgs::TaskGraphConf.

These are the primary functions used to add a htgs::ITask to a htgs::TaskGraphConf

1. htgs::TaskGraphConf::setGraphConsumerTask
  + Sets the htgs::ITask that will be consuming data that is inserted into the htgs::TaskGraphConf during execution
  + The input type of the htgs::ITask must match the input type of the htgs::TaskGraphConf
  + There can only be one htgs::ITask that consumes data from the htgs::TaskGraphConf
    - If multiple are required, then a htgs::Bookkeeper can be used to distribute data among multiple htgs::ITask

2) htgs::TaskGraphConf::addGraphProducerTask 
  - Specifies a htgs::ITask that will be producing data for the htgs::TaskGraphConf
  - The output type of the htgs::ITask must match the output type of the htgs::TaskGraphConf  
  - There can be multiple htgs::ITask s producing data for the htgs::TaskGraphConf

3) htgs::TaskGraphConf::addEdge (Demonstrated in future tutorials)
  - Adds an edge to the graph where one htgs::ITask produces data for another htgs::ITask, which consumes that data
  - The output type of the htgs::ITask producing data must match the input type of the htgs::ITask consuming that data

4) htgs::TaskGraphConf::addRuleEdge (Demonstrated in future tutorials)
  - Connects a htgs::Bookkeeper to a htgs::ITask where a htgs::IRule determines when to produce data based on the state of the computation
  - Two variants are available
    + Specifying std::shared_ptr for the htgs::IRule allows the htgs::IRule to be shared among multiple htgs::TaskGraphConf
      - Each htgs::IRule is access synchronously through a shared mutex
    + Specifying without std::shared_ptr should not be shared among multiple htgs::TaskGraphConf
      - The htgs::TaskGraphConf wraps the memory into a std::shared_ptr internally

5) htgs::TaskGraphConf::addMemoryManagerEdge (Demonstrated in future tutorials)
  - Attaches a htgs::MemoryManager to a htgs::ITask, so the htgs::ITask can use htgs::ITask::getMemory
  - Acts as a mechanism for sharing memory among multiple htgs::ITask
  - Limited resource based on memory pool size
  - Specifies htgs::m_data_t, which can be released using htgs::ITask::releaseMemory

There are two steps necessary for adding data to a graph and ensuring the graph will finish executing.

1. (Optional) Specify an ITask that will be processing the input data with htgs::TaskGraphConf::setGraphConsumerTask
  + Alternatively can specify a task in the graph as a _startTask_ to begin processing immediately
2. When there is no more data to be produced: htgs::TaskGraphConf::finishedProducingData
  + Must be called to terminate the graph
  + If it is not called, then the htgs::TaskGraphConf will never finish executing

 To process the output of a TaskGraph use the htgs::TaskGraphConf::consumeData. To
 determine if there is no more data being produced by the TaskGraph, use htgs::TaskGraphConf::isOutputTerminated.

To execute a htgs::TaskGraphConf use the htgs::TaskGraphRuntime. The
htgs::TaskGraphRuntime will create and launch threads. If a htgs::ITask has more than
one thread specified, then the htgs::TaskGraphRuntime will duplicate the htgs::ITask such that each thread will be responsible
for a separate instance of the htgs::ITask.

The htgs::TaskGraphRuntime specifies the following functions:
1. htgs::TaskGraphRuntime::executeRuntime - Spawns and launches threads, returning back to the caller
2. htgs::TaskGraphRuntime::waitForRuntime - Waits for threads to finish (thread join)
3. htgs::TaskGraphRuntime::executeAndWaitForRuntime - Executes and then waits for the runtime to finish

Calling **delete** on the runtime will release all memory associated with the htgs::TaskGraphConf, including
htgs::ITask allocations.

### Main function (create and execute TaskGraph) {#tut1-main-function}

~~~~~~~~~~{.c}
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>
#include "tasks/AddTask.h"

int main() {

    // Creates the Task
    AddTask *addTask = new AddTask();

    // Creates the TaskGraph
    auto taskGraph = new htgs::TaskGraphConf<InputData, OutputData>();

    // Declares that AddTask will be processing the input of a TaskGraph
    taskGraph->setGraphConsumerTask(addTask);

    // Declares that AddTask will be producing data for the output of a TaskGraph
    taskGraph->addGraphProducerTask(addTask);

    // Launch the taskGraph
    auto runtime = new htgs::TaskGraphRuntime(taskGraph);

    runtime->executeRuntime();

    int numData = 10;

    // Main thread producing data
    for (int i = 0; i < numData; i++)
    {
        auto inputData = new InputData(i, i);
        taskGraph->produceData(inputData);
    }

    // Indicate that the main thread has finished producing data
    taskGraph->finishedProducingData();

    // Wait until the runtime has finished processing data
    runtime->waitForRuntime();

    // Process the ouput of the TaskGraph until no more data is available
    // Could process this data prior to runtime->waitForRuntime()
    while (!taskGraph->isOutputTerminated())
    {
        auto data = taskGraph->consumeData();

        // Good idea to check for nullptr data in case termination makes its way out of consumeData from taskGraph
        if (data != nullptr)
        {
            int result = data->getResult();
            std::cout << "Result: " << result << std::endl;
        }
    }

    // Release all memory for the graph
    delete runtime;
}
~~~~~~~~~~

Sample execution:
~~~~
./tutorial1
Result: 0
Result: 2
Result: 4
Result: 6
Result: 8
Result: 10
Result: 12
Result: 14
Result: 16
Result: 18
~~~~


### Debugging and profiling a htgs::TaskGraphConf {#tut1-debug}

If there are complications when running a htgs::TaskGraphConf, then the configuration can be saved as a dot
 file with the function htgs::TaskGraphConf::writeDotToFile, which can be visually shown
 with the following command 'dot -Tpng \<filename\> -o \<filename\>.png' from [Graphviz](http://www.graphviz.org).
 Below is an example dot file that is generated and the associated image representation:

 Dot file generated using "taskGraph->writeDotToFile("tutorial1.dot")":
 ~~~~~
digraph {
node[shape=record, fontsize=10, fontname="Verdana"];
edge[fontsize=10, fontname="Verdana"];
graph [compound=true];
x10f6380 -> x10f6010;
x10f6380[label="",shape=box,style=filled,color=black,width=.2,height=.2];
x10f6010 -> x10f6720;
x10f6720[label="",shape=box,style=filled,color=black,width=.2,height=.2];
x10f6010[label="x+y=z"];
}
 ~~~~~

 And the image generated with GraphViz using "dot -Tpng tutorial1.dot -o tutorial1.png"
![Tutorial1 TaskGraph](tutorial1.png)


If the htgs::TaskGraphConf::writeDotToFile is used after the htgs::TaskGraphRuntime has finished and the PROFILE directive has been defined,
then profiling data will be generated within the dot file representation. Various flags can be used to customize the dot file, found in <htgs/types/TaskGraphDotGenFlags.hpp>.


### Notes {#taskgraph-notes}

- The input and output types for each Task being added must match based on the way they are being added
into a TaskGraph
    + Example 1: htgs::TaskGraphConf::addEdge types match based on output of producer and input of consumer
    + Example 2: htgs::TaskGraphConf::setGraphConsumerTask types match based on input of graph and input on consumer
- To release all memory for HTGS, you only need to delete the runtime.
- Threading is managed by the runtime, which will spawn threads and join on them.
- If the main thread or some other component is producing data for a TaskGraph:
    1. Specify the ITask that will be processing the input data with  htgs::TaskGraphConf::setGraphConsumerTask
    2. Always call htgs::TaskGraphConf::finishedProducingData when there is no more data to be produced
- Use the htgs::TaskGraphConf::consumeData to retrieve the output from a TaskGraph.
- Use the htgs::TaskGraphConf::isOutputTerminated to check if the TaskGraph has finished producing data.


Summary {#summary}
======

In this tutorial, we looked at the basics of the HTGS API.
- Data using htgs::IData
- Compute functions using the htgs::ITask interface
- Creating a htgs::TaskGraphConf
- Executing a htgs::TaskGraphConf with the htgs::TaskGraphRuntime
- Producing data for a htgs::TaskGraphConf
- Consuming data produced by a htgs::TaskGraphConf

In [Tutorial2a](@ref tutorial2a) and [Tutorial2b](@ref tutorial2b), we will introduce two operations that assist in representing algorithms that contain
dependencies with the htgs::Bookkeeper and strict memory limitations using the htgs::MemoryManager.

Additional information:
- Header files that begin the 'I' denote an interface that is to be implemented; examples: htgs::IData and htgs::ITask
- a htgs::ITask is managed by a htgs::TaskManager which works with the htgs::ITask's input and output htgs::Connectors, and calls the
underlying htgs::ITask functions.
- The htgs::Task use the htgs::Connector, which are setup when adding an ITask to a htgs::TaskGraphConf
- When transforming an algorithm into a htgs::TaskGraphConf, it helps to create a dataflow representation first to aid in understanding
data dependencies and parallelism. This is particularly useful when determining how to represent htgs::IData, which will be further demonstrated in [Tutorial2a](@ref tutorial2a) 
