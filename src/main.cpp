//#define DEBUG_FLAG

#include <future>

#include <htgs/api/Bookkeeper.hpp>
#include <htgs/api/TaskGraphConf.hpp>
#include <htgs/api/TaskGraphRuntime.hpp>
#include <htgs/api/ExecutionPipeline.hpp>

class TestData : public htgs::IData {
 public:
  TestData(int v) : val(v){  }
  int getVal() { return val; }
 private:
  int val;
};

class TestRule : public htgs::IRule<TestData, TestData> {
 public:

  TestRule(std::string name) : name(name) {}

  virtual ~TestRule() {

  }
  virtual bool isRuleTerminated(size_t pipelineId) {
    return name != "Rule1";
  }
  virtual void shutdownRule(size_t pipelineId) {  }

  virtual void applyRule(std::shared_ptr<TestData> data, size_t pipelineId) {
    if (name == "Rule1")
      addResult(data);
  }

  virtual std::string getName() {
    std::stringstream ss;
    ss << name << " " << this;
    return ss.str();
  }
 private:
  std::string name;
};

class TestRuleBad : public htgs::IRule<TestData, htgs::VoidData> {
 public:

  TestRuleBad(std::string name) : name(name) {}

  virtual ~TestRuleBad() {

  }
  virtual bool isRuleTerminated(size_t pipelineId) {
    return name != "Rule1";
  }
  virtual void shutdownRule(size_t pipelineId) {  }

  virtual void applyRule(std::shared_ptr<TestData> data, size_t pipelineId) {
  }

  virtual std::string getName() {
    std::stringstream ss;
    ss << name << " " << this;
    return ss.str();
  }
 private:
  std::string name;
};

class TestTask : public htgs::ITask<TestData, TestData> {
 public:
  TestTask(int n) : ITask(4), n(n) { }

  ~TestTask() override {

  }
  void initialize() override {
  }
  void executeTask(std::shared_ptr<TestData> data) override {
    addResult(data);
  }
  void shutdown() override {

  }
  std::string getName() override {
    std::stringstream ss;
    ss << "TestTask" + std::to_string(n);
    return ss.str();
  }

  std::string debugDotNode() override {
    std::stringstream ss;
    ss << "Addr: " << this->getAddress() << " (totPip: " << this->getNumPipelines() << ")\n" << this;
    return ss.str();
  }

  TestTask *copy() override {
    return new TestTask(n);
  }
 private:
  int n;
};

class TestAllocator : public htgs::IMemoryAllocator<double *> {
 public:
  TestAllocator(size_t size) : IMemoryAllocator(size) {}

  ~TestAllocator() override {
  }

  double *memAlloc(size_t size) override {
    return new double[size];
  }
  double *memAlloc() override {
    return new double[size()];
  }
  void memFree(double *&memory) override {
    delete []memory;
  }
};

void writeDotPng(htgs::AnyTaskGraphConf *graph, std::string baseFileName)
{
  graph->writeDotToFile(baseFileName + ".dot");
  std::string cmd("dot -Tpng -o " + baseFileName + ".png " + baseFileName + ".dot");
  int ret = system(cmd.c_str());
  if (ret != 0)
    std::cout << "Unable to execute dot command. status code: " << ret << std::endl;
}

