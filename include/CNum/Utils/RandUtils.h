#ifndef __RAND_UTILS_H
#define __RAND_UTILS_H

#include <random>

namespace CNum::Utils::Rand {
  template<typename T>
    void generate_n_unique_rand_in_range(size_t low_bound, size_t high_bound, T *out, int n);

  #include "CNum/Utils/RandUtils.tpp"
};

#endif
