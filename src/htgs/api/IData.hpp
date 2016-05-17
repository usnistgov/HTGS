
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file IData.hpp
 * @author Timothy Blattner
 * @date Nov 11, 2015
 *
 * @brief Implements the IData class, which is used for all data types entering/leaving a task graph
 * @details
 */
#ifndef HTGS_IDATA_H
#define HTGS_IDATA_H

#include <memory>
#include <iostream>

namespace htgs {
/**
* @class IData IData.hpp <htgs/api/IData.hpp>
* @brief Class to hold any type of data.
*
* @details Defines custom types of data that is used for data entering/leaving task graphs.
* All task's input and output types must derive from IData. Once derived a child of IData can
* store any type of data.
*
* One common usage is to store MemoryData within IData to be passed between tasks and released
* later.
*
* Ordering can be used to specify the order of a task's priority queue (lowest value is first).
*
* (Optional) You can customize the ordering to your specifications by overriding the following function:
* @code
* virtual bool compare(const std::shared_ptr<IData> p2) const
* @endcode
*
*
* Example implementation:
* @code
*
* class MatrixData : public htgs::IData {
* public:
*  MatrixData(int order, double * matrix, int matrixSize) : IData(order), matrix(matrix), matrixSize(matrixSize)
*  {}
*
* // (optional) Reverses the default ordering
* virtual bool compare(const std::shared_ptr<IData> p2) const {
*   return this->getOrder() < p2.getOrder();
* }
*
* private:
*  double *matrix;
*  int matrixSize;
*
* };
* @endcode
* @note Must define the USE_PRIORITY_QUEUE directive to enable custom ordering between tasks.
*/
class IData {

 public:

  /**
   * Constructs an IData with default ordering = 0
   */
  IData() {
    this->order = 0;
  }

  /**
   * Constructs IData with integer ordering
   * @param order the order in which a task will process the data (lowest value is processed first)
   * @note Must define the USE_PRIORITY_QUEUE directive to enable custom ordering between tasks.
   */
  IData(int order) {
    this->order = order;
  }

  /**
   * Destructor
   */
  virtual ~IData() { }

  /**
   * @brief Compares two IData pointers for ordering
   * @param p1 the first IData
   * @param p2 the second IData
   * @return the ordering of the values
   * @retval TRUE if p2 is ahead of p1
   * @retval FALSE if p1 is ahead of p2
   */
  bool operator()(const std::shared_ptr<IData> p1, const std::shared_ptr<IData> p2) const {
    if (p1 && !p2)
      return false;

    if (!p1 && p2)
      return true;

    if (!p1 && !p2)
      return true;

    return p1->compare(p2);
  }

  /**
   * @brief Virtual IData comparison function, can be used for custom ordering.
   *
   * Compares the current IData with another IData to determine ordering.
   * By default, uses the order to determine the ordering. The higher the order
   * value, the higher the priority.
   *
   * @param p2 the other compared IData
   * @return The ordering of two IData
   * @retval TRUE p2 is ahead of this
   * @retval FALSE this is ahead of p2
   * @note Must define the USE_PRIORITY_QUEUE directive to enable custom ordering between tasks.
   */
  virtual bool compare(const std::shared_ptr<IData> p2) const {
    return this->getOrder() > p2->getOrder();
  }

  /**
   * @brief Gets the order of this IData
   * @return the order
   */
  int getOrder() const {
    return order;
  }

 private:
  int order; //!< The ordering of the data (lowest first)

};
}

#endif //HTGS_IDATA_H
