#include "CNum/DataStructs/Matrix/IndexMask.h"

namespace CNum::DataStructs {
  void IndexMask::copy(const IndexMask &other) noexcept {
    if (this == &other)
      return;

    this->_size = other._size;
    if (_mask != nullptr)
      _mask.reset();

    this->_mask = ::std::make_unique<size_t[]>(other._size);
    ::std::copy(other._mask.get(), other._mask.get() + other._size, this->_mask.get());
  }
  
  void IndexMask::move(IndexMask &&other) noexcept {
    if (this == &other)
      return;

    this->_size = other._size;
    if (this->_mask != nullptr)
      this->_mask.reset();

    this->_mask = ::std::move(other._mask);
    other._size = 0;
  }

  IndexMask::IndexMask(::std::unique_ptr<size_t[]> mask, size_t size)
    : _mask(::std::move(mask)), _size(size) {
    if (_size == 0)
      throw ::std::invalid_argument("Index mask constructor error - size must be greater than 0");

    if (!_mask)
      throw ::std::invalid_argument("Index mask constructor error - mask can not be nullptr");
  }
  
  IndexMask::IndexMask(const IndexMask &other) noexcept {
    this->copy(other);
  }
  
  IndexMask &IndexMask::operator=(const IndexMask &other) noexcept {
    this->copy(other);
    return *this;
  }
  
  IndexMask::IndexMask(IndexMask &&other) noexcept {
    this->move(::std::move(other));
  }
  
  IndexMask &IndexMask::operator=(IndexMask &&other) noexcept {
    this->move(::std::move(other));
    return *this;
  }
};
