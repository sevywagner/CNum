#ifndef __HAZARD_POINTER_H
#define __HAZARD_POINTER_H

#include <atomic>
#include <vector>
#include <stdexcept>
#include <functional>
#include <unordered_set>

namespace CNum::DataStructs::HazardPointer {
  constexpr size_t MAX_HP_SLOTS = 1024;
  constexpr size_t SLOTS_PER_THREAD = 2;
  constexpr size_t RETIRED_SCAN_THRESHOLD = 64;

  struct HazardSlot {
    ::std::atomic<void *> slot{ nullptr };
    ::std::atomic<bool> active{ false };
  };

  struct Retire {
    void *ptr;
    ::std::function< void(void *) > deleter;
  };

  inline thread_local ::std::vector<Retire> tls_retire;

  // ---- Global domain of all hazard pointers ----
  class HazardPtrDomain {
  private:
    HazardSlot _global_slots[MAX_HP_SLOTS]{};

    HazardPtrDomain();
  public:
    static HazardPtrDomain *get_domain();

    HazardSlot *acquire_hazard_slot();
    void release_hazard_slot(HazardSlot *slot);

    void hazard_slots_gather(::std::vector<void *> &out);
  };

  // ---- Thread local guards ----
  class HazardPtrGuard {
  private:
    HazardSlot *_slots[SLOTS_PER_THREAD];
  public:
    HazardPtrGuard();
    HazardPtrGuard(const HazardPtrGuard &) = delete;
    HazardPtrGuard &operator=(const HazardPtrGuard &) = delete;
    ~HazardPtrGuard();
    
    HazardSlot *get_slot(size_t idx);
    void clear_slot(size_t idx);
    void clear_all_slots();
  };

  template<typename T>
  void default_hazard_ptr_delete(void *ptr);

  template<typename T>
  T *protect(::std::atomic<T *> &src, HazardSlot *slot);

  template<typename T>
  void protect(T *src, HazardSlot *slot);

  template<typename T>
  void retire(T *ptr);

  void scan();
  void flush();
  
  #include "CNum/DataStructs/Memory/HazardPointer.tpp"
};

#endif
