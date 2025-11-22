/// @brief Default delete function for pointers
/// @param ptr The pointer to free
template<typename T>
void default_hazard_ptr_delete(void *ptr) {
  delete static_cast<T *>(ptr);
}

/// @brief Protect an atomic pointer
///
/// Used for ensuring that a pointer shared by multiple threads does not get deleted (freed) when another is actively using it
/// @param src Atomic pointer to protect
/// @param slot The slot in which to store the pointer
/// @return The raw pointer
template<typename T>
T *protect(::std::atomic<T *> &src, HazardSlot *slot) {
  T *ptr = nullptr;
  do {
    ptr = src.load(::std::memory_order_acquire);
    slot->slot.store(ptr, ::std::memory_order_release);
  } while (ptr != src.load(::std::memory_order_acquire));

  return ptr;
}

/// @brief Protect a raw pointer
///
/// Used for ensuring that a pointer shared by multiple threads does not get deleted (freed) when another is actively using it
/// @param src The raw pointer to protect
/// @param slot The slot in which to store the pointer
template<typename T>
void protect(T *src, HazardSlot *slot) {
  slot->slot.store(src, ::std::memory_order_release);
}

/// @brief Retire a pointer
///
/// Mark protected pointer as ready to free
/// @param ptr The raw pointer to free
template<typename T>
void retire(T *ptr) {
  tls_retire.push_back({ ptr, default_hazard_ptr_delete<T> });
  if (tls_retire.size() >= RETIRED_SCAN_THRESHOLD) {
    scan();
  }
}
