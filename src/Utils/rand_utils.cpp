#include "CNum/Utils/RandUtils.h"

namespace CNum::Utils::Rand {
  uint64_t RandomGenerator::_epoch = 0;
  uint64_t RandomGenerator::_seed = 0;
  ::std::shared_mutex RandomGenerator::_seed_mtx;
  
  XoshiroCpp::Xoshiro256PlusPlus &RandomGenerator::instance() {
    static ::std::random_device rd;
    thread_local XoshiroCpp::Xoshiro256PlusPlus rng(rd());
    thread_local uint64_t last_epoch{ 0 };
    ::std::shared_lock sl(_seed_mtx);
    if (last_epoch != _epoch) {
      rng = XoshiroCpp::Xoshiro256PlusPlus(_seed);
      last_epoch = _epoch;
    }
  
    return rng;
  }

  void RandomGenerator::set_global_seed(uint64_t new_seed) {
    ::std::lock_guard<::std::shared_mutex> lg(_seed_mtx);
    _seed = new_seed;
    _epoch++;
  }

  void RandomGenerator::reset_state() {
    ::std::lock_guard<::std::shared_mutex> lg(_seed_mtx);
    _epoch++;
  }
}
