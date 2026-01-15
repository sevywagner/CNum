#ifndef RAND_UTILS_H
#define RAND_UTILS_H

#include <cstdint>
#include <random>
#include <map>
#include <memory>
#include <shared_mutex>
#include <unordered_set>
#include <stdexcept>

#include "XoshiroCpp.hpp"
#include "CNum/Multithreading/ThreadPool.h"

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
  /// @param logical_id The logical id of the RNG stream
  template<typename T>
  void generate_n_unique_rand_in_range(size_t low_bound,
				       size_t high_bound,
				       T *out,
				       size_t n,
				       uint64_t logical_id = 0);
  
  /**
   * @class RandomGenerator
   * @brief A global service for random number generation
   *
   * The RandomGenerator class uses stateful logical-id based RNG streams. In order to ensure determinism, with this
   * version of CNum's RNG implementation 2 threads cannot share an RNG associated with a logical id
   * but can have their own RNGs associated with unique logical ids 
   */
  class RandomGenerator {
  private:
    static uint64_t _epoch;
    static uint64_t _seed;
    static ::std::map<uint64_t, ::XoshiroCpp::Xoshiro256PlusPlus> _registry;
    static ::std::map<uint64_t, uint64_t> _stream_local_epochs;
    static ::std::mutex _seed_mtx;
    
    /// @brief Default constructor
    RandomGenerator() = default;

  public:
    /// @brief Get the instance of the RandomGenerator
    static XoshiroCpp::Xoshiro256PlusPlus &instance(uint64_t logical_id = 0);

    /// @brief Set the seed for random number generation
    /// @param seed The new seed
    static void set_global_seed(uint64_t new_seed);

    /// @brief Reconstruct all thread local random generators
    static void reset_state();
  };

#include "CNum/Utils/RandUtils.tpp"
};

#endif
