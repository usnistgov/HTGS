
#include <htgs/api/Bookkeeper.hpp>

class TestData : public htgs::IData {
  TestData(int v) : val(v){  }
  int val;
};

class TestRule : public htgs::IRule<TestData, htgs::VoidData> {
 public:
  virtual ~TestRule() {

  }
  virtual bool isRuleTerminated(int pipelineId) {
    return false;
  }
  virtual void shutdownRule(int pipelineId) {

  }
  virtual void applyRule(std::shared_ptr<TestData> data, int pipelineId) {

  }
  virtual std::string getName() {
    return "TestRuleName";
  }
};

class TestTask : public htgs::ITask<TestData, htgs::VoidData> {
 public:
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

  TestData *testData;

  htgs::Bookkeeper<TestData> *bk = new htgs::Bookkeeper<TestData>();
  auto testRule = std::shared_ptr<TestRule>(new TestRule());

  auto ruleSched = new htgs::RuleScheduler<TestData, htgs::VoidData>(testRule);

  bk->addRuleManager(ruleSched);

  auto testTask = new TestTask();

  auto test = new htgs::TaskScheduler<TestData, htgs::VoidData>(testTask, 1, false, 0, 1);

//
//  htgs::TaskGraph<htgs::VoidData, htgs::VoidData> *taskGraph = new htgs::TaskGraph<htgs::VoidData, htgs::VoidData>();
  std::cout << "Hello world " << bk->getName() << std::endl;


  std::cout << "Hello world copy " << ruleSched->copy()->getName() << std::endl;

}
