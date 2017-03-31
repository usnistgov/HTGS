//
// Created by tjb3 on 3/31/17.
//

#ifndef HTGS_SIMPLECUDAALLOCATOR_H
#define HTGS_SIMPLECUDAALLOCATOR_H

#include <htgs/api/IMemoryAllocator.hpp>
#include <cuda_runtime.h>
class SimpleCudaAllocator : public htgs::IMemoryAllocator<double>
{
 public:
  SimpleCudaAllocator(size_t size) : IMemoryAllocator(size) {}

  ~SimpleCudaAllocator() override {  }
  double *memAlloc(size_t size) override {
    double *mem;
    cudaMalloc(&mem, size*sizeof(double));
    return mem;
  }
  double *memAlloc() override {
    double *mem;
    cudaMalloc(&mem, this->size()*sizeof(double));
    return mem;
  }

  void memFree(double *&memory) override {
    cudaFree(memory);
  }
};

#endif //HTGS_SIMPLECUDAALLOCATOR_H
