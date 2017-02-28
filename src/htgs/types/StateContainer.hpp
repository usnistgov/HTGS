//
// Created by tjb3 on 2/28/17.
//

#ifndef HTGS_STATECONTAINER_HPP
#define HTGS_STATECONTAINER_HPP
/**
 * @class StateContainer StateContainer.hpp <htgs/api/StateContainer.hpp>
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
template <class T>
class StateContainer {

 public:
  /**
   * Constructs a state container with a width and height, and what it considers to be empty data.
   * The empty data is used to initialize the array of data
   * @param height the height of the state container
   * @param width the width of the state container
   * @param emptyData the data value that represents there is no data
   */
  StateContainer(size_t height, size_t width, T emptyData)
  {
    this->width = width;
    this->height = height;
    this->emptyData = emptyData;
    data = new T[width*height];

    for (size_t i = 0; i < width*height; i++)
    {
      data[i] = this->emptyData;
    }
  }

  /**
   * Destructor
   */
  ~StateContainer()
  {
    delete []data;
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
  void remove (size_t row, size_t col) {
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
  void printState()
  {
    for (size_t r = 0; r < height; r++)
    {
      for (size_t c = 0; c < width; c++)
      {
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
  void printContents()
  {
    for (size_t r = 0; r < height; r++)
    {
      for (size_t c = 0; c < width; c++)
      {
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
  size_t computeIndex(size_t row, size_t col) const
  {
    return row * width + col;
  }

  T *data; //!< The pointer to data for the StateContainer
  size_t width; //!< The width of the StateContainer
  size_t height; //!< The height of the StateContainer
  T emptyData; //!< The data value that represents no data
};
#endif //HTGS_STATECONTAINER_HPP
