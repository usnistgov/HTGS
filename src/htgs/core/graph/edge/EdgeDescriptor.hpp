//
// Created by tjb3 on 2/24/17.
//

#ifndef HTGS_EDGEDESCRIPTOR_HPP
#define HTGS_EDGEDESCRIPTOR_HPP

#include <cstddef>
#include <htgs/core/graph/AnyTaskGraphConf.hpp>

namespace htgs {

class EdgeDescriptor {
 public:
  virtual ~EdgeDescriptor() {}
  virtual void applyEdge(AnyTaskGraphConf *graph) = 0;
  virtual EdgeDescriptor *copy(AnyTaskGraphConf *graph) = 0;

};
}

#endif //HTGS_EDGEDESCRIPTOR_HPP
