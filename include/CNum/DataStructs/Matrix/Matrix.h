#ifndef __MATRIX_H
#define __MATRIX_H

#include "CNum/Multithreading/ThreadPool.h"
#include "CNum/DataStructs/Matrix/Mask.h"
#include "CNum/DataStructs/Views/StrideView.h"

#include <memory>
#include <functional>
#include <cmath>
#include <future>
#include <span>
#include <cstring>
#include <stdexcept>


namespace CNum::DataStructs {
  enum dim: uint8_t {
    ROW,
    COL
  };

  template <typename T>
  class Matrix {
  private:
    ::std::unique_ptr<T[]> _data;
    size_t _cols;
    size_t _rows;

    Matrix<T> element_wise(T val, ::std::function< void(T &, T) > func) const noexcept;
    void move(Matrix<T> &&other);
    void copy(const Matrix<T> &other);

    static void par_execute(uint8_t num_threads,
			    size_t total_el,
			    ::std::function< void(size_t) > callback);
  
    Mask<BIN, bool> comparison_mask(::std::function< bool(const T &a, const T &b) > comp, T val) const;
  
  public:
    Matrix(size_t rows = 0, size_t cols = 0, ::std::unique_ptr<T[]> ptr = nullptr);
  
    Matrix(const Matrix &other) noexcept;
    Matrix<T> &operator=(const Matrix &other) noexcept;
  
    Matrix(Matrix &&other) noexcept;
    Matrix<T> &operator=(Matrix &&other) noexcept;
  
    ~Matrix();

    Matrix<T> operator*(const Matrix &other) const;
    Matrix<T> operator*(T scale_factor) const noexcept;
    T dot(const Matrix<T> &other) const;

    Matrix<T> operator+(const Matrix &other) const;
    Matrix<T> operator+(T a) const noexcept;

    Matrix<T> operator-(const Matrix &other) const;
    Matrix<T> operator-(T a) const noexcept;

    Matrix<T> abs() const;
    Matrix<T> squared() const;
    Matrix<T> standardize() const;

    T sum() const;
    T mean() const;
    T std() const;
  
    T get(size_t row, size_t col) const;
    Matrix<T> get(enum dim d, size_t idx) const;

    CNum::DataStructs::Views::StrideView<T> get_col_view(size_t idx) const;
    ::std::span<T> get_row_view(size_t idx) const;
    
    Matrix<T> operator[](const Mask<BIN, bool> &other) const;
    Matrix<T> operator[](const Mask<IDX, uint32_t> &idx_mask) const noexcept;
    Matrix<T> col_wise_mask_application(const Mask<IDX, uint32_t> &idx_mask) const noexcept;
  
    Mask<BIN, bool> operator<(T val) const;
    Mask<BIN, bool> operator<=(T val) const;
    Mask<BIN, bool> operator>(T val) const;
    Mask<BIN, bool> operator>=(T val) const;
    Mask<BIN, bool> operator==(T val) const;
    Mask<BIN, bool> operator!=(T val) const;
  
    Mask<IDX, uint32_t> argsort() const;

    Matrix<T> transpose() const noexcept;
    static Matrix<T> join_cols(::std::vector< Matrix<T> > &cols);
    static Matrix<T> init_const (size_t rows, size_t cols, T val);
    static Matrix<T> identity(size_t dim);

    size_t get_rows() const;
    size_t get_cols() const;

    T *begin();
    T *end();
    ::std::unique_ptr<T[]> &&move_ptr();

    void print_matrix() const;

  
  };

#include "Matrix.tpp"
};

#endif
