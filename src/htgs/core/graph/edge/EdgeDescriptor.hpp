//
// Created by tjb3 on 2/24/17.
//

#ifndef HTGS_EDGEDESCRIPTOR_HPP
#define HTGS_EDGEDESCRIPTOR_HPP

#include <cstddef>
#include <htgs/core/graph/AnyTaskGraph.hpp>

namespace htgs {

class EdgeDescriptor {
 public:
  virtual ~EdgeDescriptor() {}
  virtual void applyEdge(AnyTaskGraph *graph) = 0;
  virtual EdgeDescriptor *copy() = 0;

};
}

#endif //HTGS_EDGEDESCRIPTOR_HPP
