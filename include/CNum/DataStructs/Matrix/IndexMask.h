#ifndef INDEX_MASK_H
#define INDEX_MASK_H

#include <stdexcept>
#include <memory>
#include <numeric>
#include <algorithm>

namespace CNum::DataStructs {
  template <typename T> class Matrix;

  /**
   * @class IndexMask
   * @brief A list of indecies representing a subset or ordering of data
   *
   * The index mask at its core is a list of indeces that can be used to reorder and get subsets of
   * datasets. 
   */
  class IndexMask {
  private:
    ::std::unique_ptr<size_t[]> _mask;
    size_t _size;

    /// @brief Copy logic
    void copy(const IndexMask &other) noexcept;
    /// @brief Move logic
    void move(IndexMask &&other) noexcept;
  
  public:
    IndexMask() = delete;
    /// @brief Constructor
    IndexMask(::std::unique_ptr<size_t[]> mask, size_t size);

    /// @brief Copy constructor
    IndexMask(const IndexMask &other) noexcept;
    /// @brief Copy equals operator
    IndexMask &operator=(const IndexMask &other) noexcept;

    /// @brief Move constructor
    IndexMask(IndexMask &&other) noexcept;
    /// @brief Move equals operator
    IndexMask &operator=(IndexMask &&other) noexcept;

    /// @brief Apply an index mask to a Matrix
    ///
    /// Applying an index mask creates a new Matrix using the rows of the first that appear in
    /// the index mask, in the order they appear
    /// @tparam T The data type of the Matrix
    /// @return The masked Matrix
    template <typename T>
    ::CNum::DataStructs::Matrix<T> matrix_apply_mask(const ::CNum::DataStructs::Matrix<T> &m) const;

    /// @brief Apply an index mask to a Matrix column wise
    ///
    /// Applying an index mask column wise  creates a new Matrix using the columns of the first that
    /// appear in the index mask, in the order they appear
    /// @tparam T The data type of the Matrix
    /// @return The masked Matrix
    template <typename T>
    ::CNum::DataStructs::Matrix<T> matrix_apply_mask_col_wise(const ::CNum::DataStructs::Matrix<T> &m) const;

    /// @brief Create an index mask
    /// @tparam Container an STL container or a Matrix
    /// @tparam T The data type stored in Container
    /// @tparam CompareFunction The function used to sort (greater by default)
    template <typename Container, typename T, typename CompareFunction = ::std::greater<T>>
    static IndexMask argsort(const Container &container);
  };

#include "CNum/DataStructs/Matrix/IndexMask.tpp"
};

#endif
