#ifndef __STRIDE_VIEW_H
#define __STRIDE_VIEW_H

#include "CNum/DataStructs/Matrix/Mask.h"
#include <ranges>

namespace CNum::DataStructs::Views {
  template <typename T>
  class StrideIterator {
  private:
    T *_ptr;
    size_t _stride;

  public:
    StrideIterator(T *ptr = nullptr, size_t stride = 1);

    T operator*();
    StrideIterator &operator++();
    bool operator!=(const StrideIterator &other);
  };

  template <typename T>
  class StrideView {
  private:
    T *_ptr;
    size_t _stride, _range;
    StrideIterator<T> _begin, _end;

    CNum::DataStructs::Mask<BIN, bool> comparison_mask(T comp_val, std::function< bool(T, T) > comp);
    
  public:
    StrideView(T *ptr = nullptr, size_t stride = 1, size_t range = 0);

    StrideIterator<T> begin() const;
    StrideIterator<T> end() const;

    CNum::DataStructs::Mask<BIN, bool> operator<=(T val);
  };

#include "CNum/DataStructs/Views/StrideView.tpp"
#include "CNum/DataStructs/Views/StrideIterator.tpp"
};

#endif