int main()
{

#pragma GCC diagnostic ignored "-Wunused-variable"

  int NUM_DATA = 100;
  bool useBK = true;
  int nVertices = 5;
  TestData *testData;


  htgs::Bookkeeper<TestData> *bk;
  if (useBK)
    bk = new htgs::Bookkeeper<TestData>();

  auto testRule1 = std::make_shared<TestRule>("Rule1");
  auto testRule2 = std::make_shared<TestRule>("Rule2");
  auto testRule3 = std::make_shared<TestRule>("Rule3");
  auto testRule4 = std::make_shared<TestRule>("Rule4");

  auto testAllocator = std::make_shared<TestAllocator>(10);
  auto voidAllocator = std::make_shared<htgs::VoidMemoryAllocator>();

  auto tGraph = new htgs::TaskGraphConf<TestData, TestData>();


  std::vector<TestTask *> tasks;
  for (int i = 0; i < nVertices; i++)
  {
    tasks.push_back(new TestTask(i+1));
  }

  for (int i = 0; i < nVertices; i++)
  {
    if (i == 0)
    {
      tGraph->setGraphConsumerTask(tasks[i]);
    }

    if (i == nVertices-1)
    {
      tGraph->setGraphProducerTask(tasks[i]);
    }

    if (i > 0 && i < nVertices)
    {
      if (i == 2 && useBK)
      {
        tGraph->addEdge(tasks[i-1], bk);
        // TODO: Fix segfault when not connecting the graph correctly (provide more useful error report ... )
        tGraph->addRuleEdge(bk, testRule1, tasks[i]);
        tGraph->addRuleEdge(bk, testRule2, tasks[0]);
        tGraph->addRuleEdge(bk, testRule3, tasks[1]);
        tGraph->addRuleEdge(bk, testRule4, bk);
      }
      else
      {
        tGraph->addEdge(tasks[i-1], tasks[i]);
      }
    }

  }

  if (nVertices > 4) {
    tGraph->addMemoryManagerEdge<void *>("TestMemory", tasks[1], voidAllocator, 100, htgs::MMType::Static);
    tGraph->addMemoryManagerEdge<double *>("TestMemory2",
                                           tasks[1],
                                           testAllocator,
                                           100,
                                           htgs::MMType::Static);
  }


  auto mainGraph = new htgs::TaskGraphConf<TestData, TestData>();

  auto execPipeline = new htgs::ExecutionPipeline<TestData, TestData>(2, tGraph);

  execPipeline->addInputRule(new htgs::ExecutionPipelineBroadcastRule<TestData>());

  auto testInput = new TestTask(1);

  mainGraph->setGraphConsumerTask(testInput);
  mainGraph->addEdge(testInput, execPipeline);
  mainGraph->setGraphProducerTask(execPipeline);


  auto execPipeline2 = new htgs::ExecutionPipeline<TestData, TestData>(2, mainGraph);

  execPipeline2->addInputRule(new htgs::ExecutionPipelineBroadcastRule<TestData>());

  auto finalGraph = new htgs::TaskGraphConf<TestData, TestData>();
  finalGraph->setGraphConsumerTask(execPipeline2);
  finalGraph->setGraphProducerTask(execPipeline2);


  auto execGraph = finalGraph;

  writeDotPng(execGraph, "testorig");


  std::cout << "Number of graphs spawned finalGraph: " << finalGraph->getNumberOfSubGraphs() << std::endl;
  std::cout << "Number of graphs spawned mainGraph: " << mainGraph->getNumberOfSubGraphs() << std::endl;
  std::cout << "Number of graphs spawned tGraph: " << tGraph->getNumberOfSubGraphs() << std::endl;


//  execGraph->incrementGraphProducer();

  auto runtime = new htgs::TaskGraphRuntime(execGraph);

  runtime->executeRuntime();

//  for (int i = 0; i < NUM_DATA; i++)
//  {
//    writeDotPng(execGraph, "testExec");
//  }

  for (int i = 0; i < NUM_DATA; i++)
  {
    execGraph->produceData(new TestData(i));
  }

  execGraph->decrementGraphProducer();

  int count = 0;
  while(!execGraph->isOutputTerminated())
  {
    auto data = execGraph->consumeData();
    if (data == nullptr) {
      std::cout << "NULL DATA Received" << std::endl;
    }
//    else
//      std::cout << "Data: " << data->getVal() << " received" << std::endl;

//    if ((count % 9) == 0)
//      writeDotPng(execGraph, "testExec");

    count++;

  }

  std::cout << "Finished processing " << count << " elements" << std::endl;

  runtime->waitForRuntime();

//  writeDotPng(execGraph, "testExec");

  delete runtime;

  std::cout << "Test completed" << std::endl;

}
