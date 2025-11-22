#ifndef STRIDE_VIEW_H
#define STRIDE_VIEW_H

#include <ranges>

namespace CNum::DataStructs::Views {
  template <typename T>
  class StrideIterator {
  private:
    T *_ptr;
    size_t _stride;

  public:
    /// @brief Overloaded default constructor
    StrideIterator(T *ptr = nullptr, size_t stride = 1);

    /// @brief Dereference iterator
    /// @return Raw pointer
    T operator*();

    /// @brief Increment iterator
    /// @return Iterator pointing to the next value in the view (increments _ptr by _stride)
    StrideIterator &operator++();

    /// @brief Does not equal comparison (by address)
    /// @param other The StrdeIterator to compare this one with
    /// @return Whether or not it doesn't equal other
    bool operator!=(const StrideIterator &other);
  };

  template <typename T>
  class StrideView {
  private:
    T *_ptr;
    size_t _stride, _range;
    StrideIterator<T> _begin, _end;
    
  public:
    /// @brief Overloaded default constructor
    /// @param ptr Raw pointer to the beginning of the view
    /// @param stride The amount of elements in the pointer between each element in the view
    /// @param range The range of the view (in strides)
    StrideView(T *ptr = nullptr, size_t stride = 1, size_t range = 0);

    /// @brief Get an iterator to the beginning of a view
    /// @return The iterator
    StrideIterator<T> begin() const;

    /// @brief Get an iterator to the end of the view
    /// @return The iterator
    StrideIterator<T> end() const;

    /// @brief Create a binary mask of values less than or equal to another
    /// @return Binary mask
    BinaryMask operator<=(T val);

    /// @brief Get the number of elements in the stride view (n_strides in the view)
    /// @return The number of elements
    size_t size() const;
  };

#include "CNum/DataStructs/Views/StrideView.tpp"
#include "CNum/DataStructs/Views/StrideIterator.tpp"
};

#endif
