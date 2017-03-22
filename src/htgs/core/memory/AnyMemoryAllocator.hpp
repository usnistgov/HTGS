//
// Created by tjb3 on 3/22/17.
//

#ifndef HTGS_ANYMEMORYALLOCATOR_HPP
#define HTGS_ANYMEMORYALLOCATOR_HPP
#include <cstddef>
namespace htgs {

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
