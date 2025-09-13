#ifndef __DATA_STRUCTS_H
#define __DATA_STRUCTS_H

#include "CNum/DataStructs/Matrix/Mask.h"
#include "CNum/DataStructs/Matrix/Matrix.h"
#include "CNum/DataStructs/ConcurrentQueue.h"
#include "CNum/DataStructs/Matrix/LinAlg.h"
#include "CNum/DataStructs/Views/Views.h"
#include "CNum/DataStructs/Memory/Arena.h"
#include "CNum/DataStructs/Memory/HazardPointer.h"

namespace CNum::DataStructs {
  template <typename T>
  class Matrix;

  template <enum MaskType MT, typename PT>
  class Mask;

  template <typename T>
  class ConcurrentQueue;

  using ::arena_t;
  using ::arena_view_t;

  namespace Arena {
    using ::arena_init;
    using ::arena_malloc;
    using ::arena_clear;
    using ::arena_free;
  }
};

#endif
