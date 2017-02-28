//
// Created by tjb3 on 2/24/17.
//

#ifndef HTGS_EDGEDESCRIPTOR_HPP
#define HTGS_EDGEDESCRIPTOR_HPP

class EdgeDescriptor {

  virtual void applyEdge() = 0;
  virtual EdgeDescriptor copy() = 0;

};

#endif //HTGS_EDGEDESCRIPTOR_HPP
