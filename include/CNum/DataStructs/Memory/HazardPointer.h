#ifndef HAZARD_POINTER_H
#define HAZARD_POINTER_H

#include <atomic>
#include <vector>
#include <stdexcept>
#include <functional>
#include <unordered_set>

/**
 * @namespace CNum::DataStructs::HazardPointer
 * @brief Tools for protecting "hazerdous" objects in memory
 */
namespace CNum::DataStructs::HazardPointer {
  /// @brief The maximum amount of hazard pointer slots (globally)
  /// Add more as needed
  constexpr size_t MAX_HP_SLOTS = 1024;
  
  /// @brief The amount of slots 1 thread can have. 
  ///
  /// I assigned a value of 2 because 2 is all that is required for my lock-free
  /// concurrent queue. But for your own implementations of lock-free data
  /// structures you may need to increase this value.
  constexpr size_t SLOTS_PER_THREAD = 2;

  /// @brief The threshold of Retire objects in the tls_retire at which
  /// we call scan()
  constexpr size_t RETIRED_SCAN_THRESHOLD = 64;
 
  /**
   * @struct HazardSlot
   * @brief A slot in which to store a protected pointer
   */
  struct HazardSlot {
    ::std::atomic<void *> slot{ nullptr };
    ::std::atomic<bool> active{ false };
  };

  /**
   * @struct Retire
   * @brief A retired protected pointer
   */
  struct Retire {
    void *ptr;
    ::std::function< void(void *) > deleter;
  };

  /// @brief The thread local storage of retired objects
  inline thread_local ::std::vector<Retire> tls_retire;

  /**
   * @class HazardPtrDomain
   * @brief The domain of all hazard pointers
   */
  class HazardPtrDomain {
  private:
    HazardSlot _global_slots[MAX_HP_SLOTS]{};

    /// @brief Default constructor
    HazardPtrDomain();
  public:
    /// @brief Get a pointer to the instance of the HazardPtrDomain singleton
    /// @return A raw pointer the instance
    static HazardPtrDomain *get_domain();

    /// @brief Acquire ownership of a slot to protect a hazard pointer
    ///
    /// Used by the thread local HazardPtrGuard objects
    /// @return A raw pointer to an available HazardSlot
    HazardSlot *acquire_hazard_slot();

    /// @brief Release ownership of a hazard pointer slot
    ///
    /// Used by the thread local HazardPtrGuard objects
    void release_hazard_slot(HazardSlot *slot);

    /// @brief Get all active hazard pointer slots
    ///
    /// Used in scan()
    void hazard_slots_gather(::std::vector<void *> &out);
  };

  /**
   * @class HazardPtrGuard
   * @brief Thread local guards for hazard pointers
   */
  class HazardPtrGuard {
  private:
    HazardSlot *_slots[SLOTS_PER_THREAD];
  public:
    /// @brief Default constructor
    ///
    /// Acuires ownership of SLOTS_PER_THREAD hazard slots
    HazardPtrGuard();
    HazardPtrGuard(const HazardPtrGuard &) = delete;
    HazardPtrGuard &operator=(const HazardPtrGuard &) = delete;

    /// @brief Destructor
    /// 
    /// Releases ownership of the hazard slots
    ~HazardPtrGuard();

    /// @brief Get a pointer to a guard's slot
    /// @param idx The number of the slot you need the pointer of
    /// @return A raw pointer to a guard's hazard slot at index idx
    HazardSlot *get_slot(size_t idx);

    /// @brief Clear one of the guard's slots
    ///
    /// Set the slots store value to nullptr
    /// @param idx The index of the slot we want to clear
    void clear_slot(size_t idx);

    /// @brief Clear all of the guard's slots
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

  /// @brief Free memory of objects pointed to by pointers that are no longer 
  /// hazardous
  /// 
  /// Iterate over the tls_retire and check if the pointers are hazardous by
  /// checking if it is active in the global domain (i.e. marked hazardous by
  /// thread)
  void scan();

  /// @brief Force a scan
  void flush();
  
  #include "CNum/DataStructs/Memory/HazardPointer.tpp"
};

#endif
