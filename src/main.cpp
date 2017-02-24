
//#include <htgs/api/TaskGraph.hpp>
//#include <htgs/core/rules/RuleScheduler.hpp>

#include <htgs/api/Bookkeeper.hpp>
#include <htgs/core/rules/RuleScheduler.hpp>

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

int main()
{

  TestData *testData;

  htgs::Bookkeeper<TestData> *bk = new htgs::Bookkeeper<TestData>();
  auto testRule = std::shared_ptr<TestRule>(new TestRule());

  auto ruleSched = new htgs::RuleScheduler<TestData, htgs::VoidData>(testRule);

  bk->addRuleManager(ruleSched);

//
//  htgs::TaskGraph<htgs::VoidData, htgs::VoidData> *taskGraph = new htgs::TaskGraph<htgs::VoidData, htgs::VoidData>();
  std::cout << "Hello world " << bk->getName() << std::endl;


  std::cout << "Hello world copy " << ruleSched->copy()->getName() << std::endl;

}
