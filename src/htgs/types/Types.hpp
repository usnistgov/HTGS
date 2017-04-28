
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file Types.hpp
 * @author Timothy Blattner
 * @date Feb. 24, 2017
 *
 * @brief Defines common types used throughout the HTGS API and some of which that are used by users of HTGS such as the htgs::m_data_t
 */

#ifndef HTGS_TYPES_HPP
#define HTGS_TYPES_HPP

#include <vector>
#include <unordered_map>
#include <htgs/core/graph/AnyConnector.hpp>
#include <htgs/api/IRule.hpp>
#include <htgs/api/MemoryData.hpp>
#include <map>

/**
 * @namespace the Hybrid Task Graph Scheduler Namespace
 */
namespace htgs {

template<class T, class U>
using IRuleList = std::list<std::shared_ptr<IRule<T, U>>>;

/**
 * @typedef ConnectorMap
 * A mapping between the name of a task and its connector
 */
typedef std::unordered_map<std::string, std::shared_ptr<AnyConnector>> ConnectorMap;

/**
 * @typedef ConnectorPair
 * A pair used for the ConnectorMap
 */
typedef std::pair<std::string, std::shared_ptr<AnyConnector>> ConnectorPair;

/**
 * @typedef ConnectorVector
 * A vector of Connectors.
 */
typedef std::vector<std::shared_ptr<AnyConnector>> ConnectorVector;

/**
 * @typedef ConnectorVectorMap
 * An unordered mapping of string names mapping to a pointer to ConnectorVectors.
 * This data structure is used for execution pipelines and memory edges. Each ITask
 * can only have up to 1 ConnectorVector with a given name. The vector of connectors represents
 * one per execution pipeline.
 */
typedef std::unordered_map<std::string, std::shared_ptr<ConnectorVector>> ConnectorVectorMap;

/**
 * @typedef ConnectorVectorPair
 * Defines a pair to be added to a ConnectorVectorMap.
 */
typedef std::pair<std::string, std::shared_ptr<ConnectorVector>> ConnectorVectorPair;

/**
 * @typedef IRuleMap
 * Defines a mapping between an IRule pointer and the shared pointer of that IRule
 */
typedef std::map<AnyIRule *, std::shared_ptr<AnyIRule>> IRuleMap;

/**
 * @typedef IRulePair
 * Defines a pair to be added to the IRuleMap
 */
typedef std::pair<AnyIRule *, std::shared_ptr<AnyIRule>> IRulePair;

/**
 * @typedef MemAllocMap
 * Defines a mapping between a BaseMemoryAllocator and its shared_ptr
 */
typedef std::map<AnyMemoryAllocator *, std::shared_ptr<AnyMemoryAllocator>> MemAllocMap;

/**
 * @typedef MemAllocPair
 * Defines a pair to be added to the MemAllocMap
 */
typedef std::pair<AnyMemoryAllocator *, std::shared_ptr<AnyMemoryAllocator>> MemAllocPair;

/**
 * @typedef m_data_t<V>
 * Defines a shared pointer to htgs::MemoryData
 * @tparam V the memory data type
 */
template<class V>
using m_data_t = std::shared_ptr<MemoryData<V>>;

}
#endif //HTGS_TYPES_HPP
