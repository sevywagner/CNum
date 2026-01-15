#include "CNum/Utils/RandUtils.h"

namespace CNum::Utils::Rand {
  uint64_t RandomGenerator::_epoch = 0;
  uint64_t RandomGenerator::_seed = 0;
  ::std::map<uint64_t, ::XoshiroCpp::Xoshiro256PlusPlus> RandomGenerator::_registry;
  ::std::map<uint64_t, uint64_t> RandomGenerator::_stream_local_epochs;
  ::std::mutex RandomGenerator::_seed_mtx;

  constexpr uint64_t default_seed = 42;
  
  XoshiroCpp::Xoshiro256PlusPlus &RandomGenerator::instance(uint64_t logical_id) {
    ::std::lock_guard<::std::mutex> lg(_seed_mtx);

    if (_epoch == 0) {
      _seed = default_seed;
      _epoch++;
    }
    
    if (_stream_local_epochs[logical_id] != _epoch) {
      _registry[logical_id] = XoshiroCpp::Xoshiro256PlusPlus(_seed ^ logical_id);
      _stream_local_epochs[logical_id] = _epoch;
    }

    return _registry[logical_id];
  }

  void RandomGenerator::set_global_seed(uint64_t new_seed) {
    ::std::lock_guard<::std::mutex> lg(_seed_mtx);
    _seed = new_seed;
    _epoch++;
  }

  void RandomGenerator::reset_state() {
    ::std::lock_guard<::std::mutex> lg(_seed_mtx);
    _epoch++;
  }
}
