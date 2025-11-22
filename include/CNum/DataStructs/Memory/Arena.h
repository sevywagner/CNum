#ifndef ARENA_H
#define ARENA_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief The block size of the arena
///
/// Used for detirmining padding of allocated memory. It is best to keep
/// this the size of a cache line generally
#define ARENA_BLOCK_SIZE 64
  
  /**
   * @struct linked_list_node
   * @brief A node in a linked list
   */
  struct linked_list_node {
    void *ptr;
    struct linked_list_node *next;
  };

  /**
   * @struct arena
   * @brief A thread local mini "heap" used for thread local memory allocation
   *
   * This data structure is used to maximize cache-friendliness by making
   * addressing predictable and by making allocations cache-line-aligned
   */
  typedef struct arena {
    void *heap_base;
    void *heap_ptr;
    size_t capacity;
    size_t bytes_available;
    struct linked_list_node *heap_allocations_head;
    struct linked_list_node *heap_allocations_current;
  } arena_t;

  /**
   * @struct arena_view
   * @brief A view of memory that was allocated in an arena
   *
   * When allocating memory in an arena using arena_malloc() this view is what
   * is returned
   */
  typedef struct arena_view {
    void *ptr;
    size_t range;
    size_t type_size;
  } arena_view_t;

  /// @brief Initialize Arena
  /// @param blocks_to_allocate The size of the arena in blocks
  /// @return A pointer to an arena
  arena_t *arena_init(uint32_t blocks_to_allocate);

  /// @brief Arena manual allocation with malloc fallback
  ///
  /// This function attempts to allocated memory in the heap and return a view
  /// that memory, but in the case that the arena is out of space
  /// will fall back on the standard malloc, but still return an arena_view_t
  /// @param arena Pointer to an arena
  /// @param bytes The number of bytes to allocate
  /// @param type_size The size of the data type being stored (for the view)
  /// @return A view of the allocated memory
  arena_view_t arena_malloc(arena_t *arena, size_t bytes, size_t type_size);

  /// @brief Free memory associated with an arena_view (within the arena itself)
  void arena_view_reset(arena_view_t arena_view);

  /// @brief Zero out an arena's memory
  ///
  /// This function does not use memset on the entire range of the arena's
  /// memory, but rather zeros the memory up to the internal pointer that
  /// keeps track of where the in the arena's memory the next allocation
  /// would take place. As this arena's implementation is intentionally simple,
  /// allocations are linear, so the memory behind this pointer is all of the
  /// previously allocated blocks.
  /// @param arena Pointer to an arena
  void arena_clear(arena_t *arena);

  /// @brief Free all memory associated with an arena
  void arena_free(arena_t *arena);

#ifdef __cplusplus
}
#endif
  
#endif
