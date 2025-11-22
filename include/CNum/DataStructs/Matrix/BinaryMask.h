#ifndef BINARY_MASK_H
#define BINARY_MASK_H

#include <memory>
#include <stdexcept>

namespace CNum::DataStructs {
  template <typename T>
  class Matrix;
  
  using BitMask = ::std::unique_ptr<uint8_t[]>;

  /**
   * @class BinaryMask
   * @brief A bit mask used for representing subsets of elements in a container
   *
   * A BinaryMask at its core is a bit mask where the nth bit represents index n in a container of
   * elements. The nth bit being set means the subset that the mask represents preserves that element (or
   * row of elements) at the nth index in the container. 
   */
  class BinaryMask {
  private:
    ::std::unique_ptr<uint8_t[]> _bit_mask;
    size_t _size;
    size_t _n_set;

    /// @brief Copy logic
    void copy(const BinaryMask &other) noexcept;

    /// @brief Move logic
    void move(BinaryMask &&other) noexcept;

    /// @brief Get the number of bytes to allocate for a number of bits
    /// @param The number of bits
    /// @return The number of bytes to allocate
    static size_t n_bytes_required(size_t bits) noexcept;
    
  public:
    BinaryMask() = delete;
    /// @brief Overloaded constructor
    /// @param mask The actual BitMask
    /// @param size The number of bits in the mask (excluding
    /// excess)
    BinaryMask(BitMask mask, size_t size, size_t n_set);

    /// @brief Copy constructor
    BinaryMask(const BinaryMask &other) noexcept;
    /// @brief Copy equals operator
    BinaryMask &operator=(const BinaryMask &other) noexcept;
    /// @brief Move constructor
    BinaryMask(BinaryMask &&other) noexcept;
    /// @brief Move equals operator
    BinaryMask &operator=(BinaryMask &&other) noexcept;
      
    /// @brief apply the mask to a Matrix
    /// @tparam The type of the Matrix
    /// @return The masked Matrix
    template <typename T>
    ::CNum::DataStructs::Matrix<T> mask(const ::CNum::DataStructs::Matrix<T> &m) const;

    /// @brief Create a binary mask
    /// @tparam T The type of container (holds data type U)
    /// @tparam U The data type stored in the container
    /// @tparam CompareFunction The function to use for comparison
    /// @return A binary mask
    template <typename T, typename U, typename CompareFunction>
    static BinaryMask create_binary_mask(const T &m, U val);

    /// @brief Create a BinaryMask for a Matrix
    /// @tparam T The type of the Matrix
    /// @tparam CompareFunction The function to use for comparison
    /// @return A binary mask
    template <typename T, typename CompareFunction>
    static BinaryMask create_binary_mask_matrix(const ::CNum::DataStructs::Matrix<T> &m, T val);
  };

  #include "CNum/DataStructs/Matrix/BinaryMask.tpp"
};

#endif
