#ifndef MATRIX_H
#define MATRIX_H

#include "CNum/DataStructs/Matrix/IndexMask.h"
#include "CNum/DataStructs/Matrix/BinaryMask.h"
#include "CNum/Multithreading/ThreadPool.h"
#include "CNum/DataStructs/Views/StrideView.h"

#include <iostream>
#include <vector>
#include <atomic>
#include <memory>
#include <functional>
#include <cmath>
#include <future>
#include <span>
#include <cstring>
#include <stdexcept>
#include <string>

namespace CNum::DataStructs {
  /**
   * @enum dim
   * @brief Either a ROW or COL (column)
   */
  enum Dim: uint8_t {
    ROW,
    COL
  };

  class IndexMask;
  class BinaryMask;
  
  /**
   * @class Matrix
   * @brief 2d array abstraction
   *
   * Used for storing 2d, tabular data. Used in conjuction with CNum ML models
   * and linear algebra operations
   * @tparam T The type of the data stored
   */
  template <typename T>
  class Matrix {
  private:
    ::std::unique_ptr<T[]> _data;
    size_t _cols;
    size_t _rows;

    /// @brief Parrallel element wise operation abstraction
    /// @param val the value used in the element wise operation
    /// @param func the function that performs the element-wise operation (for a single element)
    /// @return The resultant matrix
    Matrix<T> element_wise(T val, ::std::function< void(T &, T) > func) const noexcept;

    /// @brief Move Logic
    void move(Matrix<T> &&other) noexcept;

    /// @brief Copy Assignment
    void copy(const Matrix<T> &other) noexcept;

    /// @brief Parrellel Execution 
    /// @param n_threads The number of threads with which to perform the operation (thread pool only handles same number of threads as number of processors available at a time)
    /// @param total_el The total number of elements to execute the operation on
    /// @param callback The operation to perform on each element
    // Data must be aligned to the cache-line size to avoid false sharing
    static void par_execute(uint8_t num_threads,
			    size_t total_el,
			    ::std::function< void(size_t) > callback);
  
  public:
    /// @brief Default Overloaded Constructor
    /// @param rows Number of rows in the matrix
    /// @param cols Number of columns in the matrix
    /// @param ptr The unique pointer containing the matrix data
    Matrix(size_t rows = 0, size_t cols = 0, ::std::unique_ptr<T[]> ptr = nullptr);

    /// @brief Copy Constructor
    Matrix(const Matrix &other) noexcept;

    /// @brief Copy Logic
    Matrix<T> &operator=(const Matrix &other) noexcept;

    /// @brief Move Constructor
    Matrix(Matrix &&other) noexcept;

    /// @brief Move Assignment
    Matrix<T> &operator=(Matrix &&other) noexcept;

    /// @brief Destructor
    ~Matrix();

    /// @brief Dot Product
    /// @param other Another matrix with which to perform a dot product
    /// @returns The result of the dot product
    Matrix<T> operator*(const Matrix &other) const;

    /// @brief Scale a matrix
    /// @param scale_factor The factor with which to scale the matrix
    Matrix<T> operator*(T scale_factor) const noexcept;

    /// @brief Vector dot product (1d) 
    /// @param other The other vector with which to perform the dot product (shape=(n, 1))
    /// @return The result of the dot product (single value)
    T dot(const Matrix<T> &other) const;

    /// @brief Add two matrices element wise
    /// @param other The matrix to add
    /// @return The resultant matrix
    Matrix<T> operator+(const Matrix &other) const;

    /// @brief Add a value to every element in a matrix
    /// @param a The value to add
    /// @return The resultant matrix
    Matrix<T> operator+(T a) const noexcept;

    /// @brief Subtract two matrices element wise
    /// @param other The matrix to subtract
    /// @return The resultant matrix
    Matrix<T> operator-(const Matrix &other) const;

    /// @brief Subtract a value to every element in a matrix
    /// @param a The element to subtract
    /// @return The resultant matrix
    Matrix<T> operator-(T a) const noexcept;

    /// @brief Take the absolute value of all elements in a matrix
    /// @return The matrix with all non-negative values
    Matrix<T> abs() const;

    /// @brief Square all elements in a matrix
    /// @return The matrix with all squared values
    Matrix<T> squared() const;

    /// @brief Standardize Matrix
    /// @return The standardized matrix
    Matrix<T> standardize() const;

    /// @brief Get the sum of all elements in a matrix
    /// @return The sum
    T sum() const;

    /// @brief Get the mean of all values in a matrix
    /// @return The mean
    T mean() const;

    /// @brief Get the standard deviation of all elements in a matrix
    /// @return The standard deviation
    T std() const;

    /// @brief Get value of a matrix
    /// @param row The row of the value
    /// @param col The column of the value
    /// @return The value of matrix[i][j]
    T get(size_t row, size_t col) const;

