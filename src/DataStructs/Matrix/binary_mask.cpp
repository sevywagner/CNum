#include "CNum/DataStructs/Matrix/BinaryMask.h"

namespace CNum::DataStructs {
  BinaryMask::BinaryMask(BitMask mask, size_t size, size_t n_set)
    : _size(size), _n_set(n_set), _bit_mask(::std::move(mask)) {
    if (_size == 0)
      throw ::std::invalid_argument("Binary Mask Constructor error - Size must be greater than 0");

    if (_bit_mask == nullptr)
      throw ::std::invalid_argument("Binary Mask Constructor error - Mask cannnot be a nullptr");
  }

  size_t BinaryMask::n_bytes_required(size_t bits) noexcept {
    size_t bytes = bits / 8; // 8 bits in a byte
    if (bits % 8 > 0)
      bytes++;

    return bytes;
  }

  void BinaryMask::copy(const BinaryMask &other) noexcept {
    if (this == &other) {
      return;
    }

    this->_size = other._size;
    this->_n_set = other._n_set;
    
    auto n = n_bytes_required(other._size);
    this->_bit_mask = ::std::make_unique<uint8_t[]>(n);
    ::std::copy(other._bit_mask.get(), other._bit_mask.get() + n, this->_bit_mask.get());
  }

  void BinaryMask::move(BinaryMask &&other) noexcept {
    if (this == &other)
      return;

    this->_size = other._size;
    this->_n_set = other._n_set;
    if (this->_bit_mask != nullptr)
      this->_bit_mask.reset();

    _bit_mask = ::std::move(other._bit_mask);
    other._bit_mask.reset();
    other._size = 0;
    other._n_set = 0;
  }

  BinaryMask::BinaryMask(const BinaryMask &other) noexcept {
    this->copy(other);
  }

  BinaryMask &BinaryMask::operator=(const BinaryMask &other) noexcept {
    this->copy(other);
    return *this;
  }

  BinaryMask::BinaryMask(BinaryMask &&other) noexcept {
    this->move(::std::move(other));
  }

  BinaryMask &BinaryMask::operator=(BinaryMask &&other) noexcept {
    this->move(::std::move(other));
    return *this;
  }
}
