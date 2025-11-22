#ifndef RAND_UTILS_H
#define RAND_UTILS_H

#include <random>

/**
 * @namesapace CNum::Utils::Rand
 * @brief Utilities for generating random numbers
 */
namespace CNum::Utils::Rand {
  /// @brief Generate n unique random integers
  /// @param low_bound The low bound of the integer values
  /// @param high_bound The high bound of the integer values
  /// @param out The pointer to where the random integers are stored
  /// @param n The number of unique random integers to generate
  template<typename T>
  void generate_n_unique_rand_in_range(size_t low_bound, size_t high_bound, T *out, int n);

  #include "CNum/Utils/RandUtils.tpp"
};

#endif
