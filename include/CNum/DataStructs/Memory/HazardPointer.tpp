// ---- Default delete function for pointers ----
template<typename T>
void default_hazard_ptr_delete(void *ptr) {
  delete static_cast<T *>(ptr);
}

// ---- Protect an atomic pointer ----
template<typename T>
T *protect(::std::atomic<T *> &src, HazardSlot *slot) {
  T *ptr = nullptr;
  do {
    ptr = src.load(::std::memory_order_acquire);
    slot->slot.store(ptr, ::std::memory_order_release);
  } while (ptr != src.load(::std::memory_order_acquire));

  return ptr;
}

// ---- Protect a raw pointer ----
template<typename T>
void protect(T *src, HazardSlot *slot) {
  slot->slot.store(src, ::std::memory_order_release);
}

// ---- Retire a pointer ----
template<typename T>
void retire(T *ptr) {
  tls_retire.push_back({ ptr, default_hazard_ptr_delete<T> });
  if (tls_retire.size() >= RETIRED_SCAN_THRESHOLD) {
    scan();
  }
}
