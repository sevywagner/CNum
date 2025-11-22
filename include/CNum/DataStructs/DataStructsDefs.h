#ifndef DATA_STRUCTS_DEFS_H
#define DATA_STRUCTS_DEFS_H

#include "CNum/DataStructs/Memory/Arena.h"

/**
 * @namespace CNum::DataStructs
 * @brief The data structures used in CNum
 */
namespace CNum::DataStructs {
  template <typename T>
  class Matrix;

  template <typename T>
  class ConcurrentQueue;

  using ::arena_t;
  using ::arena_view_t;

  /**
   * @namespace CNum::DataStructs::Arena
   * @brief A "mini-heap" used for thread local memory allocation
   */
  namespace Arena {
    using ::arena_init;
    using ::arena_malloc;
    using ::arena_clear;
    using ::arena_free;
  }
};

#endif
