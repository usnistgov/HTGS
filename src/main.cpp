
#include <htgs/api/Bookkeeper.hpp>
#include <htgs/api/TaskGraph.hpp>

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
    return false;
  }
  virtual void shutdownRule(size_t pipelineId) {  }

  virtual void applyRule(std::shared_ptr<TestData> data, size_t pipelineId) {  }

  virtual std::string getName() {
    return name;
  }
 private:
  std::string name;
};

class TestTask : public htgs::ITask<TestData, TestData> {
 public:
  TestTask() : ITask(100) { }

  ~TestTask() override {

  }
  void initialize() override {
  }
  void executeTask(std::shared_ptr<TestData> data) override {

  }
  void shutdown() override {

  }
  std::string getName() override {
    return "TestTask";
  }

  TestTask *copy() override {
    return new TestTask();
  }
};

int main()
{

#pragma GCC diagnostic ignored "-Wunused-variable"
  TestData *testData;

  htgs::Bookkeeper<TestData> *bk = new htgs::Bookkeeper<TestData>();
  auto testRule1 = new TestRule("Rule1");
  auto testRule2 = new TestRule("Rule2");
  auto testRule3 = new TestRule("Rule3");
  auto testRule4 = new TestRule("Rule4");

  auto tGraph = new htgs::TaskGraph<TestData, TestData>();

  int nVertices = 5;
  std::vector<TestTask *> tasks;
  for (int i = 0; i < nVertices; i++)
  {
    tasks.push_back(new TestTask());
  }

  for (int i = 0; i < nVertices; i++)
  {
    if (i == 0)
    {
      tGraph->setGraphConsumerTask(tasks[i]);
    }
    else if (i == nVertices-1)
    {
      tGraph->setGraphProducerTask(tasks[i]);
    }

    if (i > 0 && i < nVertices)
    {
      if (i == 2)
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


//
//
//  tGraph->setGraphConsumerTask(testTask);
//  tGraph->setGraphProducerTask(testTask);


  tGraph->writeDotToFile("test.dot");

  system("dot -Tpng -o test.png test.dot");


//
//  htgs::TaskGraph<htgs::VoidData, htgs::VoidData> *taskGraph = new htgs::TaskGraph<htgs::VoidData, htgs::VoidData>();
  std::cout << "Hello world " << bk->getName() << std::endl;


}
