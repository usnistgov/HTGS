
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file CudaMemoryManager.hpp
 * @author Timothy Blattner
 * @date Dec 2, 2015
 *
 * @brief Provides the implementation for a MemoryManager for Cuda MemoryData
 */
#ifndef HTGS_CUDAMEMORYMANAGER_HPP
#define HTGS_CUDAMEMORYMANAGER_HPP
#ifdef USE_CUDA

#include <cuda.h>
#include <htgs/core/memory/MemoryManager.hpp>
#include <htgs/api/IMemoryAllocator.hpp>

namespace htgs {
/**
 * @class CudaMemoryManager CudaMemoryManager.hpp <htgs/core/memory/CudaMemoryManager.hpp>
 * @brief Implements a MemoryManager that binds the thread responsible for the MemoryManager to a CUDA GPU
 * prior to allocating memory.
 * @details
 * Once a TaskGraphRuntime binds a thread to the CudaMemoryManager and calls its initialize function, the CUDA GPU specified
 * by the pipelineId of the CudaMemoryManager is bound to the thread. This pipelineId accesses a CUcontext, so the number of
 * pipelines spawned for the ExecutionPipeline task should match the number of CUcontexts passed to the CudaMemoryManager
 * If the Task is not associated with an ExecutionPipeline, then there only needs to be one CUcontext.
 * @tparam T the input/output MemoryData type for the CudaMemoryManager; i.e.; cufftDoubleComplex
 */
template<class T>
class CudaMemoryManager : public MemoryManager<T> {

 public:
  /**
   * Creates a CudaMemoryManager.
   * The CUcontext should contain enough CUcontext such that there is one per Cuda GPU if this Task is
   * added into an ExecutionPipeline.
   *
   * @param name the name of the memory manager edge
   * @param contexts the Cuda contexts
   * @param memoryPoolSize the size of the memory pool
   * @param memoryAllocator the memory allocator describing how memory is allocated for the GPU.
   * @param type the memory manager type
   */
  CudaMemoryManager(std::string name,
                    CUcontext *contexts,
                    size_t memoryPoolSize,
                    std::shared_ptr<IMemoryAllocator < T>> memoryAllocator,
                    MMType type) :
      MemoryManager<T>(name, memoryPoolSize, memoryAllocator, type) {
    this->contexts = contexts;
    if (type != MMType::Static)
    {
      std::cerr << "WARNING: The CudaMemoryManagers " << name << " should use Static memory allocation to avoid "
          "unnecessary GPU synchronization" << std::endl;
    }
  }

  /**
   * Initializes the CudaMemoryManager by setting which GPU the CudaMemoryManager is repsonsible prior to allocating memory.
   * The initialize routine is called after a thread has been bound to the Task, thus enforcing the Task to
   * allocate memory on the specified Cuda GPU based on the pipelineId associated with the Task managing the
   * CudaMemoryManager.
   */
  void initialize() override {
    cuCtxSetCurrent(this->contexts[this->getPipelineId()]);
    MemoryManager<T>::initialize();
  }

  /**
   * Gets the name of the CudaMemoryManager
   * @return
   */
  std::string getName() override {
    return "Cuda" + MemoryManager<T>::getName();
  }

  /**
   * Creates a shallow copy of the CudaMemoryManager
   * @return the copy of the CudaMemoryManager
   */
  MemoryManager <T> *copy() override {
    return new CudaMemoryManager(this->getMemoryManagerName(),
                                 this->contexts,
                                 this->getMemoryPoolSize(),
                                 this->getAllocator(),
                                 this->getType());
  }

 private:
  CUcontext *contexts; //!< The array of CUDA contexts
};

}
#endif //USE_CUDA
#endif //HTGS_CUDAMEMORYMANAGER_HPP
