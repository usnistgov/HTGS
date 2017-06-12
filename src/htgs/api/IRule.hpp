
// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
// NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
// You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.

/**
 * @file IRule.hpp
 * @author Timothy Blattner
 * @date Nov 11, 2015
 *
 * @brief Provides an interface to send data along RuleManager edges for processing state and dependencies
 * @details
 */
#ifndef HTGS_IRULE_HPP
#define HTGS_IRULE_HPP

#include <iostream>
#include <functional>
#include <list>
#include <mutex>
#include <htgs/core/rules/AnyIRule.hpp>

#include "IData.hpp"

namespace htgs {

template<class T>
class StateContainer;

/**
 * @class IRule IRule.hpp <htgs/api/IRule.hpp>
 * @brief Provides an interface to send data along RuleManager edges for processing state and dependencies
 * @details
 *
 * The IRule is added to a Bookkeeper and is responsible for producing data for the Bookkeeper.
 *
 * IRule behavior is defined based on the needs of an algorithm. In many cases, the IRule is used
 * to manage the global state of the computation and issue new work when dependencies are satisfied.
 *
 * The Bookkeeper accesses each IRule synchronously, so every IRule should not require a significant amount of
 * computation to finish processing an IRule.
 *
 * When a Bookkeeper is duplicated for an ExecutionPipeline, the IRule's within that Bookkeeper are shared,
 * thus ensuring that no race conditions occur when updating the state when multiple Bookkeepers are attempting
 * to process the same IRule.
 *
 * It is possible to share the same IRule among multiple bookkeepers by wrapping the IRule into a std::shared_ptr and calling
 * TaskGraphConf::addRuleEdge. The rule will be synchronously accessed among the bookkeepers.
 *
 * Example Implementation
 * @code
 * class SimpleIRule : public htgs::IRule<Data1, Data2>
 * {
 *  public:
 *   SimpleIRule() { allocateStateArray(); }
 *   ~SimpleIRule() { deallocateStateArray(); }
 *   virtual bool isRuleTerminated(int pipelineId) { return false; }
 *   virtual void shutdownRule(int pipelineId) { }
 *   virtual void applyRule(std::shared_ptr<Data1> data, int pipelineId)
 *   {
 *		// Store data updates
 *		state[data.getRow()][data.getCol()] = data;
 *
 *		// Check for some dependency
 *		if (state[data.getRow()+1][data.getCol()])
 *		{
 *		    // Send work along edge to next task
 *			addResult(new Data2(data, state[data.getRow()+1][data.getCol()]));
 *		}
 *   }
 *   virtual std::string getName() { return "SimpleIRule"; }
 *
 * private:
 *  std::shared_ptr<Data1> **state;
 * };
 * @endcode
 *
 * Example Usage:
 * @code
 * htgs::Bookkeeper<Data1> *bkTask = new htgs::Bookkeeper<Data1>();
 *
 * // SimpleIRule has input type 'Data1' and output type 'Data2'
 * // Use std::shared_ptr<SimpleIRule> if sharing among multiple bookkeepers
 * SimpleIRule *simpleRule = new SimpleIRule();
 *
 * // SimpleTask has input type 'Data2'
 * SimpleTask *resultTask = new SimpleTask();
 *
 * htgs::TaskGraphConf<VoidData, VoidData> *taskGraph = new htgs::TaskGraphConf<VoidData, VoidData>();
 *
 * // Adds simpleRule that connects the bkTask to the resultTask
 * taskGraph->addRuleEdge(bkTask, simpleRule, resultTask);
 * @endcode
 *
 * @tparam T the input data type for the IRule, T must derive from IData.
 * @tparam U the output data type for the IRule, U must derive from IData.
 *
 */
template<class T, class U>
class IRule : public AnyIRule {
  static_assert(std::is_base_of<IData, T>::value, "T must derive from IData");
  static_assert(std::is_base_of<IData, U>::value, "U must derive from IData");
 public:

  /**
   * Creates an IRule
   */
  IRule() {
    output = new std::list<std::shared_ptr<U>>();
  }

