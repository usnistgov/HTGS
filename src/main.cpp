//#define DEBUG_FLAG
//#define DO_SLEEP
#ifdef DO_SLEEP
#include <unistd.h>
#endif

#include <htgs/api/Bookkeeper.hpp>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>
#include <htgs/api/ExecutionPipeline.hpp>
#include <htgs/log/TaskGraphSignalHandler.hpp>

class InputData : public htgs::IData
{
 public:
  InputData(int x, int y) : x(x), y(y) {}
  int getX() const { return x; }
  int getY() const { return y; }
 private:
  int x, y;
};

class OutputData : public htgs::IData
{
 public:
  OutputData(int result) : result(result) {}
  int getResult() const { return result; }
 private:
  int result;
};

class AddTask : public htgs::ITask<InputData, OutputData>
{
 public:
  virtual AddTask *copy() { return new AddTask(); }
  virtual void executeTask(std::shared_ptr<InputData> data) {
    int sum = data->getX() + data->getY();
#ifdef DO_SLEEP
    usleep(90);
#endif
    this->addResult(new OutputData(sum));
  }
  std::string getName() override {
    return "X + Y = Z";
  }
};

class SimpleRule : public htgs::IRule<OutputData, OutputData> {
 public:

  SimpleRule() {}

  virtual void shutdownRule(size_t pipelineId) {  }

  virtual void applyRule(std::shared_ptr<OutputData> data, size_t pipelineId) {
#ifdef DO_SLEEP
    usleep(100);
#endif
    if ((data->getResult() % 2) == 0)
      addResult(data);
  }

  virtual std::string getName() {
    return "EvenRule";
  }
 private:
};

class SquareResult : public htgs::ITask<OutputData, OutputData>
{
 public:
  SquareResult(size_t numThreads) : ITask(numThreads) {}
  virtual SquareResult *copy() { return new SquareResult(this->getNumThreads()); }
  virtual void executeTask(std::shared_ptr<OutputData> data) {
#ifdef DO_SLEEP
    usleep(45);
#endif
    int result = data->getResult() * data->getResult();
    this->addResult(new OutputData(result));
  }
  std::string getName() override {
    return "X^2 = Z(" + std::to_string(this->getNumThreads()) +")";
  }
};

int main() {
  auto start = std::chrono::high_resolution_clock::now();

  AddTask *addTask = new AddTask();
  SquareResult *addTask2 = new SquareResult(3);
  SquareResult *addTask3 = new SquareResult(10);
  htgs::Bookkeeper<OutputData> *bk = new htgs::Bookkeeper<OutputData>();
  SimpleRule *rule = new SimpleRule();
  auto taskGraph = new htgs::TaskGraphConf<InputData, OutputData>();
  taskGraph->setGraphConsumerTask(addTask);
  taskGraph->addEdge(addTask, addTask2);
  taskGraph->addEdge(addTask2, bk);
  taskGraph->addRuleEdge(bk, rule, addTask3);
  taskGraph->addGraphProducerTask(addTask3);


  htgs::TaskGraphSignalHandler::registerTaskGraph(taskGraph);
  htgs::TaskGraphSignalHandler::registerSignal(SIGSEGV);
//  signalHandler.registerSignal(SIGKILL);
//  signalHandler.registerSignal(SIGTERM);

//  htgs::TaskGraphSignalHandler::registerSignal();

  int numData = 1000;
  for (int i = 0; i < numData; i++) {
    auto inputData = new InputData(i, i);
    taskGraph->produceData(inputData);
  }

  taskGraph->finishedProducingData();


  auto runtime = new htgs::TaskGraphRuntime(taskGraph);
  runtime->executeRuntime();


  while(!taskGraph->isOutputTerminated()) {
    auto data = taskGraph->consumeData();
    data = nullptr;
    if (data != nullptr) {
      std::cout << "Result: " << data->getResult() << std::endl;
    }
  }

  runtime->waitForRuntime();

  auto end = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

  std::cout << "Execution time: " << diff.count() << " ns" << std::endl;

  taskGraph->writeDotToFile("test.dot", DOTGEN_FLAG_SHOW_IN_OUT_TYPES);

  delete runtime;
}


