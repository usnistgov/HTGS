
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file IMemoryReleaseRule.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief Describes how memory is released.
 * @details
 */
#ifndef HTGS_MEMORYRELEASERULE_HPP
#define HTGS_MEMORYRELEASERULE_HPP

namespace htgs {
/**
 * @class IMemoryReleaseRule IMemoryReleaseRule.hpp <htgs/api/IMemoryReleaseRule.hpp>
 * @brief Abstract class that describes when memory can be released/reused
 *
 * @details
 * This class is used anytime memory is requested by an ITask to a MemoryManager.
 * To receive memory use the function ITask::memGet
 *
 * IMemoryReleaseRule is attached to the MemoryData. The MemoryData,
 * should be added to IData as data flows through a TaskGraph until the memory
 * can be released with ITask::memRelease
 *
 * When memory is released, the MemoryManager processes the memory by first updating the
 * state with memoryUsed(), then if canReleaseMemory() returns true, the memory will be recycled.
 *
 * Example Implementation:
 * @code
 * class ReleaseCountRule : public htgs::IMemoryReleaseRule {
 * public:
 * 	ReleaseCountRule(int releaseCount) :
 * 	 releaseCount(releaseCount) {}
 *
 * 	virtual void memoryUsed() { releaseCount--; }
 *
 * 	virtual bool canReleaseMEmory() { return releaseCount == 0; }
 *
 * private:
 *  int releaseCount;
 *
 * }
 * @endcode
 *
 * Example Usage:
 * @code
 * class SampleTask : public ITask<Data1, Data2>
 * {
 *   ...
 *
 *   void executeTask(std::shared_ptr<Data1> data)
 *   {
 *     ...
 *     // Get memory from "memEdge" MemoryManager, and attach the ReleaseCount rule to the memory
 *     std::shared_ptr<htgs::MemoryData<double *>>mem = this->memGet<double *>("memEdge", new ReleaseCountRule(4));
 *
 *     ...
 *
 *     // Store the memory data to be passed along task graph
 *     addResult(new Data2(mem));
 *   }
 *   ...
 * };
 *
 * @endcode
 *
 */
class IMemoryReleaseRule {
 public:

  /**
   * Destructor
   */
  virtual ~IMemoryReleaseRule() { }

  /**
   * Pure virtual function to update the state of when memory has been used.
   */
  virtual void memoryUsed() = 0;

  /**
   * Pure virtual function to indicate when memory can be released.
   * @return whether memory can be released
   * @retval TRUE if memory is ready to be released
   * @retval FALSE if memory is not ready to be released
   */
  virtual bool canReleaseMemory() = 0;
};
}

#endif //HTGS_MEMORYRELEASERULE_HPP
