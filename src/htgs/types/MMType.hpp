
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file MMType.h
 * @author Timothy Blattner
 * @date Feb 8, 2016
 *
 * @brief Defines the Memory Manager types MMType
 */
#ifndef HTGS_MMTYPE_HPP
#define HTGS_MMTYPE_HPP

namespace htgs {
/**
 * @enum MMType
 * @brief The memory manager types.
 * @details
 * MMType::Static
 * Indicates the static memory manager is to be used.
 * This memory manager will allocate all memory during initialization and free memory during shutdown.
 * The memory is stored in a memory pool, which is recycled based on memory rules.
 * To create this edge use the TaskGraph::addMemoryManagerEdge with MMType::Static specified
 * To get memory use ITask::memGet<type>(std::string name, IMemoryReleaseRule *rule)
 * To release memory use ITask::memRelease(std::string name, std::shared_ptr<MemoryData<V>> memory)
 *
 * MMType::Dynamic
 * Indicates the dynamic memory manager is to be used.
 * This memory manager will allocate memory from within an ITask's ITask::memGet function and the memory is
 * freed within the MemoryManager based on the memory's IMemoryReleaseRule.
 * To create this edge use the TaskGraph::addMemoryManagerEdge with MMType::Dynamic specified
 * To get memory use ITask::memGet<type>(std::string name, IMemoryReleaseRule *rule, size_t numElements)
 * To release memory use ITask::memRelease(std::string name, std::shared_ptr<MemoryData<V>> memory)
 *
 */
enum class MMType {
  Static, //!< Indicates static memory management
  Dynamic, //!< Indicates dynamic memory management
};
}

#endif //HTGS_MMTYPE_HPP