  ////////////////////////////////////////////////////////////////////////////////
  ////////////////////// VIRTUAL FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Destructor
   */
  virtual ~IRule() override {
    if (output != nullptr) {
      output->clear();
      delete output;
      output = nullptr;
    }
  }

  /**
   * @copydoc AnyIRule::canTerminateRule
   * @note By default, this function returns false
   */
  virtual bool canTerminateRule(size_t pipelineId) override { return false; }

  /**
   * @copydoc AnyIRule::shutdownRule
   */
  virtual void shutdownRule(size_t pipelineId) override {}

  /**
   * @copydoc AnyIRule::getName
   */
  virtual std::string getName() override {
    return "Unnamed IRule";
  }

  /**
  * Pure virtual function to process input data.
  * Use the addResult function to add values to the output edge.
  * @param data the input data
  * @param pipelineId the pipelineId
  *
  * @note To send data to the next edge use addResult
  */
  virtual void applyRule(std::shared_ptr<T> data, size_t pipelineId) = 0;


  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// CLASS FUNCTIONS ///////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////

  /**
   * Applies the virtual rule function and processes output
   * @param data the input data
   * @param pipelineId the pipelineId
   * @return the list of output
   * @note This function should only be called by the HTGS API
   * @internal
   */
  std::list<std::shared_ptr<U>> *applyRuleFunction(std::shared_ptr<T> data, size_t pipelineId) {
    this->output->clear();
    applyRule(data, pipelineId);
    return output;
  }

  /**
   * Adds a result value to the output
   * @param result the result value that is added
   */
  void addResult(std::shared_ptr<U> result) {
    this->output->push_back(result);
  }

  /**
   * Adds a result value to the output.
   * This will convert the pointer into a shared pointer.
   * @param result the result value that is added
   */
  void addResult(U *result) {
    this->output->push_back(std::shared_ptr<U>(result));
  }

  /**
   * Allocates a two dimensional state container using the input type of the IRule.
   * @param height the height of the state container
   * @param width the width of the state container
   * @return a pointer to the state container allocated
   */
  StateContainer<std::shared_ptr<T>> *allocStateContainer(size_t height, size_t width) {
    return new StateContainer<std::shared_ptr<T>>(height, width, nullptr);
  }

  /**
   * Allocates a two dimensional state container using the template argument.
   * @param height the height of the state container
   * @param width the width of the state container
   * @param defaultValue the value that represents no data or default value
   * @return a pointer to the state container allocated
   * @tparam V the state container type
   */
  template<class V>
  StateContainer<V> *allocStateContainer(size_t height, size_t width, V defaultValue) {
    return new StateContainer<V>(height, width, defaultValue);
  }

  /**
   * Allocates a one dimensional state container using the input type of the IRule.
   * @param size the size of the state container
   * @return a pointer to the state container allocated
   */
  StateContainer<std::shared_ptr<T>> *allocStateContainer(size_t size) {
    return new StateContainer<std::shared_ptr<T>>(size, 1, nullptr);
  }

  /**
   * Allocates a one dimensional state container using the input type of the IRule.
   * @param size the size of the state container
   * @param defaultValue the value that represents no data or default value
   * @return a pointer to the state container allocated
   * @tparam V the state container type
   */
  template<class V>
  StateContainer<V> *allocStateContainer(size_t size, V defaultValue) {
    return new StateContainer<V>(size, 1, defaultValue);
  }

 private:
  std::list<std::shared_ptr<U>>
      *output; //!< The output data that is sent as soon as the applyRule has finished processing
};

/**
 * @class StateContainer IRule.hpp <htgs/api/IRule.hpp>
 * @brief Class to hold one/two dimensional state information.
 * @details
 * This class provides a quick method for identifiying state information for data that is passed to an IRule.
 *
 * There are four helper functions to aid in creating a StateContainer within an IRule.
 *
 * The StateContainer provides three core functions for handling state.
 *
 * get - Retrieves a state value from a given index that may have been stored in the past
 * set - Sets a state value at an index
 * has - Checks whether a state value has been set at an index
 *
 * Using these functions, the IRule can quickly determine when data
 * dependencies are satisfied and get the data needed for passing to the next ITask.
 *
 * @tparam T the type of data for the state container
 *
 */
template<class T>
class StateContainer {