//class TestData : public htgs::IData {
// public:
//  TestData(int v) : val(v){  }
//  int getVal() { return val; }
// private:
//  int val;
//};
//
//class TestRule : public htgs::IRule<TestData, TestData> {
// public:
//
//  TestRule(std::string name) : name(name) {}
//
//  virtual ~TestRule() {
//
//  }
//  virtual bool canTerminateRule(size_t pipelineId) {
//    return name != "Rule1";
//  }
//  virtual void shutdownRule(size_t pipelineId) {  }
//
//  virtual void applyRule(std::shared_ptr<TestData> data, size_t pipelineId) {
//    if (name == "Rule1")
//      addResult(data);
//  }
//
//  virtual std::string getName() {
//    std::stringstream ss;
//    ss << name << " " << this;
//    return ss.str();
//  }
// private:
//  std::string name;
//};
//
//class TestRuleBad : public htgs::IRule<TestData, htgs::VoidData> {
// public:
//
//  TestRuleBad(std::string name) : name(name) {}
//
//  virtual ~TestRuleBad() {
//
//  }
//  virtual bool canTerminateRule(size_t pipelineId) {
//    return name != "Rule1";
//  }
//  virtual void shutdownRule(size_t pipelineId) {  }
//
//  virtual void applyRule(std::shared_ptr<TestData> data, size_t pipelineId) {
//  }
//
//  virtual std::string getName() {
//    std::stringstream ss;
//    ss << name;
//    return ss.str();
//  }
// private:
//  std::string name;
//};
//
//class TestTask : public htgs::ITask<TestData, TestData> {
// public:
//  TestTask(int n) : ITask(4), n(n) { }
//
//  ~TestTask() override {
//
//  }
//  void initialize() override {
//  }
//  void executeTask(std::shared_ptr<TestData> data) override {
//    addResult(data);
//  }
//  void shutdown() override {
//
//  }
//  std::string getName() override {
//    std::stringstream ss;
//    ss << "TestTask" + std::to_string(n);
//    return ss.str();
//  }
//
//  std::string debugDotNode() override {
//    std::stringstream ss;
//    ss << "Addr: " << this->getAddress() << std::endl;
//    ss << "Phys Addr: " << this;
//    return ss.str();
//  }
//
//  TestTask *copy() override {
//    return new TestTask(n);
//  }
// private:
//  int n;
//};
//
//class TestAllocator : public htgs::IMemoryAllocator<double> {
// public:
//  TestAllocator(size_t size) : IMemoryAllocator(size) {}
//
//  ~TestAllocator() override {
//  }
//
//  double *memAlloc(size_t size) override {
//    return new double[size];
//  }
//  double *memAlloc() override {
//    return new double[size()];
//  }
//  void memFree(double *&memory) override {
//    delete []memory;
//  }
//};
//
//void writeDotPng(htgs::AnyTaskGraphConf *graph, std::string baseFileName)
//{
//  graph->writeDotToFile(baseFileName + ".dot");
//  std::string cmd("dot -Tpng -o " + baseFileName + ".png " + baseFileName + ".dot");
//  int ret = system(cmd.c_str());
//  if (ret != 0)
//    std::cout << "Unable to execute dot command. status code: " << ret << std::endl;
//}
//
//int main()
//{
//
//#pragma GCC diagnostic ignored "-Wunused-variable"
//
//  int NUM_DATA = 100;
//  bool useBK = true;
//  int nVertices = 3;
//  TestData *testData;
//
//
//  htgs::Bookkeeper<TestData> *bk;
//  if (useBK)
//    bk = new htgs::Bookkeeper<TestData>();
//
//  auto testRule1 = new TestRule("Rule1");
//  auto testRule2 = new TestRule("Rule2");
//  auto testRule3 = new TestRule("Rule3");
//  auto testRule4 = new TestRule("Rule4");
//
//  auto testAllocator = new TestAllocator(10);
//
//  auto tGraph = new htgs::TaskGraphConf<TestData, TestData>();
//
//
//  std::vector<TestTask *> tasks;
//  for (int i = 0; i < nVertices; i++)
//  {
//    tasks.push_back(new TestTask(i+1));
//  }
//
//  for (int i = 0; i < nVertices; i++)
//  {
//    if (i == 0)
//    {
//      tGraph->setGraphConsumerTask(tasks[i]);
//    }
//
//    if (i == nVertices-1)
//    {
//      tGraph->addGraphProducerTask(tasks[i]);
//    }
//
//    if (i > 0 && i < nVertices)
//    {
//      if (i == 2 && useBK)
//      {
//        tGraph->addEdge(tasks[i-1], bk);
//        tGraph->addRuleEdge(bk, testRule1, tasks[i]);
//        tGraph->addRuleEdge(bk, testRule2, tasks[0]);
//        tGraph->addRuleEdge(bk, testRule3, tasks[1]);
//        tGraph->addRuleEdge(bk, testRule4, bk);
//      }
//      else
//      {
//        tGraph->addEdge(tasks[i-1], tasks[i]);
//      }
//    }
//
//  }
//
//  if (nVertices > 4) {
//    tGraph->addMemoryManagerEdge("TestMemory",
//                                           tasks[1],
//                                           testAllocator,
//                                           100,
//                                           htgs::MMType::Static);
//  }
//
//
//  auto mainGraph = new htgs::TaskGraphConf<TestData, TestData>();
//
//  auto execPipeline = new htgs::ExecutionPipeline<TestData, TestData>(2, tGraph);
//
//  execPipeline->addInputRule(new htgs::ExecutionPipelineBroadcastRule<TestData>());
//
//  auto testInput = new TestTask(1);
//
//  mainGraph->setGraphConsumerTask(testInput);
//  mainGraph->addEdge(testInput, execPipeline);
//  mainGraph->addGraphProducerTask(execPipeline);
//
//
//  auto execPipeline2 = new htgs::ExecutionPipeline<TestData, TestData>(2, mainGraph);
//
//  execPipeline2->addInputRule(new htgs::ExecutionPipelineBroadcastRule<TestData>());
//
//  auto finalGraph = new htgs::TaskGraphConf<TestData, TestData>();
//  finalGraph->setGraphConsumerTask(execPipeline2);
//  finalGraph->addGraphProducerTask(execPipeline2);
//
//
//  auto execGraph = finalGraph;
//
//  writeDotPng(execGraph, "testorig");
//
//
//  std::cout << "Number of graphs spawned finalGraph: " << finalGraph->getNumberOfSubGraphs() << std::endl;
//  std::cout << "Number of graphs spawned mainGraph: " << mainGraph->getNumberOfSubGraphs() << std::endl;
//  std::cout << "Number of graphs spawned tGraph: " << tGraph->getNumberOfSubGraphs() << std::endl;
//
//
////  execGraph->incrementGraphProducer();
//
//  auto runtime = new htgs::TaskGraphRuntime(execGraph);
//
//  runtime->executeRuntime();
//
////  for (int i = 0; i < NUM_DATA; i++)
////  {
////    writeDotPng(execGraph, "testExec");
////  }
//
//  for (int i = 0; i < NUM_DATA; i++)
//  {
//    execGraph->produceData(new TestData(i));
//  }
//
//  execGraph->finishedProducingData();
//
//  int count = 0;
//  while(!execGraph->isOutputTerminated())
//  {
//    auto data = execGraph->consumeData();
//    if (data == nullptr) {
//      std::cout << "NULL DATA Received" << std::endl;
//    }
////    else
////      std::cout << "Data: " << data->getVal() << " received" << std::endl;
//
////    if ((count % 9) == 0)
////      writeDotPng(execGraph, "testExec");
//
//    count++;
//
//  }
//
//  std::cout << "Finished processing " << count << " elements" << std::endl;
//
//  runtime->waitForRuntime();
//  execGraph->writeDotToFile("test.dot", DOTGEN_FLAG_SHOW_ALL_THREADING);
//  execGraph->writeDotToFile("test.dot", DOTGEN_COLOR_MAX_Q_SZ | DOTGEN_COLOR_WAIT_TIME | DOTGEN_COLOR_COMP_TIME | DOTGEN_FLAG_SHOW_ALL_THREADING);
//
////  execGraph->printProfile();
//
////  writeDotPng(execGraph, "testExec");
//
//  delete runtime;
//
//  std::cout << "Test completed" << std::endl;
//
//}
