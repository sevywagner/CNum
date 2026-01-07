#ifndef RAND_UTILS_H
#define RAND_UTILS_H

#include "XoshiroCpp.hpp"
#include <cstdint>
#include <random>
#include <shared_mutex>
#include <unordered_set>
#include <stdexcept>

/**
 * @namespace CNum::Utils::Rand
 * @brief Utilities for generating random numbers
 */
namespace CNum::Utils::Rand {  
  /// @brief Generate n unique random integers
  /// @param low_bound The low bound of the integer values
  /// @param high_bound The high bound of the integer values
  /// @param out The pointer to where the random integers are stored
  /// @param n The number of unique random integers to generate
  template<typename T>
  void generate_n_unique_rand_in_range(size_t low_bound, size_t high_bound, T *out, size_t n);
  
  /**
   * @class RandomGenerator
   * @brief A global service for random number generation
   */
  class RandomGenerator {
  private:
    static uint64_t _epoch;
    static uint64_t _seed;
    static ::std::shared_mutex _seed_mtx;
    
    /// @brief Default constructor
    RandomGenerator() = default;

  public:
    /// @brief Get the instance of the RandomGenerator
    static XoshiroCpp::Xoshiro256PlusPlus &instance();

    /// @brief Set the seed for random number generation
    /// @param seed The new seed
    static void set_global_seed(uint64_t new_seed);

    /// @brief Reconstruct all thread local random generators
    static void reset_state();
  };

#include "CNum/Utils/RandUtils.tpp"
};

#endif
