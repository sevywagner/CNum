#include "CNum/DataStructs/Memory/HazardPointer.h"

namespace CNum::DataStructs::HazardPointer {
  HazardPtrDomain::HazardPtrDomain() {}

  // -------------------------
  // Hazard Domain Singleton
  // -------------------------
  
  HazardPtrDomain *HazardPtrDomain::get_domain() {
    // intentionally let the singleton leak to avoid shutdown order issues
    static HazardPtrDomain *domain = new HazardPtrDomain();
    return domain;
  }

  // ---- Acquire a slot to protect a hazard pointer ----
  HazardSlot *HazardPtrDomain::acquire_hazard_slot() {
    HazardSlot *slot = nullptr;
    for (int i = 0; i < MAX_HP_SLOTS; i++) {
      bool expected{ false };
      if (_global_slots[i].active.compare_exchange_strong(expected, true, ::std::memory_order_acq_rel)) {
	slot = &_global_slots[i];
	slot->slot.store(nullptr, ::std::memory_order_release);
	break;
      }
    }

    if (slot == nullptr)
      throw ::std::runtime_error("Hazard slot acquisition error - pool exhausted");

    return slot;
  }

  // ---- Release a hazard pointer slot ----
  void HazardPtrDomain::release_hazard_slot(HazardSlot *slot) {
    if (slot == nullptr)
      return;

    slot->slot.store(nullptr, ::std::memory_order_release);
    slot->active.store(false, ::std::memory_order_release);
  }

  // ---- Get a snapshot of all active slots ----
  void HazardPtrDomain::hazard_slots_gather(::std::vector<void *> &out) {
    out.clear();
    out.reserve(MAX_HP_SLOTS);
    for (size_t i = 0; i < MAX_HP_SLOTS; i++) {
      if (_global_slots[i].active.load(::std::memory_order_acquire)) {
	void *slot_ptr = _global_slots[i].slot.load(::std::memory_order_acquire);
	if (slot_ptr == nullptr)
	  continue;
      
	out.push_back(slot_ptr);
      }
    }
  }

  // --------------------
  // Hazard Guard Object
  // --------------------

  HazardPtrGuard::HazardPtrGuard() {
    auto *hp_domain = HazardPtrDomain::get_domain();
    for (size_t i = 0; i < SLOTS_PER_THREAD; i++) {
      _slots[i] = hp_domain->acquire_hazard_slot();
    }
  }

  HazardPtrGuard::~HazardPtrGuard() {
    auto *hp_domain = HazardPtrDomain::get_domain();
    for (int i = 0; i < SLOTS_PER_THREAD; i++) {
      hp_domain->release_hazard_slot(_slots[i]);
    }
  }

  // ---- Get a pointer to one of the guard's slots ----
  HazardSlot *HazardPtrGuard::get_slot(size_t idx) {
    if (idx >= SLOTS_PER_THREAD) {
      throw ::std::invalid_argument("Hazard slot indexing error - index out of range");
    }

    return _slots[idx];
  }

  // ---- Clear one of the guard's slots ----
  void HazardPtrGuard::clear_slot(size_t idx) {
    if (idx >= SLOTS_PER_THREAD) {
      throw ::std::invalid_argument("Hazard slot clearing error - index out of range");
    }

    _slots[idx]->slot.store(nullptr, ::std::memory_order_release);
  }

  // ---- Clear all of the guard's slots -----
  void HazardPtrGuard::clear_all_slots() {
    for (size_t i = 0; i < SLOTS_PER_THREAD; i++) {
      clear_slot(i);
    }
  }

  // ---- Free memory of retired pointers that are no longer hazardous ----
  void scan() {
    auto *hp_domain = HazardPtrDomain::get_domain();
    ::std::vector<void *> hazards;
    hp_domain->hazard_slots_gather(hazards);
    ::std::unordered_set<void *> hazard_set(hazards.begin(), hazards.end(), hazards.size());

    ::std::vector<Retire> keep;
    keep.reserve(tls_retire.size());

    for (auto &r: tls_retire) {
      if (hazard_set.find(r.ptr) == hazard_set.end()) {
	r.deleter(r.ptr);
      } else {
	keep.push_back(r);
      }
    }

    tls_retire.swap(keep);
  }

  // ---- Force a scan ----
  void flush() {
    while (!tls_retire.empty()) {
      scan();
    }
  }
}
