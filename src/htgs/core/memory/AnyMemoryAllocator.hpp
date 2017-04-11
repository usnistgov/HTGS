// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.
/**
 * @file AnyMemoryAllocator.hpp
 * @author Timothy Blattner
 * @date March 22, 2017
 *
 * @brief Implements the the base class for memory allocators to remove the
 * template argument and provide general functions used for any memory allocator.
 */

#ifndef HTGS_ANYMEMORYALLOCATOR_HPP
#define HTGS_ANYMEMORYALLOCATOR_HPP
#include <cstddef>
namespace htgs {

/**
 * @class AnyMemoryAllocator AnyMemoryAllocator.hpp <htgs/core/memory/AnyMemoryAllocator.hpp>
 * @brief Create a base class for a memory allocator that removes the template and defines
 * common functionality for any memory allocator.
 *
 */
class AnyMemoryAllocator {
 public:
  /**
   * Creates a memory allocator
   * @param size the number of elements to allocate
   */
  AnyMemoryAllocator(size_t size) : _size(size) {}

  /**
   * Destructor
   */
  virtual ~AnyMemoryAllocator() {}

  /**
   * Gets the size
   * @return the size
   */
  size_t size() const { return this->_size; };

 private:
  size_t _size; //!< The size of the memory (in elements)

};

}
#endif //HTGS_ANYMEMORYALLOCATOR_HPP