 public:
  /**
   * Constructs a state container with a width and height, and what it considers to be empty data.
   * The empty data is used to initialize the array of data
   * @param height the height of the state container
   * @param width the width of the state container
   * @param emptyData the data value that represents there is no data
   */
  StateContainer(size_t height, size_t width, T emptyData) {
    this->width = width;
    this->height = height;
    this->emptyData = emptyData;
    data = new T[width * height];

    for (size_t i = 0; i < width * height; i++) {
      data[i] = this->emptyData;
    }
  }

  /**
   * Destructor
   */
  ~StateContainer() {
    delete[]data;
  }

  /**
   * Sets a value (by reference) at a row column
   * @param row the row
   * @param col the column
   * @param value the value
   */
  void set(size_t row, size_t col, T &value) const {
    data[computeIndex(row, col)] = value;
  }

  /**
   * Sets a value at a row column (uses assignment operator)
   * @param row the row
   * @param col the column
   * @param value the value
   */
  void assign(size_t row, size_t col, T value) const {
    data[computeIndex(row, col)] = value;
  }

  /**
   * Sets a value (by reference) at an index
   * @param index the index
   * @param value the value
   */
  void set(size_t index, T &value) const {
    data[index] = value;
  }

  /**
   * Sets a value at an index (uses assignment operator)
   * @param index the index
   * @param value the value
   */
  void assign(size_t index, T value) const {
    data[index] = value;
  }

  /**
   * Gets a value from a row column
   * @param row the row
   * @param col the column
   * @return the value at the specified row column
   */
  T &get(size_t row, size_t col) const {
    return data[computeIndex(row, col)];
  }

  /**
   * Gets a value from an index
   * @param index the index
   * @return the value at the specified index
   */
  T &get(size_t index) const {
    return data[index];
  }

  /**
   * Removes the data from the specified row and column
   * @param row the row to remove data from
   * @param col the column to remove data from
   */
  void remove(size_t row, size_t col) {
    set(row, col, emptyData);
  }

  /**
   * Removes the data from the specified index
   * @param index the index to remove data from
   */
  void remove(size_t index) {
    set(index, emptyData);
  }

  /**
   * Checks whether the specified row column has data
   * @param row the row
   * @param col the column
   * @return whether there is data at the specified row column
   * @retval TRUE if there is data at the specified row column
   * @retval FALSE if the data at the row column is 'emptyData'
   * @note 'emptyData' is specified by the constructor of the StateContainer
   */
  bool has(size_t row, size_t col) const {
    return data[computeIndex(row, col)] != emptyData;
  }

  /**
   * Checks whether the specified index has data
   * @param index the index
   * @return whether there is data at the specified row column
   * @retval TRUE if there is data at the specified row column
   * @retval FALSE if the data at the row column is 'emptyData'
   * @note 'emptyData' is specified by the constructor of the StateContainer
   */
  bool has(size_t index) const {
    return data[index] != emptyData;
  }

  /**
   * Prints the state of the state container.
   * Iterates over all elements and prints a 1 if data is not equal to the empty data,
   * otherwise it prints 0.
   */
  void printState() {
    for (size_t r = 0; r < height; r++) {
      for (size_t c = 0; c < width; c++) {
        if (this->has(r, c))
          std::cout << "1";
        else
          std::cout << "0";
      }
      std::cout << std::endl;
    }
  }

  /**
   * Prints the contents of the state container.
   * Iterates over all elements and prints the contents within.
   */
  void printContents() {
    for (size_t r = 0; r < height; r++) {
      for (size_t c = 0; c < width; c++) {
        std::cout << this->get(r, c) << " ";
      }
      std::cout << std::endl;
    }
  }

 private:
  /**
   * Computes the one dimensional index from two dimension
   * @param row the row
   * @param col the column
   * @return the mapping from two dimensions to one dimension
   */
  size_t computeIndex(size_t row, size_t col) const {
    return row * width + col;
  }

  T *data; //!< The pointer to data for the StateContainer
  size_t width; //!< The width of the StateContainer
  size_t height; //!< The height of the StateContainer
  T emptyData; //!< The data value that represents no data
};

}

#endif //HTGS_IRULE_HPP
