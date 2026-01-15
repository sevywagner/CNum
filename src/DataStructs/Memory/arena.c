#include "CNum/DataStructs/Memory/Arena.h"

/// @brief Free all nodes in a linked list
/// @param head The head node of a linked_list
static void linked_list_free(struct linked_list_node *head) {
  struct linked_list_node *node = head;
  while (node != NULL) {
    struct linked_list_node *temp_node = node;
    node = node->next;
    free(temp_node->ptr);
    free(temp_node);
  }
}

arena_t *arena_init(uint32_t blocks_to_allocate) {
  arena_t *arena = (arena_t *) malloc(sizeof(arena_t));
  size_t capacity = blocks_to_allocate * ARENA_BLOCK_SIZE;
  
  void *base_ptr = malloc(capacity);
  memset(base_ptr, 0, capacity);
  
  arena->heap_base = base_ptr;
  arena->heap_ptr = base_ptr;
  arena->capacity = capacity;
  arena->bytes_available = capacity;
  arena->heap_allocations_head = NULL;
  arena->heap_allocations_current = NULL;

  return arena;
}

arena_view_t arena_malloc(arena_t *arena, size_t bytes, size_t type_size) {
  struct linked_list_node *new_node = NULL;
  arena_view_t res = { NULL, 0, 0 };
  size_t blocks_to_allocate;
  size_t segment_size;
  void *ptr;
  
  if (bytes % type_size != 0) {
    goto out;
  }
  
  blocks_to_allocate = bytes / ARENA_BLOCK_SIZE;
  if (bytes % ARENA_BLOCK_SIZE > 0) {
    blocks_to_allocate++;
  }

  segment_size = blocks_to_allocate * ARENA_BLOCK_SIZE;
  
  if (arena->bytes_available < segment_size) {
    ptr = malloc(segment_size);
    if (!ptr) {
      goto out;
    }
    
    memset(ptr, 0, segment_size);

    new_node = (struct linked_list_node *) malloc(sizeof(struct linked_list_node));
    new_node->ptr = ptr;
    new_node->next = NULL;
    
    if (arena->heap_allocations_head == NULL) {
      arena->heap_allocations_head = new_node;
      arena->heap_allocations_current = new_node;
    } else {
      arena->heap_allocations_current->next = new_node;
      arena->heap_allocations_current = new_node;
    }
    
  } else {
    ptr = arena->heap_ptr;
    arena->heap_ptr = ((uint8_t *) arena->heap_ptr) + segment_size;
    arena->bytes_available -= segment_size;
  }
  
  res.ptr = ptr;
  res.range = bytes / type_size;
  res.type_size = type_size;

 out:
  return res;
}

void arena_view_reset(arena_view_t arena_view) {
  size_t total_bytes = arena_view.range * arena_view.type_size;
  int blocks_to_free = total_bytes / ARENA_BLOCK_SIZE;
  if (total_bytes % ARENA_BLOCK_SIZE > 0) {
    blocks_to_free++;
  }

  size_t block_range = blocks_to_free * ARENA_BLOCK_SIZE;
  
  memset(arena_view.ptr, 0, block_range);
}

/// @brief Free heap allocated pointers
///
/// This function frees all of the pointers that were allocated with malloc if
/// arena ever ran out of memory
/// @param arena Pointer to an arena
static void heap_allocations_free(arena_t *arena) {
  linked_list_free(arena->heap_allocations_head);
  arena->heap_allocations_head = NULL;
  arena->heap_allocations_current = NULL;
}

void arena_clear(arena_t *arena) {
  size_t range = (size_t) ((uint8_t *) arena->heap_ptr - (uint8_t *) arena->heap_base);
  memset(arena->heap_base, 0, range);
  arena->heap_ptr = arena->heap_base;
  arena->bytes_available = arena->capacity;
  heap_allocations_free(arena);
}

void arena_free(arena_t *arena) {
  heap_allocations_free(arena);
  free(arena->heap_base);
  free(arena);
}