    /// @brief Get a copy of a Row/Col (prefer views for memory effeciency)
    /// @param d either ROW or COL
    /// @param idx the index of the row/col
    /// @return The row or column
    Matrix<T> get(Dim d, size_t idx) const;

    /// @brief Get a column view
    /// @param idx The index of the column
    /// @return The view
    CNum::DataStructs::Views::StrideView<T> get_col_view(size_t idx) const;

    /// @brief Get a row view
    /// @param idx The index of the row
    /// @return The view
    ::std::span<T> get_row_view(size_t idx) const;

    /// @brief Get the value at index idx of a Matrix with shape=(n,1)
    /// @param idx The index of the value
    /// @return The value at idx
    T operator[](size_t idx) const;

    /// @brief Apply a binary mask
    /// @param m The mask to apply
    /// @return The masked matrix
    Matrix<T> operator[](const BinaryMask &bin_mask) const;

    /// @brief Apply index mask
    /// @param idx_mask The index mask to apply
    /// @return The resultant matrix
    Matrix<T> operator[](const IndexMask &idx_mask) const noexcept;

    /// @brief Apply IndexMask column wise
    /// @param idx_mask The mask containing the column indeces to preserve
    /// @return The masked matrix
    Matrix<T> col_wise_mask_application(const IndexMask &idx_mask) const noexcept;

    /// @brief Create a binary mask of values less than another
    /// @param val The value with which to compare the elements of the Matrix to
    /// @return The resultant binary mask
    BinaryMask operator<(T val) const;
    
    /// @brief Create a binary mask of values less than or equal to another
    /// @param val The value with which to compare the elements of the Matrix to
    /// @return The resultant binary mask
    BinaryMask operator<=(T val) const;
    
    /// @brief Create a binary mask of values greater than another
    /// @param val The value with which to compare the elements of the Matrix to
    /// @return The resultant binary mask
    BinaryMask operator>(T val) const;
    
    /// @brief Create a binary mask of values greater than or equal to another
    /// @param val The value with which to compare the elements of the Matrix to
    /// @return The resultant binary mask
    BinaryMask operator>=(T val) const;
    
    /// @brief Create a binary mask of values equal to another
    /// @param val The value with which to compare the elements of the Matrix to
    /// @return The resultant binary mask
    BinaryMask operator==(T val) const;
    
    /// @brief Create a binary mask of values not equal to another
    /// @param val The value with which to compare the elements of the Matrix to
    /// @return The resultant binary mask
    BinaryMask operator!=(T val) const;

    /// @brief Argsort
    /// @param descending Whether or not to sort in descending order
    /// @return An index mask with the sorting order
    IndexMask argsort(bool descending = false) const;

    /// @brief Transpose a matrix
    /// @return Transposed matrix
    Matrix<T> transpose() const noexcept;

    /// @brief Initialize a matrix with a constant value in each element
    /// @param rows Amount of rows in the matrix
    /// @param cols Amount of columns in the matrix
    /// @return The matrix
    static Matrix<T> init_const(size_t rows, size_t cols, T val);

    /// @brief Get an identity matrix
    /// @param dim The dimensionality of the identity matrix
    /// @return Identity matrix
    static Matrix<T> identity(size_t dim);

    /// @brief Join a list of column matrices
    /// @param cols The list of column matrices
    /// @return The merged matrix
    static Matrix<T> join_cols(::std::vector< Matrix<T> > &cols);

    /// @brief Combine a list of row matrices
    /// @param The list of row matrices
    /// @return The merged matrix
    static Matrix<T> combine_vertically(::std::vector< Matrix<T> > &matrices, size_t total_rows);

    /// @brief Get the number of rows in a matrix
    /// @return The number of rows
    size_t get_rows() const;

    /// @brief Get the number of rows in a matrix
    size_t get_cols() const;

    /// @brief Get an iterator (pointer) to the beginning of a matrix
    /// @return Raw pointer
    const T *begin() const;

    /// @brief Get a const iterator (pointer) to the end of a matrix
    /// @return Raw pointer
    const T *end() const;

    /// @brief Get an iterator (pointer) to the beginning of a matrix
    /// @return Raw pointer
    T *begin();
    /// @brief Get an iterator (pointer) to the end of a matrix
    /// @return Raw pointer
    T *end();

    /// @brief Get the number of rows in a Matrix (helpful for stl algorithms)
    /// @return The number of rows in the Matrix
    size_t size() const;

    /// @brief Relinquish ownership of the unique pointer with the matrix data
    /// @return The unique pointer with the matrix data
    ::std::unique_ptr<T[]> &&move_ptr();

    /// @brief Print a matrix
    void print_matrix() const;

  
  };

#include "Matrix.tpp"
};

#endif
