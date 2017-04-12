
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file IMemoryAllocator.hpp
 * @author Timothy Blattner
 * @date Nov 16, 2015
 *
 * @brief Defines how memory is allocated and freed.
 * @details
 */
#ifndef HTGS_MEMORYALLOCATOR_HPP
#define HTGS_MEMORYALLOCATOR_HPP
#include <functional>
#include <htgs/core/memory/AnyMemoryAllocator.hpp>

namespace htgs {
/**
 * @class IMemoryAllocator IMemoryAllocator.hpp <htgs/api/IMemoryAllocator.hpp>
 * @brief Abstract class that describes how memory is allocated and freed
 *
 * @details
 * This class is used in conjunction with TaskGraphConf::addMemoryManagerEdge and is required
 * in order to define how a MemoryManager allocates memory for its memory pool.
 *
 * If the memory allocator is to be shared among multiple memory managers, then the allocator must be wrapped into
 * a std::shared_ptr.
 *
 * Example implementation:
 * @code
 * class FFTWMemoryAllocator : public htgs::IMemoryAllocator<fftw_complex>
 * {
 *   FFTWMemoryAllocator(size_t size) : IMemoryAllocator(size) {}
 *
 *   fftw_complex *memAlloc(size_t size) override {
 *     return fftw_alloc_complex(size);
 *   }
 *
 *   fftw_complex *memAlloc() override {
 *   	return fftw_alloc_complex(this->size());
 *   }
 *
 *   void memFree(fftw_complex *&memory) override {
 *     fftw_free(memory);
 *   }
 *
 * };
 * @endcode
 *
 * Example usage:
 * @code
 * int memoryPoolSize = 50;
 * FFTTask *fftTask = new FFTTask();
 * MatMulTask *matMul = new MatMulTask();
 *
 * htgs::TaskGraphConf<FFTData, VoidData> *taskGraph = new htgs::TaskGraphConf();
 *
 * taskGraph->addEdge(fftTask, matMul);
 *
 * taskGraph->addMemoryManagerEdge("fft", fftTask, new FFTWMemoryAllocator(imageSize), memoryPoolSize, htgs::MMType::Static);
 * @endcode
 * @tparam T the memory type
 */
template<class T>
class IMemoryAllocator : public AnyMemoryAllocator {
 public:
  /**
   * Creates a memory allocator
   * @param size the number of elements to allocate
   */
  IMemoryAllocator(size_t size) : AnyMemoryAllocator(size) {}

  /**
   * Destructor
   */
  virtual ~IMemoryAllocator() {}

  /**
   * Pure virtual function that allocates a piece of memory with a specific size.
   * @return an allocated piece of memory with type T
   */
  virtual T *memAlloc(size_t size) = 0;

  /**
   * Pure virtual function that allocates a piece of memory.
   * @return an allocated piece of memory with type T
   */
  virtual T *memAlloc() = 0;

  /**
   * Pure virtual function the frees memory
   * @param memory the memory to be freed
   */
  virtual void memFree(T *&memory) = 0;
};
}

#endif //HTGS_MEMORYALLOCATOR_HPP
