//
// Created by tjb3 on 2/24/17.
//

#ifndef HTGS_TYPES_HPP
#define HTGS_TYPES_HPP

#include <vector>
#include <unordered_map>
#include <htgs/core/graph/AnyConnector.hpp>
#include <htgs/api/MemoryData.hpp>

namespace htgs {
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

template <class V>
using m_data_t = std::shared_ptr<MemoryData<V>>;

}
#endif //HTGS_TYPES_HPP
